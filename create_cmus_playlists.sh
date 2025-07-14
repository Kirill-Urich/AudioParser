#!/bin/bash

MUSIC_DIR="$HOME/Programming/Music2"
PLAYLISTS_DIR="$HOME/.config/cmus/playlists"

mkdir -p "$PLAYLISTS_DIR"

find "$MUSIC_DIR" -type f \( -name "*.mp3" -o -name "*.flac" -o -name "*.ogg" \) | while read -r SONG_PATH; do
    RELATIVE_PATH="${SONG_PATH#$MUSIC_DIR/}"
    ARTIST=$(echo "$RELATIVE_PATH" | cut -d'/' -f1)
    ALBUM=$(echo "$RELATIVE_PATH" | cut -d'/' -f2)
    
    PLAYLIST_NAME="$ARTIST - $ALBOM"
    PLAYLIST_PATH="$PLAYLISTS_DIR/$PLAYLIST_NAME.pl"
    
    echo "$SONG_PATH" >> "$PLAYLIST_PATH"
done

echo "Готово! Плейлисты сохранены в: $PLAYLISTS_DIR"
