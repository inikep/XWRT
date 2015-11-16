#include "XWRT.h"
#include "MemBuffer.h"
#include <stdlib.h> 
#include <memory.h>
#include "Common.h"

unsigned int CMemoryBuffer::memsize=0;

CMemoryBuffer::CMemoryBuffer(std::string mname) 
{ 
	name=mname;
	Clear(); 
	AllocTgtBuf(); 
};

CMemoryBuffer::~CMemoryBuffer() 
{ 
	if (TargetBuf)
		free(TargetBuf-3);
	
	if (SourceBuf)
		free(SourceBuf);
};

inline void CMemoryBuffer::Clear()
{
	TargetBuf=NULL; SourceBuf=NULL; SrcPtr=0; TgtPtr=0; SrcLen=0; TgtLen=0;
}

inline int CMemoryBuffer::Size() 
{
	return TgtPtr;
}

inline int CMemoryBuffer::Allocated() 
{
	return TgtLen;
}

void CMemoryBuffer::OutTgtByte( unsigned char c ) 
{ 
	memsize++;

	*(TargetBuf+(TgtPtr++))=c; 
	if (TgtPtr>TgtLen-1)
	{
		if (TgtLen > (1<<19))  // 512 KB
			ReallocTgtBuf(TgtLen+(1<<19));
		else
			ReallocTgtBuf(TgtLen*2);
	}
}

int CMemoryBuffer::InpSrcByte( void ) 
{
	memsize++;

	if (SrcPtr>=SrcLen)
		return EOF;

	return *(SourceBuf+(SrcPtr++)); 
}

void CMemoryBuffer::UndoSrcByte( void ) 
{
	memsize--;
	SrcPtr--;
}

inline void CMemoryBuffer::SetSrcBuf( unsigned char* SrcBuf, unsigned int len )
{
	SrcLen = len;
	SourceBuf = SrcBuf;
}

inline void CMemoryBuffer::AllocSrcBuf( unsigned int len )
{
	SrcLen = len;
	SourceBuf = (unsigned char*) malloc(SrcLen);
	if (SourceBuf==NULL)
		OUT_OF_MEMORY();

//	fread( SourceBuf, 1, SrcLen, SourceFile );
}
	
inline void CMemoryBuffer::AllocTgtBuf( unsigned int len )
{
	TgtLen = len;
	TargetBuf = (unsigned char*) malloc(len+6);
	if (TargetBuf==NULL)
		OUT_OF_MEMORY();
	TargetBuf += 3;
}

inline void CMemoryBuffer::ReallocTgtBuf(unsigned int len)
{
	unsigned char* NewTargetBuf = (unsigned char*) malloc(len+6);

	if (NewTargetBuf==NULL)
		OUT_OF_MEMORY();

	NewTargetBuf += 3;
	memcpy(NewTargetBuf,TargetBuf,min(TgtPtr,len));
	TgtLen = len;
	delete(TargetBuf-3);
	TargetBuf=NewTargetBuf;
}

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stdout, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}


CContainers::CContainers() : bigBuffer(NULL) {};

void CContainers::prepareMemBuffers()
{
	memout=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p("!data",memout);
	memmap.insert(p);

	memout_words=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p2("!!!words",memout_words);
	memmap.insert(p2);

	memout_letters=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p3("!!letters",memout_letters);
	memmap.insert(p3);

	memout_num=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p4("!num",memout_num);
	memmap.insert(p4);

	memout_year=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p5("!year",memout_year);
	memmap.insert(p5);

	memout_date=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p6("!date",memout_date);
	memmap.insert(p6);

	memout_words2=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p7("!!!words2",memout_words2);
	memmap.insert(p7);

	memout_words3=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p8("!!!words3",memout_words3);
	memmap.insert(p8);

	memout_words4=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p9("!!!words4",memout_words4);
	memmap.insert(p9);

	memout_pages=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p10("!pages",memout_pages);
	memmap.insert(p10);

	memout_num2=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p11("!num2",memout_num2);
	memmap.insert(p11);

	memout_num3=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p12("!num3",memout_num3);
	memmap.insert(p12);

	memout_num4=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p13("!num4",memout_num4);
	memmap.insert(p13);

	memout_remain=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p14("!remain",memout_remain);
	memmap.insert(p14);

	memout_date2=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p15("!date2",memout_date2);
	memmap.insert(p15);

	memout_date3=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p16("!date3",memout_date3);
	memmap.insert(p16);

	memout_num2b=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p17("!num2b",memout_num2b);
	memmap.insert(p17);

	memout_num3b=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p18("!num3b",memout_num3b);
	memmap.insert(p18);

	memout_num4b=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p19("!num4b",memout_num4b);
	memmap.insert(p19);

	memout_numb=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p20("!numb",memout_numb);
	memmap.insert(p20);

	memout_num2c=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p21("!num2c",memout_num2c);
	memmap.insert(p21);

	memout_num3c=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p22("!num3c",memout_num3c);
	memmap.insert(p22);

	memout_num4c=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p23("!num4c",memout_num4c);
	memmap.insert(p23);

	memout_numc=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p24("!numc",memout_numc);
	memmap.insert(p24);

	memout_time=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p25("!time",memout_time);
	memmap.insert(p25);

	memout_remain2=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p26("!remain2",memout_remain2);
	memmap.insert(p26);

	memout_ip=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p27("!ip",memout_ip);
	memmap.insert(p27);

	memout_hm=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p28("!hm",memout_hm);
	memmap.insert(p28);

	memout_hms=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p29("!hms",memout_hms);
	memmap.insert(p29);
}

