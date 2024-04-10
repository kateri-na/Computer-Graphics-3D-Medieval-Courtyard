#include <iostream>
#include<GL/glew.h>
#include<GL/glut.h>
#include<GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<soil.h>

#include "camera.h"
#include "shader_s.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int loadCubemap(vector<std::string> faces);

void loadOBJ(std::string path, vector<Mesh>& meshes);
void processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransformation, vector<Mesh>& meshes);
Mesh processMesh(aiMesh* mesh);

// Константы
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Камера
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Тайминги
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Освещение
glm::vec3 lightPos(1.5f, -1.0f, 0.0f);
glm::vec3 lightColor(vec3(1.0, 0.99, 0.82));

int main()
{
	// glfw: инициализация и конфигурирование
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw: создание окна
	GLFWwindow* window = glfwCreateWindow(1400, 1100, "Medieval Castle Panina", NULL, 0);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Сообщаем GLFW, чтобы он захватил наш курсор
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Конфигурирование глобального состояния OpenGL
	glEnable(GL_DEPTH_TEST);

	// Компилирование нашей шейдерной программы

	Shader cubemapShader("cubemap.vs", "cubemap.fs");
	Shader lampShader("lamp.vs", "lamp.fs");
	Shader knightShader("knight.vs", "knight.fs");

	vector<Mesh> postLamp;
	loadOBJ("lamp_post.obj", postLamp);
	vector<Mesh> knight;
	loadOBJ("Knight.obj", knight);

	//cubemap - our scene 
	float cubemapVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// Загрузка и создание текстур
	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// Установка параметров наложения текстуры
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Установка параметров фильтрации текстуры
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Загрузка изображения, создание текстуры и генерирование мипмап-уровней
	int width, height, nrChannels;
	unsigned char* data = SOIL_load_image("Textures/wood.png", &width, &height, 0, SOIL_LOAD_RGB);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}	

	// Указываем OpenGL, какой сэмплер к какому текстурному блоку принадлежит (это нужно сделать единожды)
	knightShader.use();
	knightShader.setInt("texture", 0);

	// cubemap (skybox) VAO, VBO and loading faces textures
	unsigned int cubemapVAO, cubemapVBO;
	glGenVertexArrays(1, &cubemapVAO);
	glGenBuffers(1, &cubemapVBO);
	glBindVertexArray(cubemapVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), &cubemapVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	vector<std::string> cubemapFaces
	{
		"Textures/right.png",
		"Textures/left.png",
		"Textures/top.png",
		"Textures/bottom.png",
		"Textures/back.png",
		"Textures/front.png"
	};
	unsigned int cubemapTexture = loadCubemap(cubemapFaces);
	cubemapShader.use();
	cubemapShader.setInt("cubemap", 0);

	// Цикл рендеринга
	while (!glfwWindowShouldClose(window))
	{
		// Логическая часть работы со временем для каждого кадра
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Обработка ввода
		processInput(window);

		// Рендеринг
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // очищаем буфер цвета и буфер глубины

		// Привязка текстур к соответствующим текстурным юнитам
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), 0.4f / 0.3f, 0.1f, 100.0f);

		knightShader.use();
		knightShader.setVec3("lightColor", lightColor);
		knightShader.setVec3("lightPos", lightPos);
		knightShader.setMat4("projection", projection);
		knightShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, vec3(0.0f, -1.0f, 0.4f));
		model = glm::scale(model, glm::vec3(0.01f));
		knightShader.setMat4("model", model);
		for (auto mesh : knight)
			mesh.Draw();

		lampShader.use();
		lampShader.setVec3("lightColor", lightColor);
		lampShader.setVec3("lightPos", lightPos + vec3(1, 1, 1));
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.4f));;
		lampShader.setMat4("model", model);
		for (auto mesh : postLamp)
			mesh.Draw();

		glDepthFunc(GL_LEQUAL);
		cubemapShader.use();
		view = glm::mat4(glm::mat3(view));
		cubemapShader.setMat4("view", view);
		cubemapShader.setMat4("projection", projection);
		glBindVertexArray(cubemapVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// glfw: обмен содержимым front- и back- буферов. Отслеживание событий ввода\вывода (была ли нажата/отпущена кнопка, перемещен курсор мыши и т.п.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Опционально: освобождаем все ресурсы, как только они выполнили свое предназначение
	glDeleteBuffers(1, &cubemapVBO);

	// glfw: завершение, освобождение всех выделенных ранее GLFW-ресурсов
	glfwTerminate();
	return 0;
}

