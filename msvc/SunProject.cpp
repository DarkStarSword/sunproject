
#include "SunProject.h"

AnimationInformation AnimInfo = {0};

//BEGIN 1,0,1,0 Additions

/*
This function is used to interpolate
a value between two values using Cosine.

Source: Hugo Elias' Perlin Noise Tutorial
*/
//   b-----c
//   0--x--1
double Interpolate(double a, double b, double x) {
	double ft = x * 3.1415927;
	double f = (1 - cos(ft)) * 0.5;

	return a * (1 - f) + b * f;
}

//Linear version:
//   b-----c
//   0--x--1
double InterpolateLinear(double a, double b, double x) {
	return a * (1 - x) + b * x;
}

//Cubic version:
//   a-----b-----c-----d
//         0--x--1
double InterpolateCubic(double a, double b, double c, double d, double x) {
	double P = d - c - a + b;
	return P * x * x * x + (a - b - P) * x * x + (c - a) * x + b;
	/*double P = (d - c) - (a - b);
	double Q = (a - b) - P;
	double R = c - a;
	double S = b;
	return P * x * x * x + Q * x * x + R * x + S;*/
}

//This struct allows easy arrays of arrays:
struct ArrayOfArrays {
	LPVOID Pointer;
};

//This displays the error description:
void DisplayError(DWORD ErrorNum) {
	char *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ErrorNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, "Error!", MB_ICONEXCLAMATION);
	LocalFree(lpMsgBuf);
}

/*
This function fills the array with perlin noise given an origin,
Persistance, number of octaves and the target BitmapOps object,
and which channels to process.
Flags parameters (Combine with |):
1  - Fill the array with the 'middle' colour (Otherwise the middle colour is completely ignored)
2  - Bug (ie. Wood effect)
4  - Tile Horizontally
8  - Tile Vertically
16 - Disable array Smoothing (Unsupported)
32 - Linear (Unsupported) \
64 - Cubic (Unsupported)  - If neither are present, cosine is assumed. Cubic overrides linear.
*/
void FillPerlin(WORD Flags, LONG32 FirstOctave, double Persistance, LONG32 Octaves, BitmapOps &TargetBitmap, LONG32 RedM, LONG32 RedV, LONG32 GreenM, LONG32 GreenV, LONG32 BlueM, LONG32 BlueV, LONG32 MinDist) {
	if (!bRunning) return;
	LONG32 Total = 0;
	LONG32 Frequency = 1;
	double Amplitude = 1;
	LONG32 LastRand = 0;
	LONG32 XCount = 0;
	LONG32 YCount = 0;
	LONG32 FillX = 0;
	LONG32 FillY = 0;
	LONG32 FillXMax = 0;
	LONG32 FillYMax = 0;
	LONG32 FillXMin = 0;
	LONG32 FillYMin = 0;
	LONG32 TargetShade = 0;
	LONG32 TargetRed = 0;
	LONG32 TargetGreen = 0;
	LONG32 TargetBlue = 0;
	LONG32 Height = (LONG32)TargetBitmap.Height();
	LONG32 Width = (LONG32)TargetBitmap.Width();
	LONG32 *TBSmoothedBuff = 0;
	LONG32 *PerlinBuff = 0;
	DWORD TargetIndex = 0;
	LONG32 SideCount, CornerCount, TempSide, TempCorner; //For smoothing
	BOOL Fill = Flags & 1;
	BOOL UseBug = Flags & 2;
	BOOL HTile = Flags & 4;
	BOOL VTile = Flags & 8;
	if (Octaves < 1)
		Octaves--;

	//Fill the array:
	if (Fill) {
		for (YCount = 0; YCount < Height; ++YCount) {
			for (XCount = 0; XCount < Width; ++XCount) {
				TargetBitmap.SetPColour(XCount, YCount, RedM, GreenM, BlueM);
			}
			if (!DoEvents(NULL)) return;
		}
	}

	//If perlin noise has been disabled, exit:
	if (NoPerlin) return;

	//Autodetect the required number of octaves if it is less than 0 (specified 0 or less - it was decreased by one above)
	int i = 1;
	while (Octaves < 0) {
		Frequency = (LONG32)pow(2, i);
		if (Width > Height) {
			if (Frequency >= Height)
				Octaves = i + Octaves + 1;
		} else {
			if (Frequency >= Width)
				Octaves = i + Octaves + 1;
		}
		i++;
	}

	for (i = FirstOctave; i <= Octaves; ++i) {
		Frequency = (LONG32)pow(2, i);
		Amplitude = pow(Persistance, i);

		//Create a buffer to fill before it is smoothed:
		TBSmoothedBuff = new LONG32[(Frequency + 1) * (Frequency + 1)];
		if (!TBSmoothedBuff) return;

		//Create a buffer to store the perlin noise for this octave:
		PerlinBuff = new LONG32[(Frequency + 1) * (Frequency + 1)];
		if (!PerlinBuff) {
			if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;
			return;
		}

		//Fill in the buffer:
		for (int k = 0; k < (Frequency + 1) * (Frequency + 1); ++k)
			TBSmoothedBuff[k] = rand();
			//PerlinBuff[k] = rand();

		//Copy the To Be Smoothed Buffer to the Perlin noise buffer, smoothing as we go.
		for (YCount = 0; YCount < (Frequency + 1); ++YCount) {
			for (XCount = 0; XCount < (Frequency + 1); ++XCount) {
				PerlinBuff[YCount * (Frequency + 1) + XCount] = TBSmoothedBuff[YCount * (Frequency + 1) + XCount] / 4;
				SideCount = 4; CornerCount = 4; TempSide = 0; TempCorner = 0;
				if (XCount > 0) {
					if (YCount > 0) TempCorner += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount - 1];
					else CornerCount--;
					TempSide += TBSmoothedBuff[YCount * (Frequency + 1) + XCount - 1];
					if (YCount < Frequency) TempCorner += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount - 1];
					else CornerCount--;
				} else {
					SideCount--;
					CornerCount -= 2;
				}
				if (YCount > 0) TempSide += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount];
				else SideCount--;
				if (YCount < Frequency) TempSide += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount];
				else SideCount--;
				if (XCount < Frequency) {
					if (YCount > 0) TempCorner += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount + 1];
					else CornerCount--;
					TempSide += TBSmoothedBuff[YCount * (Frequency + 1) + XCount + 1];
					if (YCount < Frequency) TempCorner += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount + 1];
					else CornerCount--;
				} else {
					SideCount--;
					CornerCount -= 2;
				}
				PerlinBuff[YCount * (Frequency + 1) + XCount] += TempSide / (2 * SideCount) + TempCorner / (4 * CornerCount);
			}
		}

		//Delete the To Be Smoothed Beffer
		if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;

		if (HTile) {
			for (YCount = 0; YCount <= Frequency; ++YCount) {
				PerlinBuff[YCount * (Frequency + 1) + Frequency] = PerlinBuff[YCount * (Frequency + 1)];
			}
		}
		if (VTile) {
			for (XCount = 0; XCount <= Frequency; ++XCount) {
				PerlinBuff[Frequency * (Frequency + 1) + XCount] = PerlinBuff[XCount];
			}
		}

		//Draw it to the screen:
		for (YCount = 0; YCount <= Frequency; ++YCount) {
			for (XCount = 0; XCount <= Frequency; ++XCount) {
				LastRand = PerlinBuff[YCount * (Frequency + 1) + XCount];//rand();
				//Fill in the rectangle:
				if (RedV) TargetRed = LONG32((LastRand % RedV - RedV / 2) * Amplitude);
				if (GreenV) TargetGreen = LONG32((LastRand % GreenV - GreenV / 2) * Amplitude);
				if (BlueV) TargetBlue = LONG32((LastRand % BlueV - BlueV / 2) * Amplitude);
				FillXMin = LONG32(((double)Width / (double)Frequency) * (double)XCount);
				FillYMin = LONG32(((double)Height / (double)Frequency) * (double)YCount);
				FillXMax = LONG32((((double)Width / (double)Frequency) * double(XCount + 1) > Width) ? Width : ((double)Width / (double)Frequency) * double(XCount + 1));
				FillYMax = LONG32((((double)Height / (double)Frequency) * double(YCount + 1) > Height) ? Height : ((double)Height / (double)Frequency) * double(YCount + 1));
				for (FillY = FillYMin; FillY < FillYMax; ++FillY) {
					for (FillX = FillXMin; FillX < FillXMax; ++FillX) {
						if (sqrt(pow(Width / 2 - FillX, 2) + pow(Height / 2 - FillY, 2)) > MinDist) {
							TargetIndex = TargetBitmap.GetPixelIndex(FillX, FillY);
							//Red:
							double FractionalX = double(FillX - FillXMin) / double(FillXMax - FillXMin); // = Current X position in section / Width of section
							double FractionalY = double(FillY - FillYMin) / double(FillYMax - FillYMin); // = Current Y position in section / Height of section
							TargetShade = LONG32(Interpolate(Interpolate(PerlinBuff[YCount * (Frequency + 1) + XCount], PerlinBuff[YCount * (Frequency + 1) + XCount + 1], FractionalX), Interpolate(PerlinBuff[(YCount + 1) * (Frequency + 1) + XCount], PerlinBuff[(YCount + 1) * (Frequency + 1) + XCount + 1], FractionalX), FractionalY));
							if (UseBug) {
								//The following create a wood effect:
								if (RedV) TargetRed = LONG32((TargetShade % RedV - RedV / 2) * Amplitude);
								if (GreenV) TargetGreen = LONG32((TargetShade % GreenV - GreenV / 2) * Amplitude);
								if (BlueV) TargetBlue = LONG32((TargetShade % BlueV - BlueV / 2) * Amplitude);
							} else {
								//These smooth it properly:
								if (RedV) TargetRed = LONG32(((double)TargetShade / (double)RAND_MAX * (double)RedV - RedV / 2) * Amplitude);
								if (GreenV) TargetGreen = LONG32(((double)TargetShade / (double)RAND_MAX * (double)GreenV - GreenV / 2) * Amplitude);
								if (BlueV) TargetBlue = LONG32(((double)TargetShade / (double)RAND_MAX * (double)BlueV - BlueV / 2) * Amplitude);
							}
							if (TargetBitmap.PixelData[TargetIndex + 2] + TargetRed > 255)
								TargetBitmap.PixelData[TargetIndex + 2] = 255;
							else if (TargetBitmap.PixelData[TargetIndex + 2] + TargetRed < 0)
								TargetBitmap.PixelData[TargetIndex + 2] = 0;
							else
								TargetBitmap.PixelData[TargetIndex + 2] += TargetRed;
							//Green:
							if (TargetBitmap.PixelData[TargetIndex + 1] + TargetGreen > 255)
								TargetBitmap.PixelData[TargetIndex + 1] = 255;
							else if (TargetBitmap.PixelData[TargetIndex + 1] + TargetGreen < 0)
								TargetBitmap.PixelData[TargetIndex + 1] = 0;
							else
								TargetBitmap.PixelData[TargetIndex + 1] += TargetGreen;
							//Blue:
							if (TargetBitmap.PixelData[TargetIndex] + TargetBlue > 255)
								TargetBitmap.PixelData[TargetIndex] = 255;
							else if (TargetBitmap.PixelData[TargetIndex] + TargetBlue < 0)
								TargetBitmap.PixelData[TargetIndex] = 0;
							else
								TargetBitmap.PixelData[TargetIndex] += TargetBlue;
						}
					}
					if (!DoEvents(NULL)) {
						if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
						return;
					}
				}
			}
		}
		//Delete the buffer after each use:
		if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
	}
}


/*
This function fills a given BitmapOps with perlin noise. It is
based on the above formula, but designed to be compatible with
cubic interpolation as well as CoSine and Linear interpolation.
Flags parameters (Combine with |):
1  - Fill the array with the 'middle' colour (Otherwise the middle colour is completely ignored)
2  - Bug (ie. Wood effect)
4  - Tile Horizontally
8  - Tile Vertically
16 - Disable array Smoothing (Unsupported)
32 - Linear \
64 - Cubic  - If neither are present, CoSine is assumed. Cubic overrides linear.
*/
void FillPerlinAdvanced(BitmapOps &TargetBitmap, WORD Flags, COLORREF MiddleColour, LONG32 RedV, LONG32 GreenV, LONG32 BlueV, double Persistance, LONG32 Octaves, LONG32 FirstOctave, LONG32 MinDist) {
	if (!bRunning) return;
	LONG32 Total = 0;
	LONG32 Frequency = 1;
	double Amplitude = 1;
	LONG32 LastRand = 0;
	LONG32 XCount = 0;
	LONG32 YCount = 0;
	LONG32 FillX = 0;
	LONG32 FillY = 0;
	LONG32 FillXMax = 0;
	LONG32 FillYMax = 0;
	LONG32 FillXMin = 0;
	LONG32 FillYMin = 0;
	LONG32 TargetShade = 0;
	LONG32 TargetRed = 0;
	LONG32 TargetGreen = 0;
	LONG32 TargetBlue = 0;
	LONG32 Height = (LONG32)TargetBitmap.Height();
	LONG32 Width = (LONG32)TargetBitmap.Width();
	LONG32 *TBSmoothedBuff = 0;
	LONG32 *PerlinBuff = 0;
	DWORD TargetIndex = 0;
	LONG32 SideCount, CornerCount, TempSide, TempCorner; //For smoothing
	LONG32 RedM = GetRValue(MiddleColour);
	LONG32 GreenM = GetGValue(MiddleColour);
	LONG32 BlueM = GetBValue(MiddleColour);
	BOOL Fill = Flags & 1;
	BOOL UseBug = Flags & 2;
	BOOL HTile = Flags & 4;
	BOOL VTile = Flags & 8;
	BOOL UseLinear = Flags & 32;
	BOOL UseCubic = Flags & 64;
	if (Octaves < 1)
		Octaves--;

	//Fill the array:
	if (Fill) {
		for (YCount = 0; YCount < Height; ++YCount) {
			for (XCount = 0; XCount < Width; ++XCount) {
				TargetBitmap.SetPColour(XCount, YCount, RedM, GreenM, BlueM);
			}
			if (!DoEvents(NULL)) return;
		}
	}

	//If perlin noise has been disabled, exit:
	if (NoPerlin) return;

	//Autodetect the required number of octaves if it is less than 0 (specified 0 or less - it was decreased by one above)
	int i = 1;
	while (Octaves < 0) {
		Frequency = (LONG32)pow(2, i);
		if (Width > Height) {
			if (Frequency >= Height)
				Octaves = i + Octaves + 1;
		} else {
			if (Frequency >= Width)
				Octaves = i + Octaves + 1;
		}
		i++;
	}

	for (i = FirstOctave; i <= Octaves; ++i) {
		Frequency = (LONG32)pow(2, i) + 2;
		Amplitude = pow(Persistance, i);

		//Create a buffer to fill before it is smoothed:
		TBSmoothedBuff = new LONG32[(Frequency + 1) * (Frequency + 1)];
		if (!TBSmoothedBuff) return;

		//Create a buffer to store the perlin noise for this octave:
		PerlinBuff = new LONG32[(Frequency + 1) * (Frequency + 1)];
		if (!PerlinBuff) {
			if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;
			return;
		}

		//Fill in the buffer:
		for (int k = 0; k < (Frequency + 1) * (Frequency + 1); ++k)
			TBSmoothedBuff[k] = rand();
			//PerlinBuff[k] = rand();

		//Copy the To Be Smoothed Buffer to the Perlin noise buffer, smoothing as we go.
		for (YCount = 0; YCount < (Frequency + 1); ++YCount) {
			for (XCount = 0; XCount < (Frequency + 1); ++XCount) {
				PerlinBuff[YCount * (Frequency + 1) + XCount] = TBSmoothedBuff[YCount * (Frequency + 1) + XCount] / 4;
				SideCount = 4; CornerCount = 4; TempSide = 0; TempCorner = 0;
				if (XCount > 0) {
					if (YCount > 0) TempCorner += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount - 1];
					else CornerCount--;
					TempSide += TBSmoothedBuff[YCount * (Frequency + 1) + XCount - 1];
					if (YCount < Frequency) TempCorner += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount - 1];
					else CornerCount--;
				} else {
					SideCount--;
					CornerCount -= 2;
				}
				if (YCount > 0) TempSide += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount];
				else SideCount--;
				if (YCount < Frequency) TempSide += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount];
				else SideCount--;
				if (XCount < Frequency) {
					if (YCount > 0) TempCorner += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount + 1];
					else CornerCount--;
					TempSide += TBSmoothedBuff[YCount * (Frequency + 1) + XCount + 1];
					if (YCount < Frequency) TempCorner += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount + 1];
					else CornerCount--;
				} else {
					SideCount--;
					CornerCount -= 2;
				}
				PerlinBuff[YCount * (Frequency + 1) + XCount] += TempSide / (2 * SideCount) + TempCorner / (4 * CornerCount);
			}
		}

		//Delete the To Be Smoothed Beffer
		if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;

		if (HTile) {
			for (YCount = 0; YCount <= Frequency; ++YCount) {
				PerlinBuff[YCount * (Frequency + 1) + Frequency] = PerlinBuff[YCount * (Frequency + 1) + 2];
				PerlinBuff[YCount * (Frequency + 1) + Frequency - 1] = PerlinBuff[YCount * (Frequency + 1) + 1];
				PerlinBuff[YCount * (Frequency + 1)] = PerlinBuff[YCount * (Frequency + 1) + Frequency - 2];
			}
		}
		if (VTile) {
			for (XCount = 0; XCount <= Frequency; ++XCount) {
				PerlinBuff[Frequency * (Frequency + 1) + XCount] = PerlinBuff[2 * (Frequency + 1) + XCount];
				PerlinBuff[(Frequency - 1) * (Frequency + 1) + XCount] = PerlinBuff[(Frequency + 1) + XCount];
				PerlinBuff[XCount] = PerlinBuff[(Frequency - 2) * (Frequency + 1) + XCount];
			}
		}

		Frequency -= 2;

		//Draw it to the screen:
		for (YCount = 0; YCount <= Frequency; ++YCount) {
			for (XCount = 0; XCount <= Frequency; ++XCount) {
				LastRand = PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1];
				//Fill in the rectangle:
				if (RedV) TargetRed = LONG32((LastRand % RedV - RedV / 2) * Amplitude);
				if (GreenV) TargetGreen = LONG32((LastRand % GreenV - GreenV / 2) * Amplitude);
				if (BlueV) TargetBlue = LONG32((LastRand % BlueV - BlueV / 2) * Amplitude);
				FillXMin = LONG32(((double)Width / (double)Frequency) * (double)XCount);
				FillYMin = LONG32(((double)Height / (double)Frequency) * (double)YCount);
				FillXMax = LONG32((((double)Width / (double)Frequency) * double(XCount + 1) > Width) ? Width : ((double)Width / (double)Frequency) * double(XCount + 1));
				FillYMax = LONG32((((double)Height / (double)Frequency) * double(YCount + 1) > Height) ? Height : ((double)Height / (double)Frequency) * double(YCount + 1));
				if (sqrt(pow(Width / 2 - FillXMin, 2) + pow(Height / 2 - FillYMin, 2)) <= MinDist) {
					if (sqrt(pow(Width / 2 - FillXMax, 2) + pow(Height / 2 - FillYMin, 2)) <= MinDist) {
						if (sqrt(pow(Width / 2 - FillXMin, 2) + pow(Height / 2 - FillYMax, 2)) <= MinDist) {
							if (sqrt(pow(Width / 2 - FillXMax, 2) + pow(Height / 2 - FillYMax, 2)) <= MinDist) {
								if (XCount < Frequency / 2)
									XCount += 2 * (Frequency / 2 - XCount) - 1;
								continue;
							}
						}
					}
				}
				for (FillY = FillYMin; FillY < FillYMax; ++FillY) {
					for (FillX = FillXMin; FillX < FillXMax; ++FillX) {
						if (sqrt(pow(Width / 2 - FillX, 2) + pow(Height / 2 - FillY, 2)) <= MinDist) {
							FillX = (LONG32)sqrt(-pow(FillY - Height / 2, 2) + pow(MinDist, 2)) + Width / 2;
							if (FillX >= FillXMax)
								continue;
						}
						//if (sqrt(pow(Width / 2 - FillX, 2) + pow(Height / 2 - FillY, 2)) > MinDist) {
						//if (FillX < FillXMax) {
						TargetIndex = TargetBitmap.GetPixelIndex(FillX, FillY);
						double FractionalX = double(FillX - FillXMin) / double(FillXMax - FillXMin); // = Current X position in section / Width of section
						double FractionalY = double(FillY - FillYMin) / double(FillYMax - FillYMin); // = Current Y position in section / Height of section
						if (UseCubic) {
							//Cubic:
							TargetShade = LONG32(InterpolateCubic(InterpolateCubic(PerlinBuff[YCount * (Frequency + 3) + XCount], PerlinBuff[YCount * (Frequency + 3) + XCount + 1], PerlinBuff[YCount * (Frequency + 3) + XCount + 2], PerlinBuff[YCount * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 3], FractionalX), FractionalY));
							//AAAGH!!! Just wait till you see the TEMPORAL
							//Cubic interpolation that I have planned!!!
						} else if (UseLinear) {
							//Linear:
							TargetShade = LONG32(InterpolateLinear(InterpolateLinear(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], FractionalX), InterpolateLinear(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], FractionalX), FractionalY));
						} else {
							//CoSine:
							TargetShade = LONG32(Interpolate(Interpolate(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], FractionalX), Interpolate(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], FractionalX), FractionalY));
						}
						if (UseBug) {
							//The following create a wood effect:
							if (RedV) TargetRed = LONG32((TargetShade % RedV - RedV / 2) * Amplitude);
							if (GreenV) TargetGreen = LONG32((TargetShade % GreenV - GreenV / 2) * Amplitude);
							if (BlueV) TargetBlue = LONG32((TargetShade % BlueV - BlueV / 2) * Amplitude);
						} else {
							//These smooth it properly:
							if (RedV) TargetRed = LONG32(((double)TargetShade / (double)RAND_MAX * (double)RedV - RedV / 2) * Amplitude);
							if (GreenV) TargetGreen = LONG32(((double)TargetShade / (double)RAND_MAX * (double)GreenV - GreenV / 2) * Amplitude);
							if (BlueV) TargetBlue = LONG32(((double)TargetShade / (double)RAND_MAX * (double)BlueV - BlueV / 2) * Amplitude);
						}
						//Red:
						if (TargetBitmap.PixelData[TargetIndex + 2] + TargetRed > 255)
							TargetBitmap.PixelData[TargetIndex + 2] = 255;
						else if (TargetBitmap.PixelData[TargetIndex + 2] + TargetRed < 0)
							TargetBitmap.PixelData[TargetIndex + 2] = 0;
						else
							TargetBitmap.PixelData[TargetIndex + 2] += TargetRed;
						//Green:
						if (TargetBitmap.PixelData[TargetIndex + 1] + TargetGreen > 255)
							TargetBitmap.PixelData[TargetIndex + 1] = 255;
						else if (TargetBitmap.PixelData[TargetIndex + 1] + TargetGreen < 0)
							TargetBitmap.PixelData[TargetIndex + 1] = 0;
						else
							TargetBitmap.PixelData[TargetIndex + 1] += TargetGreen;
						//Blue:
						if (TargetBitmap.PixelData[TargetIndex] + TargetBlue > 255)
							TargetBitmap.PixelData[TargetIndex] = 255;
						else if (TargetBitmap.PixelData[TargetIndex] + TargetBlue < 0)
							TargetBitmap.PixelData[TargetIndex] = 0;
						else
							TargetBitmap.PixelData[TargetIndex] += TargetBlue;
						//}
					}
					if (!DoEvents(NULL)) {
						if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
						return;
					}
				}
			}
		}
		//Delete the buffer after each use:
		if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
	}
}




