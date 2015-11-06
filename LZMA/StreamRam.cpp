// StreamRam.cpp

#include "StdAfx.h"
#include "./Common/Types.h"
#include "./LZMADecoder.h"
#include "./LZMAEncoder.h"
#include "StreamRam.h"



STDMETHODIMP CInStreamRam::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  UInt32 remain = Size - Pos;
  if (size > remain)
    size = remain;
  for (UInt32 i = 0; i < size; i++)
    ((Byte *)data)[i] = Data[Pos + i];
  Pos += size;
  if(processedSize != NULL)
    *processedSize = size;
  return S_OK;
}

STDMETHODIMP COutStreamRam::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  UInt32 i;
  for (i = 0; i < size && Pos < Size; i++)
    Data[Pos++] = ((const Byte *)data)[i];
  if(processedSize != NULL)
    *processedSize = i;
  if (i != size)
  {
    Overflow = true;
    return E_FAIL;
  }
  return S_OK;
}
  
