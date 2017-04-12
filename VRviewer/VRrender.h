#pragma once
#ifndef _VR_RENDER_H
#define _VR_RENDER_H
#include "3rdParty\lodepng.h"
#include "plyloader.h"
#include "RenderUtils.h"
#include "GLobject.h"
#include "TestScene.h"
#include "HMD.h"
using namespace std;

unsigned int screenWidth = 1280;
unsigned int screenHeight = 640;
bool reComplieShader = false;
GLuint GLsceneProgram = -1;
GLuint GLdesktopWindowProgram = -1;

GLuint GLMocaPointRenderProgram = -1;

// scene
typedef struct CubeSea {
	unsigned int m_uiVertcount;
	GLuint m_glSceneVertBuffer;
	GLuint m_unSceneVAO;

	CubeSea() { 
		m_uiVertcount = 0;
		m_glSceneVertBuffer = -1;
		m_unSceneVAO = -1;
	}
}CubeSea;

GLuint cube_tex = -1;		// m_iTexture 
CubeSea cubes;

GLobject point_cloud_scene;

// RENDER

GLobject companionWnd;

GLuint sceneMatrixLocation = -1;			// gpu uniform matrix



//=========================================================================
//		HMD object
//=========================================================================
VIVE_HMD vive;

//=========================================================================
//		HMD draw methods
//=========================================================================
// render global scene
void RenderGlobalScene(float *vive_to_eye) {
	
	//glUseProgram(GLsceneProgram);
	//glUniformMatrix4fv(sceneMatrixLocation, 1, GL_FALSE, glm::value_ptr(GetCurrentViewProjectionMatrix(vive, eye)));
	//PrintGLMmat4(GetCurrentViewProjectionMatrix(vive, eye));
	//glBindVertexArray(cubes.m_unSceneVAO);
	//glBindTexture(GL_TEXTURE_2D, cube_tex);
	//glDrawArrays(GL_TRIANGLES, 0, cubes.m_uiVertcount);
	//glBindVertexArray(0);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glUseProgram(GLMocaPointRenderProgram);
	glm::mat4 model = glm::rotate(45.0f, glm::vec3(0.0, 1.0, 0.0)); // hard code
	model *= glm::translate(glm::vec3(-1.0, 0.0, -1.5));			// hard code (this mat can be replaced by vicon mat (m_to_w = mat_vicon_to_vive * mat_model_to_vicon)
	glUniformMatrix4fv(glGetUniformLocation(GLMocaPointRenderProgram, "model_to_vive"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(GLMocaPointRenderProgram, "vive_to_eye"), 1, GL_FALSE, vive_to_eye);
	glUniform3fv(glGetUniformLocation(GLMocaPointRenderProgram, "color"), 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)));
	glBindVertexArray(point_cloud_scene.m_vao);
	glDrawArrays(GL_POINTS, 0, point_cloud_scene.m_vertCount);
	glBindVertexArray(0);
	glDisable(GL_PROGRAM_POINT_SIZE);
}
//=========================================================================
//		Draw methods
//=========================================================================
void RenderCompanionWindow(VIVE_HMD &vive) {

	if (!vive.m_HMD) {
		return;
	}
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, screenWidth, screenHeight);

	glBindVertexArray(companionWnd.m_vao);
	glUseProgram(GLdesktopWindowProgram);

#if 1
	// render left eye (first half of index array )
	glBindTexture(GL_TEXTURE_2D, vive.m_leftEye.m_resolveTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, companionWnd.m_indiceCount / 2, GL_UNSIGNED_SHORT, 0);

	// render right eye (second half of index array )
	glBindTexture(GL_TEXTURE_2D, vive.m_rightEye.m_resolveTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, companionWnd.m_indiceCount / 2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(companionWnd.m_indiceCount));
#endif

	glBindVertexArray(0);
	glUseProgram(0);
}

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


