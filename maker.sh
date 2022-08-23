#!/bin/sh

build() {
    set -o xtrace
    cmake -DBUILD_SHARED_LIBS=False \
          -G Ninja -S . -B ./Build \
          -D CMAKE_BUILD_TYPE=Debug \
    && ninja -C ./Build
}

release() {
    set -o xtrace
    cmake -DBUILD_SHARED_LIBS=False \
          -G Ninja -S . -B ./Build \
          -D CMAKE_BUILD_TYPE=Release \
    && ninja -C ./Build
}

winbuild() {
    set -o xtrace
    cmake -DBUILD_SHARED_LIBS=False \
          -G "Visual Studio 16 2019" -S . -B ./Build
}

clean() {
    set -o xtrace
    rm -rf ./Build/*
}

help() {
    echo "Использование:"
    echo "./maker.sh build -> Собрать проект с помощью CMake и ninja -C ./Build/"
    echo "./maker.sh winbuild -> Сгенерировать проектные файлы для Visual Studio 16 2019 в директорию ./Build/"
    echo "./maker.sh clean -> Удалить содержимое директории ./Build/"
}

case "$1" in
    build) build ;;
    release) release ;;
    winbuild) winbuild ;;
    clean) clean ;;
    *) help ;;
esac

