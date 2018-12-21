//
// @author:ji.zheng
// @email:ji.zheng@deepcam.com on 
// @date 2018/12/20.
//
#include <jni.h>
#include <MTCNN.h>
#include <android/log.h>
#include <android/bitmap.h>

#define TAG "Mtcnn Demo "
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)

jint outJava(JNIEnv *pEnv, jobject pJobject, vector<FaceBox> vector);

static unsigned long get_cur_time(void) {
    struct timeval tv;
    unsigned long ts;
    gettimeofday(&tv, NULL);
    ts = tv.tv_sec * 1000000 + tv.tv_usec;
    return ts;
}

extern "C" {
MTCNN *mtcnn;
JNIEXPORT jint JNICALL
Java_com_deepcam_detect_natives_FaceDetectNative_init(JNIEnv *env, jobject instance, jstring path_,
                                                      jint thread_num, jint min) {
    const char *path = env->GetStringUTFChars(path_, 0);

    mtcnn = new MTCNN(path);
    mtcnn->SetMinFace(min);
    mtcnn->SetNumThreads(thread_num);
    env->ReleaseStringUTFChars(path_, path);
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_deepcam_detect_natives_FaceDetectNative_detect(JNIEnv *env, jobject instance,
                                                        jbyteArray frame_, jint cols, jint rows,
                                                        jint angle, jfloat scale, jobject faces) {
    jbyte *frame = env->GetByteArrayElements(frame_, NULL);

    if (NULL == frame) {
        LOGD("导入数据为空，直接返回空");
        env->ReleaseByteArrayElements(frame_, frame, 0);
        return -1;
    }

    if (cols < 40 || rows < 40) {
        LOGD("The frame width: %d ,height:%d ，frame so small !");
        return -2;
    }

    unsigned long start = get_cur_time();
    cv::Mat yuvMat(rows + rows / 2, cols, CV_8UC1, (uchar *) frame);
    unsigned long end = get_cur_time();
    LOGD("yuv time %ld ms.\n", (end - start) / 1000);
    start = get_cur_time();
    cv::Mat img(rows, cols, CV_8UC4);
//    imwrite("/sdcard/mtcnn/face1.jpg",yuvMat);
    cv::cvtColor(yuvMat, img, CV_YUV2BGR_NV21);
//    imwrite("/sdcard/mtcnn/face2.jpg",img);
    int rotate = 0;
    if (angle != 0) {
        if (angle == 90) {
            rotate = ROTATE_90_CLOCKWISE;
        } else if (angle == 180) {
            rotate = ROTATE_180;
        } else {
            rotate = ROTATE_90_COUNTERCLOCKWISE;
        }
        cv::rotate(img, img, rotate);
    }
    end = get_cur_time();
    LOGD("img time %ld ms.\n", (end - start) / 1000);

    imwrite("/sdcard/mtcnn/face.jpg", img);
    start = get_cur_time();
    std::vector<FaceBox> allFaces;
    mtcnn->detect(img, scale, allFaces);
    end = get_cur_time();
    LOGD("detect time %ld ms.\n", (end - start) / 1000);
    LOGD("face size = %d", allFaces.size());
    env->ReleaseByteArrayElements(frame_, frame, 0);
    if (allFaces.size() <= 0) {
        return -3;//人脸检测失败
    }
    return outJava(env, faces, allFaces);

}
JNIEXPORT void JNICALL
Java_com_deepcam_detect_natives_FaceDetectNative_close(JNIEnv *env, jobject instance) {
    if (!mtcnn) {
        delete mtcnn;
    }
}
}

jint outJava(JNIEnv *pEnv, jobject pJobject, vector <FaceBox> faces) {

    jclass clazzFace = pEnv->FindClass("com/deepcam/detect/natives/FaceDetectBean");
    if (clazzFace == 0) {
        return -4;//找不到 FaceDetectBean
    }
    jmethodID initFace = pEnv->GetMethodID(clazzFace, "<init>", "(ILandroid/graphics/Rect;)V");
    if (initFace == 0) {
//        pEnv->DeleteGlobalRef(clazzFace);
        return -5;
    }
    jmethodID setLandmark = pEnv->GetMethodID(clazzFace, "setLmk", "(D)V");

    jclass rectClazz = pEnv->FindClass("android/graphics/Rect");
    if (rectClazz == 0) {
        return -7;
    }
    jmethodID rectMethod = pEnv->GetMethodID(rectClazz, "<init>", "(IIII)V");
    if (rectMethod == 0) {
        return -8;
    }

    jclass clazzArr = pEnv->FindClass("java/util/ArrayList");
    if (clazzArr == 0) {
        return -6;
    }
    /* jmethodID addlmk = pEnv->GetMethodID(clazzArr,"add","(D)Z");
     if(addlmk ==0){
         return -9;
     }*/
    jmethodID addObj = pEnv->GetMethodID(clazzArr, "add", "(Ljava/lang/Object;)Z");
    if (addObj == 0) {
        return -11;
    }

    for (int i = 0; i < faces.size(); ++i) {
        FaceBox box = faces[i];
        jobject rect = pEnv->NewObject(rectClazz, rectMethod, box.bbox.x, box.bbox.y,
                                       (box.bbox.x + box.bbox.width),
                                       (box.bbox.y + box.bbox.height));
//        jobject array = pEnv->NewObject(clazzArr,arrInit);

        int quality = (90 - std::max(abs(box.rot_x), abs(box.rot_y))) / 0.9; //人脸质量计算
        jobject faceBean = pEnv->NewObject(clazzFace, initFace, quality, rect);
        for (int j = 0; j < box.lmk.size(); ++j) {
            Point2d point = box.lmk[j];
            pEnv->CallVoidMethod(faceBean, setLandmark, point.x);
            pEnv->CallVoidMethod(faceBean, setLandmark, point.y);
        }
        pEnv->CallBooleanMethod(pJobject, addObj, faceBean);
        pEnv->DeleteLocalRef(faceBean);
        pEnv->DeleteLocalRef(rect);
//        pEnv->DeleteLocalRef(array);
    }

    pEnv->DeleteLocalRef(clazzArr);
    pEnv->DeleteLocalRef(rectClazz);
    pEnv->DeleteLocalRef(clazzFace);
    return 0;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_deepcam_detect_natives_FaceDetectNative_detectBitmap(
        JNIEnv *env, jobject instance, jobject _bitmap, jint angle, jfloat scale, jobject faces) {
    AndroidBitmapInfo bitmapInfo;
    void *pixelsAddr;
    int ret;

    LOGD("convertToGray");
    if ((ret = AndroidBitmap_getInfo(env, _bitmap, &bitmapInfo)) < 0) {
        LOGD("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return -1;
    }

    LOGD("color image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",
         bitmapInfo.width, bitmapInfo.height, bitmapInfo.stride, bitmapInfo.format,
         bitmapInfo.flags);
    if (bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGD("Bitmap format is not RGBA_8888 !");
        return -2;
    }


    if ((ret = AndroidBitmap_lockPixels(env, _bitmap, &pixelsAddr)) < 0) {
        LOGD("AndroidBitmap_lockPixels() failed ! error=%d", ret);
    }

    Mat test(bitmapInfo.height, bitmapInfo.width, CV_8UC4, pixelsAddr);
//      Mat bgra;
    Mat bgr;

//      Mat rgba;

    //转换成BGRA
//      cvtColor(test,bgra,CV_RGBA2BGRA);
    //转换成BGR
    cvtColor(test, bgr, CV_RGBA2BGR);
    //从BGR转换回RGBA，可再将数据拷贝回bitmap
//      cvtColor(test,rgba,CV_BGR2RGBA);
    int rotate = 0;
    if (angle != 0) {
        if (angle == 90) {
            rotate = ROTATE_90_CLOCKWISE;
        } else if (angle == 180) {
            rotate = ROTATE_180;
        } else {
            rotate = ROTATE_90_COUNTERCLOCKWISE;
        }
        cv::rotate(bgr, bgr, rotate);
    }
    unsigned long start = get_cur_time();
    unsigned long end = get_cur_time();
    start = get_cur_time();
    std::vector<FaceBox> allFaces;
    mtcnn->detect(bgr, scale, allFaces);
    end = get_cur_time();
    LOGD("detect time %ld ms.\n", (end - start) / 1000);
    LOGD("face size = %d", allFaces.size());
    AndroidBitmap_unlockPixels(env, _bitmap);

    if (allFaces.size() <= 0) {
        return -3;//人脸检测失败
    }
    return outJava(env, faces, allFaces);
}