# https://discourse.cmake.org/t/checktypesize-fails-with-mingw-cross-compiler/10599/2

BUILD_DIR=../curl-win32
SOURCE_DIR=.

GCC_BIN_DIR=$(dirname $(which i486-w64-mingw32-gcc))

mkdir $BUILD_DIR

cmake -H$SOURCE_DIR -B$BUILD_DIR \
  -DCMAKE_SYSTEM_PROCESSOR=x86 \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER_TARGET=i486-w64-mingw32 \
  -DCMAKE_AR="$GCC_BIN_DIR/i486-w64-mingw32-ar" \
  -DCMAKE_ASM_COMPILER="$GCC_BIN_DIR/i486-w64-mingw32-gcc" \
  -DCMAKE_C_COMPILER="$GCC_BIN_DIR/i486-w64-mingw32-gcc" \
  -DCMAKE_CXX_COMPILER="$GCC_BIN_DIR/i486-w64-mingw32-g++" \
  -DCMAKE_LINKER="$GCC_BIN_DIR/i486-w64-mingw32-ld" \
  -DCMAKE_OBJCOPY="$GCC_BIN_DIR/i486-w64-mingw32-objcopy" \
  -DCMAKE_RANLIB="$GCC_BIN_DIR/i486-w64-mingw32-ranlib" \
  -DCMAKE_SIZE="$GCC_BIN_DIR/i486-w64-mingw32-size" \
  -DCMAKE_STRIP="$GCC_BIN_DIR/i486-w64-mingw32-strip" \
  -DMBEDTLS_INCLUDE_DIR="$DOJSPATH/$MBEDTLS/include" \
  -DMBEDTLS_LIBRARY="$DOJSPATH/$MBEDTLS/library/libmbedtls.a" \
  -DMBEDX509_LIBRARY="$DOJSPATH/$MBEDTLS/library/libmbedx509.a" \
  -DMBEDCRYPTO_LIBRARY="$DOJSPATH/$MBEDTLS/library/libmbedcrypto.a" \
  -DZLIB_INCLUDE_DIR="$DOJSPATH/$ZLIB" \
  -DZLIB_LIBRARY="$DOJSPATH/$ZLIB/libz.a" \
  -DCURL_USE_LIBPSL=OFF \
  -DCURL_USE_MBEDTLS=ON \
  -DCURL_USE_OPENSSL=OFF \
  -DENABLE_THREADED_RESOLVER=OFF \
  -DPICKY_COMPILER=OFF \
  -DBUILD_CURL_EXE=OFF \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_STATIC_LIBS=ON \
  -DCMAKE_VERBOSE_MAKEFILE=ON
#  --trace-expand 2>&1 | tee cmake-curl.log

make -C $BUILD_DIR libcurl_static
