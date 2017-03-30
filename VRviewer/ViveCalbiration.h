#pragma once
#ifndef _VIVE_CALIBRATION_H
#define _VIVE_CALIBRATION_H
#include <iostream>
#include <vector>

#include "camera_calibration.h"
#include "vicon_tracker.h"
#include "VRrender.h"

static void VRPN_CALLBACK sample_callback(void* user_data, const vrpn_TRACKERCB tData) {
	VRPN_Tracker *tracker = (VRPN_Tracker*)user_data;

	printf("[Sensor %d : %s]: \n", tData.sensor, tracker->m_id.c_str());
	printf("time : %ld %ld\n", tData.msg_time.tv_sec, tData.msg_time.tv_usec);
	printf("t : %f %f %f\n", tData.pos[0], tData.pos[1], tData.pos[2]);
	printf("q : %f %f %f %f\n\n", tData.quat[0], tData.quat[1], tData.quat[2], tData.quat[3]);
}

cv::Mat vrpn_data_to_mat(vrpn_TRACKERCB data) {
	return TranslateRotate(cv::Point3d(data.pos[0], data.pos[1], data.pos[2]), QuatToMat(data.quat[0], data.quat[1], data.quat[2], data.quat[3]));
}

static void VRPN_CALLBACK sample_cali_callback(void* user_data, const vrpn_TRACKERCB tData) {
	cv::Mat *mat = (cv::Mat*)user_data;

	*mat = vrpn_data_to_mat(tData);
}


void normlizeVec(double v[3]) {
	double len = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (fabs(len) > 1e-6) {
		v[0] /= len;
		v[1] /= len;
		v[2] /= len;
	}
	else {
		v[0] = 0;
		v[1] = 0;
		v[2] = 0;
	}
}

void crossProduct(const double u[3], const double v[3], double w[3]) {
	w[0] = u[1] * v[2] - u[2] * v[1];
	w[1] = u[2] * v[0] - u[0] * v[2];
	w[2] = u[0] * v[1] - u[1] * v[0];
}

void normalizeMatrix(cv::Mat &mat) {
	double M[3][3];

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			M[i][j] = mat.at<double>(i, j);
		}
	}

	normlizeVec(M[2]);
	crossProduct(M[2], M[0], M[1]);
	normlizeVec(M[1]);
	crossProduct(M[1], M[2], M[0]);
	normlizeVec(M[0]);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			mat.at<double>(i, j) = M[i][j];
		}
	}

	mat.at<double>(3, 3) = 1.0;
}

//=====================
// Test Vicon
//=====================
void sample_main_vicon_track(void) {
	const char *ip = "192.168.10.1";
	VRPN_Tracker vrpn_viveController1, vrpn_viveController2;
	vrpn_viveController1.Create("VIVE_CONTROLLER1", ip, &vrpn_viveController1, sample_callback);
	vrpn_viveController2.Create("VIVE_CONTROLLER2", ip, &vrpn_viveController2, sample_callback);

	while (1) {
		// update vrpn
		vrpn_viveController1.Loop();
		vrpn_viveController2.Loop();
	}
}

