package com.deepcam.mtcnn;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.ScriptIntrinsicYuvToRGB;
import android.renderscript.Type;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.Toast;


import com.deepcam.detect.natives.FaceDetectBean;
import com.deepcam.detect.natives.FaceDetectNative;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class CameraActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CameraActivity";
    private static final int PERMISSIONS_REQUEST = 1;
    private static final String PERMISSION_CAMERA = Manifest.permission.CAMERA;
    private static final String PERMISSION_STORAGE = Manifest.permission.WRITE_EXTERNAL_STORAGE;
    private static final int MSG_REMOVE_FACE_UI = 2;

    private ImageView mPreviewRect;
    private ImageView mFaceView;
    private Bitmap mFaceBitmap;
    private boolean mIsInProcess;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private int mCameraId = 0;
    private SurfaceTexture mSurfaceTexture;
    private Camera mCamera;
    private FaceDetectNative faceDetectNative;

    private boolean mIsNativeAvailable = true;
    private boolean hasDetecting = false;

    private RenderScript mRs;
    private ScriptIntrinsicYuvToRGB mYuvToRgbIntrinsic;
    private Type.Builder mYuvType, mRgbaType;
    private Allocation mIn, mOut;

    private Handler mMainHandler;

    private static final int MSG_UPDATE_FACE_UI = 100;
    private float[] mFeatureTemp = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_camera);
        mPreviewRect = (ImageView) findViewById(R.id.preview);
        mFaceView = (ImageView) findViewById(R.id.face_info);
        if (hasPermission()) {
            openFrontCamera();
        } else {
            requestPermission();
        }
        WindowManager wm = (WindowManager) this.getSystemService(Context.WINDOW_SERVICE);
        ViewGroup.LayoutParams params = mPreviewRect.getLayoutParams();
        params.width = wm.getDefaultDisplay().getWidth();
        params.height = (int) (params.width * (640.0f / 480.0f));
        Log.d(TAG, "surface view width = " + params.width + ", height = " + params.height);
        mPreviewRect.setLayoutParams(params);
        mFaceView.setLayoutParams(params);
        mRs = RenderScript.create(this);
        mYuvToRgbIntrinsic = ScriptIntrinsicYuvToRGB.create(mRs, Element.U8_4(mRs));
        faceDetectNative = FaceDetectNative.instance();
        faceDetectNative.init("/sdcard/mtcnn/models",2,40);
        openFrontCamera();
        mMainHandler = new MainHandler();
    }


    private class MainHandler extends Handler {
        private Canvas mCanvas;
        private Paint mPaint;

        MainHandler() {
            super();
            mPaint = new Paint();
            mPaint.setColor(Color.RED);
            mPaint.setStyle(Paint.Style.STROKE);
        }

        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_UPDATE_FACE_UI:
                    FaceDetectBean tmp = (FaceDetectBean) msg.obj;
                    if (mFaceBitmap != null) {
                        mFaceBitmap.recycle();
                    }
                    Log.d(TAG, "update face ui here...");
                    mFaceBitmap = Bitmap.createBitmap(mPreviewHeight, mPreviewWidth, Bitmap.Config.ARGB_8888);
//                    if(tmp.rect != null && tmp.rect.length == 4) {
                    mCanvas = new Canvas(mFaceBitmap);
                    mPaint.setStrokeWidth(mPreviewWidth / 200 + 1);
                    mPaint.setColor(Color.GREEN);
//                        if(tmp.eyeNumber > 0) {
//                            mPaint.setColor(Color.GREEN);
//                        } else {
//                            mPaint.setColor(Color.RED);
//                        }
                    mCanvas.drawRect(tmp.getFaceRect(), mPaint);
//                    }
                    mFaceView.setImageBitmap(mFaceBitmap);
                    break;
                case MSG_REMOVE_FACE_UI:
                    if (mFaceBitmap != null) {
                        mFaceBitmap.recycle();
                    }
                    mFaceBitmap = Bitmap.createBitmap(mPreviewHeight, mPreviewWidth, Bitmap.Config.ARGB_8888);
                    mCanvas = new Canvas(mFaceBitmap);
                    mPaint.setColor(Color.RED);
                    mCanvas.drawText("未检测到正常的人脸", mPreviewWidth / 4, mPreviewHeight / 2, mPaint);
                    mFaceView.setImageBitmap(mFaceBitmap);
//                    mPaint.reset();
                    break;
            }
            hasDetecting = false;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        startPreview();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mCamera != null) {
            mCamera.stopPreview();
        }
    }


    @Override
    public void onClick(View arg0) {
        switch (arg0.getId()) {

        }
    }

    private void openFrontCamera() {
        if (mCamera != null) {
            return;
        }
        mCameraId = -1;
        int cameraNum = Camera.getNumberOfCameras();
        for (int i = 0; i < cameraNum; i++) {
            Camera.CameraInfo tmp = new Camera.CameraInfo();
            Camera.getCameraInfo(i, tmp);
            if (tmp.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                mCameraId = i;
                break;
            }
        }
        if (mCameraId < 0) {
            Log.e(TAG, "can not find the front camera");
            //return;
            mCameraId = 0;
        }
        Log.d(TAG, "the front camera id : " + mCameraId);
        mCamera = Camera.open(mCameraId);
    }

    public static Bitmap rotateBitmap(Bitmap bitmap, int degrees) {
        if (degrees == 0 || null == bitmap) {
            return bitmap;
        }
        Matrix matrix = new Matrix();
        matrix.setRotate(degrees, bitmap.getWidth() / 2, bitmap.getHeight() / 2);
        Bitmap bmp = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
        if (null != bitmap) {
            bitmap.recycle();
        }
        return bmp;
    }

    private Camera.PreviewCallback dataCallback = new Camera.PreviewCallback() {
        @Override
        public void onPreviewFrame(byte[] data, Camera camera) {
            if (!mIsNativeAvailable) {
                return;
            }
            if (hasDetecting) {
                return;
            }
            hasDetecting = true;
            if (mYuvType == null) {
                mYuvType = new Type.Builder(mRs, Element.U8(mRs)).setX(data.length);
                mIn = Allocation.createTyped(mRs, mYuvType.create(), Allocation.USAGE_SCRIPT);

                mRgbaType = new Type.Builder(mRs, Element.RGBA_8888(mRs)).setX(mPreviewWidth).setY(mPreviewHeight);
                mOut = Allocation.createTyped(mRs, mRgbaType.create(), Allocation.USAGE_SCRIPT);
            }
            mIn.copyFrom(data);

            mYuvToRgbIntrinsic.setInput(mIn);
            mYuvToRgbIntrinsic.forEach(mOut);
            Bitmap bmpout = Bitmap.createBitmap(mPreviewWidth, mPreviewHeight, Bitmap.Config.ARGB_8888);
            mOut.copyTo(bmpout);
            bmpout = rotateBitmap(bmpout, 90);
            mPreviewRect.setImageBitmap(bmpout);
//            setFaceImageInBitmap(bmpout);
            setFaceImageInBitmap(data,mPreviewWidth, mPreviewHeight);
        }
    };

    private synchronized void setFaceImageInBitmap(byte[] bmpout,int width,int height) {

        ArrayList<FaceDetectBean> infos = new ArrayList<>();
        int flag = faceDetectNative.detect(bmpout, width,height,0,0.33f,infos);
//        int flag = faceDetectNative.detect()
        Log.d(TAG, "setFaceImageInBitmap: " + flag);
        Log.d(TAG, "setFaceImageInBitmap: detect face size ===="+infos.size());
        if (infos.size() <= 0) {
            hasDetecting = false;
            return;
        }
        Message msg = new Message();
        if (flag == 0) {
            FaceDetectBean base = new FaceDetectBean();
            msg.what = MSG_UPDATE_FACE_UI;
            msg.obj = infos.get(0);
            mMainHandler.handleMessage(msg);
        } else {
            msg.what = MSG_REMOVE_FACE_UI;
            mMainHandler.handleMessage(msg);
        }

    }
    private synchronized void setFaceImageInBitmap(Bitmap bmpout) {

        ArrayList<FaceDetectBean> infos = new ArrayList<>();
//        int flag = faceDetectNative.detect(bmpout, width,height,0,0.33f,infos);
        int flag = faceDetectNative.detectBitmap(bmpout,0,0.33f,infos);
        Log.d(TAG, "setFaceImageInBitmap: " + flag);
        Log.d(TAG, "setFaceImageInBitmap: detect face size ===="+infos.size());
        if (infos.size() <= 0) {
            hasDetecting = false;
            return;
        }
        Message msg = new Message();
        if (flag == 0) {
            FaceDetectBean base = new FaceDetectBean();
            msg.what = MSG_UPDATE_FACE_UI;
            msg.obj = infos.get(0);
            mMainHandler.handleMessage(msg);
        } else {
            msg.what = MSG_REMOVE_FACE_UI;
            mMainHandler.handleMessage(msg);
        }

    }

    private float[] toArray(ArrayList<Float> feature) {
        if (feature != null) {
            int len = feature.size();
            if (len <= 0) {
                return null;
            } else {
                float[] temps = new float[len];
                for (int i = 0; i < len; i++) {
                    temps[i] = feature.get(i);
                }
                return temps;
            }
        } else {
            return null;
        }
    }

    private void startPreview() {
        if (mCamera == null) {
            return;
        }
        Camera.Parameters parameters = mCamera.getParameters();
        List<Camera.Size> previewSizes = parameters.getSupportedPreviewSizes();
        for (Camera.Size size : previewSizes) {
            Log.d(TAG, "support mPreviewWidth = " + size.width + ", mPreviewHeight = " + size.height);
            if (size.width == 240 && size.height == 320) {
                mPreviewWidth = size.width;
                mPreviewHeight = size.height;
                break;
            }
        }
        if (mPreviewWidth == 0 || mPreviewHeight == 0) {
            //Camera.Size tmp = parameters.getPreferredPreviewSizeForVideo();
            mPreviewHeight = 1080;
            mPreviewWidth = 1920;
        }
        Log.d(TAG, "mPreviewWidth = " + mPreviewWidth + ", mPreviewHeight = " + mPreviewHeight);
        parameters.setPreviewSize(mPreviewWidth, mPreviewHeight);

        List<String> focusModes = parameters.getSupportedFocusModes();
        if (focusModes.contains("continuous-video")) {
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
        }
        mCamera.setDisplayOrientation(270);
        mCamera.setParameters(parameters);
        try {
            if (mSurfaceTexture == null) {
                mSurfaceTexture = new SurfaceTexture(1000);
            }
            mCamera.setPreviewTexture(mSurfaceTexture);
            mCamera.setPreviewCallback(dataCallback);
            mCamera.startPreview();
        } catch (IOException e) {

        }
    }

    @Override
    public void onRequestPermissionsResult(
            final int requestCode, final String[] permissions, final int[] grantResults) {
        if (requestCode == PERMISSIONS_REQUEST) {
            if (grantResults.length > 0
                    && grantResults[0] == PackageManager.PERMISSION_GRANTED
                    && grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                openFrontCamera();
            } else {
                requestPermission();
            }
        }
    }

    private boolean hasPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return checkSelfPermission(PERMISSION_CAMERA) == PackageManager.PERMISSION_GRANTED &&
                    checkSelfPermission(PERMISSION_STORAGE) == PackageManager.PERMISSION_GRANTED;
        } else {
            return true;
        }
    }

    private void requestPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (shouldShowRequestPermissionRationale(PERMISSION_CAMERA) ||
                    shouldShowRequestPermissionRationale(PERMISSION_STORAGE)) {
                Toast.makeText(CameraActivity.this,
                        "Camera AND storage permission are required for this demo", Toast.LENGTH_LONG).show();
            }
            requestPermissions(new String[]{PERMISSION_CAMERA, PERMISSION_STORAGE}, PERMISSIONS_REQUEST);
        }
    }


    private void saveImage(Bitmap bitmap, String name) {
        String path = Environment.getExternalStorageDirectory().getAbsolutePath();
        File folder = new File(path, "DCIM");
        if (!folder.exists()) {
            folder.mkdirs();
        }
        folder = new File(folder, "eye");
        if (!folder.exists()) {
            folder.mkdirs();
        }
        File f = new File(folder, name + "_" + System.currentTimeMillis() + ".png");
        try {
            f.createNewFile();
        } catch (IOException e) {
            System.out.println("在保存图片时出错：" + e.toString());
        }

        FileOutputStream fOut = null;
        try {
            fOut = new FileOutputStream(f);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        try {
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, fOut);
        } catch (Exception e) {
            e.printStackTrace();
        }
        try {
            fOut.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            fOut.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}