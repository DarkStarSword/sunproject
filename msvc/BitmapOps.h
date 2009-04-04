/*
 *  Sun Project Interactive
 *  Copyright (C) 2003-2009 Ian Munsie
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __BITMAPOPS_H__
#define __BITMAPOPS_H__

#include <windows.h>
#include <vfw.h>
#include <fstream.h>
#include <cmath>

#pragma comment(lib, "Vfw32.lib")

#define PI 3.1415926535897932384626433832795

typedef unsigned char uchar;


class BitmapOps {
public:

	BitmapOps(long Width, long Height);
	BitmapOps(long Width, long Height, BOOL Alpha); //Alpha = 32BPP
	~BitmapOps();

	//Member functions:
	void CopyImageArrayToDC(HWND);
	void CopyImageArrayToDC(HWND, HDC);
	void CopyImageArrayToDC(HWND hWnd, HDC hDc, int DestX, int DestY, int DestW, int DestH, BOOL Clip = FALSE, int ClipX = 0, int ClipY = 0, int ClipW = 0, int ClipH = 0);
	void SetPColour(long X, long Y, uchar Red, uchar Green, uchar Blue);
	void SetPColour(long X, long Y, uchar Red, uchar Green, uchar Blue, uchar Alpha);
	void SetPColour(long X, long Y, COLORREF Colour);
	DWORD GetPixelIndex(long X, long Y);
	void FadeBitmaps(BitmapOps &Source1, BitmapOps &Source2, double Faded); //ALL BITMAPS MUST BE SAME SIZE, THIS DOES NOT CHECK!!!
	LONG32/*unsigned long*/ Width() {return bmInfo.bmiHeader.biWidth;}
	LONG32/*unsigned long*/ Height() {return bmInfo.bmiHeader.biHeight;}
	unsigned int BPP() {return bmInfo.bmiHeader.biBitCount;}
	bool Resize(long Width, long Height);
	//DWORD UBound() {return bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biHeight * (bmInfo.bmiHeader.biBitCount / 8);} //NOT ZERO BASED -- ie, use "for (int i = 0, i < myBitmapOps.UBound(), ++i)"
	DWORD UBound() {return bmInfo.bmiHeader.biSizeImage;} //NOT ZERO BASED -- ie, use "for (int i = 0, i < myBitmapOps.UBound(), ++i)"
	void LoadImage(LPSTR FileName, BOOL SuppressInfo = FALSE);
	void DumpImage(LPSTR FileName);
	void GetBitmapFromSysMem(/*HDC hDc, */HBITMAP hBitmap);
	void GetBitmapFromResource(WORD ResourceID);
	void CopyBitmapFromBitmapOps(BitmapOps&);
	void CopyBitmapFromBitmapOps(BitmapOps&, LONG32 x, LONG32 y);
	void ResizeBitmapFromBitmapOps(BitmapOps&);
	void CopyAlphaBitmapFromBitmapOps(BitmapOps&, LONG32 x, LONG32 y);
	void CopyAlphaBitmapFromBitmapOps(BitmapOps&, LONG32 DestX, LONG32 DestY, LONG32 DestW, LONG32 DestH, LONG32 SourceX, LONG32 SourceY, LONG32 SourceW, LONG32 SourceH);

	//Primative Drawing functions:
	void Line(LONG32 x1, LONG32 y1, LONG32 x2, LONG32 y2, COLORREF Colour);
	void Rect(LONG32 x1, LONG32 y1, LONG32 x2, LONG32 y2, COLORREF BorderColour, BOOL Transparent, COLORREF FillColour);
	void Ellipse(LONG32 x, LONG32 y, LONG32 xr, LONG32 yr, COLORREF Colour, double Start = 0, double End = 2*PI);

	//Overloaded operators:
	//void operator=(const BitmapOps&);

	uchar *PixelData; //ONLY ALTER THE CONTENTS, This is only here to allow faster, more direct access to the bitmap contents!

private:
	HDRAWDIB hDd;
	HDC hMemDC;
	HBITMAP hMemBitmap;

	BITMAPFILEHEADER bmFileHeader;
	BITMAPINFO bmInfo;
};


#endif