void CContainers::writeMemBuffers(int preprocFlag, int PPMDlib_order, int comprLevel, Encoder* PAQ_encoder, unsigned char* zlibBuffer,COutFileStream* outStream)
{
	std::map<std::string,CMemoryBuffer*>::iterator it;

	int fileLen=0;
	int len=0;
	int lenCompr=0;
	int allocated=0;

#ifdef USE_LZMA_LIBRARY
	if (IF_OPTION(OPTION_LZMA))
	{
		Byte **Data;
		size_t* Size;
		int count;
		
		count=(int)memmap.size();
		Size=new size_t[count];
		Data=(unsigned char**) malloc(sizeof(unsigned char*)*count);
		if (Data==NULL)
			OUT_OF_MEMORY();
	
		int i=0;
		for (it=memmap.begin(); it!=memmap.end(); it++)
		{
			CMemoryBuffer* b=it->second;
			fileLen=b->Size();
			
			if (fileLen>0)
			{
				Size[i]=fileLen;
				Data[i]=b->TargetBuf;
				i++;
				
				PUTC((int)it->first.size());
				for (int i=0; i<(int)it->first.size(); i++)
					PUTC(it->first[i]);
				
				PUTC(fileLen>>24);
				PUTC(fileLen>>16);
				PUTC(fileLen>>8);
				PUTC(fileLen);	
			}
		}
		
		PUTC(0);
		
		count=i;
		int last=LZMAlib_GetOutputFilePos(outStream);
		LZMAlib_EncodeSolidMemToFile(Data,Size,count,outStream);
		printStatus(0,LZMAlib_GetOutputFilePos(outStream)-last,true);
	}
	else
#endif
	{
		for (it=memmap.begin(); it!=memmap.end(); it++)
		{
			CMemoryBuffer* b=it->second;
			fileLen=b->Size();
			
			PRINT_CONTAINERS(("cont=%s fileLen=%d\n",it->first.c_str(),fileLen));

			if (fileLen>0)
			{
				allocated+=b->Allocated();
				len+=fileLen;
					
				if (!IF_OPTION(OPTION_PAQ))
				{	
					PUTC((int)it->first.size());
					for (int i=0; i<(int)it->first.size(); i++)
						PUTC(it->first[i]);
				}

#ifdef USE_PAQ_LIBRARY
				if (IF_OPTION(OPTION_PAQ))
				{
					int i;
					PAQ_encoder->compress((int)it->first.size());
					for (i=0; i<(int)it->first.size(); i++)
						PAQ_encoder->compress(it->first[i]);

					PAQ_encoder->compress(fileLen>>24);
					PAQ_encoder->compress(fileLen>>16);
					PAQ_encoder->compress(fileLen>>8);
					PAQ_encoder->compress(fileLen);

					int last=ftell(XWRT_fileout);
					for (i=0; i<fileLen; )
					{
						PAQ_encoder->compress(b->TargetBuf[i++]);
						if (i%102400==0)
						{
							printStatus(0,ftell(XWRT_fileout)-last,true);
							last=ftell(XWRT_fileout);
						}
					}
					printStatus(0,ftell(XWRT_fileout)-last,true);
				}
				else
#endif
#ifdef USE_PPMD_LIBRARY
				if (IF_OPTION(OPTION_PPMD))
				{
					PPMDlib_EncodeMemToFile(PPMDlib_order,b->TargetBuf,fileLen,XWRT_fileout);
				}
				else
#endif
#ifdef USE_ZLIB_LIBRARY
				if (IF_OPTION(OPTION_ZLIB))
				{

					int l=Zlib_compress(XWRT_fileout,b->TargetBuf,fileLen,zlibBuffer,ZLIB_BUFFER_SIZE,comprLevel);
					lenCompr+=l;
				}
				else
#endif
				{
					PUTC(fileLen>>24);
					PUTC(fileLen>>16);
					PUTC(fileLen>>8);
					PUTC(fileLen);
//					fprintf(fileout, "%c%c%c%c", fileLen>>24, fileLen>>16, fileLen>>8, fileLen);

//					for (unsigned int i=0; i<it->second->TgtPtr; i++)
//						PUTC(it->second->TargetBuf[i]);
//					fwrite(it->second->TargetBuf,1,it->second->TgtPtr,XWRT_fileout);
					fwrite_fast(it->second->TargetBuf,it->second->TgtPtr,XWRT_fileout);

					lenCompr+=fileLen;

					printStatus(0,fileLen,true);
				}
				
			}
		}
		
		if (!IF_OPTION(OPTION_PAQ))
			PUTC(0)
#ifdef USE_PAQ_LIBRARY
		else
			PAQ_encoder->compress(0);
#endif
	}

	PRINT_DICT(("dataSize=%d compr=%d allocated=%d\n",len,lenCompr,allocated));

	freeMemBuffers(true);
	prepareMemBuffers();
}

