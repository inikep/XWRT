#BUILD_SYSTEM = linux
#BUILD_ARCH = 32-bit

ifeq ($(BUILD_SYSTEM),linux)
    DEFINES	+= -D_POSIX_ENVIRONMENT_ -DZ_HAVE_UNISTD_H
    LZMA_FILES = LZMA/Common/C_FileIO.o
    LDFLAGS = -lrt
	GCC		= gcc
	GPP		= g++
else
    DEFINES	+= -D_WIN32_ENVIRONMENT_
    LZMA_FILES = LZMA/Windows/FileIO.o
#	DEFINES = -march=core2 -march=nocona -march=k8 -march=native
	LDFLAGS	= -lshell32 -lole32 -loleaut32 
	GCC		= gcc.exe
	GPP		= g++.exe
endif

ifneq ($(BUILD_ARCH),32-bit)
	DEFINES	+= -D__x86_64__
	LDFLAGS	+= -static  -L C:\Aplikacje\win-builds64\lib
else
	LDFLAGS	+= -static  -L C:\Aplikacje\win-builds32\lib
endif


#DEFINES		+= -DBENCH_REMOVE_XXX
DEFINES		+= -I.
CODE_FLAGS  = -Wno-unknown-pragmas -Wno-sign-compare -Wno-conversion # -Wall   # -Wextra
OPT_FLAGS   = -fomit-frame-pointer -fstrict-aliasing -fforce-addr -ffast-math

#BUILD_TYPE = debug

ifeq ($(BUILD_TYPE),debug)
	OPT_FLAGS += -g
else
	OPT_FLAGS += -O3
endif

CFLAGS = $(CODE_FLAGS) $(OPT_FLAGS) $(DEFINES)


all: xwrt

PPMD_FILES = PPMd/Model.o PPMd/PPMdlib.o 

ZLIB_FILES = zlib/adler32.o zlib/compress.o zlib/crc32.o zlib/deflate.o zlib/gzclose.o zlib/gzlib.o zlib/gzread.o
ZLIB_FILES += zlib/gzwrite.o zlib/infback.o zlib/inffast.o zlib/inflate.o zlib/inftrees.o zlib/trees.o zlib/uncompr.o zlib/zutil.o

LZMA_FILES += LZMA/LZMAlib.o LZMA/LZMAEncoder.o LZMA/LZMADecoder.o LZMA/7zip/Common/FileStreams.o
LZMA_FILES += LZMA/7zip/Common/StreamUtils.o LZMA/7zip/Common/OutBuffer.o LZMA/7zip/Common/InBuffer.o LZMA/StreamRam.o
LZMA_FILES += LZMA/Common/Alloc.o LZMA/7zip/Compress/LZ/LZInWindow.o LZMA/7zip/Compress/LZ/LZOutWindow.o
LZMA_FILES += LZMA/7zip/Compress/RangeCoder/RangeCoderBit.o LZMA/Common/CRC.o LZMA/Common/StringConvert.o LZMA/Common/NewHandler.o

xwrt: $(LZMA_FILES) $(ZLIB_FILES) src/XWRT.o src/Encoder.o src/Decoder.o src/MemBuffer.o src/Common.o $(PPMD_FILES)
	$(GPP) $^ -o $@ $(LDFLAGS)

.c.o:
	$(GCC) $(CFLAGS) $< -c -o $@

.cc.o:
	$(GPP) $(CFLAGS) $< -c -o $@

.cpp.o:
	$(GPP) $(CFLAGS) $< -c -o $@

clean:
	rm -f xwrt.exe xwrt LZMA/*.o LZMA/Windows/*.o LZMA/Common/*.o LZMA/7zip/Common/*.o LZMA/7zip/Compress/LZ/*.o LZMA/7zip/Compress/RangeCoder/*.o zlib/*.o src/*.o PPMd/*.o *.o
