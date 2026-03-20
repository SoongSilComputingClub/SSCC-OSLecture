#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

SESSION_ORDER=(
    session02_stage0
    session03_protected_mode
    session04_memory
    session05_drivers
    session06_bios_video
    session07_libc
    session08_shell
    session09_graphics
    session10_fat
    session11_elf
)

usage() {
    echo "usage: $0 [--start|--solution] <session02|session03|...|session11> <output-dir>"
}

SELECTED_VARIANT="start"

case "${1-}" in
    --start)
        shift
        ;;
    --solution|--solved)
        SELECTED_VARIANT="solution"
        shift
        ;;
esac

if [ "$#" -ne 2 ]; then
    usage
    exit 1
fi

SESSION_SHORT="$1"
TARGET_DIR="$2"

case "$SESSION_SHORT" in
    session02) SELECTED_SESSION="session02_stage0" ;;
    session03) SELECTED_SESSION="session03_protected_mode" ;;
    session04) SELECTED_SESSION="session04_memory" ;;
    session05) SELECTED_SESSION="session05_drivers" ;;
    session06) SELECTED_SESSION="session06_bios_video" ;;
    session07) SELECTED_SESSION="session07_libc" ;;
    session08) SELECTED_SESSION="session08_shell" ;;
    session09) SELECTED_SESSION="session09_graphics" ;;
    session10) SELECTED_SESSION="session10_fat" ;;
    session11) SELECTED_SESSION="session11_elf" ;;
    *)
        usage
        exit 1
        ;;
esac

if [ -e "$TARGET_DIR" ] && [ -n "$(find "$TARGET_DIR" -mindepth 1 -maxdepth 1 2>/dev/null)" ]; then
    echo "$0: output directory must be empty or not exist: $TARGET_DIR" >&2
    exit 1
fi

mkdir -p "$TARGET_DIR"

if ! command -v rsync >/dev/null 2>&1; then
    echo "$0: rsync is required" >&2
    exit 1
fi

copy_root_file() {
    local src_rel="$1"
    local src="$ROOT_DIR/$src_rel"
    local dest="$TARGET_DIR/$src_rel"

    mkdir -p "$(dirname "$dest")"

    if [ -d "$src" ]; then
        rsync -a "$src/" "$dest/"
    else
        cp -p "$src" "$dest"
    fi
}

copy_repo_snapshot() {
    rsync -a \
        --exclude '.git' \
        --exclude 'build' \
        --exclude '.cache' \
        --exclude 'disk.img' \
        --exclude 'study' \
        "$ROOT_DIR/" \
        "$TARGET_DIR/"
}

session_has_filelist() {
    [ -f "$ROOT_DIR/study/$1/FILES.lst" ]
}

can_materialize_minimal_packet() {
    local session_dir

    for session_dir in "${SESSION_ORDER[@]}"; do
        if ! session_has_filelist "$session_dir"; then
            return 1
        fi

        if [ "$session_dir" = "$SELECTED_SESSION" ]; then
            return 0
        fi
    done

    return 1
}

copy_vendor_dep() {
    local src_dir="$1"
    local dest_dir="$2"

    if [ -d "$src_dir" ]; then
        mkdir -p "$(dirname "$dest_dir")"
        rsync -a "$src_dir/" "$dest_dir/"
    fi
}

copy_session_vendor_deps() {
    if [ "$SELECTED_SESSION" = "session11_elf" ]; then
        copy_vendor_dep "$ROOT_DIR/build/vellum/_deps/zlib-src" "$TARGET_DIR/third_party-src/zlib"
        copy_vendor_dep "$ROOT_DIR/build/vellum/_deps/uacpi-src" "$TARGET_DIR/third_party-src/uacpi"
    fi
}

copy_manifest_file() {
    local session_dir="$1"
    local src_rel="$2"
    local dest_rel="$3"
    local src="$ROOT_DIR/study/$session_dir/$src_rel"
    local dest="$TARGET_DIR/$dest_rel"

    mkdir -p "$(dirname "$dest")"
    cp -p "$src" "$dest"
}

manifest_for_session() {
    local session_dir="$1"
    local selected="$2"

    if [ "$session_dir" = "$selected" ] &&
        [ "$SELECTED_VARIANT" = "start" ] &&
        [ -f "$ROOT_DIR/study/$session_dir/START_MANIFEST.tsv" ]; then
        printf '%s' "$ROOT_DIR/study/$session_dir/START_MANIFEST.tsv"
    else
        printf '%s' "$ROOT_DIR/study/$session_dir/MANIFEST.tsv"
    fi
}

apply_stage_files() {
    local session_dir manifest src_rel dest_rel

    for session_dir in "${SESSION_ORDER[@]}"; do
        manifest="$(manifest_for_session "$session_dir" "$SELECTED_SESSION")"

        while IFS=$'\t' read -r src_rel dest_rel; do
            [ -n "$src_rel" ] || continue
            copy_manifest_file "$session_dir" "$src_rel" "$dest_rel"
        done <"$manifest"

        if [ "$session_dir" = "$SELECTED_SESSION" ]; then
            break
        fi
    done
}

copy_allowlisted_files() {
    local session_dir file_rel

    for session_dir in "${SESSION_ORDER[@]}"; do
        while IFS= read -r file_rel; do
            [ -n "$file_rel" ] || continue

            case "$file_rel" in
                \#*) continue ;;
            esac

            copy_root_file "$file_rel"
        done <"$ROOT_DIR/study/$session_dir/FILES.lst"

        if [ "$session_dir" = "$SELECTED_SESSION" ]; then
            break
        fi
    done
}

apply_future_overlays() {
    local overlay_now=0
    local session_dir manifest src_rel dest_rel

    for session_dir in "${SESSION_ORDER[@]}"; do
        if [ "$session_dir" = "$SELECTED_SESSION" ]; then
            overlay_now=1
        fi

        if [ "$overlay_now" -eq 0 ]; then
            continue
        fi

        manifest="$ROOT_DIR/study/$session_dir/MANIFEST.tsv"

        while IFS=$'\t' read -r src_rel dest_rel; do
            [ -n "$src_rel" ] || continue
            copy_manifest_file "$session_dir" "$src_rel" "$dest_rel"
        done <"$manifest"
    done
}

remove_unused_student_files() {
    rm -f "$TARGET_DIR/vellum/arch/ia32/pc/bios/instruction.c"
    rm -f "$TARGET_DIR/vellum/arch/ia32/pc/bios/include/vellum/plat/instruction.h"
}

if can_materialize_minimal_packet; then
    copy_allowlisted_files
    apply_stage_files
    copy_session_vendor_deps
else
    copy_repo_snapshot
    copy_vendor_dep "$ROOT_DIR/build/vellum/_deps/zlib-src" "$TARGET_DIR/third_party-src/zlib"
    copy_vendor_dep "$ROOT_DIR/build/vellum/_deps/uacpi-src" "$TARGET_DIR/third_party-src/uacpi"
    apply_future_overlays
fi

remove_unused_student_files

echo "Created buildable student checkout $SESSION_SHORT ($SELECTED_VARIANT) in $TARGET_DIR"
