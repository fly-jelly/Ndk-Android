package com.deepcam.detect.natives;
/*
 * @author:ji.zheng
 * @email:ji.zheng@deepcam.com
 * @date 2018/12/20
 */

import android.content.Context;
import android.graphics.Bitmap;

import java.util.ArrayList;

public class FaceDetectNative {

    private static FaceDetectNative mFaceDetectNative = null;
//    private static String mModelsPath = null;
    private  FaceDetectNative() {
        System.loadLibrary("face_detect");
    }

    public synchronized static FaceDetectNative instance(){
        if (mFaceDetectNative == null){
            mFaceDetectNative = new FaceDetectNative();
        }
        /*if (mModelsPath == null){
            mModelsPath = path;
        }*/
        return mFaceDetectNative;
    }

    /**
     * 模型文件存放路径
     * @param path 路径 (eg:/sdcard/deepcam/)
     * @param thread_num 检测人脸开启线程（四线程的设备建议开三线程 ）
     * @param min 最小检测人脸的大小（默认检测人脸大小 40 * 40）
     * @return
     */
    public native int init(String path,int thread_num,int min);

    /**
     * 检测人脸
     * @param frame 【in】nv21相机流
     * @param cols 【in】frame 的高度
     * @param rows 【in】frame 的宽度
     * @param angle 【in】帧图片旋转角度
     * @param scale 【in】图片压缩比例 （0 ~ 1）
     * @param faces【out】 回调人脸对象数组
     * @return
     */
    public native int detect(byte[] frame, int cols, int rows, int angle,float scale, ArrayList<FaceDetectBean> faces);

    public native int detectBitmap(Bitmap bitmap,int angle,float scale,ArrayList<FaceDetectBean> faces);

    /**
     * 回收jni中内存空间
     */
    public native void close();

}
