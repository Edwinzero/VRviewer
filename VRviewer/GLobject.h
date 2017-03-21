#pragma once
#ifndef _GL_RENDER_OBJECT_H
#define _GL_RENDER_OBJECT_H
#include "config.h"
#include <vector>
#include <string>
#include <iostream>


#ifndef _GL_VR_OBJECT_H
#define _GL_VR_OBJECT_H
class GLVRobject {
	GLuint m_vbo;
	GLuint m_ibo;
	GLuint m_vao;
	GLuint m_tex;
	GLsizei m_vertCount;
	std::string m_modelName;

public:
	GLVRobject(const std::string &modelName) : m_modelName(modelName){
		m_ibo = -1;
		m_tex = -1;
		m_vao = -1;
		m_vbo = -1;
	}

	~GLVRobject() { Cleanup(); }

	bool InitBuffer(const vr::RenderModel_t &vrModel, const vr::RenderModel_TextureMap_t &vrDiffuseTex);
	void Cleanup();
	void Draw();
	const std::string &GetName() const { return m_modelName; }
};

bool GLVRobject::InitBuffer(const vr::RenderModel_t &vrModel, const vr::RenderModel_TextureMap_t &vrDiffuseTex) {
	// create and bind a VAO to hold state for this model
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Populate a vertex buffer
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW);

	// Identify the components in the vertex buffer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vPosition));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vNormal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));

	// Create and populate the index buffer
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// create and populate the texture
	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vrDiffuseTex.unWidth, vrDiffuseTex.unHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, vrDiffuseTex.rubTextureMapData);

	// If this renders black ask McJohn what's wrong.
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_vertCount = vrModel.unTriangleCount * 3;

	return true;
}

void GLVRobject::Cleanup() {
	if (m_vbo)
	{
		glDeleteBuffers(1, &m_ibo);
		glDeleteVertexArrays(1, &m_vao);
		glDeleteBuffers(1, &m_vbo);
		m_ibo = -1;
		m_vao = -1;
		m_vbo = -1;
	}
}
void GLVRobject::Draw() {
	glBindVertexArray(m_vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_tex);

	glDrawElements(GL_TRIANGLES, m_vertCount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
}
#endif // !_GL_VR_OBJECT_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _GL_MODEL_OBJECT_H
#define _GL_MODEL_OBJECT_H

class GLobject {
	GLuint m_vbo;
	GLuint m_ibo;
	GLuint m_vao;
	GLuint m_tex;
	GLuint m_ubo;
	GLuint m_nbo;
	GLsizei m_vertCount;
	GLsizei m_indiceCount;
	std::string m_modelName;
public:
	GLobject(const std::string &modelName) : m_modelName(modelName) {
		m_ibo = -1;
		m_tex = -1;
		m_vao = -1;
		m_vbo = -1;
		m_ubo = -1;
		m_nbo = -1;
	}

	~GLobject() { Cleanup(); }

	void InitBuffer(std::vector<float> vertices, std::vector<float> normals, std::vector<float> uvs, std::vector<unsigned int> indices);
	bool BindTexture();
	void Cleanup();
	void DrawIBO();
	void DrawVBO();
	const std::string &GetName() const { return m_modelName; }
};

void GLobject::InitBuffer(std::vector<float> vertices, std::vector<float> normals, std::vector<float> uvs, std::vector<unsigned int> indices) {
	m_vertCount = vertices.size() / 3;
	size_t numNormals = normals.size() / 3;
	size_t numUVs = uvs.size() / 2;
	size_t numIndices = indices.size();

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	/// position
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*m_vertCount * 3, vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0); // bind vao to vbo
				
	/// normal
	if (numNormals > 0) {
		glEnableVertexAttribArray(1);
		glGenBuffers(1, &m_nbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_nbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*numNormals * 3, normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0); // bind vao to vbo
	}
	/// uvs
	if (numUVs > 0) {
		glEnableVertexAttribArray(2);
		glGenBuffers(1, &m_ubo);
		glBindBuffer(GL_ARRAY_BUFFER, m_ubo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*numUVs * 2, uvs.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)0); // bind vao to vbo
	}

	// IBO
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*numIndices, indices.data(), GL_STATIC_DRAW);
	m_indiceCount = numIndices;
	// Reset State
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

// TODO
bool GLobject::BindTexture() {
	return false;
}

void GLobject::Cleanup() {
	if (m_vbo)
	{
		glDeleteVertexArrays(1, &m_vao);
		glDeleteBuffers(1, &m_vbo);
		m_vao = -1;
		m_vbo = -1;
	}
	if (m_nbo) {
		glDeleteBuffers(1, &m_nbo);
		m_nbo = -1;
	}
	if (m_ibo) {
		glDeleteBuffers(1, &m_ibo);
		m_ibo = -1;
	}
	if (m_ubo) {
		glDeleteBuffers(1, &m_ubo);
		m_ubo = -1;
	}
}
void GLobject::DrawIBO() {
	glBindVertexArray(m_vao);

	if (m_tex) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_tex);
	}

	glDrawElements(GL_TRIANGLES, m_vertCount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
}

void GLobject::DrawVBO() {
	glBindVertexArray(m_vao);
	if (m_tex) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_tex);
	}
	glDrawArrays(GL_TRIANGLES, 0, m_vertCount);
	glBindVertexArray(0);
}
#endif // !_GL_MODEL_OBJECT_H

#endif // !_GL_RENDER_OBJECT_H