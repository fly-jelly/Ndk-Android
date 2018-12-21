#pragma once
#pragma once
#include <string>
#include <vector>

#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

#define CHANNELS 3
#define WIDTH 112
#define HEIGHT 112

#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

struct FaceParams;

struct ThresholdRange
{
  // threshold for eye distance
  float eye_dist_l1 = 60;
  float eye_dist_l2 = 30;
  float eye_dist_l3 = 20;

  // threshold for yaw 
  float yaw_l1 = 15;
  float yaw_l2 = 30;
  float yaw_l3 = 45;
  
  // threshold for pitch
  float pitch_l1 = 15;
  float pitch_l2 = 20;
  float pitch_l3 = 30;

  // threshold for landmark
  float lmk_l1 = 3;
  float lmk_l2 = 10;
  float lmk_l3 = 20;

  // threshold for sharpness
  float sharpness_l1 = 0.65;
  float sharpness_l2 = 0.35;
  float sharpness_l3 = 0;

  // threshold for brightness
  float brightness_l1 = 0.25;
  float brightness_l2 = 0.5;
  float brightness_l3 = 0.7;
  float brightness_l4 = 0.9;

  // threshold for compression
  float compression_l1 = 0.35;
  float compression_l2 = 0.65;
  float compression_l3 = 0;
};

class FaceQualityPrivate
{
  private:
    std::string thres_confFile;

    bool initialized;

    int _errorID;

    ThresholdRange threshold_range;

    void loadConfig();
    
    void getQuality(FaceParams &faceParams);

    Mat showLandmarkPoints(Mat inputImage, std::vector<Point2f> salientLandmarks, cv::Scalar color);

    std::vector<Point2f> rotateCoordinates(std::vector<Point2f> salientLandmarks, Mat rotationMatrix, Point2f rotatedCenter, Point2f originalCenter);
    Mat getRotationMatrix(Point2f rotatedCenter, Point2f originalCenter, float angleDeg);

    float computeDCT(cv::Mat img, cv::Point2f poi);
    float findEyeDistance(std::vector<cv::Point2f> lmks);
    void validateBBox(cv::Rect &bbox, const cv::Size im_size);

    float checkBlurry(cv::Mat face);
    float checkBrightness(cv::Mat face);
    float checkCompression(const cv::Mat cv_img, cv::Rect bbox, std::vector<cv::Point2f> lmks);

    void destroy();

  public:
    bool use_3D_model;

    int loadModel(std::string thres_confFile);
    cv::Point3f estimatePose2D(std::vector<cv::Point2f> lmks, cv::Size im_size);

    FaceParams checkQuality(const cv::Mat cv_img, cv::Rect bbox, std::vector<cv::Point2f> lmks, bool saveResult=false, std::string out_im_path="");
    void releaseModel();
};