// Обработка всех событий ввода: запрос GLFW о нажатии/отпускании кнопки мыши в данном кадре и соответствующая обработка данных событий
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: всякий раз, когда изменяются размеры окна (пользователем или операционной системой), вызывается данная callback-функция
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// Убеждаемся, что окно просмотра соответствует новым размерам окна.
	// Обратите внимание, высота и ширина будут значительно больше, чем указано, на Retina-дисплеях
	glViewport(0, 0, width, height);
}

// glfw: всякий раз, когда перемещается мышь, вызывается данная callback-функция
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // перевернуто, так как y-координаты идут снизу вверх

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: всякий раз, когда прокручивается колесико мыши, вызывается данная callback-функция
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

//faces contain the location of all textures, needed for cubemap 
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = SOIL_load_image(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
			);
			SOIL_free_image_data(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			SOIL_free_image_data(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return textureID;
}

// loading model from the obj file
void loadOBJ(std::string path, vector<Mesh>& meshes)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType |
		aiProcess_FlipUVs);

	// If the import failed, report it
	if (nullptr == scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		std::cout << importer.GetErrorString() << '\n';
		std::cout << path << " ERROR LOADING MODEL\n";
	}
	else
		processNode(scene->mRootNode, scene, glm::mat4(1.0f), meshes);
}

// prosses all models from the given file from assimp format to glm format
// go through each row and column and set a correct value
void processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransformation, vector<Mesh>& meshes)
{
	glm::mat4 transformation;
	//first row
	transformation[0][0] = node->mTransformation.a1;
	transformation[0][1] = node->mTransformation.b1;
	transformation[0][2] = node->mTransformation.c1;
	transformation[0][3] = node->mTransformation.d1;
	//second row
	transformation[1][0] = node->mTransformation.a2;
	transformation[1][1] = node->mTransformation.b2;
	transformation[1][2] = node->mTransformation.c2;
	transformation[1][3] = node->mTransformation.d3;
	//third row
	transformation[2][0] = node->mTransformation.a3;
	transformation[2][1] = node->mTransformation.b3;
	transformation[2][2] = node->mTransformation.c3;
	transformation[2][3] = node->mTransformation.d3;
	//fourth row
	transformation[3][0] = node->mTransformation.a3;
	transformation[3][1] = node->mTransformation.b3;
	transformation[3][2] = node->mTransformation.c3;
	transformation[3][3] = node->mTransformation.d3;

	transformation = parentTransformation * transformation;

	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		Mesh mesh = processMesh(scene->mMeshes[node->mMeshes[i]]);
		mesh.transformation = transformation;
		meshes.push_back(mesh);
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene, transformation, meshes);
}

Mesh processMesh(aiMesh* mesh)
{
	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec2> texcoords;
	vector<uint32_t> indices;
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D vec = mesh->mVertices[i];
		aiVector3D norm = mesh->mNormals[i];
		aiVector3D tex = mesh->mTextureCoords[0][i];
		vertices.push_back(vec3(vec.x, vec.y, vec.z));
		normals.push_back(vec3(norm.x, norm.y, norm.z));
		texcoords.push_back(vec2(tex.x, tex.y));
	}
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	Mesh* res = new Mesh;
	return res->Create(vertices, indices, normals, texcoords);
}