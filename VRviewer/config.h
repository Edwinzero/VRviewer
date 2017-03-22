#pragma once
#ifndef _CONFIG_H
#define _CONFIG_H
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <vector>
#include <iostream>

#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <GL\freeglut.h>

#include <openvr.h>

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	printf("GL Error: %s\n", message);
}

void ThreadSleep(unsigned long nMilliseconds)
{
#if defined(_WIN32)
	::Sleep(nMilliseconds);
#elif defined(POSIX)
	usleep(nMilliseconds * 1000);
#endif
}

#endif // !_CONFIG_H