#include "XWRT.h"
#include "Common.h"
#include <stdio.h>

#ifdef USE_PAQ_LIBRARY
	#include "lpaq6.hpp"
#endif

#ifdef WINDOWS
	#include <windows.h> // GetModuleFileName()
	#include <io.h> // _get_osfhandle()
#endif

FILE* XWRT_file;
FILE* XWRT_fileout;
unsigned char** dict=NULL;
int* dictfreq=NULL;
unsigned char* dictlen=NULL;

XWRT_Common::XWRT_Common(int fileBufferSize) : preprocType(LZ77), dictmem(NULL),
	detect(false), dictmem_end(NULL), compLevel(0),
	fileCorrupted(false), firstPassBlock(1), deleteInputFiles(false), YesToAll(false)
{
	memset(spacesCodeword,0,sizeof(spacesCodeword));
	memset(detectedSymbols,0,sizeof(detectedSymbols));
	memset(value,0,sizeof(value));
	memset(addSymbols,0,sizeof(addSymbols));
	memset(reservedSet,0,sizeof(reservedSet));
	memset(spacesCont,0,sizeof(spacesCont));

	if (fileBufferSize<10)
		fileBufferSize=10; // 1 KB

	if (fileBufferSize>23)
		fileBufferSize=23; // 8 MB

	mFileBufferSize=1<<fileBufferSize;


	word_hash=new int[HASH_TABLE_SIZE];

	if (!word_hash)
		OUT_OF_MEMORY();

#ifdef USE_PAQ_LIBRARY
	PAQ_encoder=NULL;
#endif
}

XWRT_Common::~XWRT_Common()
{
	if (word_hash)
		delete(word_hash);

	WRT_deinitialize(); 

#ifdef USE_PPMD_LIBRARY
	PPMDlib_Deinit(); 
#endif
}



// filesize() function

unsigned int XWRT_Common::flen( FILE* &f )
{
	fseek( f, 0, SEEK_END );
	unsigned int len = ftell(f);
	fseek( f, 0, SEEK_SET );
	return len;
}


// make hash from string
inline void XWRT_Common::stringHash(const unsigned char *ptr, int len,int& hash)
{
	for (hash = 0; len>0; len--, ptr++)
	{
		hash *= HASH_MULT;
		hash += *ptr;
	}

	hash=hash&(HASH_TABLE_SIZE-1);
}

int XWRT_Common::addWord(unsigned char* &mem,int &i)
{
	int c,j;

	if (i<=1 || sizeDict>=dictionary)
		return -1;
	
	dictlen[sizeDict]=i;
	dict[sizeDict]=mem;
	
	mem[i]=0;
	stringHash(mem,i,j);
	
	if (word_hash[j]!=0)
	{
		if (dictlen[sizeDict]!=dictlen[word_hash[j]] || memcmp(dict[sizeDict],dict[word_hash[j]],dictlen[sizeDict])!=0)
		{
			c=(j+i*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1);
			if (word_hash[c]!=0)
			{
				if (dictlen[sizeDict]!=dictlen[word_hash[c]] || memcmp(dict[sizeDict],dict[word_hash[c]],dictlen[sizeDict])!=0)
				{
					c=(j+i*HASH_DOUBLE_MULT*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1);
					if (word_hash[c]!=0)
					{
						collision++;
						return -1;
					}
					else
					{
						word_hash[c]=sizeDict++;
					}
				}
				else
					return -1; // word already exists
			}
			else
			{
				word_hash[c]=sizeDict++;
			}
		}
		else
			return -1; // word already exists
	}
	else
	{
		word_hash[j]=sizeDict++;
	}

	return 1;
}


unsigned char* XWRT_Common::loadDynamicDictionary(unsigned char* mem,unsigned char* mem_end)
{
	int i;

	for (i=0; i<256; i++)
		spacesCodeword[i]=0;

	if (IF_OPTION(OPTION_QUOTES_MODELING))
	{
		mem[0]='=';
		mem[1]='\"';

		quoteOpen=sizeDict;

		int len=2;
		addWord(mem,len);

		mem+=4;


		mem[0]='\"';
		mem[1]='>';

		quoteClose=sizeDict;

		addWord(mem,len);

		mem+=4;
	}

	if (IF_OPTION(OPTION_SPACES_MODELING))
	{
		for (i=0; i<256; i++)
		if (spacesCont[i]>=minSpacesFreq())
		{
			memset(mem,' ',i);
			mem[i]=0;

			spacesCodeword[i]=sizeDict;

			if (addWord(mem,i)==0)
				break;

			mem+=(i/4+1)*4;
		}
	}


	int count=sortedDictSize;
//	VERBOSE(("count=%d %d\n",count,sortedDict.size());

	for (i=0; i<count; i++)
	{
		std::string s=sortedDict[i];

	//	VERBOSE(("i=%d %s %p/%p\n",i,s.c_str(),mem,mem_end);

		int len=(int)sortedDict[i].size();
		memcpy(mem,sortedDict[i].c_str(),len+1);

		if (addWord(mem,len)==0)
			break;

		mem+=(len/4+1)*4;

		if (mem>mem_end)
			break;
	}


	if (mem<mem_end)
	{
		i=strlen("http://www.");

		memcpy(mem,"http://www.",i);

		if (addWord(mem,i)!=0)
			mem+=(i/4+1)*4;
	}

	PRINT_DICT(("count=%d sortedDict.size()=%d\n",count,sortedDictSize));

	sizeDynDict=sizeDict;

	return mem;
}

