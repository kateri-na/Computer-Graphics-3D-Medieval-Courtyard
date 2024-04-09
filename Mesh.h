#pragma once
#include <vector>
#include <glm/glm.hpp>
#include<gl/glew.h>

using glm::vec3;
using glm::vec2;
using std::vector;

class Mesh
{
private:
	vector<vec3> vertices;
	vector<uint32_t> indices;
	vector<vec3> normals;
	vector<vec2> texcoords;
	GLuint vao, vbo_vert, vbo_norm, vbo_tex, ebo;
public:
	Mesh Create(vector<vec3> vertices, vector<uint32_t> indices, vector<vec3> normals, vector<vec2> texcoords);
	void Draw() const;

	glm::mat4 transformation;
};

Mesh Mesh::Create(vector<vec3> vertices, vector<uint32_t> indices, vector<vec3> normals, vector<vec2> texcoords)
{
    this->vertices = vertices;
    this->indices = indices;
    this->normals = normals;
    this->texcoords = texcoords;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo_vert);
    glGenBuffers(1, &vbo_norm);
    glGenBuffers(1, &vbo_tex);
    glGenBuffers(1, &ebo);

    //Binding buffers
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);
    //position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glBindBuffer(GL_ARRAY_BUFFER, vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(vec2), texcoords.data(), GL_STATIC_DRAW);
    //texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_norm);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW);
    //normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return *this;
}

void Mesh::Draw() const
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);
}

