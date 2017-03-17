#pragma once
#ifndef _CONFIG_H
#define _CONFIG_H
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <vector>
#include <iostream>

#include <GL\glew.h>
#include <GL\freeglut.h>

#include <openvr.h>

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

void ThreadSleep(unsigned long nMilliseconds)
{
#if defined(_WIN32)
	::Sleep(nMilliseconds);
#elif defined(POSIX)
	usleep(nMilliseconds * 1000);
#endif
}

#endif // !_CONFIG_H