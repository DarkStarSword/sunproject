
#ifndef __PUBLICSTUFF_H__
#define __PUBLICSTUFF_H__


#include <windows.h>
#include "BitmapOps.h"

bool DoEvents(HWND);

extern bool bRunning;
extern bool bDrawing;

extern BitmapOps *SSSOldBitmapPtr;
extern BitmapOps *SSSUndoBitmapPtr;
extern BitmapOps *SSSBufferBitmapPtr;

extern BOOL NoPerlin;
extern BOOL NoFade;


#endif