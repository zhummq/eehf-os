#!/bin/bash

DOOM_BIN="$HOME/doomgeneric/doomgeneric/doomgeneric"
#DOOM_WAD="$HOME/eehf-os/doom1.wad"
#FONT_PSF="$HOME/eehf-os/Solarize.12x29.psf"
DD_OUT="$HOME/eehf-os/hd3M.img"

SEC_CNT=$(ls -l $DOOM_BIN | awk '{printf("%d", ($5+511)/512)}')
ls -l "$DOOM_BIN" | awk '{printf("file size: %d bytes\n", $5)}'
if [[ -f $DOOM_BIN ]]; then
  cp $HOME/os-dev/hd80M.img $HOME/eehf-os/
  dd if=$DOOM_BIN of=$DD_OUT bs=512 \
    count=$SEC_CNT seek=800 conv=notrunc
fi

FONT_SEC_CNT=$(ls -l $FONT_PSF | awk '{printf("%d", ($5+511)/512)}')
if [[ -f $FONT_PSF ]]; then
  dd if=$FONT_PSF of=$DD_OUT bs=512 \
    count=$SEC_CNT seek=500 conv=notrunc
fi

DOOM_CNT=$(ls -l $DOOM_WAD | awk '{printf("%d", ($5+511)/512)}')
if [[ -f $DOOM_WAD ]]; then
  dd if=$DOOM_WAD of=$DD_OUT bs=512 \
    count=$DOOM_CNT seek=5000 conv=notrunc
fi
