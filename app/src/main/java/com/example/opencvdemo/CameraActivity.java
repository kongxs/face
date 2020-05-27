package com.example.opencvdemo;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;

import androidx.annotation.Nullable;

import com.lightweh.dlib.VisionDetRet;

import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.Utils;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

public class CameraActivity extends Activity implements CameraBridgeViewBase.CvCameraViewListener2 {

    private CameraBridgeViewBase cameraView;
    private boolean isFrontCamera;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);

        cameraView = findViewById(R.id.camera_surface_view);

        isFrontCamera = false;
        cameraView.setCameraIndex(CameraBridgeViewBase.CAMERA_ID_BACK);

        cameraView.enableFpsMeter();   //显示帧率
        cameraView.setMaxFrameSize(1280,720);
        cameraView.setCvCameraViewListener(this);

        cameraView.enableView();

    }


    @Override
    public void onCameraViewStarted(int width, int height) {

    }

    @Override
    public void onCameraViewStopped() {
    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {

        Mat mat = new Mat();

        synchronized (this) {
            Mat gray = inputFrame.rgba();
            Bitmap detector = MainActivity.detector2(gray.nativeObj);
            Utils.bitmapToMat(detector, mat);
        }

        return mat;
    }
}
