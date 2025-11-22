bash ./configure \
    --build=`gcc -dumpmachine` \
    --host=${HOST} \
    --disable-shared
