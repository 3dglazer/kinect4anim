/*
 * This file is part of the OpenKinect Project. http://www.openkinect.org
 *
 * Copyright (c) 2010 individual OpenKinect contributors. See the CONTRIB file
 * for details.
 *
 * This code is licensed to you under the terms of the Apache License, version
 * 2.0, or, at your option, the terms of the GNU General Public License,
 * version 2.0. See the APACHE20 and GPL2 files for the text of the licenses,
 * or the following URLs:
 * http://www.apache.org/licenses/LICENSE-2.0
 * http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * If you redistribute this file in source form, modified or unmodified, you
 * may:
 *   1) Leave this header intact and distribute it under the same terms,
 *      accompanying it with the APACHE20 and GPL20 files, or
 *   2) Delete the Apache 2.0 clause and accompany it with the GPL2 file, or
 *   3) Delete the GPL v2 clause and accompany it with the APACHE20 file
 * In all cases you must keep the copyright notice intact and include a copy
 * of the CONTRIB file.
 *
 * Binary distributions must follow the binary distribution requirements of
 * either License.
 */

//open kinect
#include "libfreenect.hpp"
#include "libfreenect.h"
#include "libfreenect_sync.h"
#include "libfreenect-registration.h"
#include <pthread.h>



//Thread support
#include "thread.h"
#include "Queue.h"

//c++
#include <stdio.h>
#include <string>
#include <iostream>
#include <cmath>
#include <vector>

//the main stuff
#include "mtx.h"
#include "RGBWriter.h"
#include "DepthWriter.h"
#include "MyFreenectDevice.h"

// namespaces
using namespace std;

//help constants
#define MAXQUEUESIZE 2000 // two thousand images in the queue at maxium, which is approximatelly one minute 
#define GRAYMAP 255.0/2048.0
#define UINT8NORMALIZE 1.0/255.0
//recording mode
#define RECORD_RGB_DEPTH 0  //default mode both color and depth images will be recorded
#define	RECORD_DEPTH 1
#define RECORD_RGB 2

const char sysSeparator =
#ifdef _WIN32
'\\';
#else
'/';
#endif

// GLUT stuff
#if defined(__APPLE__)
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#define SPACEBAR 32

#if !defined(_WIN32) && !defined(_WIN64) // Linux - Unix
#  include <sys/time.h>
typedef timeval sys_time_t;
inline void system_time(sys_time_t* t) {
	gettimeofday(t, NULL);
}
inline long long time_to_msec(const sys_time_t& t) {
	return t.tv_sec * 1000LL + t.tv_usec / 1000;
}
#else // Windows and MinGW
#  include <sys/timeb.h>
typedef _timeb sys_time_t;
inline void system_time(sys_time_t* t) { _ftime(t); }
inline long long time_to_msec(const sys_time_t& t) {
	return t.time * 1000LL + t.millitm;
}
#endif

Rgba* createImage(int width, int height){
	Rgba* dt=new Rgba[width*height];
	for (int i=0; i<width*height; ++i) {
		float val=(float)(i%width)/(float)width;
		dt[i].r=val;
		dt[i].g=val;
		dt[i].b=val;
		dt[i].a=1.0;
	}
	return dt;
}

Freenect::Freenect freenect;
freenect_video_format requested_format(FREENECT_VIDEO_RGB);
//Synchronized queue
CQueue<RGBData*>  queue;
CQueue<DepthData*> queueDepth;
string filePath="/kinectTests/";
sys_time_t t;
long long currentTimeMs,startTime;
MyFreenectDevice* device;
RGBWriter* thread1; //rgb image writer
DepthWriter* thread2; //depth image writer
// =============== START of GL Magic ==================
GLuint gl_depth_tex;
GLuint gl_rgb_tex;

double freenect_angle(0);
int got_frames(0),window(0);
int g_argc;
char **g_argv;
int frameCounter=0;

bool record=false;
bool normalizeDepth=false;
int recordMode=0;

//won't work right now conversion to uint needed!
void drawDepthImage(std::vector<half> &depth){
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, &depth[0]);
	
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(255.0f, 255.0f, 255.0f, 255.0f);
	glTexCoord2f(0, 0); glVertex3f(0,0,0);
	glTexCoord2f(1, 0); glVertex3f(640,0,0);
	glTexCoord2f(1, 1); glVertex3f(640,480,0);
	glTexCoord2f(0, 1); glVertex3f(0,480,0);
	glEnd();
}

void drawVideo(std::vector<uint8_t> &rgb){
	
	glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
	if (device->getVideoFormat() == FREENECT_VIDEO_RGB || device->getVideoFormat() == FREENECT_VIDEO_YUV_RGB)
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, &rgb[0]);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, 1, 640, 480, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &rgb[0]);
	
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(255.0f, 255.0f, 255.0f, 255.0f);
	glTexCoord2f(0, 0); glVertex3f(0,0,0);
	glTexCoord2f(1, 0); glVertex3f(640,0,0);
	glTexCoord2f(1, 1); glVertex3f(640,480,0);
	glTexCoord2f(0, 1); glVertex3f(0,480,0);
	glEnd();
	
}

