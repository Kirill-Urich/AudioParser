#!/bin/bash

# Включение строгого режима
set -euo pipefail
IFS=$'\n\t'

# Конфигурация
SOURCE_DIR="/home/kirill/Music/Music1"
TARGET_DIR="/home/kirill/Music/Music2"
BUILD_DIR="$(pwd)/build"
PROJECT_NAME="mp3organizer"
CMUS_DIR="$HOME/.config/cmus"
CMUS_PLAYLIST_DIR="$CMUS_DIR/playlists"
LOG_FILE="/tmp/music_sync.log"

# Инициализация лога
exec > >(tee -a "$LOG_FILE") 2>&1
echo -e "\n=== Запуск $(date) ==="

# Функции
die() {
    echo -e "\033[31m[FATAL] $1\033[0m" >&2
    exit 1
}

validate_dir() {
    [ -d "$1" ] || die "Директория $1 не существует"
    [ -r "$1" ] || die "Нет прав на чтение $1"
    [ -w "$1" ] || die "Нет прав на запись $1"
}

create_playlists() {
    local music_dir=$1
    
    echo "Создание плейлистов из $music_dir"
    mkdir -p "$CMUS_PLAYLIST_DIR" || die "Не удалось создать $CMUS_PLAYLIST_DIR"
    
    # Очищаем только автоматически созданные плейлисты
    find "$CMUS_PLAYLIST_DIR" -name '*.pl' -delete
    
    # Общий плейлист
    find "$music_dir" -type f \( -name "*.mp3" -o -name "*.flac" -o -name "*.ogg" \) | sort > "$CMUS_PLAYLIST_DIR/00_ALL.pl"
    
    # Плейлисты по артистам и альбомам
    while IFS= read -r -d '' artist_dir; do
        artist=$(basename "$artist_dir")
        find "$artist_dir" -type f \( -name "*.mp3" -o -name "*.flac" -o -name "*.ogg" \) > "$CMUS_PLAYLIST_DIR/artist_$artist.pl"
        
        while IFS= read -r -d '' album_dir; do
            album=$(basename "$album_dir")
            find "$album_dir" -type f \( -name "*.mp3" -o -name "*.flac" -o -name "*.ogg" \) > "$CMUS_PLAYLIST_DIR/artist_${artist}_album_${album}.pl"
        done < <(find "$artist_dir" -mindepth 1 -maxdepth 1 -type d -print0)
    done < <(find "$music_dir" -mindepth 1 -maxdepth 1 -type d -print0)
}

# Основной процесс
validate_dir "$SOURCE_DIR"
mkdir -p "$TARGET_DIR" || die "Не удалось создать $TARGET_DIR."
validate_dir "$TARGET_DIR"

# Очистка и копирование
 echo "Очистка $TARGET_DIR и копирование файлов"
 rsync -a --delete --exclude=".*" "$SOURCE_DIR/" "$TARGET_DIR/"

# Сборка проекта
echo "Сборка проекта в $BUILD_DIR"
[ -d "$BUILD_DIR" ] && rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR" || die "Ошибка входа в $BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release || die "Ошибка cmake."
make -j$(nproc) || die "Ошибка сборки"

# Запуск организатора
echo "Запуск организатора для $TARGET_DIR"
./"$PROJECT_NAME" "$TARGET_DIR" || die "Ошибка организатора"

# Создание плейлистов и симлинка
create_playlists "$TARGET_DIR"
ln -sfn "$TARGET_DIR" "$CMUS_DIR/music_library"

# Итоговая статистика
echo -e "\n=== Результаты ==="
echo "Всего треков: $(find "$TARGET_DIR" -type f \( -name "*.mp3" -o -name "*.flac" -o -name "*.ogg" \) | wc -l)"
echo "Создано плейлистов: $(find "$CMUS_PLAYLIST_DIR" -name '*.pl' | wc -l)"
echo "Структура:"
tree -L 3 "$TARGET_DIR" | head -15
echo -e "\n\033[32mГотово! Для использования в cmus:\033[0m"
echo ":add -p $CMUS_PLAYLIST_DIR/00_ALL.pl"
echo "Лог сохранен в: $LOG_FILE"
