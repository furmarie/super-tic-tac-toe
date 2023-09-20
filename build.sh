#!/bin/sh


if [[ $# -eq 0 ]]; then
    echo "Usage:"
    echo -e "\t-d  build for desktop"
    echo -e "\t-r  enable hot reload, only works for linux"
    echo -e "\t-w  build for web, output to ./app"
    exit 0
fi

raylib_ver="4.5.0"
raylib_url="https://github.com/raysan5/raylib/archive/refs/tags/$raylib_ver.tar.gz"
raylib_dir="./raylib/raylib-$raylib_ver"

set -xe

if [[ ! -d "./raylib/raylib-${raylib_ver}" ]]; then
    mkdir -p ./raylib 
    raylib_tar=./raylib/raylib-$raylib_ver.tar.gz
    wget https://github.com/raysan5/raylib/archive/refs/tags/${raylib_ver}.tar.gz -O $raylib_tar
    tar -xf $raylib_tar -C ./raylib
    rm $raylib_tar
fi


build_desktop() {
    # Building raylib
    mkdir -p ./build

    CFLAGS="-ggdb"
    LIBS="-lraylib -lm -ldl -lpthread"
    INCLUDES="-I$raylib_dir/src/ -I$raylib_dir/src/external/"
    if [[ $hot_reload -eq 1 ]]; then
        if [[ ! -f ./build/libraylib.so ]]; then
            make -C $raylib_dir/src/ clean
            make -j4 -C "$raylib_dir/src/" PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED
            cp $raylib_dir/src/libraylib.so* ./build/
        fi
        gcc $CFLAGS -o ./build/libplug.so -fPIC -shared \
            $INCLUDES ./src/sttt.c ./src/ttt.c -L./build/ $LIBS -Wl,-rpath=./build/ -Wl,-rpath=./
        gcc $CFLAGS -DHOT_RELOAD -o ./build/ttt ./src/main.c ./src/hot_reload.c \
            $INCLUDES -L./build/ $LIBS -Wl,-rpath=./build/ -Wl,-rpath=./
    else
        if [[ ! -f ./build/libraylib.a ]]; then
            make -C $raylib_dir/src/ clean
            make -j4 -C "$raylib_dir/src/" PLATFORM=PLATFORM_DESKTOP
            cp $raylib_dir/src/libraylib.a ./build/
        fi
        gcc $CFLAGS -o ./build/ttt ./src/main.c ./src/sttt.c ./src/ttt.c $LIBS \
            $INCLUDES -L./build/
    fi

    cp -r ./resources ./build/
}

build_web() {
    if [[ ! -f ./build/web/libraylib.a ]]; then
        make -C $raylib_dir/src/ clean
        make -j 4 -C "$raylib_dir/src/" PLATFORM=PLATFORM_WEB
        mkdir -p ./build/web
        cp $raylib_dir/src/libraylib.a ./build/web/
    fi
    emcc -o ./app/ttt.js ./src/ttt.c ./src/main.c \
         -I$raylib_dir/src/ -I$raylib_dir/src/external/ \
         -L./build/web/ -lraylib \
         -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s USE_GLFW=3 \
         -s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1 --no-heap-copy -O3 \
         $LIBS 
}

flag_web=0
flag_desktop=0
hot_reload=0

while getopts "wdr" opt
do
    case $opt in
        "w") flag_web=1;;
        "d") flag_desktop=1;;
        "r") hot_reload=1;;
    esac
done

[ "$flag_desktop" -eq 1 ] && build_desktop
[ "$flag_web" -eq 1 ] && build_web

exit 0
