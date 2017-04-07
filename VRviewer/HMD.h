#pragma once
#ifndef _HMD_VIVE_H
#define _HMD_VIVE_H
#include <iostream>
#include <vector>
#include <openvr.h>

// ** HMD object to commu with vive
typedef struct HMD {
	vr::IVRSystem				*m_HMD;
	vr::IVRRenderModels			*m_RenderModels;
	vr::TrackedDevicePose_t		m_TrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	std::string					m_strDriver;
	std::string					m_strDisplay;
	glm::mat4					m_mat4DevicePose[vr::k_unMaxTrackedDeviceCount];		// get pose of each device
	bool						m_ShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];	// flag for controlling display device

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

typedef struct ControllerData {
	unsigned int m_index;
	vr::VRControllerState_t m_curState, m_prevState;

	ControllerData(unsigned int i) {
		m_index = i;
	}

	void Update(HMD *hmd) {
		m_prevState = m_curState;
		hmd->m_HMD->GetControllerState(m_index, &m_curState, sizeof(m_curState));
	}
	
}ControllerData;

class VIVE_RENDER {
public:
	// for display
	GLfbo m_leftEye;
	GLfbo m_rightEye;
	uint32_t m_RenderWidth;
	uint32_t m_RenderHeight;

	// for rendering device
	GLuint GLHMDdeviceRenderModelProgram;
	GLuint renderModelMatrixLocation;
	std::vector<GLVRobject*> vrRenderModels;								// m_vecRenderModels
	GLVRobject *trackedDeviceToRenderModel[vr::k_unMaxTrackedDeviceCount];  // m_rTrackedDeviceToRenderModel

	// controller line
	GLobject controllerObj;				// what is the purpose of this instance? For drawing controller axis
	GLuint controllerMatrixLocation;
public:
	VIVE_RENDER() {

	}

	void Init_RenderModel() {

	}

	void Init_ControllerAxis() {

	}

