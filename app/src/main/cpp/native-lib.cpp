#include <android/log.h>
#include <android/bitmap.h>
#include "matutil.h"
#include "face_detector.h"
#include "jni_primitives.h"
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>

using namespace cv;
using namespace std;
using namespace dlib;


//人脸检测核心模型
frontal_face_detector detector = get_frontal_face_detector();
shape_predictor sp;

CascadeClassifier faceCascade;//opencv

#define TAG "opencvLogTesst"

#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR, TAG, FORMAT, ##__VA_ARGS__);


//JNI_VisionDetRet *g_pJNI_VisionDetRet;

JavaVM *g_javaVM = NULL;

JNI_VisionDetRet *g_pJNI_VisionDetRet;

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    g_javaVM = vm;
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);

    g_pJNI_VisionDetRet = new JNI_VisionDetRet(env);

    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {

    g_javaVM = NULL;

//    delete g_pJNI_VisionDetRet;
}


namespace {

#define JAVA_NULL 0
    using DetPtr = FaceDetector *;

    class JNI_FaceDet {
    public:
        JNI_FaceDet(JNIEnv *env) {
            jclass clazz = env->FindClass(CLASSNAME_FACE_DET);
            mNativeContext = env->GetFieldID(clazz, "mNativeFaceDetContext", "J");
            env->DeleteLocalRef(clazz);
        }

        DetPtr getDetectorPtrFromJava(JNIEnv *env, jobject thiz) {
            DetPtr const p = (DetPtr) env->GetLongField(thiz, mNativeContext);
            return p;
        }

        void setDetectorPtrToJava(JNIEnv *env, jobject thiz, jlong ptr) {
            env->SetLongField(thiz, mNativeContext, ptr);
        }

        jfieldID mNativeContext;
    };
    // Protect getting/setting and creating/deleting pointer between java/native
    std::mutex gLock;

    std::shared_ptr<JNI_FaceDet> getJNI_FaceDet(JNIEnv *env) {
        static std::once_flag sOnceInitflag;
        static std::shared_ptr<JNI_FaceDet> sJNI_FaceDet;
        std::call_once(sOnceInitflag, [env]() {
            sJNI_FaceDet = std::make_shared<JNI_FaceDet>(env);
        });
        return sJNI_FaceDet;
    }

    DetPtr const getDetPtr(JNIEnv *env, jobject thiz) {
        std::lock_guard<std::mutex> lock(gLock);
        const shared_ptr<JNI_FaceDet> &ptr = getJNI_FaceDet(env);

        return ptr->getDetectorPtrFromJava(env, thiz);
    }

    // The function to set a pointer to java and delete it if newPtr is empty
    void setDetPtr(JNIEnv *env, jobject thiz, DetPtr newPtr) {
        std::lock_guard<std::mutex> lock(gLock);
        DetPtr oldPtr = getJNI_FaceDet(env)->getDetectorPtrFromJava(env, thiz);
        if (oldPtr != JAVA_NULL) {
            delete oldPtr;
        }

        getJNI_FaceDet(env)->setDetectorPtrToJava(env, thiz, (jlong) newPtr);
    }

}  // end unnamespace

