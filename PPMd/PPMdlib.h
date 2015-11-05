#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

extern "C"
{

void PPMDlib_Init(int memSizeInMB);
void PPMDlib_Deinit();

// EncodeMemToFile requires allocatef memory from inBuffer-3 to inBuffer+inSize+3
int PPMDlib_EncodeMemToFile(int maxOrder,unsigned char* inBuffer, unsigned int inSize, FILE* outStream);

// DecodeFileToMem requires allocatef memory from outBuffer-3 to outBuffer+outSize+3
int PPMDlib_DecodeFileToMem(int maxOrder,FILE* inStream,unsigned char* outBuffer,unsigned int outSize);

}