#include "PPMd.h"
#include "PPMdType.h"
#include "PPMdlib.h"


void printStatus(int add_read,int add_written,int encoding);
size_t last_ftell = 0;

void _STDCALL PrintInfo(PRIME_STREAM* DecodedFile,PRIME_STREAM* EncodedFile)
{
#if 1
    if (EncodedFile->compr)
        printStatus(0, ftell(EncodedFile->f)-last_ftell, EncodedFile->compr);
    else
        printStatus(ftell(EncodedFile->f)-last_ftell, 0, EncodedFile->compr);
    last_ftell = ftell(EncodedFile->f);
#else
    char WrkStr[320];
	UINT NDec=(DecodedFile->compr)?(DecodedFile->startCount - DecodedFile->Count):ftell(EncodedFile->f);
    NDec += (NDec == 0);
	UINT NEnc=(EncodedFile->compr)?ftell(EncodedFile->f):(DecodedFile->startCount - DecodedFile->Count);
    UINT n1=(8U*NEnc)/NDec;
    UINT n2=(100U*(8U*NEnc-NDec*n1)+NDec/2U)/NDec;
    if (n2 == 100) { n1++;                  n2=0; }
    UINT UsedMemory=GetUsedMemory() >> 10;
    UINT m1=UsedMemory >> 10;
    UINT m2=(10U*(UsedMemory-(m1 << 10))+(1 << 9)) >> 10;
    if (m2 == 10) { m1++;                   m2=0; }

	sprintf(WrkStr,"%8d ->%8d, %1d.%03d bpc, used %dMB", NDec,NEnc,n1,n2,m1);
    printf("%-79.79s\r",WrkStr);            
#endif
	fflush(stdout);
}


void PPMDlib_Init(int memSizeInMB)
{
    if ( !StartSubAllocator(memSizeInMB) ) {
        printf("Not enough memory!\n");                    
		exit(-1);
    }
}

void PPMDlib_Deinit()
{
	StopSubAllocator(); 
}



int PPMDlib_EncodeMemToFile(int maxOrder,unsigned char* inBuffer, unsigned int inSize, FILE* outStream)
{
    PRIME_STREAM in(true, inBuffer, inSize, NULL);
    PRIME_STREAM out(true, NULL, 0, outStream);

    last_ftell = ftell(outStream);

	EncodeFile(&out, &in, maxOrder, FALSE);

	return 1;
};

int PPMDlib_DecodeFileToMem(int maxOrder, FILE* inStream, unsigned char* outBuffer, unsigned int outSize)
{
    PRIME_STREAM in(false, NULL, 0, inStream);
    PRIME_STREAM out(false, outBuffer, outSize, NULL);

    last_ftell = ftell(inStream);

	DecodeFile(&out, &in, maxOrder, FALSE);
    
    return out.startCount - out.Count;
}

/*
int main8(int argc, const char **argv)
{
	FILE *fpOut;
	unsigned char* PPMDmem;
	unsigned int PPMDsize=0;
    int MaxOrder=8, SASize=32;

	PPMDlib_Init(MaxOrder,SASize);


	fpOut=fopen("ppmvc_out","wb");
	if (fpOut==NULL)
		exit(1);

	unsigned char test[]="\000\000\0001211112345678901234567890123456-78901234567890123456789012345+678901123456789012345678901_2345678901234567890123*45678901234567890112345%6789012345678901234567890123456789$01234567890123456789011234567890123#45678901234567890123456789012345678901234567@89011234567890123456789012345678901234!\000\000\000";

	PPMDmem=test+3;
	PPMDsize=sizeof(test)-6;

	PPMDlib_EncodeMemToFile(PPMDmem,PPMDsize,fpOut);

	unsigned char test2[]="\000\000\00091211112345678901234567890123456-78901234567890123456789012345+678901123456789012345678901_2345678901234567890123*45678901234567890112345%6789012345678901234567890123456789$01234567890123456789011234567890123#45678901234567890123456789012345678901234567@89011234567890123456789012345678901234!\000\000\000";

	PPMDmem=test2+3;
	PPMDsize=sizeof(test2)-6;

	PPMDlib_EncodeMemToFile(PPMDmem,PPMDsize,fpOut);

	fclose(fpOut);



	fpOut=fopen("ppmvc_out","rb");
	if (fpOut==NULL)
		exit(1);

	unsigned char testd[1024];
	memset(testd,0,sizeof(testd));

	PPMDmem=testd+3;
	PPMDsize=sizeof(testd)-6;

	int size=PPMDlib_DecodeFileToMem(fpOut,PPMDmem,PPMDsize);

	printf("PPMDsize=%d size=%d PPMDmem=%s\n",PPMDsize,size,PPMDmem);


	memset(testd,0,sizeof(testd));
	PPMDmem=testd+3;
	PPMDsize=sizeof(testd)-6;

	size=PPMDlib_DecodeFileToMem(fpOut,PPMDmem,PPMDsize);

	printf("PPMDsize=%d size=%d PPMDmem=%s\n",PPMDsize,size,PPMDmem);

	fclose(fpOut);


	PPMDlib_Deinit();

	return 0;
}
*/