//=========================================================================
//		Render
//=========================================================================
void Render(void)
{
	// Get Back to the Modelview
	//glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
	//glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//glEnable(GL_DEPTH_TEST);

	if (1) {
		vive.DrawToHMD(RenderGlobalScene);
		RenderCompanionWindow(vive);
	}
	if (0) {
		// draw 3D scene
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		DrawScene3D();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
	if (0) {
		// draw 2D scene
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		DrawScene2D();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
	
	glFlush();
	glFinish();
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

	glutPostRedisplay();
}
//=========================================================================
//		Update
//=========================================================================
// hhmd
glm::mat4 ConvertSteamVRMatrixToMat4(const vr::HmdMatrix34_t &matPose)
{
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}
void Update(void) {
	if (reComplieShader) {
		//GLVRRenderProgram = CompileGLShader("PointRender", "Shaders/PointRender.vs", "Shaders/PointRender.fs");
		GLsceneProgram = CompileGLShader("SceneRender", "Shaders/Scene.vs", "Shaders/Scene.fs");
		vive.GLcontrollerTransformProgram = CompileGLShader("ControllerTransform", "Shaders/Controller.vs", "Shaders/Controller.fs");
		vive.GLHMDdeviceRenderModelProgram = CompileGLShader("RenderVIVEdevice", "Shaders/RenderVIVEdevice.vs", "Shaders/RenderVIVEdevice.fs");
		GLdesktopWindowProgram = CompileGLShader("DesktopWindow", "Shaders/DesktopWindow.vs", "Shaders/DesktopWindow.fs");
		reComplieShader = false;
	}

	

	vive.UpdateHMDPose();

	vive.VRhandleInput();
}


//=========================================================================
//		keyboard & mouse callback
//=========================================================================
bool keyboardEvent(unsigned char nChar, int nX, int nY)
{

	if (nChar == 27) { //Esc-key
		glutLeaveMainLoop();
		vive.VRshutdown();
		companionWnd.Cleanup();
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
	GLsceneProgram = CompileGLShader("SceneRender", "Shaders/Scene.vs", "Shaders/Scene.fs");
	sceneMatrixLocation = glGetUniformLocation(GLsceneProgram, "matrix");
	if (sceneMatrixLocation == -1)
	{
		printf("Unable to find matrix uniform in scene shader\n");
		return;
	}
#if 0 
	GLcontrollerTransformProgram = CompileGLShader("ControllerTransform", "Shaders/Controller.vs", "Shaders/Controller.fs");
	controllerMatrixLocation = glGetUniformLocation(GLcontrollerTransformProgram, "matrix");
	if (controllerMatrixLocation == -1)
	{
		printf("Unable to find matrix uniform in scene shader\n");
		return;
	}
	GLHMDdeviceRenderModelProgram = CompileGLShader("RenderVIVEdevice", "Shaders/RenderVIVEdevice.vs", "Shaders/RenderVIVEdevice.fs");
	renderModelMatrixLocation = glGetUniformLocation(GLHMDdeviceRenderModelProgram, "matrix");
	if (renderModelMatrixLocation == -1)
	{
		printf("Unable to find matrix uniform in scene shader\n");
		return;
	}
#endif
	GLdesktopWindowProgram = CompileGLShader("DesktopWindow", "Shaders/DesktopWindow.vs", "Shaders/DesktopWindow.fs");
	GLMocaPointRenderProgram = CompileGLShader("MOCA_pointCloud", "Shaders/PointCloudRender.vs", "Shaders/PointCloudRender.fs");
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
}
void SetupTextureMap() {
	std::string path = "cube_texture.png";
	std::vector<unsigned char> imageRGBA;
	unsigned int imgWidth, imgHeight;
	unsigned int error = lodepng::decode(imageRGBA, imgWidth, imgHeight, path);
	if (error != 0) {
		printf("Decode png file failed ... \n");
		return;
	}

	glGenTextures(1, &cube_tex);
	glBindTexture(GL_TEXTURE_2D, cube_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, &imageRGBA[0]);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// anisotropy
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void SetupCubeScene() {
	//if (!vive.m_HMD)
	//	return;
	std::vector<float> vertdataarray;

	glm::mat4 matScale;
	float m_fScale = 0.3f;
	float m_fScaleSpacing = 4.0f;
	matScale = glm::scale(matScale,glm::vec3(m_fScale));

	//PrintGLMmat4(matScale, "matScale");

	int m_iSceneVolumeWidth = 20, m_iSceneVolumeHeight = 20, m_iSceneVolumeDepth = 20;
	glm::mat4 matTransform;
	matTransform = glm::translate( matTransform, glm::vec3(
		-((float)m_iSceneVolumeWidth * m_fScaleSpacing) / 2.f,
		-((float)m_iSceneVolumeHeight * m_fScaleSpacing) / 2.f,
		-((float)m_iSceneVolumeDepth * m_fScaleSpacing) / 2.f));

	//PrintGLMmat4(matTransform, "matTransform");

	glm::mat4 mat = matScale * matTransform;

	//PrintGLMmat4(mat, "matScale * matTransform");

	for (int z = 0; z< m_iSceneVolumeDepth; z++)
	{
		for (int y = 0; y< m_iSceneVolumeHeight; y++)
		{
			for (int x = 0; x< m_iSceneVolumeWidth; x++)
			{
				AddCubeToScene(mat, vertdataarray);
				mat = mat * glm::translate(glm::vec3(m_fScaleSpacing, 0, 0));
			}
			mat = mat * glm::translate(glm::vec3(-((float)m_iSceneVolumeWidth) * m_fScaleSpacing, m_fScaleSpacing, 0));
		}
		mat = mat * glm::translate(glm::vec3(0, -((float)m_iSceneVolumeHeight) * m_fScaleSpacing, m_fScaleSpacing));
	}
	cubes.m_uiVertcount = vertdataarray.size() / 5;

	glGenVertexArrays(1, &cubes.m_unSceneVAO);
	glBindVertexArray(cubes.m_unSceneVAO);

	glGenBuffers(1, &cubes.m_glSceneVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubes.m_glSceneVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STATIC_DRAW);

	GLsizei stride = sizeof(glm::vec3) + sizeof(glm::vec2);
	uintptr_t offset = 0;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

	offset += sizeof(glm::vec3);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

}
void Init_RenderScene(void) {
	vive.controllerObj.Init("Controller");
	companionWnd.Init("CompanionWindow");

	SetupTextureMap();
	SetupCubeScene();


	PLYModel moca_model("textured_model.ply", 1, 1);
	point_cloud_scene.Init("PointCloud");
	point_cloud_scene.InitBuffer(moca_model.positions, moca_model.normals, moca_model.colors);
}



struct VertexDataWindow
{
	glm::vec2 position;
	glm::vec2 texCoord;

	VertexDataWindow(const glm::vec2 & pos, const glm::vec2 tex) : position(pos), texCoord(tex) {	}
};
void SetupDesktopWindow() {
	// vive check

	std::vector<VertexDataWindow> vVerts;

	// left eye verts
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, -1), glm::vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(1, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, 1), glm::vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(1, 1)));

	// right eye verts
	vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(1, -1), glm::vec2(1, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(1, 1), glm::vec2(1, 1)));

	GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6 };
	companionWnd.m_indiceCount = _countof(vIndices);

	glGenVertexArrays(1, &companionWnd.m_vao);
	glBindVertexArray(companionWnd.m_vao);

	glGenBuffers(1, &companionWnd.m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, companionWnd.m_vbo);
	glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &companionWnd.m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, companionWnd.m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, companionWnd.m_indiceCount * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, texCoord));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



void sample_vr_viewer(int argc, char **argv) {
	Init_OpenGL(argc, argv, "VR render");
	// shaders
	Init_GLshader();
	Init_RenderScene();
	if (vive.Init_HMD()) {
		vive.Init_HMDrenderModel();
		SetupDesktopWindow();
	}
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

	glutMainLoop();

	if (vive.m_HMD) {
		vr::VR_Shutdown();
	}
}
#endif // !_VR_RENDER_H