void DrawGLScene()
{
	static std::vector<uint8_t> rgb(640*480*3);
	static std::vector<half> depth(640*480);
	bool redrawVideo=device->getRGB8bit(rgb);
	bool redrawDepth=device->getDepth16bit(depth);

	// using getTiltDegs() in a closed loop is unstable
	/*if(device->getState().m_code == TILT_STATUS_STOPPED){
	 freenect_angle = device->getState().getTiltDegs();
	 }*/
	//		printf("\r demanded tilt angle: %+4.2f device tilt angle: %+4.2f", freenect_angle, device->getState().getTiltDegs());
	fflush(stdout);
	got_frames = 0;
	//musim to nutne rozdelit na draw rgb a draw depth!!!! s tim ze bych mel asi rgb ukladat do png!
	if (redrawVideo) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		
		glEnable(GL_TEXTURE_2D);
		if (record&&(recordMode==RECORD_RGB||recordMode==RECORD_RGB_DEPTH)) {
			system_time(&t); //get the current time
			currentTimeMs = time_to_msec(t)-startTime; 
			//have to copy all the local data and put it into queue
			uint8_t* rgbData = new uint8_t[rgb.size()];
			//half* depthData= new half[depth.size()];
			std::copy(&rgb[0], &rgb[rgb.size()], rgbData);
			//std::copy(&depth[0], &depth[depth.size()], depthData);
			RGBData* item=new RGBData(rgbData,currentTimeMs,640, 480);
			queue.PushElement(item);
		}
		drawVideo(rgb);
		
	}
	if (redrawDepth) {
		if (record&&(recordMode==RECORD_DEPTH||recordMode==RECORD_RGB_DEPTH)) {
			system_time(&t); //get the current time
			currentTimeMs = time_to_msec(t)-startTime; 
			//have to copy all the local data and put it into queue
			half* depthData= new half[depth.size()];
			std::copy(&depth[0], &depth[depth.size()], depthData);
			DepthData* item=new DepthData(depthData,currentTimeMs);
			queueDepth.PushElement(item);
		}
	}
//	
	//drawDepthImage();	
	//drawOldGL();
	if (redrawVideo) {
		glutSwapBuffers();
	}
	
}

void keyPressed(unsigned char key, int x, int y)
{
	if (key == 27) {
		glutDestroyWindow(window);
		device->stopDepth();
		device->stopVideo();
		int i=0;
		// Wait for the queue to be empty
		while (queue.size() > 0||queueDepth.size()>0)
		{
			sleep(1); // we won't stress the thread without a reason
			printf("\nImages remaining to write %d!!!\n",queue.size()+queueDepth.size());
		};
		printf("done %d\n",i);
		sleep(2); // wait for the last thread work
		// Not pthread_exit because OSX leaves a thread lying around and doesn't exit
		exit(0);
	}
	if (key == '1') {
		device->setLed(LED_GREEN);
	}
	if (key == '2') {
		device->setLed(LED_RED);
	}
	if (key == '3') {
		device->setLed(LED_YELLOW);
	}
	if (key == '4') {
		device->setLed(LED_BLINK_GREEN);
	}
	if (key == '5') {
		// 5 is the same as 4
		device->setLed(LED_BLINK_GREEN);
	}
	if (key == '6') {
		device->setLed(LED_BLINK_RED_YELLOW);
	}
	if (key == '0') {
		device->setLed(LED_OFF);
	}
	//for recording
	if (key==SPACEBAR) {
		if (record) {
			record=false;
		}else {
			record=true;
			system_time(&t); //get the current time
			startTime = time_to_msec(t);
			
		}
	}
	if (key=='m') {
		recordMode++;
		recordMode=recordMode%3;
		switch (recordMode) {
			case RECORD_RGB:
				printf("\n recording only RGB data.\n");
				break;
			case RECORD_DEPTH:
				printf("\n recording only DEPTH data.\n");
				break;
			case RECORD_RGB_DEPTH:
				printf("\n recording both RGB and DEPTH data.\n");
				break;
			default:
				break;
		}
	}
	if (key == 'f') {
		if (requested_format == FREENECT_VIDEO_IR_8BIT){
			requested_format = FREENECT_VIDEO_RGB;
			printf("\nVideo format is FREENECT_VIDEO_RGB\n");
		}else if (requested_format == FREENECT_VIDEO_RGB){
			requested_format = FREENECT_VIDEO_YUV_RGB;
			printf("\nVideo format is FREENECT_VIDEO_YUV_RGB\n");
		}else{
			requested_format = FREENECT_VIDEO_IR_8BIT;
			printf("\nVideo format is FREENECT_VIDEO_IR_8BIT\n");
		}
		device->setVideoFormat(requested_format, FREENECT_RESOLUTION_MEDIUM);
	}
	if (key=='n') {
		if (normalizeDepth) {
			printf("Depth data will be outputed as distance in millimeters (usefull for pointcloud generation);\n");
			normalizeDepth=false;
			device->depthInMM();
		}else {
			printf("Depth data will be converted to range <0;1.0> (usefull for dipslacement mapping);\n");
			normalizeDepth=true;
			device->depthNormalized();
			
		}
	}
	if (key == 'w') {
		freenect_angle++;
		if (freenect_angle > 30) {
			freenect_angle = 30;
		}
	}
	if (key == 's' || key == 'd') {
		freenect_angle = 0;
	}
	if (key == 'x') {
		freenect_angle--;
		if (freenect_angle < -30) {
			freenect_angle = -30;
		}
	}
	if (key == 'e') {
		freenect_angle = 10;
	}
	if (key == 'c') {
		freenect_angle = -10;
	}
	device->setTiltDegrees(freenect_angle);
}

