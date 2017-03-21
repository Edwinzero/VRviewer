#include "VRrender.h"

int main(int argc, char **argv) {
#if 1
	sample_vr_viewer(argc, argv);
#else
	Sample_calibrate_vive_vicon();
#endif
	return 0;
}