unsigned char* XWRT_Common::loadDictionary(FILE* file,unsigned char* mem)
{
	unsigned char* word;
	int c,i;

	collision=0;


	while (!feof(file))
	{
		word=mem;
		do
		{
			c=getc(file);
			word[0]=c;
			word++;
		}
		while (c>32);

		if (c==EOF)
			break;
		if (c=='\r') 
			c=getc(file);
		
		word[-1]=0;
		i=(int)(word-mem-1);
		
		if (addWord(mem,i)==0)
			break;

		mem+=(i/4+1)*4;
	}

	if (collision>0)
		PRINT_DICT(("warning: hash collisions=%d\n",collision));

	return mem;
}

void XWRT_Common::loadCharset(FILE* file)
{
	int c;
	
	c=getc(file); 

	while (c>32)
		c=getc(file);

	if (c==13)
		c=getc(file); // skip CR+LF or LF
}


void XWRT_Common::tryAddSymbol(int c)
{
	if (!decoding)
	{
//		VERBOSE(("lenKB=%d %d=%d %d<%d\n",fileLenKB,c,value[c]<50+50*fileLenMB,value[c],50+50*fileLenMB);
//		if (value[c]<500+200*(fileLenMB/5))
		if (value[c]<50+50*fileLenMB)
		{
			addSymbols[c]=1;
			detectedSymbols[detectedSym++]=1;
		}
		else
			detectedSymbols[detectedSym++]=0;
	}
	else
		addSymbols[c]=detectedSymbols[detectedSym++];
}

int XWRT_Common::minSpacesFreq()
{
	return 300+200*(fileLenMB/5);
}


void XWRT_Common::initializeLetterSet()
{
	int c;

	for (c=0; c<256; c++)
		letterSet[c]=UNKNOWNCHAR;

	for (c='0'; c<='9'; c++)
		letterSet[c]=NUMBERCHAR;

	for (c='A'; c<='Z'; c++)
		letterSet[c]=UPPERCHAR;

	for (c='a'; c<='z'; c++)
		letterSet[c]=LOWERCHAR;

	letterSet['>']=XMLCHAR;
	letterSet['/']=XMLCHAR;
	letterSet[':']=XMLCHAR;
	letterSet['?']=XMLCHAR;
	letterSet['!']=XMLCHAR;

	for (c=0; c<256; c++)
		if (reservedSet[c])
			letterSet[c]=RESERVEDCHAR;

	for (c=0; c<256; c++)  //                                                - _ . , :
		if (c>127 || letterSet[c]==LOWERCHAR || letterSet[c]==UPPERCHAR || c==' ' /*|| c=='\''*/) // || c=='&') 
			wordSet[c]=1;
		else
			wordSet[c]=0;

	for (c=0; c<256; c++)
		if (c=='&' || c=='#' || c==';' || c>=128 || letterSet[c]==LOWERCHAR || letterSet[c]==UPPERCHAR || letterSet[c]==NUMBERCHAR || c=='.' || c=='-' || c=='_' || c==':') 
			startTagSet[c]=1;
		else
			startTagSet[c]=0;

	if (IF_OPTION(OPTION_LETTER_CONTAINER))
	{
		for (c=0; c<256; c++)
			if (letterSet[c]!=LOWERCHAR && letterSet[c]!=NUMBERCHAR && c!='.' && c!='-') 
				urlSet[c]=1;
			else
				urlSet[c]=0;
	}
	else
	{
		for (c=0; c<256; c++)
			if (letterSet[c]!=LOWERCHAR && c!='.' && c!='-') 
				urlSet[c]=1;
			else
				urlSet[c]=0;
	}

	for (c=0; c<256; c++)
		if (c==9 || c=='\r' || c=='\n' || c==32 || c=='>' || c=='?' || c=='!' || c=='/') 
			whiteSpaceSet[c]=1;
		else
			whiteSpaceSet[c]=0;
}

