#pragma once
#ifndef _GL_RENDER_OBJECT_H
#define _GL_RENDER_OBJECT_H
#include "config.h"
#include <vector>
#include <string>
#include <iostream>

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

#endif // !_GL_RENDER_OBJECT_H