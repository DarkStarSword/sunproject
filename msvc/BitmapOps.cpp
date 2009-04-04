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

#include "BitmapOps.h"

BitmapOps::BitmapOps(long Width, long Height) {
	bmInfo.bmiHeader.biWidth = Width;
	bmInfo.bmiHeader.biHeight = Height;
	bmInfo.bmiHeader.biBitCount = 24;
	bmInfo.bmiHeader.biClrImportant = 0;
	bmInfo.bmiHeader.biClrUsed = 0;
	bmInfo.bmiHeader.biCompression = BI_RGB;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);
	//bmInfo.bmiHeader.biSizeImage = (unsigned long)sqrt(pow(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biHeight * (bmInfo.bmiHeader.biBitCount / 8), 2));
	//bmInfo.bmiHeader.biSizeImage = (unsigned long)sqrt(pow(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biHeight * (bmInfo.bmiHeader.biBitCount / 8) + bmInfo.bmiHeader.biWidth % 4 * bmInfo.bmiHeader.biHeight, 2));
	bmInfo.bmiHeader.biSizeImage = DWORD((LONG32(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount + 31) / 32) * 4 * bmInfo.bmiHeader.biHeight);
	bmInfo.bmiHeader.biXPelsPerMeter = 2835; //What-ever
	bmInfo.bmiHeader.biYPelsPerMeter = 2835; //What-ever

	bmFileHeader.bfType = 19778;//'BM';
	bmFileHeader.bfOffBits = sizeof(bmInfo.bmiHeader) + sizeof(bmFileHeader);//54;
	bmFileHeader.bfReserved1 = 0;
	bmFileHeader.bfReserved2 = 0;
	bmFileHeader.bfSize = bmFileHeader.bfOffBits/*54*/ + bmInfo.bmiHeader.biSizeImage;

	PixelData = new uchar[bmInfo.bmiHeader.biSizeImage];
	if (!PixelData) {
		PostQuitMessage(EXIT_FAILURE);
	}

	//Initialise DrawDib library:
	hDd = DrawDibOpen();
}

BitmapOps::BitmapOps(long Width, long Height, BOOL Alpha) {
	bmInfo.bmiHeader.biWidth = Width;
	bmInfo.bmiHeader.biHeight = Height;
	if (Alpha)
		bmInfo.bmiHeader.biBitCount = 32;
	else
		bmInfo.bmiHeader.biBitCount = 24;
	bmInfo.bmiHeader.biClrImportant = 0;
	bmInfo.bmiHeader.biClrUsed = 0;
	bmInfo.bmiHeader.biCompression = BI_RGB;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);
	bmInfo.bmiHeader.biSizeImage = DWORD((LONG32(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount + 31) / 32) * 4 * bmInfo.bmiHeader.biHeight);
	bmInfo.bmiHeader.biXPelsPerMeter = 2835; //What-ever
	bmInfo.bmiHeader.biYPelsPerMeter = 2835; //What-ever

	bmFileHeader.bfType = 19778;//'BM';
	bmFileHeader.bfOffBits = sizeof(bmInfo.bmiHeader) + sizeof(bmFileHeader);//54;
	bmFileHeader.bfReserved1 = 0;
	bmFileHeader.bfReserved2 = 0;
	bmFileHeader.bfSize = bmFileHeader.bfOffBits/*54*/ + bmInfo.bmiHeader.biSizeImage;

	PixelData = new uchar[bmInfo.bmiHeader.biSizeImage];
	if (!PixelData) {
		PostQuitMessage(EXIT_FAILURE);
	}

	//Initialise DrawDib library:
	hDd = DrawDibOpen();
}

BitmapOps::~BitmapOps() {
	if (PixelData) delete [] PixelData; PixelData = 0;
	//DeInit DrawDib library:
	DrawDibClose(hDd);
}


void BitmapOps::CopyImageArrayToDC(HWND hWnd) {
	HDC hDc = GetDC(hWnd);
	if (!hDc) return;

	RECT ClientRect;
	GetClientRect(hWnd, &ClientRect);

	//StretchDIBits(hdc, ClientRect.left, ClientRect.top, ClientRect.right, ClientRect.bottom, 0, 0, bmInfo.bmiHeader.biWidth, bmInfo.bmiHeader.biHeight, PixelData, &bmInfo, 0, SRCCOPY);
	DrawDibDraw(hDd, hDc, 0, 0, ClientRect.right, ClientRect.bottom, &bmInfo.bmiHeader, PixelData, 0, 0, bmInfo.bmiHeader.biWidth, bmInfo.bmiHeader.biHeight, NULL);

	ReleaseDC(hWnd, hDc);
}

void BitmapOps::CopyImageArrayToDC(HWND hWnd, HDC hDc) {
	RECT ClientRect;
	GetClientRect(hWnd, &ClientRect);

	//StretchDIBits(hDc, ClientRect.left, ClientRect.top, ClientRect.right, ClientRect.bottom, 0, 0, bmInfo.bmiHeader.biWidth, bmInfo.bmiHeader.biHeight, PixelData, &bmInfo, 0, SRCCOPY);
	DrawDibDraw(hDd, hDc, 0, 0, ClientRect.right, ClientRect.bottom, &bmInfo.bmiHeader, PixelData, 0, 0, bmInfo.bmiHeader.biWidth, bmInfo.bmiHeader.biHeight, NULL);
}

