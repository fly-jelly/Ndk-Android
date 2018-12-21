package com.deepcam.detect.natives;
/*
 * @author:ji.zheng
 * @email:ji.zheng@deepcam.com
 * @date 2018/12/20
 */

import android.graphics.Rect;

import java.util.ArrayList;

public class FaceDetectBean {

    private int quality;//人脸质量 （1 ~ 100）约大质量约好
    private Rect faceRect;
    private ArrayList<Double> lmk;

    public FaceDetectBean(){}

    public FaceDetectBean(int quality, Rect faceRect) {
        this.quality = quality;
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
}
