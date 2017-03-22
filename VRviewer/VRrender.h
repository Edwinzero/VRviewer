#pragma once
#ifndef _VR_RENDER_H
#define _VR_RENDER_H
#include "3rdParty\lodepng.h"
#include "RenderUtils.h"
#include "GLobject.h"
#include "TestScene.h"
#include "HMD.h"
using namespace std;

unsigned int screenWidth = 800;
unsigned int screenHeight = 800;
bool reComplieShader = false;
GLuint GLsceneProgram = -1;
GLuint GLcontrollerTransformProgram = -1;
GLuint GLHMDdeviceRenderModelProgram = -1;
GLuint GLdesktopWindowProgram = -1;


typedef struct CubeSea {
	unsigned int m_uiVertcount;
	GLuint m_glSceneVertBuffer;
	GLuint m_unSceneVAO;
}CubeSea;

GLuint cube_tex = -1;
CubeSea cubes;
GLobject companionWnd;
GLobject controllerObj;
unsigned int controllerVertcount = 0;
std::vector<GLVRobject*> vrRenderModels;
GLVRobject *trackedDeviceToRenderModel[vr::k_unMaxTrackedDeviceCount];

GLuint sceneMatrixLocation = -1;			// gpu uniform matrix
GLuint controllerMatrixLocation = -1;
GLuint renderModelMatrixLocation = -1;

// ** HMD object to commu with vive
typedef struct HMD {
	vr::IVRSystem				*m_HMD;
	vr::IVRRenderModels			*m_RenderModels;
	vr::TrackedDevicePose_t		m_TrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	std::string					m_strDriver;
	std::string					m_strDisplay;
	glm::mat4					m_mat4DevicePose[ vr::k_unMaxTrackedDeviceCount ];		// get pose of each device
	bool						m_ShowTrackedDevice[ vr::k_unMaxTrackedDeviceCount ];	// flag for controlling display device
	
	float hmd_NearClip;
	float hmd_FarClip;

	glm::mat4 m_mat4HMDPose;
	glm::mat4 m_mat4eyePosLeft;
	glm::mat4 m_mat4eyePosRight;

	glm::mat4 m_mat4ProjectionCenter;
	glm::mat4 m_mat4ProjectionLeft;
	glm::mat4 m_mat4ProjectionRight;
}HMD;

// ** HMD info that GL render instruction (flags for rendering VIVE dev)
typedef struct HMDinfo {
	int m_TrackedControllerCount;
	int m_TrackedControllerCount_Last;
	int m_ValidPoseCount;
	int m_ValidPoseCount_Last;
	std::string m_strPoseClasses;
	char m_DevClassChar[vr::k_unMaxTrackedDeviceCount];
}HMDinfo;

typedef struct GLHMDdata {
	GLfbo leftEye;
	GLfbo rightEye;
	uint32_t m_RenderWidth;
	uint32_t m_RenderHeight;
}GLHMDdata;
//=========================================================================
//		HMD object
//=========================================================================
HMD vive;							// access all device data
HMDinfo viveInfo;					// access all device available status
GLHMDdata viveGLbuf;				// access vive render content


