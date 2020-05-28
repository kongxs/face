package com.example.opencvdemo;

import android.app.Activity;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.ViewGroup;

import androidx.annotation.Nullable;


public class CameraActivity2 extends Activity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity2_camera);

        VaccaeSurfaceView view = new VaccaeSurfaceView(this);

        ViewGroup group = findViewById(R.id.surfaceviewlayout);
        group.addView(view);

//        JavaCameraView cameraView = new JavaCameraView(this, Camera.CameraInfo.CAMERA_FACING_FRONT);

    }
}
