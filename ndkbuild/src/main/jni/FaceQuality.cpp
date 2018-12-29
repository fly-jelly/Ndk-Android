#include "FaceQuality.hpp"
#include "FaceQuality_private.hpp"

FaceQuality::FaceQuality(bool use_3D_model)
{
    this->fqPrivate = new FaceQualityPrivate;
    this->fqPrivate->use_3D_model = use_3D_model;
}

FaceQuality::~FaceQuality()
{
}

int FaceQuality::loadModel(std::string thres_confFile)
{
    cout << "Loading Model ..." << endl;
    return this->fqPrivate->loadModel(thres_confFile);
}

///*
FaceParams FaceQuality::checkQuality(const cv::Mat cv_img, cv::Rect bbox, std::vector<cv::Point2f> lmks, bool saveResult, std::string out_im_path)
{
    return this->fqPrivate->checkQuality(cv_img, bbox, lmks, saveResult, out_im_path);
}
//*/


void FaceQuality::releaseModel()
{
    this->fqPrivate->releaseModel();
}

int FaceQualityPrivate::loadModel(std::string thres_confFile)
{
    // this->fd_modelFile = fd_modelFile;
    // this->fd_paramsFile = fd_paramsFile;
    // this->lm_modelFile = lm_modelFile;
    // this->lm_paramsFile = lm_paramsFile;
    // this->geo_modelFile = geo_modelFile;
    this->thres_confFile = thres_confFile;

    if (this->use_3D_model)
    {
        // removed
    }

    try
    {
        this->loadConfig();
    }
    catch (int e)
    {
        std::cout << "Initialization error occurred. Cannot read configure file. Exception No. " << e << std::endl;
        return 1;
    }

    return 0;
}

void FaceQualityPrivate::releaseModel()
{
}

void FaceQualityPrivate::loadConfig()
{
    ifstream f_in(this->thres_confFile);

    f_in >> threshold_range.eye_dist_l1 >> threshold_range.eye_dist_l2;// >> threshold_range.eye_dist_l3;
    f_in >> threshold_range.pitch_l1 >> threshold_range.pitch_l2; // >> threshold_range.pitch_l3;
    f_in >> threshold_range.yaw_l1 >> threshold_range.yaw_l2; // >> threshold_range.yaw_l3;
    f_in >> threshold_range.sharpness_l1 >> threshold_range.sharpness_l2;// >> threshold_range.sharpness_l3;
    f_in >> threshold_range.brightness_l1 >> threshold_range.brightness_l2 >> threshold_range.brightness_l3  >> threshold_range.brightness_l4;
    f_in >> threshold_range.compression_l1 >> threshold_range.compression_l2;

    f_in.close();
}


void FaceQualityPrivate::validateBBox(cv::Rect &bbox, const cv::Size im_size)
{
    if (bbox.x < 0)
        bbox.x = 0;
    if (bbox.y < 0)
        bbox.y = 0;
    if (bbox.x + bbox.width >= im_size.width)
        bbox.width = im_size.width - bbox.x - 1;
    if (bbox.y + bbox.height >= im_size.height)
        bbox.height = im_size.height - bbox.y - 1;
}

FaceParams FaceQualityPrivate::checkQuality(const cv::Mat cv_img, cv::Rect bbox, std::vector<cv::Point2f> lmks, bool saveResult, std::string out_im_path)
{
    FaceParams faceParams;
    // validate bbox
    validateBBox(bbox, cv_img.size());

    if (this->use_3D_model)
    {
        // remove        
    }
    else
    {
        cv::Point3f pose = this->estimatePose2D(lmks, cv_img.size());

        faceParams.rot_x = pose.x;
        faceParams.rot_y = pose.y;
        faceParams.rot_z = pose.z;
    }
	
	/*
    Mat colorImage_face = cv_img(bbox).clone();

    if (saveResult)
    {
        Mat annotatedImage;

        if (!use_3D_model)
            annotatedImage = showLandmarkPoints(cv_img, lmks, Scalar(0, 255, 0));

        imwrite(out_im_path, annotatedImage);
    }

    // compute eye distance
    if (!this->use_3D_model)
        faceParams.eye_dist = findEyeDistance(lmks);

    faceParams.face_size = CvSize(bbox.width, bbox.height);

    // compute face blurriness
    faceParams.sharpness = checkBlurry(colorImage_face);

    // compute face lighting
    faceParams.brightness = checkBrightness(colorImage_face);

    // compute face compression
    faceParams.compression = checkCompression(cv_img, bbox, lmks);

    getQuality(faceParams);
	*/
    return faceParams;
}