void XWRT_Common::initializeCodeWords(int word_count,bool initMem)
{
	int c,charsUsed,i;

	detectedSym=0;

	for (c=0; c<256; c++)
	{
		addSymbols[c]=0;
		codeword2sym[c]=0;
		sym2codeword[c]=0;
		reservedSet[c]=0;
		reservedFlags[c]=0;
		outputSet[c]=0;
	}


	if (IF_OPTION(OPTION_ADD_SYMBOLS_MISC))
	{
		tryAddSymbol('&'); //38
		tryAddSymbol('('); //40
		tryAddSymbol(')'); //41
		tryAddSymbol(','); //44
		tryAddSymbol('-'); //45
		tryAddSymbol('.'); //46
		tryAddSymbol('/'); //47  
		tryAddSymbol(':'); //58

		tryAddSymbol('='); //61
		tryAddSymbol('_'); //95
		tryAddSymbol(127); //127
		tryAddSymbol('$'); //36
		tryAddSymbol('^'); //94
		tryAddSymbol('`'); //96
		tryAddSymbol('['); //91
		tryAddSymbol(']'); //93

		tryAddSymbol('!'); //33
		tryAddSymbol('#'); //35
		tryAddSymbol('?'); //63
		tryAddSymbol(9); //tab 
		tryAddSymbol('%'); //37
		tryAddSymbol('\''); //39
		tryAddSymbol('+'); //43
		tryAddSymbol(';'); //59

		tryAddSymbol('@'); //64
		tryAddSymbol('|'); //124
		tryAddSymbol('~'); //126
		tryAddSymbol('*'); //42
		tryAddSymbol('\\'); //92
		tryAddSymbol('{'); //123
		tryAddSymbol('}'); //125
	}

	if (IF_OPTION(OPTION_ADD_SYMBOLS_0_5))
		for (c=0; c<=8; c++)
			addSymbols[c]=1;

	if (IF_OPTION(OPTION_ADD_SYMBOLS_14_31))
		for (c=11; c<=31; c++)
			addSymbols[c]=1;

	for (c=0; c<256; c++)
	{
		if (c==CHAR_ESCAPE || c==CHAR_FIRSTUPPER || c==CHAR_UPPERWORD || c==CHAR_END_TAG || c==CHAR_END_TAG_EOL
			|| (IF_OPTION(OPTION_CRLF) && c==CHAR_CRLF) 
			|| (IF_OPTION(OPTION_SPACELESS_WORDS) && c==CHAR_NOSPACE)
			|| c==CHAR_HOURMINSEC || c==CHAR_HOURMIN || c==CHAR_IP || c==CHAR_PAGES || c==CHAR_DATE_ENG || c==CHAR_TIME || c==CHAR_REMAIN)
		{
			reservedSet[c]=1;
			addSymbols[c]=0;
		}
	}

	for (c=0; c<256; c++)
		if (addSymbols[c])
			reservedSet[c]=1;

	initializeLetterSet();

	for (c=BINARY_FIRST; c<=BINARY_LAST; c++)
		addSymbols[c]=1;

	if (IF_OPTION(OPTION_LETTER_CONTAINER))
	{
		for (c='a'; c<='z'; c++)
			addSymbols[c]=1;

		for (c=LETTER_LAST+1; c<='Z'; c++)
			addSymbols[c]=1; 
	}

	for (c=0; c<256; c++)
	{
		if ((reservedSet[c] || addSymbols[c]) && (c<CHAR_IP || c>CHAR_TIME))
			outputSet[c]=1;
	}

	charsUsed=0;
	for (c=0; c<256; c++)
	{
		if (addSymbols[c])
		{
			codeword2sym[c]=charsUsed;
			sym2codeword[charsUsed]=c;
			charsUsed++;

			if (codewordType==PAQ)
			{
				if (c<128+64)
					dict1size=charsUsed;
				if (c<128+64+32)
					dict2size=charsUsed;
				if (c<128+64+32+16)
					dict3size=charsUsed;
				if (c<128+64+32+16+16)
					dict4size=charsUsed;
			}
		}
	}

	c=word_count;

	if (codewordType==PAQ)
	{
		dict4size-=dict3size;
		dict3size-=dict2size;
		dict2size-=dict1size;

		if (dict1size<4 || dict2size<4 || dict3size<4 || dict4size<4)
		{
			dict2size=dict3size=dict4size=charsUsed/4;
			dict1size=charsUsed-dict4size*3;

			for (i=0; i<charsUsed/4; i++)
			{
				if (i*i*i*(charsUsed-i*3)>c)
				{
					dict1size=charsUsed-i*3;
					dict2size=i;
					dict3size=i;
					dict4size=i;
					break;
				}
			}
		}
	}
	else
		if (codewordType==PPM)
		{
			dict2size=16; // 28 
			i=7;

			do
			{	
				i++;
				dict3size=i;
				dict4size=i;
				dict1size=charsUsed-dict2size-dict3size-dict4size;
			}
			while (dict1size*dict2size*dict3size*dict4size+dict1size*dict2size*dict3size+dict1size*dict2size+dict1size<c);
		}
		else
		if (codewordType==LZMA)
		{
			i=31;
			dict4size=0;

			int c2=0;

			do
			{	
				i++;
				dict2size=i;
				dict3size=i;
				dict1size=charsUsed-dict2size;

				if (c2>dict1size*dict2size*dict3size+dict1size*dict2size+dict1size)
				{
					i--;
					dict2size=i;
					dict3size=i;
					dict1size=charsUsed-dict2size;
					break;
				}
				c2=dict1size*dict2size*dict3size+dict1size*dict2size+dict1size;
			}
			while (c2<c);
		} 
		else
		{
			if (fileLenMB>100)
				dict2size=64;
			else
			if (fileLenMB>50)
				dict2size=54;
			else
				dict2size=42;
			dict3size=-1; //3
			dict4size=0;
			do
			{
				dict3size++;
				dict1size=charsUsed-dict2size-dict3size;
			}
			while (255*255*dict3size+255*dict2size+dict1size<c);
		}

		if (codewordType==LZ77)// || codewordType==LZMA)
		{
			dictionary=(255*255*dict3size+255*dict2size+dict1size);
			bound4=255*255*dict3size+255*dict2size+dict1size;
			bound3=255*dict2size+dict1size;
			dict123size=255*255*dict3size;
			dict12size=255*dict2size;
		}
		else
		{
			dictionary=(dict1size*dict2size*dict3size*dict4size+dict1size*dict2size*dict3size+dict1size*dict2size+dict1size);
			bound4=dict1size*dict2size*dict3size+dict1size*dict2size+dict1size;
			bound3=dict1size*dict2size+dict1size;
			dict123size=dict1size*dict2size*dict3size;
			dict12size=dict1size*dict2size;
		}

	dict1plus2=dict1size+dict2size;
	dict1plus2plus3=dict1size+dict2size+dict3size;

	if (initMem)
	{
		dict=(unsigned char**)calloc(sizeof(unsigned char*)*(dictionary+1),1);
		dictlen=(unsigned char*)calloc(sizeof(unsigned char)*(dictionary+1),1);

		if (!dict || !dictlen)
			OUT_OF_MEMORY();
	}

	PRINT_DICT(("codewordType=%d %d %d %d %d(%d) charsUsed=%d sizeDict=%d\n",codewordType,dict1size,dict2size,dict3size,dict4size,dictionary,charsUsed,sizeDict));
}


