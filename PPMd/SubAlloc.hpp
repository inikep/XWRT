/****************************************************************************
 *  This file is part of PPMd project                                       *
 *  Written and distributed to public domain by Dmitry Shkarin 1997,        *
 *  1999-2001, 2006                                                         *
 *  Contents: memory allocation routines                                    *
 ****************************************************************************/
enum { UNIT_SIZE=12, N1=4, N2=4, N3=4, N4=(128+3-1*N1-2*N2-3*N3)/4,
        N_INDEXES=N1+N2+N3+N4 };

inline void PrefetchData(void* Addr)
{
#if defined(_USE_PREFETCHING)
    BYTE PrefetchByte = *(volatile BYTE*)Addr;
#endif /* defined(_USE_PREFETCHING) */
}
static BYTE Indx2Units[N_INDEXES], Units2Indx[128]; // constants
static _THREAD1 UINT _THREAD GlueCount, _THREAD GlueCount1, _THREAD SubAllocatorSize=0;
static _THREAD1 _BYTE* _THREAD HeapStart, * _THREAD pText, * _THREAD UnitsStart;
static _THREAD1 _BYTE* _THREAD LoUnit, * _THREAD HiUnit, * _THREAD AuxUnit;

#if defined(_32_NORMAL) || defined(_64_EXOTIC)
inline _DWORD Ptr2Indx(void* p) { return (_DWORD)p; }
inline void*  Indx2Ptr(_DWORD indx) { return (void*)indx; }
#else
static _THREAD1 _BYTE* _THREAD HeapNull;
inline _DWORD Ptr2Indx(void* p) { return ((_BYTE*)p)-HeapNull; }
inline void*  Indx2Ptr(_DWORD indx) { return (void*)(HeapNull+indx); }
#endif /* defined(_32_NORMAL) || defined(_64_EXOTIC) */

#pragma pack(1)
static _THREAD1 struct BLK_NODE {
    _DWORD Stamp;
    _PAD_TO_64(Dummy1)
    _DWORD NextIndx;
    _PAD_TO_64(Dummy2)
    BLK_NODE*   getNext() const      { return (BLK_NODE*)Indx2Ptr(NextIndx); }
    void        setNext(BLK_NODE* p) { NextIndx=Ptr2Indx(p); }
    BOOL          avail() const      { return (NextIndx != 0); }
    void           link(BLK_NODE* p) { p->NextIndx=NextIndx; setNext(p); }
    void         unlink()            { NextIndx=getNext()->NextIndx; }
    inline void* remove();
    inline void  insert(void* pv,int NU);
} _THREAD BList[N_INDEXES+1];
struct MEM_BLK: public BLK_NODE { _DWORD NU; _PAD_TO_64(Dummy3) };
#pragma pack()

