#pragma once
#include <string>
#include <vector>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

enum class FaceQualityEnum { good=9, average=5, bad=1 };

struct FaceParams
{
    // Rotations About Axis
    float rot_x; // degrees of rotation about x-axis
    float rot_y; // degrees of rotation about y-axis
    float rot_z; // degrees of rotation about y-axis

    float eye_dist;

    float lmk_dist;

    float sharpness;    // sharpness range [0, 1], 1 - sharpest, 0 - blurrest

    float brightness;   // brightness range [0, 1], 1 - brightest, 0 - darkest

    float compression;  // compression range [0, 1], 1 - more compressed, 0 - less compressed

    float quality;      // quality scale range [0, 1], 1 - best, 0 - worst

    cv::Size face_size;

    // Constructor
    FaceParams() : rot_x(0.),
                   rot_y(0.),
                   rot_z(0.),
                   eye_dist(60.)
    {
        // Initializations
    }
};

class FaceQualityPrivate;

class FaceQuality
{
  private:
    FaceQualityPrivate *fqPrivate;

  public:
    FaceQuality(bool use_3D_model=false);
    ~FaceQuality();
    int loadModel(std::string thres_confFile);
    FaceParams checkQuality(const cv::Mat cv_img, cv::Rect bbox, std::vector<cv::Point2f> lmks, bool saveResult=false, std::string out_im_path="");

    void releaseModel();
};