/*
This function fills the given COLORREF array with a gradient. It
is based on the above advanced perlin noise routine, but designed
to only work on one dimension.
Flags parameters (Combine with |):
1  - Fill the array with the 'middle' colour (Otherwise the middle colour is completely ignored)
2  - Bug (ie. Wood effect)
4  - Tile
8  - Disable array Smoothing (Unsupported)
16 - Linear \
32 - Cubic  - If neither are present, CoSine is assumed. Cubic overrides linear.
*/
void CreatePerlinGradient(WORD Flags, COLORREF MiddleColour, DWORD NumGradEntries, COLORREF *GradientArray, LONG32 RedV, LONG32 GreenV, LONG32 BlueV, double Persistance, LONG32 Octaves, LONG32 FirstOctave) {
	if (!bRunning) return;
	LONG32 Total = 0;
	LONG32 Frequency = 1;
	double Amplitude = 1;
	LONG32 LastRand = 0;
	LONG32 Count = 0;
	LONG32 FillC = 0;
	LONG32 FillMax = 0;
	LONG32 FillMin = 0;
	LONG32 TargetShade = 0;
	LONG32 TargetRed = 0;
	LONG32 TargetGreen = 0;
	LONG32 TargetBlue = 0;
	LONG32 *TBSmoothedBuff = 0;
	LONG32 *PerlinBuff = 0;
	DWORD TargetIndex = 0;
	LONG32 RedM = GetRValue(MiddleColour);
	LONG32 GreenM = GetGValue(MiddleColour);
	LONG32 BlueM = GetBValue(MiddleColour);
	BOOL Fill = Flags & 1;
	BOOL UseBug = Flags & 2;
	BOOL Tile = Flags & 4;
	BOOL UseLinear = Flags & 16;
	BOOL UseCubic = Flags & 32;
	if (Octaves < 1)
		Octaves--;

	//Fill the array:
	if (Fill) {
		for (Count = 0; Count < NumGradEntries; ++Count) {
			GradientArray[Count] = RGB(RedM, GreenM, BlueM);
			//if (!DoEvents(NULL)) return;
		}
	}

	//If perlin noise has been disabled, exit:
	if (NoPerlin) return;

	//Autodetect the required number of octaves if it is less than 0 (specified 0 or less - it was decreased by one above)
	int i = 1;
	while (Octaves < 0) {
		Frequency = (LONG32)pow(2, i);
		if (Frequency >= NumGradEntries)
			Octaves = i + Octaves + 1;
		i++;
	}

	for (i = FirstOctave; i <= Octaves; ++i) {
		Frequency = (LONG32)pow(2, i) + 2;
		Amplitude = pow(Persistance, i);

		//Create a buffer to fill before it is smoothed:
		TBSmoothedBuff = new LONG32[Frequency + 1];
		if (!TBSmoothedBuff) return;

		//Create a buffer to store the perlin noise for this octave:
		PerlinBuff = new LONG32[Frequency + 1];
		if (!PerlinBuff) {
			if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;
			return;
		}

		//Fill in the buffer:
		for (int k = 0; k <= Frequency; ++k)
			TBSmoothedBuff[k] = rand();
			//PerlinBuff[k] = rand();

		//Copy the To Be Smoothed Buffer to the Perlin noise buffer, smoothing as we go.
		for (Count = 0; Count <= Frequency; ++Count) {
			PerlinBuff[Count] = TBSmoothedBuff[Count] / 2;
			if (Count > 0) {
				PerlinBuff[Count] += TBSmoothedBuff[Count - 1] / 4;
			} else {
				PerlinBuff[Count] += TBSmoothedBuff[Count] / 4;
			}
			if (Count < Frequency) {
				PerlinBuff[Count] += TBSmoothedBuff[Count + 1] / 4;
			} else {
				PerlinBuff[Count] += TBSmoothedBuff[Count] / 4;
			}
		}

		//Delete the To Be Smoothed Beffer
		if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;

		if (Tile) {
			PerlinBuff[Frequency] = PerlinBuff[2];
			PerlinBuff[Frequency - 1] = PerlinBuff[1];
			PerlinBuff[0] = PerlinBuff[Frequency - 2];
		}

		Frequency -= 2;

		//Draw it to the screen:
		for (Count = 0; Count <= Frequency; ++Count) {
			LastRand = PerlinBuff[Count + 1];
			//Fill in the rectangle:
			if (RedV) TargetRed = LONG32((LastRand % RedV - RedV / 2) * Amplitude);
			if (GreenV) TargetGreen = LONG32((LastRand % GreenV - GreenV / 2) * Amplitude);
			if (BlueV) TargetBlue = LONG32((LastRand % BlueV - BlueV / 2) * Amplitude);
			FillMin = LONG32(((double)NumGradEntries / (double)Frequency) * (double)Count);
			FillMax = LONG32((((double)NumGradEntries / (double)Frequency) * double(Count + 1) > NumGradEntries) ? NumGradEntries : ((double)NumGradEntries / (double)Frequency) * double(Count + 1));
			/*if (sqrt(pow(Width / 2 - FillXMin, 2) + pow(Height / 2 - FillYMin, 2)) <= MinDist) {
				if (sqrt(pow(Width / 2 - FillXMax, 2) + pow(Height / 2 - FillYMin, 2)) <= MinDist) {
					if (sqrt(pow(Width / 2 - FillXMin, 2) + pow(Height / 2 - FillYMax, 2)) <= MinDist) {
						if (sqrt(pow(Width / 2 - FillXMax, 2) + pow(Height / 2 - FillYMax, 2)) <= MinDist) {
							if (XCount < Frequency / 2)
								XCount += 2 * (Frequency / 2 - XCount) - 1;
							continue;
						}
					}
				}
			}*/
			for (FillC = FillMin; FillC < FillMax; ++FillC) {
				/*if (sqrt(pow(NumGradEntries / 2 - Fill, 2) + pow(Height / 2 - FillY, 2)) <= MinDist) {
					FillX = (LONG32)sqrt(-pow(FillY - Height / 2, 2) + pow(MinDist, 2)) + Width / 2;
					if (FillX >= FillXMax)
						continue;
				}*/
				//if (sqrt(pow(Width / 2 - FillX, 2) + pow(Height / 2 - FillY, 2)) > MinDist) {
				//if (FillX < FillXMax) {
				double Fractional = double(FillC - FillMin) / double(FillMax - FillMin); // = Current position in section / total section
				if (UseCubic) {
					//Cubic:
					//TargetShade = LONG32(InterpolateCubic(InterpolateCubic(PerlinBuff[YCount * (Frequency + 3) + XCount], PerlinBuff[YCount * (Frequency + 3) + XCount + 1], PerlinBuff[YCount * (Frequency + 3) + XCount + 2], PerlinBuff[YCount * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 3], FractionalX), FractionalY));
					TargetShade = (LONG32)InterpolateCubic(PerlinBuff[Count], PerlinBuff[Count + 1], PerlinBuff[Count + 2], PerlinBuff[Count + 3], Fractional);
				} else if (UseLinear) {
					//Linear:
					//TargetShade = LONG32(InterpolateLinear(InterpolateLinear(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], FractionalX), InterpolateLinear(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], FractionalX), FractionalY));
					TargetShade = (LONG32)InterpolateLinear(PerlinBuff[Count + 1], PerlinBuff[Count + 2], Fractional);
				} else {
					//CoSine:
					//TargetShade = LONG32(Interpolate(Interpolate(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], FractionalX), Interpolate(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], FractionalX), FractionalY));
					TargetShade = (LONG32)Interpolate(PerlinBuff[Count + 1], PerlinBuff[Count + 2], Fractional);
				}
				if (UseBug) {
					//The following create a wood effect:
					if (RedV) TargetRed = LONG32((TargetShade % RedV - RedV / 2) * Amplitude);
					if (GreenV) TargetGreen = LONG32((TargetShade % GreenV - GreenV / 2) * Amplitude);
					if (BlueV) TargetBlue = LONG32((TargetShade % BlueV - BlueV / 2) * Amplitude);
				} else {
					//These smooth it properly:
					if (RedV) TargetRed = LONG32(((double)TargetShade / (double)RAND_MAX * (double)RedV - (double)RedV / 2.0) * Amplitude);
					if (GreenV) TargetGreen = LONG32(((double)TargetShade / (double)RAND_MAX * (double)GreenV - (double)GreenV / 2.0) * Amplitude);
					if (BlueV) TargetBlue = LONG32(((double)TargetShade / (double)RAND_MAX * (double)BlueV - (double)BlueV / 2.0) * Amplitude);
				}
				BYTE CurRed = GetRValue(GradientArray[FillC]);
				BYTE CurGreen = GetGValue(GradientArray[FillC]);
				BYTE CurBlue = GetBValue(GradientArray[FillC]);
				//Red:
				if (CurRed + TargetRed > 255)
					CurRed = 255;
				else if (CurRed + TargetRed < 0)
					CurRed = 0;
				else
					CurRed += TargetRed;
				//Green:
				if (CurGreen + TargetGreen > 255)
					CurGreen = 255;
				else if (CurGreen + TargetGreen < 0)
					CurGreen = 0;
				else
					CurGreen += TargetGreen;
				//Blue:
				if (CurBlue + TargetBlue > 255)
					CurBlue = 255;
				else if (CurBlue + TargetBlue < 0)
					CurBlue = 0;
				else
					CurBlue += TargetBlue;
				GradientArray[FillC] = RGB(CurRed, CurGreen, CurBlue);
				/*if (!DoEvents(NULL)) {
					if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
					return;
				}*/
			}
		}
		//Delete the buffer after each use:
		if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
	}
}



/*
This function fills a given BitmapOps with perlin noise. It is
based on the above advanced perlin noise formula, but designed
use a gradient to make the result look better. The
CreatePerlinGradient function can be used to create a gradient
compatible with this function. Also, note that if:
0x40000000 is set, red is negative.
0x20000000 is set, green is negative.
0x10000000 is set, blue is negative.
Flags parameters (Combine with |):
1  - Fill the array with the 'middle' colour (Otherwise the middle colour is completely ignored)
2  - Bug (ie. Wood effect)
4  - Tile Horizontally
8  - Tile Vertically
16 - Disable array Smoothing (Unsupported)
32 - Linear \
64 - Cubic  - If neither are present, CoSine is assumed. Cubic overrides linear.
*/
void FillPerlinGradient(BitmapOps &TargetBitmap, WORD Flags, COLORREF MiddleColour, DWORD NumGradEntries, COLORREF *GradientArray, double Persistance, LONG32 Octaves, LONG32 FirstOctave, LONG32 MinDist) {
	if (!bRunning) return;
	LONG32 Total = 0;
	LONG32 Frequency = 1;
	double Amplitude = 1;
	LONG32 LastRand = 0;
	LONG32 XCount = 0;
	LONG32 YCount = 0;
	LONG32 FillX = 0;
	LONG32 FillY = 0;
	LONG32 FillXMax = 0;
	LONG32 FillYMax = 0;
	LONG32 FillXMin = 0;
	LONG32 FillYMin = 0;
	LONG32 TargetShade = 0;
	LONG32 TargetRed = 0;
	LONG32 TargetGreen = 0;
	LONG32 TargetBlue = 0;
	LONG32 Height = (LONG32)TargetBitmap.Height();
	LONG32 Width = (LONG32)TargetBitmap.Width();
	LONG32 *TBSmoothedBuff = 0;
	LONG32 *PerlinBuff = 0;
	DWORD TargetIndex = 0;
	LONG32 SideCount, CornerCount, TempSide, TempCorner; //For smoothing
	LONG32 RedM = GetRValue(MiddleColour);
	LONG32 GreenM = GetGValue(MiddleColour);
	LONG32 BlueM = GetBValue(MiddleColour);
	BOOL Fill = Flags & 1;
	BOOL UseBug = Flags & 2;
	BOOL HTile = Flags & 4;
	BOOL VTile = Flags & 8;
	BOOL UseLinear = Flags & 32;
	BOOL UseCubic = Flags & 64;
	if (Octaves < 1)
		Octaves--;

	//Fill the array:
	if (Fill) {
		for (YCount = 0; YCount < Height; ++YCount) {
			for (XCount = 0; XCount < Width; ++XCount) {
				TargetBitmap.SetPColour(XCount, YCount, RedM, GreenM, BlueM);
			}
			if (!DoEvents(NULL)) return;
		}
	}

	//If perlin noise has been disabled, exit:
	if (NoPerlin) return;

	//Autodetect the required number of octaves if it is less than 0 (specified 0 or less - it was decreased by one above)
	int i = 1;
	while (Octaves < 0) {
		Frequency = (LONG32)pow(2, i);
		if (Width > Height) {
			if (Frequency >= Height)
				Octaves = i + Octaves + 1;
		} else {
			if (Frequency >= Width)
				Octaves = i + Octaves + 1;
		}
		i++;
	}

	for (i = FirstOctave; i <= Octaves; ++i) {
		Frequency = (LONG32)pow(2, i) + 2;
		Amplitude = pow(Persistance, i);

		//Create a buffer to fill before it is smoothed:
		TBSmoothedBuff = new LONG32[(Frequency + 1) * (Frequency + 1)];
		if (!TBSmoothedBuff) return;

		//Create a buffer to store the perlin noise for this octave:
		PerlinBuff = new LONG32[(Frequency + 1) * (Frequency + 1)];
		if (!PerlinBuff) {
			if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;
			return;
		}

		//Fill in the buffer:
		for (int k = 0; k < (Frequency + 1) * (Frequency + 1); ++k)
			TBSmoothedBuff[k] = rand();
			//PerlinBuff[k] = rand();

		//Copy the To Be Smoothed Buffer to the Perlin noise buffer, smoothing as we go.
		for (YCount = 0; YCount < (Frequency + 1); ++YCount) {
			for (XCount = 0; XCount < (Frequency + 1); ++XCount) {
				PerlinBuff[YCount * (Frequency + 1) + XCount] = TBSmoothedBuff[YCount * (Frequency + 1) + XCount] / 4;
				SideCount = 4; CornerCount = 4; TempSide = 0; TempCorner = 0;
				if (XCount > 0) {
					if (YCount > 0) TempCorner += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount - 1];
					else CornerCount--;
					TempSide += TBSmoothedBuff[YCount * (Frequency + 1) + XCount - 1];
					if (YCount < Frequency) TempCorner += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount - 1];
					else CornerCount--;
				} else {
					SideCount--;
					CornerCount -= 2;
				}
				if (YCount > 0) TempSide += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount];
				else SideCount--;
				if (YCount < Frequency) TempSide += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount];
				else SideCount--;
				if (XCount < Frequency) {
					if (YCount > 0) TempCorner += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount + 1];
					else CornerCount--;
					TempSide += TBSmoothedBuff[YCount * (Frequency + 1) + XCount + 1];
					if (YCount < Frequency) TempCorner += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount + 1];
					else CornerCount--;
				} else {
					SideCount--;
					CornerCount -= 2;
				}
				PerlinBuff[YCount * (Frequency + 1) + XCount] += TempSide / (2 * SideCount) + TempCorner / (4 * CornerCount);
			}
		}

		//Delete the To Be Smoothed Beffer
		if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;

		if (HTile) {
			for (YCount = 0; YCount <= Frequency; ++YCount) {
				PerlinBuff[YCount * (Frequency + 1) + Frequency] = PerlinBuff[YCount * (Frequency + 1) + 2];
				PerlinBuff[YCount * (Frequency + 1) + Frequency - 1] = PerlinBuff[YCount * (Frequency + 1) + 1];
				PerlinBuff[YCount * (Frequency + 1)] = PerlinBuff[YCount * (Frequency + 1) + Frequency - 2];
			}
		}
		if (VTile) {
			for (XCount = 0; XCount <= Frequency; ++XCount) {
				PerlinBuff[Frequency * (Frequency + 1) + XCount] = PerlinBuff[2 * (Frequency + 1) + XCount];
				PerlinBuff[(Frequency - 1) * (Frequency + 1) + XCount] = PerlinBuff[(Frequency + 1) + XCount];
				PerlinBuff[XCount] = PerlinBuff[(Frequency - 2) * (Frequency + 1) + XCount];
			}
		}

		Frequency -= 2;

		//Draw it to the screen:
		for (YCount = 0; YCount <= Frequency; ++YCount) {
			for (XCount = 0; XCount <= Frequency; ++XCount) {
				LastRand = PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1];
				//Fill in the rectangle:
//				if (RedV) TargetRed = LONG32((LastRand % RedV - RedV / 2) * Amplitude);
//				if (GreenV) TargetGreen = LONG32((LastRand % GreenV - GreenV / 2) * Amplitude);
//				if (BlueV) TargetBlue = LONG32((LastRand % BlueV - BlueV / 2) * Amplitude);
				FillXMin = LONG32(((double)Width / (double)Frequency) * (double)XCount);
				FillYMin = LONG32(((double)Height / (double)Frequency) * (double)YCount);
				FillXMax = LONG32((((double)Width / (double)Frequency) * double(XCount + 1) > Width) ? Width : ((double)Width / (double)Frequency) * double(XCount + 1));
				FillYMax = LONG32((((double)Height / (double)Frequency) * double(YCount + 1) > Height) ? Height : ((double)Height / (double)Frequency) * double(YCount + 1));
				if (sqrt(pow(Width / 2 - FillXMin, 2) + pow(Height / 2 - FillYMin, 2)) <= MinDist) {
					if (sqrt(pow(Width / 2 - FillXMax, 2) + pow(Height / 2 - FillYMin, 2)) <= MinDist) {
						if (sqrt(pow(Width / 2 - FillXMin, 2) + pow(Height / 2 - FillYMax, 2)) <= MinDist) {
							if (sqrt(pow(Width / 2 - FillXMax, 2) + pow(Height / 2 - FillYMax, 2)) <= MinDist) {
								if (XCount < Frequency / 2)
									XCount += 2 * (Frequency / 2 - XCount) - 1;
								continue;
							}
						}
					}
				}
				for (FillY = FillYMin; FillY < FillYMax; ++FillY) {
					for (FillX = FillXMin; FillX < FillXMax; ++FillX) {
						if (sqrt(pow(Width / 2 - FillX, 2) + pow(Height / 2 - FillY, 2)) <= MinDist) {
							FillX = (LONG32)sqrt(-pow(FillY - Height / 2, 2) + pow(MinDist, 2)) + Width / 2;
							if (FillX >= FillXMax)
								continue;
						}
						//if (sqrt(pow(Width / 2 - FillX, 2) + pow(Height / 2 - FillY, 2)) > MinDist) {
						//if (FillX < FillXMax) {
						TargetIndex = TargetBitmap.GetPixelIndex(FillX, FillY);
						double FractionalX = double(FillX - FillXMin) / double(FillXMax - FillXMin); // = Current X position in section / Width of section
						double FractionalY = double(FillY - FillYMin) / double(FillYMax - FillYMin); // = Current Y position in section / Height of section
						if (UseCubic) {
							//Cubic:
							TargetShade = LONG32(InterpolateCubic(InterpolateCubic(PerlinBuff[YCount * (Frequency + 3) + XCount], PerlinBuff[YCount * (Frequency + 3) + XCount + 1], PerlinBuff[YCount * (Frequency + 3) + XCount + 2], PerlinBuff[YCount * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 3], FractionalX), FractionalY));
							//AAAGH!!! Just wait till you see the TEMPORAL
							//Cubic interpolation that I have planned!!!
						} else if (UseLinear) {
							//Linear:
							TargetShade = LONG32(InterpolateLinear(InterpolateLinear(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], FractionalX), InterpolateLinear(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], FractionalX), FractionalY));
						} else {
							//CoSine:
							TargetShade = LONG32(Interpolate(Interpolate(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], FractionalX), Interpolate(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], FractionalX), FractionalY));
						}
						if (UseBug) {
							//The following create a wood effect:
							//if (RedV) TargetRed = LONG32((TargetShade % RedV - RedV / 2) * Amplitude);
							//if (GreenV) TargetGreen = LONG32((TargetShade % GreenV - GreenV / 2) * Amplitude);
							//if (BlueV) TargetBlue = LONG32((TargetShade % BlueV - BlueV / 2) * Amplitude);
							TargetShade = LONG32(TargetShade % NumGradEntries);
						} else {
							//These smooth it properly:
							//if (RedV) TargetRed = LONG32(((double)TargetShade / (double)RAND_MAX * (double)RedV - RedV / 2) * Amplitude);
							//if (GreenV) TargetGreen = LONG32(((double)TargetShade / (double)RAND_MAX * (double)GreenV - GreenV / 2) * Amplitude);
							//if (BlueV) TargetBlue = LONG32(((double)TargetShade / (double)RAND_MAX * (double)BlueV - BlueV / 2) * Amplitude);
							//TargetShade = LONG32((double)TargetShade / (double)RAND_MAX * (double)NumGradEntries);
							FractionalX = (double)TargetShade / (double)RAND_MAX * (double)NumGradEntries;
							TargetShade = (LONG32)FractionalX;
							FractionalX -= TargetShade;
						}
						/*TargetRed = GetRValue(GradientArray[TargetShade]) * Amplitude;
						TargetGreen = GetGValue(GradientArray[TargetShade]) * Amplitude;
						TargetBlue = GetBValue(GradientArray[TargetShade]) * Amplitude;*/
						LONG32 Val1, Val2, Val3, Val4;
						if (TargetShade) Val1 = TargetShade - 1;
						else Val1 = TargetShade;
						Val2 = TargetShade;
						if (TargetShade < NumGradEntries - 1) Val3 = TargetShade + 1;
						else Val3 = TargetShade;
						if (TargetShade < NumGradEntries) Val4 = TargetShade + 2;
						else Val4 = TargetShade;
						TargetRed = InterpolateCubic(GetRValue(GradientArray[Val1]), GetRValue(GradientArray[Val2]), GetRValue(GradientArray[Val3]), GetRValue(GradientArray[Val4]), FractionalX) * Amplitude;
						TargetGreen = InterpolateCubic(GetGValue(GradientArray[Val1]), GetGValue(GradientArray[Val2]), GetGValue(GradientArray[Val3]), GetGValue(GradientArray[Val4]), FractionalX) * Amplitude;
						TargetBlue = InterpolateCubic(GetBValue(GradientArray[Val1]), GetBValue(GradientArray[Val2]), GetBValue(GradientArray[Val3]), GetBValue(GradientArray[Val4]), FractionalX) * Amplitude;
						//Red:
						LONG32 Multiplier = (GradientArray[TargetShade] & 0x40000000 ? -1 : 1);
						if (TargetBitmap.PixelData[TargetIndex + 2] + TargetRed * Multiplier > 255)
							TargetBitmap.PixelData[TargetIndex + 2] = 255;
						else if (TargetBitmap.PixelData[TargetIndex + 2] + TargetRed * Multiplier < 0)
							TargetBitmap.PixelData[TargetIndex + 2] = 0;
						else
							TargetBitmap.PixelData[TargetIndex + 2] += TargetRed * Multiplier;
						//Green:
						Multiplier = (GradientArray[TargetShade] & 0x20000000 ? -1 : 1);
						if (TargetBitmap.PixelData[TargetIndex + 1] + TargetGreen * Multiplier > 255)
							TargetBitmap.PixelData[TargetIndex + 1] = 255;
						else if (TargetBitmap.PixelData[TargetIndex + 1] + TargetGreen * Multiplier < 0)
							TargetBitmap.PixelData[TargetIndex + 1] = 0;
						else
							TargetBitmap.PixelData[TargetIndex + 1] += TargetGreen * Multiplier;
						//Blue:
						Multiplier = (GradientArray[TargetShade] & 0x10000000 ? -1 : 1);
						if (TargetBitmap.PixelData[TargetIndex] + TargetBlue * Multiplier > 255)
							TargetBitmap.PixelData[TargetIndex] = 255;
						else if (TargetBitmap.PixelData[TargetIndex] + TargetBlue * Multiplier < 0)
							TargetBitmap.PixelData[TargetIndex] = 0;
						else
							TargetBitmap.PixelData[TargetIndex] += TargetBlue * Multiplier;
						//}
					}
					if (!DoEvents(NULL)) {
						if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
						return;
					}
				}
			}
		}
		//Delete the buffer after each use:
		if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
	}
}



