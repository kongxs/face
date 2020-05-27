package com.example.opencvdemo;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.AsyncTask;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class VaccaeSurfaceView extends SurfaceView implements SurfaceHolder.Callback {


    SurfaceTexture surfaceTexture;
    //Camera相关
    //设置摄像头ID，我们默认为后置
    private int mCameraIndex= Camera.CameraInfo.CAMERA_FACING_FRONT;
    //定义摄像机
    private Camera camera;
    //定义Camera的回调方法

    boolean isSaved = false;
    private Camera.PreviewCallback previewCallback=new Camera.PreviewCallback() {
        @Override
        public void onPreviewFrame(byte[] bytes, Camera camera) {

                if (!isProcessing) {
                    new sTask().execute(bytes);
                }

//                synchronized (this) {
//
//                    Canvas canvas = holder.lockCanvas();
//                    if (canvas != null) {
//
//                        int width=camera.getParameters().getPreviewSize().width;
//
//                        int height=camera.getParameters().getPreviewSize().height;
//                        Bitmap bitmap = null;//BitmapFactory.decodeResource(getResources(),R.drawable.test);
//                        bitmap = MainActivity.nv21ToBitmap(bytes, width, height);
//                        if (bitmap != null) {
//
//                            Matrix m = new Matrix();
//                            m.postRotate(-90);
//                            bitmap = Bitmap.createBitmap(bitmap,0,0,width,height,m,false);
//
//                            canvas.drawBitmap(bitmap, 0, 0, null);
//                        }
//                        holder.unlockCanvasAndPost(canvas);
//                    }
//                }
            }
    };

    //定义SurfaceHolder
    private SurfaceHolder holder;
    private boolean isProcessing = false;

    //构造函数
    public VaccaeSurfaceView(Context context) {
        super(context);
        surfaceTexture = new SurfaceTexture(2);
//        texture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());

//
        //获取Holder
        holder=getHolder();
        //加入SurfaceHolder.Callback在类中implements
        holder.addCallback(this);
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        //保持屏幕常亮
        holder.setKeepScreenOn(true);

    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        //开启摄像机
        startCamera(mCameraIndex);
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        //关闭摄像机
        stopCamera();
    }

    //region 开启关闭Camera
    //开启摄像机
    private void startCamera(int mCameraIndex) {
        // 初始化并打开摄像头
        if (camera == null) {
            try {
                camera=Camera.open(mCameraIndex);
            } catch (Exception e) {
                return;
            }
            //获取Camera参数
            Camera.Parameters params=camera.getParameters();
            if (params.getSupportedFocusModes().contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
                // 自动对焦
                params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
            }
            params.setPreviewFormat(ImageFormat.NV21); // 设置预览图片格式
            params.setPictureFormat(ImageFormat.JPEG); // 设置拍照图片格式
            camera.setParameters(params);

//            surfaceTexture.setDefaultBufferSize(camera.getParameters().getPreviewSize().width, camera.getParameters().getPreviewSize().height);

            try {
                camera.setPreviewCallback(previewCallback);
                camera.setPreviewTexture(surfaceTexture);
//                camera.setPreviewDisplay(holder);
                //旋转90度
                camera.setDisplayOrientation(90);
                camera.startPreview();
            } catch (Exception ex) {
                ex.printStackTrace();
                camera.release();
                camera=null;
            }
        }
    }

    //关闭摄像机
    private void stopCamera() {
        if (camera != null) {
            camera.setPreviewCallback(null);
            camera.stopPreview();
            camera.release();
            camera=null;
        }

    }

    class sTask extends AsyncTask<byte[],Integer,Bitmap> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            isProcessing = true;
        }

        @Override
        protected Bitmap doInBackground(byte[]... bytes) {


            int width=camera.getParameters().getPreviewSize().width;

            int height=camera.getParameters().getPreviewSize().height;
            Bitmap bitmap = null;//BitmapFactory.decodeResource(getResources(),R.drawable.test);
            bitmap = MainActivity.nv21ToBitmap(bytes[0], width, height);

            Matrix m = new Matrix();
            m.postRotate(-90);
            bitmap = Bitmap.createBitmap(bitmap,0,0,width,height,m,false);

            return bitmap;
        }

        @Override
        protected void onPostExecute(Bitmap bitmap) {
            Canvas canvas = holder.lockCanvas();
            canvas.drawBitmap(bitmap, 0, 0, null);
            holder.unlockCanvasAndPost(canvas);

            isProcessing = false;
        }
    }
}