void ReSizeGLScene(int Width, int Height)
{
	glViewport(0,0,Width,Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho (0, Width, Height, 0, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
}

void InitGL(int Width, int Height)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);
	glGenTextures(1, &gl_depth_tex);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenTextures(1, &gl_rgb_tex);
	glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	ReSizeGLScene(Width, Height);
}

void *gl_threadfunc(void *arg)
{
	printf("GL thread\n");
	
	glutInit(&g_argc, g_argv);
	
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(0, 0);
	
	window = glutCreateWindow("LibFreenect");
	printf("Window is %d",window);
	glutDisplayFunc(&DrawGLScene);
	glutIdleFunc(&DrawGLScene);
	glutReshapeFunc(&ReSizeGLScene);
	glutKeyboardFunc(&keyPressed);
	
	InitGL(640, 480);
	
	glutMainLoop();
	
	return NULL;
}

// =============== END of GL Magic ==================

//enum Compression
//{
//    NO_COMPRESSION  = 0,	// no compression
//	
//    RLE_COMPRESSION = 1,	// run length encoding
//	
//    ZIPS_COMPRESSION = 2,	// zlib compression, one scan line at a time
//	
//    ZIP_COMPRESSION = 3,	// zlib compression, in blocks of 16 scan lines
//	
//    PIZ_COMPRESSION = 4,	// piz-based wavelet compression
//	
//    PXR24_COMPRESSION = 5,	// lossy 24-bit float compression
//	
//    B44_COMPRESSION = 6,	// lossy 4-by-4 pixel block compression,
//	// fixed compression rate
//	
//    B44A_COMPRESSION = 7,	// lossy 4-by-4 pixel block compression,
//	// flat fields are compressed more
//	
//    NUM_COMPRESSION_METHODS	// number of different compression methods
//};
/*
void calibrateRGBToDepth(uint8_t* rgb_raw, uint8_t* rgb_registered){
    
	freenect_registration* registration = new freenect_registration();
	*(freenect_registration*)registration = freenect_copy_registration(device);
    
    
    freenect_registration* reg = (freenect_registration*)registration;
    uint32_t target_offset = reg->reg_pad_info.start_lines * 480;
	int x,y;
    memset(rgb_registered, 0, 680*480*3);
	for (y = 0; y < 480; y++) for (x = 0; x < 680; x++) {
        
		uint32_t index = y * 680 + x;
		uint32_t cx,cy,cindex;
        
		int wz = depthPixelsRaw[index];
		//if (wz == 0) continue;
        
		// coordinates in rgb image corresponding to x,y
		cx = (reg->registration_table[index][0] + reg->depth_to_rgb_shift[wz]) / 256;
		cy =  reg->registration_table[index][1];
        
		if (cx >= 680) continue;
        
		cindex = (cy * 680 + cx - target_offset) * 3;
		index  = index*3;
        
		rgb_registered[index+0] = rgb_raw[cindex+0];
		rgb_registered[index+1] = rgb_raw[cindex+1];
		rgb_registered[index+2] = rgb_raw[cindex+2];
	}
}
*/

int main(int argc, char **argv) {
	//=====g
	staticInitialize();
	system_time(&t); //get the current time
	startTime = time_to_msec(t); 
	//====== end of time 
	device = &freenect.createDevice<MyFreenectDevice>(0);
	device->startVideo();
	device->startDepth();
	device->setVideoFormat(requested_format, FREENECT_RESOLUTION_MEDIUM);
	device->depthInMM(); //depth data will be in mm for default
	//start image writing thread
	int jpegqual=80;
	// variables image_height and image width come from writejpg.h these are lib turbo jpeg specific
	image_width=640;
	image_height=480;
	thread1 = new RGBWriter(queue,filePath,image_width,image_height,jpegqual);
	thread2=new DepthWriter(queueDepth,filePath,image_width,image_height,Compression(3));
	thread1->start();
	thread2->start();
	gl_threadfunc(NULL);
	delete device;
	return 0;
}


