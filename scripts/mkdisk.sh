#!/usr/bin/env bash

set -e

ARCH=
BOOTBIN_EXT=
UEFI_ARCH=
BOOT_TYPE=bios
OUTPUT=
PRESERVE_TEMP=false
PART_TABLE_IMAGE=
CURRENT_PART_IMAGE=
declare -a PART_IMAGES

print_usage() {
    echo "usage: $0 [-a arch] [-hSu] output"
}

while getopts "a:hSu" arg; do
    case $arg in
        a)
            ARCH=$OPTARG
            ;;
        h)
            print_usage
            exit 0
            ;;
        S)
            PRESERVE_TEMP=true
            ;;
        u)
            BOOT_TYPE=uefi
            ;;
        *)
            print_usage
            exit 1
            ;;
    esac
done

case $ARCH in
    ia32)
        BOOTBIN_EXT=X86
        UEFI_ARCH=IA32
        ;;
    amd64)
        BOOTBIN_EXT=X64
        UEFI_ARCH=X64
        ;;
    *)
        echo "$0: unknown architecture"
        exit 1
        ;;
esac

case $BOOT_TYPE in
    uefi) ;;
    bios) ;;
    *)
        echo "$0: unknown boot type"
        exit 1
        ;;
esac

shift "$((OPTIND - 1))"
OUTPUT=$1


# Partition 0
CURRENT_PART_IMAGE=$(mktemp)
PART_IMAGES+=("$CURRENT_PART_IMAGE")
dd if=/dev/zero of="$CURRENT_PART_IMAGE" bs=512 count=16443
case $BOOT_TYPE in
    uefi)
        mformat -i "$CURRENT_PART_IMAGE" -H 63
        mmd -i "$CURRENT_PART_IMAGE" ::/EFI
        mmd -i "$CURRENT_PART_IMAGE" ::/EFI/BOOT
        mcopy -i "$CURRENT_PART_IMAGE" "build/vellum/arch/$ARCH/pc/uefi/vellum.efi" ::/EFI/BOOT/BOOT$UEFI_ARCH.EFI
        ;;
    bios)
        mformat -i "$CURRENT_PART_IMAGE" -H 63 -B "build/vellum/arch/$ARCH/pc/bios/fdboot.bin"
        mcopy -i "$CURRENT_PART_IMAGE" "build/vellum/arch/$ARCH/pc/bios/stage1.bin" ::/STAGE1.$BOOTBIN_EXT
        mcopy -i "$CURRENT_PART_IMAGE" "build/vellum/arch/$ARCH/pc/bios/vellum.bin" ::/VELLUM.$BOOTBIN_EXT
        ;;
esac
mmd -i "$CURRENT_PART_IMAGE" ::/CONFIG
mcopy -i "$CURRENT_PART_IMAGE" vellum/config/boot.json ::/CONFIG/boot.json
mmd -i "$CURRENT_PART_IMAGE" ::/MODULES
mcopy -i "$CURRENT_PART_IMAGE" build/vellum/modules/guishell/guishell.mod ::/MODULES/guishell.MOD
mcopy -i "$CURRENT_PART_IMAGE" build/vellum/modules/helloworld/helloworld.mod ::/MODULES/HELOWRLD.MOD
mcopy -i "$CURRENT_PART_IMAGE" build/vellum/vellum.map ::/VELLUM.MAP
mcopy -i "$CURRENT_PART_IMAGE" build/vellum/unifont.bfn ::/UNIFONT.BFN
mcopy -i "$CURRENT_PART_IMAGE" disk/plchldr.bmp ::/PLCHLDR.BMP
mcopy -i "$CURRENT_PART_IMAGE" disk/unicode.txt ::/UNICODE.TXT

if [ "${PRESERVE_TEMP}" = true ]; then
    cp "$CURRENT_PART_IMAGE" "${OUTPUT%.*}.part0.img";
fi

# Partition Table
PART_TABLE_IMAGE=$(mktemp)
case $BOOT_TYPE in
    uefi)
        dd if=/dev/zero of="$PART_TABLE_IMAGE" bs=512 count=63
        cat "$PART_TABLE_IMAGE" "${PART_IMAGES[@]}" > "$OUTPUT"
        cat <<'EOF' | gdisk "$OUTPUT"
o
y
x
l
63
m
n
1
63
16505
ef00
w
y
EOF
        ;;
    bios)
        cp "build/vellum/arch/$ARCH/pc/bios/mbrboot.bin" "$PART_TABLE_IMAGE"
        dd if=/dev/zero bs=512 count=62 >>"$PART_TABLE_IMAGE"
        cat "$PART_TABLE_IMAGE" "${PART_IMAGES[@]}" > "$OUTPUT"

        cat <<'EOF' | fdisk -e "$OUTPUT"
e 1
01
n
63
16443
f 1
q
EOF
        ;;
    *)  ;;
esac

if [ "${PRESERVE_TEMP}" = true ]; then
    cp "$PART_TABLE_IMAGE" "${OUTPUT%.*}.pt.img";
fi


rm "$PART_TABLE_IMAGE" "${PART_IMAGES[@]}"
