#pragma once
#ifndef _VR_RENDER_H
#define _VR_RENDER_H
#include "RenderUtils.h"
#include "GLobject.h"
#include "TestScene.h"

using namespace std;
unsigned int screenWidth = 800;
unsigned int screenHeight = 800;
bool reComplieShader = false;
GLuint GLVRRenderProgram = -1;

// ** HMD object to commu with vive
typedef struct HMD {
	vr::IVRSystem				*m_HMD;
	vr::IVRRenderModels			*m_RenderModels;
	vr::TrackedDevicePose_t		m_TrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	std::string					m_strDriver;
	std::string					m_strDisplay;
	glm::mat4					m_mat4DevicePose[ vr::k_unMaxTrackedDeviceCount ];		// get pose of each device
	bool						m_ShowTrackedDevice[ vr::k_unMaxTrackedDeviceCount ]; // flag
}HMD;

// ** HMD info that GL tracks (flags for rendering VIVE dev)
typedef struct GLHMDinfo {
	int m_TrackedControllerCount;
	int m_TrackedControllerCount_Last;
	int m_ValidPoseCount;
	int m_ValidPoseCount_Last;
}GLHMDinfo;


//=========================================================================
//		Draw methods
//=========================================================================
void DrawScene2D() {
	{
		//glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
		glDisable(GL_DEPTH_TEST);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		// Setup viewport, orthographic projection matrix
		glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0f, screenWidth, 0.0f, screenHeight, -1.0f, +1.0f);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// Draw a triangle at 0.5 z
		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(50.5, 50.5, 0.5);
		glVertex3f(550.5, 50.5, 0.5);
		glVertex3f(550.0, 150.5, 0.5);
		glEnd();

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		//glPopAttrib();
	}
}

void DrawScene3D() {
	{
		//glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
		glEnable(GL_DEPTH_TEST);
		// Setup viewport, orthographic projection matrix
		glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPerspective(45.0, (GLdouble)screenWidth / (GLdouble)screenHeight, 1.0, 10000.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//glMultMatrixf(&camera.Mat()[0][0]);

		// default coord grids
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		{
			glLineWidth(2.0);
			DrawCoord();
			glColor3f(0, 1, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 500, 0);
			glEnd();
		}
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		// Point cloud data
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		//glPopAttrib();
	}
}

void DrawToHMD() {

}

//=========================================================================
//		Render
//=========================================================================
void Render(void)
{
	// Get Back to the Modelview
	glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);

	if (1) {
		// draw 3D scene
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		DrawScene3D();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
	if (1) {
		// draw 2D scene
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		DrawScene2D();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
	if (1) {
		DrawToHMD();
	}
	
	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}
//=========================================================================
//		Reshape
//=========================================================================
void Reshape(int w, int h)
{
	// update screen width and height for imgui new frames
	screenWidth = w;
	screenHeight = h;

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;
	float ratio = 1.0f* w / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	//gluPerspective(45, ratio, 1.0f, 10000.0f);
	//glMatrixMode(GL_PROJECTION);
	//glViewport(0, 0, width, height);
	//glLoadIdentity();
	//glOrtho(0.f, w, 0.f, h, -1.f, 1.f);
	//glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);
	glutPostRedisplay();
}
//=========================================================================
//		Update
//=========================================================================
void Update(void) {
	if (reComplieShader) {
		//GLVRRenderProgram = CompileGLShader("PointRender", "Shaders/PointRender.vs", "Shaders/PointRender.fs");
		reComplieShader = false;
	}
}

//=========================================================================
//		keyboard & mouse callback
//=========================================================================
bool keyboardEvent(unsigned char nChar, int nX, int nY)
{

	if (nChar == 27) { //Esc-key
		glutLeaveMainLoop();
	}
	if (nChar == 't' || nChar == 'T') {
		screenWidth = 1920;
		screenHeight = 1080;
	}
	if (nChar == 'c') {
		reComplieShader = !reComplieShader;
	}

	return true;
}

void KeyboardSpecial(int key, int x, int y)
{
	
}

void keyboardCallback(unsigned char nChar, int x, int y)
{
	if (keyboardEvent(nChar, x, y))
	{
		glutPostRedisplay();
	}
}

bool mouseEvent(int button, int state, int x, int y)
{
	//if (state == GLUT_DOWN && (button == GLUT_LEFT_BUTTON))
	//if (state == GLUT_DOWN && (button == GLUT_RIGHT_BUTTON))
	//if (state == GLUT_DOWN && (button == GLUT_MIDDLE_BUTTON))
	return true;
}

void MouseWheel(int button, int dir, int x, int y)
{

	if (dir > 0)
	{
		// Zoom in

	}
	else if (dir < 0)
	{
		// Zoom out

	}
}

void MouseCallback(int button, int state, int x, int y)
{
	if (mouseEvent(button, state, x, y))
	{

	}
}

void MouseDragCallback(int x, int y)
{
	glutPostRedisplay();
}

void MouseMoveCallback(int x, int y)
{
	glutPostRedisplay();
}

//=========================================================================
//		Initialize platforms
//=========================================================================
void Init_GLshader(void) {
	GLVRRenderProgram = CompileGLShader("PointRender", "Shaders/PointRender.vs", "Shaders/PointRender.fs");
}
// initialize ogl and imgui
void Init_OpenGL(int argc, char **argv, const char* title)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 0);
	//glutInitContextFlags(GLUT_FORWARD_COMPATIBLE); // cause error
	glutInitContextProfile(GLUT_CORE_PROFILE);

	//glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);  // reduce speed...
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);

	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(200, 200);
	glutCreateWindow(title);
	fprintf(stdout, "INFO: OpenGL Version: %s\n", glGetString(GL_VERSION));

	// glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		getchar();
		return;
	}
	adfa
	// shaders
	//Init_GLshader();

	// callback
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);
	glutIdleFunc(Update);
	glutKeyboardFunc(keyboardCallback);
	glutSpecialFunc(KeyboardSpecial);
	glutMouseFunc(MouseCallback);
	glutMouseWheelFunc(MouseWheel);
	glutMotionFunc(MouseDragCallback);
	glutPassiveMotionFunc(MouseMoveCallback);
}

void Init_RenderScene(void) {

}

void Init_HMD(void) {

}

void sample_vr_viewer(int argc, char **argv) {
	GLVRobject obj("hehe");
	Init_OpenGL(argc, argv, "VR render");
	Init_RenderScene();
	//Init_GLshader();

	glutMainLoop();
}
#endif // !_VR_RENDER_H


