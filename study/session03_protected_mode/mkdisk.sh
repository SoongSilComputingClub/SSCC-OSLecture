#!/usr/bin/env bash

set -euo pipefail

ARCH=
OUTPUT=

print_usage() {
    echo "usage: $0 [-a arch] output"
}

while getopts "a:h" arg; do
    case $arg in
        a)
            ARCH=$OPTARG
            ;;
        h)
            print_usage
            exit 0
            ;;
        *)
            print_usage
            exit 1
            ;;
    esac
done

case $ARCH in
    ia32) ;;
    *)
        echo "$0: unknown architecture"
        exit 1
        ;;
esac

shift "$((OPTIND - 1))"
OUTPUT=$1

PART_IMAGE=$(mktemp)
PART_TABLE_IMAGE=$(mktemp)

dd if=/dev/zero of="$PART_IMAGE" bs=512 count=16443
mformat -i "$PART_IMAGE" -H 63 -B "build/vellum/arch/ia32/pc/bios/fdboot.bin"
mcopy -i "$PART_IMAGE" "build/vellum/arch/ia32/pc/bios/stage1.bin" ::/STAGE1.X86
mcopy -i "$PART_IMAGE" "build/vellum/arch/ia32/pc/bios/vellum.bin" ::/VELLUM.X86

cp "build/vellum/arch/ia32/pc/bios/mbrboot.bin" "$PART_TABLE_IMAGE"
dd if=/dev/zero bs=512 count=62 >>"$PART_TABLE_IMAGE"
cat "$PART_TABLE_IMAGE" "$PART_IMAGE" > "$OUTPUT"

OS_TYPE=$(uname -s)

if [ "$OS_TYPE" = "Darwin" ]; then
    cat <<'EOF' | fdisk -e "$OUTPUT"
e 1
01
n
63
16443
f 1
q
EOF
elif [ "$OS_TYPE" = "Linux" ]; then
    sfdisk "$OUTPUT" <<EOF
label: dos
start=63, size=16443, type=01, bootable
EOF
else
    echo "$0: unsupported operating system ($OS_TYPE)"
    rm "$PART_TABLE_IMAGE" "$PART_IMAGE"
    exit 1
fi

rm "$PART_TABLE_IMAGE" "$PART_IMAGE"