void BitmapOps::CopyImageArrayToDC(HWND hWnd, HDC hDc, int DestX, int DestY, int DestW, int DestH, BOOL Clip, int ClipX, int ClipY, int ClipW, int ClipH) {
	if (Clip)
		//StretchDIBits(hDc, DestX, DestY, DestW, DestH, ClipX, ClipY, ClipW, ClipH, PixelData, &bmInfo, 0, SRCCOPY);
		DrawDibDraw(hDd, hDc, DestX, DestY, DestW, DestH, &bmInfo.bmiHeader, PixelData, ClipX, ClipY, ClipW, ClipH, NULL);
	else
		//StretchDIBits(hDc, DestX, DestY, DestW, DestH, 0, 0, bmInfo.bmiHeader.biWidth, bmInfo.bmiHeader.biHeight, PixelData, &bmInfo, 0, SRCCOPY);
		DrawDibDraw(hDd, hDc, DestX, DestY, DestW, DestH, &bmInfo.bmiHeader, PixelData, 0, 0, bmInfo.bmiHeader.biWidth, bmInfo.bmiHeader.biHeight, NULL);
}


void BitmapOps::SetPColour(long X, long Y, uchar Red, uchar Green, uchar Blue) {
	//DWORD PixelIndex = DWORD(Y * bmInfo.bmiHeader.biWidth * (bmInfo.bmiHeader.biBitCount / 8) + X * (bmInfo.bmiHeader.biBitCount / 8));
	//DWORD PixelIndex = DWORD(Y * bmInfo.bmiHeader.biWidth * (bmInfo.bmiHeader.biBitCount / 8) + bmInfo.bmiHeader.biWidth % 4 * Y + X * (bmInfo.bmiHeader.biBitCount / 8));
	DWORD PixelIndex = DWORD((LONG32(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount + 31) / 32) * 4 * Y + X * (bmInfo.bmiHeader.biBitCount / 8));
	//if (PixelIndex >= UBound()) return;
	PixelData[PixelIndex + 2] = Red;
	PixelData[PixelIndex + 1] = Green;
	PixelData[PixelIndex] = Blue;
}


void BitmapOps::SetPColour(long X, long Y, uchar Red, uchar Green, uchar Blue, uchar Alpha) {
	DWORD PixelIndex = DWORD((LONG32(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount + 31) / 32) * 4 * Y + X * (bmInfo.bmiHeader.biBitCount / 8));
	//if (PixelIndex >= UBound()) return;
	PixelData[PixelIndex + 3] = Alpha;
	PixelData[PixelIndex + 2] = Red;
	PixelData[PixelIndex + 1] = Green;
	PixelData[PixelIndex] = Blue;
}

void BitmapOps::SetPColour(long X, long Y, COLORREF Colour) {
	if (X < 0 || Y < 0 || X >= bmInfo.bmiHeader.biWidth || Y >= bmInfo.bmiHeader.biHeight) return;
	DWORD PixelIndex = DWORD((LONG32(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount + 31) / 32) * 4 * Y + X * (bmInfo.bmiHeader.biBitCount / 8));
	//if (PixelIndex >= UBound()) return;
	memcpy(&PixelData[PixelIndex], &Colour, bmInfo.bmiHeader.biBitCount / 8);
}


DWORD BitmapOps::GetPixelIndex(long X, long Y) {
	return DWORD((LONG32(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount + 31) / 32) * 4 * Y + X * (bmInfo.bmiHeader.biBitCount / 8));
	//return DWORD(Y * bmInfo.bmiHeader.biWidth * (bmInfo.bmiHeader.biBitCount / 8) + bmInfo.bmiHeader.biWidth % 4 * Y + X * (bmInfo.bmiHeader.biBitCount / 8));
}

bool BitmapOps::Resize(long Width, long Height) {
	bmInfo.bmiHeader.biWidth = Width;
	bmInfo.bmiHeader.biHeight = Height;
	bmInfo.bmiHeader.biSizeImage = (LONG32(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount + 31) / 32) * 4 * bmInfo.bmiHeader.biHeight;//*/(unsigned long)sqrt(pow(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biHeight * (bmInfo.bmiHeader.biBitCount / 8) + bmInfo.bmiHeader.biWidth % 4 * bmInfo.bmiHeader.biHeight, 2));
	bmFileHeader.bfSize = bmFileHeader.bfOffBits/*54*/ + bmInfo.bmiHeader.biSizeImage;
	uchar *TempArray = new uchar[bmInfo.bmiHeader.biSizeImage];
	if (TempArray) {
		if (PixelData) delete [] PixelData;
		PixelData = TempArray;
		return true;
	} else {
		return false;
	}
}

void BitmapOps::FadeBitmaps(BitmapOps &Source1, BitmapOps &Source2, double Faded) {
	for (DWORD i = 0; i < UBound(); i += 3) {
		PixelData[i + 2] = uchar(Source1.PixelData[i + 2] * (1 - Faded) + Source2.PixelData[i + 2] * Faded);
		PixelData[i + 1] = uchar(Source1.PixelData[i + 1] * (1 - Faded) + Source2.PixelData[i + 1] * Faded);
		PixelData[i] = uchar(Source1.PixelData[i] * (1 - Faded) + Source2.PixelData[i] * Faded);
	}
}


						/******INCOMPLETE******/
