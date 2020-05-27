package com.example.opencvdemo;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.example.opencvdemo.camera.CameraAct2;
import com.lightweh.dlib.VisionDetRet;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");

        initDlib();
    }

    private ImageView origin;

    private ImageView gray;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);



        origin = findViewById(R.id.origin);
        gray = findViewById(R.id.gray);
        origin.setOnClickListener(this);

        findViewById(R.id.cvcamera).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(MainActivity.this, CameraActivity.class));
            }
        });

        findViewById(R.id.camera2).setOnClickListener(new View.OnClickListener() {
                                                         @Override
                                                         public void onClick(View view) {

                                                             startActivity(new Intent(MainActivity.this, CameraAct2.class));
                                                         }
                                                     });


        findViewById(R.id.camera).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                startActivity(new Intent(MainActivity.this,CameraActivity2.class));
                String absolutePath = Environment.getExternalStorageDirectory()
                        .getAbsolutePath() + "/hello2.png";
                System.out.println(absolutePath);
//
//                Bitmap camera = camera(absolutePath);
//
//                if (camera !=null) {
//                    gray.setImageBitmap(camera);
//                }

            }
        });

//        transPic(Environment.getExternalStorageDirectory().getAbsolutePath());
    }


    public static native void initDlib();


    public static native VisionDetRet[] detector(Bitmap bitmap);

    public static native VisionDetRet[] detectorInCV(Bitmap bitmap);

    public static native Bitmap detector2(long ptrpoint);


    public static native String stringFromJNI();

    public static native String transPic(String path);

    public static native Bitmap Bitmap2Grey(Bitmap origin);

    public static native int[] Bitmap2Grey2(int[] bytes ,int width,int height);

    public static native Bitmap camera(String path);

    public static native Bitmap nv21ToBitmap(byte[] bytes,int width ,int height);

    @Override
    public void onClick(View view) {

        Bitmap bitmap = BitmapFactory.decodeResource(getResources(),R.drawable.test);

        if (bitmap != null) {

            Bitmap.createBitmap(1,1, Bitmap.Config.RGB_565);


//            transPic(Environment.getExternalStorageDirectory().getAbsolutePath());

//            Bitmap bitmap1 = Bitmap2Grey(bitmap);

            int w = bitmap.getWidth(), h = bitmap.getHeight();

            int[] pix = new int[w * h];

            bitmap.getPixels(pix, 0, w, 0, 0, w, h);

            int[] resultPixes = Bitmap2Grey2(pix, w, h);

            Bitmap result = Bitmap.createBitmap(w,h, Bitmap.Config.RGB_565);
            result.setPixels(resultPixes, 0, w, 0, 0,w, h);


            if (bitmap != null)
             gray.setImageBitmap(result);
        }

//        transPic(Environment.getExternalStorageDirectory().getAbsolutePath());

//        Bitmap bitmap = BitmapFactory.decodeResource(getResources(),R.drawable.ic_launcher);
//
//        int w = origin.getWidth();
//        int h = origin.getHeight();
//        int[] piexls = new int[w*h];
//        bitmap.getPixels(piexls,0,w,0,0,w,h);
//        int[] resultData = Bitmap2Grey(piexls,w,h);
//        Bitmap resultImage = Bitmap.createBitmap(w,h, Bitmap.Config.ARGB_8888);
//        resultImage.setPixels(resultData,0,w,0,0,w,h);
//        gray.setImageBitmap(resultImage);
    }
}
