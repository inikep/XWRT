// LzmaLib.h

#ifndef __LzmaLib_h
#define __LzmaLib_h

//#include <stdlib.h>
#include "./Common/MyCom.h"
#include "./7zip/IStream.h"
#include "./7zip/Common/FileStreams.h"

void LZMAlib_Init(int dictionarySizeInMB);

int LZMAlib_EncodeMemToFile(unsigned char* inBuffer, int inSize, COutFileStream* outStream);
int LZMAlib_EncodeSolidMemToFile(unsigned char* inBuffer[], size_t inSize[], int count, COutFileStream* outStream);
int LZMAlib_PrepareOutputFile(char* outputName,COutFileStream *outStream);
int LZMAlib_CloseOutputFile(COutFileStream* outStream);

int LZMAlib_DecodeFileToMem(CInFileStream * inStream,unsigned char* outBuffer,UInt32& outSize);
int LZMAlib_PrepareInputFile(char* inputName, CInFileStream *inStream);
int LZMAlib_CloseInputFile(CInFileStream * inStream);

unsigned int LZMAlib_GetFileSize(CInFileStream * inStream);
unsigned int LZMAlib_GetInputFilePos(CInFileStream * inStream);
unsigned int LZMAlib_GetOutputFilePos(COutFileStream* outStream);
void LZMAlib_putc(unsigned char b, COutFileStream * outStream);
int LZMAlib_getc(CInFileStream * inStream);

#endif
