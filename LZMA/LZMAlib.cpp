// LzmaLib.cpp

#include "StdAfx.h"

#include "./Common/MyWindows.h"
#include "./Common/MyInitGuid.h"

#include <stdio.h>

#if defined(_WIN32) || defined(OS2) || defined(MSDOS)
#include <fcntl.h>
#include <io.h>
#define MY_SET_BINARY_MODE(file) setmode(fileno(file),O_BINARY)
#else
#define MY_SET_BINARY_MODE(file)
#endif

#include "./7zip/Common/FileStreams.h"
#include "./7zip/Common/StreamUtils.h"

#include "./LZMADecoder.h"
#include "./LZMAEncoder.h"


#include "StreamRamSolid.h"


#ifdef _WIN32
bool g_IsNT = false;
static inline bool IsItWindowsNT()
{
  OSVERSIONINFO versionInfo;
  versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
  if (!::GetVersionEx(&versionInfo)) 
    return false;
  return (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
}
#endif





void LZMAlib_putc(unsigned char b, COutFileStream * outStream)
{
	outStream->Write(&b, 1, 0);
}

int LZMAlib_getc(CInFileStream * inStream)
{
	Byte b;
	inStream->Read(&b, 1, 0);

	return b;
}

unsigned int LZMAlib_GetFileSize(CInFileStream * inStream)
{
	Byte b;
	UInt32 outSize=0;

	for (int i=0; i<4; i++)
	{
		inStream->Read(&b, 1, 0);
	    outSize=outSize*256+b;
	}

//	printf("LZMAlib outSize=%d\n",outSize);

	return outSize;
}

unsigned int LZMAlib_GetInputFilePos(CInFileStream * inStream)
{
	UInt64 pos;
	inStream->File.GetPosition(pos);
    
	return (unsigned int) pos;
}

unsigned int LZMAlib_GetOutputFilePos(COutFileStream* outStream)
{
	UInt64 pos;
	outStream->File.GetPosition(pos);

	return (unsigned int) pos;
}

int LZMAlib_PrepareInputFile(char* inputName, CInFileStream *inStream)
{
//	CInFileStream *inStreamSpec=NULL;

//	inStreamSpec = new CInFileStream;
  //  inStream = inStreamSpec;
    return (inStream->Open(inputName));
}


int LZMAlib_PrepareOutputFile(char* outputName,COutFileStream *outStream)
{
 //   COutFileStream *outStreamSpec;
	
//	outStreamSpec = new COutFileStream;
//    outStream = outStreamSpec;
    return (outStream->Create(outputName, true));
}

int LZMAlib_CloseInputFile(CInFileStream * inStream)
{
	inStream->File.Close();

	return 0;
}

int LZMAlib_CloseOutputFile(COutFileStream* outStream)
{
	outStream->File.Close();

	return 0;
}


NCompress::NLZMA::CDecoder decoder;
int g_dictionarySize=23;

int LZMAlib_DecodeFileToMem(CInFileStream * inStream,unsigned char* outBuffer,UInt32& outSize)
{
	// prepare output
	CMyComPtr<ISequentialOutStream> outStream;

    COutStreamRam *outStreamSpec = new COutStreamRam;
    outStream = outStreamSpec;

    outStreamSpec->Init(outBuffer, outSize);

	UInt64 outSize64=outSize;


//	printf("g_dictionarySize=%d\n",g_dictionarySize);

	decoder.SetDictionarySize(g_dictionarySize);

	// decode
 	UInt64 before;
	inStream->File.GetPosition(before);

    if (decoder.Code(inStream, outStream, 0, &outSize64, 0) != S_OK)
    {
      fprintf(stderr, "Decoder error\n");
      return 1;
    }

	UInt32 pos=decoder.GetFilePos();

	inStream->Seek(pos+before,0,0);

	return 0;
}

NCompress::NLZMA::CEncoder encoder;

int LZMAlib_EncodeMemToFile(unsigned char* inBuffer, int inSize, COutFileStream* outStream)
{
	// prepare input
	CMyComPtr<ISequentialInStream> inStream;

    CInStreamRam *inStreamSpec = new CInStreamRam;
    inStream = inStreamSpec;

    inStreamSpec->Init(inBuffer, inSize);


//	LZMACoderProperties(encoderSpec);


	// write len
	Byte b;
//	printf("LZMAlib inSize=%d\n",inSize);
	b=inSize>>24; outStream->Write(&b, 1, 0);
	b=inSize>>16; outStream->Write(&b, 1, 0);
	b=inSize>>8; outStream->Write(&b, 1, 0);
	b=inSize; outStream->Write(&b, 1, 0);


//	CMyComPtr<ISequentialOutStream> oStream;
//	oStream=outStream;

	encoder.SetDictionarySize(g_dictionarySize);

    HRESULT result = encoder.Code(inStream, outStream, 0, 0, 0);
    if (result == E_OUTOFMEMORY)
    {
      fprintf(stdout, "\nError: Can not allocate memory\n");
      return 1;
    }   
    else if (result != S_OK)
    {
      fprintf(stdout, "\nEncoder error = %X\n", (unsigned int)result);
      return 1;
    }   

	return 0;
}

int LZMAlib_EncodeSolidMemToFile(unsigned char* inBuffer[], size_t inSize[], int count, COutFileStream* outStream)
{ 
	// prepare input
	CMyComPtr<ISequentialInStream> inStream;

    CInStreamRamSolid *inStreamSpec = new CInStreamRamSolid;
    inStream = inStreamSpec;

    inStreamSpec->Init(inBuffer, inSize, count);


//	LZMACoderProperties(encoderSpec);

	int i,insize;
	for (i=0,insize=0; i<count; i++)
		insize+=inSize[i];

	// write len
	Byte b;
//	printf("LZMAlib insize=%d count=%d\n",insize,count);
	b=insize>>24; outStream->Write(&b, 1, 0);
	b=insize>>16; outStream->Write(&b, 1, 0);
	b=insize>>8; outStream->Write(&b, 1, 0);
	b=insize; outStream->Write(&b, 1, 0);


//	CMyComPtr<ISequentialOutStream> oStream;
//	oStream=outStream;


	encoder.SetDictionarySize(g_dictionarySize);

    HRESULT result = encoder.Code(inStream, outStream, 0, 0, 0);
    if (result == E_OUTOFMEMORY)
    {
      fprintf(stdout, "\nError: Can not allocate memory\n");
      return 1;
    }   
    else if (result != S_OK)
    {
      fprintf(stdout, "\nEncoder error = %X\n", (unsigned int)result);
      return 1;
    }   

	return 0;
}

void LZMAlib_Init(int dictionarySizeInMB)
{
  #ifdef _WIN32
  g_IsNT = IsItWindowsNT();
  #endif

	if (dictionarySizeInMB>=64)
		g_dictionarySize=26;
	else
	if (dictionarySizeInMB>=32)
		g_dictionarySize=25;
	else
	if (dictionarySizeInMB>=16)
		g_dictionarySize=24;
	else
	if (dictionarySizeInMB>=8)
		g_dictionarySize=23;
	else
	if (dictionarySizeInMB>=4)
		g_dictionarySize=22;
	else
	if (dictionarySizeInMB>=2)
		g_dictionarySize=21;
	else
	if (dictionarySizeInMB>=1)
		g_dictionarySize=20;
	else
		g_dictionarySize=16;
}
