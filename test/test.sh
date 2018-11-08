#!/bin/bash

DIR=$( dirname "$0" )
cd "$DIR"

if [ -d out_test ]; then
	rm -Rf out_test
fi
if [ -f eraser_test.art ]; then
	rm -f eraser_test.art
fi

# ---

if [ -f ba_test.art ]; then
	rm -f ba_test.art
fi
../ipe_artfile_packer -v -i ba_test -o ba_test.art -t ba
if [ -f ba_test.art ]; then
	mkdir out_test
	../ipe_artfile_unpacker -v -i ba_test.art -o out_test
fi
diff ba_test/MENU.bmp out_test/MENU.bmp
RES=$?
#if [ $RES -ne 0 ]; then
#	exit 1
#fi
echo "DIFF Result (BA): $RES"
if [ -d out_test ]; then
	rm -Rf out_test
fi
if [ -f ba_test.art ]; then
	rm -f ba_test.art
fi
echo "------------------------"

../ipe_artfile_packer -v -i pip_test -o pip_test.art -t pip
if [ -f pip_test.art ]; then
	mkdir out_test
	../ipe_artfile_unpacker -v -i pip_test.art -o out_test
fi
diff pip_test/CCES2S.bmp out_test/CCES2S.bmp
RES=$?
#if [ $RES -ne 0 ]; then
#	exit
#fi
echo "DIFF Result (PiP): $RES"
if [ -d out_test ]; then
	rm -Rf out_test
fi
if [ -f pip_test.art ]; then
	rm -f pip_test.art
fi
echo "------------------------"

../ipe_artfile_packer -v -i eraser_test -o eraser_test.art -t eraser
if [ -f eraser_test.art ]; then
	mkdir out_test
	../ipe_artfile_unpacker -v -i eraser_test.art -o out_test
fi
diff eraser_test/CHRBDOSS.bmp out_test/CHRBDOSS.bmp
RES=$?
#if [ $RES -ne 0 ]; then
#	exit
#fi
echo "DIFF Result (Eraser): $RES"
if [ -d out_test ]; then
	rm -Rf out_test
fi
if [ -f eraser_test.art ]; then
	rm -f eraser_test.art
fi
