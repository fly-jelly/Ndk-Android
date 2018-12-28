package com.deepcam.detect.natives;
/*
 * @author:ji.zheng
 * @email:ji.zheng@deepcam.com
 * @date 2018/12/20
 */

import android.graphics.Rect;

import java.util.ArrayList;

public class FaceDetectBean {

    private int ID;

    private int frameCount;
    private int quality;//人脸质量 （1 ~ 100）约大质量约好
    private int confidence;
    private Rect faceRect;
    private ArrayList<Double> lmk;

    public FaceDetectBean(){}

    public FaceDetectBean(int quality,int ID,int frameCount,int confidence, Rect faceRect) {
        this.ID = ID;
        this.quality = quality;
        this.frameCount = frameCount;
        this.confidence = confidence;
        this.faceRect = faceRect;
        this.lmk = new ArrayList<>();
    }

    public int getQuality() {
        return quality;
    }

    public void setLmk(double point) {
        this.lmk.add(point);
    }

    public Rect getFaceRect() {
        return faceRect;
    }

    public ArrayList<Double> getLmk() {
        return lmk;
    }


    public int getID() {
        return ID;
    }
    public int getFrameCount() {
        return frameCount;
    }
    public int getConfidence() {
        return confidence;
    }
}