void BitmapOps::LoadImage(LPSTR FileName, BOOL SuppressInfo) {
	HANDLE hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile ==  INVALID_HANDLE_VALUE) {
		MessageBox(NULL, "File not found!", "Error!", MB_ICONERROR);
		return;
	}
	DWORD BytesRead = 0;
	DWORD FileSize = GetFileSize(hFile, NULL);
	if (!FileSize) {
		MessageBox(NULL, "Filesize reported as 0!\nUnable to open!", "Error!", MB_ICONERROR);
		CloseHandle(hFile);
		return;
	}
	char *FileBuffer = NULL;
	try {
		FileBuffer = new char[FileSize];
		if (!FileBuffer) {
			MessageBox(NULL, "Insufficient Memory to open file!", "Error!", MB_ICONERROR);
		} else {
			//Load the entire file into memory:
			BOOL Retry;
			do {
				Retry = FALSE;
				if (!ReadFile(hFile, FileBuffer, FileSize, &BytesRead, NULL)) {
					char *lpMsgBuf;
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
					//if (MessageBox(NULL, "An Error occured while reading the file", "Read Error!", MB_ICONEXCLAMATION | MB_RETRYCANCEL) == IDRETRY) Retry = TRUE;
					if (MessageBox(NULL, (LPCTSTR)lpMsgBuf, "Error!", MB_ICONEXCLAMATION | MB_RETRYCANCEL) == IDRETRY) Retry = TRUE;
					LocalFree(lpMsgBuf);
				}
			} while (Retry);
			if (FileBuffer[0] == 'B' && FileBuffer[1] == 'M') {
				//Bitmap:
				BITMAPFILEHEADER tmpFileHeader = {0};
				BITMAPINFOHEADER tmpInfoHeader = {0};
				if (FileSize < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) {
					MessageBox(NULL, "Bitmap filesize check failed!", "Error!", MB_ICONERROR);
				} else {
					memcpy(&tmpFileHeader, FileBuffer, sizeof(BITMAPFILEHEADER));
					memcpy(&tmpInfoHeader, FileBuffer + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));
					if (tmpInfoHeader.biBitCount == 24) {
						bmInfo.bmiHeader.biBitCount = 24;
						if (tmpInfoHeader.biHeight >= 0) {
							Resize(tmpInfoHeader.biWidth, tmpInfoHeader.biHeight);
							memcpy(PixelData, FileBuffer + tmpFileHeader.bfOffBits, bmInfo.bmiHeader.biSizeImage);//FileSize - tmpFileHeader.bfOffBits);
						} else { //top-down bitmap
							if (!SuppressInfo)
								MessageBox(NULL, "This is a top-down bitmap and will be converted into a bottom-up bitmap", "Bitmap conversion information", MB_ICONINFORMATION);
							Resize(tmpInfoHeader.biWidth, -tmpInfoHeader.biHeight);
							LONG32 y;
							DWORD i, l;
							for (y = 0; y < -tmpInfoHeader.biHeight; y++) {
								i = GetPixelIndex(0, y);
								l = (DWORD)(((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * (-tmpInfoHeader.biHeight - y - 1));
								memcpy(PixelData + i, FileBuffer + tmpFileHeader.bfOffBits + l, tmpInfoHeader.biWidth * 3);
							}
						}
					} else if (tmpInfoHeader.biBitCount == 32) {
						if (tmpInfoHeader.biCompression == BI_RGB) {
							if (SuppressInfo) {
								bmInfo.bmiHeader.biBitCount = 32;
									if (tmpInfoHeader.biHeight >= 0) {
										Resize(tmpInfoHeader.biWidth, tmpInfoHeader.biHeight);
										memcpy(PixelData, FileBuffer + tmpFileHeader.bfOffBits, bmInfo.bmiHeader.biSizeImage);//FileSize - tmpFileHeader.bfOffBits);
									} else { //top-down bitmap
										if (!SuppressInfo)
											MessageBox(NULL, "This is a top-down bitmap and will be converted into a bottom-up bitmap", "Bitmap conversion information", MB_ICONINFORMATION);
										Resize(tmpInfoHeader.biWidth, -tmpInfoHeader.biHeight);
										LONG32 y;
										DWORD i, l;
										for (y = 0; y < -tmpInfoHeader.biHeight; y++) {
											i = GetPixelIndex(0, y);
											l = (DWORD)(((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * (-tmpInfoHeader.biHeight - y - 1));
											memcpy(PixelData + i, FileBuffer + tmpFileHeader.bfOffBits + l, tmpInfoHeader.biWidth * 4);
											//memcpy(PixelData + i, FileBuffer + tmpFileHeader.bfOffBits, bmInfo.bmiHeader.biSizeImage);//FileSize - tmpFileHeader.bfOffBits);
										}
									}
							} else {
								if (MessageBox(NULL, "This bitmap contains an alpha channel, Do you wish to preserve it?", "Preserve alpha?", MB_YESNO | MB_ICONQUESTION) == IDYES) {
									bmInfo.bmiHeader.biBitCount = 32;
									if (tmpInfoHeader.biHeight >= 0) {
										Resize(tmpInfoHeader.biWidth, tmpInfoHeader.biHeight);
										memcpy(PixelData, FileBuffer + tmpFileHeader.bfOffBits, bmInfo.bmiHeader.biSizeImage);//FileSize - tmpFileHeader.bfOffBits);
									} else { //top-down bitmap
										if (!SuppressInfo)
											MessageBox(NULL, "This is a top-down bitmap and will be converted into a bottom-up bitmap", "Bitmap conversion information", MB_ICONINFORMATION);
										Resize(tmpInfoHeader.biWidth, -tmpInfoHeader.biHeight);
										LONG32 y;
										DWORD i, l;
										for (y = 0; y < -tmpInfoHeader.biHeight; y++) {
											i = GetPixelIndex(0, y);
											l = (DWORD)(((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * (-tmpInfoHeader.biHeight - y - 1));
											memcpy(PixelData + i, FileBuffer + tmpFileHeader.bfOffBits + l, tmpInfoHeader.biWidth * 4);
											//memcpy(PixelData + i, FileBuffer + tmpFileHeader.bfOffBits, bmInfo.bmiHeader.biSizeImage);//FileSize - tmpFileHeader.bfOffBits);
										}
									}
								} else {
									bmInfo.bmiHeader.biBitCount = 24;
									if (tmpInfoHeader.biHeight >= 0) {
										Resize(tmpInfoHeader.biWidth, tmpInfoHeader.biHeight);
										LONG32 x, y;
										DWORD i, l;
										for (y = 0; y < tmpInfoHeader.biHeight; y++) {
											for (x = 0; x < tmpInfoHeader.biWidth; x++) {
												i = GetPixelIndex(x, y);
												l = (DWORD)(((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * y + x * (tmpInfoHeader.biBitCount / 8));//*/DWORD(y * tmpInfoHeader.biWidth * (tmpInfoHeader.biBitCount / 8) + tmpInfoHeader.biWidth % 4 * y + x * (tmpInfoHeader.biBitCount / 8));
												/*PixelData[i + 2] = FileBuffer[tmpFileHeader.bfOffBits + l + 2];
												PixelData[i + 1] = FileBuffer[tmpFileHeader.bfOffBits + l + 1];
												PixelData[i] = FileBuffer[tmpFileHeader.bfOffBits + l];*/
												memcpy(PixelData + i, FileBuffer + tmpFileHeader.bfOffBits + l, 3);
											}
										}
										/*for (DWORD i = tmpFileHeader.bfOffBits; i <= FileSize - tmpFileHeader.bfOffBits; i += 4) {
											memcpy(PixelData + i, FileBuffer + tmpFileHeader.bfOffBits + i, 3);
										}*/
									} else { //top-down bitmap
										if (!SuppressInfo)
											MessageBox(NULL, "This is a top-down bitmap and will be converted into a bottom-up bitmap", "Bitmap conversion information", MB_ICONINFORMATION);
										Resize(tmpInfoHeader.biWidth, -tmpInfoHeader.biHeight);
										LONG32 x, y;
										DWORD i, l;
										for (y = 0; y < -tmpInfoHeader.biHeight; y++) {
											for (x = 0; x < tmpInfoHeader.biWidth; x++) {
												i = GetPixelIndex(x, y);
												l = (DWORD)(((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * (-tmpInfoHeader.biHeight - y - 1) + x * (tmpInfoHeader.biBitCount / 8));
												/*PixelData[i + 2] = FileBuffer[tmpFileHeader.bfOffBits + l + 2];
												PixelData[i + 1] = FileBuffer[tmpFileHeader.bfOffBits + l + 1];
												PixelData[i] = FileBuffer[tmpFileHeader.bfOffBits + l];*/
												memcpy(PixelData + i, FileBuffer + tmpFileHeader.bfOffBits + l, 3);
											}
										}
									}
								}
							}
						} else {
							MessageBox(NULL, "This bitmap uses a bit mask, and cannot be loaded at this stage.\nPlease convert the bitmap into an unmasked format and try again.", "Error!", MB_ICONERROR);
						}
					} else if (tmpInfoHeader.biBitCount == 8) {
						if (tmpInfoHeader.biCompression == BI_RGB) {
							bmInfo.bmiHeader.biBitCount = 24;
							if (tmpInfoHeader.biHeight >= 0) {
								if (!SuppressInfo)
									MessageBox(NULL, "This is an 8 bit bitmap and will be converted into true colour", "Bitmap conversion information", MB_ICONINFORMATION);
								Resize(tmpInfoHeader.biWidth, tmpInfoHeader.biHeight);
								LONG32 NumColours = (tmpFileHeader.bfOffBits - tmpInfoHeader.biSize) / 4;
								RGBQUAD *Palette = (RGBQUAD*)(FileBuffer + sizeof(BITMAPFILEHEADER) + tmpInfoHeader.biSize);
								LONG32 x, y;
								DWORD i, l;
								for (y = 0; y < tmpInfoHeader.biHeight; y++) {
									for (x = 0; x < tmpInfoHeader.biWidth; x++) {
										i = GetPixelIndex(x, y);
										l = (DWORD)(tmpFileHeader.bfOffBits + ((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * y + x * (tmpInfoHeader.biBitCount / 8));
										PixelData[i + 2] = Palette[(unsigned char)FileBuffer[l]].rgbRed;
										PixelData[i + 1] = Palette[(unsigned char)FileBuffer[l]].rgbGreen;
										PixelData[i] = Palette[(unsigned char)FileBuffer[l]].rgbBlue;
									}
								}
							} else { //top-down bitmap
								if (!SuppressInfo)
									MessageBox(NULL, "This is an 8 bit, top-down bitmap and will be converted into a bottom-up, true colour bitmap", "Bitmap conversion information", MB_ICONINFORMATION);
								Resize(tmpInfoHeader.biWidth, -tmpInfoHeader.biHeight);
								LONG32 NumColours = (tmpFileHeader.bfOffBits - tmpInfoHeader.biSize) / 4;
								RGBQUAD *Palette = (RGBQUAD*)(FileBuffer + sizeof(BITMAPFILEHEADER) + tmpInfoHeader.biSize);
								LONG32 x, y;
								DWORD i, l;
								for (y = 0; y < -tmpInfoHeader.biHeight; y++) {
									for (x = 0; x < tmpInfoHeader.biWidth; x++) {
										i = GetPixelIndex(x, y);
										l = (DWORD)(tmpFileHeader.bfOffBits + ((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * (-tmpInfoHeader.biHeight - y - 1) + x * (tmpInfoHeader.biBitCount / 8));
										PixelData[i + 2] = Palette[(unsigned char)FileBuffer[l]].rgbRed;
										PixelData[i + 1] = Palette[(unsigned char)FileBuffer[l]].rgbGreen;
										PixelData[i] = Palette[(unsigned char)FileBuffer[l]].rgbBlue;
									}
								}
							}
						} else {
							MessageBox(NULL, "This bitmap is compressed, and cannot be loaded at this stage.\nPlease convert the bitmap into an uncompressed format and try again.", "Error!", MB_ICONERROR);
						}
					} else {
						MessageBox(NULL, "Conversion is not available for selected bitmap!\nPlease use only uncompressed 256 and true colour bitmaps!", "Error!", MB_ICONERROR);
					}
				}
			} else {
				MessageBox(NULL, "File type check failed!\nThis is not a recognised Image!", "Error!", MB_ICONERROR);
			}
		}
	}
	catch(...) {
		MessageBox(NULL, "Unknown error while reading file!", "Error!", MB_ICONERROR);
	}
	if (FileBuffer) delete [] FileBuffer; FileBuffer = NULL;
	CloseHandle(hFile);
}

void BitmapOps::DumpImage(LPSTR FileName) {
	BOOL Retry = FALSE;
	fstream FileObject;
	do {
		Retry = FALSE;
		try {
			DWORD ErrorCode = 0;
			DWORD Attributes = GetFileAttributes(FileName);
			if (Attributes & FILE_ATTRIBUTE_READONLY && Attributes != 0xFFFFFFFF) {
				if (MessageBox(NULL, "This file is marked as Read-Only, are you sure you wish to overwrite it?", "Confirm Read-Only overwriting", MB_YESNO | MB_ICONQUESTION) == IDNO) {
					return;
				}
			}
			SetFileAttributes(FileName, FILE_ATTRIBUTE_NORMAL);
			FileObject.open(FileName, fstream::out | fstream::binary);
			if (!FileObject.is_open()) {
				MessageBox(NULL, "There was an error opening the file to write to.\nPlease try saving under a different filename", "Error!", MB_ICONERROR);
				return;
			}

			if (!FileObject.write(reinterpret_cast<const char *>(&bmFileHeader), sizeof(bmFileHeader))) {
				ErrorCode = GetLastError();
			} else if (!FileObject.write(reinterpret_cast<const char *>(&bmInfo.bmiHeader), sizeof(bmInfo.bmiHeader))) {
				ErrorCode = GetLastError();
			} else if (!FileObject.write(reinterpret_cast<const char *>(PixelData), UBound())) {
				ErrorCode = GetLastError();
			}
			if (ErrorCode) {
				char *lpMsgBuf;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
				//strcat(lpMsgBuf, "\nRetry?");
				if (MessageBox(NULL, (LPCTSTR)lpMsgBuf, "Error!", MB_RETRYCANCEL | MB_ICONEXCLAMATION) == IDRETRY)
					Retry = TRUE;
				LocalFree(lpMsgBuf);
			}
		}
		catch(...) {
			MessageBox(NULL, "Unknown error while saving file!", "Error!", MB_ICONERROR);
		}
		FileObject.close();
	} while (Retry);
}

void BitmapOps::GetBitmapFromResource(WORD ResourceID) {
	HMODULE hModule = GetModuleHandle(NULL);

	HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(ResourceID), RT_BITMAP);
	if (!hRes) {
		MessageBox(NULL, "Could not find resource", "Error", MB_ICONEXCLAMATION);
	} else {
		DWORD FileSize = SizeofResource(hModule, hRes);
		if (!FileSize) {
			MessageBox(NULL, "Filesize reported as 0!\nUnable to open!", "Error!", MB_ICONERROR);
			return;
		}
		HGLOBAL hResLoad = LoadResource(hModule, hRes);
		if (!hResLoad) {
			MessageBox(NULL, "Could not load resource", "Error", MB_ICONEXCLAMATION);
		} else {
			char *FileBuffer = NULL;
			FileBuffer = (char*)LockResource(hResLoad);
			if (!FileBuffer) {
				MessageBox(NULL, "Could not lock resource", "Error", MB_ICONEXCLAMATION);
			} else {
				//Bitmap:
				BITMAPFILEHEADER tmpFileHeader = {0};
				tmpFileHeader.bfOffBits = sizeof(BITMAPINFOHEADER);
				BITMAPINFOHEADER tmpInfoHeader = {0};
				memcpy(&tmpInfoHeader, FileBuffer, sizeof(BITMAPINFOHEADER));
				if (tmpInfoHeader.biBitCount == 24) {
					bmInfo.bmiHeader.biBitCount = 24;
					if (tmpInfoHeader.biHeight >= 0) {
						Resize(tmpInfoHeader.biWidth, tmpInfoHeader.biHeight);
						memcpy(PixelData, FileBuffer + tmpFileHeader.bfOffBits, bmInfo.bmiHeader.biSizeImage);//FileSize - tmpFileHeader.bfOffBits);
					} else { //top-down bitmap
						Resize(tmpInfoHeader.biWidth, -tmpInfoHeader.biHeight);
						LONG32 y;
						DWORD i, l;
						for (y = 0; y < -tmpInfoHeader.biHeight; y++) {
							i = GetPixelIndex(0, y);
							l = (DWORD)(((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * (-tmpInfoHeader.biHeight - y - 1));
							memcpy(PixelData + i, FileBuffer + tmpFileHeader.bfOffBits + l, tmpInfoHeader.biWidth * 3);
						}
					}
				} else if (tmpInfoHeader.biBitCount == 32) {
					if (tmpInfoHeader.biCompression == BI_RGB) {
						bmInfo.bmiHeader.biBitCount = 32;
						if (tmpInfoHeader.biHeight >= 0) {
							Resize(tmpInfoHeader.biWidth, tmpInfoHeader.biHeight);
							memcpy(PixelData, FileBuffer + tmpFileHeader.bfOffBits, bmInfo.bmiHeader.biSizeImage);//FileSize - tmpFileHeader.bfOffBits);
						} else { //top-down bitmap
							Resize(tmpInfoHeader.biWidth, -tmpInfoHeader.biHeight);
							LONG32 y;
							DWORD i, l;
							for (y = 0; y < -tmpInfoHeader.biHeight; y++) {
								i = GetPixelIndex(0, y);
								l = (DWORD)(((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * (-tmpInfoHeader.biHeight - y - 1));
								memcpy(PixelData + i, FileBuffer + tmpFileHeader.bfOffBits + l, tmpInfoHeader.biWidth * 4);
							}
						}
					} else {
						MessageBox(NULL, "This bitmap uses a bit mask, and cannot be loaded at this stage.\nPlease convert the bitmap into an unmasked format and try again.", "Error!", MB_ICONERROR);
					}
				} else if (tmpInfoHeader.biBitCount == 8) {
					if (tmpInfoHeader.biCompression == BI_RGB) {
						bmInfo.bmiHeader.biBitCount = 24;
						if (tmpInfoHeader.biHeight >= 0) {
							Resize(tmpInfoHeader.biWidth, tmpInfoHeader.biHeight);
							LONG32 NumColours = (tmpFileHeader.bfOffBits - tmpInfoHeader.biSize) / 4;
							RGBQUAD *Palette = (RGBQUAD*)(FileBuffer + sizeof(BITMAPFILEHEADER) + tmpInfoHeader.biSize);
							LONG32 x, y;
							DWORD i, l;
							for (y = 0; y < tmpInfoHeader.biHeight; y++) {
								for (x = 0; x < tmpInfoHeader.biWidth; x++) {
									i = GetPixelIndex(x, y);
									l = (DWORD)(tmpFileHeader.bfOffBits + ((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * y + x * (tmpInfoHeader.biBitCount / 8));
									PixelData[i + 2] = Palette[(unsigned char)FileBuffer[l]].rgbRed;
									PixelData[i + 1] = Palette[(unsigned char)FileBuffer[l]].rgbGreen;
									PixelData[i] = Palette[(unsigned char)FileBuffer[l]].rgbBlue;
								}
							}
						} else { //top-down bitmap
							Resize(tmpInfoHeader.biWidth, -tmpInfoHeader.biHeight);
							LONG32 NumColours = (tmpFileHeader.bfOffBits - tmpInfoHeader.biSize) / 4;
							RGBQUAD *Palette = (RGBQUAD*)(FileBuffer + sizeof(BITMAPFILEHEADER) + tmpInfoHeader.biSize);
							LONG32 x, y;
							DWORD i, l;
							for (y = 0; y < -tmpInfoHeader.biHeight; y++) {
								for (x = 0; x < tmpInfoHeader.biWidth; x++) {
									i = GetPixelIndex(x, y);
									l = (DWORD)(tmpFileHeader.bfOffBits + ((LONG32)(tmpInfoHeader.biWidth * tmpInfoHeader.biBitCount + 31) / 32) * 4 * (-tmpInfoHeader.biHeight - y - 1) + x * (tmpInfoHeader.biBitCount / 8));
									PixelData[i + 2] = Palette[(unsigned char)FileBuffer[l]].rgbRed;
									PixelData[i + 1] = Palette[(unsigned char)FileBuffer[l]].rgbGreen;
									PixelData[i] = Palette[(unsigned char)FileBuffer[l]].rgbBlue;
								}
							}
						}
					} else {
						MessageBox(NULL, "This bitmap is compressed, and cannot be loaded at this stage.\nPlease convert the bitmap into an uncompressed format and try again.", "Error!", MB_ICONERROR);
					}
				} else {
					MessageBox(NULL, "Conversion is not available for selected bitmap!\nPlease use only uncompressed 256 and true colour bitmaps!", "Error!", MB_ICONERROR);
				}
			}
		}
	}
}

void BitmapOps::GetBitmapFromSysMem(/*HDC hDc, */HBITMAP hBitmap) {
	BITMAP Bits = {0};
	GetObject(hBitmap, sizeof(BITMAP), &Bits);

	bmInfo.bmiHeader.biWidth = Bits.bmWidth;
	bmInfo.bmiHeader.biHeight = Bits.bmHeight;
	bmInfo.bmiHeader.biBitCount = 24;
	bmInfo.bmiHeader.biClrImportant = 0;
	bmInfo.bmiHeader.biClrUsed = 0;
	bmInfo.bmiHeader.biCompression = BI_RGB;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);
	//bmInfo.bmiHeader.biSizeImage = (unsigned long)sqrt(pow(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biHeight * (bmInfo.bmiHeader.biBitCount / 8)/* + bmInfo.bmiHeader.biWidth % 4 * bmInfo.bmiHeader.biHeight*/, 2));
	bmInfo.bmiHeader.biSizeImage = DWORD((LONG32(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount + 31) / 32) * 4 * bmInfo.bmiHeader.biHeight);//*/(unsigned long)sqrt(pow(bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biHeight * (bmInfo.bmiHeader.biBitCount / 8) + bmInfo.bmiHeader.biWidth % 4 * bmInfo.bmiHeader.biHeight, 2));
	bmInfo.bmiHeader.biXPelsPerMeter = 2835; //What-ever
	bmInfo.bmiHeader.biYPelsPerMeter = 2835; //What-ever

	bmFileHeader.bfType = 19778;//'BM';
	bmFileHeader.bfOffBits = sizeof(bmInfo.bmiHeader) + sizeof(bmFileHeader);//54;
	bmFileHeader.bfReserved1 = 0;
	bmFileHeader.bfReserved2 = 0;
	bmFileHeader.bfSize = bmFileHeader.bfOffBits/*54*/ + bmInfo.bmiHeader.biSizeImage;
	
	if (PixelData) delete [] PixelData;
	PixelData = new uchar[bmInfo.bmiHeader.biSizeImage];
	if (!PixelData) {
		PostQuitMessage(EXIT_FAILURE);
	}

	//HDC hDc = CreateDC("DISPLAY", NULL, NULL, NULL);
	HDC hDc = CreateCompatibleDC(NULL);

	/*DEVMODE DevM = {0};
	DevM.dmSize = sizeof(DEVMODE);
	DevM.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	DevM.dmBitsPerPel = 24;
	DevM.dmPelsWidth = bmInfo.bmiHeader.biWidth;
	DevM.dmPelsWidth = bmInfo.bmiHeader.biHeight;

	ResetDC(hDc, &DevM);*/

	GetDIBits(hDc, hBitmap, 0, bmInfo.bmiHeader.biHeight, PixelData, &bmInfo, DIB_RGB_COLORS);
	
	DeleteObject(hBitmap);
}

void BitmapOps::CopyBitmapFromBitmapOps(BitmapOps &BmOps) {
	bmInfo.bmiHeader.biBitCount = BmOps.BPP();
	Resize(BmOps.Width(), BmOps.Height());
	memcpy(PixelData, BmOps.PixelData, BmOps.UBound());
}

void BitmapOps::CopyBitmapFromBitmapOps(BitmapOps &BmOps, LONG32 x, LONG32 y) {
	DWORD i, j;
	LONG32 X, Y;
	LONG32 MinX, MinY, MaxX, MaxY, SourceW, SourceH;
	LONG32 Width = this->Width();
	LONG32 Height = this->Height();
	SourceW = BmOps.Width();
	SourceH = BmOps.Height();
	LONG32 OffX = x;
	LONG32 OffY = y;
	if (OffX < 0) MinX = -OffX;
	else MinX = 0;
	if (OffY < 0) MinY = -OffY;
	else MinY = 0;
	if (OffX + SourceW > Width)
		MaxX = SourceW - (OffX + SourceW - Width);
	else MaxX = SourceW;
	if (OffY + SourceH > Height)
		MaxY = SourceH - (OffY + SourceH - Height);
	else MaxY = SourceH;
	for (Y = MinY; Y < MaxY; Y++) {
		for (X = MinX; X < MaxX; X++) {
			i = BmOps.GetPixelIndex(X, Y);
			j = GetPixelIndex(X + OffX, Y + OffY);
			memcpy(&PixelData[j], &BmOps.PixelData[i], 3);
		}
	}
}

void BitmapOps::ResizeBitmapFromBitmapOps(BitmapOps &BmOps) {
	DWORD Width = bmInfo.bmiHeader.biWidth;
	DWORD Height = bmInfo.bmiHeader.biHeight;
	DWORD SWidth = BmOps.Width();
	DWORD SHeight = BmOps.Height();
	DWORD x, y, i, j;
	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			i = GetPixelIndex(x, y);
			j = BmOps.GetPixelIndex((LONG32)((double)x / (double)Width * (double)SWidth), (LONG32)((double)y / (double)Height * (double)SHeight));
			PixelData[i + 2] = BmOps.PixelData[j + 2];
			PixelData[i + 1] = BmOps.PixelData[j + 1];
			PixelData[i] = BmOps.PixelData[j];
		}
	}
}

void BitmapOps::CopyAlphaBitmapFromBitmapOps(BitmapOps &BmOps, LONG32 x, LONG32 y) {
	if (BmOps.BPP() < 32) {
		CopyBitmapFromBitmapOps(BmOps, x, y);
		return;
	}
	DWORD i, j;
	LONG32 X, Y;
	LONG32 MinX, MinY, MaxX, MaxY;
	LONG32 Width = this->Width();
	LONG32 Height = this->Height();
	LONG32 SourceW = BmOps.Width();
	LONG32 SourceH = BmOps.Height();
	LONG32 OffX = x;
	LONG32 OffY = y;
	if (OffX < 0) MinX = -OffX;
	else MinX = 0;
	if (OffY < 0) MinY = -OffY;
	else MinY = 0;
	if (OffX + SourceW > Width)
		MaxX = SourceW - (OffX + SourceW - Width);
	else MaxX = SourceW;
	if (OffY + SourceH > Height)
		MaxY = SourceH - (OffY + SourceH - Height);
	else MaxY = SourceH;
	double Alpha;
	for (Y = MinY; Y < MaxY; Y++) {
		for (X = MinX; X < MaxX; X++) {
			i = BmOps.GetPixelIndex(X, Y);
			j = GetPixelIndex(X + OffX, Y + OffY);
			Alpha = (double)(BmOps.PixelData[i + 3] / 255.0);
			PixelData[j + 2] = (unsigned char)(PixelData[j + 2] * Alpha + BmOps.PixelData[i + 2]);
			PixelData[j + 1] = (unsigned char)(PixelData[j + 1] * Alpha + BmOps.PixelData[i + 1]);
			PixelData[j] = (unsigned char)(PixelData[j] * Alpha + BmOps.PixelData[i]);
		}
	}
}

void CopyAlphaBitmapFromBitmapOps(BitmapOps &BmOps, LONG32 DestX, LONG32 DestY, LONG32 SourceX, LONG32 SourceY, LONG32 SourceW, LONG32 SourceH) {
	/*if (BmOps.BPP() < 32) {
		//CopyBitmapFromBitmapOps(BmOps, DestX, DestY);
		return;
	}
	DWORD i, j;
	LONG32 X, Y;
	LONG32 MinX, MinY, MaxX, MaxY;
	LONG32 Width = this->Width();
	LONG32 Height = this->Height();
	LONG32 SourceBW = BmOps.Width();
	LONG32 SourceBH = BmOps.Height();
	LONG32 OffX = SourceX - DestX;
	LONG32 OffY = SourceY - DestY;
	MinX = DestX;
	MinY = DestY;
	MaxX = DestX + DestW;
	MaxY = DestY = DestH;
	double Alpha;
	for (Y = MinY; Y < MaxY; Y++) {
		for (X = MinX; X < MaxX; X++) {
			i = BmOps.GetPixelIndex(X + OffX, Y + OffY);
			j = GetPixelIndex(X, Y);
			Alpha = (double)(BmOps.PixelData[i + 3] / 255.0);
			PixelData[j + 2] = (unsigned char)(PixelData[j + 2] * Alpha + BmOps.PixelData[i + 2]);
			PixelData[j + 1] = (unsigned char)(PixelData[j + 1] * Alpha + BmOps.PixelData[i + 1]);
			PixelData[j] = (unsigned char)(PixelData[j] * Alpha + BmOps.PixelData[i]);
		}
	}*/
}

/*void BitmapOps::operator=(const BitmapOps &BmOps) {
	//Resize(BmOps.Width(), BmOps.Height());
	//for (DWORD i = 0; i < BmOps.UBound(); i++) {
	Resize(BmOps.bmInfo.bmiHeader.biWidth, BmOps.bmInfo.bmiHeader.biHeight);
	for (LONG32 i = 0; i < BmOps.bmInfo.bmiHeader.biWidth * BmOps.bmInfo.bmiHeader.biHeight * (BmOps.bmInfo.bmiHeader.biBitCount / 8); i++) {
		PixelData[i] = BmOps.PixelData[i];
	}
}*/



//Primative Drawing functions:

void BitmapOps::Line(LONG32 x1, LONG32 y1, LONG32 x2, LONG32 y2, COLORREF Colour) {
	LONG32 LoVal, HiVal;
	//Check that the line is not vertical:
	if (x1 == x2) {
		if (y2 > y1) {
			HiVal = y2;
			LoVal = y1;
		} else {
			HiVal = y1;
			LoVal = y2;
		}
		for (LONG32 y = LoVal; y <= HiVal; y++) {
			SetPColour(x1, y, Colour);
		}
		return;
	}
	//Determine the gradient:
	double m = (double)(y2 - y1) / (double)(x2 - x1);
	//Determine which way to draw the line:
	if (m > 1.0 || m < -1.0) { //More vertical
		//Find the x-intercept:
		//m(x - x1) = y - y1
		//x - x1 = (y - y1)/m
		//x = (y - y1)/m + x1
		//b = (0 - y1)/m + x1
		//b = -y1/m + x1
		double b = -(double)y1 / m + (double)x1;
		if (y2 > y1) {
			HiVal = y2;
			LoVal = y1;
		} else {
			HiVal = y1;
			LoVal = y2;
		}
		//y = mx + b		(b = y-int)
		//x = y/m + b		(b = x-int)
		for (LONG32 y = LoVal; y <= HiVal; y++) {
			SetPColour((LONG32)(y/m + b), y, Colour);
		}
	} else { //More horizontal
		//Find the y-intercept:
		//y - y1 = m(x - x1)
		//y = m(x - x1) + y1
		//b = m(0 - x1) + y1
		//b = -x1m + y1
		double b = -(double)x1 * m + (double)y1;
		if (x2 > x1) {
			HiVal = x2;
			LoVal = x1;
		} else {
			HiVal = x1;
			LoVal = x2;
		}
		for (LONG32 x = LoVal; x <= HiVal; x++) {
			SetPColour(x, (LONG32)(m * x + b), Colour);
		}
	}
}

void BitmapOps::Rect(LONG32 x1, LONG32 y1, LONG32 x2, LONG32 y2, COLORREF BorderColour, BOOL Transparent, COLORREF FillColour) {
	LONG32 Width = bmInfo.bmiHeader.biWidth;
	LONG32 Height = bmInfo.bmiHeader.biWidth;
	//Draw the box:
	if (!Transparent) {
		//Find filling restrictions:
		LONG32 FillX1, FillY1, FillX2, FillY2, FillT, FillI;
		if (x1 < 0) FillX1 = 0;
		else if (x1 >= Width) FillX1 = Width - 1;
		else FillX1 = x1 + 1;
		if (x2 < 0) FillX2 = 0;
		else if (x2 >= Width) FillX2 = Width - 1;
		else FillX2 = x2 - 1;
		if (y1 < 0) FillY1 = 0;
		else if (y1 >= Height) FillY1 = Height - 1;
		else FillY1 = y1 + 1;
		if (y2 < 0) FillY2 = 0;
		else if (y2 >= Height) FillY2 = Height - 1;
		else FillY2 = y2 - 1;
		if (FillX1 > FillX2) {
			FillT = FillX2; FillX2 = FillX1; FillX1 = FillT;
		}
		if (FillY1 > FillY2) {
			FillT = FillY2; FillY2 = FillY1; FillY1 = FillT;
		}
		//Fill in the middle:
		if (FillColour) {
			for (LONG32 CounterY = FillY1; CounterY <= FillY2; CounterY++) {
				for (LONG32 CounterX = FillX1; CounterX <= FillX2; CounterX++) {
					SetPColour(CounterX, CounterY, FillColour);
				}
			}
		} else { //Black shortcut:
			for (LONG32 CounterY = FillY1; CounterY <= FillY2; CounterY++) {
				FillI = GetPixelIndex(FillX1, CounterY);
				memset(&PixelData[FillI], 0, (FillX2 - FillX1 + 1) * BPP() / 8);
			}
		}
	}
	//Draw the outline:
	Line(x1, y1, x1, y2, BorderColour); //left
	Line(x1, y1, x2, y1, BorderColour); //top
	Line(x2, y1, x2, y2, BorderColour); //right
	Line(x1, y2, x2, y2, BorderColour); //bottom
}

void BitmapOps::Ellipse(LONG32 x, LONG32 y, LONG32 xr, LONG32 yr, COLORREF Colour, double Start, double End) {
	double Theta = Start;
	double x1, y1;//, m;
	//double n = sqrt(pow((double)yr / (double)xr, 2)); //Cross-over grad
	do {
		x1 = xr * cos(Theta);
		y1 = yr * sin(Theta);
		SetPColour((LONG32)(x + x1), (LONG32)(y + y1), Colour);
		//Update Theta:
		//d = sqrt(pow(x1, 2) + pow(y1, 2));
		/*m = sqrt(pow(y1 / x1, 2));
		if (End > Start) {
			if (m > n) { //x more significant
				if (y1 > 0) {
					Theta = acos(sqrt(pow(x1 - 1.0, 2)) / (sqrt(pow(x1 - 1.0, 2)) + ((double)yr * (1.0 - sqrt(pow(x1 - 1.0, 2)) / (double)xr))));
				} else {
					Theta += 0.01;
				}
			} else { //y more significant
				Theta += 0.01;
			}
		} else {
			Theta -= 0.01;
		}*/
		if (End > Start) {
			Theta += 0.01;
		} else {
			Theta -= 0.01;
		}
	} while (Theta <= End);
}