void whiteFace2(Mat matSelfPhoto,int alpha, int beta);

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_opencvdemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jclass /* this */) {
    std::string hello = "Hello from C++";

    return env->NewStringUTF(hello.c_str());
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_opencvdemo_MainActivity_transPic(JNIEnv *env, jclass thiz, jstring path) {

    const char *_path = env->GetStringUTFChars(path, JNI_FALSE);

    LOGE("%s" ,_path);

    Mat mat(2,2, CV_8UC3, Scalar(0,0,255));

    char result[200] = "";

    sprintf(result, "%s/hello.png" , _path);

    LOGE("%s",result);

    imwrite(result,mat);

    LOGE("write  success ");

//    FILE *file = fopen(result, "wrb");

//    fputs(result,file);

//    fclose(file);


    return env->NewStringUTF(_path);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_example_opencvdemo_MainActivity_Bitmap2Grey(JNIEnv *env, jclass clazz,
        jobject origin) {

    LOGE("bit gray ");
    AndroidBitmapInfo info ;

    int ret = AndroidBitmap_getInfo(env, origin, &info);

    jobject result;

    if (ret  < 0 ) {
        LOGE("获取bitmap失败");
        return origin;
    }

    LOGE("bibtmap success ")

    int32_t format = info.format;

    if (format != ANDROID_BITMAP_FORMAT_RGBA_8888 && format != ANDROID_BITMAP_FORMAT_RGB_565) {
        LOGE("not 8888 or 565");
        return origin;
    }

    void *bitmapPixels = NULL;

    LOGE("begain lock ");

    ret = AndroidBitmap_lockPixels(env, origin, &bitmapPixels);
    
    if (ret < 0) {
        LOGE("lock error ... ");
        return origin;
    }

    cv::Mat mat;

    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(info.height, info.width, CV_8UC4, bitmapPixels);    // 建立临时 mat
        tmp.copyTo(mat);                                                         // 拷贝到目标 matrix
    } else {
        cv::Mat tmp(info.height, info.width, CV_8UC2, bitmapPixels);
        cv::cvtColor(tmp, mat, cv::COLOR_BGR5652RGB);
    }


    AndroidBitmap_unlockPixels(env, origin);


    LOGE("bitmap to mat success ");

    jobject bitmap_config = getConfig(env, origin);

    cvtColor(mat,mat,CV_BGR2GRAY);

    return mat2Bitmap(env,mat,false,bitmap_config);
//    return origin;

}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_example_opencvdemo_MainActivity_Bitmap2Grey2(JNIEnv *env, jclass clazz, jintArray bytes,
                                                      jint width, jint height) {


    jint *cbuf = env->GetIntArrayElements(bytes, JNI_FALSE);


    Mat imgData(height, width, CV_8UC4, (unsigned char *) cbuf);


    uchar* ptr = imgData.ptr(0);
    for(int i = 0; i < width*height; i ++){
        //计算公式：Y(亮度) = 0.299*R + 0.587*G + 0.114*B
        //对于一个int四字节，其彩色值存储方式为：BGRA
        int grayScale = (int)(ptr[4*i+2]*0.299 + ptr[4*i+1]*0.587 + ptr[4*i+0]*0.114);
        ptr[4*i+1] = grayScale;
        ptr[4*i+2] = grayScale;
        ptr[4*i+0] = grayScale;
    }

    int size = width * height;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, cbuf);
    env->ReleaseIntArrayElements(bytes, cbuf, 0);
    return result;
}


extern "C"
JNIEXPORT jobject JNICALL
Java_com_example_opencvdemo_MainActivity_camera(JNIEnv *env, jclass thiz,
        jstring _path) {

    const char *path = env->GetStringUTFChars(_path ,JNI_FALSE);

    jobject bitmap = decodeBitmap(env, _path);

    const Mat mat = bitmap2Mat(env, bitmap);

    whiteFace2(mat, 1.5,30);

    return  mat2Bitmap(env , mat ,false,getConfig(env, bitmap));
}

