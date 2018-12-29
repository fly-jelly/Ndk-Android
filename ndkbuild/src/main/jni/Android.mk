LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  


OPENCV_LIB_TYPE:=STATIC
include D:/Android/opencv-3.4.0-android-sdk/opencv-android-sdk/sdk/native/jni/OpenCV.mk		
  
LOCAL_MODULE    := face-detect  

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ \
					$(LOCAL_PATH)/include \
					D:/Android/opencv-3.4.0-android-sdk/opencv-android-sdk/sdk/native/jni/include 
					

LOCAL_SRC_FILES := face-detect.cpp \
					FaceQuality.cpp \
					MTCNN.cpp \
					dtracker.cpp	

# -mfloat-abi=softfp -mfpu=neon 使用 arm_neon.h 必须    
LOCAL_CFLAGS := -D__cpusplus -g -mfloat-abi=softfp -mfpu=neon    

LOCAL_ARM_NEON := true					
				   
LOCAL_LDFLAGS	:= $(LOCAL_PATH)/ncnn-lib/$(TARGET_ARCH_ABI)/libncnn.a 
LOCAL_LDLIBS    := -lz -lm -llog -landroid -lomp -ljnigraphics 
				   
include $(BUILD_SHARED_LIBRARY) 
