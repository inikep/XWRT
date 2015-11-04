#if !defined lpaq6_h_included
#define lpaq6_h_included

#pragma warning(disable:4554)

#define WIKI

#include <stdio.h>

// 8, 16, 32 bit unsigned types (adjust as appropriate)
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned int   U32;

// min, max functions
#ifndef min
inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}
#endif

template <int B>
class HashTable {
  U8* t;	// table: 1 element = B bytes: checksum,priority,data,data,...
  const int NB;	// size in bytes
public:
  HashTable(int n);
  U8* get(U32 i);
#ifdef WIKI
  U8* get0(U32 i);
  U8* get1(U32 i);
#endif
};

#define COMPRESS	1
#define DECOMPRESS	2

class Predictor;
class MatchModel;

class Encoder {
public:
#ifdef WIKI
  HashTable<16> t4a;  // cxt -> state
  HashTable<16> t4b;  // cxt -> state
#define t1a t4a
#define t2a t4a
#define t3a t4a
#define t1b t4b
#define t2b t4b
#define t3b t4b
#else
  HashTable<16> t1;  // cxt -> state
  HashTable<16> t2;  // cxt -> state
  HashTable<16> t3;  // cxt -> state
#define t1a t1
#define t2a t2
#define t3a t3
#define t1b t1
#define t2b t2
#define t3b t3
#endif
  MatchModel* mm;	// predicts next bit by matching context
  FILE *archive;	// Compressed data file
  U32 x1, x2;		// Range, initially [0, 1), scaled by 2^32
  U32 x;		// Decompress mode: last 4 input bytes of archive
  U32 p;
  int *add2order;
  U32 Predictor_upd(int y);
  Encoder(int m, FILE* f);
  ~Encoder();
  void flush();  // call this when compression is finished
  void compress(int c);
  int decompress();
};

void set_PAQ_level(int level);

#endif
