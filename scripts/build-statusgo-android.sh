#!/bin/bash
ANDROID_NDK_HOME=/home/richard/Android/Sdk/ndk/21.0.6113669
ANDROID_NDK_TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64
STATUS_GO_PATH=../vendor/status-go

cd $STATUS_GO_PATH
mkdir -p statusgo-lib
go run cmd/library/*.go > statusgo-lib/main.go
   
env \
  CC=$ANDROID_NDK_TOOLCHAIN/bin/clang \
  CXX=$ANDROID_NDK_TOOLCHAIN/bin/clang++ \
  GOOS=android GOARCH=386 API=23 \
  CGO_CFLAGS="-isysroot $ANDROID_NDK_TOOLCHAIN/sysroot -target i686-linux-android23" \
  CGO_LDFLAGS=" --sysroot $ANDROID_NDK_TOOLCHAIN/sysroot -target i686-linux-android23 -v -Wl,-soname,libstatus.so" \
  CGO_ENABLED=1 \
  go build -buildmode="c-shared" -o "./libstatus.so" ./statusgo-lib