// read dictionary from files to arrays
bool XWRT_Common::init_dict(unsigned char* dictName)
{
	int i,c,fileLen;
	FILE* file;

	WRT_deinitialize();

	memset(&word_hash[0],0,HASH_TABLE_SIZE*sizeof(word_hash[0]));

	if (!IF_OPTION(OPTION_USE_DICTIONARY))
	{
		dict123size=sortedDictSize;
		if (dict123size<20)
			dict123size=20;

		initializeCodeWords(dict123size);
		int dicsize=dictionary*WORD_AVG_SIZE*2;
		dictmem=(unsigned char*)calloc(dicsize,1);
		dictmem_end=dictmem+dicsize-256;
		PRINT_DICT(("allocated memory=%d\n",dicsize));

		if (!dictmem)
			OUT_OF_MEMORY();

		sizeDict=1;
		mem=loadDynamicDictionary(dictmem,dictmem_end);
	}
	else
	{
		VERBOSE(("- loading dictionary %s\n",dictName)); 

		file=fopen((const char*)dictName,"rb");
		if (file==NULL)
		{
			VERBOSE(("Can't open dictionary %s\n",dictName));
			return false;
		}

		fileLen=flen(file);
		fscanf(file,"%d",&dict123size);


		do { c=getc(file); } while (c>=32); if (c==13) c=getc(file); // skip CR+LF or LF

		for (i=0; i<CHARSET_COUNT; i++)
		{
			loadCharset(file);
			loadCharset(file);
		}

		dict123size+=sortedDictSize;

		initializeCodeWords(dict123size);
		int dicsize=fileLen*2+dict123size*WORD_AVG_SIZE*2;
		dictmem=(unsigned char*)calloc(dicsize,1);
		dictmem_end=dictmem+dicsize-256;
		PRINT_DICT(("allocated memory=%d\n",dicsize));

		if (!dictmem)
			OUT_OF_MEMORY();

		sizeDict=1;
		mem=loadDynamicDictionary(dictmem,dictmem_end);
		mem=loadDictionary(file,mem);

		fclose(file);
	}

	VERBOSE((" + loaded dictionary %d/%d words\n",sizeDict,dictionary));

	return true;
}