	// hhmd render
	// Finds a render model we've already loaded or loads a new one
	GLVRobject *FindOrLoadRenderModel(const char *renderModelName) {
		GLVRobject *rendermodel = NULL;
		for (std::vector< GLVRobject * >::iterator i = vrRenderModels.begin(); i != vrRenderModels.end(); i++)
		{
			if (!_stricmp((*i)->GetName().c_str(), renderModelName))
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


};


class VIVE_HMD {
public:
	vr::IVRSystem				*m_HMD;
	vr::IVRRenderModels			*m_RenderModels;
	vr::TrackedDevicePose_t		m_TrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	std::string					m_strDriver;
	std::string					m_strDisplay;
	glm::mat4					m_mat4DevicePose[vr::k_unMaxTrackedDeviceCount];		// get pose of each device
	bool						m_ShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];	// flag for controlling display device

	float hmd_NearClip;
	float hmd_FarClip;

	glm::mat4 m_mat4HMDPose;
	glm::mat4 m_mat4eyePosLeft;
	glm::mat4 m_mat4eyePosRight;

	glm::mat4 m_mat4ProjectionCenter;
	glm::mat4 m_mat4ProjectionLeft;
	glm::mat4 m_mat4ProjectionRight;


public:
	VIVE_HMD() {

	}

	// Pipeline
	bool Init_HMD(void) {
		// HMD 
		hmd_NearClip = 0.1f;
		hmd_FarClip = 30.0f;

		// Loading the SteamVR Runtime
		vr::EVRInitError eError = vr::VRInitError_None;
		m_HMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
		if (eError != vr::VRInitError_None) {
			m_HMD = NULL;
			printf("Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
			return false;
		}

		m_RenderModels = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
		if (!m_RenderModels) {
			m_HMD = NULL;
			vr::VR_Shutdown();
			printf("Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
			return false;
		}

		m_strDriver = "No Driver";
		m_strDisplay = "No Display";
		m_strDriver = GetTrackedDeviceString(m_HMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
		m_strDisplay = GetTrackedDeviceString(m_HMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
		printf("HMD driver: %s\n", m_strDriver.c_str());
		printf("HMD display: %s\n", m_strDisplay.c_str());

		if (!InitVRCompositor()) {
			return false;
		}
		printf("HMD init success! \n");
		return true;
	}

	// Utils
	glm::mat4 GetHMDMatrixProjectionEye( vr::Hmd_Eye eye ) {
		if (!m_HMD)
			return glm::mat4();

		vr::HmdMatrix44_t mat = m_HMD->GetProjectionMatrix(eye, hmd_NearClip, hmd_FarClip);

		return glm::mat4(
			mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
			mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
		);
	}
	// hhmd
	glm::mat4 GetHMDMatrixPoseEye( vr::Hmd_Eye eye )
	{
		if (!m_HMD)
			return glm::mat4();

		vr::HmdMatrix34_t matEyeRight = m_HMD->GetEyeToHeadTransform(eye);
		glm::mat4 matrixObj(
			matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
			matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
			matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
			matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

		return glm::inverse(matrixObj);
	}
	// hmd
	glm::mat4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye eye)
	{
		glm::mat4 matMVP;
		if (eye == vr::Eye_Left)
		{
			matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
		}
		else if (eye == vr::Eye_Right)
		{
			matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
		}

		return matMVP;
	}

	// hhmd render
	void RenderVRStereoTargets(GLuint &program, GLobject &scene) {
		glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
		//glClearColor(1.f, 0.f, 0.f, 1.0f);
		/*
		// left eye
		glEnable(GL_MULTISAMPLE);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_leftEye.m_framebuffer);
		glViewport(0, 0, m_fbo_RenderWidth, m_fbo_RenderHeight);
		//glClearColor(1.f, 0.f, 0.f, 1.0f);
		RenderSceneToEye(vr::Eye_Left, program, scene);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_MULTISAMPLE);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_leftEye.m_framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_leftEye.m_resolveFramebuffer);
		glBlitFramebuffer(0, 0, m_fbo_RenderWidth, m_fbo_RenderHeight, 0, 0, m_fbo_RenderWidth, m_fbo_RenderHeight,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


		// right eye
		glEnable(GL_MULTISAMPLE);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_rightEye.m_framebuffer);
		glViewport(0, 0, m_fbo_RenderWidth, m_fbo_RenderHeight);
		//glClearColor(0.f, 1.f, 0.f, 1.0f);
		RenderSceneToEye(vr::Eye_Right, program, scene);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_MULTISAMPLE);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo_rightEye.m_framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_rightEye.m_resolveFramebuffer);
		glBlitFramebuffer(0, 0, m_fbo_RenderWidth, m_fbo_RenderHeight, 0, 0, m_fbo_RenderWidth, m_fbo_RenderHeight,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		//*/
	}

private:
	// hhmd render
	void RenderSceneToEye(vr::Hmd_Eye eye, GLuint &scene_program, GLobject &scene) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		/*
		// draw scene for the eye
		{
			glUseProgram(scene_program);
			glUniformMatrix4fv(sceneMatrixLocation, 1, GL_FALSE, glm::value_ptr(GetCurrentViewProjectionMatrix(eye)));
			//PrintGLMmat4(GetCurrentViewProjectionMatrix(vive, eye));
			glBindVertexArray(scene.m_vao);
			glBindTexture(GL_TEXTURE_2D, scene.m_tex);
			glDrawArrays(GL_TRIANGLES, 0, scene.m_indiceCount);
			glBindVertexArray(0);

		}

		bool isInputCapturedByAnotherProcess = vive.m_HMD->IsInputFocusCapturedByAnotherProcess();
		if (!isInputCapturedByAnotherProcess) {
			// draw the controller axis lines
			glUseProgram(controller_program);
			glUniformMatrix4fv(controllerMatrixLocation, 1, GL_FALSE, glm::value_ptr(GetCurrentViewProjectionMatrix(eye)));
			glBindVertexArray(controllerObj.m_vao);
			glDrawArrays(GL_LINES, 0, controllerVertcount);  // change this controllerVercount to obj member var?
			glBindVertexArray(0);
		}

		// ----- Render Model rendering -----
		glUseProgram(GLHMDdeviceRenderModelProgram);
		for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
		{
			if (!trackedDeviceToRenderModel[unTrackedDevice] || !vive.m_ShowTrackedDevice[unTrackedDevice])
				continue;

			const vr::TrackedDevicePose_t & pose = vive.m_TrackedDevicePose[unTrackedDevice];
			if (!pose.bPoseIsValid)
				continue;

			if (isInputCapturedByAnotherProcess && vive.m_HMD->GetTrackedDeviceClass(unTrackedDevice) == vr::TrackedDeviceClass_Controller)
				continue;

			const glm::mat4 & matDeviceToTracking = vive.m_mat4DevicePose[unTrackedDevice];
			glm::mat4 matMVP = GetCurrentViewProjectionMatrix(eye) * matDeviceToTracking;
			glUniformMatrix4fv(renderModelMatrixLocation, 1, GL_FALSE, glm::value_ptr(matMVP));

			trackedDeviceToRenderModel[unTrackedDevice]->Draw();
		}
		glUseProgram(0);
		//*/
	}

	bool InitVRCompositor() {
		vr::EVRInitError peError = vr::VRInitError_None;

		if (!vr::VRCompositor())
		{
			printf("Compositor initialization failed. See log file for details\n");
			return false;
		}

		return true;
	}

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
};

class VIVE_HMDRENDER {
public:
	// info
	int m_TrackedControllerCount;
	int m_TrackedControllerCount_Last;
	int m_ValidPoseCount;
	int m_ValidPoseCount_Last;
	std::string m_strPoseClasses;
	char m_DevClassChar[vr::k_unMaxTrackedDeviceCount];

	// GL data
	GLfbo leftEye;
	GLfbo rightEye;
	uint32_t m_RenderWidth;
	uint32_t m_RenderHeight;

public:
	VIVE_HMDRENDER() {

	}
};

#endif // !_HMD_VIVE_H