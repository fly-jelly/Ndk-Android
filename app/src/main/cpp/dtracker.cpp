/*************************************************************************
 *
 * DeepCam CONFIDENTIAL
 * FILE: dtracker.cpp
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

#include "dtracker.h"

bool DTracker::check_edge(cv::Mat &image, cv::Rect &bbox, float ratio) {
    if ((bbox.x - bbox.width / ratio) <= 0) return true;

    if ((bbox.y - bbox.height / ratio) <= 0) return true;

    if ((bbox.x + bbox.width + bbox.width / ratio) >= image.cols) return true;

    if ((bbox.y + bbox.height + bbox.height / ratio) >= image.rows) return true;

    return false;
}

float DTracker::IoU(const cv::Rect &boxA, const cv::Rect &boxB) {
    cv::Rect rect2 = boxA | boxB;
    cv::Rect rect3 = boxA & boxB;
    float iou = rect3.area() * 1.0 / rect2.area();
    return iou;
}

int DTracker::check_status() {
    std::vector<int> fidsToDelete;
    for (auto fid = 0; fid < faceTrackers.size(); fid++) {
        if (faceTrackers[fid].frameCount != frameCount) {
            faceTrackers[fid].quality -= 1;
            if (faceTrackers[fid].quality < 0) faceTrackers[fid].quality = 0;
            faceTrackers[fid].confidence -= 1;
            if (faceTrackers[fid].confidence < 0) faceTrackers[fid].confidence = 0;
        }
        if (faceTrackers[fid].age > trackerAge) {
            fidsToDelete.push_back(faceTrackers[fid].ID);
        } else {
            faceTrackers[fid].age += 1;
        }
    }
    for (auto del = 0; del < fidsToDelete.size(); del++) {
        for (auto fid = faceTrackers.begin(); fid != faceTrackers.end();) {
            if (fidsToDelete[del] == fid->ID) {
                int id = fid->ID;
                faceTrackers.erase(fid);
                //printf("Remove %d from faceTrackers.\n",id);
                break;
            }
            fid++;
        }
    }
    return 0;
}

int DTracker::detect(cv::Mat &image,float scale) {
    if (image.empty()) {
        printf("input image is empty.\n");
        return -1;
    }
    std::vector<FaceBox> allFaces;
    mtcnn->detect(image, scale, allFaces);
    for (size_t i = 0; i < allFaces.size(); i++) {
        int confidence = 0;
        cv::Rect currFace = allFaces[i].bbox;
        int quality = (90 - std::max(abs(allFaces[i].rot_x), abs(allFaces[i].rot_y))) / 0.9;
        if (allFaces[i].bbox.height <= 80)
            confidence = allFaces[i].bbox.height * 0.375 + quality * 0.7;
        else confidence = quality * 0.7 + 30;

        int matchedFid = -1;
        float matchedScore = 0.f;
        for (auto fid = 0; fid < faceTrackers.size(); fid++) {
            cv::Rect bbox = faceTrackers[fid].tbox;
            float iou = IoU(bbox, currFace);
            if (iou > matchedScore) {
                matchedFid = fid;
                matchedScore = iou;
            }
        }
        if ((matchedFid >= 0) && (matchedScore >= 0.2)) {
            int fid = matchedFid;
            faceTrackers[fid].age = 0;
            faceTrackers[fid].tbox = currFace;
            faceTrackers[fid].quality = confidence;
            faceTrackers[fid].confidence = confidence;
            faceTrackers[fid].frameCount = frameCount;
            faceTrackers[fid].lmks = allFaces[i].lmk;
        } else {
            TrackerBox trackerBox;
            trackerBox.age = 0;
            trackerBox.ID = currFaceID;
            trackerBox.tbox = currFace;
            trackerBox.quality = confidence;
            trackerBox.confidence = confidence;
            trackerBox.frameCount = frameCount;
            trackerBox.lmks = allFaces[i].lmk;
            faceTrackers.push_back(trackerBox);
            //printf("Create new tracker %d.\n",currFaceID);
            currFaceID++;
        }
    }
    return 0;
}

int DTracker::init(char *face_model,int min,int thread_num, float detect_threshold) {
    currFaceID = 1;
    frameCount = 0;
    trackerAge = 10;

    std::string model_path(face_model);
    mtcnn = new MTCNN(model_path);
    mtcnn->SetMinFace(min);
    mtcnn->SetNumThreads(thread_num);

    return 0;
}

std::vector<TrackerBox> DTracker::track(cv::Mat &image,float scale) {
    if (image.empty()) {
        printf("input image is empty!\n");
        return faceTrackers;
    }
    LOGD("tracker image width : %d ,height : %d",image.cols,image.rows);
    long start = get_cur_time();
    check_status();
    long end = get_cur_time();
    LOGD("check time %ld ms.\n", (end - start) / 1000);
    start = get_cur_time();
    detect(image,scale);
    end = get_cur_time();
    LOGD("check time detect %ld ms.\n", (end - start) / 1000);
    frameCount++;
    return faceTrackers;
}

int DTracker::destroy() {
    delete mtcnn;
    return 0;
}