void XWRT_Common::WRT_deinitialize()
{
	if (dict)
	{
		free(dict);
		dict=NULL;
	}
	if (dictlen)
	{
		free(dictlen);
		dictlen=NULL;
	}
	if (dictmem)
	{
		free(dictmem);
		dictmem=NULL;
	}
	if (dictfreq)
	{
		free(dictfreq);
		dictfreq=NULL;
	}

	sizeDict=0;
}

void XWRT_Common::WRT_print_options()
{
	if (IF_OPTION(OPTION_UNICODE_LE) || IF_OPTION(OPTION_UNICODE_BE)) PRINT_OPTIONS(("UNICODE "));
	if (IF_OPTION(OPTION_BINARY_DATA)) PRINT_OPTIONS(("BINARY_DATA "));
	if (IF_OPTION(OPTION_USE_DICTIONARY)) PRINT_OPTIONS(("USE_DICTIONARY "));
	if (IF_OPTION(OPTION_SPACELESS_WORDS)) PRINT_OPTIONS(("SPACELESS "));
	if (IF_OPTION(OPTION_SPACES_MODELING)) PRINT_OPTIONS(("SPACES_MODELING "));
	if (IF_OPTION(OPTION_TRY_SHORTER_WORD)) PRINT_OPTIONS(("TRY_SHORTER "));
	if (IF_OPTION(OPTION_NUMBER_CONTAINER)) PRINT_OPTIONS(("NUMBER_CONTAINER "));
	if (IF_OPTION(OPTION_ADD_SYMBOLS_14_31)) PRINT_OPTIONS(("ADD_SYMBOLS_14_31 "));
	if (IF_OPTION(OPTION_ADD_SYMBOLS_0_5)) PRINT_OPTIONS(("ADD_SYMBOLS_0_5 "));
	if (IF_OPTION(OPTION_ZLIB)) PRINT_OPTIONS(("ZLIB "));
	if (IF_OPTION(OPTION_END_TAG_OMISSION)) PRINT_OPTIONS(("HTML "));
	if (IF_OPTION(OPTION_QUOTES_MODELING)) PRINT_OPTIONS(("QUOTES_MODELING "));
    if (IF_OPTION(OPTION_CRLF)) PRINT_OPTIONS(("CRLF "));
	if (IF_OPTION(OPTION_LETTER_CONTAINER)) PRINT_OPTIONS(("RAREWORDS "));
	PRINT_OPTIONS(("prepType=%d\n",preprocType));
}

void XWRT_Common::init_PPMD(int fileSizeInMB,int mode)
{
#ifdef USE_PPMD_LIBRARY

	if (IF_OPTION(OPTION_PPMD)) // preprocType==PPM
	{
		PPMDlib_Deinit();

		PPMDlib_Init(additionalParam);

		if (fileSizeInMB>=23)
			PPMDlib_order=6;
		else
		if (fileSizeInMB>=7)
			PPMDlib_order=8;
		else
			PPMDlib_order=10;
	}
#endif

#ifdef USE_LZMA_LIBRARY
	if (IF_OPTION(OPTION_LZMA)) // preprocType==LZMA
		LZMAlib_Init(additionalParam);
#endif

#ifdef USE_PAQ_LIBRARY
	if (IF_OPTION(OPTION_PAQ)) // preprocType==PAQ
	{
		set_PAQ_level(additionalParam);
		if (mode==COMPRESS)
			PAQ_encoder=new Encoder(mode,XWRT_fileout);
		else
			PAQ_encoder=new Encoder(mode,XWRT_file);
	}

#endif
}

void XWRT_Common::getAlgName(std::string& compName)
{
	if (IF_OPTION(OPTION_PAQ))
	{

		switch (additionalParam)
		{
			case 5:
				compName="lpaq6 104 MB";
				break;
			case 6:
				compName="lpaq6 198 MB"; 
				break;
			case 7:
				compName="lpaq6 390 MB";
				break;
			case 8:
				compName="lpaq6 774 MB";
				break;
			case 9:
				compName="lpaq6 1542 MB";
				break;
		}


#ifndef USE_PAQ_LIBRARY
		VERBOSE(("lpaq6 compression not supported (in this compilation of XWRT)!\n"));
		exit(0);
#endif
	}
	else
	if (IF_OPTION(OPTION_PPMD))
	{
		if (additionalParam==16)
			compName="PPMD 16MB";
		else
		if (additionalParam==32)
			compName="PPMD 32MB";
		else
			compName="PPMD 64MB";

#ifndef USE_PPMD_LIBRARY
		VERBOSE(("PPMD compression not supported (in this compilation of XWRT)!\n"));
		exit(0);
#endif
	}
	else
	if (IF_OPTION(OPTION_LZMA))
	{
		if (additionalParam==0)	
			compName="LZMA 64KB";
		else
		if (additionalParam==1)	
			compName="LZMA 1MB";
		else
			compName="LZMA 8MB";

#ifndef USE_LZMA_LIBRARY
		VERBOSE(("LZMA compression not supported (in this compilation of XWRT)!\n"));
		exit(0);
#endif
	}
	else
	if (IF_OPTION(OPTION_ZLIB))
	{
		if (additionalParam==1)	
			compName="zlib fast";
		else
		if (additionalParam==6)	
			compName="zlib normal";
		else
			compName="zlib best";

#ifndef USE_ZLIB_LIBRARY
		VERBOSE(("Zlib compression not supported (in this compilation of XWRT)!\n"));
		exit(0);
#endif
	}
	else
		compName="store";
}


