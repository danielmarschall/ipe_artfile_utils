#!/bin/bash

DIR=$( dirname "$0" )
cd "$DIR"

gcc --std=c99 test_bitmap.c
gcc --std=c99 test_utils.c
gcc --std=c99 test_ipe16_artfile.c
gcc --std=c99 test_ipe16_bmpexport.c
gcc --std=c99 test_ipe16_bmpimport.c
gcc --std=c99 test_ipe16_lzw_encoder.c
gcc --std=c99 test_ipe16_lzw_decoder.c
gcc --std=c99 test_ipe32_artfile.c
gcc --std=c99 test_ipe32_bmpexport.c
gcc --std=c99 test_ipe32_bmpimport.c
gcc --std=c99 test_ipe32_lzw_encoder.c
gcc --std=c99 test_ipe32_lzw_decoder.c
gcc --std=c99 test_ipe_artfile_packer_ipe16_ba.c
gcc --std=c99 test_ipe_artfile_packer_ipe16_pip.c
gcc --std=c99 test_ipe_artfile_packer_ipe32.c
gcc --std=c99 test_ipe_artfile_unpacker_ipe16.c
gcc --std=c99 test_ipe_artfile_unpacker_ipe32.c

rm a.out

