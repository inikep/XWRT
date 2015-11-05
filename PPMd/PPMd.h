/****************************************************************************
 *  This file is part of PPMd project                                       *
 *  Written and distributed to public domain by Dmitry Shkarin 1997,        *
 *  1999-2001, 2006                                                         *
 *  Contents: interface to encoding/decoding routines                       *
 *  Comments: this file can be used as an interface to PPMd module          *
 *  (consisting of Model.cpp) from external program                         *
 ****************************************************************************/
#if !defined(_PPMD_H_)
#define _PPMD_H_

#include "PPMdType.h"

#ifdef  __cplusplus
extern "C" {
#endif

BOOL _STDCALL StartSubAllocator(UINT SubAllocatorSize);
void _STDCALL StopSubAllocator();           /* it can be called once        */
UINT _STDCALL GetUsedMemory();              /* for information only         */

/****************************************************************************
 * (MaxOrder == 1) parameter value has special meaning, it does not restart *
 * model and can be used for solid mode archives;                           *
 * Call sequence:                                                           *
 *     StartSubAllocator(SubAllocatorSize);                                 *
 *     EncodeFile(SolidArcFile,File1,MaxOrder,TRUE);                        *
 *     EncodeFile(SolidArcFile,File2,       1,TRUE);                        *
 *     ...                                                                  *
 *     EncodeFile(SolidArcFile,FileN,       1,TRUE);                        *
 *     StopSubAllocator();                                                  *
 ****************************************************************************/
void _STDCALL EncodeFile(_PPMD_FILE* EncodedFile,_PPMD_FILE* DecodedFile,
                        int MaxOrder,BOOL CutOff);
void _STDCALL DecodeFile(_PPMD_FILE* DecodedFile,_PPMD_FILE* EncodedFile,
                        int MaxOrder,BOOL CutOff);

/*  imported function                                                       */
void _STDCALL  PrintInfo(_PPMD_FILE* DecodedFile,_PPMD_FILE* EncodedFile);

#ifdef  __cplusplus
}
#endif

#endif /* !defined(_PPMD_H_) */