int XWRT_Common::defaultSettings(int argc, char* argv[])
{
    PRINT_OPTIONS(("defaultSettings\n"));

	static bool firstTime=true;
	bool minWordFreqChanged=false;

	RESET_OPTIONS;
	codewordType=preprocType;

//	TURN_ON(OPTION_USE_DICTIONARY);
//	TURN_ON(OPTION_QUOTES_MODELING);
//	TURN_ON(OPTION_CRLF);
	TURN_ON(OPTION_TRY_SHORTER_WORD);
	TURN_ON(OPTION_SPACES_MODELING);
#ifdef USE_ZLIB_LIBRARY
	TURN_ON(OPTION_ZLIB);
	additionalParam=6; // zlib normal
    compLevel = 2;
#endif

	tryShorterBound=2;
	maxMemSize=8*1024*1024;
	maxDynDictBuf=8;
	maxDictSize=65535*32700;
	compName="zlib normal";

	switch (preprocType)
	{
		case NONE:
		case LZ77: // for LZ77 there are different codeWords
			TURN_ON(OPTION_SPACELESS_WORDS);
			TURN_ON(OPTION_ADD_SYMBOLS_0_5);
			TURN_ON(OPTION_ADD_SYMBOLS_14_31);
		//	TURN_ON(OPTION_ADD_SYMBOLS_MISC);
			TURN_ON(OPTION_USE_CONTAINERS);
			minWordFreq=6;
			TURN_ON(OPTION_NUMBER_CONTAINER);
			TURN_ON(OPTION_LETTER_CONTAINER);
			break;
		case LZMA: // for LZMA there are different codeWords 
			TURN_ON(OPTION_USE_CONTAINERS);
			TURN_ON(OPTION_SPACELESS_WORDS);
//			TURN_ON(OPTION_ADD_SYMBOLS_0_5);
//			TURN_ON(OPTION_ADD_SYMBOLS_14_31);
//			TURN_ON(OPTION_ADD_SYMBOLS_MISC);
			minWordFreq=6;
			TURN_ON(OPTION_NUMBER_CONTAINER);
			break;
		case PPM:
			TURN_ON(OPTION_SPACE_AFTER_CC_FLAG);
			TURN_ON(OPTION_ADD_SYMBOLS_MISC);
			TURN_ON(OPTION_ADD_SYMBOLS_0_5);
			TURN_ON(OPTION_ADD_SYMBOLS_14_31);
		case PAQ:
			TURN_ON(OPTION_NUMBER_CONTAINER);
			tryShorterBound=4;
			minWordFreq=64;
			break;
	}

	int optCount=0;
	int backArgc=argc;
	char** backArgv=argv;
	EPreprocessType newPreprocType=preprocType;

	while (argc>1 && (argv[1][0]=='-' || argv[1][0]=='+')) 
	{
		switch (argv[1][1])
		{
			case 'h':
				if (argv[1][0]=='+')
				{
                    minWordFreq=100000;
                    TURN_ON(OPTION_QUOTES_MODELING);
                    TURN_ON(OPTION_CRLF);
                    TURN_ON(OPTION_END_TAG_OMISSION);
                    TURN_ON(OPTION_USE_DICTIONARY); // +d
                    TURN_OFF(OPTION_ADD_SYMBOLS_MISC);
                    TURN_OFF(OPTION_USE_CONTAINERS); // -c
                    TURN_OFF(OPTION_LETTER_CONTAINER); // -w
                    TURN_OFF(OPTION_SPACES_MODELING); // -s

					if (firstTime)
						VERBOSE(("* Turn on HTML optimization\n"));
				}
				break;
			case 'r':
				if (argv[1][0]=='+')
				{
					TURN_ON(OPTION_RTF_SUPPORT);
					if (firstTime)
						VERBOSE(("* Turn on RTF optimization\n"));
				}
				break;
			case 'o':
				if (argv[1][0]=='-')
				{
					YesToAll=true;
					if (firstTime)
						VERBOSE(("* Force overwrite of output files is turned on\n"));
				}
				break;
			case 'i':
				if (argv[1][0]=='-')
				{
					deleteInputFiles=true;
					if (firstTime)
						VERBOSE(("* Delete input files is turned on\n"));
				}
				break;
			case 'e':
				if (argv[1][0]=='-')
				{
					maxDictSize=CLAMP(atoi(argv[1]+2),0,65535*32700);
					if (firstTime)
						VERBOSE(("* Maximum dictionary size is %d\n",maxDictSize));
				}
				break;
			case 'f':
				if (argv[1][0]=='-')
				{
					minWordFreq=CLAMP(atoi(argv[1]+2),1,65535);
					minWordFreqChanged=true;
					if (firstTime)
						VERBOSE(("* Minimal word frequency is %d\n",minWordFreq));
				}
				break;
			case 'm':
				if (argv[1][0]=='-')
				{
					maxMemSize=CLAMP(atoi(argv[1]+2),1,255);

					if (firstTime)
						VERBOSE(("* Maximum memory buffer size is %d MB\n",maxMemSize));
					maxMemSize*=1024*1024;
				}
				break;

			case 'b':
				if (argv[1][0]=='-')
				{
					maxDynDictBuf=CLAMP(atoi(argv[1]+2),1,255);

					if (firstTime)
						VERBOSE(("* Maximum buffer for creating dynamic dictionary size is %d MB\n",maxDynDictBuf));
				}
				break;

			case 'l':
				if (argv[1][0]=='-')
				{
					compLevel=CLAMP(atoi(argv[1]+2),0,14);

					if (firstTime)
						VERBOSE(("* Compression level=%d\n",compLevel));

					TURN_OFF(OPTION_ZLIB);
					TURN_OFF(OPTION_LZMA);
					TURN_OFF(OPTION_PPMD);
					TURN_OFF(OPTION_PAQ);

					switch (compLevel)
					{
						case 0:
							break;
#ifdef USE_ZLIB_LIBRARY
						case 1:
							TURN_ON(OPTION_ZLIB);
							newPreprocType=LZ77;
							additionalParam=1;
							break;
						case 2:
							TURN_ON(OPTION_ZLIB);
							newPreprocType=LZ77;
							additionalParam=6;
							break;
						case 3:
							TURN_ON(OPTION_ZLIB);
							newPreprocType=LZ77;
							additionalParam=9;
							break;
#else
						case 1:
						case 2:
						case 3:
							VERBOSE(("warning: ZLIB compression not supported in this compilation!\n"));
							break;
#endif
#ifdef USE_LZMA_LIBRARY
						case 4:
						case 5:
						case 6:
							TURN_ON(OPTION_LZMA);
							newPreprocType=LZMA;

							if (compLevel==4)
								additionalParam=0;
							else
							if (compLevel==5)
								additionalParam=1;
							else
								additionalParam=8;
							break;
#else
						case 4:
						case 5:
						case 6:
							VERBOSE(("warning: LZMA compression not supported in this compilation!\n"));
							break;
#endif
#ifdef USE_PPMD_LIBRARY
						case 7:
						case 8:
						case 9:
							TURN_ON(OPTION_PPMD);
							newPreprocType=PPM;

							if (compLevel==7)
							{
								additionalParam=16;
								if (!minWordFreqChanged)
									minWordFreq=6;
							}
							else
							if (compLevel==8)
							{
								additionalParam=32;
								if (!minWordFreqChanged)
									minWordFreq=16;
							}
							else
							{
								additionalParam=64;
								if (!minWordFreqChanged)
									minWordFreq=64;
							}
							break;
#else
						case 7:
						case 8:
						case 9:
							VERBOSE(("warning: PPMD compression not supported in this compilation!\n"));
							break;
#endif
#ifdef USE_PAQ_LIBRARY
						case 10:
							TURN_ON(OPTION_PAQ);
							additionalParam=5;
							newPreprocType=PAQ;
							break;
						case 11:
							TURN_ON(OPTION_PAQ);
							additionalParam=6;
							newPreprocType=PAQ;
							break;
						case 12:
							TURN_ON(OPTION_PAQ);
							additionalParam=7;
							newPreprocType=PAQ;
							break;
						case 13:
							TURN_ON(OPTION_PAQ);
							additionalParam=8;
							newPreprocType=PAQ;
							break;
						case 14:
							TURN_ON(OPTION_PAQ);
							additionalParam=9;
							newPreprocType=PAQ;
							break;
#else
						case 10:
						case 11:
						case 12:
						case 13:
						case 14:
							VERBOSE(("warning: lpaq6 compression not supported in this compilation!\n"));
							break;
#endif
					}

				}
				break;

			case 's':
				if (argv[1][0]=='-')
				{
					if (firstTime)
						VERBOSE(("* Spaces modeling is off\n"));
					TURN_OFF(OPTION_SPACES_MODELING);
				}
				break;
			case 'd':
				if (argv[1][0]=='+')
				{
					if (firstTime)
						VERBOSE(("* Use static dictionary option is on\n"));
					TURN_ON(OPTION_USE_DICTIONARY);
				}
				break;
			case 't':
				if (argv[1][0]=='-')
				{
					if (firstTime)
						VERBOSE(("* Try shoter word option is off\n"));
					TURN_OFF(OPTION_TRY_SHORTER_WORD);
				}
				break;
			case 'c':
				if (argv[1][0]=='-')
				{
					if (firstTime)
						VERBOSE(("* Use containers is off\n"));
					TURN_OFF(OPTION_USE_CONTAINERS);
				}
				break;
			case 'a':
				if (argv[1][0]=='-')
				{
					if (firstTime)
						VERBOSE(("* Spaceless word model is off\n"));
					TURN_OFF(OPTION_SPACELESS_WORDS);
				}
				break;
			case 'n':
				if (argv[1][0]=='-')
				{
					if (firstTime)
						VERBOSE(("* Number encoding is off\n"));
					TURN_OFF(OPTION_NUMBER_CONTAINER);
				}
				break;
			case 'w':
				if (argv[1][0]=='-')
				{
					if (firstTime)
						VERBOSE(("* Word conntainers are off\n"));
					TURN_OFF(OPTION_LETTER_CONTAINER);
				}
				break;
			case 'p':
				if (argv[1][0]=='-')
				{
					firstPassBlock=CLAMP(atoi(argv[1]+2),1,255);
					if (firstTime)
						VERBOSE(("* Preprocess only (file_size/%d) bytes in a first pass\n",firstPassBlock));
				}
				else
				if (argv[1][0]=='+')
				{
					TURN_ON(OPTION_PDF_SUPPORT);
					if (firstTime)
						VERBOSE(("* Turn on PDF optimization\n"));
				}
				break;
			case '0':
			case '1':
			case '2':
			case '3':
                compLevel = 0;
				TURN_OFF(OPTION_ZLIB);
				TURN_OFF(OPTION_LZMA);
				TURN_OFF(OPTION_PPMD);
				TURN_OFF(OPTION_PAQ);

				if (firstTime)
				switch (argv[1][1])
				{
					case '0': VERBOSE(("* LZ77 optimized preprocessing\n")); break;
					case '1': VERBOSE(("* LZMA/BWT optimized preprocessing\n")); break;
					case '2': VERBOSE(("* PPM optimized preprocessing\n")); break;
					case '3': VERBOSE(("* PAQ optimized preprocessing\n")); break;
				}

				newPreprocType=(EPreprocessType)(argv[1][1]-'0');
				break;
			default:
				if (firstTime)
					VERBOSE(("* Option %s ignored\n", argv[1]));
		}
		argc--;
		optCount++;
		argv++;
	}

	firstTime=false;

	if (newPreprocType!=preprocType)
	{
		preprocType=newPreprocType;
		return defaultSettings(backArgc,backArgv);
	}

	return optCount;
}



