/*************************************************************************
 *
 * DeepCam CONFIDENTIAL
 * FILE: dtracker.h
 *
 *  [2018] - [2019] DeepCam, LLC and DeepCam

NOTICE:
 * All information contained herein is, and remains the property of DeepCam LLC.
 * The intellectual and technical concepts contained herein are proprietary to DeepCam
 * and may be covered by U.S. and Foreign Patents,patents in process, and are protected by
 * trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * DeepCam, LLC.
 *
 *
 * Written: Delong Qi
 * Date: 17/12/2018
 * Mail: delong.qi@deepcam.com
 */


#ifndef __DTRACKER_H
#define __DTRACKER_H

#include <deque>
#include <vector>
#include "MTCNN.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <android/log.h>

struct TrackerBox
{
	int ID;
	int age;
	int quality;
	int confidence;
	int frameCount;
	cv::Rect tbox;
	std::vector<Point2f> lmks;
};


#define TAG "Mtcnn Demo "
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
static unsigned long get_cur_time(void) {
    struct timeval tv;
    unsigned long ts;
    gettimeofday(&tv, NULL);
    ts = tv.tv_sec * 1000000 + tv.tv_usec;
    return ts;
}

class DTracker
{

public:

	int init(char* face_model,int min ,int thread_num = 4, float detect_threshold = 0.6f);

	std::vector<TrackerBox> track(cv::Mat &image,float scale = 0.33);

	int destroy();

private:

	float IoU(const cv::Rect &boxA, const cv::Rect &boxB);
	
	int check_status();
	
	bool check_edge(cv::Mat &image, cv::Rect &bbox, float ratio = 6.0);

	int detect(cv::Mat &image,float scale);

private:
	
	MTCNN *mtcnn;
	
	int trackerAge;

	float scale = 1.0f;

	int num_threads = 3;
	
	unsigned int frameCount;

	unsigned int currFaceID;

	cv::Mat detect_img;

	int trackerBufferLen = 4;
	
	std::vector<TrackerBox> faceTrackers;
};


#endif