//=========================================================================
//		HMD draw methods
//=========================================================================
void RenderSceneToEye(vr::Hmd_Eye nEye) {

}
//=========================================================================
//		Draw methods
//=========================================================================
// hhmd render
void RenderControllerAxes() {
	// don't draw controllers if somebody else has input focus
	if (vive.m_HMD->IsInputFocusCapturedByAnotherProcess())
		return;

	std::vector<float> vertdataarray;
	controllerVertcount = 0;
	viveInfo.m_TrackedControllerCount = 0;
	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		if (!vive.m_HMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		if (vive.m_HMD->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
			continue;

		viveInfo.m_TrackedControllerCount += 1;

		if (!vive.m_TrackedDevicePose[unTrackedDevice].bPoseIsValid)
			continue;

		const glm::mat4 & mat = vive.m_mat4DevicePose[unTrackedDevice];

		glm::vec4 center = mat * glm::vec4(0, 0, 0, 1);

		for (int i = 0; i < 3; ++i)
		{
			glm::vec3 color(0, 0, 0);
			glm::vec4 point(0, 0, 0, 1);
			point[i] += 0.05f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = mat * point;
			vertdataarray.push_back(center.x);
			vertdataarray.push_back(center.y);
			vertdataarray.push_back(center.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);

			vertdataarray.push_back(point.x);
			vertdataarray.push_back(point.y);
			vertdataarray.push_back(point.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);

			controllerVertcount += 2;
		}

		glm::vec4 start = mat * glm::vec4(0, 0, -0.02f, 1);
		glm::vec4 end = mat * glm::vec4(0, 0, -39.f, 1);
		glm::vec3 color(.92f, .92f, .71f);

		vertdataarray.push_back(start.x); vertdataarray.push_back(start.y); vertdataarray.push_back(start.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

		vertdataarray.push_back(end.x); vertdataarray.push_back(end.y); vertdataarray.push_back(end.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);
		controllerVertcount += 2;
	}

	// Setup the VAO the first time through.
	if (controllerObj.m_vao == 0)
	{
		glGenVertexArrays(1, &controllerObj.m_vao);
		glBindVertexArray(controllerObj.m_vao);

		glGenBuffers(1, &controllerObj.m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, controllerObj.m_vbo);

		GLuint stride = 2 * 3 * sizeof(float);
		uintptr_t offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(glm::vec3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, controllerObj.m_vbo);

	// set vertex data if we have some
	if (vertdataarray.size() > 0)
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW);
	}
}
// hhmd render
void StreamEyeTexToHMD() {

}
// hhmd render
void DrawHMDScene() {
	glEnable(GL_MULTISAMPLE);
	// left eye

	// right eye
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
// hhmd
void UpdateHMDPose() {
	if (!vive.m_HMD) {
		return;
	}

	vr::VRCompositor()->WaitGetPoses(vive.m_TrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);
	viveInfo.m_ValidPoseCount = 0;
	viveInfo.m_strPoseClasses = "";
	for (int dev = 0; dev < vr::k_unMaxTrackedDeviceCount; dev++) {
		if (vive.m_TrackedDevicePose[dev].bPoseIsValid) {
			viveInfo.m_ValidPoseCount++;
			vive.m_mat4DevicePose[dev] = ConvertSteamVRMatrixToMat4(vive.m_TrackedDevicePose[dev].mDeviceToAbsoluteTracking);
			if (viveInfo.m_DevClassChar[dev] == 0) {
				switch (vive.m_HMD->GetTrackedDeviceClass(dev)) {
				case vr::TrackedDeviceClass_Controller:			viveInfo.m_DevClassChar[dev] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:				viveInfo.m_DevClassChar[dev] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:			viveInfo.m_DevClassChar[dev] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:		viveInfo.m_DevClassChar[dev] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference:	viveInfo.m_DevClassChar[dev] = 'T'; break;
				default:										viveInfo.m_DevClassChar[dev] = '?'; break;
				}
			}
			viveInfo.m_strPoseClasses += viveInfo.m_DevClassChar[dev];
		}
	}

	if (vive.m_TrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid) {
		vive.m_mat4HMDPose = vive.m_mat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
		vive.m_mat4HMDPose = glm::inverse(vive.m_mat4HMDPose);
	}

}

//=========================================================================
//		keyboard & mouse callback
//=========================================================================
// hhmd
void VRshutdown() {
	// shutdown hmd
	if (vive.m_HMD) {
		vr::VR_Shutdown();
		vive.m_HMD = NULL;
	}
	// clean vrrendermodel
	for (std::vector< GLVRobject * >::iterator i = vrRenderModels.begin(); i != vrRenderModels.end(); i++)
	{
		delete (*i);
	}
	vrRenderModels.clear();
	// hmd fbo
	CleanFBO(viveGLbuf.leftEye);
	CleanFBO(viveGLbuf.rightEye);
	
	// irrelevant with vive
	if (GLsceneProgram)
	{
		glDeleteProgram(GLsceneProgram);
	}
	if (GLcontrollerTransformProgram)
	{
		glDeleteProgram(GLcontrollerTransformProgram);
	}
	if (GLHMDdeviceRenderModelProgram)
	{
		glDeleteProgram(GLHMDdeviceRenderModelProgram);
	}
	if (GLdesktopWindowProgram)
	{
		glDeleteProgram(GLdesktopWindowProgram);
	}

	companionWnd.Cleanup();
	controllerObj.Cleanup();
	if (cubes.m_unSceneVAO != 0) {
		glDeleteVertexArrays(1, &cubes.m_unSceneVAO);
	}
}
bool keyboardEvent(unsigned char nChar, int nX, int nY)
{

	if (nChar == 27) { //Esc-key
		glutLeaveMainLoop();
		VRshutdown();
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
	GLcontrollerTransformProgram = CompileGLShader("ControllerTransform", "Shaders/Controller.vs", "Shaders/Controller.fs");
	GLHMDdeviceRenderModelProgram = CompileGLShader("RenderVIVEdevice", "Shaders/RenderVIVEdevice.vs", "Shaders/RenderVIVEdevice.fs");
	GLdesktopWindowProgram = CompileGLShader("DesktopWindow", "Shaders/DesktopWindow.vs", "Shaders/DesktopWindow.fs");
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

	int m_iSceneVolumeWidth = 20, m_iSceneVolumeHeight = 20, m_iSceneVolumeDepth = 20;
	glm::mat4 matTransform;
	matTransform = glm::translate( matTransform, glm::vec3(
		-((float)m_iSceneVolumeWidth * m_fScaleSpacing) / 2.f,
		-((float)m_iSceneVolumeHeight * m_fScaleSpacing) / 2.f,
		-((float)m_iSceneVolumeDepth * m_fScaleSpacing) / 2.f));

	glm::mat4 mat = matScale * matTransform;

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
	SetupTextureMap();
	SetupCubeScene();
}

// hhmd
glm::mat4 GetHMDMatrixProjectionEye(HMD &hmd, vr::Hmd_Eye eye) {
	if (!hmd.m_HMD)
		return glm::mat4();

	vr::HmdMatrix44_t mat = hmd.m_HMD->GetProjectionMatrix(eye, hmd.hmd_NearClip, hmd.hmd_FarClip);

	return glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}
// hhmd
glm::mat4 GetHMDMatrixPoseEye(HMD &hmd, vr::Hmd_Eye eye)
{
	if (!hmd.m_HMD)
		return glm::mat4();

	vr::HmdMatrix34_t matEyeRight = hmd.m_HMD->GetEyeToHeadTransform(eye);
	glm::mat4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
	);

	return glm::inverse(matrixObj);
}
// hmd
glm::mat4 GetCurrentViewProjectionMatrix(HMD &hmd, vr::Hmd_Eye eye)
{
	glm::mat4 matMVP;
	if (eye == vr::Eye_Left)
	{
		matMVP = hmd.m_mat4ProjectionLeft * hmd.m_mat4eyePosLeft * hmd.m_mat4HMDPose;
	}
	else if (eye == vr::Eye_Right)
	{
		matMVP = hmd.m_mat4ProjectionRight * hmd.m_mat4eyePosRight *  hmd.m_mat4HMDPose;
	}

	return matMVP;
}
// hhmd
void SetupCameras() {
	// not good, to be fixed
	vive.m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vive, vr::Eye_Left);
	vive.m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vive, vr::Eye_Right);
	vive.m_mat4eyePosLeft = GetHMDMatrixPoseEye(vive, vr::Eye_Left);
	vive.m_mat4eyePosRight = GetHMDMatrixPoseEye(vive, vr::Eye_Right);
}

