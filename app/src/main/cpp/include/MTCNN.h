//
// Created by Lonqi on 2017/11/18.
//
#pragma once

#ifndef __MTCNN_NCNN_H__
#define __MTCNN_NCNN_H__

#include "net.h"
#include <string>
#include <vector>
#include <time.h>
#include <algorithm>
#include <map>
#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <opencv2/opencv.hpp>

#include "FaceQuality.hpp"
#include "FaceQuality_private.hpp"

using namespace std;

struct Bbox
{
	float score;
	int x1;
	int y1;
	int x2;
	int y2;
	float area;
	float ppoint[10];
	float regreCoord[4];
};

struct FaceBox
{
	float rot_x;
	float rot_y;
	float rot_z;
	float score;
	cv::Rect bbox;
	std::vector<cv::Point2f> lmk;
};	

class MTCNN 
{

public:
	MTCNN(const string &model_path);
	MTCNN(const std::vector<std::string>& param_files, const std::vector<std::string>& bin_files);
	~MTCNN();

	void SetMinFace(int minSize);
	void SetNumThreads(int numThreads);
	void SetTimeCount(int timeCount);
	
	void detect(cv::Mat& img, /*int minsize,*/ float scale, std::vector<FaceBox>& faceBoxs/*, int num_threads = 4*/);
	void detectMaxFace(cv::Mat& img, int minsize, float scale, std::vector<FaceBox>& faceBoxs, int num_threads = 4);

private:
	void detect(cv::Mat& img_, std::vector<Bbox>& finalBbox);
	void detectMaxFace(cv::Mat& img_, std::vector<Bbox>& finalBbox);
	void generateBbox(ncnn::Mat score, ncnn::Mat location, vector<Bbox>& boundingBox_, float scale);
	void nmsTwoBoxs(vector<Bbox> &boundingBox_, vector<Bbox> &previousBox_, const float overlap_threshold, string modelname = "Union");
	void nms(vector<Bbox> &boundingBox_, const float overlap_threshold, string modelname="Union");
	void refine(vector<Bbox> &vecBbox, const int &height, const int &width, bool square);
	void extractMaxFace(vector<Bbox> &boundingBox_);

	void PNet(float scale);
	void PNet();
	void RNet();
	void ONet();
	ncnn::Net Pnet, Rnet, Onet;
	ncnn::Mat img;
	const float nms_threshold[3] = {0.5f, 0.7f, 0.7f};

	const float mean_vals[3] = {127.5, 127.5, 127.5};
	const float norm_vals[3] = {0.0078125, 0.0078125, 0.0078125};
	const int MIN_DET_SIZE = 12;
	std::vector<Bbox> firstBbox_, secondBbox_,thirdBbox_;
	std::vector<Bbox> firstPreviousBbox_, secondPreviousBbox_, thirdPrevioussBbox_;
	int img_w, img_h;

private:
	const float threshold[3] = { 0.8f, 0.8f, 0.6f };
	int minsize = 40;
	const float pre_facetor = 0.309f;
	
	FaceQuality* faceQual;

	cv::Mat re_img;
	float scale = 1.0;
	int count = 1;
	int num_threads = 4;

};


#endif //__MTCNN_NCNN_H__
