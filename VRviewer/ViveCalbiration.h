#pragma once
#ifndef _VIVE_CALIBRATION_H
#define _VIVE_CALIBRATION_H
#include <iostream>
#include <vector>

#include "camera_calibration.h"
#include "vicon_tracker.h"

static void VRPN_CALLBACK sample_callback(void* user_data, const vrpn_TRACKERCB tData) {
	VRPN_Tracker *tracker = (VRPN_Tracker*)user_data;

	printf("[Sensor %d : %s]: \n", tData.sensor, tracker->m_id.c_str());
	printf("time : %ld %ld\n", tData.msg_time.tv_sec, tData.msg_time.tv_usec);
	printf("t : %f %f %f\n", tData.pos[0], tData.pos[1], tData.pos[2]);
	printf("q : %f %f %f %f\n\n", tData.quat[0], tData.quat[1], tData.quat[2], tData.quat[3]);
}

//=====================
// Test Vicon
//=====================
void sample_main_vicon_track(void) {
	const char *ip = "192.168.10.1";
	VRPN_Tracker vrpn_pattern, vrpn_kinect;
	vrpn_pattern.Create("PATTERN", ip, &vrpn_pattern, sample_callback);
	vrpn_kinect.Create("KINECT0", ip, &vrpn_kinect, sample_callback);

	while (1) {
		// update vrpn
		vrpn_pattern.Loop();
		vrpn_kinect.Loop();
	}
}

void Sample_calibrate_vive_vicon() {

}
#endif // !_VIVE_CALIBRATION_H