void CContainers::readMemBuffers(int preprocFlag, int maxMemSize, int PPMDlib_order, Encoder* PAQ_encoder, unsigned char* zlibBuffer,CInFileStream* inStream)
{
	unsigned char* buf=NULL;
	unsigned int bufLen=0;
	unsigned int fileLen;
	unsigned int ui;
	int len=0;
	int lenCompr=0;
	int i,c;
	unsigned char s[STRING_MAX_SIZE];

	if (IF_OPTION(OPTION_ZLIB) || IF_OPTION(OPTION_LZMA) || IF_OPTION(OPTION_PPMD) || IF_OPTION(OPTION_PAQ))
	{
		bufLen=maxMemSize+10240;
		if (bigBuffer==NULL)
		{
			bigBuffer=(unsigned char*)malloc(bufLen);
			if (bigBuffer==NULL)
				OUT_OF_MEMORY();
		}
		buf=bigBuffer+3;
		bufLen-=6;
		freeMemBuffers(false);
	}
	else
		freeMemBuffers(true);


	prepareMemBuffers();
	CMemoryBuffer* memout_tmp=NULL;

 	while (true)
	{	
		if (!IF_OPTION(OPTION_PAQ))
		{
			GETC(i);

			if (i<=0)
				break;

			for (c=0; c<i; c++)
				GETC(s[c]);
		}
		else
		{
#ifdef USE_PAQ_LIBRARY
			i=PAQ_encoder->decompress();

			if (i<=0)
				break;

			for (c=0; c<i; c++)
				s[c]=PAQ_encoder->decompress();
#endif
		}

		std::string str;
		str.append((char*)s,i);

		PRINT_CONTAINERS(("cont=%s\n",str.c_str()));

		if (str=="!data")
			memout_tmp=memout;
		else
		if (str=="!!letters")
			memout_tmp=memout_letters;
		else
		if (str=="!!!words")
			memout_tmp=memout_words;
		else
		if (str=="!!!words2")
			memout_tmp=memout_words2;
		else
		if (str=="!!!words3")
			memout_tmp=memout_words3;
		else
		if (str=="!!!words4")
			memout_tmp=memout_words4;
		else
		if (str=="!num")
			memout_tmp=memout_num;
		else
		if (str=="!numb")
			memout_tmp=memout_numb;
		else
		if (str=="!numc")
			memout_tmp=memout_numc;
		else
		if (str=="!num2")
			memout_tmp=memout_num2;
		else
		if (str=="!num2b")
			memout_tmp=memout_num2b;
		else
		if (str=="!num2c")
			memout_tmp=memout_num2c;
		else
		if (str=="!num3")
			memout_tmp=memout_num3;
		else
		if (str=="!num3b")
			memout_tmp=memout_num3b;
		else
		if (str=="!num3c")
			memout_tmp=memout_num3c;
		else
		if (str=="!num4")
			memout_tmp=memout_num4;
		else
		if (str=="!num4b")
			memout_tmp=memout_num4b;
		else
		if (str=="!num4c")
			memout_tmp=memout_num4c;
		else
		if (str=="!year")
			memout_tmp=memout_year;
		else
		if (str=="!date")
			memout_tmp=memout_date;
		else
		if (str=="!date2")
			memout_tmp=memout_date2;
		else
		if (str=="!date3")
			memout_tmp=memout_date3;
		else
		if (str=="!pages")
			memout_tmp=memout_pages;
		else
		if (str=="!remain")
			memout_tmp=memout_remain;
		else
		if (str=="!time")
			memout_tmp=memout_time;
		else
		if (str=="!remain2")
			memout_tmp=memout_remain2;
		else
		if (str=="!ip")
			memout_tmp=memout_ip;
		else
		if (str=="!hm")
			memout_tmp=memout_hm;
		else
		if (str=="!hms")
			memout_tmp=memout_hms;
		else
		{
			memout_tmp=new CMemoryBuffer(str);
			std::pair<std::string,CMemoryBuffer*> p(str,memout_tmp);
			memmap.insert(p);
		}


		if (IF_OPTION(OPTION_LZMA))
		{
			for (i=0, fileLen=0; i<4; i++)
			{
				GETC(c);
				fileLen=fileLen*256+c;
			}	
		
			memout_tmp->SetSrcBuf(buf, fileLen);
			buf+=fileLen;
			bufLen-=fileLen;
			len+=fileLen;
			lenCompr+=1;
		}
		else
		{
#ifdef USE_PAQ_LIBRARY
			if (IF_OPTION(OPTION_PAQ))
			{
				for (i=0, fileLen=0; i<4; i++)
					fileLen=fileLen*256+PAQ_encoder->decompress();

				int last=ftell(XWRT_file);
				for (ui=0; ui<fileLen; )
				{
					buf[ui++]=PAQ_encoder->decompress();
					if (ui%102400==0)
					{
						printStatus(ftell(XWRT_file)-last,0,false);
						last=ftell(XWRT_file);
					}
				}

				printStatus(ftell(XWRT_file)-last,0,false);
			
				memout_tmp->SetSrcBuf(buf, fileLen);
				buf+=fileLen+3;
				len+=fileLen;
				bufLen-=fileLen+3;
				lenCompr+=1;
			}
			else
#endif
#ifdef USE_PPMD_LIBRARY
			if (IF_OPTION(OPTION_PPMD))
			{
				fileLen=PPMDlib_DecodeFileToMem(PPMDlib_order,XWRT_file,buf,bufLen);

				memout_tmp->SetSrcBuf(buf, fileLen);
				buf+=fileLen+3;
				len+=fileLen;
				bufLen-=fileLen+3;
				lenCompr+=1;
			}
			else
#endif
#ifdef USE_ZLIB_LIBRARY
			if (IF_OPTION(OPTION_ZLIB))
			{
				fileLen=Zlib_decompress(XWRT_file,zlibBuffer,ZLIB_BUFFER_SIZE,buf,bufLen,i);
				
				PRINT_CONTAINERS(("fileLen=%d\n",fileLen));

				memout_tmp->SetSrcBuf(buf, fileLen);
				buf+=fileLen;
				len+=fileLen;
				lenCompr+=i;
				bufLen-=fileLen;			
			}
			else
#endif
			{
				int c;
				for (i=0, fileLen=0; i<4; i++)
				{
					GETC(c);
					fileLen=fileLen*256+c;
				}
		
				len+=fileLen;
				lenCompr+=fileLen;
				memout_tmp->AllocSrcBuf(fileLen);

//				for (unsigned int i=0; i<memout_tmp->SrcLen; i++)
//					GETC(memout_tmp->SourceBuf[i]);
//				fread(memout_tmp->SourceBuf,1,memout_tmp->SrcLen,XWRT_file);
				fread_fast(memout_tmp->SourceBuf,memout_tmp->SrcLen,XWRT_file);

				printStatus(fileLen,0,false);
			}
		}

	}

#ifdef USE_LZMA_LIBRARY
	if (IF_OPTION(OPTION_LZMA))
	{
		unsigned int newLen=LZMAlib_GetFileSize(inStream);
		bufLen=maxMemSize+10240;
		bufLen-=6;
		buf=bigBuffer+3;

		if (newLen>bufLen)
			OUT_OF_MEMORY();

		int last=LZMAlib_GetInputFilePos(inStream);
		LZMAlib_DecodeFileToMem(inStream,buf,newLen);
		printStatus(LZMAlib_GetInputFilePos(inStream)-last,0,false);
	}
#endif

	PRINT_DICT(("readMemBuffers() dataSize=%d compr=%d allocated=%d\n",len,lenCompr,maxMemSize+10240));
}

