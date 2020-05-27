//
// Created by 孔祥书 on 2020/5/21.
//

#include <jni.h>
#include "opencv2/opencv.hpp"

#ifndef OPENCVDEMO_MATUTIL_H
#define OPENCVDEMO_MATUTIL_H

#endif //OPENCVDEMO_MATUTIL_H

cv::Mat bitmap2Mat(JNIEnv *env, jobject origin) ;

jobject mat2Bitmap(JNIEnv *env ,cv::Mat src, bool needPremultiplyAlpha,jobject bitmap_config);

jobject mat2Bitmap(JNIEnv *env ,cv::Mat src);

jobject getConfig(JNIEnv *env , jobject origin);

jobject getConfig(JNIEnv *env);

jobject decodeBitmap(JNIEnv *env , jstring path);