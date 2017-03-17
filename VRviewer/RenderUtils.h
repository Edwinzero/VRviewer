#pragma once
#ifndef _RENDER_UTILS_H
#define _RENDER_UTILS_H
#include "config.h"

// Method to load the shader contents from a string
void LoadGLShaderFromFile(std::vector<char> &str, const char *path) {
	FILE *fp = fopen(path, "rb");
	if (!fp) {
		return;
	}

	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	str.resize(size + 1);
	fread(str.data(), 1, size, fp);
	str[size] = '\0';

	fclose(fp);
}

GLuint CompileGLShaderFromSource(const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader)
{
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(nSceneVertexShader, 1, &pchVertexShader, NULL);
	glCompileShader(nSceneVertexShader);

	GLint vShaderCompiled = 0;
	glGetShaderiv(nSceneVertexShader, GL_INFO_LOG_LENGTH, &vShaderCompiled);
	if (vShaderCompiled > 1)
	{
		printf("%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);
		GLchar* log = new char[vShaderCompiled + 1];
		glGetShaderInfoLog(nSceneVertexShader, vShaderCompiled, 0, log);
		printf("Log:: \n%s\n", log);
		delete log;
		glDeleteProgram(unProgramID);
		glDeleteShader(nSceneVertexShader);
		return 0;
	}
	glAttachShader(unProgramID, nSceneVertexShader);
	glDeleteShader(nSceneVertexShader); // the program hangs onto this once it's attached

	GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(nSceneFragmentShader, 1, &pchFragmentShader, NULL);
	glCompileShader(nSceneFragmentShader);

	GLint fShaderCompiled = 0;
	glGetShaderiv(nSceneFragmentShader, GL_INFO_LOG_LENGTH, &fShaderCompiled);
	if (fShaderCompiled > 1)
	{
		printf("%s - Unable to compile fragment shader %d!\n", pchShaderName, nSceneFragmentShader);
		GLchar* log = new char[fShaderCompiled + 1];
		glGetShaderInfoLog(nSceneFragmentShader, fShaderCompiled, 0, log);
		printf("Log:: \n%s\n", log);
		delete log;
		glDeleteProgram(unProgramID);
		glDeleteShader(nSceneFragmentShader);
		return 0;
	}
	glAttachShader(unProgramID, nSceneFragmentShader);
	glDeleteShader(nSceneFragmentShader); // the program hangs onto this once it's attached

	glLinkProgram(unProgramID);

	GLint programSuccess = GL_TRUE;
	glGetProgramiv(unProgramID, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE)
	{
		printf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram(unProgramID);
		return 0;
	}

	glUseProgram(unProgramID);
	glUseProgram(0);

	return unProgramID;
}

GLuint CompileGLShader(const char *ShaderName, const char *VertexShaderPath, const char *FragmentShaderPath) {
	std::vector<char> vert, frag;
	LoadGLShaderFromFile(vert, VertexShaderPath);
	LoadGLShaderFromFile(frag, FragmentShaderPath);
	return CompileGLShaderFromSource(ShaderName, vert.data(), frag.data());
}


#endif // !_RENDER_UTILS_H