#include "XWRT.h"
#include <time.h>
#include <vector>
#include "Encoder.h"
#include "Decoder.h"

#ifdef WINDOWS
	#include <windows.h> // createFileList
	#include <conio.h> // getch()
#else
	#define getch getchar
#endif

unsigned int bytesRead=0,bytesWritten=0; //supports up to 4GB
int g_fileLenMB;
bool YesToAll=false;

void printStatus(int add_read,int add_written,int encoding) 
{  // print progress
	bytesRead+=add_read;
	bytesWritten+=add_written;
	if (encoding)
		PRINT_STATUS(("%10u (%3d%%) -> %10u \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", bytesRead,(g_fileLenMB>0)?(bytesRead/1024/1024*100)/g_fileLenMB:0,bytesWritten))
	else
		PRINT_STATUS(("%10u -> %10u (%3d%%) \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", bytesRead,bytesWritten,(g_fileLenMB>0)?(bytesWritten/1024/1024*100)/g_fileLenMB:0));
}


bool tryOpen(const char* FName, bool& YesToAll)
{
    FILE* fp=fopen(FName,"rb");
    if (!fp)                              
		return true;
    fclose(fp);

    if (YesToAll)
	{
		remove(FName);
		return true;
	}

    VERBOSE(("%s already exists, overwrite?: <Y>es, <N>o, <A>ll, <Q>uit?",FName));
    for ( ; ; )
        switch ( toupper(getch()) ) {
            case 'A':
				YesToAll=true;
            case '\r': 
			case 'Y':
				remove(FName);
				return true;
            case 0x1B: 
			case 'Q':
				VERBOSE(("quit\n")); exit(-1);
            case 'N':                       
				return false;
        }
}


void start_encode(XWRT_Encoder &xml_wrt,FILE* file,char* filename)
{
	FILE* fileout;
	char fileoutbuf[32*1024];
	clock_t start_file;  // in ticks
	std::string outputFilename;

	bytesRead=bytesWritten=0;

	start_file=clock();

    format(outputFilename, "%s" ADD_FILENAME_EXT "%d", filename, xml_wrt.compLevel);

	if (!tryOpen(outputFilename.c_str(),xml_wrt.YesToAll))
		return;
	
#ifdef USE_LZMA_LIBRARY
    int preprocFlag=xml_wrt.preprocFlag;
	if (IF_OPTION(OPTION_LZMA))
	{
		if (!LZMAlib_PrepareOutputFile((char*)outputFilename.c_str(),xml_wrt.outStream))
		{
			VERBOSE(("Can't open file %s\n",outputFilename.c_str()));
			return;
		}
		fileout=(FILE*)1;
	}
	else
#endif
	{
		fileout=fopen(outputFilename.c_str(),"wb");
		if (fileout==NULL)
		{
			VERBOSE(("Can't open file %s\n",outputFilename.c_str()));
			fclose(file);
			return;
		}
		setvbuf(fileout, fileoutbuf, _IOFBF, sizeof(fileoutbuf));
	}
			
	std::string compName;
	xml_wrt.getAlgName(compName);
	VERBOSE(("- encoding %s to %s (%s)\n",filename,outputFilename.c_str(),compName.c_str()));
					
	int flen=xml_wrt.flen(file);
	XWRT_file=file;
	XWRT_fileout=fileout;
	fseek(XWRT_file, 0, SEEK_SET );
	xml_wrt.WRT_start_encoding(flen,false);

    float tim=float(clock()-start_file)/CLOCKS_PER_SEC;
#ifdef USE_LZMA_LIBRARY
    if (IF_OPTION(OPTION_LZMA))
        VERBOSE((" + encoding finished (%d->%d bytes, %.03f bpc) in %.02fs (%.0f kb/s)\n",ftell(file),LZMAlib_GetOutputFilePos(xml_wrt.outStream),LZMAlib_GetOutputFilePos(xml_wrt.outStream)*8.0/ftell(file),tim,ftell(file)/1024/tim));
    else
#endif
        VERBOSE((" + encoding finished (%d->%d bytes, %.03f bpc) in %.02fs (%.0f kb/s)\n",ftell(file),ftell(fileout),ftell(fileout)*8.0/ftell(file),tim,ftell(file)/1024/tim));
				
	fclose(file);


#ifdef USE_LZMA_LIBRARY
	if (IF_OPTION(OPTION_LZMA))
		LZMAlib_CloseOutputFile(xml_wrt.outStream);
	else
#endif
		fclose(fileout);

		
	if (xml_wrt.deleteInputFiles)
		remove(filename);

	if (!YesToAll)
		YesToAll=xml_wrt.YesToAll;
}