/*
This function creates animating perlin noise. It is based on my
FillPerlinAdvanced formula, but has an entire new dimention. Also,
it is necessary to store all the data for every octave on the nearest
SIX time frames at a time. Due to the major memory requirements of
this function, I will shortly add a memory conservation mode, where
it will apply one octave at a time to an entire animation.
Flags parameters (Combine with |):
1   - Fill the array with the 'middle' colour (Otherwise the middle colour is completely ignored)
2   - Bug (ie. Wood effect)
4   - Tile Horizontally
8   - Tile Vertically
16  - Disable array Smoothing (Unsupported)
32  - Linear \
64  - Cubic  - If neither are present, CoSine is assumed. Cubic overrides linear.
128 - Memory conservation mode (Unsupported)
256 - Loop
512 - Smooth the time axis
*/
void FillPerlinTemporal(BitmapOps &TargetBitmap, WORD Flags, COLORREF MiddleColour, LONG32 RedV, LONG32 GreenV, LONG32 BlueV, double Persistance, LONG32 Octaves, LONG32 FirstOctave, LONG32 MinDist, LPSTR FileNameBase, LONG32 KeyFrames, LONG32 FramesPerKeyFrame, LONG32 FileFormat = ANIMPERLINFORMATBITMAPSERIES, PCOMPVARS CompVars = NULL, PAVISTREAM *pAviStream = NULL) {
	if (!bRunning) return;
	LONG32 Total = 0;
	LONG32 Frequency = 1;
	double Amplitude = 1;
	LONG32 LastRand = 0;
	LONG32 XCount = 0;
	LONG32 YCount = 0;
	LONG32 FillX = 0;
	LONG32 FillY = 0;
	LONG32 FillXMax = 0;
	LONG32 FillYMax = 0;
	LONG32 FillXMin = 0;
	LONG32 FillYMin = 0;
	LONG32 TargetShade = 0;
	LONG32 TargetRed = 0;
	LONG32 TargetGreen = 0;
	LONG32 TargetBlue = 0;
	LONG32 Height = (LONG32)TargetBitmap.Height();
	LONG32 Width = (LONG32)TargetBitmap.Width();
	LONG32 *TBSmoothedBuff = 0;
	LONG32 *PerlinBuff = 0;
	DWORD TargetIndex = 0;
	LONG32 SideCount, CornerCount, TempSide, TempCorner; //For smoothing
	LONG32 RedM = GetRValue(MiddleColour);
	LONG32 GreenM = GetGValue(MiddleColour);
	LONG32 BlueM = GetBValue(MiddleColour);
	BOOL Fill = Flags & 1;
	BOOL UseBug = Flags & 2;
	BOOL HTile = Flags & 4;
	BOOL VTile = Flags & 8;
	//Disable array smoothing - 16
	BOOL UseLinear = Flags & 32;
	BOOL UseCubic = Flags & 64;
	//Memory conservation mode - 128
	BOOL Loop = Flags & 256;
	BOOL SmoothT = Flags & 512;
	if (Octaves < 1)
		Octaves--;

	//Autodetect the required number of octaves if it is less than 0 (specified 0 or less - it was decreased by one above)
	int i = 1;
	while (Octaves < 0) {
		Frequency = (LONG32)pow(2, i);
		if (Width > Height) {
			if (Frequency >= Height)
				Octaves = i + Octaves + 1;
		} else {
			if (Frequency >= Width)
				Octaves = i + Octaves + 1;
		}
		i++;
	}

	//Create an array of 5 that we store some seeds in, so that we can re-create the beginning key frames if loop is enabled:
	LONG32 SeedArray[5] = {rand(), rand(), rand(), rand(), rand()};
	//Unlike before, we need to keep accessing the octave data, so let's prepare some:
	//First, an array of four pointers:
	ArrayOfArrays TemporalArray[6];
	//Each entry must point to an array with as many elements as there are octaves:
	TemporalArray[0].Pointer = new ArrayOfArrays[Octaves + 1];
	TemporalArray[1].Pointer = new ArrayOfArrays[Octaves + 1];
	TemporalArray[2].Pointer = new ArrayOfArrays[Octaves + 1];
	TemporalArray[3].Pointer = new ArrayOfArrays[Octaves + 1];
	TemporalArray[4].Pointer = new ArrayOfArrays[Octaves + 1];
	TemporalArray[5].Pointer = new ArrayOfArrays[Octaves + 1];
	//If any array failed to create, clean up any survivors and return:
	//if (!((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) {
	if (!((ArrayOfArrays*)TemporalArray[0].Pointer) || !((ArrayOfArrays*)TemporalArray[1].Pointer) || !((ArrayOfArrays*)TemporalArray[2].Pointer) || !((ArrayOfArrays*)TemporalArray[3].Pointer) || !((ArrayOfArrays*)TemporalArray[4].Pointer) || !((ArrayOfArrays*)TemporalArray[5].Pointer)) {
		DisplayError(GetLastError());
#ifdef _DEBUG
		MessageBox(NULL, "Error occured during temporal array allocation", "Error! (Debug)", MB_ICONERROR);
#endif
		if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
		if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
		if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
		if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
		if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
		if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
		return;
	}
	//Loop through all the octaves (in question) and create heaps of arrays:
	BOOL ErrorOccured = FALSE;
	for (i = FirstOctave; i <= Octaves; ++i) {
		Frequency = (LONG32)pow(2, i) + 2;
		((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		if (!((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer)
			ErrorOccured = TRUE;
	}
	if (ErrorOccured) {
		DisplayError(GetLastError());
#ifdef _DEBUG
		MessageBox(NULL, "Error occured during octave array allocation", "Error! (Debug)", MB_ICONERROR);
#endif
		//Loop through all the octaves (in question) and delete all the arrays:
		for (i = FirstOctave; i <= Octaves; ++i) {
			if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
			if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
			if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
			if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
			if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
			if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
		}
		if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
		if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
		if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
		if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
		if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
		if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
		return;
	}
	for (int l = 0; l < 5; l++) {
		srand(SeedArray[l]); rand();
		for (i = FirstOctave; i <= Octaves; ++i) {
			Frequency = (LONG32)pow(2, i) + 2;
			//Fill in the buffers except the first, as it will be filled in shortly:
			//if (!ErrorOccured) {
			for (int k = 0; k < (Frequency + 1) * (Frequency + 1); ++k) {
				((LONG32*)((ArrayOfArrays*)TemporalArray[l + 1].Pointer)[i].Pointer)[k] = rand();
				/*((LONG32*)((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer)[k] = rand();
				((LONG32*)((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer)[k] = rand();
				((LONG32*)((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer)[k] = rand();
				((LONG32*)((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer)[k] = rand();
				((LONG32*)((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer)[k] = rand();*/
			}
			//}
		}
	}

	for (int TemporalCounter = 0; TemporalCounter < FramesPerKeyFrame * KeyFrames; TemporalCounter++) {
		AnimInfo.CurrentFrame = TemporalCounter + 1;
		if (TemporalCounter % FramesPerKeyFrame == 0) {
			if (Loop && KeyFrames - TemporalCounter / FramesPerKeyFrame < 6) { //Only the last five should register
				srand(SeedArray[5 - KeyFrames + TemporalCounter / FramesPerKeyFrame]); rand();
			}
			for (i = FirstOctave; i <= Octaves; ++i) {
				Frequency = (LONG32)pow(2, i) + 2;
				//Rotate the pointers left, placing the first at the end:
				LONG32 *TempPointer = ((LONG32*)((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer);
				((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer = ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
				((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer = ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
				((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer = ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
				((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer = ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
				((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer = ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
				((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer = TempPointer;
				for (int k = 0; k < (Frequency + 1) * (Frequency + 1); ++k) {
					((LONG32*)((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer)[k] = rand();
				}
			}
		}
		//Fill the array:
		if (Fill) {
			for (YCount = 0; YCount < Height; ++YCount) {
				for (XCount = 0; XCount < Width; ++XCount) {
					TargetBitmap.SetPColour(XCount, YCount, RedM, GreenM, BlueM);
				}
				if (!DoEvents(NULL)) {
					//Loop through all the octaves (in question) and delete all the arrays:
					for (i = FirstOctave; i <= Octaves; ++i) {
						if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
					}
					if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
					if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
					if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
					if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
					if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
					if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
					return;
				}
			}
		}

		for (i = FirstOctave; i <= Octaves; ++i) {
			Frequency = (LONG32)pow(2, i) + 2;
			Amplitude = pow(Persistance, i);

			//Create a buffer to fill before it is smoothed:
			TBSmoothedBuff = new LONG32[(Frequency + 1) * (Frequency + 1)];
			if (!TBSmoothedBuff) {
				DisplayError(GetLastError());
#ifdef _DEBUG
				MessageBox(NULL, "Error occured during TBSmoothedBuff array allocation", "Error! (Debug)", MB_ICONERROR);
#endif
				//Loop through all the octaves (in question) and delete all the arrays:
				for (i = FirstOctave; i <= Octaves; ++i) {
					if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
				}
				if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
				if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
				if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
				if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
				if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
				if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
				return;
			}

			//Create a buffer to store the perlin noise for this octave:
			PerlinBuff = new LONG32[(Frequency + 1) * (Frequency + 1)];
			if (!PerlinBuff) {
				DisplayError(GetLastError());
#ifdef _DEBUG
				MessageBox(NULL, "Error occured during PerlinBuff array allocation", "Error! (Debug)", MB_ICONERROR);
#endif
				if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;
				//Loop through all the octaves (in question) and delete all the arrays:
				for (i = FirstOctave; i <= Octaves; ++i) {
					if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
				}
				if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
				if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
				if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
				if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
				if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
				if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
				return;
			}

			//Fill in the buffer:
			for (int k = 0; k < (Frequency + 1) * (Frequency + 1); ++k) {
				if (SmoothT) {
					LONG32 P = ((LONG32*)((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer)[k];
					LONG32 Q = ((LONG32*)((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer)[k];
					LONG32 R = ((LONG32*)((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer)[k];
					LONG32 S = ((LONG32*)((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer)[k];
					LONG32 T = ((LONG32*)((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer)[k];
					LONG32 U = ((LONG32*)((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer)[k];
					TBSmoothedBuff[k] = (LONG32)(InterpolateCubic((P + R) / 4 + Q / 2, (Q + S) / 4 + R / 2, (R + T) / 4 + S / 2, (S + U) / 4 + T / 2, TemporalCounter % FramesPerKeyFrame / (double)FramesPerKeyFrame));
				} else {
					LONG32 Q = ((LONG32*)((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer)[k];
					LONG32 R = ((LONG32*)((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer)[k];
					LONG32 S = ((LONG32*)((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer)[k];
					LONG32 T = ((LONG32*)((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer)[k];
					TBSmoothedBuff[k] = (LONG32)(InterpolateCubic(Q, R, S, T, TemporalCounter % FramesPerKeyFrame / (double)FramesPerKeyFrame));
				}
				//TBSmoothedBuff[k] = (LONG32)(InterpolateCubic(((LONG32*)((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer)[k], ((LONG32*)((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer)[k], ((LONG32*)((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer)[k], ((LONG32*)((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer)[k], TemporalCounter % FramesPerKeyFrame / (double)FramesPerKeyFrame));
				//TBSmoothedBuff[k] = rand();
				//PerlinBuff[k] = rand();
			}

			//Copy the To Be Smoothed Buffer to the Perlin noise buffer, smoothing as we go.
			for (YCount = 0; YCount < (Frequency + 1); ++YCount) {
				for (XCount = 0; XCount < (Frequency + 1); ++XCount) {
					PerlinBuff[YCount * (Frequency + 1) + XCount] = TBSmoothedBuff[YCount * (Frequency + 1) + XCount] / 4;
					SideCount = 4; CornerCount = 4; TempSide = 0; TempCorner = 0;
					if (XCount > 0) {
						if (YCount > 0) TempCorner += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount - 1];
						else CornerCount--;
						TempSide += TBSmoothedBuff[YCount * (Frequency + 1) + XCount - 1];
						if (YCount < Frequency) TempCorner += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount - 1];
						else CornerCount--;
					} else {
						SideCount--;
						CornerCount -= 2;
					}
					if (YCount > 0) TempSide += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount];
					else SideCount--;
					if (YCount < Frequency) TempSide += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount];
					else SideCount--;
					if (XCount < Frequency) {
						if (YCount > 0) TempCorner += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount + 1];
						else CornerCount--;
						TempSide += TBSmoothedBuff[YCount * (Frequency + 1) + XCount + 1];
						if (YCount < Frequency) TempCorner += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount + 1];
						else CornerCount--;
					} else {
						SideCount--;
						CornerCount -= 2;
					}
					PerlinBuff[YCount * (Frequency + 1) + XCount] += TempSide / (2 * SideCount) + TempCorner / (4 * CornerCount);
				}
			}

			//Delete the To Be Smoothed Beffer
			if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;

			if (HTile) {
				for (YCount = 0; YCount <= Frequency; ++YCount) {
					PerlinBuff[YCount * (Frequency + 1) + Frequency] = PerlinBuff[YCount * (Frequency + 1) + 2];
					PerlinBuff[YCount * (Frequency + 1) + Frequency - 1] = PerlinBuff[YCount * (Frequency + 1) + 1];
					PerlinBuff[YCount * (Frequency + 1)] = PerlinBuff[YCount * (Frequency + 1) + Frequency - 2];
				}
			}
			if (VTile) {
				for (XCount = 0; XCount <= Frequency; ++XCount) {
					PerlinBuff[Frequency * (Frequency + 1) + XCount] = PerlinBuff[2 * (Frequency + 1) + XCount];
					PerlinBuff[(Frequency - 1) * (Frequency + 1) + XCount] = PerlinBuff[(Frequency + 1) + XCount];
					PerlinBuff[XCount] = PerlinBuff[(Frequency - 2) * (Frequency + 1) + XCount];
				}
			}

			Frequency -= 2;

			//Draw it to the screen:
			for (YCount = 0; YCount <= Frequency; ++YCount) {
				for (XCount = 0; XCount <= Frequency; ++XCount) {
					LastRand = PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1];
					//Fill in the rectangle:
					if (RedV) TargetRed = LONG32((LastRand % RedV - RedV / 2) * Amplitude);
					if (GreenV) TargetGreen = LONG32((LastRand % GreenV - GreenV / 2) * Amplitude);
					if (BlueV) TargetBlue = LONG32((LastRand % BlueV - BlueV / 2) * Amplitude);
					FillXMin = LONG32(((double)Width / (double)Frequency) * (double)XCount);
					FillYMin = LONG32(((double)Height / (double)Frequency) * (double)YCount);
					FillXMax = LONG32((((double)Width / (double)Frequency) * double(XCount + 1) > Width) ? Width : ((double)Width / (double)Frequency) * double(XCount + 1));
					FillYMax = LONG32((((double)Height / (double)Frequency) * double(YCount + 1) > Height) ? Height : ((double)Height / (double)Frequency) * double(YCount + 1));
					if (sqrt(pow(Width / 2 - FillXMin, 2) + pow(Height / 2 - FillYMin, 2)) <= MinDist) {
						if (sqrt(pow(Width / 2 - FillXMax, 2) + pow(Height / 2 - FillYMin, 2)) <= MinDist) {
							if (sqrt(pow(Width / 2 - FillXMin, 2) + pow(Height / 2 - FillYMax, 2)) <= MinDist) {
								if (sqrt(pow(Width / 2 - FillXMax, 2) + pow(Height / 2 - FillYMax, 2)) <= MinDist) {
									if (XCount < Frequency / 2)
										XCount += 2 * (Frequency / 2 - XCount) - 1;
									continue;
								}
							}
						}
					}
					for (FillY = FillYMin; FillY < FillYMax; ++FillY) {
						for (FillX = FillXMin; FillX < FillXMax; ++FillX) {
							if (sqrt(pow(Width / 2 - FillX, 2) + pow(Height / 2 - FillY, 2)) <= MinDist) {
								FillX = (LONG32)sqrt(-pow(FillY - Height / 2, 2) + pow(MinDist, 2)) + Width / 2;
								if (FillX >= FillXMax)
									continue;
							}
							//if (sqrt(pow(Width / 2 - FillX, 2) + pow(Height / 2 - FillY, 2)) > MinDist) {
							//if (FillX < FillXMax) {
							TargetIndex = TargetBitmap.GetPixelIndex(FillX, FillY);
							double FractionalX = double(FillX - FillXMin) / double(FillXMax - FillXMin); // = Current X position in section / Width of section
							double FractionalY = double(FillY - FillYMin) / double(FillYMax - FillYMin); // = Current Y position in section / Height of section
							if (UseCubic) {
								//Cubic:
								TargetShade = LONG32(InterpolateCubic(InterpolateCubic(PerlinBuff[YCount * (Frequency + 3) + XCount], PerlinBuff[YCount * (Frequency + 3) + XCount + 1], PerlinBuff[YCount * (Frequency + 3) + XCount + 2], PerlinBuff[YCount * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 3], FractionalX), FractionalY));
								//AAAGH!!! Just wait till you see the TEMPORAL
								//Cubic interpolation that I have planned!!!
							} else if (UseLinear) {
								//Linear:
								TargetShade = LONG32(InterpolateLinear(InterpolateLinear(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], FractionalX), InterpolateLinear(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], FractionalX), FractionalY));
							} else {
								//CoSine:
								TargetShade = LONG32(Interpolate(Interpolate(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], FractionalX), Interpolate(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], FractionalX), FractionalY));
							}
							if (UseBug) {
								//The following create a wood effect:
								if (RedV) TargetRed = LONG32((TargetShade % RedV - RedV / 2) * Amplitude);
								if (GreenV) TargetGreen = LONG32((TargetShade % GreenV - GreenV / 2) * Amplitude);
								if (BlueV) TargetBlue = LONG32((TargetShade % BlueV - BlueV / 2) * Amplitude);
							} else {
								//These smooth it properly:
								if (RedV) TargetRed = LONG32(((double)TargetShade / (double)RAND_MAX * (double)RedV - RedV / 2) * Amplitude);
								if (GreenV) TargetGreen = LONG32(((double)TargetShade / (double)RAND_MAX * (double)GreenV - GreenV / 2) * Amplitude);
								if (BlueV) TargetBlue = LONG32(((double)TargetShade / (double)RAND_MAX * (double)BlueV - BlueV / 2) * Amplitude);
							}
							//Red:
							if (TargetBitmap.PixelData[TargetIndex + 2] + TargetRed > 255)
								TargetBitmap.PixelData[TargetIndex + 2] = 255;
							else if (TargetBitmap.PixelData[TargetIndex + 2] + TargetRed < 0)
								TargetBitmap.PixelData[TargetIndex + 2] = 0;
							else
								TargetBitmap.PixelData[TargetIndex + 2] += TargetRed;
							//Green:
							if (TargetBitmap.PixelData[TargetIndex + 1] + TargetGreen > 255)
								TargetBitmap.PixelData[TargetIndex + 1] = 255;
							else if (TargetBitmap.PixelData[TargetIndex + 1] + TargetGreen < 0)
								TargetBitmap.PixelData[TargetIndex + 1] = 0;
							else
								TargetBitmap.PixelData[TargetIndex + 1] += TargetGreen;
							//Blue:
							if (TargetBitmap.PixelData[TargetIndex] + TargetBlue > 255)
								TargetBitmap.PixelData[TargetIndex] = 255;
							else if (TargetBitmap.PixelData[TargetIndex] + TargetBlue < 0)
								TargetBitmap.PixelData[TargetIndex] = 0;
							else
								TargetBitmap.PixelData[TargetIndex] += TargetBlue;
							//}
						}
						if (!DoEvents(NULL)) {
							if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
							//Loop through all the octaves (in question) and delete all the arrays:
							for (i = FirstOctave; i <= Octaves; ++i) {
								if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
								if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
								if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
								if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
								if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
								if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
							}
							if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
							if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
							if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
							if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
							if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
							if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
							return;
						}
					}
				}
			}
			//Delete the buffer after each use:
			if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
		}
		switch(FileFormat) {
		case ANIMPERLINFORMATBITMAPSERIES:
			{
				//Save the bitmap:
				char FileName[MAX_PATH];
				char FrameString[MAX_PATH];
				char TempString[MAX_PATH];
				sprintf(FrameString, "%d", FramesPerKeyFrame * KeyFrames);
				int NumberLen = strlen(FrameString);
				sprintf(FrameString, "%d", TemporalCounter);
				for (int i = strlen(FrameString); i < NumberLen; ++i) {
					sprintf(TempString, "0%s", FrameString);
					strcpy(FrameString, TempString);
				}
				sprintf(FileName, "%s%s.bmp", FileNameBase, FrameString);
				//sprintf(FileName, "%s%d.bmp", FileNameBase, TemporalCounter);
				TargetBitmap.DumpImage(FileName);
			}
			break;
		case ANIMPERLINFORMATFRAMESTACK:
			{
				MessageBox(NULL, "Frame stacks are not yet supported!", "Unsupported", MB_ICONEXCLAMATION);
				return;
			}
			break;
		case ANIMPERLINFORMATAVI:
			{
				BOOL KeyFrame = FALSE;
				LONG lSize = 0;
				BYTE *CompData = NULL;
				if (CompVars->hic) {
					CompData = (BYTE*)ICSeqCompressFrame(CompVars, 0, TargetBitmap.PixelData, &KeyFrame, &lSize);
				} else {
					CompData = TargetBitmap.PixelData;
				}
				if (CompData) {
					if (CompVars->hic) {
						AVIStreamWrite(*pAviStream, TemporalCounter, 1, CompData, CompVars->lpbiOut->bmiHeader.biSizeImage, (KeyFrame ? AVIIF_KEYFRAME : 0), NULL, NULL);
					} else {
						AVIStreamWrite(*pAviStream, TemporalCounter, 1, CompData, TargetBitmap.UBound(), AVIIF_KEYFRAME, NULL, NULL);
					}
				} else {
					MessageBox(NULL, "Error during video compression!", "Compression error", MB_ICONEXCLAMATION);
					//Loop through all the octaves (in question) and delete all the arrays:
					if (CompData) delete [] CompData;
					for (i = FirstOctave; i <= Octaves; ++i) {
						if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
					}
					if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
					if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
					if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
					if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
					if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
					if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
					return;
				}
			}
			break;
		}
	}
	//Loop through all the octaves (in question) and delete all the arrays:
	for (i = FirstOctave; i <= Octaves; ++i) {
		if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
		if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
		if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
		if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
		if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
		if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
	}
	if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
	if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
	if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
	if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
	if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
	if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
}







/*
This function creates animating perlin noise. It is based on my
FillPerlinTemporal formula, but designed to use a gradient to
make the result look better.  The CreatePerlinGradient function
can be used to create a gradient compatible with this function.
Also, note that if:
0x40000000 is set, red is negative.
0x20000000 is set, green is negative.
0x10000000 is set, blue is negative.
I am also planning the same memory conservation mode as described
in the above FillPerlinTemporal funstion description.
Flags parameters (Combine with |):
1  - Fill the array with the 'middle' colour (Otherwise the middle colour is completely ignored)
2  - Bug (ie. Wood effect)
4  - Tile Horizontally
8  - Tile Vertically
16 - Disable array Smoothing (Unsupported)
32 - Linear \
64 - Cubic  - If neither are present, CoSine is assumed. Cubic overrides linear.
128 - Memory conservation mode (Unsupported)
256 - Loop
512 - Smooth the time axis
*/
void FillPerlinGradientTemporal(BitmapOps &TargetBitmap, WORD Flags, COLORREF MiddleColour, DWORD NumGradEntries, COLORREF *GradientArray, double Persistance, LONG32 Octaves, LONG32 FirstOctave, LONG32 MinDist, LPSTR FileNameBase, LONG32 KeyFrames, LONG32 FramesPerKeyFrame, LONG32 FileFormat = ANIMPERLINFORMATBITMAPSERIES, PCOMPVARS CompVars = NULL, PAVISTREAM *pAviStream = NULL) {
	if (!bRunning) return;
	LONG32 Total = 0;
	LONG32 Frequency = 1;
	double Amplitude = 1;
	LONG32 LastRand = 0;
	LONG32 XCount = 0;
	LONG32 YCount = 0;
	LONG32 FillX = 0;
	LONG32 FillY = 0;
	LONG32 FillXMax = 0;
	LONG32 FillYMax = 0;
	LONG32 FillXMin = 0;
	LONG32 FillYMin = 0;
	LONG32 TargetShade = 0;
	LONG32 TargetRed = 0;
	LONG32 TargetGreen = 0;
	LONG32 TargetBlue = 0;
	LONG32 Height = (LONG32)TargetBitmap.Height();
	LONG32 Width = (LONG32)TargetBitmap.Width();
	LONG32 *TBSmoothedBuff = 0;
	LONG32 *PerlinBuff = 0;
	DWORD TargetIndex = 0;
	LONG32 SideCount, CornerCount, TempSide, TempCorner; //For smoothing
	LONG32 RedM = GetRValue(MiddleColour);
	LONG32 GreenM = GetGValue(MiddleColour);
	LONG32 BlueM = GetBValue(MiddleColour);
	BOOL Fill = Flags & 1;
	BOOL UseBug = Flags & 2;
	BOOL HTile = Flags & 4;
	BOOL VTile = Flags & 8;
	//Disable array smoothing - 16
	BOOL UseLinear = Flags & 32;
	BOOL UseCubic = Flags & 64;
	//Memory conservation mode - 128
	BOOL Loop = Flags & 256;
	BOOL SmoothT = Flags & 512;
	if (Octaves < 1)
		Octaves--;


	//Autodetect the required number of octaves if it is less than 0 (specified 0 or less - it was decreased by one above)
	int i = 1;
	while (Octaves < 0) {
		Frequency = (LONG32)pow(2, i);
		if (Width > Height) {
			if (Frequency >= Height)
				Octaves = i + Octaves + 1;
		} else {
			if (Frequency >= Width)
				Octaves = i + Octaves + 1;
		}
		i++;
	}

	//Create an array of 5 that we store some seeds in, so that we can re-create the beginning key frames if loop is enabled:
	LONG32 SeedArray[5] = {rand(), rand(), rand(), rand(), rand()};
	//Unlike before, we need to keep accessing the octave data, so let's prepare some:
	//First, an array of four pointers:
	ArrayOfArrays TemporalArray[6];
	//Each entry must point to an array with as many elements as there are octaves:
	TemporalArray[0].Pointer = new ArrayOfArrays[Octaves + 1];
	TemporalArray[1].Pointer = new ArrayOfArrays[Octaves + 1];
	TemporalArray[2].Pointer = new ArrayOfArrays[Octaves + 1];
	TemporalArray[3].Pointer = new ArrayOfArrays[Octaves + 1];
	TemporalArray[4].Pointer = new ArrayOfArrays[Octaves + 1];
	TemporalArray[5].Pointer = new ArrayOfArrays[Octaves + 1];
	//If any array failed to create, clean up any survivors and return:
	//if (!((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) {
	if (!((ArrayOfArrays*)TemporalArray[0].Pointer) || !((ArrayOfArrays*)TemporalArray[1].Pointer) || !((ArrayOfArrays*)TemporalArray[2].Pointer) || !((ArrayOfArrays*)TemporalArray[3].Pointer) || !((ArrayOfArrays*)TemporalArray[4].Pointer) || !((ArrayOfArrays*)TemporalArray[5].Pointer)) {
		DisplayError(GetLastError());
#ifdef _DEBUG
		MessageBox(NULL, "Error occured during temporal array allocation", "Error! (Debug)", MB_ICONERROR);
#endif
		if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
		if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
		if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
		if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
		if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
		if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
		return;
	}
	//Loop through all the octaves (in question) and create heaps of arrays:
	BOOL ErrorOccured = FALSE;
	for (i = FirstOctave; i <= Octaves; ++i) {
		Frequency = (LONG32)pow(2, i) + 2;
		((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer = new LONG32[(Frequency + 1) * (Frequency + 1)];
		if (!((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer || !((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer)
			ErrorOccured = TRUE;
	}
	if (ErrorOccured) {
		DisplayError(GetLastError());
#ifdef _DEBUG
		MessageBox(NULL, "Error occured during octave array allocation", "Error! (Debug)", MB_ICONERROR);
#endif
		//Loop through all the octaves (in question) and delete all the arrays:
		for (i = FirstOctave; i <= Octaves; ++i) {
			if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
			if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
			if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
			if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
			if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
			if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
		}
		if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
		if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
		if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
		if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
		if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
		if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
		return;
	}
	for (int l = 0; l < 5; l++) {
		srand(SeedArray[l]); rand();
		for (i = FirstOctave; i <= Octaves; ++i) {
			Frequency = (LONG32)pow(2, i) + 2;
			//Fill in the buffers except the first, as it will be filled in shortly:
			//if (!ErrorOccured) {
			for (int k = 0; k < (Frequency + 1) * (Frequency + 1); ++k) {
				((LONG32*)((ArrayOfArrays*)TemporalArray[l + 1].Pointer)[i].Pointer)[k] = rand();
				/*((LONG32*)((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer)[k] = rand();
				((LONG32*)((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer)[k] = rand();
				((LONG32*)((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer)[k] = rand();
				((LONG32*)((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer)[k] = rand();
				((LONG32*)((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer)[k] = rand();*/
			}
			//}
		}
	}

	for (int TemporalCounter = 0; TemporalCounter < FramesPerKeyFrame * KeyFrames; TemporalCounter++) {
		AnimInfo.CurrentFrame = TemporalCounter + 1;
		if (TemporalCounter % FramesPerKeyFrame == 0) {
			if (Loop && KeyFrames - TemporalCounter / FramesPerKeyFrame < 6) { //Only the last five should register
				srand(SeedArray[5 - KeyFrames + TemporalCounter / FramesPerKeyFrame]); rand();
			}
			for (i = FirstOctave; i <= Octaves; ++i) {
				Frequency = (LONG32)pow(2, i) + 2;
				//Rotate the pointers left, placing the first at the end:
				LONG32 *TempPointer = ((LONG32*)((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer);
				((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer = ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
				((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer = ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
				((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer = ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
				((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer = ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
				((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer = ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
				((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer = TempPointer;
				for (int k = 0; k < (Frequency + 1) * (Frequency + 1); ++k) {
					((LONG32*)((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer)[k] = rand();
				}
			}
		}
		//Fill the array:
		if (Fill) {
			for (YCount = 0; YCount < Height; ++YCount) {
				for (XCount = 0; XCount < Width; ++XCount) {
					TargetBitmap.SetPColour(XCount, YCount, RedM, GreenM, BlueM);
				}
				if (!DoEvents(NULL)) {
					//Loop through all the octaves (in question) and delete all the arrays:
					for (i = FirstOctave; i <= Octaves; ++i) {
						if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
					}
					if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
					if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
					if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
					if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
					if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
					if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
					return;
				}
			}
		}

		for (i = FirstOctave; i <= Octaves; ++i) {
			Frequency = (LONG32)pow(2, i) + 2;
			Amplitude = pow(Persistance, i);

			//Create a buffer to fill before it is smoothed:
			TBSmoothedBuff = new LONG32[(Frequency + 1) * (Frequency + 1)];
			if (!TBSmoothedBuff) {
				DisplayError(GetLastError());
#ifdef _DEBUG
				MessageBox(NULL, "Error occured during TBSmoothedBuff array allocation", "Error! (Debug)", MB_ICONERROR);
#endif
				//Loop through all the octaves (in question) and delete all the arrays:
				for (i = FirstOctave; i <= Octaves; ++i) {
					if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
				}
				if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
				if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
				if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
				if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
				if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
				if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
				return;
			}

			//Create a buffer to store the perlin noise for this octave:
			PerlinBuff = new LONG32[(Frequency + 1) * (Frequency + 1)];
			if (!PerlinBuff) {
				DisplayError(GetLastError());
#ifdef _DEBUG
				MessageBox(NULL, "Error occured during PerlinBuff array allocation", "Error! (Debug)", MB_ICONERROR);
#endif
				if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;
				//Loop through all the octaves (in question) and delete all the arrays:
				for (i = FirstOctave; i <= Octaves; ++i) {
					if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
					if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
				}
				if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
				if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
				if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
				if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
				if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
				if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
				return;
			}

			//Fill in the buffer:
			for (int k = 0; k < (Frequency + 1) * (Frequency + 1); ++k) {
				if (SmoothT) {
					LONG32 P = ((LONG32*)((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer)[k];
					LONG32 Q = ((LONG32*)((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer)[k];
					LONG32 R = ((LONG32*)((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer)[k];
					LONG32 S = ((LONG32*)((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer)[k];
					LONG32 T = ((LONG32*)((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer)[k];
					LONG32 U = ((LONG32*)((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer)[k];
					TBSmoothedBuff[k] = (LONG32)(InterpolateCubic((P + R) / 4 + Q / 2, (Q + S) / 4 + R / 2, (R + T) / 4 + S / 2, (S + U) / 4 + T / 2, TemporalCounter % FramesPerKeyFrame / (double)FramesPerKeyFrame));
				} else {
					LONG32 Q = ((LONG32*)((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer)[k];
					LONG32 R = ((LONG32*)((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer)[k];
					LONG32 S = ((LONG32*)((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer)[k];
					LONG32 T = ((LONG32*)((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer)[k];
					TBSmoothedBuff[k] = (LONG32)(InterpolateCubic(Q, R, S, T, TemporalCounter % FramesPerKeyFrame / (double)FramesPerKeyFrame));
				}
				//TBSmoothedBuff[k] = (LONG32)(InterpolateCubic(((LONG32*)((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer)[k], ((LONG32*)((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer)[k], ((LONG32*)((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer)[k], ((LONG32*)((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer)[k], TemporalCounter % FramesPerKeyFrame / (double)FramesPerKeyFrame));
				//TBSmoothedBuff[k] = rand();
				//PerlinBuff[k] = rand();
			}

			//Copy the To Be Smoothed Buffer to the Perlin noise buffer, smoothing as we go.
			for (YCount = 0; YCount < (Frequency + 1); ++YCount) {
				for (XCount = 0; XCount < (Frequency + 1); ++XCount) {
					PerlinBuff[YCount * (Frequency + 1) + XCount] = TBSmoothedBuff[YCount * (Frequency + 1) + XCount] / 4;
					SideCount = 4; CornerCount = 4; TempSide = 0; TempCorner = 0;
					if (XCount > 0) {
						if (YCount > 0) TempCorner += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount - 1];
						else CornerCount--;
						TempSide += TBSmoothedBuff[YCount * (Frequency + 1) + XCount - 1];
						if (YCount < Frequency) TempCorner += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount - 1];
						else CornerCount--;
					} else {
						SideCount--;
						CornerCount -= 2;
					}
					if (YCount > 0) TempSide += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount];
					else SideCount--;
					if (YCount < Frequency) TempSide += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount];
					else SideCount--;
					if (XCount < Frequency) {
						if (YCount > 0) TempCorner += TBSmoothedBuff[(YCount - 1) * (Frequency + 1) + XCount + 1];
						else CornerCount--;
						TempSide += TBSmoothedBuff[YCount * (Frequency + 1) + XCount + 1];
						if (YCount < Frequency) TempCorner += TBSmoothedBuff[(YCount + 1) * (Frequency + 1) + XCount + 1];
						else CornerCount--;
					} else {
						SideCount--;
						CornerCount -= 2;
					}
					PerlinBuff[YCount * (Frequency + 1) + XCount] += TempSide / (2 * SideCount) + TempCorner / (4 * CornerCount);
				}
			}

			//Delete the To Be Smoothed Beffer
			if (TBSmoothedBuff) delete [] TBSmoothedBuff; TBSmoothedBuff = 0;

			if (HTile) {
				for (YCount = 0; YCount <= Frequency; ++YCount) {
					PerlinBuff[YCount * (Frequency + 1) + Frequency] = PerlinBuff[YCount * (Frequency + 1) + 2];
					PerlinBuff[YCount * (Frequency + 1) + Frequency - 1] = PerlinBuff[YCount * (Frequency + 1) + 1];
					PerlinBuff[YCount * (Frequency + 1)] = PerlinBuff[YCount * (Frequency + 1) + Frequency - 2];
				}
			}
			if (VTile) {
				for (XCount = 0; XCount <= Frequency; ++XCount) {
					PerlinBuff[Frequency * (Frequency + 1) + XCount] = PerlinBuff[2 * (Frequency + 1) + XCount];
					PerlinBuff[(Frequency - 1) * (Frequency + 1) + XCount] = PerlinBuff[(Frequency + 1) + XCount];
					PerlinBuff[XCount] = PerlinBuff[(Frequency - 2) * (Frequency + 1) + XCount];
				}
			}

			Frequency -= 2;

			//Draw it to the screen:
			for (YCount = 0; YCount <= Frequency; ++YCount) {
				for (XCount = 0; XCount <= Frequency; ++XCount) {
					LastRand = PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1];
					//Fill in the rectangle:
//					if (RedV) TargetRed = LONG32((LastRand % RedV - RedV / 2) * Amplitude);
//					if (GreenV) TargetGreen = LONG32((LastRand % GreenV - GreenV / 2) * Amplitude);
//					if (BlueV) TargetBlue = LONG32((LastRand % BlueV - BlueV / 2) * Amplitude);
					FillXMin = LONG32(((double)Width / (double)Frequency) * (double)XCount);
					FillYMin = LONG32(((double)Height / (double)Frequency) * (double)YCount);
					FillXMax = LONG32((((double)Width / (double)Frequency) * double(XCount + 1) > Width) ? Width : ((double)Width / (double)Frequency) * double(XCount + 1));
					FillYMax = LONG32((((double)Height / (double)Frequency) * double(YCount + 1) > Height) ? Height : ((double)Height / (double)Frequency) * double(YCount + 1));
					if (sqrt(pow(Width / 2 - FillXMin, 2) + pow(Height / 2 - FillYMin, 2)) <= MinDist) {
						if (sqrt(pow(Width / 2 - FillXMax, 2) + pow(Height / 2 - FillYMin, 2)) <= MinDist) {
							if (sqrt(pow(Width / 2 - FillXMin, 2) + pow(Height / 2 - FillYMax, 2)) <= MinDist) {
								if (sqrt(pow(Width / 2 - FillXMax, 2) + pow(Height / 2 - FillYMax, 2)) <= MinDist) {
									if (XCount < Frequency / 2)
										XCount += 2 * (Frequency / 2 - XCount) - 1;
									continue;
								}
							}
						}
					}
					for (FillY = FillYMin; FillY < FillYMax; ++FillY) {
						for (FillX = FillXMin; FillX < FillXMax; ++FillX) {
							if (sqrt(pow(Width / 2 - FillX, 2) + pow(Height / 2 - FillY, 2)) <= MinDist) {
								FillX = (LONG32)sqrt(-pow(FillY - Height / 2, 2) + pow(MinDist, 2)) + Width / 2;
								if (FillX >= FillXMax)
									continue;
							}
							//if (sqrt(pow(Width / 2 - FillX, 2) + pow(Height / 2 - FillY, 2)) > MinDist) {
							//if (FillX < FillXMax) {
							TargetIndex = TargetBitmap.GetPixelIndex(FillX, FillY);
							double FractionalX = double(FillX - FillXMin) / double(FillXMax - FillXMin); // = Current X position in section / Width of section
							double FractionalY = double(FillY - FillYMin) / double(FillYMax - FillYMin); // = Current Y position in section / Height of section
							if (UseCubic) {
								//Cubic:
								TargetShade = LONG32(InterpolateCubic(InterpolateCubic(PerlinBuff[YCount * (Frequency + 3) + XCount], PerlinBuff[YCount * (Frequency + 3) + XCount + 1], PerlinBuff[YCount * (Frequency + 3) + XCount + 2], PerlinBuff[YCount * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 3], FractionalX), InterpolateCubic(PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 2], PerlinBuff[(YCount + 3) * (Frequency + 3) + XCount + 3], FractionalX), FractionalY));
							} else if (UseLinear) {
								//Linear:
								TargetShade = LONG32(InterpolateLinear(InterpolateLinear(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], FractionalX), InterpolateLinear(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], FractionalX), FractionalY));
							} else {
								//CoSine:
								TargetShade = LONG32(Interpolate(Interpolate(PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 1) * (Frequency + 3) + XCount + 2], FractionalX), Interpolate(PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 1], PerlinBuff[(YCount + 2) * (Frequency + 3) + XCount + 2], FractionalX), FractionalY));
							}
							if (UseBug) {
								//The following create a wood effect:
								//if (RedV) TargetRed = LONG32((TargetShade % RedV - RedV / 2) * Amplitude);
								//if (GreenV) TargetGreen = LONG32((TargetShade % GreenV - GreenV / 2) * Amplitude);
								//if (BlueV) TargetBlue = LONG32((TargetShade % BlueV - BlueV / 2) * Amplitude);
								TargetShade = (TargetShade % NumGradEntries); 
							} else {
								//These smooth it properly:
								//if (RedV) TargetRed = LONG32(((double)TargetShade / (double)RAND_MAX * (double)RedV - RedV / 2) * Amplitude);
								//if (GreenV) TargetGreen = LONG32(((double)TargetShade / (double)RAND_MAX * (double)GreenV - GreenV / 2) * Amplitude);
								//if (BlueV) TargetBlue = LONG32(((double)TargetShade / (double)RAND_MAX * (double)BlueV - BlueV / 2) * Amplitude);
								FractionalX = (double)TargetShade / (double)RAND_MAX * (double)NumGradEntries;
								TargetShade = (LONG32)FractionalX;
								FractionalX -= TargetShade;
							}
							/*TargetRed = GetRValue(GradientArray[TargetShade]) * Amplitude;
							TargetGreen = GetGValue(GradientArray[TargetShade]) * Amplitude;
							TargetBlue = GetBValue(GradientArray[TargetShade]) * Amplitude;*/
							LONG32 Val1, Val2, Val3, Val4;
							if (TargetShade) Val1 = TargetShade - 1;
							else Val1 = TargetShade;
							Val2 = TargetShade;
							if (TargetShade < NumGradEntries - 1) Val3 = TargetShade + 1;
							else Val3 = TargetShade;
							if (TargetShade < NumGradEntries) Val4 = TargetShade + 2;
							else Val4 = TargetShade;
							TargetRed = InterpolateCubic(GetRValue(GradientArray[Val1]), GetRValue(GradientArray[Val2]), GetRValue(GradientArray[Val3]), GetRValue(GradientArray[Val4]), FractionalX) * Amplitude;
							TargetGreen = InterpolateCubic(GetGValue(GradientArray[Val1]), GetGValue(GradientArray[Val2]), GetGValue(GradientArray[Val3]), GetGValue(GradientArray[Val4]), FractionalX) * Amplitude;
							TargetBlue = InterpolateCubic(GetBValue(GradientArray[Val1]), GetBValue(GradientArray[Val2]), GetBValue(GradientArray[Val3]), GetBValue(GradientArray[Val4]), FractionalX) * Amplitude;
							//Red:
							LONG32 Multiplier = (GradientArray[TargetShade] & 0x40000000 ? -1 : 1);
							if (TargetBitmap.PixelData[TargetIndex + 2] + TargetRed * Multiplier > 255)
								TargetBitmap.PixelData[TargetIndex + 2] = 255;
							else if (TargetBitmap.PixelData[TargetIndex + 2] + TargetRed * Multiplier < 0)
								TargetBitmap.PixelData[TargetIndex + 2] = 0;
							else
								TargetBitmap.PixelData[TargetIndex + 2] += TargetRed * Multiplier;
							//Green:
							Multiplier = (GradientArray[TargetShade] & 0x20000000 ? -1 : 1);
							if (TargetBitmap.PixelData[TargetIndex + 1] + TargetGreen * Multiplier > 255)
								TargetBitmap.PixelData[TargetIndex + 1] = 255;
							else if (TargetBitmap.PixelData[TargetIndex + 1] + TargetGreen * Multiplier < 0)
								TargetBitmap.PixelData[TargetIndex + 1] = 0;
							else
								TargetBitmap.PixelData[TargetIndex + 1] += TargetGreen * Multiplier;
							//Blue:
							Multiplier = (GradientArray[TargetShade] & 0x10000000 ? -1 : 1);
							if (TargetBitmap.PixelData[TargetIndex] + TargetBlue * Multiplier > 255)
								TargetBitmap.PixelData[TargetIndex] = 255;
							else if (TargetBitmap.PixelData[TargetIndex] + TargetBlue * Multiplier < 0)
								TargetBitmap.PixelData[TargetIndex] = 0;
							else
								TargetBitmap.PixelData[TargetIndex] += TargetBlue * Multiplier;
							//}
						}
						if (!DoEvents(NULL)) {
							if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
							//Loop through all the octaves (in question) and delete all the arrays:
							for (i = FirstOctave; i <= Octaves; ++i) {
								if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
								if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
								if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
								if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
								if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
								if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
							}
							if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
							if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
							if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
							if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
							if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
							if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
							return;
						}
					}
				}
			}
			//Delete the buffer after each use:
			if (PerlinBuff) delete [] PerlinBuff; PerlinBuff = 0;
		}
		switch(FileFormat) {
		case ANIMPERLINFORMATBITMAPSERIES:
			{
				//Save the bitmap:
				char FileName[MAX_PATH];
				char FrameString[MAX_PATH];
				char TempString[MAX_PATH];
				sprintf(FrameString, "%d", FramesPerKeyFrame * KeyFrames);
				int NumberLen = strlen(FrameString);
				sprintf(FrameString, "%d", TemporalCounter);
				for (int i = strlen(FrameString); i < NumberLen; ++i) {
					sprintf(TempString, "0%s", FrameString);
					strcpy(FrameString, TempString);
				}
				sprintf(FileName, "%s%s.bmp", FileNameBase, FrameString);
				//sprintf(FileName, "%s%d.bmp", FileNameBase, TemporalCounter);
				TargetBitmap.DumpImage(FileName);
			}
			break;
		case ANIMPERLINFORMATFRAMESTACK:
			{
				MessageBox(NULL, "Frame stacks are not yet supported!", "Unsupported", MB_ICONEXCLAMATION);
				return;
			}
			break;
		case ANIMPERLINFORMATAVI:
			{
				BOOL KeyFrame = FALSE;
				LONG lSize = 0;
				BYTE *CompData = NULL;
				if (CompVars->hic) {
					CompData = (BYTE*)ICSeqCompressFrame(CompVars, 0, TargetBitmap.PixelData, &KeyFrame, &lSize);
				} else {
					CompData = TargetBitmap.PixelData;
				}
				if (CompData) {
					if (CompVars->hic) {
						AVIStreamWrite(*pAviStream, TemporalCounter, 1, CompData, CompVars->lpbiOut->bmiHeader.biSizeImage, (KeyFrame ? AVIIF_KEYFRAME : 0), NULL, NULL);
					} else {
						AVIStreamWrite(*pAviStream, TemporalCounter, 1, CompData, TargetBitmap.UBound(), AVIIF_KEYFRAME, NULL, NULL);
					}
				} else {
					MessageBox(NULL, "Error during video compression!", "Compression error", MB_ICONEXCLAMATION);
					//Loop through all the octaves (in question) and delete all the arrays:
					if (CompData) delete [] CompData;
					for (i = FirstOctave; i <= Octaves; ++i) {
						if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
						if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
					}
					if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
					if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
					if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
					if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
					if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
					if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
					return;
				}
			}
			break;
		}
	}
	//Loop through all the octaves (in question) and delete all the arrays:
	for (i = FirstOctave; i <= Octaves; ++i) {
		if (((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[0].Pointer)[i].Pointer;
		if (((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[1].Pointer)[i].Pointer;
		if (((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[2].Pointer)[i].Pointer;
		if (((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[3].Pointer)[i].Pointer;
		if (((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[4].Pointer)[i].Pointer;
		if (((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer) delete [] ((ArrayOfArrays*)TemporalArray[5].Pointer)[i].Pointer;
	}
	if (TemporalArray[0].Pointer) delete [] TemporalArray[0].Pointer; TemporalArray[0].Pointer = NULL;
	if (TemporalArray[1].Pointer) delete [] TemporalArray[1].Pointer; TemporalArray[1].Pointer = NULL;
	if (TemporalArray[2].Pointer) delete [] TemporalArray[2].Pointer; TemporalArray[2].Pointer = NULL;
	if (TemporalArray[3].Pointer) delete [] TemporalArray[3].Pointer; TemporalArray[3].Pointer = NULL;
	if (TemporalArray[4].Pointer) delete [] TemporalArray[4].Pointer; TemporalArray[4].Pointer = NULL;
	if (TemporalArray[5].Pointer) delete [] TemporalArray[5].Pointer; TemporalArray[5].Pointer = NULL;
}






//END 1,0,1,0 Additions


/*
This function will either (depending on 'Position'):
0: Completely fill in the given BitmapOps with the given colour,
1: Attempt to underlay the image in the BitmapOps with the given colour,
2: Merge the BitmapOps image with the colour
*/
void DrawSolidColour(BitmapOps &SPBitmap, uchar Red, uchar Green, uchar Blue, int Position) {
	LONG32 Width = SPBitmap.Width(), Height = SPBitmap.Height();
	LONG32 x = 0, y = 0;
	DWORD i = 0;
	srand(GetTickCount());rand(); //Seed the random number generator
	if (bDrawing || !bRunning) return;
	switch(Position) {
	case 0: //Replace
		{
			for (y = 0; y < Height; y ++) {
				for (x = 0; x < Width; x ++) {
					i = SPBitmap.GetPixelIndex(x, y);
					SPBitmap.PixelData[i] = Blue;
					SPBitmap.PixelData[i + 1] = Green;
					SPBitmap.PixelData[i + 2] = Red;
				}
			}
		}
		break;
	case 1: //Underlay
		{
			for (y = 0; y < Height; y ++) {
				for (x = 0; x < Width; x ++) {
					i = SPBitmap.GetPixelIndex(x, y);
					LONG32 TempRed = SPBitmap.PixelData[i + 2];
					LONG32 TempGreen = SPBitmap.PixelData[i + 1];
					LONG32 TempBlue = SPBitmap.PixelData[i];
					LONG32 TempAvg = (TempRed + TempGreen + TempBlue) / 3;
					//if ((Red + Green + Blue) / 3 > TempAvg) {
					if (Red - TempAvg > TempRed) TempRed = Red - TempAvg;
					if (Green - TempAvg > TempGreen) TempGreen = Green - TempAvg;
					if (Blue - TempAvg > TempBlue) TempBlue = Blue - TempAvg;
					//}
					if (TempRed > 255) TempRed = 255;
					if (TempGreen > 255) TempGreen = 255;
					if (TempBlue > 255) TempBlue = 255;
					SPBitmap.PixelData[i] = (uchar)TempBlue;
					SPBitmap.PixelData[i + 1] = (uchar)TempGreen;
					SPBitmap.PixelData[i + 2] = (uchar)TempRed;
				}
			}
		}
		break;
	case 2: //Overlay
		{
			for (y = 0; y < Height; y ++) {
				for (x = 0; x < Width; x ++) {
					i = SPBitmap.GetPixelIndex(x, y);
					SPBitmap.PixelData[i] = (SPBitmap.PixelData[i] + Blue) / 2;
					SPBitmap.PixelData[i + 1] = (SPBitmap.PixelData[i + 1] + Green) / 2;
					SPBitmap.PixelData[i + 2] = (SPBitmap.PixelData[i + 2] + Red) / 2;
				}
			}
		}
		break;
	}
}

/*
This function draws a nebula using the settings
passed in the SunProjectNebulaParams
*/
void DrawNebula(BitmapOps &SPBitmap, SunProjectNebulaParams &NebParams, LONG32 MinDist) {
	if (bDrawing || !bRunning) return;
	bDrawing = true;
	BitmapOps SPUnderlayBuffer(1, 1);
	//BitmapOps SPAntiAliasBuffer(1, 1);
	for (int i = 0; i < (NebParams.Animate ? 1 : NebParams.NumLayers); i++) {
		//Seed the random number generator:
		srand(GetTickCount());rand();
		//Fill in the last seed:
		if (NebParams.LayerParams[i].Seed) {
			NebParams.LayerParams[i].LastSeed = NebParams.LayerParams[i].Seed;
		} else {
			NebParams.LayerParams[i].LastSeed = rand();
		}
		//Seed the random number generator with the last seed value
		srand(NebParams.LayerParams[i].LastSeed);rand();
		//If the previous image is to be underlayed, copy it into a buffer:
		if (NebParams.LayerParams[i].KeepPrevious == 1) {
			SPUnderlayBuffer.CopyBitmapFromBitmapOps(SPBitmap);
		}
		//Antialiasing may be OK if it is first smoothed, but it looks pretty crap otherwise and takes up too much processing power (Besides, it's unecessary unless the 'bug' is active)...
		/*if (1) { //Anti-Aliased
			//If it is to be anti-aliased, make the anti-alias buffer 4x as large as the normal buffer:
			SPAntiAliasBuffer.Resize(SPBitmap.Width() * 2, SPBitmap.Height() * 2);
			//Draw the nebula for this layer:
			FillPerlin((NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0), NebParams.LayerParams[i].FirstOctave, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, SPAntiAliasBuffer, GetRValue(NebParams.LayerParams[i].MiddleColour), NebParams.LayerParams[i].RedVariance, GetGValue(NebParams.LayerParams[i].MiddleColour), NebParams.LayerParams[i].GreenVariance, GetBValue(NebParams.LayerParams[i].MiddleColour), NebParams.LayerParams[i].BlueVariance, MinDist);
			//Copy it back into the original bitmap, anti-aliasing as we go
			for (int j = 0; j < SPBitmap.Height(); j++) {
				for (int l = 0; l < SPBitmap.Width() ; l++) {
					DWORD TempIndex1 = SPAntiAliasBuffer.GetPixelIndex(l * 2, j * 2);
					DWORD TempIndex2 = SPAntiAliasBuffer.GetPixelIndex(l * 2 + 1, j * 2);
					DWORD TempIndex3 = SPAntiAliasBuffer.GetPixelIndex(l * 2, j * 2 + 1);
					DWORD TempIndex4 = SPAntiAliasBuffer.GetPixelIndex(l * 2 + 1, j * 2 + 1);
					SPBitmap.SetPColour(l, j, (SPAntiAliasBuffer.PixelData[TempIndex1 + 2] + SPAntiAliasBuffer.PixelData[TempIndex2 + 2] + SPAntiAliasBuffer.PixelData[TempIndex3 + 2] + SPAntiAliasBuffer.PixelData[TempIndex4 + 2]) / 4, (SPAntiAliasBuffer.PixelData[TempIndex1 + 1] + SPAntiAliasBuffer.PixelData[TempIndex2 + 1] + SPAntiAliasBuffer.PixelData[TempIndex3 + 1] + SPAntiAliasBuffer.PixelData[TempIndex4 + 1]) / 4, (SPAntiAliasBuffer.PixelData[TempIndex1] + SPAntiAliasBuffer.PixelData[TempIndex2] + SPAntiAliasBuffer.PixelData[TempIndex3] + SPAntiAliasBuffer.PixelData[TempIndex4]) / 4);
				}
			}
		} else {*/
		//Draw the nebula for this layer:
		if (NebParams.LayerParams[i].UseGradient) {
			//Let's see what a gradient looks like:
			//COLORREF TestGradient[512];
			//CreatePerlinGradient((NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (!NebParams.LayerParams[i].Interpolation ? 16 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 32 : 0), NebParams.LayerParams[i].MiddleColour, 512, TestGradient, NebParams.LayerParams[i].RedVariance, NebParams.LayerParams[i].GreenVariance, NebParams.LayerParams[i].BlueVariance, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave);
			//FillPerlinGradient(SPBitmap, (NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0) | (!NebParams.LayerParams[i].Interpolation ? 32 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 64 : 0), NebParams.LayerParams[i].MiddleColour, 512, TestGradient, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave, MinDist);
			DWORD NumGradEntries = NebParams.LayerParams[i].NumGradientEntries;
			COLORREF *GradientArray = new COLORREF[NumGradEntries];
			memcpy(GradientArray, NebParams.LayerParams[i].GradientArray, NumGradEntries * sizeof(COLORREF));
			{
				LONG32 LRed = 0, LGreen = 0, LBlue = 0, TargetRed, TargetGreen, TargetBlue;
				BOOL MRed, MGreen, MBlue;
				//Average all the entries:
				for (DWORD Count = 0; Count < NumGradEntries; Count++) {
					LRed += GetRValue(GradientArray[Count]);
					LGreen += GetGValue(GradientArray[Count]);
					LBlue += GetBValue(GradientArray[Count]);
				}
				LRed /= NumGradEntries;
				LGreen /= NumGradEntries;
				LBlue /= NumGradEntries;
				for (Count = 0; Count < NumGradEntries; Count++) {
					TargetRed = GetRValue(GradientArray[Count]);
					TargetGreen = GetGValue(GradientArray[Count]);
					TargetBlue = GetBValue(GradientArray[Count]);
					if (TargetRed > LRed) {
						TargetRed -= LRed;
						MRed = TRUE;
					} else {
						TargetRed = LRed - TargetRed;
						MRed = FALSE;
					}
					if (TargetGreen > LGreen) {
						TargetGreen -= LGreen;
						MGreen = TRUE;
					} else {
						TargetGreen = LGreen - TargetGreen;
						MGreen = FALSE;
					}
					if (TargetBlue > LBlue) {
						TargetBlue -= LBlue;
						MBlue = TRUE;
					} else {
						TargetBlue = LBlue - TargetBlue;
						MBlue = FALSE;
					}
					GradientArray[Count] = RGB(TargetRed, TargetGreen, TargetBlue);
					if (!MRed) GradientArray[Count] |= 0x40000000;
					if (!MGreen) GradientArray[Count] |= 0x20000000;
					if (!MBlue) GradientArray[Count] |= 0x10000000;
				}
			}
			if (NebParams.Animate) {
				char tmpFileType[5];
				strcpy(tmpFileType, &NebParams.BaseFileName[strlen(NebParams.BaseFileName) - 4]);
				if (!_stricmp(tmpFileType, ".avi")) {
					//AVI Test using VCM:
					COMPVARS CompVars = {0};
					CompVars.cbSize = sizeof(COMPVARS);
					BITMAPINFO bmInfo = {0};
					bmInfo.bmiHeader.biWidth = SPBitmap.Width();
					bmInfo.bmiHeader.biBitCount = SPBitmap.BPP();
					bmInfo.bmiHeader.biHeight = SPBitmap.Height();
					bmInfo.bmiHeader.biCompression = BI_RGB;
					bmInfo.bmiHeader.biPlanes = 1;
					bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);
					bmInfo.bmiHeader.biSizeImage = SPBitmap.UBound();
					AVIFileInit(); //Initialise the AVI library
					PAVIFILE AviFile = {0};
					PAVISTREAM AviStream = {0};
					AVISTREAMINFO AviStreamInfo = {0};
					if (ICCompressorChoose(NULL, ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME, &bmInfo, NULL, &CompVars, NULL)) {
						if (AVIFileOpen(&AviFile, NebParams.BaseFileName, OF_CREATE | OF_WRITE, NULL)) {
							MessageBox(NULL, "Error creating the AVI file!", "File creation error!", MB_ICONEXCLAMATION);
							ICCompressorFree(&CompVars);
							AVIFileExit(); //Kill the AVI library
							bDrawing = false;
							return;
						} else {
							AviStreamInfo.fccType = streamtypeVIDEO;//CompVars.fccType;
							AviStreamInfo.fccHandler = CompVars.fccHandler;
							AviStreamInfo.dwScale = 1;
							AviStreamInfo.dwRate = NebParams.FramesPerKey;//FPS
							//AviStreamInfo.dwLength = AviStreamInfo.dwScale * AviStreamInfo.dwRate * NebParams.KeyFrames * NebParams.FramesPerKey;
							AviStreamInfo.dwLength = NebParams.KeyFrames * NebParams.FramesPerKey;
							AviStreamInfo.dwQuality = CompVars.lQ;
							AviStreamInfo.rcFrame.right = bmInfo.bmiHeader.biWidth;
							AviStreamInfo.rcFrame.bottom = bmInfo.bmiHeader.biHeight;
							strcpy(AviStreamInfo.szName, "Generated using the Sun Project Interactive");
							if (AVIFileCreateStream(AviFile, &AviStream, &AviStreamInfo)) {
								MessageBox(NULL, "Error creating the video stream!", "Stream creation error", MB_ICONEXCLAMATION);
								AVIFileRelease(AviFile);
								ICCompressorFree(&CompVars);
								AVIFileExit(); //Kill the AVI library
								bDrawing = false;
								return;
							} else {
								if (AVIStreamSetFormat(AviStream, 0, (CompVars.hic ? CompVars.lpbiOut : &bmInfo), (CompVars.hic ? sizeof(*CompVars.lpbiOut) : sizeof(bmInfo)))) {
									MessageBox(NULL, "Error setting the stream format!", "Stream creation error", MB_ICONEXCLAMATION);
									AVIStreamRelease(AviStream);
									AVIFileRelease(AviFile);
									ICCompressorFree(&CompVars);
									AVIFileExit(); //Kill the AVI library
									bDrawing = false;
									return;
								} else {
									AnimInfo.DrawingAnimation = TRUE;
									AnimInfo.TotalFrames = NebParams.KeyFrames * NebParams.FramesPerKey;
									if (CompVars.hic) {
										if (ICSeqCompressFrameStart(&CompVars, &bmInfo)) {
											FillPerlinGradientTemporal(SPBitmap, (NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0) | (!NebParams.LayerParams[i].Interpolation ? 32 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 64 : 0) | (NebParams.Loop ? 256 : 0) | (NebParams.SmoothT ? 512 : 0), NebParams.LayerParams[i].MiddleColour, NumGradEntries, GradientArray, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave, MinDist, NebParams.BaseFileName, NebParams.KeyFrames, NebParams.FramesPerKey, ANIMPERLINFORMATAVI, &CompVars, &AviStream);
											ICSeqCompressFrameEnd(&CompVars);
										} else {
											MessageBox(NULL, "Error initialising the compressor", "Compression error", MB_ICONEXCLAMATION);
										}
										AVIStreamRelease(AviStream);
										AVIFileRelease(AviFile);
										ICCompressorFree(&CompVars);
									} else {
										FillPerlinGradientTemporal(SPBitmap, (NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0) | (!NebParams.LayerParams[i].Interpolation ? 32 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 64 : 0) | (NebParams.Loop ? 256 : 0) | (NebParams.SmoothT ? 512 : 0), NebParams.LayerParams[i].MiddleColour, NumGradEntries, GradientArray, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave, MinDist, NebParams.BaseFileName, NebParams.KeyFrames, NebParams.FramesPerKey, ANIMPERLINFORMATAVI, &CompVars, &AviStream);
										AVIStreamRelease(AviStream);
										AVIFileRelease(AviFile);
										ICCompressorFree(&CompVars);
									}
									AnimInfo.DrawingAnimation = FALSE;
								}
							}
						}
					} else {
						AVIFileExit(); //Kill the AVI library
						bDrawing = false;
						return;
					}
					AVIFileExit(); //Kill the AVI library
				} else if (!_stricmp(tmpFileType, ".bmp")) {
					NebParams.BaseFileName[strlen(NebParams.BaseFileName) - 4] = '\0';
					AnimInfo.DrawingAnimation = TRUE;
					AnimInfo.TotalFrames = NebParams.KeyFrames * NebParams.FramesPerKey;
					FillPerlinGradientTemporal(SPBitmap, (NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0) | (!NebParams.LayerParams[i].Interpolation ? 32 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 64 : 0) | (NebParams.Loop ? 256 : 0) | (NebParams.SmoothT ? 512 : 0), NebParams.LayerParams[i].MiddleColour, NumGradEntries, GradientArray, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave, MinDist, NebParams.BaseFileName, NebParams.KeyFrames, NebParams.FramesPerKey);
					AnimInfo.DrawingAnimation = FALSE;
				} else {
					MessageBox(NULL, "Unsupported Format, please save as either AVI or BMP", "Unknown Format", MB_ICONEXCLAMATION);
				}
			} else {
				FillPerlinGradient(SPBitmap, (NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0) | (!NebParams.LayerParams[i].Interpolation ? 32 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 64 : 0), NebParams.LayerParams[i].MiddleColour, NumGradEntries, GradientArray, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave, MinDist);
			}
			if (GradientArray) delete [] GradientArray; GradientArray = NULL;
			//FillPerlinGradient(SPBitmap, (NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0) | (!NebParams.LayerParams[i].Interpolation ? 32 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 64 : 0), NebParams.LayerParams[i].MiddleColour, NebParams.LayerParams[i].NumGradientEntries, NebParams.LayerParams[i].GradientArray, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave, MinDist);
		} else {
			if (NebParams.Animate) {
				char tmpFileType[5];
				strcpy(tmpFileType, &NebParams.BaseFileName[strlen(NebParams.BaseFileName) - 4]);
				if (!_stricmp(tmpFileType, ".avi")) {
					//AVI Test using VCM:
					COMPVARS CompVars = {0};
					CompVars.cbSize = sizeof(COMPVARS);
					BITMAPINFO bmInfo = {0};
					bmInfo.bmiHeader.biWidth = SPBitmap.Width();
					bmInfo.bmiHeader.biBitCount = SPBitmap.BPP();
					bmInfo.bmiHeader.biHeight = SPBitmap.Height();
					bmInfo.bmiHeader.biCompression = BI_RGB;
					bmInfo.bmiHeader.biPlanes = 1;
					bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);
					bmInfo.bmiHeader.biSizeImage = SPBitmap.UBound();
					AVIFileInit(); //Initialise the AVI library
					PAVIFILE AviFile = {0};
					PAVISTREAM AviStream = {0};
					AVISTREAMINFO AviStreamInfo = {0};
					if (ICCompressorChoose(NULL, ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME, &bmInfo, NULL, &CompVars, NULL)) {
						if (AVIFileOpen(&AviFile, NebParams.BaseFileName, OF_CREATE | OF_WRITE, NULL)) {
							MessageBox(NULL, "Error creating the AVI file!", "File creation error!", MB_ICONEXCLAMATION);
							ICCompressorFree(&CompVars);
							AVIFileExit(); //Kill the AVI library
							bDrawing = false;
							return;
						} else {
							AviStreamInfo.fccType = streamtypeVIDEO;//CompVars.fccType;
							AviStreamInfo.fccHandler = CompVars.fccHandler;
							AviStreamInfo.dwScale = 1;
							AviStreamInfo.dwRate = NebParams.FramesPerKey;//FPS
							//AviStreamInfo.dwLength = AviStreamInfo.dwScale * AviStreamInfo.dwRate * NebParams.KeyFrames * NebParams.FramesPerKey;
							AviStreamInfo.dwLength = NebParams.KeyFrames * NebParams.FramesPerKey;
							AviStreamInfo.dwQuality = CompVars.lQ;
							AviStreamInfo.rcFrame.right = bmInfo.bmiHeader.biWidth;
							AviStreamInfo.rcFrame.bottom = bmInfo.bmiHeader.biHeight;
							strcpy(AviStreamInfo.szName, "Generated using the Sun Project Interactive");
							if (AVIFileCreateStream(AviFile, &AviStream, &AviStreamInfo)) {
								MessageBox(NULL, "Error creating the video stream!", "Stream creation error", MB_ICONEXCLAMATION);
								AVIFileRelease(AviFile);
								ICCompressorFree(&CompVars);
								AVIFileExit(); //Kill the AVI library
								bDrawing = false;
								return;
							} else {
								if (AVIStreamSetFormat(AviStream, 0, (CompVars.hic ? CompVars.lpbiOut : &bmInfo), (CompVars.hic ? sizeof(*CompVars.lpbiOut) : sizeof(bmInfo)))) {
									MessageBox(NULL, "Error setting the stream format!", "Stream creation error", MB_ICONEXCLAMATION);
									AVIStreamRelease(AviStream);
									AVIFileRelease(AviFile);
									ICCompressorFree(&CompVars);
									AVIFileExit(); //Kill the AVI library
									bDrawing = false;
									return;
								} else {
									AnimInfo.DrawingAnimation = TRUE;
									AnimInfo.TotalFrames = NebParams.KeyFrames * NebParams.FramesPerKey;
									if (CompVars.hic) {
										if (ICSeqCompressFrameStart(&CompVars, &bmInfo)) {
											FillPerlinTemporal(SPBitmap, (NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0) | (!NebParams.LayerParams[i].Interpolation ? 32 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 64 : 0) | (NebParams.Loop ? 256 : 0) | (NebParams.SmoothT ? 512 : 0), NebParams.LayerParams[i].MiddleColour, NebParams.LayerParams[i].RedVariance, NebParams.LayerParams[i].GreenVariance, NebParams.LayerParams[i].BlueVariance, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave, MinDist, NebParams.BaseFileName, NebParams.KeyFrames, NebParams.FramesPerKey, ANIMPERLINFORMATAVI, &CompVars, &AviStream);
											ICSeqCompressFrameEnd(&CompVars);
										} else {
											MessageBox(NULL, "Error initialising the compressor", "Compression error", MB_ICONEXCLAMATION);
										}
										AVIStreamRelease(AviStream);
										AVIFileRelease(AviFile);
										ICCompressorFree(&CompVars);
									} else {
										FillPerlinTemporal(SPBitmap, (NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0) | (!NebParams.LayerParams[i].Interpolation ? 32 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 64 : 0) | (NebParams.Loop ? 256 : 0) | (NebParams.SmoothT ? 512 : 0), NebParams.LayerParams[i].MiddleColour, NebParams.LayerParams[i].RedVariance, NebParams.LayerParams[i].GreenVariance, NebParams.LayerParams[i].BlueVariance, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave, MinDist, NebParams.BaseFileName, NebParams.KeyFrames, NebParams.FramesPerKey, ANIMPERLINFORMATAVI, &CompVars, &AviStream);
										AVIStreamRelease(AviStream);
										AVIFileRelease(AviFile);
										ICCompressorFree(&CompVars);
									}
									AnimInfo.DrawingAnimation = FALSE;
								}
							}
						}
					} else {
						AVIFileExit(); //Kill the AVI library
						bDrawing = false;
						return;
					}
					AVIFileExit(); //Kill the AVI library
				} else if (!_stricmp(tmpFileType, ".bmp")) {
					NebParams.BaseFileName[strlen(NebParams.BaseFileName) - 4] = '\0';
					AnimInfo.DrawingAnimation = TRUE;
					AnimInfo.TotalFrames = NebParams.KeyFrames * NebParams.FramesPerKey;
					FillPerlinTemporal(SPBitmap, (NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0) | (!NebParams.LayerParams[i].Interpolation ? 32 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 64 : 0) | (NebParams.Loop ? 256 : 0) | (NebParams.SmoothT ? 512 : 0), NebParams.LayerParams[i].MiddleColour, NebParams.LayerParams[i].RedVariance, NebParams.LayerParams[i].GreenVariance, NebParams.LayerParams[i].BlueVariance, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave, MinDist, NebParams.BaseFileName, NebParams.KeyFrames, NebParams.FramesPerKey);
					AnimInfo.DrawingAnimation = FALSE;
				} else {
					MessageBox(NULL, "Unsupported Format, please save as either AVI or BMP", "Unknown Format", MB_ICONEXCLAMATION);
				}
			} else {
				//FillPerlin((NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0), NebParams.LayerParams[i].FirstOctave, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, SPBitmap, GetRValue(NebParams.LayerParams[i].MiddleColour), NebParams.LayerParams[i].RedVariance, GetGValue(NebParams.LayerParams[i].MiddleColour), NebParams.LayerParams[i].GreenVariance, GetBValue(NebParams.LayerParams[i].MiddleColour), NebParams.LayerParams[i].BlueVariance, MinDist);
				FillPerlinAdvanced(SPBitmap, (NebParams.LayerParams[i].Bug ? 2 : 0) | (NebParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebParams.LayerParams[i].HTile ? 4 : 0) | (NebParams.LayerParams[i].VTile ? 8 : 0) | (!NebParams.LayerParams[i].Interpolation ? 32 : 0) | (NebParams.LayerParams[i].Interpolation == 2 ? 64 : 0), NebParams.LayerParams[i].MiddleColour, NebParams.LayerParams[i].RedVariance, NebParams.LayerParams[i].GreenVariance, NebParams.LayerParams[i].BlueVariance, NebParams.LayerParams[i].Persistance, NebParams.LayerParams[i].Octaves, NebParams.LayerParams[i].FirstOctave, MinDist);
			}
		}
		//}
		//If the previous image is to be underlayed, apply my underlay formula:
		if (NebParams.LayerParams[i].KeepPrevious == 1 && bRunning) {
			for (DWORD i = 0; i < SPBitmap.UBound(); i += 3) {
				LONG32 Red = SPUnderlayBuffer.PixelData[i + 2];
				LONG32 Green = SPUnderlayBuffer.PixelData[i + 1];
				LONG32 Blue = SPUnderlayBuffer.PixelData[i];
				LONG32 TempRed = SPBitmap.PixelData[i + 2];
				LONG32 TempGreen = SPBitmap.PixelData[i + 1];
				LONG32 TempBlue = SPBitmap.PixelData[i];
				LONG32 TempAvg = (TempRed + TempGreen + TempBlue) / 3;
				if (Red - TempAvg > TempRed) TempRed = Red - TempAvg;
				if (Green - TempAvg > TempGreen) TempGreen = Green - TempAvg;
				if (Blue - TempAvg > TempBlue) TempBlue = Blue - TempAvg;
				if (TempRed > 255) TempRed = 255;
				if (TempGreen > 255) TempGreen = 255;
				if (TempBlue > 255) TempBlue = 255;
				SPBitmap.PixelData[i] = (uchar)TempBlue;
				SPBitmap.PixelData[i + 1] = (uchar)TempGreen;
				SPBitmap.PixelData[i + 2] = (uchar)TempRed;
			}
		}
		if (!bRunning) break;
	}
	//FillPerlin(1, 1, (double)rand() / (double)RAND_MAX * 0.45 + 0.5, 0, SPBitmap, 0, rand() % 512, 0, rand() % 512, 0, rand() % 512, MinDist);
	/*FillPerlin(1, 1, 0.75, 0, SPBitmap, 0, 64, 128, 256, 128, 512, MinDist);
	FillPerlin(0, 1, 0.75, 0, SPBitmap, 0, 64, 128, 512, 64, 128, MinDist);*/
	/*if (bRunning) FillPerlin(1, 1, 0.75, 0, SPBitmap, 128, 512, 64, 256, 0, 128, MinDist);
	if (bRunning) FillPerlin(0, 1, 0.75, 0, SPBitmap, 0, 128, 64, 256, 128, 512, MinDist);*/
	bDrawing = false;
}



/*
This function draws a 'basic' star (without a surface
texture or flares) on the given BitmapOps at the given
coordinates with the given 'fade-out' corona type.


NOTE: the fade-out type is still hard-coded!!!
(I think it's the best looking anyway, but...)
*/
void DrawBasicStar(BitmapOps &SPBitmap, LONG32 XPos, LONG32 YPos, DWORD StarRadius, DWORD CoronaRadius, COLORREF StarColour) {
	if (bDrawing || !bRunning) return;
	bDrawing = true;

	LONG32 Width = SPBitmap.Width();
	LONG32 Height = SPBitmap.Height();
	LONG32 TempIndex = 0;

	LONG32 TempX = 0;
	LONG32 TempY = 0;
	LONG32 XMin = 0;
	LONG32 YMin = 0;
	LONG32 XMax = 0;
	LONG32 YMax = 0;
	LONG32 XRealMin = 0;
	LONG32 YRealMin = 0;
	LONG32 XRealMax = 0;
	LONG32 YRealMax = 0;
	
	LONG32 TempRed = 0;
	LONG32 TempGreen = 0;
	LONG32 TempBlue = 0;

//	LONG32 MaxSEven = 0;
//	LONG32 TempEven = 0;
	double TempDist = 0;
	double TempCDist = 0;

	BOOL Alpha = FALSE;
	if (SPBitmap.BPP() == 32) Alpha = TRUE;

	BYTE StarRed = GetRValue(StarColour);
	BYTE StarGreen = GetGValue(StarColour);
	BYTE StarBlue = GetBValue(StarColour);
	LONG32 FullStarRadius;

	FullStarRadius = StarRadius + CoronaRadius;
//	MaxSEven = rand() % 64 + 1;
	//BEGIN 1,0,1,0 ADDITIONS + MODIFICATIONS
	XRealMin = XPos - FullStarRadius;
	YRealMin = YPos - FullStarRadius;
	XRealMax = XPos + FullStarRadius + 1;
	YRealMax = YPos + FullStarRadius + 1;
	XMin = ((XRealMin < 0) ? 0 : XRealMin);
	YMin = ((YRealMin < 0) ? 0 : YRealMin);
	XMax = ((XRealMax > Width) ? Width : XRealMax);
	YMax = ((YRealMax > Height) ? Height : YRealMax);
//	FlareBmp.Resize(FullStarRadius * 2, FullStarRadius * 2);
	/*if (rand() % 15 == 0) {
		CompletePerlin = TRUE;
		FillPerlin(1, 1, 1, -2, FlareBmp, StarRed, 128, StarGreen, 128, StarBlue, 128);
	} else if (rand() % 5 == 0) {*/
//		CompletePerlin = FALSE;
//		FillPerlin(1, 1, (double)rand() / (double)RAND_MAX * 0.2 + 0.75, -2, FlareBmp, StarRed, 128, StarGreen, 128, StarBlue, 128, StarRadius);
	/*} else {
		CompletePerlin = FALSE;
		FillPerlin(1, 1, 0, 1, FlareBmp, StarRed, 0, StarGreen, 0, StarBlue, 0);
	}*/
//	if (!bRunning) { //Chance that the DoEvents in FillPerlin caught the exit, don't want to keep drawing if that happened...
//		if (TempArrayRed) delete [] TempArrayRed;
//		if (TempArrayGreen) delete [] TempArrayGreen;
//		if (TempArrayBlue) delete [] TempArrayBlue;
//		return;
//	}
	//END 1,0,1,0 ADDITIONS + MODIFICATIONS
	//BEGIN OLD 1,0,0,0 CODE
	/*for (TempY = ((YPos - FullStarRadius < 0) ? 0 : YPos - FullStarRadius); TempY < ((YPos + FullStarRadius + 1 > Height) ? Height : YPos + FullStarRadius + 1); ++TempY) {
		for (TempX = ((XPos - FullStarRadius < 0) ? 0 : XPos - FullStarRadius); TempX < ((XPos + FullStarRadius + 1 > Width) ? Width : XPos + FullStarRadius + 1); ++TempX) {*/
	/*XMin = ((XPos - FullStarRadius < 0) ? 0 : XPos - FullStarRadius);
	YMin = ((YPos - FullStarRadius < 0) ? 0 : YPos - FullStarRadius);
	XMax = ((XPos + FullStarRadius + 1 > Width) ? Width : XPos + FullStarRadius + 1);
	YMax = ((YPos + FullStarRadius + 1 > Height) ? Height : YPos + FullStarRadius + 1);*/
	//END OLD 1,0,0,0 CODE
	for (TempY = YMin; TempY < YMax; ++TempY) {
		for (TempX = XMin; TempX < XMax; ++TempX) {
			TempDist = sqrt(pow((signed)TempX - (signed)XPos, 2) + pow((signed)TempY - (signed)YPos, 2));
			TempIndex = SPBitmap.GetPixelIndex(TempX, TempY);
//			FlareIndex = FlareBmp.GetPixelIndex(TempX - XRealMin, TempY - YRealMin);
			if (TempDist < StarRadius) {
//				TempEven = LONG32((rand() % (MaxSEven * 2) - MaxSEven) / double(StarRadius) * double(StarRadius - TempDist));
				//BEGIN OLD 1,0,0,0 CODE
				/*TempRed = StarRed + TempEven;
				TempGreen = StarGreen + TempEven;
				TempBlue = StarBlue + TempEven;*/
				//END OLD 1,0,0,0 CODE
				//BEGIN 1,0,1,0 MODIFICATIONS
//				if (CompletePerlin) {
//					TempRed = FlareBmp.PixelData[FlareIndex + 2];// + TempEven;
//					TempGreen = FlareBmp.PixelData[FlareIndex + 1];// + TempEven;
//					TempBlue = FlareBmp.PixelData[FlareIndex];// + TempEven;
//				} else {
					/*TempRed = LONG32(FlareBmp.PixelData[FlareIndex + 2] / double(StarRadius) * double(TempDist) + StarRed / double(StarRadius) * double(StarRadius - TempDist) + TempEven);
					TempGreen = LONG32(FlareBmp.PixelData[FlareIndex + 1] / double(StarRadius) * double(TempDist) + StarGreen / double(StarRadius) * double(StarRadius - TempDist) + TempEven);
					TempBlue = LONG32(FlareBmp.PixelData[FlareIndex] / double(StarRadius) * double(TempDist) + StarBlue / double(StarRadius) * double(StarRadius - TempDist) + TempEven);*/
					/*TempRed = LONG32((FlareBmp.PixelData[FlareIndex + 2] - StarRed) * 0.65 + StarRed + TempEven);
					TempGreen = LONG32((FlareBmp.PixelData[FlareIndex + 1] - StarGreen) * 0.65 + StarGreen + TempEven);
					TempBlue = LONG32((FlareBmp.PixelData[FlareIndex] - StarBlue) * 0.65 + StarBlue + TempEven);*/
					//Good:
					/*TempRed = LONG32(((FlareBmp.PixelData[FlareIndex + 2] - StarRed) * 0.65 + StarRed) / double(StarRadius) * double(TempDist) + StarRed / double(StarRadius) * double(StarRadius - TempDist) + TempEven);
					TempGreen = LONG32(((FlareBmp.PixelData[FlareIndex + 1] - StarGreen) * 0.65 + StarGreen) / double(StarRadius) * double(TempDist) + StarGreen / double(StarRadius) * double(StarRadius - TempDist) + TempEven);
					TempBlue = LONG32(((FlareBmp.PixelData[FlareIndex] - StarBlue) * 0.65 + StarBlue) / double(StarRadius) * double(TempDist) + StarBlue / double(StarRadius) * double(StarRadius - TempDist) + TempEven);*/
//					TempRed = StarRed + TempEven;
//					TempGreen = StarGreen + TempEven;
//					TempBlue = StarBlue + TempEven;
//				}
				//END 1,0,1,0 MODIFICATIONS
//				if (TempRed > 255) TempRed = 255;
//				else if (TempRed < 0) TempRed = 0;
//				if (TempGreen > 255) TempGreen = 255;
//				else if (TempGreen < 0) TempGreen = 0;
//				if (TempBlue > 255) TempBlue = 255;
//				else if (TempBlue < 0) TempBlue = 0;
				//SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue));
				if (Alpha) SPBitmap.SetPColour(TempX, TempY, uchar(StarRed), uchar(StarGreen), uchar(StarBlue), 0);
				else SPBitmap.SetPColour(TempX, TempY, uchar(StarRed), uchar(StarGreen), uchar(StarBlue));
			} else if (TempDist < FullStarRadius) {
				TempCDist = (CoronaRadius - (TempDist - StarRadius)) / (double)CoronaRadius * 255;
				//BEGIN OLD 1,0,0,0 CODE
				/*TempRed = LONG32(SPBitmap.PixelData[TempIndex + 2] / (double)CoronaRadius * (TempDist - StarRadius) + StarRed / 255.0 * TempCDist);
				TempGreen = LONG32(SPBitmap.PixelData[TempIndex + 1] / (double)CoronaRadius * (TempDist - StarRadius) + StarGreen / 255.0 * TempCDist);
				TempBlue = LONG32(SPBitmap.PixelData[TempIndex] / (double)CoronaRadius * (TempDist - StarRadius) + StarBlue / 255.0 * TempCDist);*/
				//END OLD 1,0,0,0 CODE
				//BEGIN 1,0,1,0 MODIFICATIONS
				//Best!!!:
//				TempRed = LONG32(SPBitmap.PixelData[TempIndex + 2] / (double)CoronaRadius * (TempDist - StarRadius) / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex + 2] / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius) + StarRed / 255.0 * TempCDist - StarRed / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
//				TempGreen = LONG32(SPBitmap.PixelData[TempIndex + 1] / (double)CoronaRadius * (TempDist - StarRadius) / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex + 1] / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius) + StarGreen / 255.0 * TempCDist - StarGreen / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
//				TempBlue = LONG32(SPBitmap.PixelData[TempIndex] / (double)CoronaRadius * (TempDist - StarRadius) / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex] / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius) + StarBlue / 255.0 * TempCDist - StarBlue / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
////////////////////////////////////////////////
				TempRed = LONG32(SPBitmap.PixelData[TempIndex + 2] / (double)CoronaRadius * (TempDist - StarRadius) / (double)CoronaRadius * (TempDist - StarRadius) + StarRed / 255.0 * TempCDist);// - StarRed / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
				TempGreen = LONG32(SPBitmap.PixelData[TempIndex + 1] / (double)CoronaRadius * (TempDist - StarRadius) / (double)CoronaRadius * (TempDist - StarRadius) + StarGreen / 255.0 * TempCDist);// - StarGreen / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
				TempBlue = LONG32(SPBitmap.PixelData[TempIndex] / (double)CoronaRadius * (TempDist - StarRadius) / (double)CoronaRadius * (TempDist - StarRadius) + StarBlue / 255.0 * TempCDist);// - StarBlue / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
//////////////////////////////////////////////
				//Good:
				/*TempRed = LONG32(SPBitmap.PixelData[TempIndex + 2] / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex + 2] * 0.95 / 255.0 * TempCDist);
				TempGreen = LONG32(SPBitmap.PixelData[TempIndex + 1] / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex + 1] * 0.95 / 255.0 * TempCDist);
				TempBlue = LONG32(SPBitmap.PixelData[TempIndex] / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex] * 0.95 / 255.0 * TempCDist);*/
				//END 1,0,1,0 MODIFICATIONS
				if (TempRed > 255) TempRed = 255;
				//else if (TempRed < 0) TempRed = 0; //Required for some experimental formula variants
				if (TempGreen > 255) TempGreen = 255;
				//else if (TempGreen < 0) TempGreen = 0; //Required for some experimental formula variants
				if (TempBlue > 255) TempBlue = 255; 
				//else if (TempBlue < 0) TempBlue = 0; //Required for some experimental formula variants
				if (Alpha) SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue), uchar(255 / (double)CoronaRadius * (TempDist - StarRadius) / (double)CoronaRadius * (TempDist - StarRadius)));
				else SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue));
			} else if (Alpha) SPBitmap.SetPColour(TempX, TempY, 0, 0, 0, 255);
		}
		if (!DoEvents(NULL)) {
//			if (TempArrayRed) delete [] TempArrayRed;
//			if (TempArrayGreen) delete [] TempArrayGreen;
//			if (TempArrayBlue) delete [] TempArrayBlue;
			return;
		}
	}
	bDrawing = false;
}




/*
This function draws a noise star surface on the given BitmapOps
at the given coordinates with the given variance to give it a
3d look. It can optionally erase and redraw the star 'face'.
It also can optionally smooth the result.
*/
void DrawStarSurfaceNoise(BitmapOps &SPBitmap, LONG32 XPos, LONG32 YPos, DWORD StarRadius, LONG32 Variance, BOOL Smooth, DWORD Seed, BOOL Erase, COLORREF StarColour) {
	if (bDrawing || !bRunning) return;
	bDrawing = true;

	LONG32 Width = SPBitmap.Width();
	LONG32 Height = SPBitmap.Height();
	LONG32 TempIndex = 0;

	LONG32 TempX = 0;
	LONG32 TempY = 0;
	LONG32 XMin = 0;
	LONG32 YMin = 0;
	LONG32 XMax = 0;
	LONG32 YMax = 0;
	LONG32 XRealMin = 0;
	LONG32 YRealMin = 0;
	LONG32 XRealMax = 0;
	LONG32 YRealMax = 0;
	
	LONG32 TempRed = 0;
	LONG32 TempGreen = 0;
	LONG32 TempBlue = 0;

	LONG32 MaxSEven = 0;
	LONG32 TempEven = 0;
	double TempDist = 0;

	BYTE StarRed = GetRValue(StarColour);
	BYTE StarGreen = GetGValue(StarColour);
	BYTE StarBlue = GetBValue(StarColour);


	MaxSEven = Variance;
	if (!MaxSEven) MaxSEven++;
	if (Seed) srand(Seed); else srand(GetTickCount()); rand();
	
	XRealMin = XPos - StarRadius;
	YRealMin = YPos - StarRadius;
	XRealMax = XPos + StarRadius + 1;
	YRealMax = YPos + StarRadius + 1;
	XMin = ((XRealMin < 0) ? 0 : XRealMin);
	YMin = ((YRealMin < 0) ? 0 : YRealMin);
	XMax = ((XRealMax > Width) ? Width : XRealMax);
	YMax = ((YRealMax > Height) ? Height : YRealMax);
	for (TempY = YMin; TempY < YMax; ++TempY) {
		for (TempX = XMin; TempX < XMax; ++TempX) {
			TempDist = sqrt(pow((signed)TempX - (signed)XPos, 2) + pow((signed)TempY - (signed)YPos, 2));
			TempIndex = SPBitmap.GetPixelIndex(TempX, TempY);
			if (TempDist <= StarRadius) {
				TempEven = LONG32((rand() % (MaxSEven * 2) - MaxSEven) / double(StarRadius) * double(StarRadius - TempDist));
				if (Erase) {
					TempRed = StarRed + TempEven;
					TempGreen = StarGreen + TempEven;
					TempBlue = StarBlue + TempEven;
				} else {
					TempRed = SPBitmap.PixelData[TempIndex + 2] + TempEven;
					TempGreen = SPBitmap.PixelData[TempIndex + 1] + TempEven;
					TempBlue = SPBitmap.PixelData[TempIndex] + TempEven;
				}
				if (TempRed > 255) TempRed = 255;
				else if (TempRed < 0) TempRed = 0;
				if (TempGreen > 255) TempGreen = 255;
				else if (TempGreen < 0) TempGreen = 0;
				if (TempBlue > 255) TempBlue = 255;
				else if (TempBlue < 0) TempBlue = 0;
				SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue));
			}
		}
		if (!DoEvents(NULL)) return;
	}
	if (Smooth) SmoothBitmapOps(SPBitmap, TRUE, XPos, YPos, 0, StarRadius);
	bDrawing = false;
}




/*
This function adds perlin noise to the given star on the
given BitmapOps at the given coordinates and radius.
*/
void DrawFlaresPerlinNoise(BitmapOps &SPBitmap, SunProjectNebulaParams &FlrParams, LONG32 XPos, LONG32 YPos, DWORD StarRadius, DWORD CoronaRadius, COLORREF StarColour, DWORD Seed, BOOL Erase, BOOL SubtractExistingCorona) {
	if (bDrawing || !bRunning) return;
	bDrawing = true;

	LONG32 Width = SPBitmap.Width();
	LONG32 Height = SPBitmap.Height();
	LONG32 TempIndex = 0;

	LONG32 TempX = 0;
	LONG32 TempY = 0;
	LONG32 XMin = 0;
	LONG32 YMin = 0;
	LONG32 XMax = 0;
	LONG32 YMax = 0;
	LONG32 XRealMin = 0;
	LONG32 YRealMin = 0;
	LONG32 XRealMax = 0;
	LONG32 YRealMax = 0;
	
	LONG32 TempRed = 0;
	LONG32 TempGreen = 0;
	LONG32 TempBlue = 0;

	LONG32 MaxSEven = 0;
	LONG32 TempEven = 0;
	double TempDist = 0;
	double TempCDist = 0;

	BYTE StarRed = GetRValue(StarColour);
	BYTE StarGreen = GetGValue(StarColour);
	BYTE StarBlue = GetBValue(StarColour);
	LONG32 FullStarRadius;

	FullStarRadius = StarRadius + CoronaRadius;

	BitmapOps FlareBmp(2 * FullStarRadius, 2 * FullStarRadius);
	LONG32 FlareIndex = 0;

	if (Seed) srand(Seed); else srand(GetTickCount()); rand();

	XRealMin = XPos - FullStarRadius;
	YRealMin = YPos - FullStarRadius;
	XRealMax = XPos + FullStarRadius + 1;
	YRealMax = YPos + FullStarRadius + 1;
	XMin = ((XRealMin < 0) ? 0 : XRealMin);
	YMin = ((YRealMin < 0) ? 0 : YRealMin);
	XMax = ((XRealMax > Width) ? Width : XRealMax);
	YMax = ((YRealMax > Height) ? Height : YRealMax);

	FlareBmp.Resize(FullStarRadius * 2, FullStarRadius * 2);
	//FillPerlinAdvanced(FlareBmp, 65, StarColour, 128, 128, 128, (double)rand() / (double)RAND_MAX * 0.2 + 0.75, -2, 1, StarRadius);
	bDrawing = false;DrawNebula(FlareBmp, FlrParams, StarRadius);bDrawing = true;

	if (!bRunning) return;
	for (TempY = YMin; TempY < YMax; ++TempY) {
		for (TempX = XMin; TempX < XMax; ++TempX) {
			TempDist = sqrt(pow((signed)TempX - (signed)XPos, 2) + pow((signed)TempY - (signed)YPos, 2));
			TempIndex = SPBitmap.GetPixelIndex(TempX, TempY);
			FlareIndex = FlareBmp.GetPixelIndex(TempX - XRealMin, TempY - YRealMin);
			if (TempDist < FullStarRadius && TempDist >= StarRadius) {
				TempCDist = (CoronaRadius - (TempDist - StarRadius)) / (double)CoronaRadius * 255;
				//All in one calculation:
//				TempRed = LONG32(SPBitmap.PixelData[TempIndex + 2] / (double)CoronaRadius * (TempDist - StarRadius) / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex + 2] / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius) + StarRed / 255.0 * TempCDist - StarRed / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
//				TempGreen = LONG32(SPBitmap.PixelData[TempIndex + 1] / (double)CoronaRadius * (TempDist - StarRadius) / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex + 1] / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius) + StarGreen / 255.0 * TempCDist - StarGreen / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
//				TempBlue = LONG32(SPBitmap.PixelData[TempIndex] / (double)CoronaRadius * (TempDist - StarRadius) / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex] / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius) + StarBlue / 255.0 * TempCDist - StarBlue / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
				if (Erase) {
					TempRed = LONG32(StarRed / 255.0 * TempCDist - StarRed / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
					TempGreen = LONG32(StarGreen / 255.0 * TempCDist - StarGreen / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
					TempBlue = LONG32(StarBlue / 255.0 * TempCDist - StarBlue / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
				} else {
					if (SubtractExistingCorona) {
						TempRed = LONG32(SPBitmap.PixelData[TempIndex + 2] - StarRed / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
						TempGreen = LONG32(SPBitmap.PixelData[TempIndex + 1] - StarGreen / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
						TempBlue = LONG32(SPBitmap.PixelData[TempIndex] - StarBlue / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
					} else {
						TempRed = SPBitmap.PixelData[TempIndex + 2];
						TempGreen = SPBitmap.PixelData[TempIndex + 1];
						TempBlue = SPBitmap.PixelData[TempIndex];
					}
				}
				TempRed = LONG32(TempRed + FlareBmp.PixelData[FlareIndex + 2] / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
				TempGreen = LONG32(TempGreen + FlareBmp.PixelData[FlareIndex + 1] / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
				TempBlue = LONG32(TempBlue + FlareBmp.PixelData[FlareIndex] / 255.0 * TempCDist / (double)CoronaRadius * (TempDist - StarRadius));
				//Good:
				/*TempRed = LONG32(SPBitmap.PixelData[TempIndex + 2] / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex + 2] * 0.95 / 255.0 * TempCDist);
				TempGreen = LONG32(SPBitmap.PixelData[TempIndex + 1] / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex + 1] * 0.95 / 255.0 * TempCDist);
				TempBlue = LONG32(SPBitmap.PixelData[TempIndex] / (double)CoronaRadius * (TempDist - StarRadius) + FlareBmp.PixelData[FlareIndex] * 0.95 / 255.0 * TempCDist);*/
				if (TempRed > 255) TempRed = 255;
				else if (TempRed < 0) TempRed = 0; //Required for some experimental formula variants
				if (TempGreen > 255) TempGreen = 255;
				else if (TempGreen < 0) TempGreen = 0; //Required for some experimental formula variants
				if (TempBlue > 255) TempBlue = 255; 
				else if (TempBlue < 0) TempBlue = 0; //Required for some experimental formula variants
				SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue));
			}
		}
		if (!DoEvents(NULL)) return;
	}

	bDrawing = false;
}




/*
This function underlays one BitmapOps with another and
places the resulting bitmap in another BitmapOps
*/
void UnderlayBitmapOps(BitmapOps &TargetBitmap, BitmapOps &UnderlayBitmap, BitmapOps &OverlayBitmap) {
	if (bDrawing || !bRunning) return;
	bDrawing = true;
	BitmapOps *tmpUnderlayBitmap = &UnderlayBitmap;
	BitmapOps *tmpOverlayBitmap = &OverlayBitmap;
	BOOL Resized1 = FALSE;
	BOOL Resized2 = FALSE;
	if (TargetBitmap.Width() != UnderlayBitmap.Width() || TargetBitmap.Height() != UnderlayBitmap.Height()) {
		tmpUnderlayBitmap= new BitmapOps(TargetBitmap.Width(), TargetBitmap.Height());
		tmpUnderlayBitmap->ResizeBitmapFromBitmapOps(UnderlayBitmap);
		Resized1 = TRUE;
	}
	if (TargetBitmap.Width() != OverlayBitmap.Width() || TargetBitmap.Height() != OverlayBitmap.Height()) {
		tmpOverlayBitmap = new BitmapOps(TargetBitmap.Width(), TargetBitmap.Height());
		tmpOverlayBitmap->ResizeBitmapFromBitmapOps(OverlayBitmap);
		Resized2 = TRUE;
	}
	for (DWORD i = 0; i < TargetBitmap.UBound(); i += 3) {
		LONG32 Red = tmpUnderlayBitmap->PixelData[i + 2];
		LONG32 Green = tmpUnderlayBitmap->PixelData[i + 1];
		LONG32 Blue = tmpUnderlayBitmap->PixelData[i];
		LONG32 TempRed = tmpOverlayBitmap->PixelData[i + 2];
		LONG32 TempGreen = tmpOverlayBitmap->PixelData[i + 1];
		LONG32 TempBlue = tmpOverlayBitmap->PixelData[i];
		LONG32 TempAvg = (TempRed + TempGreen + TempBlue) / 3;
		if (Red - TempAvg > TempRed) TempRed = Red - TempAvg;
		if (Green - TempAvg > TempGreen) TempGreen = Green - TempAvg;
		if (Blue - TempAvg > TempBlue) TempBlue = Blue - TempAvg;
		if (TempRed > 255) TempRed = 255;
		if (TempGreen > 255) TempGreen = 255;
		if (TempBlue > 255) TempBlue = 255;
		TargetBitmap.PixelData[i] = (uchar)TempBlue;
		TargetBitmap.PixelData[i + 1] = (uchar)TempGreen;
		TargetBitmap.PixelData[i + 2] = (uchar)TempRed;
	}
	if (Resized1 && tmpUnderlayBitmap) delete tmpUnderlayBitmap;
	if (Resized2 && tmpOverlayBitmap) delete tmpOverlayBitmap;
	bDrawing = false;
}

/*
This function merges two bitmaps together and places the result
in another BitmapOps. Amount: Bitmap1 0<->1 Bitmap2
*/
void MergeBitmapOps(BitmapOps &TargetBitmap, BitmapOps &Bitmap1, BitmapOps &Bitmap2, double Amount) {
	if (bDrawing || !bRunning) return;
	bDrawing = true;
	//Ensure the dimensions of each BitmapOps are identical:
	//if (TargetBitmap.Width() == Bitmap1.Width() && Bitmap1.Width() == Bitmap2.Width() && TargetBitmap.Height() == Bitmap1.Height() && Bitmap1.Height() == Bitmap2.Height()) {
	BitmapOps *tmpBitmap1 = &Bitmap1;
	BitmapOps *tmpBitmap2 = &Bitmap2;
	BOOL Resized1 = FALSE;
	BOOL Resized2 = FALSE;
	//if (TargetBitmap.Width() != Bitmap1.Width() || TargetBitmap.Width() != Bitmap2.Width() || TargetBitmap.Height() != Bitmap1.Height() || TargetBitmap.Height() != Bitmap2.Height()) {
	if (TargetBitmap.Width() != Bitmap1.Width() || TargetBitmap.Height() != Bitmap1.Height()) {
		tmpBitmap1 = new BitmapOps(TargetBitmap.Width(), TargetBitmap.Height());
		tmpBitmap1->ResizeBitmapFromBitmapOps(Bitmap1);
		Resized1 = TRUE;
	}
	if (TargetBitmap.Width() != Bitmap2.Width() || TargetBitmap.Height() != Bitmap2.Height()) {
		tmpBitmap2 = new BitmapOps(TargetBitmap.Width(), TargetBitmap.Height());
		tmpBitmap2->ResizeBitmapFromBitmapOps(Bitmap2);
		Resized2 = TRUE;
	}
	//Ensure Amount is between 0 and 1:
	if (Amount < 0 || Amount > 1) {
		MessageBox(NULL, "Error: The merge amount is out of bounds", "Error", MB_ICONEXCLAMATION);
	} else {
		for (DWORD i = 0; i < TargetBitmap.UBound(); i += 3) {
			TargetBitmap.PixelData[i + 2] = (uchar)((1 - Amount) * (double)tmpBitmap1->PixelData[i + 2] + Amount * (double)tmpBitmap2->PixelData[i + 2]);
			TargetBitmap.PixelData[i + 1] = (uchar)((1 - Amount) * (double)tmpBitmap1->PixelData[i + 1] + Amount * (double)tmpBitmap2->PixelData[i + 1]);
			TargetBitmap.PixelData[i] = (uchar)((1 - Amount) * (double)(tmpBitmap1->PixelData[i]) + Amount * (double)(tmpBitmap2->PixelData[i]));
		}
	}
	if (Resized1 && tmpBitmap1) delete tmpBitmap1;
	if (Resized2 && tmpBitmap2) delete tmpBitmap2;
	/*} else {
		MessageBox(NULL, "Error: the bitmaps do not have the same dimensions!", "Unmatching dimensions", MB_ICONEXCLAMATION);
	}*/
	bDrawing = false;
}

/*
This function smooths the contents of a BitmapOps. ***
Still needs fast/accurate, true/fake bi/tri-linear, V/H-tile, etc.
*/
void SmoothBitmapOps(BitmapOps &TargetBitmap, BOOL UseDist, LONG32 XPos, LONG32 YPos, DWORD MinRange, DWORD MaxRange) {
	LONG32 x, y, Width, Height, i, CornerCount, SideCount, TempDist, MinX, MaxX, MinY, MaxY;
	DWORD TempRed, TempGreen, TempBlue;
	DWORD TempCornerR, TempCornerG, TempCornerB;
	DWORD TempSideR, TempSideG, TempSideB;
	BOOL DistInRange = TRUE;
	Width = TargetBitmap.Width();
	Height = TargetBitmap.Height();
	//Create a temporary BitmapOps:
	BitmapOps *tmpBitmap = new BitmapOps(Width, Height);
	//If we have to use min/max dists then calculate a tighter loop:
	if (UseDist) {
		MinX = XPos - MaxRange;
		MinY = YPos - MaxRange;
		MaxX = XPos + MaxRange + 1;
		MaxY = YPos + MaxRange + 1;
		if (MinX < 0) MinX = 0;
		if (MinY < 0) MinY = 0;
		if (MaxX > Width) MaxX = Width;
		if (MaxY > Height) MaxY = Height;
	} else {
		MinY = 0; MinX = 0; MaxX = Width; MaxY = Height;
	}
	//Loop through the array, smoothing as we go:
	for (y = MinY; y < MaxY; y++) {
		for (x = MinX; x < MaxX; x++) {
			if (UseDist) {
				TempDist = sqrt(pow((double)x - (double)XPos, 2) + pow((double)y - (double)YPos, 2));
				if (TempDist >= MinRange && TempDist <= MaxRange)
					DistInRange = TRUE;
				else
					DistInRange = FALSE;
			}
			if (DistInRange) {
				//Reset variables:
				SideCount = 4; CornerCount = 4;
				TempSideR = 0; TempSideG = 0; TempSideB = 0;
				TempCornerR = 0; TempCornerG = 0; TempCornerB = 0;

				i = TargetBitmap.GetPixelIndex(x, y);
				TempRed = TargetBitmap.PixelData[i + 2] / 4;
				TempGreen = TargetBitmap.PixelData[i + 1] / 4;
				TempBlue = TargetBitmap.PixelData[i] / 4;
				if (x > 0) {
					if (y > 0) {
						i = TargetBitmap.GetPixelIndex(x - 1, y - 1);
						TempCornerR += TargetBitmap.PixelData[i + 2];
						TempCornerG += TargetBitmap.PixelData[i + 1];
						TempCornerB += TargetBitmap.PixelData[i];
					} else CornerCount--;
					i = TargetBitmap.GetPixelIndex(x - 1, y);
					TempSideR += TargetBitmap.PixelData[i + 2];
					TempSideG += TargetBitmap.PixelData[i + 1];
					TempSideB += TargetBitmap.PixelData[i];
					if (y < Height - 1) {
						i = TargetBitmap.GetPixelIndex(x - 1, y + 1);
						TempCornerR += TargetBitmap.PixelData[i + 2];
						TempCornerG += TargetBitmap.PixelData[i + 1];
						TempCornerB += TargetBitmap.PixelData[i];
					} else CornerCount--;
				} else {
					SideCount--;
					CornerCount -= 2;
				}
				if (y > 0) {
					i = TargetBitmap.GetPixelIndex(x, y - 1);
					TempSideR += TargetBitmap.PixelData[i + 2];
					TempSideG += TargetBitmap.PixelData[i + 1];
					TempSideB += TargetBitmap.PixelData[i];
				} else SideCount--;
				if (y < Height - 1) {
					i = TargetBitmap.GetPixelIndex(x, y + 1);
					TempSideR += TargetBitmap.PixelData[i + 2];
					TempSideG += TargetBitmap.PixelData[i + 1];
					TempSideB += TargetBitmap.PixelData[i];
				} else SideCount--;
				if (x < Width - 1) {
					if (y > 0) {
						i = TargetBitmap.GetPixelIndex(x + 1, y - 1);
						TempCornerR += TargetBitmap.PixelData[i + 2];
						TempCornerG += TargetBitmap.PixelData[i + 1];
						TempCornerB += TargetBitmap.PixelData[i];
					} else CornerCount--;
					i = TargetBitmap.GetPixelIndex(x + 1, y);
					TempSideR += TargetBitmap.PixelData[i + 2];
					TempSideG += TargetBitmap.PixelData[i + 1];
					TempSideB += TargetBitmap.PixelData[i];
					if (y < Height - 1) {
						i = TargetBitmap.GetPixelIndex(x + 1, y + 1);
						TempCornerR += TargetBitmap.PixelData[i + 2];
						TempCornerG += TargetBitmap.PixelData[i + 1];
						TempCornerB += TargetBitmap.PixelData[i];
					} else CornerCount--;
				} else {
					SideCount--;
					CornerCount -= 2;
				}
				if (SideCount) {
					TempRed += TempSideR / (2 * SideCount);
					TempGreen += TempSideG / (2 * SideCount);
					TempBlue += TempSideB / (2 * SideCount);
					if (CornerCount) {
						TempRed += TempCornerR / (4 * CornerCount);
						TempGreen += TempCornerG / (4 * CornerCount);
						TempBlue += TempCornerB / (4 * CornerCount);
					} else {
						TempRed *= 4.0/3.0;
						TempGreen *= 4.0/3.0;
						TempBlue *= 4.0/3.0;
					}
				} else {
					TempRed *= 4;
					TempGreen *= 4;
					TempBlue *= 4;
				}
				tmpBitmap->SetPColour(x, y, (uchar)(TempRed), (uchar)(TempGreen), (uchar)(TempBlue));
			}
		}
	}

	if (UseDist) {
		//Loop through, copying only what was changed:
		for (y = MinY; y < MaxY; y++) {
			for (x = MinX; x < MaxX; x++) {
				TempDist = sqrt(pow((double)x - (double)XPos, 2) + pow((double)y - (double)YPos, 2));
				if (TempDist >= MinRange && TempDist <= MaxRange)
					DistInRange = TRUE;
				else
					DistInRange = FALSE;
				if (DistInRange) {
					i = tmpBitmap->GetPixelIndex(x, y);
					TargetBitmap.SetPColour(x, y, tmpBitmap->PixelData[i + 2], tmpBitmap->PixelData[i + 1], tmpBitmap->PixelData[i]);
				}
			}
		}
	} else {
		//Copy the temp BitmapOps into the real one:
		TargetBitmap.CopyBitmapFromBitmapOps(*tmpBitmap);
	}
	
	if (tmpBitmap) delete tmpBitmap;
}


/*
This function draws a random starfield ***
*/
void DrawStarField(BitmapOps &SPBitmap, WORD Type) {
	if (bDrawing || !bRunning) return;
	bDrawing = true;
	srand(GetTickCount());rand(); //Seed the random number generator
	LONG32 StarFieldDensity = rand() % 7500 + 2500; //Between 2500 and 10000
	LONG32 StarFieldBrightness = 0; //This is the base starfield brightness
	LONG32 StarFieldBrightnessVar = 230; //This is the TOTAL maximum brightness variation (+ | -)
	LONG32 TempBrightness = 0;
	LONG32 TempAvg = 0;
	LONG32 TempX = 0;
	LONG32 TempY = 0;
	LONG32 Width = SPBitmap.Width();
	LONG32 Height = SPBitmap.Height();
	LONG32 TempIndex = 0;
	LONG32 TempRed = 0;
	LONG32 TempGreen = 0;
	LONG32 TempBlue = 0;
	for (LONG32 TempCounter = 0; TempCounter <= StarFieldDensity; ++TempCounter) {
		TempX = rand() % Width;
		TempY = rand() % Height;
		TempBrightness = StarFieldBrightness + StarFieldBrightnessVar / 2 - rand() % StarFieldBrightnessVar;
		//BEGIN 1,0,1,0 Additions
		if (rand() % 1000 == 0)
			TempBrightness += StarFieldBrightnessVar / 2;
		//END 1,0,1,0 Additions
		if (TempBrightness < 0) TempBrightness = 0;
		TempIndex = SPBitmap.GetPixelIndex(TempX, TempY);
		/*
		BEGIN OLD 1,0,0,0 CODE
		TempRed = SPBitmap.PixelData[TempIndex + 2] + TempBrightness;
		TempGreen = SPBitmap.PixelData[TempIndex + 1] + TempBrightness;
		TempBlue = SPBitmap.PixelData[TempIndex] + TempBrightness;
		END OLD 1,0,0,0 CODE
		*/
		//BEGIN 1,0,1,0 Modifications
		/*
		Ok, the TempBrightness is going to be compared against the RGB average.
		If it is brighter, add the RGB differences
		*/
		TempRed = SPBitmap.PixelData[TempIndex + 2];
		TempGreen = SPBitmap.PixelData[TempIndex + 1];
		TempBlue = SPBitmap.PixelData[TempIndex];
		TempAvg = (TempRed + TempGreen + TempBlue) / 3;
		if (TempBrightness > TempAvg) {
			/*if (TempBrightness > TempRed) TempRed = TempBrightness;
			if (TempBrightness > TempGreen) TempGreen = TempBrightness;
			if (TempBrightness > TempBlue) TempBlue = TempBrightness;*/
			if (TempBrightness - TempAvg > TempRed) TempRed = TempBrightness - TempAvg;
			if (TempBrightness - TempAvg > TempGreen) TempGreen = TempBrightness - TempAvg;
			if (TempBrightness - TempAvg > TempBlue) TempBlue = TempBrightness - TempAvg;
			/*TempRed += TempBrightness - TempAvg;
			TempGreen += TempBrightness - TempAvg;
			TempBlue += TempBrightness - TempAvg;*/
			/*if (TempBrightness > TempRed) TempRed += TempBrightness - TempAvg;
			if (TempBrightness > TempGreen) TempGreen += TempBrightness - TempAvg;
			if (TempBrightness > TempBlue) TempBlue += TempBrightness - TempAvg;*/
			/*TempRed |= (TempBrightness - TempAvg);
			TempGreen |= (TempBrightness - TempAvg);
			TempBlue |= (TempBrightness - TempAvg);*/
		}
		//END 1,0,1,0 Modifications
		if (TempRed > 255) TempRed = 255;
		if (TempGreen > 255) TempGreen = 255;
		if (TempBlue > 255) TempBlue = 255;
		SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue));
		if (!DoEvents(NULL)) return;
	}
	bDrawing = false;
}



/*
This function will draw a/several random star(s) into the
BitmapOps class passed in, but will not tell it to copy it to the
target hWnd. The code is completely self-contained and has been very
optimised (though, it still can be customised to some extent)
to not waste processor power on completely unused features. It:
1. Clears the screen
2. Draws a random starfield
3. Draws a star w noise & corona
4. Smooths the star
*/
void DrawRandomStarCompact(HWND hWnd, BitmapOps &SPBitmap) {
	if (bDrawing || !bRunning) return;
	bDrawing = true;
	LONG32 *TempArrayRed = 0;
	LONG32 *TempArrayGreen = 0;
	LONG32 *TempArrayBlue = 0;

	try {
		LONG32 Width = SPBitmap.Width();
		LONG32 Height = SPBitmap.Height();
		LONG32 TempIndex = 0;

		LONG32 TempX = 0;
		LONG32 TempY = 0;
		LONG32 XMin = 0;
		LONG32 YMin = 0;
		LONG32 XMax = 0;
		LONG32 YMax = 0;
		LONG32 XRealMin = 0;
		LONG32 YRealMin = 0;
		LONG32 XRealMax = 0;
		LONG32 YRealMax = 0;

		LONG32 TempRed = 0;
		LONG32 TempGreen = 0;
		LONG32 TempBlue = 0;

		TempArrayRed = new LONG32[Width * Height];
		TempArrayGreen = new LONG32[Width * Height];
		TempArrayBlue = new LONG32[Width * Height];


		srand(GetTickCount());rand(); //Seed the random number generator
		
//Step 1 - Clear the screen
		/*BEGIN OLD 1,0,0,0 CODE
		if (rand() % 10 == 0) { //1/10 chance of non-black background
			TempRed = rand() % 64;
			TempGreen = rand() % 64;
			TempBlue = rand() % 64;
		} //else already initialised to zero
		//Fill the array with the colour:
		for (TempY = 0; TempY < Height; ++TempY) {
			for (TempX = 0; TempX < Width; ++TempX) {
				SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue));
			}
			if (!DoEvents(NULL)) return;
		}
		END OLD 1,0,0,0 CODE*/

		//BEGIN 1,0,1,0 Modifications
		if (rand() % 8 == 0) { //1/8 chance of perlin noise background
			FillPerlin(1, 1, (double)rand() / (double)RAND_MAX * 0.45 + 0.5, 0, SPBitmap, 0, rand() % 512, 0, rand() % 512, 0, rand() % 512, -1);
			//FillPerlin(1, 1, (double)rand() / (double)RAND_MAX * 0.45 + 0.5, 0, SPBitmap, rand() % 64, rand() % 384, rand() % 64, rand() % 384, rand() % 64, rand() % 384, 0);
			//FillPerlin(1, 1, (double)rand() / (double)RAND_MAX * 0.5 + 0.5, 8 + rand() % 2, SPBitmap, rand() % 64, rand() % 256, rand() % 64, rand() % 256, rand() % 64, rand() % 256);
			//Use for wood (in combination with different perlin effect above)
			//FillPerlin(3, 1, 0.75, 3, SPBitmap, 128, 128, 64, 128, 0, 0);	
			if (!bRunning) { //Chance that the DoEvents in FillPerlin caught the exit, don't want to keep drawing if that happened...
				if (TempArrayRed) delete [] TempArrayRed;
				if (TempArrayGreen) delete [] TempArrayGreen;
				if (TempArrayBlue) delete [] TempArrayBlue;
				return;
			}
		} else {
			//Fill the array with black:
			for (TempY = 0; TempY < Height; ++TempY) {
				for (TempX = 0; TempX < Width; ++TempX) {
					SPBitmap.SetPColour(TempX, TempY, 0, 0, 0);
				}
				if (!DoEvents(NULL)) {
					if (TempArrayRed) delete [] TempArrayRed;
					if (TempArrayGreen) delete [] TempArrayGreen;
					if (TempArrayBlue) delete [] TempArrayBlue;
					return;
				}
			}
		}

		//END   1,0,1,0 Modification

//Step 2 - Draw starfield
		LONG32 StarFieldDensity = rand() % 7500 + 2500; //Between 2500 and 10000
		LONG32 StarFieldBrightness = 0; //This is the base starfield brightness
		LONG32 StarFieldBrightnessVar = 230; //This is the TOTAL maximum brightness variation (+ | -)
		LONG32 TempBrightness = 0;
		LONG32 TempAvg = 0;
		for (LONG32 TempCounter = 0; TempCounter <= StarFieldDensity; ++TempCounter) {
			TempX = rand() % Width;
			TempY = rand() % Height;
			TempBrightness = StarFieldBrightness + StarFieldBrightnessVar / 2 - rand() % StarFieldBrightnessVar;
			//BEGIN 1,0,1,0 Additions
			if (rand() % 1000 == 0)
				TempBrightness += StarFieldBrightnessVar / 2;
			//END 1,0,1,0 Additions
			if (TempBrightness < 0) TempBrightness = 0;
			TempIndex = SPBitmap.GetPixelIndex(TempX, TempY);
			/*
			BEGIN OLD 1,0,0,0 CODE
			TempRed = SPBitmap.PixelData[TempIndex + 2] + TempBrightness;
			TempGreen = SPBitmap.PixelData[TempIndex + 1] + TempBrightness;
			TempBlue = SPBitmap.PixelData[TempIndex] + TempBrightness;
			END OLD 1,0,0,0 CODE
			*/
			//BEGIN 1,0,1,0 Modifications
			/*
			Ok, the TempBrightness is going to be compared against the RGB average.
			If it is brighter, add the RGB differences
			*/
			TempRed = SPBitmap.PixelData[TempIndex + 2];
			TempGreen = SPBitmap.PixelData[TempIndex + 1];
			TempBlue = SPBitmap.PixelData[TempIndex];
			TempAvg = (TempRed + TempGreen + TempBlue) / 3;
			if (TempBrightness > TempAvg) {
				/*if (TempBrightness > TempRed) TempRed = TempBrightness;
				if (TempBrightness > TempGreen) TempGreen = TempBrightness;
				if (TempBrightness > TempBlue) TempBlue = TempBrightness;*/
				if (TempBrightness - TempAvg > TempRed) TempRed = TempBrightness - TempAvg;
				if (TempBrightness - TempAvg > TempGreen) TempGreen = TempBrightness - TempAvg;
				if (TempBrightness - TempAvg > TempBlue) TempBlue = TempBrightness - TempAvg;
				/*TempRed += TempBrightness - TempAvg;
				TempGreen += TempBrightness - TempAvg;
				TempBlue += TempBrightness - TempAvg;*/
				/*if (TempBrightness > TempRed) TempRed += TempBrightness - TempAvg;
				if (TempBrightness > TempGreen) TempGreen += TempBrightness - TempAvg;
				if (TempBrightness > TempBlue) TempBlue += TempBrightness - TempAvg;*/
				/*TempRed |= (TempBrightness - TempAvg);
				TempGreen |= (TempBrightness - TempAvg);
				TempBlue |= (TempBrightness - TempAvg);*/
			}
			//END 1,0,1,0 Modifications
			if (TempRed > 255) TempRed = 255;
			if (TempGreen > 255) TempGreen = 255;
			if (TempBlue > 255) TempBlue = 255;
			SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue));
			if (!DoEvents(NULL))  {
				if (TempArrayRed) delete [] TempArrayRed;
				if (TempArrayGreen) delete [] TempArrayGreen;
				if (TempArrayBlue) delete [] TempArrayBlue;
				return;
			}
		}

//Step 3 - Draw a basic star with noise
		int NumStars = 1;
		//RGBTRIPLE StarColour = {0};
		LONG32 StarRed = 0;
		LONG32 StarGreen = 0;
		LONG32 StarBlue = 0;
		LONG32 StarSize = 0;
		LONG32 CoronaSize = 0;
		LONG32 FullStarSize = 0;
		LONG32 CurrentStarX = 0;
		LONG32 CurrentStarY = 0;
		LONG32 MaxSEven = 0;
		LONG32 TempEven = 0;
		double TempDist = 0;
		double TempCDist = 0;
		BOOL CompletePerlin = FALSE;
		//BEGIN 1,0,1,0 ADDITIONS
		BitmapOps FlareBmp(1, 1); //Dimentions don't matter yet
		LONG32 FlareIndex = 0;
		//END 1,0,1,0 ADDITIONS
		if (rand() % 10 == 0) { //1/10 chance of multiple stars
			NumStars = rand() % 4 + 2; //2 - 5
		} //else already initialised to 1
		for (TempCounter = 0; TempCounter < NumStars; ++TempCounter) {
			StarRed = rand() % 256;
			StarGreen = rand() % 256;
			StarBlue = rand() % 256;
			StarSize = rand() % 250 + 1;
			CoronaSize = rand() % 150 + 1;
			FullStarSize = StarSize + CoronaSize;
			CurrentStarX = rand() % Width;
			CurrentStarY = rand() % Height;
			MaxSEven = rand() % 64 + 1;
			//BEGIN 1,0,1,0 ADDITIONS + MODIFICATIONS
			XRealMin = CurrentStarX - FullStarSize;
			YRealMin = CurrentStarY - FullStarSize;
			XRealMax = CurrentStarX + FullStarSize + 1;
			YRealMax = CurrentStarY + FullStarSize + 1;
			XMin = ((XRealMin < 0) ? 0 : XRealMin);
			YMin = ((YRealMin < 0) ? 0 : YRealMin);
			XMax = ((XRealMax > Width) ? Width : XRealMax);
			YMax = ((YRealMax > Height) ? Height : YRealMax);
			FlareBmp.Resize(FullStarSize * 2, FullStarSize * 2);
			/*if (rand() % 15 == 0) {
				CompletePerlin = TRUE;
				FillPerlin(1, 1, 1, -2, FlareBmp, StarRed, 128, StarGreen, 128, StarBlue, 128);
			} else if (rand() % 5 == 0) {*/
				CompletePerlin = FALSE;
				FillPerlin(1, 1, (double)rand() / (double)RAND_MAX * 0.2 + 0.75, -2, FlareBmp, StarRed, 128, StarGreen, 128, StarBlue, 128, StarSize);
			/*} else {
				CompletePerlin = FALSE;
				FillPerlin(1, 1, 0, 1, FlareBmp, StarRed, 0, StarGreen, 0, StarBlue, 0);
			}*/
			if (!bRunning) { //Chance that the DoEvents in FillPerlin caught the exit, don't want to keep drawing if that happened...
				if (TempArrayRed) delete [] TempArrayRed;
				if (TempArrayGreen) delete [] TempArrayGreen;
				if (TempArrayBlue) delete [] TempArrayBlue;
				return;
			}
			//END 1,0,1,0 ADDITIONS + MODIFICATIONS
			//BEGIN OLD 1,0,0,0 CODE
			/*for (TempY = ((CurrentStarY - FullStarSize < 0) ? 0 : CurrentStarY - FullStarSize); TempY < ((CurrentStarY + FullStarSize + 1 > Height) ? Height : CurrentStarY + FullStarSize + 1); ++TempY) {
				for (TempX = ((CurrentStarX - FullStarSize < 0) ? 0 : CurrentStarX - FullStarSize); TempX < ((CurrentStarX + FullStarSize + 1 > Width) ? Width : CurrentStarX + FullStarSize + 1); ++TempX) {*/
			/*XMin = ((CurrentStarX - FullStarSize < 0) ? 0 : CurrentStarX - FullStarSize);
			YMin = ((CurrentStarY - FullStarSize < 0) ? 0 : CurrentStarY - FullStarSize);
			XMax = ((CurrentStarX + FullStarSize + 1 > Width) ? Width : CurrentStarX + FullStarSize + 1);
			YMax = ((CurrentStarY + FullStarSize + 1 > Height) ? Height : CurrentStarY + FullStarSize + 1);*/
			//END OLD 1,0,0,0 CODE
			for (TempY = YMin; TempY < YMax; ++TempY) {
				for (TempX = XMin; TempX < XMax; ++TempX) {
					TempDist = sqrt(pow((signed)TempX - (signed)CurrentStarX, 2) + pow((signed)TempY - (signed)CurrentStarY, 2));
					TempIndex = SPBitmap.GetPixelIndex(TempX, TempY);
					FlareIndex = FlareBmp.GetPixelIndex(TempX - XRealMin, TempY - YRealMin);
					if (TempDist < StarSize) {
						TempEven = LONG32((rand() % (MaxSEven * 2) - MaxSEven) / double(StarSize) * double(StarSize - TempDist));
						//BEGIN OLD 1,0,0,0 CODE
						/*TempRed = StarRed + TempEven;
						TempGreen = StarGreen + TempEven;
						TempBlue = StarBlue + TempEven;*/
						//END OLD 1,0,0,0 CODE
						//BEGIN 1,0,1,0 MODIFICATIONS
						if (CompletePerlin) {
							TempRed = FlareBmp.PixelData[FlareIndex + 2];// + TempEven;
							TempGreen = FlareBmp.PixelData[FlareIndex + 1];// + TempEven;
							TempBlue = FlareBmp.PixelData[FlareIndex];// + TempEven;
						} else {
							/*TempRed = LONG32(FlareBmp.PixelData[FlareIndex + 2] / double(StarSize) * double(TempDist) + StarRed / double(StarSize) * double(StarSize - TempDist) + TempEven);
							TempGreen = LONG32(FlareBmp.PixelData[FlareIndex + 1] / double(StarSize) * double(TempDist) + StarGreen / double(StarSize) * double(StarSize - TempDist) + TempEven);
							TempBlue = LONG32(FlareBmp.PixelData[FlareIndex] / double(StarSize) * double(TempDist) + StarBlue / double(StarSize) * double(StarSize - TempDist) + TempEven);*/
							/*TempRed = LONG32((FlareBmp.PixelData[FlareIndex + 2] - StarRed) * 0.65 + StarRed + TempEven);
							TempGreen = LONG32((FlareBmp.PixelData[FlareIndex + 1] - StarGreen) * 0.65 + StarGreen + TempEven);
							TempBlue = LONG32((FlareBmp.PixelData[FlareIndex] - StarBlue) * 0.65 + StarBlue + TempEven);*/
							//Good:
							/*TempRed = LONG32(((FlareBmp.PixelData[FlareIndex + 2] - StarRed) * 0.65 + StarRed) / double(StarSize) * double(TempDist) + StarRed / double(StarSize) * double(StarSize - TempDist) + TempEven);
							TempGreen = LONG32(((FlareBmp.PixelData[FlareIndex + 1] - StarGreen) * 0.65 + StarGreen) / double(StarSize) * double(TempDist) + StarGreen / double(StarSize) * double(StarSize - TempDist) + TempEven);
							TempBlue = LONG32(((FlareBmp.PixelData[FlareIndex] - StarBlue) * 0.65 + StarBlue) / double(StarSize) * double(TempDist) + StarBlue / double(StarSize) * double(StarSize - TempDist) + TempEven);*/
							TempRed = StarRed + TempEven;
							TempGreen = StarGreen + TempEven;
							TempBlue = StarBlue + TempEven;
						}
						//END 1,0,1,0 MODIFICATIONS
						if (TempRed > 255) TempRed = 255;
						else if (TempRed < 0) TempRed = 0;
						if (TempGreen > 255) TempGreen = 255;
						else if (TempGreen < 0) TempGreen = 0;
						if (TempBlue > 255) TempBlue = 255;
						else if (TempBlue < 0) TempBlue = 0;
						SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue));
					} else if (TempDist < FullStarSize) {
						TempCDist = (CoronaSize - (TempDist - StarSize)) / (double)CoronaSize * 255;
						//BEGIN OLD 1,0,0,0 CODE
						/*TempRed = LONG32(SPBitmap.PixelData[TempIndex + 2] / (double)CoronaSize * (TempDist - StarSize) + StarRed / 255.0 * TempCDist);
						TempGreen = LONG32(SPBitmap.PixelData[TempIndex + 1] / (double)CoronaSize * (TempDist - StarSize) + StarGreen / 255.0 * TempCDist);
						TempBlue = LONG32(SPBitmap.PixelData[TempIndex] / (double)CoronaSize * (TempDist - StarSize) + StarBlue / 255.0 * TempCDist);*/
						//END OLD 1,0,0,0 CODE
						//BEGIN 1,0,1,0 MODIFICATIONS
						//Best!!!:
						TempRed = LONG32(SPBitmap.PixelData[TempIndex + 2] / (double)CoronaSize * (TempDist - StarSize) / (double)CoronaSize * (TempDist - StarSize) + FlareBmp.PixelData[FlareIndex + 2] / 255.0 * TempCDist / (double)CoronaSize * (TempDist - StarSize) + StarRed / 255.0 * TempCDist - StarRed / 255.0 * TempCDist / (double)CoronaSize * (TempDist - StarSize));
						TempGreen = LONG32(SPBitmap.PixelData[TempIndex + 1] / (double)CoronaSize * (TempDist - StarSize) / (double)CoronaSize * (TempDist - StarSize) + FlareBmp.PixelData[FlareIndex + 1] / 255.0 * TempCDist / (double)CoronaSize * (TempDist - StarSize) + StarGreen / 255.0 * TempCDist - StarGreen / 255.0 * TempCDist / (double)CoronaSize * (TempDist - StarSize));
						TempBlue = LONG32(SPBitmap.PixelData[TempIndex] / (double)CoronaSize * (TempDist - StarSize) / (double)CoronaSize * (TempDist - StarSize) + FlareBmp.PixelData[FlareIndex] / 255.0 * TempCDist / (double)CoronaSize * (TempDist - StarSize) + StarBlue / 255.0 * TempCDist - StarBlue / 255.0 * TempCDist / (double)CoronaSize * (TempDist - StarSize));
						//Good:
						/*TempRed = LONG32(SPBitmap.PixelData[TempIndex + 2] / (double)CoronaSize * (TempDist - StarSize) + FlareBmp.PixelData[FlareIndex + 2] * 0.95 / 255.0 * TempCDist);
						TempGreen = LONG32(SPBitmap.PixelData[TempIndex + 1] / (double)CoronaSize * (TempDist - StarSize) + FlareBmp.PixelData[FlareIndex + 1] * 0.95 / 255.0 * TempCDist);
						TempBlue = LONG32(SPBitmap.PixelData[TempIndex] / (double)CoronaSize * (TempDist - StarSize) + FlareBmp.PixelData[FlareIndex] * 0.95 / 255.0 * TempCDist);*/
						//END 1,0,1,0 MODIFICATIONS
						if (TempRed > 255) TempRed = 255;
						//else if (TempRed < 0) TempRed = 0; //Required for some experimental formula variants
						if (TempGreen > 255) TempGreen = 255;
						//else if (TempGreen < 0) TempGreen = 0; //Required for some experimental formula variants
						if (TempBlue > 255) TempBlue = 255; 
						//else if (TempBlue < 0) TempBlue = 0; //Required for some experimental formula variants
						SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue));
					}
				}
				if (!DoEvents(NULL)) {
					if (TempArrayRed) delete [] TempArrayRed;
					if (TempArrayGreen) delete [] TempArrayGreen;
					if (TempArrayBlue) delete [] TempArrayBlue;
					return;
				}
			}
//Step 4 - Smooth star
			LONG32 SideCount, CornerCount, TempSide, TempCorner;
			//First, copy the star into three colour coded arrays:
			for (TempY = ((CurrentStarY - FullStarSize < 0) ? 0 : CurrentStarY - FullStarSize); TempY < ((CurrentStarY + FullStarSize + 1 > Height) ? Height : CurrentStarY + FullStarSize + 1); ++TempY) {
				for (TempX = ((CurrentStarX - FullStarSize < 0) ? 0 : CurrentStarX - FullStarSize); TempX < ((CurrentStarX + FullStarSize + 1 > Width) ? Width : CurrentStarX + FullStarSize + 1); ++TempX) {
					TempIndex = SPBitmap.GetPixelIndex(TempX, TempY);
					TempArrayRed[TempY * Width + TempX] = SPBitmap.PixelData[TempIndex + 2];
					TempArrayGreen[TempY * Width + TempX] = SPBitmap.PixelData[TempIndex + 1];
					TempArrayBlue[TempY * Width + TempX] = SPBitmap.PixelData[TempIndex];
				}
				if (!DoEvents(NULL)) {
					if (TempArrayRed) delete [] TempArrayRed;
					if (TempArrayGreen) delete [] TempArrayGreen;
					if (TempArrayBlue) delete [] TempArrayBlue;
					return;
				}
			}
			//Now, copy it back, smoothing as we go:
			for (TempY = ((CurrentStarY - FullStarSize < 0) ? 0 : CurrentStarY - FullStarSize); TempY < ((CurrentStarY + FullStarSize + 1 > Height) ? Height : CurrentStarY + FullStarSize + 1); ++TempY) {
				for (TempX = ((CurrentStarX - FullStarSize < 0) ? 0 : CurrentStarX - FullStarSize); TempX < ((CurrentStarX + FullStarSize + 1 > Width) ? Width : CurrentStarX + FullStarSize + 1); ++TempX) {
					TempDist = sqrt(pow((signed)TempX - (signed)CurrentStarX, 2) + pow((signed)TempY - (signed)CurrentStarY, 2));
					if (TempDist < StarSize) {
						//Red:
						TempRed = TempArrayRed[TempY * Width + TempX] / 4;
						SideCount = 4; CornerCount = 4; TempSide = 0; TempCorner = 0;
						if (TempX > 0) {
							if (TempY > 0) TempCorner += TempArrayRed[(TempY - 1) * Width + TempX - 1];
							else CornerCount--;
							TempSide += TempArrayRed[TempY * Width + TempX - 1];
							if (TempY < Height - 1) TempCorner += TempArrayRed[(TempY + 1) * Width + TempX - 1];
							else CornerCount--;
						} else {
							SideCount--;
							CornerCount -= 2;
						}
						if (TempY > 0) TempSide += TempArrayRed[(TempY - 1) * Width + TempX];
						else SideCount--;
						if (TempY < Height - 1) TempSide += TempArrayRed[(TempY + 1) * Width + TempX];
						else SideCount--;
						if (TempX < Width - 1) {
							if (TempY > 0) TempCorner += TempArrayRed[(TempY - 1) * Width + TempX + 1];
							else CornerCount--;
							TempSide += TempArrayRed[TempY * Width + TempX + 1];
							if (TempY < Height - 1) TempCorner += TempArrayRed[(TempY + 1) * Width + TempX + 1];
							else CornerCount--;
						} else {
							SideCount--;
							CornerCount -= 2;
						}
						TempRed += TempSide / (2 * SideCount) + TempCorner / (4 * CornerCount);
						//Green:
						TempGreen = TempArrayGreen[TempY * Width + TempX] / 4;
						SideCount = 4; CornerCount = 4; TempSide = 0; TempCorner = 0;
						if (TempX > 0) {
							if (TempY > 0) TempCorner += TempArrayGreen[(TempY - 1) * Width + TempX - 1];
							else CornerCount--;
							TempSide += TempArrayGreen[TempY * Width + TempX - 1];
							if (TempY < Height - 1) TempCorner += TempArrayGreen[(TempY + 1) * Width + TempX - 1];
							else CornerCount--;
						} else {
							SideCount--;
							CornerCount -= 2;
						}
						if (TempY > 0) TempSide += TempArrayGreen[(TempY - 1) * Width + TempX];
						else SideCount--;
						if (TempY < Height - 1) TempSide += TempArrayGreen[(TempY + 1) * Width + TempX];
						else SideCount--;
						if (TempX < Width - 1) {
							if (TempY > 0) TempCorner += TempArrayGreen[(TempY - 1) * Width + TempX + 1];
							else CornerCount--;
							TempSide += TempArrayGreen[TempY * Width + TempX + 1];
							if (TempY < Height - 1) TempCorner += TempArrayGreen[(TempY + 1) * Width + TempX + 1];
							else CornerCount--;
						} else {
							SideCount--;
							CornerCount -= 2;
						}
						//Blue:
						TempGreen += TempSide / (2 * SideCount) + TempCorner / (4 * CornerCount);
						TempBlue = TempArrayBlue[TempY * Width + TempX] / 4;
						SideCount = 4; CornerCount = 4; TempSide = 0; TempCorner = 0;
						if (TempX > 0) {
							if (TempY > 0) TempCorner += TempArrayBlue[(TempY - 1) * Width + TempX - 1];
							else CornerCount--;
							TempSide += TempArrayBlue[TempY * Width + TempX - 1];
							if (TempY < Height - 1) TempCorner += TempArrayBlue[(TempY + 1) * Width + TempX - 1];
							else CornerCount--;
						} else {
							SideCount--;
							CornerCount -= 2;
						}
						if (TempY > 0) TempSide += TempArrayBlue[(TempY - 1) * Width + TempX];
						else SideCount--;
						if (TempY < Height - 1) TempSide += TempArrayBlue[(TempY + 1) * Width + TempX];
						else SideCount--;
						if (TempX < Width - 1) {
							if (TempY > 0) TempCorner += TempArrayBlue[(TempY - 1) * Width + TempX + 1];
							else CornerCount--;
							TempSide += TempArrayBlue[TempY * Width + TempX + 1];
							if (TempY < Height - 1) TempCorner += TempArrayBlue[(TempY + 1) * Width + TempX + 1];
							else CornerCount--;
						} else {
							SideCount--;
							CornerCount -= 2;
						}
						TempBlue += TempSide / (2 * SideCount) + TempCorner / (4 * CornerCount);
						SPBitmap.SetPColour(TempX, TempY, uchar(TempRed), uchar(TempGreen), uchar(TempBlue));
					}
				}
				if (!DoEvents(NULL)) {
					if (TempArrayRed) delete [] TempArrayRed;
					if (TempArrayGreen) delete [] TempArrayGreen;
					if (TempArrayBlue) delete [] TempArrayBlue;
					return;
				}
			}
		}


		if (TempArrayRed) delete [] TempArrayRed;
		if (TempArrayGreen) delete [] TempArrayGreen;
		if (TempArrayBlue) delete [] TempArrayBlue;
	}
	catch(...) {
		//In the unlikely event that an error occured, we must unalocate memory
		if (TempArrayRed) delete [] TempArrayRed;
		if (TempArrayGreen) delete [] TempArrayGreen;
		if (TempArrayBlue) delete [] TempArrayBlue;
	}
	bDrawing = false;
}