void CContainers::freeMemBuffers(bool freeMem)
{
	mem_stack.clear();

	std::map<std::string,CMemoryBuffer*>::iterator it;

	for (it=memmap.begin(); it!=memmap.end(); it++)
	{
		if (!freeMem)
			it->second->Clear();
		delete(it->second);			
	}

	memmap.clear();
}

#ifdef USE_ZLIB_LIBRARY

int Zlib_compress(FILE* fileout,Byte *uncompr, uLong uncomprLen,Byte *compr, uLong comprLen, int comprLevel)
{
    z_stream c_stream; /* compression stream */
    int err;
	int outSize=0;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, comprLevel);

    CHECK_ERR(err, "deflateInit");


    c_stream.next_out = compr;
    c_stream.avail_out = (uInt)comprLen;
    c_stream.next_in = uncompr;
    c_stream.avail_in = (uInt)uncomprLen;

	do
	{
	    err = deflate(&c_stream, Z_FINISH);
		fprintf(fileout, "%c%c%c", (c_stream.total_out>>16)%256, (c_stream.total_out>>8)%256, (c_stream.total_out)%256);
//		fwrite(compr, 1, c_stream.total_out, fileout);
		fwrite_fast(compr, c_stream.total_out, fileout);
		outSize+=c_stream.total_out;
		printStatus(0,c_stream.total_out,true);
		c_stream.next_out = compr;
	    c_stream.avail_out = (uInt)comprLen;
		c_stream.total_out = 0;
	}
	while (err == Z_OK);


    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");

	return outSize;
}

