BUILD_DIR=../curl-djgpp
SOURCE_DIR=.

GCC_BIN_DIR=$(dirname $(which i586-pc-msdosdjgpp-gcc))

mkdir $BUILD_DIR

cmake -H$SOURCE_DIR -B$BUILD_DIR \
  -DCMAKE_SYSTEM_NAME=DOS \
  -DCMAKE_SYSTEM_PROCESSOR=x86 \
  -DCMAKE_C_COMPILER_TARGET=i586-pc-msdosdjgpp \
  -DCMAKE_AR="$GCC_BIN_DIR/i586-pc-msdosdjgpp-ar" \
  -DCMAKE_ASM_COMPILER="$GCC_BIN_DIR/i586-pc-msdosdjgpp-gcc" \
  -DCMAKE_C_COMPILER="$GCC_BIN_DIR/i586-pc-msdosdjgpp-gcc" \
  -DCMAKE_CXX_COMPILER="$GCC_BIN_DIR/i586-pc-msdosdjgpp-g++" \
  -DCMAKE_LINKER="$GCC_BIN_DIR/i586-pc-msdosdjgpp-ld" \
  -DCMAKE_OBJCOPY="$GCC_BIN_DIR/i586-pc-msdosdjgpp-objcopy" \
  -DCMAKE_RANLIB="$GCC_BIN_DIR/i586-pc-msdosdjgpp-ranlib" \
  -DCMAKE_SIZE="$GCC_BIN_DIR/i586-pc-msdosdjgpp-size" \
  -DCMAKE_STRIP="$GCC_BIN_DIR/i586-pc-msdosdjgpp-strip" \
  -DWATT_ROOT="$DOJSPATH/$WATT32" \
  -DMBEDTLS_INCLUDE_DIR="$DOJSPATH/$MBEDTLS/include" \
  -DMBEDTLS_LIBRARY="$DOJSPATH/$MBEDTLS/library/libmbedtls.a" \
  -DMBEDX509_LIBRARY="$DOJSPATH/$MBEDTLS/library/libmbedx509.a" \
  -DMBEDCRYPTO_LIBRARY="$DOJSPATH/$MBEDTLS/library/libmbedcrypto.a" \
  -DZLIB_INCLUDE_DIR="$DOJSPATH/$ZLIB" \
  -DZLIB_LIBRARY="$DOJSPATH/$ZLIB/libz.a" \
  -DCURL_USE_LIBPSL=OFF \
  -DCURL_USE_MBEDTLS=ON \
  -DCURL_USE_OPENSSL=OFF \
  -DCMAKE_VERBOSE_MAKEFILE=ON \
  -DHAVE_FCNTL=OFF

make -C $BUILD_DIR libcurl_static
