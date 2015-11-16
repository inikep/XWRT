#if !defined membuffer_h_included
#define membuffer_h_included

#pragma warning(disable:4786)

#include <stdio.h>
#include <vector>
#include <string>
#include <map>

#define OUTPUT_BUFFER_MIN_SIZE 10240


// Input/Output using dynamic memory allocation
class CMemoryBuffer
{
public:
	CMemoryBuffer(std::string mname="");
	~CMemoryBuffer();

	void OutTgtByte( unsigned char c );
	int InpSrcByte( void );
	void UndoSrcByte( void ); 
	inline int Size();
	inline int Allocated(); 
	inline void AllocSrcBuf( unsigned int len );
	inline void Clear();
	inline void SetSrcBuf( unsigned char* SrcBuf, unsigned int len );

	static unsigned int memsize;
	unsigned char* TargetBuf;
	unsigned char* SourceBuf;
	unsigned int SrcLen, TgtLen;
	unsigned int SrcPtr, TgtPtr;
	std::string name;

private:
	inline void AllocTgtBuf( unsigned int len = OUTPUT_BUFFER_MIN_SIZE );
	inline void ReallocTgtBuf(unsigned int len);
};

class CContainers
{
public:
	CContainers();
	void prepareMemBuffers();
	void writeMemBuffers(int preprocFlag, int PPMDlib_order, int comprLevel, Encoder* PAQ_encoder, unsigned char* zlibBuffer,COutFileStream* outStream);
	void readMemBuffers(int preprocFlag, int maxMemSize, int PPMDlib_order, Encoder* PAQ_encoder, unsigned char* zlibBuffer,CInFileStream* inStream);
	void freeMemBuffers(bool freeMem);
	void selectMemBuffer(unsigned char* s,int s_size, bool full_path);
	void MemBufferPopBack();

	CMemoryBuffer *memout,*memout_letters,*memout_ip,*memout_hm,*memout_hms;
	CMemoryBuffer *memout_words,*memout_words2,*memout_words3,*memout_words4;
	CMemoryBuffer *memout_num,*memout_numb,*memout_numc,*memout_num2,*memout_num2b,*memout_num2c,*memout_num3,*memout_num3b,*memout_num3c,*memout_num4,*memout_num4b,*memout_num4c;
	CMemoryBuffer *memout_year,*memout_pages,*memout_remain,*memout_date,*memout_date2,*memout_date3,*memout_time,*memout_remain2;
	unsigned char *bigBuffer;	

private:
	std::vector<CMemoryBuffer*> mem_stack;
	std::map<std::string,CMemoryBuffer*> memmap;
};

#ifdef USE_ZLIB_LIBRARY

int Zlib_decompress(FILE* file,Byte *compr, uLong comprLen,Byte *uncompr, uLong uncomprLen,int& totalIn);
int Zlib_compress(FILE* fileout,Byte *uncompr, uLong uncomprLen,Byte *compr, uLong comprLen, int comprLevel);

#endif

#endif
