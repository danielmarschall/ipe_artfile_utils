
all: ipe_artfile_unpacker ipe_artfile_packer

ipe_artfile_unpacker: ipe_artfile_unpacker.c ipe_artfile_unpacker_ipe16.c ipe_artfile_unpacker_ipe32.c ipe16_lzw_decoder.c ipe32_lzw_decoder.c ipe16_bmpexport.c ipe32_bmpexport.c utils.c
	gcc -std=c99 -Wall -c ipe_artfile_unpacker.c -o ipe_artfile_unpacker.o
	gcc -std=c99 -Wall -c ipe_artfile_unpacker_ipe16.c -o ipe_artfile_unpacker_ipe16.o
	gcc -std=c99 -Wall -c ipe_artfile_unpacker_ipe32.c -o ipe_artfile_unpacker_ipe32.o
	gcc -std=c99 -Wall -c ipe16_lzw_decoder.c -o ipe16_lzw_decoder.o
	gcc -std=c99 -Wall -c ipe32_lzw_decoder.c -o ipe32_lzw_decoder.o
	gcc -std=c99 -Wall -c ipe16_bmpexport.c -o ipe16_bmpexport.o
	gcc -std=c99 -Wall -c ipe32_bmpexport.c -o ipe32_bmpexport.o
	gcc -std=c99 -Wall -c utils.c -o utils.o
	gcc -o ipe_artfile_unpacker ipe_artfile_unpacker.o ipe_artfile_unpacker_ipe16.o ipe_artfile_unpacker_ipe32.o ipe16_lzw_decoder.o ipe32_lzw_decoder.o ipe16_bmpexport.o ipe32_bmpexport.o utils.o
	rm *.o

ipe_artfile_packer: ipe_artfile_packer.c ipe_artfile_packer_ipe16_ba.c ipe_artfile_packer_ipe16_pip.c ipe_artfile_packer_ipe32.c ipe16_lzw_encoder.c ipe32_lzw_encoder.c ipe16_bmpimport.c ipe32_bmpimport.c utils.c
	gcc -std=c99 -Wall -c ipe_artfile_packer.c -o ipe_artfile_packer.o
	gcc -std=c99 -Wall -c ipe_artfile_packer_ipe16_ba.c -o ipe_artfile_packer_ipe16_ba.o
	gcc -std=c99 -Wall -c ipe_artfile_packer_ipe16_pip.c -o ipe_artfile_packer_ipe16_pip.o
	gcc -std=c99 -Wall -c ipe_artfile_packer_ipe32.c -o ipe_artfile_packer_ipe32.o
	gcc -std=c99 -Wall -c ipe16_lzw_encoder.c -o ipe16_lzw_encoder.o
	gcc -std=c99 -Wall -c ipe32_lzw_encoder.c -o ipe32_lzw_encoder.o
	gcc -std=c99 -Wall -c ipe16_bmpimport.c -o ipe16_bmpimport.o
	gcc -std=c99 -Wall -c ipe32_bmpimport.c -o ipe32_bmpimport.o
	gcc -std=c99 -Wall -c utils.c -o utils.o
	gcc -lm -o ipe_artfile_packer ipe_artfile_packer.o ipe_artfile_packer_ipe16_ba.o ipe_artfile_packer_ipe16_pip.o ipe_artfile_packer_ipe32.o ipe16_lzw_encoder.c ipe32_lzw_encoder.o ipe16_bmpimport.o ipe32_bmpimport.o utils.o
	rm *.o

clean:
	rm -f  *.o
	# TODO: if [ -f ... ] then rm
	rm ipe_artfile_packer
	rm ipe_artfile_unpacker
