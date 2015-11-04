#include "XWRT.h"
#include "Decoder.h"
#include <time.h>

extern int g_fileLenMB;

XWRT_Decoder::XWRT_Decoder() : utf8pos(0), WRTd_s(&WRTd_data[0])
{
	putcBuffer=new unsigned char[mFileBufferSize];
	zlibBuffer=new unsigned char[ZLIB_BUFFER_SIZE];

	if (!putcBuffer || !zlibBuffer)
		OUT_OF_MEMORY();   

#ifdef USE_LZMA_LIBRARY
	LZMAlib_Init(8);
	inStream=new CInFileStream;
	iStream=inStream;
#endif
};

XWRT_Decoder::~XWRT_Decoder() 
{ 
	if (putcBuffer)
		delete(putcBuffer);

	if (zlibBuffer)
		delete(zlibBuffer);
}

#define WRITE_CHAR(c)\
{ \
	putcBufferData[0]=c; \
	putcBufferData++; \
	putcBufferSize++; \
	if (putcBufferSize>=mFileBufferSize) \
		writePutcBuffer(XWRT_fileout); \
}

#define DECODE_PUTC(c)\
{\
	if (IF_OPTION(OPTION_UNICODE_BE) || IF_OPTION(OPTION_UNICODE_LE)) \
	{ \
		if (c<0x80) \
		{ \
			if (IF_OPTION(OPTION_UNICODE_LE)) \
			{ \
				WRITE_CHAR(c); \
				WRITE_CHAR(0); \
			} \
			else \
			{ \
				WRITE_CHAR(0); \
				WRITE_CHAR(c); \
			} \
		} \
		else \
		{ \
			utf8buff[utf8pos++]=c; \
			if (sequence_length(utf8buff[0])==utf8pos) \
			{ \
				int d=utf8_to_unicode(&utf8buff[0]); \
				utf8pos=0; \
			\
				if (IF_OPTION(OPTION_UNICODE_LE)) \
				{ \
					if (d<65536) \
					{ \
						WRITE_CHAR(d%256); \
						WRITE_CHAR(d/256); \
					} \
					else \
					{ \
						d-=65536; \
						WRITE_CHAR(d%256); \
					} \
				} \
				else \
				{ \
					if (d<65536) \
					{ \
						WRITE_CHAR(d/256); \
						WRITE_CHAR(d%256); \
					} \
					else \
					{ \
						d-=65536; \
						WRITE_CHAR(d/256); \
					} \
				} \
			} \
		} \
	} \
	else \
		WRITE_CHAR(c); \
}



#define DECODE_GETC(c)\
{\
	if (cont.memout->memsize>maxMemSize) \
	{ \
		PRINT_DICT(("%d maxMemSize=%d\n",cont.memout->memsize,maxMemSize)); \
		cont.readMemBuffers(preprocFlag,maxMemSize,PPMVClib_order,PAQ_encoder,zlibBuffer,inStream); \
		cont.memout->memsize=0; \
	} \
 \
	c=cont.memout->InpSrcByte(); \
}

// decode word using dictionary
#define DECODE_WORD(dictNo,i)\
{\
		i++;\
		if (i>0 && i<sizeDict)\
		{\
			PRINT_CODEWORDS(("i=%d ",i)); \
			s_size=dictlen[i];\
			memcpy(s,dict[i],s_size+1);\
			PRINT_CODEWORDS(("%s\n",dict[i])); \
		}\
		else\
		{\
			s_size=0; \
			printf("File is corrupted %d/%d!\n",i,sizeDict);\
			fileCorrupted=true;\
		}\
}

void XWRT_Decoder::writePutcBuffer(FILE* &fileout)
{
	printStatus(0,(int)putcBufferSize,false);
	size_t res=fwrite_fast(putcBuffer,putcBufferSize,fileout);
	putcBufferData-=res;
	putcBufferSize-=res;
}

inline unsigned char XWRT_Decoder::mask8(unsigned char oc)
{
	return static_cast<unsigned char>(0xff & oc);
}

inline int XWRT_Decoder::sequence_length(unsigned char lead_it)
{
        unsigned char lead = mask8(lead_it);
        if (lead < 0x80) 
            return 1;
        else if ((lead >> 5) == 0x6)
            return 2;
        else if ((lead >> 4) == 0xe)
            return 3;
        else if ((lead >> 3) == 0x1e)
            return 4;
        else 
            return 0;
}

inline unsigned int XWRT_Decoder::utf8_to_unicode(unsigned char* it)
{
	unsigned int cp = mask8(*it);
	int length = sequence_length(*it);
	switch (length) {
		case 1:
			break;
		case 2:
			it++;
			cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
			break;
		case 3:
			++it; 
			cp = ((cp << 12) & 0xffff) + ((mask8(*it) << 6) & 0xfff);
			++it;
			cp += (*it) & 0x3f;
			break;
		case 4:
			++it;
			cp = ((cp << 18) & 0x1fffff) + ((mask8(*it) << 12) & 0x3ffff);                
			++it;
			cp += (mask8(*it) << 6) & 0xfff;
			++it;
			cp += (*it) & 0x3f; 
			break;
	}
	++it;
	return cp;        
}


