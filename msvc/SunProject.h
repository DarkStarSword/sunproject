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

#ifndef __SUNPROJECT_H__
#define __SUNPROJECT_H__

#include <windows.h>
#include <cmath>
#include <string>
#include "BitmapOps.h"
#include "PublicStuff.h"

/////AVI
#pragma comment(lib, "Vfw32.lib")
#include <Vfw.h>
/////AVI

#define ANIMPERLINFORMATBITMAPSERIES	0	//Default
#define ANIMPERLINFORMATFRAMESTACK		1	//Unsupported
#define ANIMPERLINFORMATAVI				2	//Experimental

struct AnimationInformation {
	BOOL DrawingAnimation;
	DWORD CurrentFrame;
	DWORD TotalFrames;
};
extern AnimationInformation AnimInfo;

typedef unsigned char uchar;


struct SunProjectNebulaData {
	int KeepPrevious; //0 - Erase, 1 - Underlay Previous Image, 2 - Overlay
	int Interpolation; //0 - Linear, 1 - CoSine, 2 - Cubic, Other - CoSine
	LONG32 LastSeed; //This will store the last seed used, so it can be recovered later if required
	LONG32 Seed; //0 is random
	double Persistance; //A number between 0 and 1 (You can go higher, but it doesn't create good results)
	LONG32 Octaves; //Amount of detail, 0 = Maximum visible
	LONG32 FirstOctave; //Octave to start drawing at
	COLORREF MiddleColour; //Only used if KeepPrevious is 0 or 1
	LONG32 RedVariance;
	LONG32 GreenVariance;
	LONG32 BlueVariance;
	BOOL Bug; //Makes a kind of wood effect
	BOOL Smooth; //Unsupported
	BOOL HTile; //Generate a nebula that can be tiled horizontally
	BOOL VTile; //Generate a nebula that can be tiled vertically
	BOOL UseGradient;
	DWORD NumGradientEntries;
	COLORREF *GradientArray;
};

struct SunProjectNebulaParams {
	int NumLayers; //Max 5
	SunProjectNebulaData LayerParams[5];
	
	BOOL Animate; //Still a little buggy & DOESN'T SUPPORT GRADIENTS
	BOOL Loop; //Generate an animated 'nebula' that loops perfectly
	BOOL SmoothT; //Smooth the time axis to create a nicer transition
	char *BaseFileName; //The filename to save the animation under, appends ####.bmp
	LONG32 KeyFrames; //This is the number of keyframes in the animation
	LONG32 FramesPerKey; //This is the amount of frames to each keyframe
	LONG32 FileFormat; //UNSUPPORTED, BITMAP SERIES ASSUMED
};


void CreatePerlinGradient(WORD Flags, COLORREF MiddleColour, DWORD NumGradEntries, COLORREF *GradientArray, LONG32 RedV, LONG32 GreenV, LONG32 BlueV, double Persistance, LONG32 Octaves, LONG32 FirstOctave);

void DrawSolidColour(BitmapOps &SPBitmap, uchar Red, uchar Green, uchar Blue, int Position = 0);
void DrawNebula(BitmapOps &SPBitmap, SunProjectNebulaParams &NebParams, LONG32 MinDist = -1);
void DrawStarField(BitmapOps &SPBitmap, WORD Type);

void DrawBasicStar(BitmapOps &SPBitmap, LONG32 XPos, LONG32 YPos, DWORD StarRadius, DWORD CoronaRadius, COLORREF StarColour);
void DrawStarSurfaceNoise(BitmapOps &SPBitmap, LONG32 XPos, LONG32 YPos, DWORD StarRadius, LONG32 Variance, BOOL Smooth = TRUE, DWORD Seed = 0, BOOL Erase = FALSE, COLORREF StarColour = 0);
void DrawFlaresPerlinNoise(BitmapOps &SPBitmap, SunProjectNebulaParams &FlrParams, LONG32 XPos, LONG32 YPos, DWORD StarRadius, DWORD CoronaRadius, COLORREF StarColour, DWORD Seed = 0, BOOL Erase = TRUE, BOOL SubtractExistingCorona = TRUE);

void UnderlayBitmapOps(BitmapOps &TargetBitmap, BitmapOps &UnderlayBitmap, BitmapOps &OverlayBitmap);
void MergeBitmapOps(BitmapOps &TargetBitmap, BitmapOps &Bitmap1, BitmapOps &Bitmap2, double Amount); //Amount: Bitmap1 0<->1 Bitmap2
void SmoothBitmapOps(BitmapOps &TargetBitmap, BOOL UseDist = FALSE, LONG32 XPos = 0, LONG32 YPos = 0, DWORD MinRange = 0, DWORD MaxRange = 0); //Still needs fast/accurate, true/fake bi/tri-linear, V/H-tile, etc.

void DrawRandomStarCompact(HWND hWnd, BitmapOps &SPBitmap);



#endif