void start_decode(XWRT_Decoder& xml_wrt,FILE* file,char* filename)
{
	FILE* fileout;
	char fileoutbuf[32*1024];
	clock_t start_file;  // in ticks

	bytesRead=bytesWritten=0;


	start_file=clock();

	if (getc(file)!=XWRT_HEADER[3])
	{
		printf("Bad XWRT version!\n");
		return;
	}

    int c;
	if ((c=getc(file))!=XWRT_VERSION-150)
	{
        c += 150;
		printf("Bad XWRT version, you need XWRT %d.%d!\n", c/100, (c/10)%10);
		return;
	}

	c=getc(file); // 6th header byte
	int preprocFlag=0;
	if ((c&8)!=0)
		TURN_ON(OPTION_LZMA);

	std::string outputFilename=filename;
	size_t pos=outputFilename.rfind(CUT_FILENAME_CHAR);
	if (pos!=std::string::npos && pos>0)
	{
		outputFilename.resize(pos);
		outputFilename+=AFTER_DECOMPRESED_NAME;
	}
	else
		outputFilename+="_decomp";
	
	if (!tryOpen(outputFilename.c_str(),xml_wrt.YesToAll))
		return;
	
	fileout=fopen(outputFilename.c_str(),"wb");
	if (fileout==NULL)
	{
		VERBOSE(("Can't open file %s\n",outputFilename.c_str()));
		fclose(file);
		return;
	}
	setvbuf(fileout, fileoutbuf, _IOFBF, sizeof(fileoutbuf));
	
	
#ifdef USE_LZMA_LIBRARY
	if (IF_OPTION(OPTION_LZMA))
	{
		fclose(file);
		if (!LZMAlib_PrepareInputFile(filename,xml_wrt.inStream))
		{
			VERBOSE(("Can't open file %s\n",outputFilename.c_str()));
			return;
		}	

		for (int i=0;i<6; i++)
			c=LZMAlib_getc(xml_wrt.inStream);  // XWRT header + ver + 1 byte
	}
#endif

	XWRT_file=file;
	XWRT_fileout=fileout;
	xml_wrt.WRT_start_decoding(c,filename,(char*)outputFilename.c_str());
	
    float tim=float(clock()-start_file)/CLOCKS_PER_SEC;

#ifdef USE_LZMA_LIBRARY
    if (IF_OPTION(OPTION_LZMA))
        VERBOSE((" + decoding finished (%d->%d bytes, %.03f bpc) in %.02fs (%.0f kb/s)\n",LZMAlib_GetInputFilePos(xml_wrt.inStream),ftell(fileout),LZMAlib_GetInputFilePos(xml_wrt.inStream)*8.0/ftell(fileout),tim,ftell(fileout)/1024/tim));
    else
#endif
        VERBOSE((" + decoding finished (%d->%d bytes, %.03f bpc) in %.02fs (%.0f kb/s)\n",ftell(file),ftell(fileout),ftell(file)*8.0/ftell(fileout),tim,ftell(fileout)/1024/tim));

	fclose(fileout);

#ifdef USE_LZMA_LIBRARY
	if (IF_OPTION(OPTION_LZMA))
		LZMAlib_CloseInputFile(xml_wrt.inStream);
	else
#endif
		fclose(file);

	
		
	if (xml_wrt.deleteInputFiles)
		remove(filename);

	if (!YesToAll)
		YesToAll=xml_wrt.YesToAll;
}


bool first_decode=true;
bool first_encode=true;

void processFile(char* filename,int argc, char* argv[],XWRT_Encoder &xml_wrt, XWRT_Decoder &xml_wrtd)
{
	FILE* file;
	char filebuf[32*1024];

	file=fopen((const char*)filename,"rb");
	if (file==NULL)
	{
		VERBOSE(("Can't open file %s\n",filename));
		return;
	}
	
	setvbuf(file, filebuf, _IOFBF, sizeof(filebuf));
	
	
	if (getc(file)==XWRT_HEADER[0] && getc(file)==XWRT_HEADER[1] && 
		getc(file)==XWRT_HEADER[2])
	{
        xml_wrtd.YesToAll=YesToAll;
        xml_wrtd.defaultSettings(argc,argv);
        start_decode(xml_wrtd,file,filename);
	}
	else
	{
        xml_wrt.YesToAll=YesToAll;
        xml_wrt.defaultSettings(argc,argv);
        start_encode(xml_wrt,file,filename);
	}
}

#ifdef WINDOWS

std::vector<std::string> list;

