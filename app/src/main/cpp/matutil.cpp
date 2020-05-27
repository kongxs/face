//
// Created by 孔祥书 on 2020/5/21.
//

#include "matutil.h"
//
// Created by 孔祥书 on 2020/5/21.
//
#include <jni.h>
#include <string>
#include "opencv2/opencv.hpp"

#include <android/log.h>
#include <android/bitmap.h>


#include "opencv2/opencv.hpp"
//#include "opencv2/highgui.hpp"
//#include "opencv2/core/core.hpp"

using namespace cv;
using namespace std;


cv::Mat bitmap2Mat(JNIEnv *env, jobject origin) {


    AndroidBitmapInfo info ;

    AndroidBitmap_getInfo(env, origin, &info);

    int32_t format = info.format;

    if (format != ANDROID_BITMAP_FORMAT_RGBA_8888 && format != ANDROID_BITMAP_FORMAT_RGB_565) {
//        return NULL;
    }

    void *bitmapPixels = NULL;


    AndroidBitmap_lockPixels(env, origin, &bitmapPixels);

    cv::Mat mat;

    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(info.height, info.width, CV_8UC4, bitmapPixels);    // 建立临时 mat
        tmp.copyTo(mat);                                                         // 拷贝到目标 matrix
    } else {
        cv::Mat tmp(info.height, info.width, CV_8UC2, bitmapPixels);
        cv::cvtColor(tmp, mat, cv::COLOR_BGR5652RGB);
    }

    AndroidBitmap_unlockPixels(env, origin);

    return mat;
}

jobject mat2Bitmap(JNIEnv *env ,cv::Mat src) {
    mat2Bitmap(env,src,false,getConfig(env));
}


jobject mat2Bitmap(JNIEnv *env ,cv::Mat src, bool needPremultiplyAlpha,jobject bitmap_config) {

    if (bitmap_config == NULL) {
        bitmap_config = getConfig(env);
    }

    jclass java_bitmap_class = (jclass) env->FindClass("android/graphics/Bitmap");

    jmethodID mid = env->GetStaticMethodID(java_bitmap_class,
                                     "createBitmap",
                                     "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");

    jobject bitmap = env->CallStaticObjectMethod(java_bitmap_class,
                                                 mid, src.size().width, src.size().height, bitmap_config);

    AndroidBitmapInfo  info;
    void*              pixels = 0;

    try {
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3 || src.type() == CV_8UC4);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        if(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888){
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if(src.type() == CV_8UC1){
                cvtColor(src, tmp, CV_GRAY2RGBA);
            }else if(src.type() == CV_8UC3){
                cvtColor(src, tmp, CV_RGB2RGBA);
            }else if(src.type() == CV_8UC4){
                if(needPremultiplyAlpha){
                    cvtColor(src, tmp, COLOR_RGBA2mRGBA);
                }else{
                    src.copyTo(tmp);
                }
            }
        }else{
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            if(src.type() == CV_8UC1){
                cvtColor(src, tmp, CV_GRAY2BGR565);
            }else if(src.type() == CV_8UC3){
                cvtColor(src, tmp, CV_RGB2BGR565);
            }else if(src.type() == CV_8UC4){
                cvtColor(src, tmp, CV_RGBA2BGR565);
            }
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return bitmap;
    }catch(cv::Exception e){
        AndroidBitmap_unlockPixels(env, bitmap);
        jclass je = env->FindClass("java/lang/Exception");
        if(!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return bitmap;
    }catch (...){
        AndroidBitmap_unlockPixels(env, bitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nMatToBitmap}");
        return bitmap;
    }

}


jobject getConfig(JNIEnv *env , jobject origin) {

    jclass java_bitmap_class = (jclass)env->FindClass("android/graphics/Bitmap");
    jmethodID mid = env->GetMethodID(java_bitmap_class, "getConfig", "()Landroid/graphics/Bitmap$Config;");
    jobject bitmap_config = env->CallObjectMethod(origin, mid);

    return bitmap_config;
}

jobject getConfig(JNIEnv *env) {
    
    jclass config_class = env->FindClass("android/graphics/Bitmap$Config");

    jfieldID fieldId = env->GetStaticFieldID(config_class, "RGB_565","Landroid/graphics/Bitmap$Config;");


//    return env->GetStaticObjectField(env, config_class, fieldId);

    return env->GetStaticObjectField(config_class,fieldId);
}



jobject decodeBitmap(JNIEnv *env , jstring file_path) {


    jclass  bitmap_factory = env->FindClass("android/graphics/BitmapFactory");

    jmethodID pId = env->GetStaticMethodID(bitmap_factory, "decodeFile",
                                     "(Ljava/lang/String;)Landroid/graphics/Bitmap;");

    return env->CallStaticObjectMethod(bitmap_factory, pId,file_path);
}