float FaceQualityPrivate::findEyeDistance(std::vector<cv::Point2f> lmks)
{
    cv::Point2f left_eye, right_eye;

    if (this->use_3D_model)
    {
        left_eye = (lmks[36] + lmks[37] + lmks[38] + lmks[39] + lmks[40] + lmks[41]) / 6;
        right_eye = (lmks[42] + lmks[43] + lmks[44] + lmks[45] + lmks[46] + lmks[47]) / 6;
    }
    else
    {
        left_eye = lmks[0];
        right_eye = lmks[1];
    }

    return cv::norm(left_eye - right_eye);
}

float FaceQualityPrivate::computeDCT(cv::Mat img, cv::Point2f poi)
{
    int min_nonzeros = 65;
    float min_value = 0.000001;

    cv::Mat roi_img, dct_img;
    int non_zeros, min_idx_x, min_idx_y;

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            cv::Rect rectROI(poi.x + x, poi.y + y, 8, 8);

            roi_img = img(rectROI);

            cv::dct(roi_img, dct_img);

            cv::threshold(dct_img, dct_img, min_value, 1.0, THRESH_TOZERO);

            non_zeros = cv::countNonZero(dct_img);

            if (non_zeros < min_nonzeros)
            {
                min_nonzeros = non_zeros;
                min_idx_x = poi.x + x;
                min_idx_y = poi.y + y;
            }
        }
    }

    int dist = 8;
    float num_zeros = 0;

    float count = 0;
    int h = img.rows;
    int w = img.cols;

    for (int x = -2; x < 3; x++)
    {
        for (int y = -2; y < 3; y++)
        {
            cv::Rect rectROI;

            if ((min_idx_y + y * dist > 0) && (min_idx_x + x * dist > 0) 
            && (min_idx_y + y * dist + dist < h) && (min_idx_x + x * dist + dist < w)) {

                rectROI = cvRect(min_idx_x + x * dist, min_idx_y + y * dist, 8, 8);

                roi_img = img(rectROI);

                cv::dct(roi_img, dct_img);

                cv::threshold(dct_img, dct_img, min_value, 1.0, THRESH_TOZERO);

                num_zeros += dist * dist - cv::countNonZero(dct_img);

                count += 1;
            }
        }
    }

    num_zeros = num_zeros / count;     

    float result = 1 - lgamma(float(num_zeros) / (dist * dist));
    result = 1 - lgamma(result);

    return std::min(1.0f, std::max(0.0f, result));
}

float FaceQualityPrivate::checkCompression(const cv::Mat cv_img, cv::Rect bbox, std::vector<cv::Point2f> lmks)
{
    cv::Mat gray, gray_norm;
    cv::Point2f leye, reye, nose, lmouth, rmouth;

    cv::cvtColor(cv_img, gray, CV_BGR2GRAY);
 
    gray.convertTo(gray_norm, CV_32F, 1.0/255);
    //cv::normalize(gray, gray_norm);

    if (this->use_3D_model)
    {
        leye = (lmks[36] + lmks[37] + lmks[38] + lmks[39] + lmks[40] + lmks[41]) / 6;
        reye = (lmks[42] + lmks[43] + lmks[44] + lmks[45] + lmks[46] + lmks[47]) / 6;

        nose = lmks[30];

        lmouth = lmks[48];
        rmouth = lmks[54];
    }
    else
    {
        leye = lmks[0];
        reye = lmks[1];
        nose = lmks[2];
        lmouth = lmks[3];
        rmouth = lmks[4];
    }

    float num_zeros_leye = computeDCT(gray_norm, leye);
    float num_zeros_reye = computeDCT(gray_norm, reye);
    float num_zeros_nose = computeDCT(gray_norm, nose);
    float num_zeros_tmouth = computeDCT(gray_norm, lmouth);
    float num_zeros_rmouth = computeDCT(gray_norm, rmouth);

    float mean_count = (num_zeros_leye + num_zeros_reye + num_zeros_nose + num_zeros_tmouth + num_zeros_rmouth) / 5;

    return mean_count;
}