bool createFileList(char* pattern)
{
	std::string str;
	std::string path;
    WIN32_FIND_DATA c_file;
    HANDLE hFile=FindFirstFile( pattern, &c_file );

	path=pattern;
	if (path.find_last_of('\\')==std::string::npos &&
		path.find_last_of('/')==std::string::npos)
		path.erase();
	if ((int)path.find_last_of('\\')>(int)path.find_last_of('/'))
	{
		if (path.find_last_of('\\')!=std::string::npos)
			path.resize(path.find_last_of('\\'));
	}
	else
	{
		if (path.find_last_of('/')!=std::string::npos)
			path.resize(path.find_last_of('/'));
	}

	if (hFile != INVALID_HANDLE_VALUE) 
	do
	{
		if (path.size()>0)
			str=path+'\\'+c_file.cFileName;
		else
			str=c_file.cFileName;

//		if (patt[patt.size()-1]=='*' || tolower(str[str.size()-1])==tolower(patt[patt.size()-1]))
		if (!(c_file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			list.push_back(str);
	}
	while (FindNextFile( hFile, &c_file ));

	FindClose( hFile );

	return true;
}

#endif


int getOptionsCount(int argc, char* argv[])
{
	int optCount=0;

	while (argc>1 && (argv[1][0]=='-' || argv[1][0]=='+')) 
	{
		argc--;
		optCount++;
		argv++;
	}

	return optCount;
}

void usage()
{
	VERBOSE(("Usage: " XWRT_SHORT ".exe [options] <file1> [file2] [file3] ...\n"));
	VERBOSE((" where <file> is a XML file or a " XWRT_SHORT " compressed file (it's auto-detected)\n"));
	VERBOSE((" 	     you can also use wildcards (e.g., \"*.xml\")\n"));
	VERBOSE((" GENERAL OPTIONS (which also set default additional options):\n"));
	VERBOSE(("  -i = Delete input files\n"));
	VERBOSE(("  -lX = Set compression level (0=store"));
#ifdef USE_ZLIB_LIBRARY
	VERBOSE(("; 1,2,3=zlib [fast,normal(default),best]"));
#endif
#if defined(USE_LZMA_LIBRARY) || defined(USE_PPMD_LIBRARY) || defined(USE_PAQ_LIBRARY)
	VERBOSE((";\n     "));
#endif
#ifdef USE_LZMA_LIBRARY
	VERBOSE(("4,5,6=LZMA [64 KB, 1 MB, 8 MB]; "));
#endif
#ifdef USE_PPMD_LIBRARY
	VERBOSE(("7,8,9=PPMd [16 MB, 32 MB, 64 MB];\n     "));
#endif
#ifdef USE_PAQ_LIBRARY
	VERBOSE(("10,11,12,13,14=lpaq6 [104 MB, 198 MB, 390 MB, 774 MB, 1542 MB]"));
#endif
	VERBOSE((")\n"));
	VERBOSE(("  -o = Force overwrite of output files\n"));
	VERBOSE(("  -0, -1, -2, -3 = -l0 (store) optimized for further LZ77, LZMA, PPM, PAQ (-3)\n"));

	VERBOSE((" ADDITIONAL OPTIONS:\n"));
	VERBOSE(("  -a  = Turn off spaceless word model\n"));
	VERBOSE(("  -bX = Set maximum buffer size while creating dynamic dictionary to X MB\n"));
	VERBOSE(("  -c = Turn off containers (without number and word containers)\n"));
	VERBOSE(("  +d = Turn on usage of the static dictionary (requires wrt-eng.dic,\n"));
	VERBOSE(("       which is available at http://pskibinski.pl/research)\n"));
	VERBOSE(("  -eX = Set maximum dictionary size to X words\n"));
	VERBOSE(("  -fX = Set minimal word frequency to X\n"));
	VERBOSE(("  +h = Turn on HTML optimization\n"));
	VERBOSE(("  -mX = Set maximum memory buffer size to X MB (default=8)\n"));
	VERBOSE(("  -n = Turn off number containers\n"));
	VERBOSE(("  -pX = Preprocess only (file_size/X) bytes in a first pass\n"));
	VERBOSE(("  +p = Turn on PDF optimization\n"));
	VERBOSE(("  +r = Turn on RTF/TeX optimization\n"));
	VERBOSE(("  -s = Turn off spaces modeling option\n"));
	VERBOSE(("  -t = Turn off \"try shorter word\" option\n"));
	VERBOSE(("  -w = Turn off word containers\n"));
}


int main(int argc, char* argv[])
{
	int optCount;

	VERBOSE((PRG_NAME "\n"));

	clock_t start_time;  // in ticks
    start_time=clock();

	optCount=getOptionsCount(argc,argv);


	if (argc-optCount<2)
	{
		usage();
//		VERBOSE(("sizeof(void*)=%d\n",sizeof(void*));
		return 0;
	};

	XWRT_Encoder xml_wrt;
	XWRT_Decoder xml_wrtd;

	int curr=optCount+1;

	while (curr<argc)
	{
#ifdef WINDOWS
		createFileList(argv[curr]);
#else
		processFile(argv[curr], argc, argv, xml_wrt, xml_wrtd);
#endif
		curr++;
	}	

#ifdef WINDOWS
	std::vector<std::string>::iterator it;
	if (list.size()<=0)
		VERBOSE(("Can't open input file(s)\n"));
	else
	for (it=list.begin(); it!=list.end(); it++)
		processFile((char*)it->c_str(), argc, argv, xml_wrt, xml_wrtd);
//		VERBOSE(("%s\n",it->c_str());
#endif

	if (argc>optCount+2 || list.size()>1)
	    VERBOSE(("+ Total time %1.2f sec\n",double(clock()-start_time)/CLOCKS_PER_SEC));

	return 0;
};

