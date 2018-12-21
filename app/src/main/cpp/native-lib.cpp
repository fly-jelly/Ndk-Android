#include <jni.h>
#include <string>
#include <MTCNN.h>
#include <android/log.h>
#include <opencv2/opencv.hpp>

#define TAG "Mtcnn Demo "
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
static unsigned long get_cur_time(void)
{
    struct timeval tv;
    unsigned long ts;
    gettimeofday(&tv,NULL);
    ts=tv.tv_sec*1000000+tv.tv_usec;
    return ts;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_deepcam_mtcnn_MainActivity_init(JNIEnv *env, jobject instance) {

    // TODO
    int minsize = 40;
    float scale = 0.2;
    std::string model_path = "/sdcard/mtcnn/models";
    LOGD("init");
    MTCNN *mtcnn = new MTCNN(model_path);
    for (int i = 0; i < 6; ++i) {
//    cv::Mat frame =  cv::imread("/sdcard/mtcnn/pic/2.jpg");
//    cv::Mat frame =  cv::imread("/sdcard/DCIM/Camera/IMG_20181219_155824.jpg");
    cv::Mat frame =  cv::imread("/sdcard/mtcnn/pic/IMG_20181219_155824.jpg");
    if(frame.empty())
    {
        std::cout << "load frame "  << " failed.\n";
        return -1;
    }
//    std::cout << "image size : " << frame.size() << std::endl;
//    LOGD("image size : %d", frame.size());
    /*for(int i = 0; i < 20; i++)
    {*/
        std::vector<FaceBox>allFaces;
//        mtcnn->SetNumThreads(4);
        mtcnn->SetMinFace(40);
        unsigned long start = get_cur_time();
//        ncnn::Mat mface = ncnn::Mat::from_pixels(frame.data, ncnn::Mat::PIXEL_BGR2RGB, frame.cols, frame.rows);
        mtcnn->detect(frame,minsize,scale, allFaces,3);
        unsigned long end = get_cur_time();
//        cout << "detected " << allFaces.size() << " faces , takes " << (end-start) / 1000 << " ms.\n";
        LOGD("detected %zu faces , takes %ld ms.\n", allFaces.size() ,(end-start) / 1000 );
        for(int f = 0; f < allFaces.size(); f++)
        {
            int quality = (90 - std::max(abs(allFaces[f].rot_x), abs(allFaces[f].rot_y))) / 0.9; //人脸质量计算
            /*for(int l = 0; l < allFaces[f].lmk.size(); l++){
                cv::circle(frame, allFaces[f].lmk[l], 2, Scalar(0,0,255));
            }
            cv::rectangle(frame, allFaces[f].bbox, Scalar(0,255,0), 2, 8, 0);
            if(i == 0) imwrite("result.jpg",frame);*/
//            cout << "Confidence: " << allFaces[f].score << " ,Pose: " << allFaces[f].rot_x << " " << allFaces[f].rot_y << " " << allFaces[f].rot_z << endl;
//            LOGD("Confidence %f ,Pose: %d , %d , %f ", allFaces[f].score ,allFaces[f].x1,allFaces[f].y1,allFaces[f].area);
//            LOGD("Confidence %f ,Pose: %f , %f , %f ", allFaces[f].score ,allFaces[f].rot_x,allFaces[f].rot_y,allFaces[f].rot_z);
        }

//    }

    }
    delete mtcnn;
    return 0;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_deepcam_mtcnn_MainActivity_close(JNIEnv *env, jobject instance) {

//    delete mtcnn;
    // TODO

}

