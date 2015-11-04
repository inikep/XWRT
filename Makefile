#BUILD_SYSTEM = linux
BUILD_ARCH = 64-bit

ifeq ($(BUILD_SYSTEM),linux)
	LDFLAGS = -lrt
	GCC		= gcc
	GPP		= g++
else
#	DEFINES = -march=core2 -march=nocona -march=k8 -march=native
	LDFLAGS	= -lshell32 -lole32 -loleaut32 
	GCC		= gcc.exe
	GPP		= g++.exe
endif


ifeq ($(BUILD_ARCH),64-bit)
	DEFINES	+= -D__x86_64__
	LDFLAGS	+= -static  -L C:\Aplikacje\win-builds64\lib
else
	LDFLAGS	+= -static  -L C:\Aplikacje\win-builds32\lib
endif


#DEFINES		+= -DBENCH_REMOVE_XXX
DEFINES		+= -I.
CODE_FLAGS  = -Wno-unknown-pragmas -Wno-sign-compare -Wno-conversion -Wall   # -Wextra
OPT_FLAGS   = -fomit-frame-pointer -fstrict-aliasing -fforce-addr -ffast-math

#BUILD_TYPE = debug

ifeq ($(BUILD_TYPE),debug)
	OPT_FLAGS += -g
else
	OPT_FLAGS += -O3
endif

CFLAGS = $(CODE_FLAGS) $(OPT_FLAGS) $(DEFINES)


all: xwrt


xwrt: XWRT/XWRT.o XWRT/Encoder.o XWRT/Decoder.o XWRT/MemBuffer.o XWRT/Common.o
	$(GPP) $^ -o $@ $(LDFLAGS)

.c.o:
	$(GCC) $(CFLAGS) $< -c -o $@

.cc.o:
	$(GPP) $(CFLAGS) $< -c -o $@

.cpp.o:
	$(GPP) $(CFLAGS) $< -c -o $@

clean:
	rm -f XWRT/*.o *.o *.exe
