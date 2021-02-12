export ANDROID_NDK=/home/richard/Android/Sdk/ndk/21.0.6113669
export ANDROID_NDK_HOME=$ANDROID_NDK
export ANDROID_API=23
export ANDROID_HOME=/home/richard/Android/Sdk/
export TOOLCHAIN=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi

export CMAKE_PREFIX_PATH=/home/richard/Qt/5.14.2/android


#rm -Rf build/*
cd build

cmake \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_BUILD_ABI_x86=ON \
    -DANDROID_BUILD_ABI_armeabi-v7a=OFF \
    -DANDROID_NATIVE_API_LEVEL=$ANDROID_API \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_FIND_ROOT_PATH=$CMAKE_PREFIX_PATH \
    -DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_STL=c++_shared \
    -DANDROID_TOOLCHAIN=clang \
    -DANDROID_SDK=$ANDROID_HOME \
    -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
     -GNinja .. 


cmake --build . --target all
cmake --build . --target apk

#$ANDROID_HOME/platform-tools/adb install android-build//build/outputs/apk/debug/android-build-debug.apk