void Sample_calibrate_vive_vicon() {
	// VICON
	const char *ip = "192.168.10.1";
	VRPN_Tracker vrpn_viveController1, vrpn_viveController2;
	cv::Mat pos_controller1 = cv::Mat_<double>(4, 4);
	cv::Mat pos_controller2 = cv::Mat_<double>(4, 4);
	vrpn_viveController1.Create("VIVE_CONTROLLER1", ip, &pos_controller1, sample_cali_callback);
	vrpn_viveController2.Create("VIVE_CONTROLLER2", ip, &pos_controller2, sample_cali_callback);
	cv::Point3f local_offset1 = cv::Point3f(0);
	cv::Point3f local_offset2 = cv::Point3f(0);

	// HMD
	HMD hmd;
	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	hmd.m_HMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
	if (eError != vr::VRInitError_None) {
		hmd.m_HMD = NULL;
		printf("Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return;
	}

	// Calibration parameter
	Moca::CameraParameter vive_param;

	// create window for listening key input
	cv::Mat img = cv::imread("cube_texture.png", CV_LOAD_IMAGE_COLOR);
	cv::imshow("Monitor", img);
	cv::waitKey(1);

	// key state
	int doCapture1 = 0, doCapture2 = 0;
	int doCalibrate1 = 0, doCalibrate2 = 0;
	vr::TrackedDeviceIndex_t leftControllerID = -1;
	vr::TrackedDeviceIndex_t rightControllerID = -1;
	// buffer
	std::vector<cv::Point3f> controller_points1, controller_points2;
	std::vector<cv::Point3f> vicon_controller_points1, vicon_controller_points2;
	// loop
	while (1) {
		//vrpn_viveController1.Loop();
		//vrpn_viveController2.Loop();

		// wait for key; 
		// https://github.com/sketchpunk/Unity3dExperiments/blob/master/Vive/Assets/ViveControllerMan.cs
		int key = cv::waitKey(1);
		if (key == 27) {
			// shutdown hmd
			if (hmd.m_HMD) {
				vr::VR_Shutdown();
				hmd.m_HMD = NULL;
			}
			exit(0);
		}



		// vr event
		//vr::VREvent_t event;
		//while (hmd.m_HMD->PollNextEvent(&event, sizeof(event)))
		//{
		//	switch (event.eventType) {
		//		// Controller
		//		case vr::VREvent_ButtonPress: // data is controller
		//		{
		//			printf("button press down\n");
		//		}
		//		break;
		//		case vr::VREvent_ButtonUnpress: // data is controller
		//		{
		//			printf("button press up\n");
		//		}
		//		break;
		//	}
		//}
		vr::VREvent_t event;
		while (hmd.m_HMD->PollNextEvent(&event, sizeof(event)))
		{
			switch (event.eventType) {
			case vr::VREvent_TrackedDeviceActivated:
			{
				leftControllerID = hmd.m_HMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
				rightControllerID = hmd.m_HMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
				printf("left id: %d\n", leftControllerID);
				printf("right id: %d\n", rightControllerID);
				printf("Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
			}
			break;
			case vr::VREvent_TrackedDeviceDeactivated:
			{
				printf("Device %u detached.\n", event.trackedDeviceIndex);
			}
			break;
				// Controller
			case vr::VREvent_ButtonPress: // data is controller
			{
				printf("button press down\n");
				// Process SteamVR controller state
				vr::TrackedPropertyError error;
				int32_t controllerRole;
				for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
				{
					controllerRole = hmd.m_HMD->GetInt32TrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32, &error);

					if (controllerRole == vr::ETrackedControllerRole::TrackedControllerRole_RightHand)
					{

						printf("[RIGHT CONTROLLER] button press down\n");


					}
					else if (controllerRole == vr::ETrackedControllerRole::TrackedControllerRole_LeftHand)
					{
						// left hand controller
						printf("[LEFT CONTROLLER] button press down\n");


					}

				}
			}
			break;
			case vr::VREvent_ButtonUnpress: // data is controller
			{
				printf("button press up\n");
			}
			break;
			}
		}

			
		

		// hmd process
		if (!hmd.m_HMD->IsInputFocusCapturedByAnotherProcess()){
			for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
			{
				if (!hmd.m_HMD->IsTrackedDeviceConnected(unTrackedDevice))
					continue;

				if (hmd.m_HMD->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
					continue;

				if (!hmd.m_TrackedDevicePose[unTrackedDevice].bPoseIsValid)
					continue;

				const glm::mat4 & mat = hmd.m_mat4DevicePose[unTrackedDevice];
				glm::vec4 center = mat * glm::vec4(0, 0, 0, 1); // center is the equal to marker in vicon space
				if (unTrackedDevice == leftControllerID) {

				}
				
			}
		}
		

		// vicon record
		cv::Point3f exact_pos_controller1 = pos_controller1 * local_offset1;
		cv::Point3f exact_pos_controller2 = pos_controller2 * local_offset2;

		// point to point reigstration
		if (doCalibrate1) {
			cv::Mat res1;
			Moca::PairPointsRigidRegistration(controller_points1, vicon_controller_points1, res1);
		}
		if (doCalibrate2) {
			cv::Mat res2;
			Moca::PairPointsRigidRegistration(controller_points2, vicon_controller_points2, res2);
		}
	}


	// shutdown hmd
	if (hmd.m_HMD) {
		vr::VR_Shutdown();
		hmd.m_HMD = NULL;
	}

}
#endif // !_VIVE_CALIBRATION_H