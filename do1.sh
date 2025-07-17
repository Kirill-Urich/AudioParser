#!/bin/bash

# Включение строгого режима для безопасности
set -euo pipefail

# Конфигурационные переменные
SOURCE_DIR="/home/kirill/Programming/Music1"
TARGET_DIR="/home/kirill/Programming/Music2"
BUILD_DIR="build"
PROJECT_NAME="mp3organizer"

# Функция для цветного вывода сообщений
color_echo() {
    local color=$1
    local message=$2
    case $color in
        red)    echo -e "\033[31m[ERROR] $message\033[0m" >&2 ;;
        green)  echo -e "\033[32m[OK] $message\033[0m" ;;
        yellow) echo -e "\033[33m[WARN] $message\033[0m" >&2 ;;
        *)      echo -e "[INFO] $message" ;;
    esac
}

# Проверка и подготовка директорий
[ -d "$SOURCE_DIR" ] || { color_echo red "Source directory $SOURCE_DIR not found"; exit 1; }

color_echo green "Preparing target directory $TARGET_DIR"
if [ -d "$TARGET_DIR" ]; then
    color_echo yellow "Cleaning existing target directory"
    rm -rf "${TARGET_DIR:?}/"*  # Защита от случайного удаления
else
    mkdir -p "$TARGET_DIR"
fi

# Копирование файлов
color_echo green "Copying files from $SOURCE_DIR to $TARGET_DIR"
cp -r "$SOURCE_DIR"/* "$TARGET_DIR" || {
    color_echo red "Failed to copy files";
    exit 1;
}

# Сборка проекта
color_echo green "Building project...."
[ -d "$BUILD_DIR" ] && { color_echo yellow "Removing old build directory"; rm -rf "$BUILD_DIR"; }

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || { color_echo red "Failed to enter build directory"; exit 1; }

cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON || {
    color_echo red "CMake configuration failed";
    exit 1;
}

cmake --build . --parallel || {
    color_echo red "Build failed";
    exit 1;
}

# Запуск приложения
color_echo green "Running organizer on $TARGET_DIR"
./"$PROJECT_NAME" "$TARGET_DIR" || {
    color_echo red "Organizer execution failed";
    exit 1;
}

color_echo green "Operation completed successfully"
exit 0