void whiteFace2(Mat matSelfPhoto,int alpha, int beta){

    for (int y = 0; y < matSelfPhoto.rows; y++)
    {
        for (int x = 0; x < matSelfPhoto.cols; x++)
        {
            for (int c = 0; c < 3; c++)
            {
                matSelfPhoto.at<Vec3b>(y, x)[c] = saturate_cast<uchar>(alpha*(matSelfPhoto.at<Vec3b>(y, x)[c]) + beta);
            }
        }
    }
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_example_opencvdemo_MainActivity_nv21ToBitmap(JNIEnv *env, jclass clazz,
        jbyteArray bytes,jint width,jint height) {

    char *path1 = "/storage/emulated/0/me.jpg";
    char *faces = "/storage/emulated/0/dlib_68.dat";
    
    
    jbyte *buf = env->GetByteArrayElements(bytes, JNI_FALSE);

    Mat yuvimg(height * 3 / 2, width, CV_8UC1, buf);

    Mat rgbimg(height, width, CV_8UC4);

    cvtColor(yuvimg, rgbimg, COLOR_YUV2RGB_NV21);

//    //旋转
//    Point2f center( (float)(height / 2) , (float) (width/2));
//    Mat affine_matrix = getRotationMatrix2D( center, 90, 1.0 );
//    warpAffine(rgbimg, rgbimg, affine_matrix, rgbimg.size());


    dlib::cv_image<bgr_pixel> dlibImg(rgbimg); //dlib的facedetector只接受cv_image<bgr_pixel>图像格式

    const dlib::rectangle &rectOri = get_rect(dlibImg);
//
//    pyramid_up(dlibImg);//为了检测更小的人脸，放大图片
//
//    const rectangle &rectUp = get_rect(dlibImg);
//    float scalY = rectUp.bottom() / rectOri.bottom();
//    float scalX = rectUp.right() / rectOri.right();
//    LOGE("%f == %f",scalY, scalX);

    const Vector<dlib::rectangle> &vector = detector(dlibImg);
    LOGE("检测到 %d 个 \n "  ,  vector.size());

    jobject pJobject = mat2Bitmap(env, rgbimg, false, NULL);

    return pJobject;
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_opencvdemo_MainActivity_initDlib(JNIEnv *env, jclass clazz) {

    char *faces = "/storage/emulated/0/shape_predictor_68_face_landmarks.dat";
    deserialize(faces) >> sp;

    faceCascade.load("/storage/emulated/0/haarcascade_frontalface_alt2.xml");


    LOGE("dlib init success ...");
}

extern "C" float getDistance(CvPoint pointO, CvPoint pointA)
{
    float distance;
    distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
    distance = sqrtf(distance);
    return distance;
}

cv::Rect box(0, 0, 0, 0);
std::vector<cv::Point2d> pts2d;		// 用于存储检测的点
array2d<rgb_pixel> img;//注意变量类型 rgb_pixel 三通道彩色图像
std::vector<full_object_detection> shapes;//注意形状变量的类型，full_object_detection

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_example_opencvdemo_MainActivity_detector(JNIEnv *env, jclass clazz, jobject bitmap) {


    clock_t start = clock();

    Mat src = bitmap2Mat(env, bitmap);

    Mat rgbimg  , face_gray;
    cvtColor(src, rgbimg, COLOR_RGBA2BGR);

    cvtColor( rgbimg, face_gray, CV_BGRA2GRAY );  //rgb类型转换为灰度类型
    equalizeHist( face_gray, face_gray );   //直方图均衡化

    dlib::cv_image<bgr_pixel> dlibImg(rgbimg);
    
    const Vector<dlib::rectangle> &vector = detector(dlibImg);

    clock_t dettector = clock() - start;

    LOGE("detector time ： %d \n" , dettector/CLOCKS_PER_SEC);

    jobjectArray pArray = JNI_VisionDetRet::createJObjectArray(env, vector.size());

    size_t size = vector.size();

    std::vector<dlib::full_object_detection> shapes;

    if (size > 0) {

        for (int i = 0; i < size; i++) {

            jobject pJobject = JNI_VisionDetRet::createJObject(env);
            env->SetObjectArrayElement(pArray, i, pJobject);
            dlib::rectangle rect = vector[i];

            g_pJNI_VisionDetRet->setRect(env, pJobject, rect.left(), rect.top(),
                                         rect.right(), rect.bottom());

            const full_object_detection &detection = sp(dlibImg, rect);

            for (int i = 0; i < 68; i++) {
                int x = detection.part(i).x();
                int y = detection.part(i).y();

                g_pJNI_VisionDetRet->addPoint(env,pJobject,x,y);

            }

            shapes.push_back(detection);
        }

        LOGE("finish time ： %d \n" , (clock() - dettector) / CLOCKS_PER_SEC);
    }

//    if (!shapes.empty()) {
//
//        int faceNumber = shapes.size();
//        for (int j = 0; j < faceNumber; j++) {
//            1.
//            for (int i = 0; i < 68; i++) {
//                full_object_detection &face = shapes[j];
//                point &part = face.part(i);
//
//                int x = part.x();
//                int y = part.y();
//
//                LOGE("face index: %d , part index: %d , x= %d , y= %d \n" ,j,i,x,y);
//
//                cv::circle(rgbimg, cvPoint(x, y), 3, cv::Scalar(0, 0, 255));
//            }

//            2.
//            full_object_detection &face = shapes[j];
//
//            for (int i = 17; i < 27; i++)//眉毛
//                circle(rgbimg, Point(face.part(i).x(),face.part(i).y()), 4, cv::Scalar(0, 0, 255), -1, 8, 0);
//
//            for (int i = 27; i < 31; i++)//鼻根 到鼻尖
//                circle(rgbimg, Point(face.part(i).x(),face.part(i).y()), 4, cv::Scalar(0, 0, 255), -1, 8, 0);
//
//            for (int i = 31; i < 36; i++)// 鼻子下沿
//                circle(rgbimg, Point(face.part(i).x(),face.part(i).y()), 4, cv::Scalar(0, 0, 255), -1, 8, 0);
//
//            for (int i = 36; i < 48; i++) // 左右眼睛
//                circle(rgbimg, Point(face.part(i).x(),face.part(i).y()), 4, cv::Scalar(0, 0, 255), -1, 8, 0);
//
//            for (int i = 48; i < 60; i++) // 上唇上和下唇下  -> 嘴巴外轮廓
//                circle(rgbimg, Point(face.part(i).x(),face.part(i).y()), 4, cv::Scalar(0, 0, 255), -1, 8, 0);
//
//            for (int i = 60; i < 68; i++) // 上唇下和下唇上  -> 嘴巴内轮廓
//                circle(rgbimg, Point(face.part(i).x(),face.part(i).y()), 4, cv::Scalar(0, 0, 255), -1, 8, 0);
//        }
//        char *faces = "/storage/emulated/0/face.png";
//        imwrite(faces , rgbimg);
//    }





    return pArray;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_lightweh_dlib_FaceDet_jniBitmapDet(JNIEnv *env,jclass clazz, jobject bitmap) {




    cv::Mat rgbaMat;
    cv::Mat bgrMat;

    rgbaMat = bitmap2Mat(env,bitmap);

    cv::cvtColor(rgbaMat, bgrMat, cv::COLOR_RGBA2BGR);

    LOGE("hello .... ");

    DetPtr mDetPtr = getDetPtr(env, clazz);

    LOGE("mDetPtr .... ");

    jint size = mDetPtr->Detect(bgrMat);

    LOGE("Java_com_lightweh_dlib_FaceDet_jniBitmapDet size : %d \n" , size);
    return bitmap;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lightweh_dlib_FaceDet_jniNativeClassInit(JNIEnv *env, jclass clazz) {


}extern "C"
JNIEXPORT jobject JNICALL
Java_com_example_opencvdemo_MainActivity_detector2(JNIEnv *env, jclass clazz, jlong ptr) {

    Mat &src = *(Mat*)ptr;

    Mat rgbimg;


    cvtColor(src, rgbimg, COLOR_RGBA2BGR);

    dlib::cv_image<bgr_pixel> dlibImg(rgbimg);

//    const Vector<dlib::rectangle> &vector = detector(dlibImg);

//    LOGE("size : %d \n" , vector.size());

    return mat2Bitmap(env,src,false,getConfig(env));
}

jobjectArray getRecResult(JNIEnv *env, DetPtr faceDetector, const std::vector<dlib::rectangle> &vector);

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_example_opencvdemo_MainActivity_detectorInCV(JNIEnv *env, jclass clazz, jobject bitmap) {

    cv::Mat rgbaMat;
    cv::Mat bgrMat;
    rgbaMat = bitmap2Mat(env, bitmap);
    cv::cvtColor(rgbaMat, bgrMat, cv::COLOR_RGBA2BGR);

    DetPtr mDetPtr = getDetPtr(env, clazz);
    Mat image = bgrMat;
    LOGE("begain detect .... ");
    if (image.empty())
        return 0;

    if (image.channels() == 1) {
        cv::cvtColor(image, image, CV_GRAY2BGR);
    }

    dlib::cv_image<dlib::bgr_pixel> dlib_image(image);
    
    __android_log_print(ANDROID_LOG_ERROR,"opencvLogTesst" ,  "begain face_detector ");

    const std::vector<dlib::rectangle> &vector = detector(dlib_image);

    __android_log_print(ANDROID_LOG_ERROR,"opencvLogTesst" ,  "end face_detector ");

    LOGE("end detect .... ");
    

    return getRecResult(env, mDetPtr, vector);

}

jobjectArray getRecResult(JNIEnv *env, DetPtr faceDetector, const std::vector<dlib::rectangle> &vector) {
    LOGE("begain getRecResult .... size :  \n" , vector.size());
    jobjectArray jDetRetArray = JNI_VisionDetRet::createJObjectArray(env, vector.size());

    for (int i = 0; i < vector.size(); i++) {
        jobject jDetRet = JNI_VisionDetRet::createJObject(env);
        env->SetObjectArrayElement(jDetRetArray, i, jDetRet);

        dlib::rectangle rect = vector[i];

        g_pJNI_VisionDetRet->setRect(env, jDetRet, rect.left(), rect.top(),
                                     rect.right(), rect.bottom());
    }
    return jDetRetArray;
}