// convert lower string to upper
inline void XWRT_Decoder::toUpper(unsigned char* s,int &s_size)
{
	for (int i=0; i<s_size; i++)
		s[i]=toupper(s[i]); 
}

inline int XWRT_Decoder::decodeCodeWord(unsigned char* &s,int& c)
{
	int i,s_size;

	if (codeword2sym[c]<dict1size)
	{
		i=codeword2sym[c];
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
	if (codeword2sym[c]<dict1plus2)
		i=dict1size*(codeword2sym[c]-dict1size);
	else
	if (codeword2sym[c]<dict1plus2plus3)
	{
		PRINT_CODEWORDS(("DC1b c=%d\n",codeword2sym[c]-dict1plus2));
		i=dict12size*(codeword2sym[c]-dict1plus2);
	}
	else
		i=dict123size*(codeword2sym[c]-dict1plus2plus3);



	DECODE_GETC(c);
	PRINT_CODEWORDS(("DC1 c=%d i=%d\n",c,i));



	if (codeword2sym[c]<dict1size)
	{
		i+=codeword2sym[c];
		i+=dict1size; //dictNo=2;
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
	if (codeword2sym[c]<dict1plus2)
	{
		PRINT_CODEWORDS(("DC2b c=%d\n",codeword2sym[c]-dict1size));
		i+=dict1size*(codeword2sym[c]-dict1size);
	}
	else
		i+=dict12size*(codeword2sym[c]-dict1plus2);

	DECODE_GETC(c);
	PRINT_CODEWORDS(("DC2 c=%d i=%d\n",c,i));



	if (codeword2sym[c]<dict1size)
	{
		PRINT_CODEWORDS(("DC3b c=%d\n",codeword2sym[c]));
		i+=codeword2sym[c];
		i+=bound3; //dictNo=3;
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
	if (codeword2sym[c]<dict1plus2)
		i+=dict1size*(codeword2sym[c]-dict1size);


	DECODE_GETC(c);
	PRINT_CODEWORDS(("DC3 c=%d i=%d\n",c,i));


	if (codeword2sym[c]<dict1size)
		i+=codeword2sym[c];
	else 
		printf("File is corrupted (codeword2sym[c]<dict1size)!\n");



	i+=bound4; //dictNo=4;
	DECODE_WORD(dictNo, i);

	return s_size;
}

inline int XWRT_Decoder::decodeCodeWord_LZMA(unsigned char* &s,int& c)
{
	int i,i2,s_size;

	if (codeword2sym[c]<dict1size)
	{
		i=codeword2sym[c];
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
		i=codeword2sym[c]-dict1size;


	DECODE_GETC(c);


	if (codeword2sym[c]<dict1size)
	{
		i=i*dict1size+codeword2sym[c];
		i+=dict1size; //dictNo=2;
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
		i2=codeword2sym[c]-dict1size;


	DECODE_GETC(c);


	i=i*dict12size+i2*dict1size+codeword2sym[c];
	i+=bound3; // 	dictNo=3;
	DECODE_WORD(dictNo, i);
	return s_size;
}


inline int XWRT_Decoder::decodeCodeWord_LZ(unsigned char* &s,int& c)
{
	int i,s_size;

	if (codeword2sym[c]<dict1size)
	{
		i=codeword2sym[c];
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
	if (codeword2sym[c]<dict1size+dict2size)
	{
		i=256*(codeword2sym[c]-dict1size);
		DECODE_GETC(c);
		i+=c;
		i+=dict1size; //dictNo=2;

		DECODE_WORD(dictNo, i);
		return s_size;
	}

	i=256*256*(codeword2sym[c]-dict1size-dict2size);
	DECODE_GETC(c);
	i+=256*c;
	DECODE_GETC(c);
	i+=c;
	i+=bound3; // 	dictNo=3;
	DECODE_WORD(dictNo, i);
	return s_size;
}



inline unsigned char* XWRT_Decoder::uint2string(unsigned int x,unsigned char* buf,int size)
{
	int i=size-1;

	buf[i]=0;
	i--;
	while (x>=10 && i>=0)
	{
		buf[i]='0'+x%10;
		x=x/10;
		i--;
	}
	buf[i]='0'+x%10;

	return &buf[i];
}


inline void XWRT_Decoder::hook_putc(int& c)
{
	if (c==EOF)
		return;

	if (XMLState==CLOSE && !startTagSet[c])
		XMLState=UNKNOWN;

	if (XMLState==OPEN)
	{
		if (!startTagSet[c])
		{
			if (WRTd_xs_size>0)
			{
				WRTd_xs[WRTd_xs_size]=0;
				PRINT_STACK(("push c1=%d (%c) letterType=%d WRTd_xs=%s WRTd_xs_size=%d\n",c,c,letterType,WRTd_xs,WRTd_xs_size));
				stack.push_back((char*)WRTd_xs);
				XMLState=ADDED;
			}
			else
			{
				if (c=='/')
				{
					static std::string str;
					if (stack.size()>0)
					{
						str=stack.back();
						stack.pop_back();
						PRINT_STACK(("pop %s\n",str.c_str()));
					}
					else
						str.erase();

					XMLState=CLOSE;
				}
				else
					XMLState=INSIDE;
			}

			if (IF_OPTION(OPTION_USE_CONTAINERS))
			{
				cont.selectMemBuffer(WRTd_xs,WRTd_xs_size);

				if (WRTd_xs_size>0 && c==' ')
				{
					last_c=c;
					return;
				}
			}
		}
		else
		{
			WRTd_xs[WRTd_xs_size++]=c;

			if (WRTd_xs_size>=STRING_MAX_SIZE)
				WRTd_xs_size=0; 
		}
	}


	if (c=='<')
	{
		WRTd_xs_size=0;
		XMLState=OPEN;
	}
	
	if (c=='>')
	{
		if (XMLState==ADDED && last_c=='/')
		{
			PRINT_STACK(("pop\n"));
			stack.pop_back();
		}
		
		XMLState=UNKNOWN;
	}

	last_c=c;

	DECODE_PUTC(c);
	return;
}

void XWRT_Decoder::WRT_decode()
{
	while (WRTd_c!=EOF)
	{
		if (fileCorrupted)
			return;

		PRINT_CHARS(("c=%d (%c)\n",WRTd_c,WRTd_c));

		if (outputSet[WRTd_c])
		{
			PRINT_CHARS(("addSymbols[%d] upperWord=%d\n",WRTd_c,upperWord));
			static std::string str;

			switch (WRTd_c)
			{
				case CHAR_ESCAPE:
					WRTd_upper=false;
					upperWord=UFALSE;

					DECODE_GETC(WRTd_c);
					PRINT_CHARS(("c==CHAR_ESCAPE, next=%x\n",WRTd_c));
					hook_putc(WRTd_c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_CRLF:
					if (!IF_OPTION(OPTION_CRLF))
						break;
					PRINT_CHARS(("c==CHAR_CRLF\n"));

					WRTd_c=13; hook_putc(WRTd_c);
					WRTd_c=10; hook_putc(WRTd_c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_END_TAG:
				case CHAR_END_TAG_EOL:
					PRINT_CHARS(("c==CHAR_END_TAG\n"));
					int c,i;
					c='<'; DECODE_PUTC(c);
					c='/'; DECODE_PUTC(c);

					if (stack.size()>0)
					{
						str=stack.back();
						stack.pop_back();
						PRINT_STACK(("pop2 %s\n",str.c_str()));
					}
					else
						str.erase();

					for (i=0; i<(int)str.size(); i++)
					{
						c=str[i];
						DECODE_PUTC(c);
					}

					c='>'; DECODE_PUTC(c);
					last_c=c;

					if (WRTd_c==CHAR_END_TAG_EOL)
					{
						if (IF_OPTION(OPTION_CRLF))
						{
							c=13;
							DECODE_PUTC(c);
						}
						c=10; 
						DECODE_PUTC(c);
						last_c=c;
					}

					cont.MemBufferPopBack();

					DECODE_GETC(WRTd_c);
					continue;


				case CHAR_FIRSTUPPER:
					PRINT_CHARS(("c==CHAR_FIRSTUPPER\n"));

					if (IF_OPTION(OPTION_SPACE_AFTER_CC_FLAG))
						DECODE_GETC(WRTd_c); // skip space

					WRTd_upper=true;
					upperWord=UFALSE;
					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_UPPERWORD:
					PRINT_CHARS(("c==CHAR_UPPERWORD\n"));

					if (IF_OPTION(OPTION_SPACE_AFTER_CC_FLAG))
						DECODE_GETC(WRTd_c); // skip space

					upperWord=FORCE;
					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_NOSPACE:
					if (!IF_OPTION(OPTION_SPACELESS_WORDS))
						break;

					PRINT_CHARS(("c==CHAR_NOSPACE\n"));

					if (upperWord==FORCE)
						upperWord=UTRUE;

					DECODE_GETC(WRTd_c);
					forceSpace=true;
					continue;

#ifdef DYNAMIC_DICTIONARY
				case CHAR_NEWWORD:

					if (IF_OPTION(OPTION_LETTER_CONTAINER))
						break;

					s_size=0;
					while (true)
					{
						DECODE_GETC(WRTd_c);
						if (WRTd_c!=0)
							WRTd_s[s_size++]=WRTd_c;
						else
							break;
					}
							
					if (s_size>0)
					{
						memcpy(mem,WRTd_s,s_size);
						
						if (mem<dictmem_end && addWord(mem,s_size)!=0)
						{	
							mem+=(s_size/4+1)*4;
						}
					}
					
					for (i=0; i<s_size; i++)
					{
						c=WRTd_s[i];
						hook_putc(c);
					}
					
					DECODE_GETC(WRTd_c);
					continue;
#endif

			}


			if (upperWord==FORCE)
				upperWord=UTRUE;
			else
				upperWord=UFALSE;
			
			if (codewordType==LZ77)
				s_size=decodeCodeWord_LZ(WRTd_s,WRTd_c);
			else
				if (codewordType==LZMA)
					s_size=decodeCodeWord_LZMA(WRTd_s,WRTd_c);
				else
					s_size=decodeCodeWord(WRTd_s,WRTd_c);
				
			int i;
				
			if (IF_OPTION(OPTION_SPACELESS_WORDS))
			{
				letterType=letterSet[WRTd_s[0]];

				if ((letterType==LOWERCHAR || letterType==UPPERCHAR))// && last_c!=' ')
				{
					beforeWord=last_c;
					if (forceSpace || (beforeWord=='/' || beforeWord=='-' || beforeWord=='\"' || beforeWord=='_' || beforeWord=='>'))
						forceSpace=false;
					else
					{
						int c=' ';
						hook_putc(c);
					}
				}
			}
			
			if (WRTd_upper)
			{
				WRTd_upper=false;
				WRTd_s[0]=toupper(WRTd_s[0]);
			}
			
			if (upperWord!=UFALSE)
				toUpper(&WRTd_s[0],s_size);
			
			upperWord=UFALSE;
			
			
			
			int c;
			for (i=0; i<s_size; i++)
			{
				c=WRTd_s[i];
				hook_putc(c);
			}
			
			DECODE_GETC(WRTd_c);
			continue;
		}

		if (IF_OPTION(OPTION_LETTER_CONTAINER) && (WRTd_c>='A' && WRTd_c<=LETTER_LAST))
		{
			s_size=0;
			switch (WRTd_c)
			{
				case 'A':
					WRTd_c=cont.memout_letters->InpSrcByte();
					hook_putc(WRTd_c);
					break;
				case 'B':

					while (true)
					{
						WRTd_c=cont.memout_words->InpSrcByte();
						if (WRTd_c==' ' || WRTd_c==EOF)
							break;
						hook_putc(WRTd_c);
						WRTd_s[s_size++]=WRTd_c;
					}
					break;
				case 'E': // LOWERCHAR
					WRTd_c=cont.memout_words2->InpSrcByte();
					WRTd_c=tolower(WRTd_c);
					while ((WRTd_c<'A' || WRTd_c>'Z') && WRTd_c!=EOF)
					{
						hook_putc(WRTd_c);
						WRTd_s[s_size++]=WRTd_c;
						WRTd_c=cont.memout_words2->InpSrcByte();
					}
					cont.memout_words2->UndoSrcByte();
					break;
				case 'C':
					WRTd_c=cont.memout_words3->InpSrcByte();
					do
					{
						hook_putc(WRTd_c);
						WRTd_s[s_size++]=WRTd_c;
						WRTd_c=cont.memout_words3->InpSrcByte();
					}
					while ((WRTd_c<'A' || WRTd_c>'Z') && WRTd_c!=EOF);
					cont.memout_words3->UndoSrcByte();
					break;
				case 'D':
					WRTd_c=cont.memout_words4->InpSrcByte();
					do
					{
						WRTd_c=toupper(WRTd_c);
						hook_putc(WRTd_c);
						WRTd_s[s_size++]=WRTd_c;
						WRTd_c=cont.memout_words4->InpSrcByte();
					}
					while ((WRTd_c<'A' || WRTd_c>'Z') && WRTd_c!=EOF);
					cont.memout_words4->UndoSrcByte();
					break;
			}

#ifdef DYNAMIC_DICTIONARY
			if (s_size>0)
			{
				memcpy(mem,WRTd_s,s_size);
		
				if (mem<dictmem_end && addWord(mem,s_size)!=0)
				{	
					mem+=(s_size/4+1)*4;
				}
			}
#endif
	
			DECODE_GETC(WRTd_c);
			continue;
		}

		if ((XMLState!=OPEN && XMLState!=CLOSE && (WRTd_c>='0' && WRTd_c<='9')) || (WRTd_c>=CHAR_IP && WRTd_c<=CHAR_TIME))
		{
			unsigned int no,mult;
			int c,i;
			no=0;
			mult=1;
			static int wType=0;
			static std::string mon;
			int day,month,year,newAll;

			switch (WRTd_c)
			{
				case CHAR_REMAIN:
					if (IF_OPTION(OPTION_NUMBER_CONTAINER))
						WRTd_c=cont.memout_remain2->InpSrcByte();
					else
						DECODE_GETC(WRTd_c);

					if (WRTd_c>=100)
					{
						c='0'+(WRTd_c/100); hook_putc(c);
					}
					c='0'+(WRTd_c/10)%10; hook_putc(c);
					c='.'; hook_putc(c);
					c='0'+WRTd_c%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_IP:
					int x1;

					for (i=0; i<4; i++)
					{
						if (IF_OPTION(OPTION_NUMBER_CONTAINER))
							x1=cont.memout_ip->InpSrcByte();
						else
							DECODE_GETC(x1);
				
						if (x1>=100) { c='0'+x1/100; hook_putc(c); }
						if (x1>=10) { c='0'+(x1/10)%10; hook_putc(c); }
						c='0'+x1%10; hook_putc(c);
						if (i<3) { c='.'; hook_putc(c); }
					}

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_HOURMINSEC:
					int h,m,s;

					if (IF_OPTION(OPTION_NUMBER_CONTAINER))
					{
						h=cont.memout_hms->InpSrcByte();
						m=cont.memout_hms->InpSrcByte();
						s=cont.memout_hms->InpSrcByte();
					}
					else
					{
						DECODE_GETC(h);
						DECODE_GETC(m);
						DECODE_GETC(s);
					}

					c='0'+(h/10)%10; hook_putc(c);
					c='0'+h%10; hook_putc(c);
					c=':'; hook_putc(c);
					c='0'+(m/10)%10; hook_putc(c);
					c='0'+m%10; hook_putc(c);
					c=':'; hook_putc(c);
					c='0'+(s/10)%10; hook_putc(c);
					c='0'+s%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_HOURMIN:

					if (IF_OPTION(OPTION_NUMBER_CONTAINER))
					{
						h=cont.memout_hm->InpSrcByte();
						m=cont.memout_hm->InpSrcByte();
					}
					else
					{
						DECODE_GETC(h);
						DECODE_GETC(m);
					}

					c='0'+(h/10)%10; hook_putc(c);
					c='0'+h%10; hook_putc(c);
					c=':'; hook_putc(c);
					c='0'+(m/10)%10; hook_putc(c);
					c='0'+m%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_PAGES:
					if (IF_OPTION(OPTION_NUMBER_CONTAINER))
					{
						no=256*cont.memout_pages->InpSrcByte();
						no+=cont.memout_pages->InpSrcByte();
						WRTd_c=cont.memout_pages->InpSrcByte();
					}
					else
					{
						DECODE_GETC(WRTd_c);
						no=256*WRTd_c;
						DECODE_GETC(WRTd_c);
						no+=WRTd_c;
						DECODE_GETC(WRTd_c);
					}

					WRTd_c+=no;

					if (WRTd_c>=1000)
					{
						c='0'+no/1000; hook_putc(c);
					}
					c='0'+(no/100)%10; hook_putc(c);
					c='0'+(no/10)%10; hook_putc(c);
					c='0'+no%10; hook_putc(c);
					c='-'; hook_putc(c);
					if (WRTd_c>=1000)
					{
						c='0'+WRTd_c/1000; hook_putc(c);
					}
					c='0'+(WRTd_c/100)%10; hook_putc(c);
					c='0'+(WRTd_c/10)%10; hook_putc(c);
					c='0'+WRTd_c%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;

				case CHAR_TIME:
					if (IF_OPTION(OPTION_NUMBER_CONTAINER))
					{
						day=cont.memout_time->InpSrcByte();
						month=cont.memout_time->InpSrcByte();
					}
					else
					{
						DECODE_GETC(day);
						DECODE_GETC(month);
					}

					day++;
					if (day>12)
					{
						day-=12;
						year=1;
					}
					else
						year=0;

					if (day>=10)
					{
						c='0'+day/10; hook_putc(c);
					}
					c='0'+day%10; hook_putc(c);

					c=':'; hook_putc(c);

					c='0'+month/10; hook_putc(c);
					c='0'+month%10; hook_putc(c);

					if (year)
						c='p';
					else
						c='a';

					hook_putc(c);
					c='m'; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;
				case CHAR_DATE_ENG:

					if (IF_OPTION(OPTION_NUMBER_CONTAINER))
					{
						no=cont.memout_date->InpSrcByte();
						WRTd_c=cont.memout_date2->InpSrcByte();
					}
					else
					{
						DECODE_GETC(no);
						DECODE_GETC(WRTd_c);
					}

					no+=256*WRTd_c;

					day=no%31+1;
					no=no/31;
					month=no%12+1;
					year=no/12+1929;

					c='0'+day/10; hook_putc(c);
					c='0'+day%10; hook_putc(c);
					c='-'; hook_putc(c);
					switch (month)
					{
						case 1: mon="JAN"; break;
						case 2: mon="FEB"; break;
						case 3: mon="MAR"; break;
						case 4: mon="APR"; break;
						case 5: mon="MAY"; break;
						case 6: mon="JUN"; break;
						case 7: mon="JUL"; break;
						case 8: mon="AUG"; break;
						case 9: mon="SEP"; break;
						case 10: mon="OCT"; break;
						case 11: mon="NOV"; break;
						default: mon="DEC"; break;
					}

					c=mon[0]; hook_putc(c);
					c=mon[1]; hook_putc(c);
					c=mon[2]; hook_putc(c);
					c='-'; hook_putc(c);
					c='0'+year/1000; hook_putc(c);
					c='0'+(year/100)%10; hook_putc(c);
					c='0'+(year/10)%10; hook_putc(c);
					c='0'+year%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;
				case '6':
					wType=2;
					DECODE_GETC(WRTd_c);
					continue;

				case '7':
					if (IF_OPTION(OPTION_NUMBER_CONTAINER))
						WRTd_c=cont.memout_remain->InpSrcByte();
					else
						DECODE_GETC(WRTd_c);

					c='.'; hook_putc(c);
					c='0'+(WRTd_c/10)%10; hook_putc(c);
					c='0'+WRTd_c%10; hook_putc(c);

					DECODE_GETC(WRTd_c);
					continue;
				case '8':
				case '9':
					if (WRTd_c=='8')
					{
						if (IF_OPTION(OPTION_NUMBER_CONTAINER))
						{
							no=cont.memout_date->InpSrcByte();
							WRTd_c=cont.memout_date2->InpSrcByte();
						}
						else
						{
							DECODE_GETC(no);
							DECODE_GETC(WRTd_c);
						}
						no+=256*WRTd_c;
						newAll=no+lastAll-(65536/2);
					}
					else
					{
						if (IF_OPTION(OPTION_NUMBER_CONTAINER))
							no=cont.memout_date3->InpSrcByte();
						else
							DECODE_GETC(no);
						newAll=no+lastAll-(256/2);
					}

					no=newAll;

					day=no%31+1;
					no=no/31;
					month=no%12+1;
					year=no/12+1929;

					c='0'+year/1000; hook_putc(c);
					c='0'+(year/100)%10; hook_putc(c);
					c='0'+(year/10)%10; hook_putc(c);
					c='0'+year%10; hook_putc(c);
					c='-'; hook_putc(c);
					c='0'+month/10; hook_putc(c);
					c='0'+month%10; hook_putc(c);
					c='-'; hook_putc(c);
					c='0'+day/10; hook_putc(c);
					c='0'+day%10; hook_putc(c);

					lastAll=newAll;

					DECODE_GETC(WRTd_c);
					continue;
				case '5':
					if (IF_OPTION(OPTION_NUMBER_CONTAINER))
						WRTd_c=cont.memout_year->InpSrcByte();
					else
						DECODE_GETC(WRTd_c);

					no=1900+(WRTd_c);

					if (wType==2)
						wType=3;
					else
					if (wType==3)
					{
						WRTd_c='-'; hook_putc(WRTd_c);
						wType=0;
					}
					break;
				default:
					c=WRTd_c-'0';

					if (!IF_OPTION(OPTION_NUMBER_CONTAINER))
					{
						for (i=0; i<c; i++)
						{
							DECODE_GETC(WRTd_c);	

							no+=mult*WRTd_c;
							mult*=NUM_BASE;
						}
					}
					else
					if (wType==2)
					{
						for (i=0; i<c; i++)
						{
							switch (c)
							{
							case 2:
								WRTd_c=cont.memout_num2b->InpSrcByte();
								break;
							case 3:
								WRTd_c=cont.memout_num3b->InpSrcByte(); 
								break;
							case 4:
								WRTd_c=cont.memout_num4b->InpSrcByte();
								break;
							default:
								WRTd_c=cont.memout_numb->InpSrcByte();
								break;
							}

							no+=mult*WRTd_c;
							mult*=NUM_BASE;
						}
					}
					else
					if (wType==3)
					{
						for (i=0; i<c; i++)
						{
							switch (c)
							{
							case 2:
								WRTd_c=cont.memout_num2c->InpSrcByte();
								break;
							case 3:
								WRTd_c=cont.memout_num3c->InpSrcByte(); 
								break;
							case 4:
								WRTd_c=cont.memout_num4c->InpSrcByte();
								break;
							default:
								WRTd_c=cont.memout_numc->InpSrcByte();
								break;
							}

							no+=mult*WRTd_c;
							mult*=NUM_BASE;
						}
					}
					else
					{

						for (i=0; i<c; i++)
						{
							switch (c)
							{
							case 2:
								WRTd_c=cont.memout_num2->InpSrcByte();
								break;
							case 3:
								WRTd_c=cont.memout_num3->InpSrcByte(); 
								break;
							case 4:
								WRTd_c=cont.memout_num4->InpSrcByte();
								break;
							default:
								WRTd_c=cont.memout_num->InpSrcByte();
								break;
							}

							no+=mult*WRTd_c;
							mult*=NUM_BASE;
						}
					}

					if (wType==2)
						wType=3;
					else
					if (wType==3)
					{
						WRTd_c='-'; hook_putc(WRTd_c);
						wType=0;
					}
			}

			unsigned char* numdata;
			numdata=uint2string(no,num,sizeof(num));

			while (numdata[0])
			{
				c=numdata[0];
				hook_putc(c);
				numdata++;
			}

			DECODE_GETC(WRTd_c);
			continue;
		}

 		PRINT_CHARS(("other c=%d (%d) upperWord=%d\n",fileLenMB,upperWord));

		if (upperWord!=UFALSE)
		{
			if (upperWord==FORCE)
				upperWord=UTRUE;

			if (WRTd_c>='a' && WRTd_c<='z')
				WRTd_c=toupper(WRTd_c);
			else
				upperWord=UFALSE;
		}
		else
		if (WRTd_upper)
		{
			WRTd_upper=false;
			WRTd_c=toupper(WRTd_c);
		}

		hook_putc(WRTd_c);

		DECODE_GETC(WRTd_c);
	}

	WRTd_c=EOF;
	hook_putc(WRTd_c);

	writePutcBuffer(XWRT_fileout);
}




void XWRT_Decoder::read_dict()
{
	int i,c,count;
	unsigned char* bound=(unsigned char*)&word_hash[0] + HASH_TABLE_SIZE*sizeof(word_hash[0]) - 6;

	unsigned char* bufferData=(unsigned char*)&word_hash[0] + 3;

#ifdef USE_PAQ_LIBRARY
	if (IF_OPTION(OPTION_PAQ))
	{
		for (i=0, count=0; i<3; i++)
			count=count*256+PAQ_encoder->decompress();


		int last=ftell(XWRT_file);
		for (i=0; i<count; )
		{
			bufferData[i++]=PAQ_encoder->decompress();
			if (i%102400==0)
			{
				printStatus(ftell(XWRT_file)-last,0,false);
				last=ftell(XWRT_file);
			}
		}

		printStatus(ftell(XWRT_file)-last,0,false);
	}
	else
#endif
#ifdef USE_LZMA_LIBRARY
	if (IF_OPTION(OPTION_LZMA))
	{
		unsigned int len=LZMAlib_GetFileSize(inStream);
		if (len>HASH_TABLE_SIZE*sizeof(word_hash[0]))
			OUT_OF_MEMORY();
		int last=LZMAlib_GetInputFilePos(inStream);
		LZMAlib_DecodeFileToMem(inStream,bufferData,len);
		printStatus(LZMAlib_GetInputFilePos(inStream)-last,0,false);
	}
	else
#endif
#ifdef USE_PPMVC_LIBRARY
	if (IF_OPTION(OPTION_PPMVC))
	{
		int last=ftell(XWRT_file);
		PPMVClib_DecodeFileToMem(PPMVClib_order,XWRT_file,bufferData,HASH_TABLE_SIZE*sizeof(word_hash[0])-6);
		printStatus(ftell(XWRT_file)-last,0,false);
	}
	else
#endif
#ifdef USE_ZLIB_LIBRARY
	if (IF_OPTION(OPTION_ZLIB))
		Zlib_decompress(XWRT_file,zlibBuffer,ZLIB_BUFFER_SIZE,bufferData,HASH_TABLE_SIZE*sizeof(word_hash[0])-6,i);
	else
#endif
	{
		for (i=0, count=0; i<3; i++)
		{
			GETC(c);
		    count=count*256+c;
		}

		fread_fast(bufferData,count,XWRT_file);
		printStatus(count,0,false);
	}


	if (IF_OPTION(OPTION_SPACES_MODELING))
	{
		for (i=0; i<256; i++)
			spacesCont[i]=0;

		count=bufferData[0]; bufferData++;

		PRINT_DICT(("sp_count=%d\n",count));

		for (i=0; i<count; i++)
		{
			c=bufferData[0]; bufferData++;
			spacesCont[c]=minSpacesFreq();
		}
	}			
	

	count=bufferData[0]; bufferData++;
	count+=256*bufferData[0]; bufferData++;
	count+=65536*bufferData[0]; bufferData++;
	
	sortedDict.clear();
	
	PRINT_DICT(("count=%d\n",count));
	
	std::string s;
	std::string last_s;
	for (i=0; i<count; i++)
	{
		if (preprocType!=LZ77 && bufferData[0]>=128)
		{
			s.append(last_s.c_str(),bufferData[0]-128);
			bufferData++;
		}

		while (bufferData[0]!=10)
		{
			s.append(1,bufferData[0]);
			bufferData++;

			if (s.size()>WORD_MAX_SIZE || bufferData>bound)
			{
				printf("File corrupted (s.size()>WORD_MAX_SIZE)!\n");
				OUT_OF_MEMORY();
			}
		}
		bufferData++;

		sortedDict.push_back(s);
		last_s=s;
		s.erase();
	}

	sortedDictSize=(int)sortedDict.size();

	PRINT_DICT(("read_dict count2=%d\n",count));

}

void XWRT_Decoder::WRT_set_options(char c,char c2)
{
	if ((c&128)==0)
		TURN_OFF(OPTION_USE_CONTAINERS)
	else
		TURN_ON(OPTION_USE_CONTAINERS);

	if ((c&64)==0)
		TURN_OFF(OPTION_PAQ)
	else
		TURN_ON(OPTION_PAQ);

	if ((c&32)==0)
		TURN_OFF(OPTION_ZLIB)
	else
		TURN_ON(OPTION_ZLIB);

	if ((c&16)==0)
		TURN_OFF(OPTION_PPMVC)
	else
		TURN_ON(OPTION_PPMVC);

	if ((c&8)==0)
		TURN_OFF(OPTION_LZMA)
	else
		TURN_ON(OPTION_LZMA);

	if ((c&4)==0)
		TURN_OFF(OPTION_BINARY_DATA)
	else
		TURN_ON(OPTION_BINARY_DATA);


	if ((c2&128)==0)
		TURN_OFF(OPTION_LETTER_CONTAINER)
	else
		TURN_ON(OPTION_LETTER_CONTAINER);

	if ((c2&64)==0)
		TURN_OFF(OPTION_NUMBER_CONTAINER)
	else
		TURN_ON(OPTION_NUMBER_CONTAINER);

	if ((c2&32)==0)
		TURN_OFF(OPTION_SPACES_MODELING)
	else
		TURN_ON(OPTION_SPACES_MODELING);

	if ((c2&16)==0)
		TURN_OFF(OPTION_CRLF)
	else
		TURN_ON(OPTION_CRLF);

	if ((c2&8)==0)
		TURN_OFF(OPTION_QUOTES_MODELING)
	else
		TURN_ON(OPTION_QUOTES_MODELING);

	if ((c2&4)==0)
		TURN_OFF(OPTION_USE_DICTIONARY)
	else
		TURN_ON(OPTION_USE_DICTIONARY);

	if ((c2&2)==0)
		TURN_OFF(OPTION_UNICODE_LE)
	else
		TURN_ON(OPTION_UNICODE_LE);

	if ((c2&1)==0)
		TURN_OFF(OPTION_UNICODE_BE)
	else
		TURN_ON(OPTION_UNICODE_BE);
}

void XWRT_Decoder::WRT_start_decoding(int c,char* filename,char* filenameOut)
{
	int i,j,k,c2,dictPathLen;
	unsigned char s[STRING_MAX_SIZE];
	unsigned char dictPath[STRING_MAX_SIZE];
	s[0]=0;

	lastAll=0;
	XMLState=UNKNOWN;	
	last_c=0;
	forceSpace=false;
	WRTd_upper=false;
	upperWord=UFALSE;
	s_size=0;
	WRTd_xs_size=0;
	collision=0;


	preprocType=(EPreprocessType)(c%4); // { LZ77, LZMA/BWT, PPM, PAQ }
	if ((c&8)!=0)
		TURN_ON(OPTION_LZMA);
	GETC(c2);

	defaultSettings(0,NULL); // after setting preprocType 
	WRT_set_options(c,c2);

	GETC(maxMemSize); // after defaultSettings()
	maxMemSize*=1024*1024;

	GETC(additionalParam); // fileLenMB/256
	GETC(fileLenMB); // fileLenMB%256
	fileLenMB+=256*additionalParam;
	g_fileLenMB=fileLenMB;

	GETC(additionalParam);



	getAlgName(compName);
	printf("- decoding %s (%s) to %s\n",filename,compName.c_str(),filenameOut);


	init_PPMVC(fileLenMB,DECOMPRESS);


	WRT_print_options();

	PRINT_DICT(("maxMemSize=%d fileLenMB=%d preprocType=%d\n",maxMemSize,fileLenMB,preprocType));


	if (IF_OPTION(OPTION_BINARY_DATA))
	{
		cont.readMemBuffers(preprocFlag,maxMemSize,PPMVClib_order,PAQ_encoder,zlibBuffer,inStream);
		cont.memout->memsize=0;

		putcBufferData=&putcBuffer[0];
		putcBufferSize=0;

		while (true)
		{
			DECODE_GETC(c);
			if (c<0)
				break;
			WRITE_CHAR(c);
		}

		writePutcBuffer(XWRT_fileout);

		if (cont.bigBuffer)
		{
			free(cont.bigBuffer);
			cont.bigBuffer=NULL;
			cont.freeMemBuffers(false);
		}
		else
			cont.freeMemBuffers(true);

#ifdef USE_PAQ_LIBRARY
		if (PAQ_encoder)
		{
			PAQ_encoder->flush();
			delete(PAQ_encoder);
			PAQ_encoder=NULL;
		}
#endif
		return;
	}


	read_dict();


	memset(detectedSymbols,0,sizeof(detectedSymbols));

	if (!IF_OPTION(OPTION_PAQ))
		GETC(i)
#ifdef USE_PAQ_LIBRARY
	else
		i=PAQ_encoder->decompress();
#endif

	k=1;
	for (j=0; j<8; j++)
	{
		if (i & k) detectedSymbols[j]=1;
		k*=2;
	}

	if (!IF_OPTION(OPTION_PAQ))
		GETC(i)
#ifdef USE_PAQ_LIBRARY
	else
		i=PAQ_encoder->decompress();
#endif

	k=1;
	for (j=8; j<16; j++)
	{
		if (i & k) detectedSymbols[j]=1;
		k*=2;
	}

	if (!IF_OPTION(OPTION_PAQ))
		GETC(i)
#ifdef USE_PAQ_LIBRARY
	else
		i=PAQ_encoder->decompress();
#endif

	k=1;
	for (j=16; j<24; j++)
	{
		if (i & k) detectedSymbols[j]=1;
		k*=2;
	}
	
	
	dictPathLen=getSourcePath((char*)dictPath,sizeof(dictPath));


	if (dictPathLen>0)
	{
		dictPath[dictPathLen]=0;
		strcat((char*)dictPath,(char*)s);
		strcat((char*)dictPath,(char*)"wrt-eng.dic");
		strcpy((char*)s,(char*)dictPath);
	}

	cont.readMemBuffers(preprocFlag,maxMemSize,PPMVClib_order,PAQ_encoder,zlibBuffer,inStream);
	cont.memout->memsize=0;

	WRT_deinitialize();

	decoding=true;
	if (!initialize(s,false))
		return;

	putcBufferData=&putcBuffer[0];
	putcBufferSize=0;

	DECODE_GETC(WRTd_c);
	PRINT_CHARS(("WRT_start_decoding WRTd_c=%d ftell=%d\n",WRTd_c,ftell(XWRT_file)));

	WRT_decode(); 

	if (cont.bigBuffer)
	{
		free(cont.bigBuffer);
		cont.bigBuffer=NULL;
		cont.freeMemBuffers(false);
	}
	else
		cont.freeMemBuffers(true);

#ifdef USE_PAQ_LIBRARY
	if (PAQ_encoder)
	{
		PAQ_encoder->flush();
		delete(PAQ_encoder);
		PAQ_encoder=NULL;
	}
#endif
}