std::string XWRT_Common::getSourcePath()
{
    std::string str;
	char buf[STRING_MAX_SIZE];

#ifdef WINDOWS
	int pos;

	pos=GetModuleFileName(NULL,buf,STRING_MAX_SIZE);

	if (pos>0)
	{	
		for (int i=pos-1; i>=0; i--)
			if (buf[i]=='\\')
			{
				buf[i+1]=0;
				pos=i+1;
				break;
			}
	}

    str.append(buf, pos);
#endif
	return str;
}

size_t fread_fast(unsigned char* dst, int len, FILE* file)
{
	return fread(dst,1,len,file);

/*	int rd;
	size_t sum=0;

	while (len > 1<<17) // 128 kb
	{
		rd=fread(dst,1,1<<17,file);
		dst+=rd;
		len-=rd;
		sum+=rd;
	}

	sum+=fread(dst,1,len,file);

	return sum;*/
}

size_t fwrite_fast(unsigned char* dst, int len, FILE* file)
{
	return fwrite(dst,1,len,file);

/*	int wt;
	size_t sum=0;

	while (len > 1<<17) // 128 kb
	{
		wt=fwrite(dst,1,1<<17,file);
		dst+=wt;
		len-=wt;
		sum+=wt;
	}

	sum+=fwrite(dst,1,len,file);

	return sum;*/
}

void format(std::string& s,const char* formatstring, ...) 
{
   char buff[1024];
   va_list args;
   va_start(args, formatstring);

#ifdef WIN32
   _vsnprintf( buff, sizeof(buff), formatstring, args);
#else
   vsnprintf( buff, sizeof(buff), formatstring, args);
#endif

   va_end(args);

   s=buff;
}
