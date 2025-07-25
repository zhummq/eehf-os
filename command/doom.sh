#!/bin/bash

DOOM_BIN="$HOME/doomgeneric/doomgeneric/doomgeneric"
DOOM_WAD="$HOME/freedoom-0.13.0/freedoom1.wad"
DD_OUT="$HOME/eehf-os/hd3M.img"

SEC_CNT=$(ls -l $DOOM_BIN | awk '{printf("%d", ($5+511)/512)}')
ls -l "$DOOM_BIN" | awk '{printf("file size: %d bytes\n", $5)}'
if [[ -f $DOOM_BIN ]]; then
  dd if=$DOOM_BIN of=$DD_OUT bs=512 \
    count=$SEC_CNT seek=800 conv=notrunc
fi
