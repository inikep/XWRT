#if !defined decoder_h_included
#define decoder_h_included

#include "Common.h"


class XWRT_Decoder : public XWRT_Common
{
public:

	XWRT_Decoder();
	~XWRT_Decoder();

	void WRT_start_decoding(int c,char* filename,char* filenameOut);

private:

	inline void toUpper(unsigned char* s,int &s_size);
	inline unsigned char* uint2string(unsigned int x,unsigned char* buf,int size);
	inline void hook_putc(int& c);
	void WRT_decode();

	void read_dict();
	void WRT_set_options(char c,char c2);
	inline int decodeCodeWord(unsigned char* &s,int& c);
	inline int decodeCodeWord_LZMA(unsigned char* &s,int& c);
	inline int decodeCodeWord_LZ(unsigned char* &s,int& c);

	void writePutcBuffer(FILE* &fileout);

	inline unsigned char mask8(unsigned char oc);
	inline int sequence_length(unsigned char lead_it);
	inline unsigned int utf8_to_unicode(unsigned char* it);

	unsigned char utf8buff[4];
	int utf8pos;

	unsigned char* putcBufferData;
	size_t putcBufferSize;
	unsigned char* putcBuffer;
	unsigned char* zlibBuffer;

	enum EUpperType { UFALSE, UTRUE, FORCE };

	int s_size,WRTd_c;
	int last_c,quotes,lastAll,WRTd_xs_size;
	bool WRTd_upper,forceSpace;
	bool WRTd_initialized;
	unsigned char WRTd_xs[STRING_MAX_SIZE];
	unsigned char WRTd_data[STRING_MAX_SIZE];
	unsigned char *WRTd_s;
	EUpperType upperWord;
	EXMLState XMLState;

public:
#ifdef USE_LZMA_LIBRARY
	CInFileStream* inStream;
	CMyComPtr<ISequentialInStream> iStream;
#else
	CInFileStream* inStream;
#endif

}; // end class 

#endif