inline void* BLK_NODE::remove() {
    BLK_NODE* p=getNext();                  unlink();
    Stamp--;                                return p;
}
inline void BLK_NODE::insert(void* pv,int NU) {
    MEM_BLK* p=(MEM_BLK*)pv;                link(p);
    p->Stamp=~_DWORD(0);                    p->NU=NU;
    Stamp++;
}
inline UINT U2B(UINT NU) { return 8*NU+4*NU; }
inline void SplitBlock(void* pv,UINT OldIndx,UINT NewIndx)
{
    UINT i, k, UDiff=Indx2Units[OldIndx]-Indx2Units[NewIndx];
    _BYTE* p=((_BYTE*)pv)+U2B(Indx2Units[NewIndx]);
    if (Indx2Units[i=Units2Indx[UDiff-1]] != UDiff) {
        k=Indx2Units[--i];                  BList[i].insert(p,k);
        p += U2B(k);                        UDiff -= k;
    }
    BList[Units2Indx[UDiff-1]].insert(p,UDiff);
}
UINT _STDCALL GetUsedMemory()
{
    UINT i, RetVal=SubAllocatorSize-(HiUnit-LoUnit)-(UnitsStart-pText);
    for (i=0;i < N_INDEXES;i++)
            RetVal -= U2B(Indx2Units[i]*BList[i].Stamp);
    return RetVal;
}
void _STDCALL StopSubAllocator() {
    if ( SubAllocatorSize ) {
        SubAllocatorSize=0;                 delete HeapStart;
    }
}
BOOL _STDCALL StartSubAllocator(UINT SASize)
{
    UINT t=SASize << 20U;
    if (SubAllocatorSize == t)              return TRUE;
    StopSubAllocator();
    if ((HeapStart = new _BYTE[t]) == NULL) return FALSE;
    SubAllocatorSize=t;                     return TRUE;
}
inline void InitSubAllocator()
{
    memset(BList,0,sizeof(BList));
    HiUnit=(pText=HeapStart)+SubAllocatorSize;
    UINT Diff=U2B(SubAllocatorSize/8/UNIT_SIZE*7);
    LoUnit=UnitsStart=HiUnit-Diff;          GlueCount=GlueCount1=0;
#if !defined(_32_NORMAL) && !defined(_64_EXOTIC)
    HeapNull=HeapStart-1;
#endif /* !defined(_32_NORMAL) && !defined(_64_EXOTIC) */
}
inline void GlueFreeBlocks()
{
    UINT i, k, sz;
    MEM_BLK s0, * p, * p0, * p1;
    if (LoUnit != HiUnit)                   *LoUnit=0;
    for ((p0=&s0)->NextIndx=i=0;i <= N_INDEXES;i++)
            while ( BList[i].avail() ) {
                p=(MEM_BLK*)BList[i].remove();
                if ( !p->NU )               continue;
                while ((p1=p+p->NU)->Stamp == ~_DWORD(0)) {
                    p->NU += p1->NU;        p1->NU=0;
                }
                p0->link(p);                p0=p;
            }
    while ( s0.avail() ) {
        p=(MEM_BLK*)s0.remove();            sz=p->NU;
        if ( !sz )                          continue;
        for ( ;sz > 128;sz -= 128, p += 128)
                BList[N_INDEXES-1].insert(p,128);
        if (Indx2Units[i=Units2Indx[sz-1]] != sz) {
            k=sz-Indx2Units[--i];           BList[k-1].insert(p+(sz-k),k);
        }
        BList[i].insert(p,Indx2Units[i]);
    }
    GlueCount=1 << (13+GlueCount1++);
}
static void* _STDCALL AllocUnitsRare(UINT indx)
{
    UINT i=indx;
    do {
        if (++i == N_INDEXES) {
            if ( !GlueCount-- ) {
                GlueFreeBlocks();
                if (BList[i=indx].avail())  return BList[i].remove();
            } else {
                i=U2B(Indx2Units[indx]);
                return (UnitsStart-pText > i)?(UnitsStart -= i):(NULL);
            }
        }
    } while ( !BList[i].avail() );
    void* RetVal=BList[i].remove();         SplitBlock(RetVal,i,indx);
    return RetVal;
}
inline void* AllocUnits(UINT NU)
{
    UINT indx=Units2Indx[NU-1];
    if ( BList[indx].avail() )              return BList[indx].remove();
    void* RetVal=LoUnit;                    LoUnit += U2B(Indx2Units[indx]);
    if (LoUnit <= HiUnit)                   return RetVal;
    LoUnit -= U2B(Indx2Units[indx]);        return AllocUnitsRare(indx);
}
inline void* AllocContext()
{
    if (HiUnit != LoUnit)                   return (HiUnit -= UNIT_SIZE);
    return (BList->avail())?(BList->remove()):(AllocUnitsRare(0));
}
inline void UnitsCpy(void* Dest,void* Src,UINT NU)
{
#if defined(_32_NORMAL) || defined(_64_NORMAL)
    DWORD* p1=(DWORD*)Dest, * p2=(DWORD*)Src;
    do {
        p1[0]=p2[0];                        p1[1]=p2[1];
        p1[2]=p2[2];
        p1 += 3;                            p2 += 3;
    } while ( --NU );
#else
    MEM_BLK* p1=(MEM_BLK*)Dest, * p2=(MEM_BLK*)Src;
    do { *p1++ = *p2++; } while ( --NU );
#endif /* defined(_32_NORMAL) || defined(_64_NORMAL) */
}
inline void* ExpandUnits(void* OldPtr,UINT OldNU)
{
    UINT i0=Units2Indx[OldNU-1], i1=Units2Indx[OldNU-1+1];
    if (i0 == i1)                           return OldPtr;
    void* ptr=AllocUnits(OldNU+1);
    if (ptr) { UnitsCpy(ptr,OldPtr,OldNU);  BList[i0].insert(OldPtr,OldNU); }
    return ptr;
}
inline void* ShrinkUnits(void* OldPtr,UINT OldNU,UINT NewNU)
{
    UINT i0=Units2Indx[OldNU-1], i1=Units2Indx[NewNU-1];
    if (i0 == i1)                           return OldPtr;
    if ( BList[i1].avail() ) {
        void* ptr=BList[i1].remove();       UnitsCpy(ptr,OldPtr,NewNU);
        BList[i0].insert(OldPtr,Indx2Units[i0]);
        return ptr;
    } else { SplitBlock(OldPtr,i0,i1);      return OldPtr; }
}
inline void FreeUnits(void* ptr,UINT NU) {
    UINT indx=Units2Indx[NU-1];
    BList[indx].insert(ptr,Indx2Units[indx]);
}
inline void FreeUnit(void* ptr)
{
    BList[((_BYTE*)ptr > UnitsStart+128*1024)?(0):(N_INDEXES)].insert(ptr,1);
}
inline void* MoveUnitsUp(void* OldPtr,UINT NU)
{
    UINT indx;                              PrefetchData(OldPtr);
    if ((_BYTE*)OldPtr > UnitsStart+128*1024 ||
        (BLK_NODE*)OldPtr > BList[indx=Units2Indx[NU-1]].getNext())
            return OldPtr;
    void* ptr=BList[indx].remove();         UnitsCpy(ptr,OldPtr,NU);
    BList[N_INDEXES].insert(OldPtr,Indx2Units[indx]);
    return ptr;
}
inline void PrepareTextArea()
{
    AuxUnit = (_BYTE*)AllocContext();
    if ( !AuxUnit )                         AuxUnit = UnitsStart;
    else if (AuxUnit == UnitsStart)         AuxUnit = (UnitsStart += UNIT_SIZE);
}
static void ExpandTextArea()
{
    BLK_NODE* p;
    UINT Count[N_INDEXES], i=0;             memset(Count,0,sizeof(Count));
    if (AuxUnit != UnitsStart) {
        if(*(_DWORD*)AuxUnit != ~_DWORD(0)) UnitsStart += UNIT_SIZE;
        else                                BList->insert(AuxUnit,1);
    }
    while ((p=(BLK_NODE*)UnitsStart)->Stamp == ~_DWORD(0)) {
        MEM_BLK* pm=(MEM_BLK*)p;            UnitsStart=(_BYTE*)(pm+pm->NU);
        Count[Units2Indx[pm->NU-1]]++;      i++;
        pm->Stamp=0;
    }
    if ( !i )                               return;
    for (p=BList+N_INDEXES;p->NextIndx;p=p->getNext()) {
        while (p->NextIndx && !p->getNext()->Stamp) {
            Count[Units2Indx[((MEM_BLK*)p->getNext())->NU-1]]--;
            p->unlink();                    BList[N_INDEXES].Stamp--;
        }
        if ( !p->NextIndx )                 break;
    }
    for (i=0;i < N_INDEXES;i++)
        for (p=BList+i;Count[i] != 0;p=p->getNext())
            while ( !p->getNext()->Stamp ) {
                p->unlink();                BList[i].Stamp--;
                if ( !--Count[i] )          break;
            }
}
