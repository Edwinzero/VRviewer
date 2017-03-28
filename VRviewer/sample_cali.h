//
//  sample.h
//  Moc
//
//  Created by Wei Li on 6/4/15.
//

/*
#include "camera_calibration.h"
#include "vicon_tracker.h"
#include "freenect.h"
#include "fusion.h"

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
void sample_main_vicon_track( void ) {
	const char *ip = "192.168.10.1";
	VRPN_Tracker vrpn_pattern, vrpn_kinect;
	vrpn_pattern.Create( "PATTERN", ip, &vrpn_pattern, sample_callback );
	vrpn_kinect .Create( "KINECT0", ip, &vrpn_kinect , sample_callback );

	while( 1 ) {
		// update vrpn
		vrpn_pattern.Loop();
		vrpn_kinect.Loop();
	}
}

//=====================
// Test Kinect
//=====================
void sample_main_kinect( void ) {
	Freenect kinect;
	kinect.InitDevices();

	//=====================
	// Loop
	//=====================
	while( 1 ) {
		// update kinect
		kinect.Loop();
		cv::imshow( "ir", kinect.img_ir / 2000.0f );
		cv::waitKey(1);
	}
}

cv::Mat vrpn_data_to_mat(vrpn_TRACKERCB data) {
	return TranslateRotate(cv::Point3d(data.pos[0], data.pos[1], data.pos[2]), QuatToMat(data.quat[0], data.quat[1], data.quat[2], data.quat[3]));
}

static void VRPN_CALLBACK sample_cali_callback(void* user_data, const vrpn_TRACKERCB tData) {
	cv::Mat *mat = (cv::Mat*)user_data;

	*mat = vrpn_data_to_mat(tData);
}

void normlizeVec(double v[3]) {
	double len = sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );

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

void normalizeMatrix( cv::Mat &mat ) {
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
// Test Stereo Calibration
//=====================
void sample_main_calibrate_vicon_kinect_stereo(void) {
	//=====================
	// Vicon
	//=====================
	const char *ip = "192.168.10.1";
	VRPN_Tracker vrpn_pattern, vrpn_kinect;
	cv::Mat mat_pattern = cv::Mat_<double>(4, 4);
	cv::Mat mat_kinect = cv::Mat_<double>(4, 4);
	vrpn_pattern.Create("PATTERN", ip, &mat_pattern, sample_cali_callback);
	vrpn_kinect.Create("KINECT6", ip, &mat_kinect, sample_cali_callback);

	//=====================
	// Kinect
	//=====================
	Freenect kinect;
	kinect.InitDevices();

	//=====================
	// Pattern
	//=====================
	Moca::Pattern pattern2d, pattern3d;
	{
		// 1 --- 2 \
		// |     |  4
		// 0 --- 3 /
		// vsk unit : mm
		vector< cv::Point3f > markers(4);
		markers[0].x = -370.15447998046875;
		markers[0].y = -199.68521118164062;
		markers[0].z = -321.833740234375;
		markers[1].x = -427.5858154296875; 
		markers[1].y = 35.668434143066406; 
		markers[1].z = 258.58831787109375; 
		markers[2].x = 191.12762451171875; 
		markers[2].y = 146.83847045898437; 
		markers[2].z = 275.42535400390625; 
		markers[3].x = 248.92303466796875; 
		markers[3].y = -85.252586364746094;
		markers[3].z = -305.8043212890625; 

		//subtract the height of the marker
		cv::Point3f vec_1 = markers[0] - markers[1];
		cv::Point3f vec_2 = markers[2] - markers[1];
		cv::Point3f norm_pattern = vec_1.cross(vec_2) / (float)(cv::norm(vec_1.cross(vec_2))) * 8.0f;

		for (int i = 0; i < markers.size(); i++) {
			markers[i] = (markers[i] - norm_pattern) * 0.001f; // convert to meter
		}
		
		pattern2d.SetChessboard2D(cv::Size(7, 6), 0.09); // unit : meter
		pattern3d.SetChessboard3D(cv::Size(7, 6), markers);
	}

	//=====================
	// Calibration
	//=====================
	Moca::CameraParameter cam_rgb, cam_ir;
	Moca::Calibrator_Camera cali_rgb, cali_ir;
	Moca::Calibrator_Vicon cali_rgb_vicon; 
	Moca::Calibratior_Stereo cali_stereo;

	bool isCalibrated = false;

	// image corners
	vector< cv::Point2f > corners_ir, corners_rgb;

#if 0
	cam_ir.LoadCali("KINECT7_ir_cali.txt");
	cam_rgb.LoadCali("KINECT7_rgb_cali.txt");
	isCalibrated = true;
#endif

	//=====================
	// Loop
	//=====================
	while (1) {
		// update vrpn
		vrpn_pattern.Loop();
		vrpn_kinect.Loop();

		cv::Mat avg_mat_kinect = mat_kinect;
		cv::Mat avg_mat_pattern = mat_pattern;

		// update kinect
		kinect.Loop();
		
		if (isCalibrated) {
			cv::Mat undistortedImage;
			//cv::undistort(kinect.img_ir, undistortedImage, cali.intr.cameraMatrix, cali.intr.distCoef);
	        //undistortedImage = kinect.img_ir;
			undistortedImage = kinect.img_rgb;

			Moca::CameraIntrinsic intr;
			//intr.cameraMatrix = cam_ir.intr.cameraMatrix;
			intr.cameraMatrix = cam_rgb.intr.cameraMatrix;
			intr.distCoeffs = cv::Mat::zeros(8, 1, CV_64F);

			vector<cv::Point2f> proj;
			Moca::ProjectPoints(pattern3d.corners, proj, cam_rgb.intr, cam_rgb.extr * mat_kinect.inv() * mat_pattern);
			for (int i = 0; i < proj.size(); i++) {
				cv::Point pt(proj[i].x, proj[i].y);
				cv::Scalar color(0, 0, 255);
				cv::circle(undistortedImage, pt, 3, color, 1, CV_AA, 0);
			}
			//cv::imshow("ir", undistortedImage / 2000.0f);
			cv::imshow("rgb", undistortedImage);
		}
		else {
		    //cv::imshow("rgb", kinect.img_rgb);
			cv::imshow( "ir", kinect.img_ir / 2000.0f );
		}

		int key = cv::waitKey(1);

		// capture new frame
		if ( key == '1' ) {
			bool success_rgb = Moca::CaptureRGBFrame(kinect.img_rgb, pattern2d.boardSize, 5, corners_rgb);
			bool success_ir  = Moca::CaptureIRFrame (kinect.img_ir , pattern2d.boardSize, 4, corners_ir);

			cout << "capture rgb :" << endl;
			if (success_rgb) {
				Moca::DisplayCorners(kinect.img_rgb, pattern2d.boardSize, corners_rgb, "corner_rgb");
				cali_rgb.CaptureFrame(kinect.img_rgb.size(), corners_rgb);
				cali_rgb_vicon.CaptureFrame(avg_mat_pattern, avg_mat_kinect);
			}

			cout << "capture ir :" << endl; 
			if (success_ir) {
				cv::Mat img_ir = kinect.img_ir / 2000.0f;
				Moca::DisplayCorners(img_ir, pattern2d.boardSize, corners_ir, "corner_ir");
				cali_ir.CaptureFrame(kinect.img_ir.size(), corners_ir);
			}

			cout << "capture stereo :" << endl;
			if (success_rgb && success_ir) {
				cali_stereo.CaptureFrame(kinect.img_rgb.size(), kinect.img_ir.size(), corners_rgb, corners_ir);
			}
		}

		// calibrate kinect and save config
		if (key == '2') {
			if (cali_rgb.imgPoints.size() > 0 && cali_ir.imgPoints.size() > 0) {
				// intr : rgb
				cali_rgb.Calibrate(cam_rgb, pattern2d);
				
				// intr : ir
				cali_ir.Calibrate(cam_ir, pattern2d);

				// extr : vicon->rgb
				cali_rgb_vicon.Calibrate(cam_rgb, pattern3d, cali_rgb.imgPoints);

				// extr : rgb->ir
				cali_stereo.Calibrate(cam_rgb, cam_ir, pattern2d);

				cam_rgb.SaveCali((vrpn_kinect.m_id + "_rgb_cali.txt").c_str());
				cam_ir.SaveCali((vrpn_kinect.m_id + "_ir_cali.txt").c_str());
				isCalibrated = true;
			}
		}
	}
}

//=====================
// Test Stereo Calibration
//=====================
void sample_main_calibrate_kinect_stereo(void) {
	//=====================
	// Kinect
	//=====================
	Freenect kinect;
	kinect.InitDevices();

	//=====================
	// Pattern
	//=====================
	Moca::Pattern pattern2d;
	pattern2d.SetChessboard2D(cv::Size(7, 6), 0.09);

	//=====================
	// Calibration
	//=====================
	Moca::CameraParameter cam_rgb, cam_ir;
	Moca::Calibrator_Camera cali_rgb, cali_ir;
	Moca::Calibratior_Stereo cali_stereo;

	bool isCalibrated = false;

	// image corners
	vector< cv::Point2f > corners_ir, corners_rgb;

	//isCalibrated = true;

	cam_rgb.LoadCali("stereo_rgb_cali.txt");
	cam_ir.LoadCali("stereo_ir_cali.txt");
	
	cam_ir.intr.InitUndistortMap(cv::Size(512, 424)); 
	cam_rgb.intr.InitUndistortMap(cv::Size(1920, 1080));

	//=====================
	// Loop
	//=====================
	while (1) {
		// update kinect
		kinect.Loop();

		cv::imshow("ir", kinect.img_ir / 2000.0f);

		int key = cv::waitKey(1);

		// capture new frame
		if (key == '1') {
			bool success_rgb = Moca::CaptureRGBFrame(kinect.img_rgb, pattern2d.boardSize, 5, corners_rgb);
			bool success_ir  = Moca::CaptureIRFrame (kinect.img_ir , pattern2d.boardSize, 4, corners_ir);

			cout << "capture rgb :" << endl;
			if (success_rgb) {
				Moca::DisplayCorners(kinect.img_rgb, pattern2d.boardSize, corners_rgb, "corner_rgb");
				cali_rgb.CaptureFrame(kinect.img_rgb.size(), corners_rgb);
			}

			cout << "capture ir :" << endl;
			if (success_ir) {
				cv::Mat img_ir = kinect.img_ir / 2000.0f;
				Moca::DisplayCorners(img_ir, pattern2d.boardSize, corners_ir, "corner_ir");
				cali_ir.CaptureFrame(kinect.img_ir.size(), corners_ir);
			}

			cout << "capture stereo :" << endl;
			if (success_rgb && success_ir) {
				cali_stereo.CaptureFrame(kinect.img_rgb.size(), kinect.img_ir.size(), corners_rgb, corners_ir);
			}
		}

		// calibrate kinect and save config
		if (key == '2') {
			if (cali_rgb.imgPoints.size() > 0 && cali_ir.imgPoints.size() > 0) {
				// intr : rgb
				cali_rgb.Calibrate(cam_rgb, pattern2d);

				// intr : ir
				cali_ir.Calibrate(cam_ir, pattern2d);

				// extr : rgb->ir
				cali_stereo.Calibrate(cam_rgb, cam_ir, pattern2d);

				cam_rgb.SaveCali("stereo_rgb_cali.txt");
				cam_ir.SaveCali("stereo_ir_cali.txt");
				isCalibrated = true;
			}
		}

		if (key == '9') {
			if (Moca::CaptureRGBFrame(kinect.img_rgb, pattern2d.boardSize, 4, corners_rgb)) {

				cv::Mat extr;
				Moca::EstimateCameraPose(pattern2d.corners, corners_rgb, cam_rgb.intr, extr);

				Moca::ProjectPoints(pattern2d.corners, corners_ir, cam_ir.intr, cam_ir.extr * extr);

				Moca::DrawCircles(kinect.img_ir, corners_ir);
				cv::imshow("proj_u", kinect.img_ir / (255.0f*20.0f));
			}
		}
		
		if (key == '0') {
			cv::Mat depth;
			Moca::UndistortImage(kinect.img_depth, depth, cam_ir.intr, cv::INTER_LINEAR);

			cv::Mat ir;
			Moca::UndistortImage(kinect.img_ir, ir, cam_ir.intr, cv::INTER_LINEAR);

			cv::Mat rgb;
			Moca::UndistortImage(kinect.img_rgb, rgb, cam_rgb.intr, cv::INTER_LINEAR);

			//cv::imshow("irr", ir/(255.0f*20.0f));

			if (Moca::CaptureIRFrame(ir, pattern2d.boardSize, 4, corners_ir)) {
				corners_rgb.resize(corners_ir.size());

				vector<cv::Point3f> points(corners_ir.size());
				for (int i = 0; i < corners_ir.size(); i++) {
					float x = corners_ir[i].x;
					float y = corners_ir[i].y;
					float d = ((float*)depth.data)[(int)x + (int)y * depth.size().width] * 0.001f;

					x -= cam_ir.intr.cameraMatrix.at<double>(0, 2);
					y -= cam_ir.intr.cameraMatrix.at<double>(1, 2);
					x /= cam_ir.intr.cameraMatrix.at<double>(0, 0);
					y /= cam_ir.intr.cameraMatrix.at<double>(1, 1);

					points[i] = cam_ir.extr.inv() * cv::Point3d(x * d, y * d, d);
				}
				Moca::ProjectPoints(points, corners_rgb, cam_rgb.intr, cam_rgb.extr);
				Moca::DrawCircles(kinect.img_rgb, corners_rgb);
				cv::imshow("proj", kinect.img_rgb);

				for (int i = 0; i < corners_ir.size(); i++) {
					cv::Point3f v = points[i];
					
					v.x /= v.z;
					v.y /= v.z;

					v.x *= cam_rgb.intr.cameraMatrix.at<double>(0, 0);
					v.y *= cam_rgb.intr.cameraMatrix.at<double>(1, 1);
					v.x += cam_rgb.intr.cameraMatrix.at<double>(0, 2);
					v.y += cam_rgb.intr.cameraMatrix.at<double>(1, 2);

					corners_rgb[i].x = v.x;
					corners_rgb[i].y = v.y;
				}
				Moca::DrawCircles(kinect.img_rgb, corners_rgb);
				cv::imshow("proj_u", kinect.img_rgb);
			}
		}
	}
}
//*/
