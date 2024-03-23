// TextureCube.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.

#include <iostream>
#include<GL/glew.h>
#include<GL/glut.h>
#include<GLFW/glfw3.h>
#include"shader_s.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<soil.h>
#include "camera.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Константы
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Камера
Camera camera(glm::vec3(0.0f, 0.0f, 3.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Тайминги
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Освещение
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main()
{
	// glfw: инициализация и конфигурирование
	//glfwInit();
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// glfw: создание окна
	GLFWwindow* window = glfwCreateWindow(1100, 800, "Triangular Prism", NULL, 0);
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

	// Конфигурирование глобального состояния OpenGL
	glEnable(GL_DEPTH_TEST);

	// Компилирование нашей шейдерной программы
	Shader ourShader("6.2.coordinate_systems.vs", "6.2.coordinate_systems.fs");
	Shader lightingShader("basic_lighting.vs", "basic_lighting.fs");
	Shader lampShader("lamp.vs", "lamp.fs");

	// Указание вершин (и буфера(ов)) и настройка вершинных атрибутов
	float vertices[] = {
		// координаты        // текстурные координаты
		0.5f, 0.5f, 0.0f,  0.0f, 0.0f, //1
		0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
		0.5f,  0.0f, 0.5f,  1.0f, 1.0f,
		//2
		0.5f, -0.5f,  0.0f,  0.0f, 0.0f, //B
		-0.5f, -0.5f,  0.0f,  1.0f, 0.0f, //D
		-0.5f,  0.0f,  0.5f,  1.0f, 1.0f, //F
		-0.5f,  0.0f,  0.5f,  1.0f, 1.0f, //F
		0.5f,  0.0f,  0.5f,  0.0f, 1.0f, //C
		0.5f, -0.5f,  0.0f,  0.0f, 0.0f, //B
		//3
		0.5f,  0.5f,  0.0f,  1.0f, 0.0f, //A
	   -0.5f,  0.5f,  0.0f,  1.0f, 1.0f, //E
	   -0.5f,  0.0f,  0.5f,  0.0f, 1.0f, //F
	   -0.5f,  0.0f,  0.5f,  0.0f, 1.0f, //F
		0.5f,  0.0f,  0.5f,  0.0f, 0.0f, //C
		0.5f,  0.5f,  0.0f,  1.0f, 0.0f, //A
		//4
		0.5f,  0.5f,  0.0f,  1.0f, 0.0f, //A
		-0.5f,  0.5f,  0.0f,  1.0f, 1.0f, //E
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, //D
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, //D
		0.5f, -0.5f,  0.0f,  0.0f, 0.0f, //B
		0.5f,  0.5f,  0.0f,  1.0f, 0.0f, //A

		-0.5f, 0.5f, 0.0f,  0.0f, 1.0f, //5
		-0.5f, -0.5f, 0.0f,  1.0f, 1.0f,
		-0.5f, 0.0f,  0.5f,  1.0f, 0.0f,

	};
	unsigned int indices[] = {
		0, 1, 3, // первый треугольник
		1, 2, 3  // второй треугольник
	};
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Координатные атрибуты
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Цветовые атрибуты
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// 2. Настраиваем VAO света (VBO остается неизменным)
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Обратите внимание, что мы обновляем шаг атрибута положения лампы, чтобы отразить обновленные данные буфера
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Загрузка и создание текстур
	unsigned int texture1;

	// Текстура №1 - цветок
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
	//stbi_set_flip_vertically_on_load(true); // указываем stb_image.h на то, чтобы перевернуть для загруженной текстуры ось y
	unsigned char* data = SOIL_load_image("Textures/texture2.png", &width, &height, 0, SOIL_LOAD_RGB);
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
	ourShader.use();
	ourShader.setInt("texture", 0);

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

		// Убеждаемся, что активировали шейдер прежде, чем настраивать uniform-переменные/объекты_рисования
		lightingShader.use();
		lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.setVec3("lightPos", lightPos);

		// Активируем шейдер
		ourShader.use();

		//// Создаем преобразование
		//glm::mat4 model = glm::mat4(1.0f); // сначала инициализируем единичную матрицу
		//glm::mat4 view = glm::mat4(1.0f);
		//glm::mat4 projection = glm::mat4(1.0f);
		////model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
		//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.5f));
		//projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		// Преобразования Вида / Проекции
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 0.4f / 0.3f, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		// Мировое преобразование
		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);

		// Получаем местоположение uniform-матриц...
		unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
		unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
		// ...передаем их в шейдеры (разными способами)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

		// Примечание: В настоящее время мы устанавливаем матрицу проекции для каждого кадра, но поскольку матрица проекции редко меняется, то рекомендуется устанавливать её (единожды) вне основного цикла
		ourShader.setMat4("projection", projection);

		// Рендерим ящик
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Также отрисовываем наш объект-"лампочку" 
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // куб меньшего размера
		lampShader.setMat4("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// glfw: обмен содержимым front- и back- буферов. Отслеживание событий ввода\вывода (была ли нажата/отпущена кнопка, перемещен курсор мыши и т.п.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Опционально: освобождаем все ресурсы, как только они выполнили свое предназначение
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

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