float FaceQualityPrivate::checkBlurry(cv::Mat colorImage_face)
{
    int kernel_size = 3;
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;
    cv::Mat gradImage;
    cv::Scalar mean;
    cv::Scalar stddev;

    cv::Laplacian(colorImage_face, gradImage, CV_8UC3, kernel_size, scale, delta, BORDER_DEFAULT);
    meanStdDev(gradImage, mean, stddev);

    float result = cv::mean(stddev).val[0] / sqrt(4 * 255.0);

    return std::min(1.0f, std::max(0.0f, result));
}

float FaceQualityPrivate::checkBrightness(cv::Mat colorImage_face)
{
    cv::Scalar mean;
    cv::Scalar stddev;
    cv::Mat gray_base, hist_base;
    int channels[] = {0};
    int bins = 60;
    int histSize[] = {bins};
    float h_ranges[] = {0, 256};
    const float *ranges[] = {h_ranges};

    cvtColor(colorImage_face, gray_base, COLOR_BGR2GRAY);

    meanStdDev(gray_base, mean, stddev);

    return std::min(1.0f, std::max(0.0f, (float)(mean.val[0] / 255.0f)));
}

Mat FaceQualityPrivate::showLandmarkPoints(Mat inputImage, std::vector<Point2f> salientLandmarks, cv::Scalar color)
{
    //text on screen
    std::ostringstream outtext;

    Mat annotatedImage = inputImage.clone();
    for (int i = 0; i < salientLandmarks.size(); i++)
    {
        outtext.str("");
        outtext << i;

        circle(annotatedImage, salientLandmarks[i], 2, color, -1, 8); // left eye1
        cv::putText(annotatedImage, outtext.str(), salientLandmarks[i], cv::FONT_HERSHEY_SIMPLEX, 0.25, cv::Scalar(0, 0, 0));
    }
    return annotatedImage;
}

void FaceQualityPrivate::getQuality(FaceParams &faceParams)
{
    float eye_q, yaw_q, pitch_q, lmks_q, sharpness_q, brightness_q, compression_q;

    // check eye distance
    if (faceParams.eye_dist >= threshold_range.eye_dist_l1)
        eye_q = float(FaceQualityEnum::good) / 10;
    else if (faceParams.eye_dist >= threshold_range.eye_dist_l2)
        eye_q = float(FaceQualityEnum::average) / 10;
    else //if (faceParams.eye_dist >= threshold_range.eye_dist_l3)
        eye_q = float(FaceQualityEnum::bad) / 10;

    // check yaw
    if (abs(faceParams.rot_y) <= threshold_range.yaw_l1)
        yaw_q = float(FaceQualityEnum::good) / 10;
    else if (abs(faceParams.rot_y) <= threshold_range.yaw_l1)
        yaw_q = float(FaceQualityEnum::average) / 10;
    else //if (abs(faceParams.rot_y) <= threshold_range.yaw_l1)
        yaw_q = float(FaceQualityEnum::bad) / 10;
    //else
    //    yaw_q = float(FaceQualityEnum::bad) / 10;

    // check pitch
    if (abs(faceParams.rot_x) <= threshold_range.pitch_l1)
        pitch_q = float(FaceQualityEnum::good) / 10;
    else if (abs(faceParams.rot_x) <= threshold_range.pitch_l2)
        pitch_q = float(FaceQualityEnum::average) / 10;
    else //if (abs(faceParams.rot_x) <= threshold_range.pitch_l3)
        pitch_q = float(FaceQualityEnum::bad) / 10;
    //else
    //    pitch_q = float(FaceQualityEnum::bad) / 10;

    // check landmark quality
    if ((faceParams.lmk_dist) <= threshold_range.lmk_l1)
        lmks_q = float(FaceQualityEnum::good) / 10;
    else if ((faceParams.lmk_dist) <= threshold_range.lmk_l2)
        lmks_q = float(FaceQualityEnum::average) / 10;
    else //if (abs(faceParams.lmk_dist) <= threshold_range.lmk_l3)
        lmks_q = float(FaceQualityEnum::bad) / 10;

    // check sharpness quality
    if ((faceParams.sharpness) >= threshold_range.sharpness_l1)
        sharpness_q = float(FaceQualityEnum::good) / 10;
    else if ((faceParams.sharpness) >= threshold_range.sharpness_l2)
        sharpness_q = float(FaceQualityEnum::average) / 10;
    else //if (abs(faceParams.sharpness) <= threshold_range.sharpness_l3)
        sharpness_q = float(FaceQualityEnum::bad) / 10;

    // check lighting quality
    if ((faceParams.brightness) > threshold_range.brightness_l2 && faceParams.brightness <= threshold_range.brightness_l3 )
        brightness_q = float(FaceQualityEnum::good) / 10;
    else if (((faceParams.brightness) > threshold_range.brightness_l1 && (faceParams.brightness) <= threshold_range.brightness_l2) || ((faceParams.brightness) > threshold_range.brightness_l3 && (faceParams.brightness) <= threshold_range.brightness_l4))
        brightness_q = float(FaceQualityEnum::average) / 10;
    else if ((faceParams.brightness) <= threshold_range.brightness_l1 || (faceParams.brightness) > threshold_range.brightness_l4)
        brightness_q = float(FaceQualityEnum::bad) / 10;

    // check compression quality
    if (abs(faceParams.compression) <= threshold_range.compression_l1)
        compression_q = float(FaceQualityEnum::good) / 10;
    else if (abs(faceParams.compression) <= threshold_range.compression_l2)
        compression_q = float(FaceQualityEnum::average) / 10;
    else //if (abs(faceParams.sharpness) <= threshold_range.sharpness_l3)
        compression_q = float(FaceQualityEnum::bad) / 10;

    // update final quality
    faceParams.quality = min(sharpness_q, min(brightness_q, min(compression_q, min(eye_q, min(yaw_q, pitch_q)))));
}

