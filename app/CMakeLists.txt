# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

include_directories(
        src/main/cpp/include
        D:/Android/opencv-3.4.0-android-sdk/opencv-android-sdk/sdk/native/jni/include
)

add_library(lib_opencv STATIC IMPORTED ) #表示创建一个导入库，静态方式
#引入libopencv_java3.so文件
set_target_properties(lib_opencv
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libopencv_java3.so
        )#包含libopencv_java3.so这个库

add_library( # Sets the name of the library.
        face_detect

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
#             src/main/cpp/native-lib.cpp
             src/main/cpp/face-detect.cpp
             src/main/cpp/FaceQuality.cpp
             src/main/cpp/MTCNN.cpp
        )

#添加ncnn库
add_library(libncnn STATIC IMPORTED )
set_target_properties(libncnn
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libncnn.a)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        face_detect
                       libncnn
                       lib_opencv
        jnigraphics
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )