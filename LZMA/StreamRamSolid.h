// StreamRamSolid.h

#ifndef __StreamRamSolid_h
#define __StreamRamSolid_h

#include <stdlib.h>
#include "StreamRam.h"

class CInStreamRamSolid: 
  public CInStreamRam
{
  Byte **Data;
  size_t* Size;
  size_t* Pos;
  int Count;
  int CountMax;
public:
  MY_UNKNOWN_IMP
  void Init(Byte *data[], size_t size[], int count)
  {
 	Pos=new size_t[count];
	Size=new size_t[count];
	Data=(unsigned char**) malloc(sizeof(unsigned char*)*count);

	for (int i=0; i<count; i++)
	{
		Pos[i]=0;
		Data[i] = data[i];
		Size[i] = size[i];
	//	printf("%d %d\n",i,size[i]);
	}

	Count=0;
	CountMax=count-1;
  }

  STDMETHODIMP Read(void *data, UInt32 size, UInt32 *processedSize)
  {
    UInt32 remain = Size[Count] - Pos[Count];
 //   printf("count=%d size=%d remain=%d\n",Count,size,remain);
    if (size > remain)
     size = remain;
	Byte * Dat=Data[Count];
    for (UInt32 i = 0; i < size; i++)
     ((Byte *)data)[i] = Dat[Pos[Count] + i];
    Pos[Count] += size;

    if(processedSize != NULL) *processedSize = size; 

	if (Size[Count] - Pos[Count]==0 && Count<CountMax)
		Count++;

    return S_OK;
  } 
};


#endif