std::vector<Point2f> FaceQualityPrivate::rotateCoordinates(std::vector<Point2f> salientLandmarks,
                                                           Mat rotationMatrix, Point2f rotatedCenter, Point2f originalCenter)
{
    std::vector<Point2f> rotatedLandmarks;
    for (int iCoord = 0; iCoord < salientLandmarks.size(); iCoord++)
    {
        Point2f currPoint = salientLandmarks.at(iCoord);
        Point2f centeredLandmark = currPoint - originalCenter;

        Point2f rotatedCenteredLandmark;
        rotatedCenteredLandmark.x = centeredLandmark.x * rotationMatrix.at<double>(0, 0) + centeredLandmark.y * rotationMatrix.at<double>(0, 1);
        rotatedCenteredLandmark.y = centeredLandmark.x * rotationMatrix.at<double>(1, 0) + centeredLandmark.y * rotationMatrix.at<double>(1, 1);
        Point2f rotatedLandmark = rotatedCenteredLandmark + rotatedCenter;

        rotatedLandmarks.push_back(rotatedLandmark);
    }
    return rotatedLandmarks;
}

Mat FaceQualityPrivate::getRotationMatrix(Point2f rotatedCenter, Point2f originalCenter, float angleDeg)
{
    Mat rotationMatrix = getRotationMatrix2D(originalCenter, angleDeg, 1.0);
    rotationMatrix.at<double>(0, 2) += rotatedCenter.x - originalCenter.x;
    rotationMatrix.at<double>(1, 2) += rotatedCenter.y - originalCenter.y;
    return rotationMatrix;
}

cv::Point3f FaceQualityPrivate::estimatePose2D(std::vector<cv::Point2f> lmks, cv::Size im_size)
{
    float pose_x;
    float pose_y;

    // compute roll
    Point2f leftEye;
    Point2f rightEye;

    leftEye = lmks.at(0);
    rightEye = lmks.at(1);

    Point2f eyeDifference = rightEye - leftEye;
    float angleRad = atan2(eyeDifference.y, eyeDifference.x);
    float angleDeg = (180 / CV_PI) * angleRad;
    pose_x = angleDeg;

    // rotate face to normalize roll
    Point2f originalCenter(im_size.width / 2.0f, im_size.height / 2.0f);
    Point2f rotatedCenter;
    Mat rotationMatrix = getRotationMatrix(originalCenter, originalCenter, angleDeg);
    std::vector<Point2f> rotatedLandmarks = rotateCoordinates(lmks, rotationMatrix, rotatedCenter, originalCenter);

    float alpha_1, alpha_2;

    alpha_1 = atan((lmks[2].x - lmks[0].x) / (lmks[2].y - lmks[0].y)) * 180 / M_PI;
    alpha_2 = atan((lmks[1].x - lmks[2].x) / (lmks[2].y - lmks[1].y)) * 180 / M_PI;

    // compute yaw
    pose_y = (alpha_1 - alpha_2);

    return cv::Point3f(pose_x, pose_y, 0);
}
