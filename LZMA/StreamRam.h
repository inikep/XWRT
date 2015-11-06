// StreamRam.h

#ifndef __LzmaRam_h
#define __LzmaRam_h

#include <stdlib.h>
#include "./Common/Types.h"

class CInStreamRam: 
  public ISequentialInStream,
  public CMyUnknownImp
{
  const Byte *Data;
  size_t Size;
  size_t Pos;
public:
  MY_UNKNOWN_IMP
  void Init(const Byte *data, size_t size)
  {
    Data = data;
    Size = size;
    Pos = 0;
  }
  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
};


class COutStreamRam: 
  public ISequentialOutStream,
  public CMyUnknownImp
{
  size_t Size;
public:
  Byte *Data;
  size_t Pos;
  bool Overflow;
  void Init(Byte *data, size_t size)
  {
    Data = data;
    Size = size;
    Pos = 0;
    Overflow = false;
  }
  void SetPos(size_t pos)
  {
    Overflow = false;
    Pos = pos;
  }
  MY_UNKNOWN_IMP
  HRESULT WriteByte(Byte b)
  {
    if (Pos >= Size)
    {
      Overflow = true;
      return E_FAIL;
    }
    Data[Pos++] = b;
    return S_OK;
  }
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

#endif