// hhmd render
bool SetupStereoRenderTarget() {
	if (!vive.m_HMD) {
		return false;
	}
	vive.m_HMD->GetRecommendedRenderTargetSize(&viveGLbuf.m_RenderWidth, &viveGLbuf.m_RenderHeight);
	CreateFBO(viveGLbuf.leftEye, viveGLbuf.m_RenderWidth, viveGLbuf.m_RenderHeight);		// left eye fbo
	CreateFBO(viveGLbuf.rightEye, viveGLbuf.m_RenderWidth, viveGLbuf.m_RenderHeight);		// right eye fbo
	return true;
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
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, -1), glm::vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(1, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, 1), glm::vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(1, 0)));

	// right eye verts
	vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(1, -1), glm::vec2(1, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(1, 1), glm::vec2(1, 0)));

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

// hhmd helper
std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}
// hhmd render
// Finds a render model we've already loaded or loads a new one
GLVRobject *FindOrLoadRenderModel(const char *renderModelName) {
	GLVRobject *rendermodel = NULL;
	for (std::vector< GLVRobject * >::iterator i = vrRenderModels.begin(); i != vrRenderModels.end(); i++)
	{
		if (!stricmp((*i)->GetName().c_str(), renderModelName))
		{
			rendermodel = *i;
			break;
		}
	}

	// load the model if we didn't find one
	if (!rendermodel)
	{
		vr::RenderModel_t *pModel;
		vr::EVRRenderModelError error;
		while (1)
		{
			error = vr::VRRenderModels()->LoadRenderModel_Async(renderModelName, &pModel);
			if (error != vr::VRRenderModelError_Loading)
				break;

			ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			printf("Unable to load render model %s - %s\n", renderModelName, vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t *pTexture;
		while (1)
		{
			error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
			if (error != vr::VRRenderModelError_Loading)
				break;

			ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			printf("Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, renderModelName);
			vr::VRRenderModels()->FreeRenderModel(pModel);
			return NULL; // move on to the next tracked device
		}

		rendermodel = new GLVRobject(renderModelName);
		if (!rendermodel->InitBuffer(*pModel, *pTexture))
		{
			printf("Unable to create GL model from render model %s\n", renderModelName);
			delete rendermodel;
			rendermodel = NULL;
		}
		else
		{
			vrRenderModels.push_back(rendermodel);
		}
		vr::VRRenderModels()->FreeRenderModel(pModel);
		vr::VRRenderModels()->FreeTexture(pTexture);
	}
	return rendermodel;
}
// hhmd render
// Create/destroy GL a Render Model for a single tracked device
void SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t devID) {
	if (devID >= vr::k_unMaxTrackedDeviceCount) {
		return;
	}

	std::string renderModelName = GetTrackedDeviceString(vive.m_HMD, devID, vr::Prop_RenderModelName_String);
	GLVRobject *vrobj = FindOrLoadRenderModel(renderModelName.c_str());
	if (!vrobj) {
		std::string sTrackingSystemName = GetTrackedDeviceString(vive.m_HMD, devID, vr::Prop_TrackingSystemName_String);
		printf("Unable to load render model for tracked device %d (%s.%s)", devID, sTrackingSystemName.c_str(), renderModelName.c_str());
	}
	else
	{
		trackedDeviceToRenderModel[devID] = vrobj;
		vive.m_ShowTrackedDevice[devID] = true;
	}
}
// hhmd render
// Create/destroy GL Render Models
void SetupHMDdeviceRenderModels() {
	memset(trackedDeviceToRenderModel, 0, sizeof(trackedDeviceToRenderModel));

	if (!vive.m_HMD)
		return;

	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!vive.m_HMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		SetupRenderModelForTrackedDevice(unTrackedDevice);
	}
}
// hhmd
bool InitVRCompositor() {
	vr::EVRInitError peError = vr::VRInitError_None;

	if (!vr::VRCompositor())
	{
		printf("Compositor initialization failed. See log file for details\n");
		return false;
	}

	return true;
}
// hhmd
bool Init_HMD(void) {
	// HMD 
	vive.hmd_NearClip = 0.1f;
	vive.hmd_FarClip = 30.0f;
	
	// HMDinfo
	viveInfo.m_ValidPoseCount = 0;
	viveInfo.m_ValidPoseCount_Last = -1;
	viveInfo.m_TrackedControllerCount = 0;
	viveInfo.m_TrackedControllerCount_Last = -1;
	viveInfo.m_strPoseClasses = "";
	memset(viveInfo.m_DevClassChar, 0, sizeof(viveInfo.m_DevClassChar));

	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	vive.m_HMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
	if (eError != vr::VRInitError_None) {
		vive.m_HMD = NULL;
		printf("Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return false;
	}

	vive.m_RenderModels = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (!vive.m_RenderModels) {
		vive.m_HMD = NULL;
		vr::VR_Shutdown();
		printf("Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return false;
	}

	vive.m_strDriver = "No Driver";
	vive.m_strDisplay = "No Display";

	vive.m_strDriver = GetTrackedDeviceString(vive.m_HMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	vive.m_strDisplay = GetTrackedDeviceString(vive.m_HMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	if (!InitVRCompositor()) {
		return false;
	}
	return true;
}
// hhmd
void Init_HMDrenderModel() {
	SetupCameras();
	SetupStereoRenderTarget();
	SetupHMDdeviceRenderModels();
}

void sample_vr_viewer(int argc, char **argv) {
	GLVRobject obj("hehe");
	Init_OpenGL(argc, argv, "VR render");
	// shaders
	Init_GLshader();
	Init_RenderScene();
	Init_HMD();
	Init_HMDrenderModel();


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
}
#endif // !_VR_RENDER_H