int Zlib_decompress(FILE* file,Byte *compr, uLong comprLen,Byte *uncompr, uLong uncomprLen,int& totalIn)
{
    int err;
    z_stream d_stream; /* decompression stream */
 
    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

	int i,fileLen;
	while (true)
	{
		for (i=0, fileLen=0; i<3; i++)
		{
			int c=getc(file);
			fileLen=fileLen*256+c;
		}

		if ((int)comprLen<fileLen)
		{
			VERBOSE(("error: comprLen(%d)<fileLen(%d)\n",comprLen,fileLen));
			OUT_OF_MEMORY();
		}

		fread_fast((unsigned char*)compr,fileLen,file);

//		/*size_t rd=*/fread(compr,1,fileLen,file);
		d_stream.next_in  = compr;
		d_stream.avail_in = fileLen; //(int) rd;

		err = inflate(&d_stream, Z_FINISH);

//        if (err == Z_STREAM_END) 
        if (err != Z_BUF_ERROR) 
			break; 
    }


    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

	totalIn=d_stream.total_in;
	printStatus(d_stream.total_in,0,false);
	return d_stream.total_out;
} 

#endif

void CContainers::selectMemBuffer(unsigned char* s,int s_size, bool full_path)
{
	while ((s[0]==' ' || s[0]=='<') && s_size>0)
	{
		s++;
		s_size--;
	}
	
	
	if (s_size>0 && s[s_size-1]=='>')
		s_size--;
	
	if (s_size==0)
		return;
	
	
	std::string str;
	str.append((char*)s,s_size);
	
	static std::map<std::string,CMemoryBuffer*>::iterator it;
	
	it=memmap.find(str);
	
	mem_stack.push_back(memout);

	PRINT_STACK(("selectMemBuffer mem_stack=%d str=%s\n",mem_stack.size(),str.c_str()));

	if (it!=memmap.end())
		memout=it->second;
	else
	{
		memout=new CMemoryBuffer(str);
		std::pair<std::string,CMemoryBuffer*> p(str,memout);
		memmap.insert(p);
	}

	return;
}


void CContainers::MemBufferPopBack()
{
	if (mem_stack.size()>0)
	{
		PRINT_STACK(("\nmemout3 pop_back\n"));
		memout=mem_stack.back();
		mem_stack.pop_back();
	}
}
