#!/bin/bash
set -eu
cd "$(dirname "$0")/.."
: "${CXX:=g++}"
: "${SDL_CFLAGS:=$(sdl2-config --cflags)}"
: "${SDL_LIBS:=$(sdl2-config --libs)}"
mkdir -p linux/build
mapfile -t cpu < <(find source/SM5XX -name '*.cpp' | sort)
"$CXX" -std=c++20 -O2 -DYOKOI_LINUX=1 -DYOKOI_ROMPACK_REQUIRED_CONTENT_VERSION=3 \
 ${CPPFLAGS:-} $SDL_CFLAGS -Isource -Ilinux -Iandroid/app/src/main/cpp/core -Iandroid/app/src/main/cpp/input \
 "${MAIN_SOURCE:-linux/main.cpp}" linux/third_party/lodepng.cpp "${cpu[@]}" source/std/gw_pack.cpp source/std/debug_log.cpp source/std/timer.cpp source/std/platform_paths.cpp \
 android/app/src/main/cpp/core/yokoi_cpu_utils.cpp android/app/src/main/cpp/input/yokoi_input_mapping.cpp \
 ${LDFLAGS:-} $SDL_LIBS -pthread -o "${OUTPUT:-linux/build/yokoi}"
