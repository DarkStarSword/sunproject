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
#include "MultiSoftGControls.h"
#include "BitmapOps.h"

//Window Procedure Prototypes:
//MultiSoftGButton:
LRESULT CALLBACK GButtonProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
//MultiSoftGTitle:
LRESULT CALLBACK GTitleProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
//MultiSoftGToolTip:
LRESULT CALLBACK GTTipProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
//MultiSoftGWndCon:
LRESULT CALLBACK GWndConProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
//MultiSoftGVScroll:
LRESULT CALLBACK GVScrolProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
//MultiSoftGHScroll:
LRESULT CALLBACK GHScrolProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
//MultiSoftGHProgress:
LRESULT CALLBACK GHProgressProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);



enum WindowStates {Off, Over, DownNotOver, Down};

HINSTANCE hInstance;
HWND ToolTipHWnd;


LONG32 NumControls = 0;
HWND *CtrlHandles = NULL;
COLORREF *CtrlClrFace = NULL;
COLORREF *CtrlClrDarkShadow = NULL;
COLORREF *CtrlClrShadow = NULL;
COLORREF *CtrlClrExtraHighlight = NULL;
COLORREF *CtrlClrHighlight = NULL;
COLORREF *CtrlClrText = NULL;
COLORREF *CtrlClrActive = NULL;
COLORREF *CtrlClrDefault = NULL;
COLORREF *CtrlClrDisabledFace = NULL;
COLORREF *CtrlClrDisabledDarkShadow = NULL;
COLORREF *CtrlClrDisabledShadow = NULL;
COLORREF *CtrlClrDisabledExtraHighlight = NULL;
COLORREF *CtrlClrDisabledHighlight = NULL;
COLORREF *CtrlClrDisabledText = NULL;
WindowStates *CtrlStates = NULL;
BOOL *CtrlToolTipState = NULL;
LPVOID *CtrlBmpOff = NULL;
LPVOID *CtrlBmpOver = NULL;
LPVOID *CtrlBmpOn = NULL;
LPVOID *CtrlBmpDisabled = NULL;
//Title Bar:
LPVOID *CtrlTitleBmp = NULL;
DWORD *CtrlTitleHeight = NULL;
DWORD *CtrlMinWidth = NULL;
HWND *CtrlCloseHandles = NULL;
HWND *CtrlMaxHandles = NULL;
HWND *CtrlRestoreHandles = NULL;
HWND *CtrlMinHandles = NULL;
DWORD *CtrlLastClickTickCount = NULL;
//Window Container:
LONG32 *CtrlStartX = NULL;
LONG32 *CtrlStartY = NULL;
LONG32 *CtrlTargetX = NULL;
LONG32 *CtrlTargetY = NULL;
DWORD *CtrlCurFrame = NULL;
DWORD *CtrlTotalFrames = NULL;
//Scrollbars:
LONG32 *CtrlMin = NULL;
LONG32 *CtrlMax = NULL;
LONG32 *CtrlVal = NULL;
LONG32 *CtrlSmUpdate = NULL;
LONG32 *CtrlLgUpdate = NULL;



void MultiSoftGControlsInit(HINSTANCE hInst) {
	//MessageBox(NULL, "Contructing", "", MB_OK);
	hInstance = hInst;
	//Create the window classes:
	//MultiSoftGButton:
	WNDCLASSEX GButtonWndClass = {0};
	GButtonWndClass.cbSize = sizeof(WNDCLASSEX);
	GButtonWndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	GButtonWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	GButtonWndClass.hInstance = hInstance;
	GButtonWndClass.lpfnWndProc = GButtonProc;
	GButtonWndClass.lpszClassName = "MultiSoftGButton";
	GButtonWndClass.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&GButtonWndClass);
	//MultiSoftGTitle:
	WNDCLASSEX GTitleWndClass = {0};
	GTitleWndClass.cbSize = sizeof(WNDCLASSEX);
	GTitleWndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	GTitleWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	GTitleWndClass.hInstance = hInstance;
	GTitleWndClass.lpfnWndProc = GTitleProc;
	GTitleWndClass.lpszClassName = "MultiSoftGTitle";
	GTitleWndClass.style = CS_SAVEBITS;//CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&GTitleWndClass);
	//MultiSoftGWndCon:
	WNDCLASSEX GWndConWndClass = {0};
	GWndConWndClass.cbSize = sizeof(WNDCLASSEX);
	GWndConWndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	GWndConWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	GWndConWndClass.hInstance = hInstance;
	GWndConWndClass.lpfnWndProc = GWndConProc;
	GWndConWndClass.lpszClassName = "MultiSoftGWndCon";
	GWndConWndClass.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&GWndConWndClass);
	//MultiSoftGToolTip:
	WNDCLASSEX GTTipWndClass = {0};
	GTTipWndClass.cbSize = sizeof(WNDCLASSEX);
	GTTipWndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	GTTipWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	GTTipWndClass.hInstance = hInstance;
	GTTipWndClass.lpfnWndProc = GTTipProc;
	GTTipWndClass.lpszClassName = "MultiSoftGToolTip";
	GTTipWndClass.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&GTTipWndClass);
	//MultiSoftGVScroll:
	WNDCLASSEX GVScrollClass = {0};
	GVScrollClass.cbSize = sizeof(WNDCLASSEX);
	GVScrollClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	GVScrollClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	GVScrollClass.hInstance = hInstance;
	GVScrollClass.lpfnWndProc = GVScrolProc;
	GVScrollClass.lpszClassName = "MultiSoftGVScroll";
	GVScrollClass.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&GVScrollClass);
	//MultiSoftGHScroll:
	WNDCLASSEX GHScrollClass = {0};
	GHScrollClass.cbSize = sizeof(WNDCLASSEX);
	GHScrollClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	GHScrollClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	GHScrollClass.hInstance = hInstance;
	GHScrollClass.lpfnWndProc = GHScrolProc;
	GHScrollClass.lpszClassName = "MultiSoftGHScroll";
	GHScrollClass.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&GHScrollClass);
	//MultiSoftGHProgress:
	WNDCLASSEX GHProgressClass = {0};
	GHProgressClass.cbSize = sizeof(WNDCLASSEX);
	GHProgressClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	GHProgressClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	GHProgressClass.hInstance = hInstance;
	GHProgressClass.lpfnWndProc = GHProgressProc;
	GHProgressClass.lpszClassName = "MultiSoftGHProgress";
	GHProgressClass.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&GHProgressClass);
	//Tooltip window:
	ToolTipHWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, "MultiSoftGToolTip", NULL, WS_POPUP, 0, 0, 10, 10, NULL, NULL, hInstance, NULL);

	CtrlHandles = new HWND[NumControls];
	CtrlClrFace = new COLORREF[NumControls];
	CtrlClrDarkShadow = new COLORREF[NumControls];
	CtrlClrShadow = new COLORREF[NumControls];
	CtrlClrExtraHighlight = new COLORREF[NumControls];
	CtrlClrHighlight = new COLORREF[NumControls];
	CtrlClrText = new COLORREF[NumControls];
	CtrlClrActive = new COLORREF[NumControls];
	CtrlClrDefault = new COLORREF[NumControls];
	CtrlClrDisabledFace = new COLORREF[NumControls];
	CtrlClrDisabledDarkShadow = new COLORREF[NumControls];
	CtrlClrDisabledShadow = new COLORREF[NumControls];
	CtrlClrDisabledExtraHighlight = new COLORREF[NumControls];
	CtrlClrDisabledHighlight = new COLORREF[NumControls];
	CtrlClrDisabledText = new COLORREF[NumControls];
	CtrlStates = new WindowStates[NumControls];
	CtrlToolTipState = new BOOL[NumControls];
	CtrlBmpOff = new LPVOID[NumControls];
	CtrlBmpOver = new LPVOID[NumControls];
	CtrlBmpOn = new LPVOID[NumControls];
	CtrlBmpDisabled = new LPVOID[NumControls];
	//Title bar:
	CtrlTitleBmp = new LPVOID[NumControls];
	CtrlTitleHeight = new DWORD[NumControls];
	CtrlMinWidth = new DWORD[NumControls];
	CtrlCloseHandles = new HWND[NumControls];
	CtrlMaxHandles = new HWND[NumControls];
	CtrlRestoreHandles = new HWND[NumControls];
	CtrlMinHandles = new HWND[NumControls];
	CtrlLastClickTickCount = new DWORD[NumControls];
	//Window Container:
	CtrlStartX = new LONG32[NumControls];
	CtrlStartY = new LONG32[NumControls];
	CtrlTargetX = new LONG32[NumControls];
	CtrlTargetY = new LONG32[NumControls];
	CtrlCurFrame = new DWORD[NumControls];
	CtrlTotalFrames = new DWORD[NumControls];
	//Scrollbars:
	CtrlMin = new LONG32[NumControls];
	CtrlMax = new LONG32[NumControls];
	CtrlMax = new LONG32[NumControls];
	CtrlSmUpdate = new LONG32[NumControls];
	CtrlLgUpdate = new LONG32[NumControls];
}

void MultiSoftGControlsDeInit() {
	//MessageBox(NULL, "Destructing", "", MB_OK);
	//Destroy the window classes:
	SendMessage(ToolTipHWnd, GTTM_HIDETIP, 0, 0);
	CloseWindow(ToolTipHWnd);
	UnregisterClass("MultiSoftGHProgress", hInstance);
	UnregisterClass("MultiSoftGHScroll", hInstance);
	UnregisterClass("MultiSoftGVScroll", hInstance);
	UnregisterClass("MultiSoftGToolTip", hInstance);
	UnregisterClass("MultiSoftGWndCon", hInstance);
	UnregisterClass("MultiSoftGTitle", hInstance);
	UnregisterClass("MultiSoftGButton", hInstance);
	if (CtrlHandles) delete [] CtrlHandles; CtrlHandles = NULL;
	if (CtrlClrFace) delete [] CtrlClrFace; CtrlClrFace = NULL;
	if (CtrlClrDarkShadow) delete [] CtrlClrDarkShadow; CtrlClrDarkShadow = NULL;
	if (CtrlClrShadow) delete [] CtrlClrShadow; CtrlClrShadow = NULL;
	if (CtrlClrExtraHighlight) delete [] CtrlClrExtraHighlight; CtrlClrExtraHighlight = NULL;
	if (CtrlClrHighlight) delete [] CtrlClrHighlight; CtrlClrHighlight = NULL;
	if (CtrlClrText) delete [] CtrlClrText; CtrlClrText = NULL;
	if (CtrlClrActive) delete [] CtrlClrActive; CtrlClrActive = NULL;
	if (CtrlClrDefault) delete [] CtrlClrDefault; CtrlClrDefault = NULL;
	if (CtrlClrDisabledFace) delete [] CtrlClrDisabledFace; CtrlClrDisabledFace = NULL;
	if (CtrlClrDisabledDarkShadow) delete [] CtrlClrDisabledDarkShadow; CtrlClrDisabledDarkShadow = NULL;
	if (CtrlClrDisabledShadow) delete [] CtrlClrDisabledShadow; CtrlClrDisabledShadow = NULL;
	if (CtrlClrDisabledExtraHighlight) delete [] CtrlClrDisabledExtraHighlight; CtrlClrDisabledExtraHighlight = NULL;
	if (CtrlClrDisabledHighlight) delete [] CtrlClrDisabledHighlight; CtrlClrDisabledHighlight = NULL;
	if (CtrlClrDisabledText) delete [] CtrlClrDisabledText; CtrlClrDisabledText = NULL;
	if (CtrlStates) delete [] CtrlStates; CtrlStates = NULL;
	if (CtrlToolTipState) delete [] CtrlToolTipState; CtrlToolTipState = NULL;
	for (int i = 0; i < NumControls; ++i) {
		if (CtrlBmpOff[i]) delete ((BitmapOps*)CtrlBmpOff[i]);
	}
	if (CtrlBmpOff) delete [] CtrlBmpOff; CtrlBmpOff = NULL;
	for (i = 0; i < NumControls; ++i) {
		if (CtrlBmpOver[i]) delete ((BitmapOps*)CtrlBmpOver[i]);
	}
	if (CtrlBmpOver) delete [] CtrlBmpOver; CtrlBmpOver = NULL;
	for (i = 0; i < NumControls; ++i) {
		if (CtrlBmpOn[i]) delete ((BitmapOps*)CtrlBmpOn[i]);
	}
	if (CtrlBmpOn) delete [] CtrlBmpOn; CtrlBmpOn = NULL;
	for (i = 0; i < NumControls; ++i) {
		if (CtrlBmpDisabled[i]) delete ((BitmapOps*)CtrlBmpDisabled[i]);
	}
	if (CtrlBmpDisabled) delete [] CtrlBmpDisabled; CtrlBmpDisabled = NULL;
	//Title bar:
	for (i = 0; i < NumControls; ++i) {
		if (CtrlTitleBmp[i]) delete ((BitmapOps*)CtrlTitleBmp[i]);
	}
	if (CtrlTitleBmp) delete [] CtrlTitleBmp; CtrlTitleBmp = NULL;
	if (CtrlTitleHeight) delete [] CtrlTitleHeight; CtrlTitleHeight = NULL;
	if (CtrlMinWidth) delete [] CtrlMinWidth; CtrlMinWidth = NULL;
	if (CtrlCloseHandles) delete [] CtrlCloseHandles; CtrlCloseHandles = NULL;
	if (CtrlMaxHandles) delete [] CtrlMaxHandles; CtrlMaxHandles = NULL;
	if (CtrlRestoreHandles) delete [] CtrlRestoreHandles; CtrlRestoreHandles = NULL;
	if (CtrlMinHandles) delete [] CtrlMinHandles; CtrlMinHandles = NULL;
	if (CtrlLastClickTickCount) delete [] CtrlLastClickTickCount; CtrlLastClickTickCount = NULL;
	//Window Container:
	if (CtrlStartX) delete [] CtrlStartX; CtrlStartX = NULL;
	if (CtrlStartY) delete [] CtrlStartY; CtrlStartY = NULL;
	if (CtrlTargetX) delete [] CtrlTargetX; CtrlTargetX = NULL;
	if (CtrlTargetY) delete [] CtrlTargetY; CtrlTargetY = NULL;
	if (CtrlCurFrame) delete [] CtrlCurFrame; CtrlCurFrame = NULL;
	if (CtrlTotalFrames) delete [] CtrlTotalFrames; CtrlTotalFrames = NULL;
	//Scrollbars:
	if (CtrlMin) delete [] CtrlMin; CtrlMin = NULL;
	if (CtrlMax) delete [] CtrlMax; CtrlMax = NULL;
	if (CtrlVal) delete [] CtrlVal; CtrlVal = NULL;
	if (CtrlSmUpdate) delete [] CtrlSmUpdate; CtrlSmUpdate = NULL;
	if (CtrlLgUpdate) delete [] CtrlLgUpdate; CtrlLgUpdate = NULL;
}

/***/
#define ADDGCONTROLMEMBER(DATATYPE, TEMPARRAYNAME, SIMARRAYNAME, NEWVALUE)					\
	TEMPARRAYNAME = new DATATYPE[NumControls];												\
if (!TEMPARRAYNAME) {																		\
		NumControls--;																		\
		return false;																		\
	}																						\
	if (SIMARRAYNAME) memcpy(TEMPARRAYNAME, SIMARRAYNAME, NumControls * sizeof(DATATYPE));	\
	TEMPARRAYNAME[NumControls - 1] = NEWVALUE;												\
	if (SIMARRAYNAME) delete [] SIMARRAYNAME;												\
	SIMARRAYNAME = TEMPARRAYNAME;
/***/

BOOL ControlCreated(HWND hWnd) {
	NumControls++;
	HWND *TempHWndArray;
	COLORREF *TempColourArray;
	WindowStates *TempStateArray;
	BOOL *TempBoolArray;
	DWORD *TempDwordArray;
	LONG32 *TempLongArray;
	LPVOID *TempLPVoidArray;
	//HWND:
	ADDGCONTROLMEMBER(HWND, TempHWndArray, CtrlHandles, hWnd)
	//Colours:
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrFace, 0xcc6600)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrDarkShadow, 0x0)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrShadow, 0xcc3300)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrExtraHighlight, 0xcc6600)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrHighlight, 0xcc9900)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrText, 0xcccccc)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrActive, 0x669900)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrDefault, 0x0)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrDisabledFace, 0xAAAAAA)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrDisabledDarkShadow, 0x333333)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrDisabledShadow, 0x999999)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrDisabledExtraHighlight, 0xAAAAAA)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrDisabledHighlight, 0xCCCCCC)
	ADDGCONTROLMEMBER(COLORREF, TempColourArray, CtrlClrDisabledText, 0xFFFFFF)
	//Window State:
	ADDGCONTROLMEMBER(WindowStates, TempStateArray, CtrlStates, Off)
	//Tool tip state:
	ADDGCONTROLMEMBER(BOOL, TempBoolArray, CtrlToolTipState, FALSE);
	//Bitmaps:
	ADDGCONTROLMEMBER(LPVOID, TempLPVoidArray, CtrlBmpOff, NULL)
	ADDGCONTROLMEMBER(LPVOID, TempLPVoidArray, CtrlBmpOver, NULL)
	ADDGCONTROLMEMBER(LPVOID, TempLPVoidArray, CtrlBmpOn, NULL)
	ADDGCONTROLMEMBER(LPVOID, TempLPVoidArray, CtrlBmpDisabled, NULL)
	//TITLE BAR:
	//Image:
	ADDGCONTROLMEMBER(LPVOID, TempLPVoidArray, CtrlTitleBmp, NULL);//new BitmapOps(1,1));
	//Height:
	ADDGCONTROLMEMBER(DWORD, TempDwordArray, CtrlTitleHeight, 20)
	//Min window width:
	ADDGCONTROLMEMBER(DWORD, TempDwordArray, CtrlMinWidth, 100);
	//Button Window handles:
	ADDGCONTROLMEMBER(HWND, TempHWndArray, CtrlCloseHandles, NULL)
	ADDGCONTROLMEMBER(HWND, TempHWndArray, CtrlMaxHandles, NULL)
	ADDGCONTROLMEMBER(HWND, TempHWndArray, CtrlRestoreHandles, NULL);
	ADDGCONTROLMEMBER(HWND, TempHWndArray, CtrlMinHandles, NULL)
	//Last click tick count:
	ADDGCONTROLMEMBER(DWORD, TempDwordArray, CtrlLastClickTickCount, 0)
	//WINDOW CONTAINER:
	ADDGCONTROLMEMBER(LONG32, TempLongArray, CtrlStartX, 0);
	ADDGCONTROLMEMBER(LONG32, TempLongArray, CtrlStartY, 0);
	ADDGCONTROLMEMBER(LONG32, TempLongArray, CtrlTargetX, 0);
	ADDGCONTROLMEMBER(LONG32, TempLongArray, CtrlTargetY, 0);
	ADDGCONTROLMEMBER(DWORD, TempDwordArray, CtrlCurFrame, 0);
	ADDGCONTROLMEMBER(DWORD, TempDwordArray, CtrlTotalFrames, 0);
	//SCROLLBARS:
	ADDGCONTROLMEMBER(LONG32, TempLongArray, CtrlMin, 0);
	ADDGCONTROLMEMBER(LONG32, TempLongArray, CtrlMax, 100);
	ADDGCONTROLMEMBER(LONG32, TempLongArray, CtrlVal, 0);
	ADDGCONTROLMEMBER(LONG32, TempLongArray, CtrlSmUpdate, 1);
	ADDGCONTROLMEMBER(LONG32, TempLongArray, CtrlLgUpdate, 5);

	return true;
}


template <class TMPDataType>
TMPDataType GControlProperty(HWND hWnd, TMPDataType *SimArray) {
	for (int i = 0; i < NumControls; ++i) {
		if (CtrlHandles[i] == hWnd) return SimArray[i];
	}
	return (TMPDataType)NULL;
}
template <class TMPDataType>
void GControlProperty(HWND hWnd, TMPDataType NewValue, TMPDataType *SimArray) {
	for (int i = 0; i < NumControls; ++i) {
		if (CtrlHandles[i] == hWnd) {SimArray[i] = NewValue; return;}
	}
}

void ControlDestroyed(HWND hWnd) {
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
	//FIX THIS (MEM EATING DANGER)
	//If it isn't, all the mem will still be de-allocated when the controls are de-initialised
	//MessageBox(NULL, "Control Destroyed", "", MB_OK);
}




/*
This function is used to interpolate
a value between two values using Cosine.

Modified from Hugo Elias' Perlin Noise Tutorial
*/
//   b-----c
//   0--x--1
double MultiSoftGInterpolateCoSine(double a, double b, double x) {
	if (a == b) return a;
	double ft = x * 3.141592653589793;
	double f = (1.0 - cos(ft)) * 0.5;

	return a * (1.0 - f) + b * f;
}



/*
This function draws a border on the passed in DC with the passed
in colours. It also optioanlly fills it in with the face colour.
*/
void MultiSoftDrawBorder(HDC hDc, RECT& Coords, BOOL DrawFace = FALSE, COLORREF clrDarkShadow = 0x0, COLORREF clrShadow = 0xCC3300, COLORREF clrExtraHighlight = 0xCC6600, COLORREF clrHighlight = 0xCC9900, COLORREF clrFace = 0xCC6600) {
	HBRUSH hBrush, hOldBrush;
	HPEN hPen, hOldPen;
	
	//Borders:
	if (DrawFace) {
		//Face:
		hBrush = CreateSolidBrush(clrFace);
		hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
		hOldPen = (HPEN)GetStockObject(NULL_PEN);
		Rectangle(hDc, Coords.left + 1, Coords.top + 1, Coords.right - 1, Coords.bottom - 1);
		SelectObject(hDc, hOldPen);
		SelectObject(hDc, hOldBrush);
		DeleteObject(hBrush);
	}
	//Highlight:
	hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	hPen = CreatePen(PS_SOLID, 1, clrHighlight);
	hOldPen = (HPEN)SelectObject(hDc, hPen);
	
	MoveToEx(hDc, Coords.right - 3, Coords.top + 1, NULL);
	LineTo(hDc, Coords.left + 1, Coords.top + 1);
	LineTo(hDc, Coords.left + 1, Coords.bottom - 2);

	SelectObject(hDc, hOldPen);
	DeleteObject(hPen);
	
	//Shadow:
	hPen = CreatePen(PS_SOLID, 1, clrShadow);
	hOldPen = (HPEN)SelectObject(hDc, hPen);
	SelectObject(hDc, hPen);
	
	LineTo(hDc, Coords.right - 2, Coords.bottom - 2);
	LineTo(hDc, Coords.right - 2, Coords.top);
	
	SelectObject(hDc, hOldPen);
	DeleteObject(hPen);

	//3D Highlight:
	hPen = CreatePen(PS_SOLID, 1, clrExtraHighlight);
	hOldPen = (HPEN)SelectObject(hDc, hPen);
	SelectObject(hDc, hPen);
	
	LineTo(hDc, Coords.left, Coords.top);
	LineTo(hDc, Coords.left, Coords.bottom - 1);

	SelectObject(hDc, hOldPen);
	DeleteObject(hPen);
	
	//Dark Shadow:
	hPen = CreatePen(PS_SOLID, 1, clrDarkShadow);
	hOldPen = (HPEN)SelectObject(hDc, hPen);
	SelectObject(hDc, hPen);
	
	LineTo(hDc, Coords.right - 1, Coords.bottom - 1);
	LineTo(hDc, Coords.right - 1, Coords.top - 1);

	SelectObject(hDc, hOldPen);
	DeleteObject(hPen);

	SelectObject(hDc, hOldBrush);
}




void TransBlt (HDC hDcDest, HBITMAP BmpSrc, LONG32 XDest, LONG32 YDest, COLORREF TransColour) {
	//Get bmp dimentions:
	BITMAP BmpSize;
	GetObject(BmpSrc, sizeof(BITMAP), &BmpSize);
	//DC's to hold storage:
	HDC SrcDC = CreateCompatibleDC(hDcDest);
	HDC SaveDC = CreateCompatibleDC(hDcDest);
	HDC MaskDC = CreateCompatibleDC(hDcDest);
	HDC InvDC = CreateCompatibleDC(hDcDest);
	HDC ResultDC = CreateCompatibleDC(hDcDest);
	//Create monochrome bmps for mask-related bmps:
	HBITMAP MaskBmp = CreateBitmap(BmpSize.bmWidth, BmpSize.bmHeight, 1, 1, NULL);
	HBITMAP InvBmp = CreateBitmap(BmpSize.bmWidth, BmpSize.bmHeight, 1, 1, NULL);
	//Create colour bmps for final & copy of source:
	HBITMAP ResultBmp = CreateCompatibleBitmap(hDcDest, BmpSize.bmWidth, BmpSize.bmHeight);
	HBITMAP SaveBmp = CreateCompatibleBitmap(hDcDest, BmpSize.bmWidth, BmpSize.bmHeight);
	//Select BMPs into DCs:
	HBITMAP OldSrcBmp = (HBITMAP)SelectObject(SrcDC, BmpSrc);
	HBITMAP OldSaveBmp = (HBITMAP)SelectObject(SaveDC, SaveBmp);
	HBITMAP OldMaskBmp = (HBITMAP)SelectObject(MaskDC, MaskBmp);
	HBITMAP OldInvBmp = (HBITMAP)SelectObject(InvDC, InvBmp);
	HBITMAP OldDestBmp = (HBITMAP)SelectObject(ResultDC, ResultBmp);
	//Backup source bmp to later restore:
	BitBlt(SaveDC, 0, 0, BmpSize.bmWidth, BmpSize.bmHeight, SrcDC, 0, 0, SRCCOPY);
	//Create mask (Set bkgrnd col to trans col):
	COLORREF OrigColour = SetBkColor(SrcDC, TransColour);
	BitBlt(MaskDC, 0, 0, BmpSize.bmWidth, BmpSize.bmHeight, SrcDC, 0, 0, SRCCOPY);
	SetBkColor(SrcDC, OrigColour);
	//Create inverse of mask to & with source and combine with background:
	BitBlt(InvDC, 0, 0, BmpSize.bmWidth, BmpSize.bmHeight, MaskDC, 0, 0, NOTSRCCOPY);
	//Copy bckgrnd bmp to result & create final trans bmp:
	BitBlt(ResultDC, 0, 0, BmpSize.bmWidth, BmpSize.bmHeight, hDcDest, XDest, YDest, SRCCOPY);
	//AND mask bitmap w/ result DC to punch hole in the background by
	//painting black area for non-transparent portion of source bitmap.
	BitBlt(ResultDC, 0, 0, BmpSize.bmWidth, BmpSize.bmHeight, MaskDC, 0, 0, SRCAND);
	//AND inverse mask w/ source bitmap to turn off bits associated
	//with transparent area of source bitmap by making it black.
	BitBlt(SrcDC, 0, 0, BmpSize.bmWidth, BmpSize.bmHeight, InvDC, 0, 0, SRCAND);
	//XOR result with src bmp to make bkgrnd show thru:
	BitBlt(ResultDC, 0, 0, BmpSize.bmWidth, BmpSize.bmHeight, SrcDC, 0, 0, SRCPAINT);
	//Display trans bmp on bkgrnd:
	BitBlt(hDcDest, XDest, YDest, BmpSize.bmWidth, BmpSize.bmHeight, ResultDC, 0, 0, SRCCOPY);
	//Restore bkup of bmp:
	BitBlt(SrcDC, 0, 0, BmpSize.bmWidth, BmpSize.bmHeight, SaveDC, 0, 0, SRCCOPY);
	//Unselect objects:
	SelectObject(SrcDC, OldSrcBmp);
	SelectObject(SaveDC, OldSaveBmp);
	SelectObject(ResultDC, OldDestBmp);
	SelectObject(MaskDC, OldMaskBmp);
	SelectObject(InvDC, OldInvBmp);
	//Delete objects:
	DeleteObject(SaveBmp);
	DeleteObject(MaskBmp);
	DeleteObject(InvBmp);
	DeleteObject(ResultBmp);
	//Delete mem DCs:
	DeleteDC(SrcDC);
	DeleteDC(SaveDC);
	DeleteDC(InvDC);
	DeleteDC(MaskDC);
	DeleteDC(ResultDC);
}





//Window Procedures:





/*****************************************************************/
/***************************BUTTON********************************/
/*****************************************************************/





LRESULT CALLBACK GButtonProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {	
	WindowStates WndState = GControlProperty(hWnd, CtrlStates);
	//Only one tool tip window should exist, so use statics:
	static LONG32 TimerInitX = 0;
	static LONG32 TimerInitY = 0;
	static HWND TimerHWnd = NULL;
	switch(Message) {
	case WM_CREATE:
		if (!ControlCreated(hWnd)) return -1;
		return 0;
	case WM_ENABLE:
		SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GBN_ENABLECHANGED), (LPARAM)hWnd);
	case WM_ERASEBKGND:
		{
			int WindowIsActive = 0;
			
			COLORREF clrFace, clrDarkShadow, clrShadow, clrExtraHighlight, clrHighlight, clrText, clrActive, clrDefault;
			LPVOID BmpOff, BmpOver, BmpOn;
			if (IsWindowEnabled(hWnd)) {
				if (hWnd == GetFocus()) WindowIsActive = 1 && (GetWindowLong(hWnd, GWL_STYLE) & WS_TABSTOP);
				clrFace = GControlProperty(hWnd, CtrlClrFace);
				clrDarkShadow = GControlProperty(hWnd, CtrlClrDarkShadow);
				clrShadow = GControlProperty(hWnd, CtrlClrShadow);
				clrExtraHighlight = GControlProperty(hWnd, CtrlClrExtraHighlight);
				clrHighlight = GControlProperty(hWnd, CtrlClrHighlight);
				clrText = GControlProperty(hWnd, CtrlClrText);
				clrActive = GControlProperty(hWnd, CtrlClrActive);
				clrDefault = GControlProperty(hWnd, CtrlClrDefault);
				BmpOff = GControlProperty(hWnd, CtrlBmpOff);
				BmpOver = GControlProperty(hWnd, CtrlBmpOver);
				BmpOn = GControlProperty(hWnd, CtrlBmpOn);
			} else {
				clrFace = GControlProperty(hWnd, CtrlClrDisabledFace);
				clrDarkShadow = GControlProperty(hWnd, CtrlClrDisabledDarkShadow);
				clrShadow = GControlProperty(hWnd, CtrlClrDisabledShadow);
				clrExtraHighlight = GControlProperty(hWnd, CtrlClrDisabledExtraHighlight);
				clrHighlight = GControlProperty(hWnd, CtrlClrDisabledHighlight);
				clrText = GControlProperty(hWnd, CtrlClrDisabledText);
				clrActive = 0;
				clrDefault = 0;
				BmpOff = GControlProperty(hWnd, CtrlBmpDisabled);
				BmpOver = BmpOff;
				BmpOn = BmpOff;
			}

			HDC hDc = GetDC(hWnd);

			RECT ClientRect = {0};
			GetClientRect(hWnd, &ClientRect);
			
			switch(WndState) {
			case DownNotOver:
			case Over:
				{
					if (BmpOver) {
						((BitmapOps*)BmpOver)->CopyImageArrayToDC(hWnd, hDc);
						break;
					}
				}
			case Off:
				{
					if (BmpOff) {
						((BitmapOps*)BmpOff)->CopyImageArrayToDC(hWnd, hDc);
						break;
					} else {
						//Face:
						HBRUSH hBrush = CreateSolidBrush(clrFace);
						HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
						HPEN hPen = (HPEN)GetStockObject(NULL_PEN);
						HPEN hOldPen = (HPEN)SelectObject(hDc, hPen);

						Rectangle(hDc, ClientRect.left + 2, ClientRect.top + 2, ClientRect.right - 1, ClientRect.bottom - 1);

						//Caption:
						SetTextColor(hDc, clrText);
						SetBkMode(hDc, 0);
						char txtCaption[128];
						GetWindowText(hWnd, txtCaption, 128);
						SIZE txtSize;
						GetTextExtentPoint32(hDc, txtCaption, strlen(txtCaption), &txtSize);
						TextOut(hDc, (ClientRect.left + ClientRect.right) / 2 - txtSize.cx / 2, (ClientRect.top + ClientRect.bottom) / 2 - txtSize.cy / 2, txtCaption, strlen(txtCaption));

						SelectObject(hDc, hOldPen);
						SelectObject(hDc, hOldBrush);
						DeleteObject(hBrush);
						
						//Borders:
						RECT Coords = {ClientRect.left + WindowIsActive, ClientRect.top + WindowIsActive, ClientRect.right - WindowIsActive, ClientRect.bottom - WindowIsActive};
						MultiSoftDrawBorder(hDc, Coords, FALSE, clrDarkShadow, clrShadow, clrExtraHighlight, clrHighlight);

						//Active:
						if (WindowIsActive) {
							//IDEA: IF CONTROL IS DEFAULT (AND POSSIBLE), BUT NOT FOCUSED, USE 'DEFAULT' COLOUR INSTEAD OF ACTIVE COLOUR
							hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
							hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
							hPen = CreatePen(PS_SOLID, 1, clrActive);
							hOldPen = (HPEN)SelectObject(hDc, hPen);
							SelectObject(hDc, hPen);

							Rectangle(hDc, ClientRect.left, ClientRect.top, ClientRect.right, ClientRect.bottom);

							SelectObject(hDc, hOldBrush);
							SelectObject(hDc, hOldPen);
							DeleteObject(hPen);
						}
						break;
					}
				}
			case Down:
				{
					if (BmpOn) {
						((BitmapOps*)BmpOn)->CopyImageArrayToDC(hWnd, hDc);
						break;
					} else {
						HPEN hOldPen, hPen;
						HBRUSH hOldBrush, hBrush;

						hBrush = CreateSolidBrush(clrFace);
						hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
						hPen = (HPEN)GetStockObject(NULL_PEN);
						hOldPen = (HPEN)SelectObject(hDc, hPen);

						//Rectangle:
						Rectangle(hDc, ClientRect.left + 1 + WindowIsActive, ClientRect.top + 1 + WindowIsActive, ClientRect.right - WindowIsActive, ClientRect.bottom - WindowIsActive);

						//Caption:
						SetTextColor(hDc, clrText);
						SetBkMode(hDc, 0);
						char txtCaption[128];
						GetWindowText(hWnd, txtCaption, 128);
						SIZE txtSize;
						GetTextExtentPoint32(hDc, txtCaption, strlen(txtCaption), &txtSize);
						TextOut(hDc, (ClientRect.left + ClientRect.right) / 2 - txtSize.cx / 2 + 1, (ClientRect.top + ClientRect.bottom) / 2 - txtSize.cy / 2 + 1, txtCaption, strlen(txtCaption));

						SelectObject(hDc, hOldPen);
						hPen = CreatePen(PS_SOLID, 1, clrShadow);
						hOldPen = (HPEN)SelectObject(hDc, hPen);
						SelectObject(hDc, hOldBrush);
						DeleteObject(hBrush);
						hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
						hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);

						//Rectangle:
						Rectangle(hDc, ClientRect.left + WindowIsActive, ClientRect.top + WindowIsActive, ClientRect.right - WindowIsActive, ClientRect.bottom - WindowIsActive);

						//Active:
						if (WindowIsActive) {
							SelectObject(hDc, hOldBrush);
							hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
							hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
							SelectObject(hDc, hOldPen);
							DeleteObject(hPen);
							hPen = CreatePen(PS_SOLID, 1, clrActive);
							hOldPen = (HPEN)SelectObject(hDc, hPen);
							SelectObject(hDc, hPen);

							Rectangle(hDc, ClientRect.left, ClientRect.top, ClientRect.right, ClientRect.bottom);
						}

						//Clean up:
						SelectObject(hDc, hOldPen);
						DeleteObject(hPen);
						SelectObject(hDc, hOldBrush);
						break;
					}
				}
			}
			ReleaseDC(hWnd, hDc);
		}
		return 0;
	case WM_SETFOCUS:
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_KILLFOCUS:
		if (WndState == Down)
			WndState = Over;
		else
			WndState = Off;
		GControlProperty(hWnd, WndState, CtrlStates);
		InvalidateRect(hWnd, NULL, TRUE);
		if (GetCapture() == hWnd)
			ReleaseCapture();
		break;
	case WM_TIMER:
		{
			switch (wParam) {
			case 1: //Mouse hover
				{
					KillTimer(hWnd, 1);
					if (TimerHWnd == hWnd && GControlProperty(hWnd, CtrlToolTipState)) {
						TimerHWnd = NULL;
						POINT MousePos;
						GetCursorPos(&MousePos);
						if (MousePos.x == TimerInitX && MousePos.y == TimerInitY && WndState == Over) {
							char txtCaption[128];
							RECT WindowRect;
							GetWindowText(hWnd, txtCaption, 128);
							GetWindowRect(hWnd, &WindowRect);
							SendMessage(ToolTipHWnd, GTTM_SETHOTSPOT, NULL, (LPARAM)&WindowRect);
							SendMessage(ToolTipHWnd, GTTM_SHOWTIP, MAKEWPARAM(TimerInitX, TimerInitY + 28), (LPARAM)txtCaption);
						}
					}
				}
			case 2: //Mouse down
				{
					KillTimer(hWnd, 2);
					POINT MouseCoords = {0};
					RECT WindowRect;
					GetWindowRect(hWnd, &WindowRect);
					GetCursorPos(&MouseCoords);
					if (GetKeyState(VK_LBUTTON) & 0x800000) {
						if (MouseCoords.x >= WindowRect.left && MouseCoords.x < WindowRect.right && MouseCoords.y >= WindowRect.top && MouseCoords.y < WindowRect.bottom && GetKeyState(VK_LBUTTON)) {
							SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GBN_LBUTTONDOWN), (LPARAM)hWnd);
						}
						SetTimer(hWnd, 2, 50, NULL);
					}
				}
			}
		}
		break;
	case WM_CAPTURECHANGED:
		{
			if ((HWND)lParam != hWnd && WndState != Off) {
				WndState = Off;
				GControlProperty(hWnd, WndState, CtrlStates);
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	case WM_MOUSEMOVE:
		{
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			WindowStates OldWndState = WndState;
			if (LOWORD(lParam) >= ClientRect.left && LOWORD(lParam) < ClientRect.right && HIWORD(lParam) >= ClientRect.top && HIWORD(lParam) < ClientRect.bottom) {
				if (WndState == DownNotOver) {
					WndState = Down;
				}
				if (WndState != Down) {
					WndState = Over;
				}
				SetCapture(hWnd);
				KillTimer(TimerHWnd, 1);
				SetTimer(hWnd, 1, 400, NULL);
				POINT MousePos;
				GetCursorPos(&MousePos);
				TimerInitX = MousePos.x;
				TimerInitY = MousePos.y;
				TimerHWnd = hWnd;
			} else {
				if (WndState != Over) {
					WndState = DownNotOver;
				} else {
					WndState = Off;
					ReleaseCapture();
				}
			}
			if (OldWndState != WndState) {
				GControlProperty(hWnd, WndState, CtrlStates);
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		return 0;
	case WM_LBUTTONDOWN:
		SendMessage(ToolTipHWnd, GTTM_HIDETIP, 0, 0);
		WndState = Down;
		GControlProperty(hWnd, WndState, CtrlStates);
		InvalidateRect(hWnd, NULL, TRUE);
		if (GetWindowLong(hWnd, GWL_STYLE) & WS_TABSTOP) SetFocus(hWnd);
		SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GBN_LBUTTONDOWN), (LPARAM)hWnd);
		SetTimer(hWnd, 2, 500, NULL);
		return 0;
	case WM_LBUTTONUP:
		KillTimer(hWnd, 2);
		if (WndState == Down) {
			WndState = Over;
			GControlProperty(hWnd, WndState, CtrlStates);
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GBN_CLICKED), (LPARAM)hWnd);
		} else {
			WndState = Off;
			GControlProperty(hWnd, WndState, CtrlStates);
			//ReleaseCapture();
		}
		ReleaseCapture(); //Possible bug created by previous bug fix - further testing required on more OS's to confirm this works with NO sideaffects (eg. Losing the capture when it should still be captured)
		InvalidateRect(hWnd, NULL, TRUE);
		SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GBN_LBUTTONUP), (LPARAM)hWnd);
		return 0;
	case WM_WINDOWPOSCHANGED:
		WndState = Off;
		GControlProperty(hWnd, WndState, CtrlStates);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case GBM_SETFACECOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrFace);
		return 0;
	case GBM_SETDARKSHADOWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrDarkShadow);
		return 0;
	case GBM_SETSHADOWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrShadow);
		return 0;
	case GBM_SETEXTRAHIGHLIGHTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrExtraHighlight);
		return 0;
	case GBM_SETHIGHLIGHTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrHighlight);
		return 0;
	case GBM_SETTEXTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrText);
		return 0;
	case GBM_SETACTIVECOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrActive);
		return 0;
	case GBM_SETDEFAULTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrDefault);
		return 0;
	case GBM_SETTOOLTIPSTATE:
		GControlProperty(hWnd, (BOOL)lParam, CtrlToolTipState);
		return 0;
	case GBM_SETOFFIMAGE:
		{
			LPVOID BmpOff = GControlProperty(hWnd, CtrlBmpOff);
			if (!BmpOff) {
				BmpOff = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpOff, CtrlBmpOff);
			}
			((BitmapOps*)BmpOff)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case GBM_SETOVERIMAGE:
		{
			LPVOID BmpOver = GControlProperty(hWnd, CtrlBmpOver);
			if (!BmpOver) {
				BmpOver = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpOver, CtrlBmpOver);
			}
			((BitmapOps*)BmpOver)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case GBM_SETONIMAGE:
		{
			LPVOID BmpOn = GControlProperty(hWnd, CtrlBmpOn);
			if (!BmpOn) {
				BmpOn = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpOn, CtrlBmpOn);
			}
			((BitmapOps*)BmpOn)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case GBM_SETDISABLEDIMAGE:
		{
			LPVOID BmpDisabled = GControlProperty(hWnd, CtrlBmpDisabled);
			if (!BmpDisabled) {
				BmpDisabled = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpDisabled, CtrlBmpDisabled);
			}
			((BitmapOps*)BmpDisabled)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case GBM_AUTOSIZE:
		{
			LPVOID BmpOff = GControlProperty(hWnd, CtrlBmpOff);
			if (BmpOff)
				SetWindowPos(hWnd, NULL, 0, 0, ((BitmapOps*)BmpOff)->Width(), ((BitmapOps*)BmpOff)->Height(), SWP_NOZORDER | SWP_NOMOVE);
			else
				SetWindowPos(hWnd, NULL, 0, 0, (LONG32)wParam, (LONG32)lParam, SWP_NOZORDER | SWP_NOMOVE);
		}
		return 0;
	case WM_CLOSE:
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		KillTimer(hWnd, 2);
		ControlDestroyed(hWnd);
		break;
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}





/*****************************************************************/
/*************************TITLE BAR*******************************/
/*****************************************************************/






LRESULT CALLBACK GTitleProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	DWORD TitleHeight = GControlProperty(hWnd, CtrlTitleHeight);
	HWND hWndClose = GControlProperty(hWnd, CtrlCloseHandles);
	HWND hWndMax = GControlProperty(hWnd, CtrlMaxHandles);
	HWND hWndRestore = GControlProperty(hWnd, CtrlRestoreHandles);
	HWND hWndMin = GControlProperty(hWnd, CtrlMinHandles);
	static BOOL Moving;
	static LONG32 OffsetX;
	static LONG32 OffsetY;
	switch (Message) {
	case WM_CREATE:
		{
			if (!ControlCreated(hWnd)) return -1;
			RECT WindowRect;
			GetClientRect(GetParent(hWnd), &WindowRect);
			SetWindowPos(hWnd, NULL, WindowRect.left, WindowRect.top, WindowRect.right, TitleHeight, SWP_NOZORDER);
			SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
			//Close button:
			hWndClose = CreateWindowEx(NULL, "MultiSoftGButton", "Close", WS_CHILD, WindowRect.right - 18, 2, 16, 16, hWnd, NULL, hInstance, NULL);
			if (!hWndClose) return -1;
			SendMessage(hWndClose, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			ShowWindow(hWndClose, SW_SHOW);
			GControlProperty(hWnd, hWndClose, CtrlCloseHandles);
			//Max button:
			hWndMax = CreateWindowEx(NULL, "MultiSoftGButton", "Maximise", WS_CHILD, WindowRect.right - 36, 2, 16, 16, hWnd, NULL, hInstance, NULL);
			if (!hWndMax) return -1;
			SendMessage(hWndMax, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			ShowWindow(hWndMax, SW_SHOW);
			GControlProperty(hWnd, hWndMax, CtrlMaxHandles);
			//Restore button:
			hWndRestore = CreateWindowEx(NULL, "MultiSoftGButton", "Restore", WS_CHILD, WindowRect.right - 36, 2, 16, 16, hWnd, NULL, hInstance, NULL);
			if (!hWndRestore) return -1;
			SendMessage(hWndRestore, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//ShowWindow(hWndMax, SW_SHOW);
			GControlProperty(hWnd, hWndRestore, CtrlRestoreHandles);
			//Min button:
			hWndMin = CreateWindowEx(NULL, "MultiSoftGButton", "Minimise", WS_CHILD, WindowRect.right - 52, 2, 16, 16, hWnd, NULL, hInstance, NULL);
			if (!hWndMin) return -1;
			SendMessage(hWndMin, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			ShowWindow(hWndMin, SW_SHOW);
			GControlProperty(hWnd, hWndMin, CtrlMinHandles);
			//Set default title text colour to black:
			GControlProperty(hWnd, (COLORREF)0x0, CtrlClrText);
			//Setup the parent system menu
			SendMessage(hWnd, WM_INITMENU, (WPARAM)GetSystemMenu(GetParent(hWnd), FALSE), 0);
		}
		return 0;
	case WM_COMMAND:
		if ((HWND)lParam == hWndClose) {
			switch (HIWORD(wParam)) {
			case GBN_CLICKED:
				SendMessage(GetParent(hWnd), WM_CLOSE, NULL, NULL);
			}
			return 0;
		}
		if ((HWND)lParam == hWndMax || (HWND)lParam == hWndRestore) {
			switch (HIWORD(wParam)) {
			case GBN_CLICKED:
				WINDOWPLACEMENT WndPlace;
				WndPlace.length = sizeof(WINDOWPLACEMENT);
				GetWindowPlacement(GetParent(hWnd), &WndPlace);
				if (WndPlace.showCmd == SW_NORMAL) {
					ShowWindow(GetParent(hWnd), SW_MAXIMIZE);
					//SendMessage(GetParent(hWnd), WM_SYSCOMMAND, SC_MAXIMIZE, NULL);
					ShowWindow(hWndRestore, SW_SHOW);
					ShowWindow(hWndMax, SW_HIDE);
				} else {
					ShowWindow(GetParent(hWnd), SW_RESTORE);
					//SendMessage(GetParent(hWnd), WM_SYSCOMMAND, SC_RESTORE, NULL);
					ShowWindow(hWndRestore, SW_HIDE);
					ShowWindow(hWndMax, SW_SHOW);
				}
				
			}
			return 0;
		}
		if ((HWND)lParam == hWndMin) {
			switch (HIWORD(wParam)) {
			case GBN_CLICKED:
				ShowWindow(GetParent(hWnd), SW_MINIMIZE);
			}
			return 0;
		}
		return 0;
	case WM_ERASEBKGND:
		{
			LPVOID BmpBar = GControlProperty(hWnd, CtrlTitleBmp);
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			HWND hWndParent = GetParent(hWnd);
			if (BmpBar) {
				HDC hDcD = GetDC(hWnd);
				HDC hDc = CreateCompatibleDC(hDcD);
				HBITMAP hBitmap = CreateCompatibleBitmap(hDcD, ClientRect.right, ClientRect.bottom);
				HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDc, hBitmap);
				//Draw:
				((BitmapOps*)BmpBar)->CopyImageArrayToDC(hWnd, hDc);
				DrawIconEx(hDc, 2, ClientRect.bottom / 2 - 8, (HICON)GetClassLong(hWndParent, GCL_HICONSM), 16, 16, 0, NULL, DI_NORMAL);
				//Caption:
				SetBkMode(hDc, 0);
				SetTextColor(hDc, GControlProperty(hWnd, CtrlClrText));
				char txtCaption[256];
				GetWindowText(hWnd, txtCaption, 256);
				SIZE txtSize;
				GetTextExtentPoint32(hDc, txtCaption, strlen(txtCaption), &txtSize);
				TextOut(hDc, 20, (ClientRect.top + ClientRect.bottom) / 2 - txtSize.cy / 2, txtCaption, strlen(txtCaption));
				//Copy and cleanup:
				BitBlt(hDcD, 0, 0, ClientRect.right, ClientRect.bottom, hDc, 0, 0, SRCCOPY);
				SelectObject(hDc, hOldBitmap);
				DeleteObject(hBitmap);
				DeleteDC(hDc);
				ReleaseDC(hWnd, hDcD);
			} else {
				HDC hDcD = GetDC(hWnd);
				HDC hDc = CreateCompatibleDC(hDcD);
				HBITMAP hBitmap = CreateCompatibleBitmap(hDcD, ClientRect.right, ClientRect.bottom);
				HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDc, hBitmap);
				//Draw:
				HPEN hOldPen = (HPEN)SelectObject(hDc, (HPEN)GetStockObject(NULL_PEN));
				HBRUSH hBrush = CreateSolidBrush(0xCC6600);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
				Rectangle(hDc, ClientRect.left, ClientRect.top, ClientRect.right + 1, ClientRect.bottom + 1);
				DrawIconEx(hDc, 2, ClientRect.bottom / 2 - 8, (HICON)GetClassLong(hWndParent, GCL_HICONSM), 16, 16, 0, NULL, DI_NORMAL);
				SelectObject(hDc, hOldBrush);
				DeleteObject(hBrush);
				SelectObject(hDc, hOldPen);
				//Caption:
				SetBkMode(hDc, 0);
				SetTextColor(hDc, GControlProperty(hWnd, CtrlClrText));
				char txtCaption[256];
				GetWindowText(hWnd, txtCaption, 256);
				SIZE txtSize;
				GetTextExtentPoint32(hDc, txtCaption, strlen(txtCaption), &txtSize);
				TextOut(hDc, 20, (ClientRect.top + ClientRect.bottom) / 2 - txtSize.cy / 2, txtCaption, strlen(txtCaption));
				//Copy and cleanup:
				BitBlt(hDcD, 0, 0, ClientRect.right, ClientRect.bottom, hDc, 0, 0, SRCCOPY);
				SelectObject(hDc, hOldBitmap);
				DeleteObject(hBitmap);
				DeleteDC(hDc);
				ReleaseDC(hWnd, hDcD);
			}
			/*InvalidateRect(hWndClose, NULL, false);
			InvalidateRect(hWndMax, NULL, false);
			InvalidateRect(hWndMin, NULL, false);*/
		}
		return 0;
	case WM_SIZING:
		{
			LONG32 BorderSizeX = 0;
			LONG32 BorderSizeY = 0;
			if (GetWindowLong(GetParent(hWnd), GWL_STYLE) & WS_THICKFRAME || GetWindowLong(GetParent(hWnd), GWL_STYLE) & WS_DLGFRAME) {
				BorderSizeX = 2 * GetSystemMetrics(SM_CXDLGFRAME);
				BorderSizeY = 2 * GetSystemMetrics(SM_CYDLGFRAME);
				//BorderSize = 6;
			}
			if (GetWindowLong(GetParent(hWnd), GWL_STYLE) & WS_BORDER) {
				BorderSizeX = 2 * GetSystemMetrics(SM_CXBORDER);
				BorderSizeY = 2 * GetSystemMetrics(SM_CYBORDER);
				//BorderSize = 2;
			}
			LONG32 MinWidth = GControlProperty(hWnd, CtrlMinWidth);
			if (((LPRECT)lParam)->bottom - ((LPRECT)lParam)->top <= (LONG32)TitleHeight + BorderSizeY) {
				if (wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMRIGHT)
					((LPRECT)lParam)->bottom = ((LPRECT)lParam)->top + TitleHeight + BorderSizeY;
				else
					((LPRECT)lParam)->top = ((LPRECT)lParam)->bottom - TitleHeight - BorderSizeY;
			}
			if (((LPRECT)lParam)->right - ((LPRECT)lParam)->left <= MinWidth + BorderSizeX) {
				if (wParam == WMSZ_TOPRIGHT || wParam == WMSZ_RIGHT || wParam == WMSZ_BOTTOMRIGHT)
					((LPRECT)lParam)->right = ((LPRECT)lParam)->left + MinWidth + BorderSizeX;
				else
					((LPRECT)lParam)->left = ((LPRECT)lParam)->right - MinWidth - BorderSizeX;
			}
			return TRUE;
		}
	case GTM_NCPARENTPAINT:
		{
			//Borders:
			RECT ClientRect;
			LONG32 BorderSizeX = 0;
			LONG32 BorderSizeY = 0;
			GetClientRect(GetParent(hWnd), &ClientRect);
			HDC ParentDC =  GetWindowDC(GetParent(hWnd));
			if (GetWindowLong(GetParent(hWnd), GWL_STYLE) & WS_THICKFRAME || GetWindowLong(GetParent(hWnd), GWL_STYLE) & WS_DLGFRAME || GetWindowLong(GetParent(hWnd), GWL_STYLE) & DS_MODALFRAME) {
				BorderSizeX = GetSystemMetrics(SM_CXDLGFRAME);
				BorderSizeY = GetSystemMetrics(SM_CYDLGFRAME);
			}
			if (GetWindowLong(GetParent(hWnd), GWL_STYLE) & WS_BORDER) {
				BorderSizeX = GetSystemMetrics(SM_CXBORDER);
				BorderSizeY = GetSystemMetrics(SM_CYBORDER);
			}
			ClientRect.right += 2 * BorderSizeX;
			ClientRect.bottom += 2 * BorderSizeY;

			if (BorderSizeX == 3 && BorderSizeY == 3) {
				//Face:
				HPEN hPen = CreatePen(PS_SOLID, 1, GControlProperty(hWnd, CtrlClrFace));
				HPEN hOldPen = (HPEN)SelectObject(ParentDC, hPen);
				HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(ParentDC, hBrush);
				Rectangle(ParentDC, ClientRect.left + 2, ClientRect.top + 2, ClientRect.right - 2, ClientRect.bottom - 2);
				SelectObject(ParentDC, hOldBrush);
				SelectObject(ParentDC, hOldPen);
				DeleteObject(hPen);
				//Border:
				MultiSoftDrawBorder(ParentDC, ClientRect, FALSE, GControlProperty(hWnd, CtrlClrDarkShadow), GControlProperty(hWnd, CtrlClrShadow), GControlProperty(hWnd, CtrlClrExtraHighlight), GControlProperty(hWnd, CtrlClrHighlight));
			}


			ReleaseDC(GetParent(hWnd), ParentDC);
			return TRUE;
		}
	case WM_SIZE:
		{
			RECT ButtonRect;
			RECT ClientRect;
			GetClientRect(GetParent(hWnd), &ClientRect);
			DWORD ButtonPos = ClientRect.right;
			SetWindowPos(hWnd, NULL, ClientRect.left, ClientRect.top, ClientRect.right, TitleHeight, SWP_NOZORDER);
			//Close button:
			GetWindowRect(hWndClose, &ButtonRect);
			ButtonPos -= 2 + (GetWindowLong(hWndClose, GWL_STYLE) & WS_VISIBLE ? ButtonRect.right - ButtonRect.left : 0);
			SetWindowPos(hWndClose, NULL, ButtonPos, (TitleHeight - ButtonRect.bottom + ButtonRect.top) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			//Max/Restore button:
			GetWindowRect(hWndMax, &ButtonRect);
			ButtonPos -= 2 + (GetWindowLong(hWndMax, GWL_STYLE) & WS_VISIBLE || GetWindowLong(hWndRestore, GWL_STYLE) & WS_VISIBLE ? ButtonRect.right - ButtonRect.left : 0);
			SetWindowPos(hWndMax, NULL, ButtonPos, (TitleHeight - ButtonRect.bottom + ButtonRect.top) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(hWndRestore, NULL, ButtonPos, (TitleHeight - ButtonRect.bottom + ButtonRect.top) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			//Min button:
			GetWindowRect(hWndMin, &ButtonRect);
			ButtonPos -= 0 + (GetWindowLong(hWndMin, GWL_STYLE) & WS_VISIBLE ? ButtonRect.right - ButtonRect.left : 0);
			SetWindowPos(hWndMin, NULL, ButtonPos, (TitleHeight - ButtonRect.bottom + ButtonRect.top) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			//Force the buttons to draw:
			//InvalidateRect(hWnd, NULL, false);
			/*InvalidateRect(hWndClose, NULL, false);
			InvalidateRect(hWndMax, NULL, false);
			InvalidateRect(hWndMin, NULL, false);*/
		}
		return TRUE;
	case WM_SYSCOMMAND:
		{
			LRESULT lResult = DefWindowProc(GetParent(hWnd), Message, wParam, lParam);
			WINDOWPLACEMENT WndPlace;
			WndPlace.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(GetParent(hWnd), &WndPlace);
			if (IsWindowVisible(hWndRestore) || IsWindowVisible(hWndMax)) {
				if (WndPlace.showCmd == SW_SHOWMAXIMIZED) {
					ShowWindow(hWndRestore, SW_SHOW);
					ShowWindow(hWndMax, SW_HIDE);
				}
				if (WndPlace.showCmd == SW_NORMAL) {
					ShowWindow(hWndRestore, SW_HIDE);
					ShowWindow(hWndMax, SW_SHOW);
				}
			}
			return lResult;
		}
	case WM_INITMENU:
		{
			HMENU hSysMenu = GetSystemMenu(GetParent(hWnd), FALSE);
			WINDOWPLACEMENT WndPlace;
			WndPlace.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(GetParent(hWnd), &WndPlace);
			if (IsWindowEnabled(hWndClose) && IsWindowVisible(hWndClose))
				EnableMenuItem(hSysMenu, SC_CLOSE, MF_ENABLED);
			else
				EnableMenuItem(hSysMenu, SC_CLOSE, MF_GRAYED);
			switch (WndPlace.showCmd) {
			case SW_NORMAL:
				{
					if (IsWindowEnabled(hWndMax) && IsWindowVisible(hWndMax))
						EnableMenuItem(hSysMenu, SC_MAXIMIZE, MF_ENABLED);
					else
						EnableMenuItem(hSysMenu, SC_MAXIMIZE, MF_GRAYED);
					EnableMenuItem(hSysMenu, SC_RESTORE, MF_GRAYED);
					if (IsWindowEnabled(hWndMin) && IsWindowVisible(hWndMin))
						EnableMenuItem(hSysMenu, SC_MINIMIZE, MF_ENABLED);
					else
						EnableMenuItem(hSysMenu, SC_MINIMIZE, MF_GRAYED);
					EnableMenuItem(hSysMenu, SC_MOVE, MF_ENABLED);
					if (GetWindowLong(GetParent(hWnd), GWL_STYLE) & WS_THICKFRAME)
						EnableMenuItem(hSysMenu, SC_SIZE, MF_ENABLED);
					else
						EnableMenuItem(hSysMenu, SC_SIZE, MF_GRAYED);
				}
				break;
			case SW_SHOWMAXIMIZED:
				{
					EnableMenuItem(hSysMenu, SC_MAXIMIZE, MF_GRAYED);
					if (IsWindowEnabled(hWndRestore) && IsWindowVisible(hWndRestore))
						EnableMenuItem(hSysMenu, SC_RESTORE, MF_ENABLED);
					else
						EnableMenuItem(hSysMenu, SC_RESTORE, MF_GRAYED);
					if (IsWindowEnabled(hWndMin) && IsWindowVisible(hWndMin))
						EnableMenuItem(hSysMenu, SC_MINIMIZE, MF_ENABLED);
					else
						EnableMenuItem(hSysMenu, SC_MINIMIZE, MF_GRAYED);
					EnableMenuItem(hSysMenu, SC_MOVE, MF_GRAYED);
					EnableMenuItem(hSysMenu, SC_SIZE, MF_GRAYED);
				}
				break;
			case SW_SHOWMINIMIZED:
				{
					EnableMenuItem(hSysMenu, SC_MINIMIZE, MF_GRAYED);
					EnableMenuItem(hSysMenu, SC_RESTORE, MF_ENABLED);
					if (IsWindowEnabled(hWndMax) && (IsWindowVisible(hWndMax) || IsWindowVisible(hWndRestore)))
						EnableMenuItem(hSysMenu, SC_MAXIMIZE, MF_ENABLED);
					else
						EnableMenuItem(hSysMenu, SC_MAXIMIZE, MF_GRAYED);
					EnableMenuItem(hSysMenu, SC_MOVE, MF_GRAYED);
					EnableMenuItem(hSysMenu, SC_SIZE, MF_GRAYED);
				}
				break;
			}
		}
		return 0;
	case WM_MOUSEMOVE:
		if (Moving) {
			POINT Coords = {(SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam)};
			ClientToScreen(hWnd, &Coords);
			SetWindowPos(GetParent(hWnd), NULL, Coords.x - OffsetX, Coords.y - OffsetY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		break;
	case WM_LBUTTONDOWN:
		{
			WINDOWPLACEMENT WndPlace;
			WndPlace.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(GetParent(hWnd), &WndPlace);
			if (WndPlace.showCmd == SW_NORMAL) {
				Moving = TRUE;
				RECT WindowRect;
				GetWindowRect(GetParent(hWnd), &WindowRect);
				POINT Coords = {LOWORD(lParam), HIWORD(lParam)};
				ClientToScreen(hWnd, &Coords);
				OffsetX = Coords.x - WindowRect.left;
				OffsetY = Coords.y - WindowRect.top;
				SetCapture(hWnd);
				RECT CursorRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &CursorRect, 0);
				ClipCursor(&CursorRect);
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			DWORD LastClickTickCount = GControlProperty(hWnd, CtrlLastClickTickCount);
			Moving = FALSE;
			ClipCursor(NULL);
			ReleaseCapture();
			if (GetTickCount() - LastClickTickCount < GetDoubleClickTime() && (IsWindowVisible(hWndMax) || IsWindowVisible(hWndRestore)) && IsWindowEnabled(hWndMax)) {
				LastClickTickCount = 0;
				WINDOWPLACEMENT WndPlace;
				WndPlace.length = sizeof(WINDOWPLACEMENT);
				GetWindowPlacement(GetParent(hWnd), &WndPlace);
				if (WndPlace.showCmd == SW_NORMAL) {
					ShowWindow(GetParent(hWnd), SW_MAXIMIZE);
					//SendMessage(GetParent(hWnd), WM_SYSCOMMAND, SC_MAXIMIZE, NULL);
					ShowWindow(hWndRestore, SW_SHOW);
					ShowWindow(hWndMax, SW_HIDE);
				} else {
					ShowWindow(GetParent(hWnd), SW_RESTORE);
					//SendMessage(GetParent(hWnd), WM_SYSCOMMAND, SC_RESTORE, NULL);
					ShowWindow(hWndRestore, SW_HIDE);
					ShowWindow(hWndMax, SW_SHOW);
				}
			} else
				LastClickTickCount = GetTickCount();
			GControlProperty(hWnd, LastClickTickCount, CtrlLastClickTickCount);
		}
		break;
	case WM_SETTEXT:
		{
			LRESULT lResult = DefWindowProc(hWnd, Message, wParam, lParam);
			InvalidateRect(hWnd, NULL, TRUE);
			return lResult;
		}
	case GTM_SETHEIGHT:
		GControlProperty(hWnd, (DWORD)lParam, CtrlTitleHeight);
		SendMessage(hWnd, WM_SIZE, 0, 0);
		break;
	case GTM_SETMINWIDTH:
		GControlProperty(hWnd, (DWORD)lParam, CtrlMinWidth);
		break;
	case GTM_GETCLOSEHWND:
		return (LRESULT)hWndClose;
	case GTM_GETMAXHWND:
		return (LRESULT)hWndMax;
	case GTM_GETRESTOREHWND:
		return (LRESULT)hWndRestore;
	case GTM_GETMINHWND:
		return (LRESULT)hWndMin;
	case GTM_SETBARIMAGE:
		{
			LPVOID BmpBar = GControlProperty(hWnd, CtrlTitleBmp);
			if (!BmpBar) {
				BmpBar = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpBar, CtrlTitleBmp);
			}
			((BitmapOps*)BmpBar)->GetBitmapFromResource((WORD)lParam);
		}
		break;
	case GTM_SETFACECOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrFace);
		return 0;
	case GTM_SETDARKSHADOWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrDarkShadow);
		return 0;
	case GTM_SETSHADOWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrShadow);
		return 0;
	case GTM_SETEXTRAHIGHLIGHTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrExtraHighlight);
		return 0;
	case GTM_SETHIGHLIGHTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrHighlight);
		return 0;
	case GTM_SETTEXTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrText);
		return 0;
	case WM_CLOSE:
		break;
	case WM_DESTROY:
		ControlDestroyed(hWnd);
		break;
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}





/*****************************************************************/
/**************************TOOL TIP*******************************/
/*****************************************************************/





HWND GetMultiSoftGToolTipHWnd() {
	return ToolTipHWnd;
}


LRESULT CALLBACK GTTipProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	static char txtCaption[128];
	static RECT HotSpotRect = {0};
	static BitmapOps *BmpBar = NULL;
	static BitmapOps *BmpLeft = NULL;
	static BitmapOps *BmpRight = NULL;
	static LONG32 AlignOptions = 0;
	static COLORREF TextColour = 0x0;
	switch (Message) {
	case WM_CREATE:
		break;
	case GTTM_SETHOTSPOT:
		HotSpotRect.left = ((LPRECT)lParam)->left;
		HotSpotRect.top = ((LPRECT)lParam)->top;
		HotSpotRect.right = ((LPRECT)lParam)->right;
		HotSpotRect.bottom = ((LPRECT)lParam)->bottom;
		AlignOptions = wParam;
		break;
	case WM_TIMER:
		{
			POINT MousePos;
			RECT WindowRect;
			GetCursorPos(&MousePos);
			GetWindowRect(hWnd, &WindowRect);
			if (MousePos.x >= HotSpotRect.left  && MousePos.y >= HotSpotRect.top && MousePos.x < HotSpotRect.right && MousePos.y < HotSpotRect.bottom)
				break;
		}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case GTTM_HIDETIP:
		KillTimer(hWnd, 1);
		ShowWindow(hWnd, SW_HIDE);
		break;
	case GTTM_SHOWTIP:
		{
			HDC hDc = GetDC(hWnd);
			strcpy(txtCaption, (LPSTR)lParam);
			SIZE txtSize;
			LONG32 ScreenX = GetSystemMetrics(SM_CXSCREEN);
			LONG32 ScreenY = GetSystemMetrics(SM_CYSCREEN);
			RECT ClientRect;
			GetTextExtentPoint32(hDc, txtCaption, strlen(txtCaption), &txtSize);
			LONG32 Height = txtSize.cy;
			LONG32 LWidth = 0;
			LONG32 RWidth = 0;
			if (BmpBar) Height = BmpBar->Height();
			if (BmpLeft) LWidth = BmpLeft->Width();
			if (BmpRight) RWidth = BmpRight->Width();
			LONG32 NewX, NewY;
			if (AlignOptions & GTT_XALIGNCENTER)
				NewX = (LOWORD(wParam) + (txtSize.cx + LWidth + RWidth) / 2 > ScreenX ? ScreenX - txtSize.cx - LWidth - RWidth : (LOWORD(wParam) - (txtSize.cx + LWidth + RWidth) / 2 < 0 ? 0 : LOWORD(wParam) - (txtSize.cx + LWidth + RWidth) / 2));
			else if (AlignOptions & GTT_XALIGNRIGHT)
				NewX = (LOWORD(wParam) > ScreenX ? ScreenX - txtSize.cx - LWidth - RWidth : (LOWORD(wParam) - txtSize.cx - LWidth - RWidth < 0 ? 0 : LOWORD(wParam) - txtSize.cx - LWidth - RWidth));
			else
				NewX = (LOWORD(wParam) + txtSize.cx + LWidth + RWidth > ScreenX ? ScreenX - txtSize.cx - LWidth - RWidth : (LOWORD(wParam) < 0 ? 0 : LOWORD(wParam)));
			if (AlignOptions & GTT_YALIGNCENTER)
				NewY = (HIWORD(wParam) + Height / 2 > ScreenY ? ScreenY - Height : (HIWORD(wParam) - Height / 2 < 0 ? 0 : HIWORD(wParam) - Height / 2));
			else if (AlignOptions & GTT_YALIGNBOTTOM)
				NewY = (HIWORD(wParam) > ScreenY ? ScreenY - Height : (HIWORD(wParam) - Height < 0 ? 0 : HIWORD(wParam) - Height));
			else
				NewY = (HIWORD(wParam) + Height > ScreenY ? ScreenY - Height : (HIWORD(wParam) < 0 ? 0 : HIWORD(wParam)));
			SetWindowPos(hWnd, HWND_TOPMOST, NewX, NewY, txtSize.cx + LWidth + RWidth, Height, SWP_SHOWWINDOW | SWP_NOACTIVATE);
			GetClientRect(hWnd, &ClientRect);
			HRGN ToolTipRgn = CreateRoundRectRgn(ClientRect.left, ClientRect.top, ClientRect.right + 1, ClientRect.bottom + 1, LWidth * 2, Height);
			SetWindowRgn(hWnd, ToolTipRgn, true);
			ReleaseDC(hWnd, hDc);
			SetTimer(hWnd, 1, 50, NULL);
		}
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hDc = BeginPaint(hWnd, &ps);
			RECT ClientRect;
			SIZE txtSize;
			GetTextExtentPoint32(hDc, txtCaption, strlen(txtCaption), &txtSize);
			GetClientRect(hWnd, &ClientRect);
			if (BmpBar) {
				HDC hDc_bmp = CreateCompatibleDC(hDc);
				if (hDc_bmp) {
					HBITMAP hBitmap = CreateCompatibleBitmap(hDc, ClientRect.right, ClientRect.bottom);
					if (hBitmap) {
						HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDc_bmp, hBitmap);
						if (BmpBar) {
							BmpBar->CopyImageArrayToDC(hWnd, hDc_bmp);
						}
						if (BmpLeft) {
							BmpLeft->CopyImageArrayToDC(hWnd, hDc_bmp, 0, 0, BmpLeft->Width(), ClientRect.bottom);
						}
						if (BmpRight) {
							BmpRight->CopyImageArrayToDC(hWnd, hDc_bmp, ClientRect.right - BmpLeft->Width(), 0, BmpLeft->Width(), ClientRect.bottom);
						}
						SetBkMode(hDc_bmp, 0);
						SetTextColor(hDc_bmp, TextColour);
						TextOut(hDc_bmp, (ClientRect.left + ClientRect.right) / 2 - txtSize.cx / 2, (ClientRect.top + ClientRect.bottom) / 2 - txtSize.cy / 2, txtCaption, strlen(txtCaption));
						BitBlt(hDc, 0, 0, ClientRect.right, ClientRect.bottom, hDc_bmp, 0, 0, SRCCOPY);
						SelectObject(hDc_bmp, hOldBitmap);
						DeleteObject(hBitmap);
					}
					DeleteDC(hDc_bmp);
				}
			}
			EndPaint(hWnd, &ps);
		}
		return 0;
	case GTTM_SETIMAGE:
		if (!BmpBar) BmpBar = new BitmapOps(1, 1);
		((BitmapOps*)BmpBar)->GetBitmapFromResource((WORD)lParam);
		break;
	case GTTM_SETLEFTIMAGE:
		if (!BmpLeft) BmpLeft = new BitmapOps(1, 1);
		((BitmapOps*)BmpLeft)->GetBitmapFromResource((WORD)lParam);
		break;
	case GTTM_SETRIGHTIMAGE:
		if (!BmpRight) BmpRight = new BitmapOps(1, 1);
		((BitmapOps*)BmpRight)->GetBitmapFromResource((WORD)lParam);
		break;
	case GTTM_SETTEXTCOLOUR:
		TextColour = lParam;
		break;
	case WM_CLOSE:
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		break;
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}





/*****************************************************************/
/*************************WINDOW CONTAINER************************/
/*****************************************************************/





LRESULT CALLBACK GWndConProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch (Message) {
	case WM_CREATE:
		if (!ControlCreated(hWnd)) return -1;
		break;
	case WM_NCCALCSIZE:
		{
			((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0].left += 3;
			((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0].top += 3;
			((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0].right -= 3;
			((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0].bottom -= 3;
			return 0;//WVR_VALIDRECTS;
		}
		break;
	case WM_NCPAINT:
		{
			//Borders:
			RECT ClientRect;
			LONG32 BorderSizeX = 3;
			LONG32 BorderSizeY = 3;
			GetClientRect(hWnd, &ClientRect);
			HDC hDc =  GetWindowDC(hWnd);
			ClientRect.right += 2 * BorderSizeX;
			ClientRect.bottom += 2 * BorderSizeY;

			if (BorderSizeX == 3 && BorderSizeY == 3) {
				//Face:
				HPEN hPen = CreatePen(PS_SOLID, 1, GControlProperty(hWnd, CtrlClrFace));
				HPEN hOldPen = (HPEN)SelectObject(hDc, hPen);
				HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
				Rectangle(hDc, ClientRect.left + 2, ClientRect.top + 2, ClientRect.right - 2, ClientRect.bottom - 2);
				SelectObject(hDc, hOldBrush);
				SelectObject(hDc, hOldPen);
				DeleteObject(hPen);
				//Border:
				MultiSoftDrawBorder(hDc, ClientRect, FALSE, GControlProperty(hWnd, CtrlClrDarkShadow), GControlProperty(hWnd, CtrlClrShadow), GControlProperty(hWnd, CtrlClrExtraHighlight), GControlProperty(hWnd, CtrlClrHighlight));
			}


			ReleaseDC(hWnd, hDc);
			return TRUE;
		}
	case WM_PAINT:
		{
			if (GControlProperty(hWnd, CtrlToolTipState)) { //Parent Paint:
				SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GWCN_PAINT), (LPARAM)hWnd);
			} else {
				PAINTSTRUCT ps;
				HDC hDc = BeginPaint(hWnd, &ps);//GetDC(hWnd);
				RECT ClientRect;
				GetClientRect(hWnd, &ClientRect);
				COLORREF clrFace = GControlProperty(hWnd, CtrlClrActive);
				HBRUSH hBrush = CreateSolidBrush(clrFace);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
				HPEN hOldPen = (HPEN)SelectObject(hDc, (HPEN)GetStockObject(NULL_PEN));
				Rectangle(hDc, ClientRect.left, ClientRect.top, ClientRect.right + 1, ClientRect.bottom + 1);
				SelectObject(hDc, hOldPen);
				SelectObject(hDc, hOldBrush);
				DeleteObject(hBrush);
				EndPaint(hWnd, &ps);
				//ReleaseDC(hWnd, hDc);
			}
		}
		return 0;
	/*case WM_CAPTURECHANGED:
		{
			RECT WindowRect;
			GetWindowRect(hWnd, &WindowRect);
			POINT MouseCoords = {0};
			GetCursorPos(&MouseCoords); 
			if (MouseCoords.x < WindowRect.left || MouseCoords.x > WindowRect.right || MouseCoords.y < WindowRect.top || MouseCoords.y > WindowRect.bottom) {
				hWndHadCapture = GetCapture();
			} else {
				hWndHadCapture = NULL;
			}
		}
		break;*/
	case WM_TIMER:
		{
			RECT WindowRect;
			RECT ClientRect;
			GetWindowRect(GetParent(hWnd), &WindowRect);
			GetClientRect(GetParent(hWnd), &ClientRect);
			DWORD CurFrame = GControlProperty(hWnd, CtrlCurFrame);
			DWORD TotalFrames = GControlProperty(hWnd, CtrlTotalFrames);
			CurFrame++;
			SetWindowPos(hWnd, NULL, 
				(LONG32)MultiSoftGInterpolateCoSine(GControlProperty(hWnd, CtrlStartX), GControlProperty(hWnd, CtrlTargetX), (double)CurFrame / (double)TotalFrames) - (WindowRect.right - WindowRect.left - ClientRect.right + ClientRect.left) / 2,
				(LONG32)MultiSoftGInterpolateCoSine(GControlProperty(hWnd, CtrlStartY), GControlProperty(hWnd, CtrlTargetY), (double)CurFrame / (double)TotalFrames) - (WindowRect.bottom - WindowRect.top - ClientRect.bottom + ClientRect.top) / 2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			GControlProperty(hWnd, CurFrame, CtrlCurFrame);
			if (CurFrame >= TotalFrames) {
				KillTimer(hWnd, 1);
			}
			GetWindowRect(hWnd, &WindowRect); //Get the updated coords
			POINT MouseCoords = {0};
			GetCursorPos(&MouseCoords); 
			if (MouseCoords.x >= WindowRect.left && MouseCoords.x < WindowRect.right && MouseCoords.y >= WindowRect.top && MouseCoords.y < WindowRect.bottom) {
				if (GetParent(GetCapture()) != hWnd) {
					ReleaseCapture();
				}
			}
		}
		break;
	case GWCM_MOVETOCOSINE:
		{
			KillTimer(hWnd, 1);
			RECT WindowRect;
			GetWindowRect(hWnd, &WindowRect);
			RECT ParentWindowRect;
			GetWindowRect(GetParent(hWnd), &ParentWindowRect);
			GControlProperty(hWnd, (LONG32)(WindowRect.left - ParentWindowRect.left), CtrlStartX);
			GControlProperty(hWnd, (LONG32)(WindowRect.top - ParentWindowRect.top), CtrlStartY);
			GControlProperty(hWnd, (LONG32)(SHORT)LOWORD(wParam) + 3, CtrlTargetX);
			GControlProperty(hWnd, (LONG32)(SHORT)HIWORD(wParam) + 3, CtrlTargetY);
			GControlProperty(hWnd, (DWORD)LOWORD(lParam), CtrlTotalFrames);
			GControlProperty(hWnd, (DWORD)0, CtrlCurFrame);
			SetTimer(hWnd, 1, 1000 / HIWORD(lParam), NULL);
			POINT MouseCoords = {0};
			GetCursorPos(&MouseCoords); 
		}
		return 0;
	case WM_LBUTTONDOWN:
		{
			SetCapture(hWnd);
			GControlProperty(hWnd, Down, CtrlStates);
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GWCN_LBUTTONDOWN), (LPARAM)hWnd);
		}
		return 0;
	case WM_LBUTTONUP:
		{
			ReleaseCapture();
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GWCN_LBUTTONUP), (LPARAM)hWnd);
			WindowStates WndState = GControlProperty(hWnd, CtrlStates);
			GControlProperty(hWnd, Off, CtrlStates);
			if (WndState == Down) {
				LONG32 x = LOWORD(lParam);
				LONG32 y = HIWORD(lParam);
				RECT ClientRect;
				GetClientRect(hWnd, &ClientRect);
				if (x >= 0 && y >= 0 && x < ClientRect.right && y < ClientRect.bottom)
					SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GWCN_CLICKED), (LPARAM)hWnd);
			}
		}
		return 0;
	case WM_RBUTTONDOWN:
		{
			SetCapture(hWnd);
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GWCN_RBUTTONDOWN), (LPARAM)hWnd);
		}
		return 0;
	case WM_RBUTTONUP:
		{
			ReleaseCapture();
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GWCN_RBUTTONUP), (LPARAM)hWnd);
		}
		return 0;
	case WM_MBUTTONDOWN:
		{
			SetCapture(hWnd);
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GWCN_MBUTTONDOWN), (LPARAM)hWnd);
		}
		return 0;
	case WM_MBUTTONUP:
		{
			ReleaseCapture();
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GWCN_MBUTTONUP), (LPARAM)hWnd);
		}
		return 0;
	case WM_MOUSEMOVE:
		{
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GWCN_MOUSEMOVE), (LPARAM)hWnd);
			return 0;
		}
	case WM_COMMAND:
		return SendMessage(GetParent(hWnd), Message, wParam, lParam);
	case GWCM_SETFACECOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrFace);
		return 0;
	case GWCM_SETDARKSHADOWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrDarkShadow);
		return 0;
	case GWCM_SETSHADOWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrShadow);
		return 0;
	case GWCM_SETEXTRAHIGHLIGHTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrExtraHighlight);
		return 0;
	case GWCM_SETHIGHLIGHTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrHighlight);
		return 0;
	case GWCM_SETBACKCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrActive);
		return 0;
	case GWCM_SETPARENTPAINT:
		GControlProperty(hWnd, (BOOL)lParam, CtrlToolTipState);
		return 0;
	case WM_CLOSE:
		break;
	case WM_DESTROY:
		{
			KillTimer(hWnd, 1);
		}
		ControlDestroyed(hWnd);
		break;
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}





/*****************************************************************/
/***********************VERTICAL SCROLLBAR************************/
/*****************************************************************/





LRESULT CALLBACK GVScrolProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	HWND hWndUp = GControlProperty(hWnd, CtrlMinHandles);
	HWND hWndDown = GControlProperty(hWnd, CtrlMaxHandles);
	static BOOL Dragging = FALSE;
	static LONG32 OffsetY = 0;
	LPVOID BmpBarTop = GControlProperty(hWnd, CtrlBmpOff);
	LPVOID BmpBarBottom = GControlProperty(hWnd, CtrlBmpOn);
	LONG32 MinBarHeight = 0;
	if (BmpBarTop) MinBarHeight += ((BitmapOps*)BmpBarTop)->Height();
	if (BmpBarBottom) MinBarHeight += ((BitmapOps*)BmpBarBottom)->Height();
	if (!MinBarHeight) MinBarHeight = 4;
	switch (Message) {
	case WM_CREATE:
		{
			if (!ControlCreated(hWnd)) return -1;
			SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
			RECT WindowRect;
			GetClientRect(hWnd, &WindowRect);
			//Up button:
			hWndUp = CreateWindowEx(NULL, "MultiSoftGButton", "Up", WS_CHILD | WS_VISIBLE, 0, 0, WindowRect.right, 16, hWnd, NULL, hInstance, NULL);
			if (!hWndUp) return -1;
			GControlProperty(hWnd, hWndUp, CtrlMinHandles);
			//Down button:
			hWndDown = CreateWindowEx(NULL, "MultiSoftGButton", "Down", WS_CHILD | WS_VISIBLE, 0, WindowRect.bottom - 16, WindowRect.right, 16, hWnd, NULL, hInstance, NULL);
			if (!hWndDown) return -1;
			GControlProperty(hWnd, hWndDown, CtrlMaxHandles);
		}
		break;
	case WM_ENABLE:
		{
			EnableWindow(hWndUp, wParam);
			EnableWindow(hWndDown, wParam);
		}
		return 0;
	case WM_ERASEBKGND:
		{
			LPVOID BmpTrack = GControlProperty(hWnd, CtrlTitleBmp);
			LPVOID BmpBar = GControlProperty(hWnd, CtrlBmpOver);

			HDC hDc = GetDC(hWnd);
			HDC hDcMem = CreateCompatibleDC(hDc);

			RECT UpWindowRect, DownWindowRect, WindowRect;
			LONG32 Difference;
			LONG32 BarSize;
			LONG32 BarPosition;
			GetWindowRect(hWndUp, &UpWindowRect);
			GetWindowRect(hWndDown, &DownWindowRect);
			GetWindowRect(hWnd, &WindowRect);
			Difference = GControlProperty(hWnd, CtrlMax) - GControlProperty(hWnd, CtrlMin);
			if (Difference < 0) Difference *= -1;
			if (Dragging) {
				//Get the coords of the bar:
				BarSize = WindowRect.bottom - WindowRect.top - DownWindowRect.bottom + DownWindowRect.top - UpWindowRect.bottom + UpWindowRect.top;
				if (Difference) { //Otherwise Divide by zero error would occur
					BarSize = (LONG32)((double)BarSize / (double)(Difference + 1));
					if (BarSize < 0) BarSize *= -1; //If Min > Max, make it positive
					if (BarSize < MinBarHeight) BarSize = MinBarHeight;
				}
				POINT MouseCoords;
				GetCursorPos(&MouseCoords);
				BarPosition = MouseCoords.y - WindowRect.top - OffsetY;// + UpWindowRect.bottom - UpWindowRect.top - OffsetY;
				if (BarPosition < UpWindowRect.bottom - UpWindowRect.top) BarPosition = UpWindowRect.bottom - UpWindowRect.top;
				if (BarPosition + BarSize > DownWindowRect.top - WindowRect.top) BarPosition = DownWindowRect.top - WindowRect.top - BarSize;
			} else {
				//Get the coords of the bar:
				BarPosition = UpWindowRect.bottom - UpWindowRect.top;
				BarSize = WindowRect.bottom - WindowRect.top - DownWindowRect.bottom + DownWindowRect.top - BarPosition;
				if (Difference) { //Otherwise Divide by zero error would occur
					BarPosition = BarSize;
					BarSize = (LONG32)((double)BarSize / (double)(Difference + 1));
					if (BarSize < 0) BarSize *= -1; //If Min > Max, make it positive
					if (BarSize < MinBarHeight) BarSize = MinBarHeight;
					if (GControlProperty(hWnd, CtrlMin) < GControlProperty(hWnd, CtrlMax)) {
						BarPosition = (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMin)) + UpWindowRect.bottom - UpWindowRect.top);
					} else {
						BarPosition = DownWindowRect.top - UpWindowRect.bottom - BarSize - (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMax))) + UpWindowRect.bottom - UpWindowRect.top;
					}
				}
			}
			HBITMAP hBitmap = CreateCompatibleBitmap(hDc, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top);
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDcMem, hBitmap);

			//Track:
			if (BmpTrack) {
				((BitmapOps*)BmpTrack)->CopyImageArrayToDC(hWnd, hDcMem, 0, UpWindowRect.bottom - UpWindowRect.top, WindowRect.right - WindowRect.left, DownWindowRect.top - UpWindowRect.bottom);
			} else {
				HBRUSH hBrush = CreateSolidBrush(GControlProperty(hWnd, CtrlClrActive));
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDcMem, hBrush);
				HPEN hPen = (HPEN)GetStockObject(NULL_PEN);
				HPEN hOldPen = (HPEN)SelectObject(hDcMem, hPen);
				Rectangle(hDcMem, 0, UpWindowRect.bottom - UpWindowRect.top, WindowRect.right - WindowRect.left + 1, DownWindowRect.bottom);
				SelectObject(hDcMem, hOldPen);
				SelectObject(hDcMem, hOldBrush);
				DeleteObject(hBrush);
			}

			if (BmpBar) {
				((BitmapOps*)BmpBar)->CopyImageArrayToDC(hWnd, hDcMem, 0, BarPosition, WindowRect.right - WindowRect.left, BarSize);
			} else {
				RECT BarCoords = {0, BarPosition, WindowRect.right - WindowRect.left, BarPosition + BarSize};
				MultiSoftDrawBorder(hDcMem, BarCoords, TRUE, GControlProperty(hWnd, CtrlClrDarkShadow), GControlProperty(hWnd, CtrlClrShadow), GControlProperty(hWnd, CtrlClrExtraHighlight), GControlProperty(hWnd, CtrlClrHighlight), GControlProperty(hWnd, CtrlClrFace));
			}
			if (BmpBarBottom) {
				((BitmapOps*)BmpBarBottom)->CopyImageArrayToDC(hWnd, hDcMem, 0, BarPosition + BarSize - ((BitmapOps*)BmpBarBottom)->Height(), WindowRect.right - WindowRect.left, ((BitmapOps*)BmpBarBottom)->Height());
			}
			if (BmpBarTop) {
				((BitmapOps*)BmpBarTop)->CopyImageArrayToDC(hWnd, hDcMem, 0, BarPosition, WindowRect.right - WindowRect.left, ((BitmapOps*)BmpBarTop)->Height());
			}

			BitBlt(hDc, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, hDcMem, 0, 0, SRCCOPY);

			SelectObject(hDcMem, hOldBitmap);
			DeleteObject(hBitmap);
			DeleteDC(hDcMem);
			ReleaseDC(hWnd, hDc);
		}
		return 0;
	case WM_LBUTTONDOWN:
		{
			//Get the coords of the bar:
			RECT UpWindowRect, DownWindowRect, WindowRect;
			GetWindowRect(hWndUp, &UpWindowRect);
			GetWindowRect(hWndDown, &DownWindowRect);
			GetWindowRect(hWnd, &WindowRect);
			LONG32 Difference = GControlProperty(hWnd, CtrlMax) - GControlProperty(hWnd, CtrlMin);
			if (Difference < 0) Difference *= -1;
			LONG32 BarPosition = UpWindowRect.bottom - UpWindowRect.top;
			LONG32 BarSize = WindowRect.bottom - WindowRect.top - DownWindowRect.bottom + DownWindowRect.top - BarPosition;
			if (Difference) { //Otherwise Divide by zero error would occur
				BarPosition = BarSize;
				BarSize = (LONG32)((double)BarSize / (double)(Difference + 1));
				if (BarSize < 0) BarSize *= -1; //If Min > Max, make it positive
				if (BarSize < MinBarHeight) BarSize = MinBarHeight;
				if (GControlProperty(hWnd, CtrlMin) < GControlProperty(hWnd, CtrlMax)) {
					BarPosition = (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMin)) + UpWindowRect.bottom - UpWindowRect.top);
				} else {
					BarPosition = DownWindowRect.top - UpWindowRect.bottom - BarSize - (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMax))) + UpWindowRect.bottom - UpWindowRect.top;
				}
			}

			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val = GControlProperty(hWnd, CtrlVal);
			if (HIWORD(lParam) < BarPosition) {
				if (Min < Max)
					Val -= GControlProperty(hWnd, CtrlLgUpdate);
				else
					Val += GControlProperty(hWnd, CtrlLgUpdate);
				SetTimer(hWnd, 1, 500, NULL);
			} else if (HIWORD(lParam) >= BarPosition + BarSize) {
				if (Min < Max)
					Val += GControlProperty(hWnd, CtrlLgUpdate);
				else
					Val -= GControlProperty(hWnd, CtrlLgUpdate);
				SetTimer(hWnd, 2, 500, NULL);
			} else {
				Dragging = TRUE;
				OffsetY = HIWORD(lParam) - BarPosition;
			}
			if (Min < Max) {
				if (Val < Min) Val = Min;
				if (Val > Max) Val = Max;
			} else {
				if (Val < Max) Val = Max;
				if (Val > Min) Val = Min;
			}
			GControlProperty(hWnd, Val, CtrlVal);
			InvalidateRect(hWnd, NULL, TRUE);
			SetCapture(hWnd);
		}
		return 0;
	case GSM_GETDRAGVALUE:
		{
			if (Dragging) {
				RECT UpWindowRect, DownWindowRect, WindowRect;
				LONG32 BarSize, BarPosition;
				LONG32 Min = GControlProperty(hWnd, CtrlMin);
				LONG32 Max = GControlProperty(hWnd, CtrlMax);
				LONG32 Difference;
				if (Max > Min) {
					Difference = Max - Min;
				} else {
					Difference = Min - Max;
				}
				double AbBarSize;
				GetWindowRect(hWndUp, &UpWindowRect);
				GetWindowRect(hWndDown, &DownWindowRect);
				GetWindowRect(hWnd, &WindowRect);
				//Get the coords of the bar:
				BarSize = WindowRect.bottom - WindowRect.top - DownWindowRect.bottom + DownWindowRect.top - UpWindowRect.bottom + UpWindowRect.top;
				if (Difference) {
					AbBarSize = (double)BarSize / (double)(Difference + 1);
					if (AbBarSize < 0) AbBarSize *= -1.0; //If Min > Max, make it positive
				} else { //Otherwise Divide by zero error would occur
					AbBarSize = DownWindowRect.top - UpWindowRect.bottom;
				}
				BarSize = (LONG32)AbBarSize;
				if (BarSize < MinBarHeight) BarSize = MinBarHeight;
				POINT MouseCoords;
				GetCursorPos(&MouseCoords);
				BarPosition = MouseCoords.y - UpWindowRect.bottom - OffsetY;
				if (BarPosition < 0) BarPosition = 0;
				if (BarPosition + BarSize > DownWindowRect.top - UpWindowRect.bottom) BarPosition = DownWindowRect.top - UpWindowRect.bottom - BarSize;
				//Calculate the value:
				/*double Val;
				LONG32 Multiplyer;
				if (Max > Min) {*/
					/*if (Min < 0) Multiplyer = -1;
					else Multiplyer = 1;*/
					/*if (Max < 0) Multiplyer = -1;
					else if (Min > 0) Multiplyer = 1;
					else Multiplyer = 0;
				} else {*/
					/*if (Max < 0) Multiplyer = -1;
					else Multiplyer = 1;*/
					/*if (Min < 0) Multiplyer = -1;
					else if (Max > 0) Multiplyer = 1;
					else Multiplyer = 0;
				}*/
				/*if (Max > Min)
					Val = (LONG32)( ( BarPosition + Multiplyer * AbBarSize / 2.0 ) * Difference / ( DownWindowRect.top - UpWindowRect.bottom - BarSize ) + Min );
				else if (Max < Min)
					Val = (LONG32)( ( ( ( DownWindowRect.top - UpWindowRect.bottom - AbBarSize ) - BarPosition ) + Multiplyer * AbBarSize / 2.0) * Difference / ( DownWindowRect.top - UpWindowRect.bottom - BarSize ) + Max );
				else
					Val = Min;*/
				LONG32 Val;
				if (Difference) {
					Val = (LONG32)( ( BarPosition + AbBarSize / 2.0 ) * Difference / ( DownWindowRect.top - UpWindowRect.bottom - BarSize ) );
					if (Max > Min) {
						Val += Min;
					} else if (Max < Min) {
						Val = Difference - Val + Max;
					}
				} else {
					Val = Min;
				}
				return Val;
			}
		}
		return GControlProperty(hWnd, CtrlVal);
	case WM_LBUTTONUP:
		{
			if (Dragging) {
				GControlProperty(hWnd, (LONG32)SendMessage(hWnd, GSM_GETDRAGVALUE, 0, 0), CtrlVal);
				SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GSN_SCROLL), (LPARAM)hWnd);
			}
			Dragging = FALSE;
			ReleaseCapture();
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case WM_MOUSEMOVE:
		if (Dragging) {
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GSN_BARDRAG), (LPARAM)hWnd);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case WM_CAPTURECHANGED:
		Dragging = FALSE;
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_TIMER:
		{
			KillTimer(hWnd, 1);
			KillTimer(hWnd, 2);
			//Get the coords of the bar:
			RECT UpWindowRect, DownWindowRect, WindowRect;
			GetWindowRect(hWndUp, &UpWindowRect);
			GetWindowRect(hWndDown, &DownWindowRect);
			GetWindowRect(hWnd, &WindowRect);
			LONG32 Difference = GControlProperty(hWnd, CtrlMax) - GControlProperty(hWnd, CtrlMin);
			if (Difference < 0) Difference *= -1;
			LONG32 BarPosition = UpWindowRect.bottom - UpWindowRect.top;
			LONG32 BarSize = WindowRect.bottom - WindowRect.top - DownWindowRect.bottom + DownWindowRect.top - BarPosition;
			if (Difference) { //Otherwise Divide by zero error would occur
				BarPosition = BarSize;
				BarSize = (LONG32)((double)BarSize / (double)Difference);
				if (BarSize < 0) BarSize *= -1; //If Min > Max, make it positive
				if (BarSize < MinBarHeight) BarSize = MinBarHeight;
				if (GControlProperty(hWnd, CtrlMin) < GControlProperty(hWnd, CtrlMax)) {
					BarPosition = (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMin)) + UpWindowRect.bottom - UpWindowRect.top);
				} else {
					BarPosition = DownWindowRect.top - UpWindowRect.bottom - BarSize - (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMax))) + UpWindowRect.bottom - UpWindowRect.top;
				}
			}

			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val = GControlProperty(hWnd, CtrlVal);
			BOOL SendParentMsg = FALSE;
			POINT CursorPos;
			GetCursorPos(&CursorPos);
			if (GetKeyState(VK_LBUTTON) & 0x80000000) {
				if (CursorPos.x >= WindowRect.left && CursorPos.x < WindowRect.right && CursorPos.y >= UpWindowRect.bottom && CursorPos.y < DownWindowRect.top) {
					if (CursorPos.y < WindowRect.top + BarPosition && wParam == 1) {
						if (Min < Max)
							Val -= GControlProperty(hWnd, CtrlLgUpdate);
						else
							Val += GControlProperty(hWnd, CtrlLgUpdate);
						SendParentMsg = TRUE;
					} else if (CursorPos.y >= WindowRect.top + BarPosition + BarSize && wParam == 2) {
						if (Min < Max)
							Val += GControlProperty(hWnd, CtrlLgUpdate);
						else
							Val -= GControlProperty(hWnd, CtrlLgUpdate);
						SendParentMsg = TRUE;
					}
					if (Min < Max) {
						if (Val < Min) Val = Min;
						if (Val > Max) Val = Max;
					} else {
						if (Val < Max) Val = Max;
						if (Val > Min) Val = Min;
					}
					GControlProperty(hWnd, Val, CtrlVal);
					if (SendParentMsg) SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GSN_SCROLL), (LPARAM)hWnd);
					InvalidateRect(hWnd, NULL, TRUE);
				}
				SetTimer(hWnd, wParam, 50, NULL);
			}
		}
		return 0;
	case WM_COMMAND:
		{
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val = GControlProperty(hWnd, CtrlVal);
			BOOL SendParentMsg = FALSE;
			if ((HWND)lParam == hWndUp) {
				if (HIWORD(wParam) == GBN_LBUTTONDOWN) {
					if (Min < Max)
						Val -= GControlProperty(hWnd, CtrlSmUpdate);
					else
						Val += GControlProperty(hWnd, CtrlSmUpdate);
					SendParentMsg = TRUE;
				}
			} else if ((HWND)lParam == hWndDown) {
				if (HIWORD(wParam) == GBN_LBUTTONDOWN) {
					if (Min < Max)
						Val += GControlProperty(hWnd, CtrlSmUpdate);
					else
						Val -= GControlProperty(hWnd, CtrlSmUpdate);
					SendParentMsg = TRUE;
				}
			}
			if (Min < Max) {
				if (Val < Min) Val = Min;
				if (Val > Max) Val = Max;
			} else {
				if (Val < Max) Val = Max;
				if (Val > Min) Val = Min;
			}
			GControlProperty(hWnd, Val, CtrlVal);
			if (SendParentMsg) SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GSN_SCROLL), (LPARAM)hWnd);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case GSM_SETVALUE:
		{
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val = (LONG32)lParam;
			if (Min < Max) {
				if (Val < Min) Val = Min;
				if (Val > Max) Val = Max;
			} else {
				if (Val < Max) Val = Max;
				if (Val > Min) Val = Min;
			}
			GControlProperty(hWnd, Val, CtrlVal);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case GSM_GETVALUE:
		return GControlProperty(hWnd, CtrlVal);
	case GSM_SETMIN:
		{
			LONG32 Min = (LONG32)lParam;
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val = GControlProperty(hWnd, CtrlVal);
			if (Min < Max) {
				if (Val < Min) Val = Min;
				if (Val > Max) Val = Max;
			} else {
				if (Val < Max) Val = Max;
				if (Val > Min) Val = Min;
			}
			GControlProperty(hWnd, Min, CtrlMin);
			GControlProperty(hWnd, Val, CtrlVal);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case GSM_SETMAX:
		{
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = (LONG32)lParam;
			LONG32 Val = GControlProperty(hWnd, CtrlVal);
			if (Min < Max) {
				if (Val < Min) Val = Min;
				if (Val > Max) Val = Max;
			} else {
				if (Val < Max) Val = Max;
				if (Val > Min) Val = Min;
			}
			GControlProperty(hWnd, Max, CtrlMax);
			GControlProperty(hWnd, Val, CtrlVal);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case GSM_SETSMALLUPDATE:
		GControlProperty(hWnd, (LONG32)lParam, CtrlSmUpdate);
		return 0;
	case GSM_SETLARGEUPDATE:
		GControlProperty(hWnd, (LONG32)lParam, CtrlLgUpdate);
		return 0;
	case GSM_GETUPLEFTHWND:
		return (LRESULT)hWndUp;
	case GSM_GETDOWNRIGHTHWND:
		return (LRESULT)hWndDown;
	//Colours:
	case GSM_SETFACECOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrFace);
		SendMessage(hWndUp, GBM_SETFACECOLOUR, wParam, lParam);
		SendMessage(hWndDown, GBM_SETFACECOLOUR, wParam, lParam);
		return 0;
	case GSM_SETDARKSHADOWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrDarkShadow);
		SendMessage(hWndUp, GBM_SETDARKSHADOWCOLOUR, wParam, lParam);
		SendMessage(hWndDown, GBM_SETDARKSHADOWCOLOUR, wParam, lParam);
		return 0;
	case GSM_SETSHADOWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrShadow);
		SendMessage(hWndUp, GBM_SETSHADOWCOLOUR, wParam, lParam);
		SendMessage(hWndDown, GBM_SETSHADOWCOLOUR, wParam, lParam);
		return 0;
	case GSM_SETEXTRAHIGHLIGHTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrExtraHighlight);
		SendMessage(hWndUp, GBM_SETEXTRAHIGHLIGHTCOLOUR, wParam, lParam);
		SendMessage(hWndDown, GBM_SETEXTRAHIGHLIGHTCOLOUR, wParam, lParam);
		return 0;
	case GSM_SETHIGHLIGHTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrHighlight);
		SendMessage(hWndUp, GBM_SETHIGHLIGHTCOLOUR, wParam, lParam);
		SendMessage(hWndDown, GBM_SETHIGHLIGHTCOLOUR, wParam, lParam);
		return 0;
	case GSM_SETARROWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrText);
		SendMessage(hWndUp, GBM_SETTEXTCOLOUR, wParam, lParam);
		SendMessage(hWndDown, GBM_SETTEXTCOLOUR, wParam, lParam);
		return 0;
	case GSM_SETTRACKCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrActive);
		return 0;
	case GSM_SETTRACKIMAGE:
		{
			LPVOID BmpTrack = GControlProperty(hWnd, CtrlTitleBmp);
			if (!BmpTrack) {
				BmpTrack = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpTrack, CtrlTitleBmp);
			}
			((BitmapOps*)BmpTrack)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
		case GSM_SETBARTOPIMAGE:
		{
			LPVOID BmpTop = GControlProperty(hWnd, CtrlBmpOff);
			if (!BmpTop) {
				BmpTop = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpTop, CtrlBmpOff);
			}
			((BitmapOps*)BmpTop)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
		case GSM_SETBARIMAGE:
		{
			LPVOID BmpBar = GControlProperty(hWnd, CtrlBmpOver);
			if (!BmpBar) {
				BmpBar = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpBar, CtrlBmpOver);
			}
			((BitmapOps*)BmpBar)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
		case GSM_SETBARBOTTOMIMAGE:
		{
			LPVOID BmpBottom = GControlProperty(hWnd, CtrlBmpOn);
			if (!BmpBottom) {
				BmpBottom = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpBottom, CtrlBmpOn);
			}
			((BitmapOps*)BmpBottom)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case WM_CLOSE:
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		KillTimer(hWnd, 2);
		ControlDestroyed(hWnd);
		break;
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}





/*****************************************************************/
/**********************HORIZONTAL SCROLLBAR***********************/
/*****************************************************************/





LRESULT CALLBACK GHScrolProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	HWND hWndLeft = GControlProperty(hWnd, CtrlMinHandles);
	HWND hWndRight = GControlProperty(hWnd, CtrlMaxHandles);
	static BOOL Dragging = FALSE;
	static LONG32 OffsetX = 0;
	LPVOID BmpBarLeft = GControlProperty(hWnd, CtrlBmpOff);
	LPVOID BmpBarRight = GControlProperty(hWnd, CtrlBmpOn);
	LONG32 MinBarWidth = 0;
	if (BmpBarLeft) MinBarWidth += ((BitmapOps*)BmpBarLeft)->Width();
	if (BmpBarRight) MinBarWidth += ((BitmapOps*)BmpBarRight)->Width();
	if (!MinBarWidth) MinBarWidth = 4;
	switch (Message) {
	case WM_CREATE:
		{
			if (!ControlCreated(hWnd)) return -1;
			SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
			RECT WindowRect;
			GetClientRect(hWnd, &WindowRect);
			//Left button:
			hWndLeft = CreateWindowEx(NULL, "MultiSoftGButton", "Left", WS_CHILD | WS_VISIBLE, 0, 0, 16, WindowRect.bottom, hWnd, NULL, hInstance, NULL);
			if (!hWndLeft) return -1;
			GControlProperty(hWnd, hWndLeft, CtrlMinHandles);
			//Right button:
			hWndRight = CreateWindowEx(NULL, "MultiSoftGButton", "Right", WS_CHILD | WS_VISIBLE, WindowRect.right - 16, 0, 16, WindowRect.bottom, hWnd, NULL, hInstance, NULL);
			if (!hWndRight) return -1;
			GControlProperty(hWnd, hWndRight, CtrlMaxHandles);
		}
		break;
	case WM_ENABLE:
		{
			EnableWindow(hWndLeft, wParam);
			EnableWindow(hWndRight, wParam);
		}
		return 0;
	case WM_ERASEBKGND:
		{
			LPVOID BmpTrack = GControlProperty(hWnd, CtrlTitleBmp);
			LPVOID BmpBar = GControlProperty(hWnd, CtrlBmpOver);

			HDC hDc = GetDC(hWnd);
			HDC hDcMem = CreateCompatibleDC(hDc);

			RECT LeftWindowRect, RightWindowRect, WindowRect;
			LONG32 Difference;
			LONG32 BarSize;
			LONG32 BarPosition;
			GetWindowRect(hWndLeft, &LeftWindowRect);
			GetWindowRect(hWndRight, &RightWindowRect);
			GetWindowRect(hWnd, &WindowRect);
			Difference = GControlProperty(hWnd, CtrlMax) - GControlProperty(hWnd, CtrlMin);
			if (Difference < 0) Difference *= -1;
			if (Dragging) {
				//Get the coords of the bar:
				BarSize = WindowRect.right - WindowRect.left - RightWindowRect.right + RightWindowRect.left - LeftWindowRect.right + LeftWindowRect.left;
				if (Difference) { //Otherwise Divide by zero error would occur
					BarSize = (LONG32)((double)BarSize / (double)(Difference + 1));
					if (BarSize < 0) BarSize *= -1; //If Min > Max, make it positive
					if (BarSize < MinBarWidth) BarSize = MinBarWidth;
				}
				POINT MouseCoords;
				GetCursorPos(&MouseCoords);
				BarPosition = MouseCoords.x - WindowRect.left - OffsetX;
				if (BarPosition < LeftWindowRect.right - LeftWindowRect.left) BarPosition = LeftWindowRect.right - LeftWindowRect.left;
				if (BarPosition + BarSize > RightWindowRect.left - WindowRect.left) BarPosition = RightWindowRect.left - WindowRect.left - BarSize;
			} else {
				//Get the coords of the bar:
				BarPosition = LeftWindowRect.right - LeftWindowRect.left;
				BarSize = WindowRect.right - WindowRect.left - RightWindowRect.right + RightWindowRect.left - BarPosition;
				if (Difference) { //Otherwise Divide by zero error would occur
					BarPosition = BarSize;
					BarSize = (LONG32)((double)BarSize / (double)(Difference + 1));
					if (BarSize < 0) BarSize *= -1; //If Min > Max, make it positive
					if (BarSize < MinBarWidth) BarSize = MinBarWidth;
					if (GControlProperty(hWnd, CtrlMin) < GControlProperty(hWnd, CtrlMax)) {
						BarPosition = (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMin)) + LeftWindowRect.right - LeftWindowRect.left);
					} else {
						BarPosition = RightWindowRect.left - LeftWindowRect.right - BarSize - (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMax))) + LeftWindowRect.right - LeftWindowRect.left;
					}
				}
			}
			HBITMAP hBitmap = CreateCompatibleBitmap(hDc, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top);
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDcMem, hBitmap);

			//Track:
			if (BmpTrack) {
				((BitmapOps*)BmpTrack)->CopyImageArrayToDC(hWnd, hDcMem, LeftWindowRect.right - LeftWindowRect.left, 0, RightWindowRect.left - LeftWindowRect.right, WindowRect.bottom - WindowRect.top);
			} else {
				HBRUSH hBrush = CreateSolidBrush(GControlProperty(hWnd, CtrlClrActive));
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDcMem, hBrush);
				HPEN hPen = (HPEN)GetStockObject(NULL_PEN);
				HPEN hOldPen = (HPEN)SelectObject(hDcMem, hPen);
				Rectangle(hDcMem, LeftWindowRect.right - LeftWindowRect.left, 0, RightWindowRect.right, WindowRect.bottom - WindowRect.top + 1);
				SelectObject(hDcMem, hOldPen);
				SelectObject(hDcMem, hOldBrush);
				DeleteObject(hBrush);
			}

			if (BmpBar) {
				((BitmapOps*)BmpBar)->CopyImageArrayToDC(hWnd, hDcMem, BarPosition, 0, BarSize, WindowRect.bottom - WindowRect.top);
			} else {
				RECT BarCoords = {BarPosition, 0, BarPosition + BarSize, WindowRect.bottom - WindowRect.top};
				MultiSoftDrawBorder(hDcMem, BarCoords, TRUE, GControlProperty(hWnd, CtrlClrDarkShadow), GControlProperty(hWnd, CtrlClrShadow), GControlProperty(hWnd, CtrlClrExtraHighlight), GControlProperty(hWnd, CtrlClrHighlight), GControlProperty(hWnd, CtrlClrFace));
			}
			if (BmpBarRight) {
				((BitmapOps*)BmpBarRight)->CopyImageArrayToDC(hWnd, hDcMem, BarPosition + BarSize - ((BitmapOps*)BmpBarRight)->Width(), 0, ((BitmapOps*)BmpBarRight)->Width(), WindowRect.bottom - WindowRect.top);
			}
			if (BmpBarLeft) {
				((BitmapOps*)BmpBarLeft)->CopyImageArrayToDC(hWnd, hDcMem, BarPosition, 0, ((BitmapOps*)BmpBarLeft)->Width(), WindowRect.bottom - WindowRect.top);
			}

			BitBlt(hDc, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, hDcMem, 0, 0, SRCCOPY);

			SelectObject(hDcMem, hOldBitmap);
			DeleteObject(hBitmap);
			DeleteDC(hDcMem);
			ReleaseDC(hWnd, hDc);
		}
		return 0;
	case WM_LBUTTONDOWN:
		{
			//Get the coords of the bar:
			RECT LeftWindowRect, RightWindowRect, WindowRect;
			GetWindowRect(hWndLeft, &LeftWindowRect);
			GetWindowRect(hWndRight, &RightWindowRect);
			GetWindowRect(hWnd, &WindowRect);
			LONG32 Difference = GControlProperty(hWnd, CtrlMax) - GControlProperty(hWnd, CtrlMin);
			if (Difference < 0) Difference *= -1;
			LONG32 BarPosition = LeftWindowRect.right - LeftWindowRect.left;
			LONG32 BarSize = WindowRect.right - WindowRect.left - RightWindowRect.right + RightWindowRect.left - BarPosition;
			if (Difference) { //Otherwise Divide by zero error would occur
				BarPosition = BarSize;
				BarSize = (LONG32)((double)BarSize / (double)(Difference + 1));
				if (BarSize < 0) BarSize *= -1; //If Min > Max, make it positive
				if (BarSize < MinBarWidth) BarSize = MinBarWidth;
				if (GControlProperty(hWnd, CtrlMin) < GControlProperty(hWnd, CtrlMax)) {
					BarPosition = (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMin)) + LeftWindowRect.right - LeftWindowRect.left);
				} else {
					BarPosition = RightWindowRect.left - LeftWindowRect.right - BarSize - (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMax))) + LeftWindowRect.right - LeftWindowRect.left;
				}
			}

			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val = GControlProperty(hWnd, CtrlVal);
			if (LOWORD(lParam) < BarPosition) {
				if (Min < Max)
					Val -= GControlProperty(hWnd, CtrlLgUpdate);
				else
					Val += GControlProperty(hWnd, CtrlLgUpdate);
				SetTimer(hWnd, 1, 500, NULL);
			} else if (LOWORD(lParam) >= BarPosition + BarSize) {
				if (Min < Max)
					Val += GControlProperty(hWnd, CtrlLgUpdate);
				else
					Val -= GControlProperty(hWnd, CtrlLgUpdate);
				SetTimer(hWnd, 2, 500, NULL);
			} else {
				Dragging = TRUE;
				OffsetX = LOWORD(lParam) - BarPosition;
			}
			if (Min < Max) {
				if (Val < Min) Val = Min;
				if (Val > Max) Val = Max;
			} else {
				if (Val < Max) Val = Max;
				if (Val > Min) Val = Min;
			}
			GControlProperty(hWnd, Val, CtrlVal);
			InvalidateRect(hWnd, NULL, TRUE);
			SetCapture(hWnd);
		}
		return 0;
	case GSM_GETDRAGVALUE:
		{
			if (Dragging) {
				RECT LeftWindowRect, RightWindowRect, WindowRect;
				LONG32 BarSize, BarPosition;
				LONG32 Min = GControlProperty(hWnd, CtrlMin);
				LONG32 Max = GControlProperty(hWnd, CtrlMax);
				LONG32 Difference;
				if (Max > Min) {
					Difference = Max - Min;
				} else {
					Difference = Min - Max;
				}
				double AbBarSize;
				GetWindowRect(hWndLeft, &LeftWindowRect);
				GetWindowRect(hWndRight, &RightWindowRect);
				GetWindowRect(hWnd, &WindowRect);
				//Get the coords of the bar:
				BarSize = WindowRect.right - WindowRect.left - RightWindowRect.right + RightWindowRect.left - LeftWindowRect.right + LeftWindowRect.left;
				if (Difference) {
					AbBarSize = (double)BarSize / (double)(Difference + 1);
					if (AbBarSize < 0) AbBarSize *= -1.0; //If Min > Max, make it positive
				} else { //Otherwise Divide by zero error would occur
					AbBarSize = RightWindowRect.left - LeftWindowRect.right;
				}
				BarSize = (LONG32)AbBarSize;
				if (BarSize < MinBarWidth) BarSize = MinBarWidth;
				POINT MouseCoords;
				GetCursorPos(&MouseCoords);
				BarPosition = MouseCoords.x - LeftWindowRect.right - OffsetX;
				if (BarPosition < 0) BarPosition = 0;
				if (BarPosition + BarSize > RightWindowRect.left - LeftWindowRect.right) BarPosition = RightWindowRect.left - LeftWindowRect.right - BarSize;
				LONG32 Val;
				if (Difference) {
					Val = (LONG32)( ( BarPosition + AbBarSize / 2.0 ) * Difference / ( RightWindowRect.left - LeftWindowRect.right - BarSize ) );
					if (Max > Min) {
						Val += Min;
					} else if (Max < Min) {
						Val = Difference - Val + Max;
					}
				} else {
					Val = Min;
				}
				return Val;
			}
		}
		return GControlProperty(hWnd, CtrlVal);
	case WM_LBUTTONUP:
		{
			if (Dragging) {
				GControlProperty(hWnd, (LONG32)SendMessage(hWnd, GSM_GETDRAGVALUE, 0, 0), CtrlVal);
				SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GSN_SCROLL), (LPARAM)hWnd);
			}
			Dragging = FALSE;
			ReleaseCapture();
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case WM_MOUSEMOVE:
		if (Dragging) {
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GSN_BARDRAG), (LPARAM)hWnd);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case WM_CAPTURECHANGED:
		Dragging = FALSE;
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_TIMER:
		{
			KillTimer(hWnd, 1);
			KillTimer(hWnd, 2);
			//Get the coords of the bar:
			RECT LeftWindowRect, RightWindowRect, WindowRect;
			GetWindowRect(hWndLeft, &LeftWindowRect);
			GetWindowRect(hWndRight, &RightWindowRect);
			GetWindowRect(hWnd, &WindowRect);
			LONG32 Difference = GControlProperty(hWnd, CtrlMax) - GControlProperty(hWnd, CtrlMin);
			if (Difference < 0) Difference *= -1;
			LONG32 BarPosition = LeftWindowRect.right - LeftWindowRect.left;
			LONG32 BarSize = WindowRect.right - WindowRect.left - RightWindowRect.right + RightWindowRect.left - BarPosition;
			if (Difference) { //Otherwise Divide by zero error would occur
				BarPosition = BarSize;
				BarSize = (LONG32)((double)BarSize / (double)Difference);
				if (BarSize < 0) BarSize *= -1; //If Min > Max, make it positive
				if (BarSize < MinBarWidth) BarSize = MinBarWidth;
				if (GControlProperty(hWnd, CtrlMin) < GControlProperty(hWnd, CtrlMax)) {
					BarPosition = (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMin)) + LeftWindowRect.right - LeftWindowRect.left);
				} else {
					BarPosition = RightWindowRect.left - LeftWindowRect.right - BarSize - (LONG32)((double)(BarPosition - BarSize) / (double)Difference * (double)(GControlProperty(hWnd, CtrlVal) - GControlProperty(hWnd, CtrlMax))) + LeftWindowRect.right - LeftWindowRect.left;
				}
			}

			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val = GControlProperty(hWnd, CtrlVal);
			BOOL SendParentMsg = FALSE;
			POINT CursorPos;
			GetCursorPos(&CursorPos);
			if (GetKeyState(VK_LBUTTON) & 0x80000000) {
				if (CursorPos.y >= WindowRect.top && CursorPos.y < WindowRect.bottom && CursorPos.x >= LeftWindowRect.right && CursorPos.x < RightWindowRect.left) {
					if (CursorPos.x < WindowRect.left + BarPosition && wParam == 1) {
						if (Min < Max)
							Val -= GControlProperty(hWnd, CtrlLgUpdate);
						else
							Val += GControlProperty(hWnd, CtrlLgUpdate);
						SendParentMsg = TRUE;
					} else if (CursorPos.x >= WindowRect.left + BarPosition + BarSize && wParam == 2) {
						if (Min < Max)
							Val += GControlProperty(hWnd, CtrlLgUpdate);
						else
							Val -= GControlProperty(hWnd, CtrlLgUpdate);
						SendParentMsg = TRUE;
					}
					if (Min < Max) {
						if (Val < Min) Val = Min;
						if (Val > Max) Val = Max;
					} else {
						if (Val < Max) Val = Max;
						if (Val > Min) Val = Min;
					}
					GControlProperty(hWnd, Val, CtrlVal);
					if (SendParentMsg) SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GSN_SCROLL), (LPARAM)hWnd);
					InvalidateRect(hWnd, NULL, TRUE);
				}
				SetTimer(hWnd, wParam, 50, NULL);
			}
		}
		return 0;
	case WM_COMMAND:
		{
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val = GControlProperty(hWnd, CtrlVal);
			BOOL SendParentMsg = FALSE;
			if ((HWND)lParam == hWndLeft) {
				if (HIWORD(wParam) == GBN_LBUTTONDOWN) {
					if (Min < Max)
						Val -= GControlProperty(hWnd, CtrlSmUpdate);
					else
						Val += GControlProperty(hWnd, CtrlSmUpdate);
					SendParentMsg = TRUE;
				}
			} else if ((HWND)lParam == hWndRight) {
				if (HIWORD(wParam) == GBN_LBUTTONDOWN) {
					if (Min < Max)
						Val += GControlProperty(hWnd, CtrlSmUpdate);
					else
						Val -= GControlProperty(hWnd, CtrlSmUpdate);
					SendParentMsg = TRUE;
				}
			}
			if (Min < Max) {
				if (Val < Min) Val = Min;
				if (Val > Max) Val = Max;
			} else {
				if (Val < Max) Val = Max;
				if (Val > Min) Val = Min;
			}
			GControlProperty(hWnd, Val, CtrlVal);
			if (SendParentMsg) SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), GSN_SCROLL), (LPARAM)hWnd);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case GSM_SETVALUE:
		{
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val = (LONG32)lParam;
			if (Min < Max) {
				if (Val < Min) Val = Min;
				if (Val > Max) Val = Max;
			} else {
				if (Val < Max) Val = Max;
				if (Val > Min) Val = Min;
			}
			GControlProperty(hWnd, Val, CtrlVal);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case GSM_GETVALUE:
		return GControlProperty(hWnd, CtrlVal);
	case GSM_SETMIN:
		{
			LONG32 Min = (LONG32)lParam;
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val = GControlProperty(hWnd, CtrlVal);
			if (Min < Max) {
				if (Val < Min) Val = Min;
				if (Val > Max) Val = Max;
			} else {
				if (Val < Max) Val = Max;
				if (Val > Min) Val = Min;
			}
			GControlProperty(hWnd, Min, CtrlMin);
			GControlProperty(hWnd, Val, CtrlVal);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case GSM_SETMAX:
		{
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = (LONG32)lParam;
			LONG32 Val = GControlProperty(hWnd, CtrlVal);
			if (Min < Max) {
				if (Val < Min) Val = Min;
				if (Val > Max) Val = Max;
			} else {
				if (Val < Max) Val = Max;
				if (Val > Min) Val = Min;
			}
			GControlProperty(hWnd, Max, CtrlMax);
			GControlProperty(hWnd, Val, CtrlVal);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case GSM_SETSMALLUPDATE:
		GControlProperty(hWnd, (LONG32)lParam, CtrlSmUpdate);
		return 0;
	case GSM_SETLARGEUPDATE:
		GControlProperty(hWnd, (LONG32)lParam, CtrlLgUpdate);
		return 0;
	case GSM_GETUPLEFTHWND:
		return (LRESULT)hWndLeft;
	case GSM_GETDOWNRIGHTHWND:
		return (LRESULT)hWndRight;
	//Colours:
	case GSM_SETFACECOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrFace);
		SendMessage(hWndLeft, GBM_SETFACECOLOUR, wParam, lParam);
		SendMessage(hWndRight, GBM_SETFACECOLOUR, wParam, lParam);
		return 0;
	case GSM_SETDARKSHADOWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrDarkShadow);
		SendMessage(hWndLeft, GBM_SETDARKSHADOWCOLOUR, wParam, lParam);
		SendMessage(hWndRight, GBM_SETDARKSHADOWCOLOUR, wParam, lParam);
		return 0;
	case GSM_SETSHADOWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrShadow);
		SendMessage(hWndLeft, GBM_SETSHADOWCOLOUR, wParam, lParam);
		SendMessage(hWndRight, GBM_SETSHADOWCOLOUR, wParam, lParam);
		return 0;
	case GSM_SETEXTRAHIGHLIGHTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrExtraHighlight);
		SendMessage(hWndLeft, GBM_SETEXTRAHIGHLIGHTCOLOUR, wParam, lParam);
		SendMessage(hWndRight, GBM_SETEXTRAHIGHLIGHTCOLOUR, wParam, lParam);
		return 0;
	case GSM_SETHIGHLIGHTCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrHighlight);
		SendMessage(hWndLeft, GBM_SETHIGHLIGHTCOLOUR, wParam, lParam);
		SendMessage(hWndRight, GBM_SETHIGHLIGHTCOLOUR, wParam, lParam);
		return 0;
	case GSM_SETARROWCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrText);
		SendMessage(hWndLeft, GBM_SETTEXTCOLOUR, wParam, lParam);
		SendMessage(hWndRight, GBM_SETTEXTCOLOUR, wParam, lParam);
		return 0;
	case GSM_SETTRACKCOLOUR:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrActive);
		return 0;
	case GSM_SETTRACKIMAGE:
		{
			LPVOID BmpTrack = GControlProperty(hWnd, CtrlTitleBmp);
			if (!BmpTrack) {
				BmpTrack = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpTrack, CtrlTitleBmp);
			}
			((BitmapOps*)BmpTrack)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
		case GSM_SETBARTOPIMAGE:
		{
			LPVOID BmpTop = GControlProperty(hWnd, CtrlBmpOff);
			if (!BmpTop) {
				BmpTop = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpTop, CtrlBmpOff);
			}
			((BitmapOps*)BmpTop)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
		case GSM_SETBARIMAGE:
		{
			LPVOID BmpBar = GControlProperty(hWnd, CtrlBmpOver);
			if (!BmpBar) {
				BmpBar = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpBar, CtrlBmpOver);
			}
			((BitmapOps*)BmpBar)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
		case GSM_SETBARBOTTOMIMAGE:
		{
			LPVOID BmpBottom = GControlProperty(hWnd, CtrlBmpOn);
			if (!BmpBottom) {
				BmpBottom = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpBottom, CtrlBmpOn);
			}
			((BitmapOps*)BmpBottom)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case WM_CLOSE:
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		KillTimer(hWnd, 2);
		ControlDestroyed(hWnd);
		break;
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}





/*****************************************************************/
/**************************PROGRESS BAR***************************/
/*****************************************************************/





LRESULT CALLBACK GHProgressProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch (Message) {
	case WM_CREATE:
		{
			if (!ControlCreated(hWnd)) return -1;
			SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		}
		break;
	case WM_ERASEBKGND:
		{
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			HDC hDcD = GetDC(hWnd);
			HDC hDc = CreateCompatibleDC(hDcD);
			HBITMAP hBitmap = CreateCompatibleBitmap(hDcD, ClientRect.right, ClientRect.bottom);
			
			BitBlt(hDcD, 0, 0, ClientRect.right, ClientRect.bottom, hDc, 0, 0, SRCCOPY);
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDc, hBitmap);
			SelectObject(hDc, hOldBitmap);
			DeleteObject(hBitmap);
			DeleteDC(hDc);
			ReleaseDC(hWnd, hDcD);
		}
		return 0;
	case GPM_SETBARIMAGE0: //background
		{
			LPVOID BmpTmp = GControlProperty(hWnd, CtrlTitleBmp);
			if (!BmpTmp) {
				BmpTmp = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpTmp, CtrlTitleBmp);
			}
			((BitmapOps*)BmpTmp)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case GPM_SETBARIMAGE1:
		{
			LPVOID BmpTmp = GControlProperty(hWnd, CtrlBmpOff);
			if (!BmpTmp) {
				BmpTmp = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpTmp, CtrlBmpOff);
			}
			((BitmapOps*)BmpTmp)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case GPM_SETBARIMAGE2:
		{
			LPVOID BmpTmp = GControlProperty(hWnd, CtrlBmpOver);
			if (!BmpTmp) {
				BmpTmp = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpTmp, CtrlBmpOver);
			}
			((BitmapOps*)BmpTmp)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case GPM_SETBARIMAGE3:
		{
			LPVOID BmpTmp = GControlProperty(hWnd, CtrlBmpOn);
			if (!BmpTmp) {
				BmpTmp = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpTmp, CtrlBmpOn);
			}
			((BitmapOps*)BmpTmp)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case GPM_SETBARIMAGE4:
		{
			LPVOID BmpTmp = GControlProperty(hWnd, CtrlBmpDisabled);
			if (!BmpTmp) {
				BmpTmp = new BitmapOps(1, 1);
				GControlProperty(hWnd, (LPVOID)BmpTmp, CtrlBmpDisabled);
			}
			((BitmapOps*)BmpTmp)->GetBitmapFromResource((WORD)lParam);
		}
		return 0;
	case GPM_SETCOLOUR0: //Background
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrFace);
		return 0;
	case GPM_SETCOLOUR1:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrDarkShadow);
		return 0;
	case GPM_SETCOLOUR2:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrShadow);
		return 0;
	case GPM_SETCOLOUR3:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrHighlight);
		return 0;
	case GPM_SETCOLOUR4:
		GControlProperty(hWnd, (COLORREF)lParam, CtrlClrExtraHighlight);
		return 0;
	case GPM_SETMIN:
		{
			GControlProperty(hWnd, (LONG32)lParam, CtrlMin);
			LONG32 Min = lParam;
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			LONG32 Val[4] = {GControlProperty(hWnd, CtrlVal), GControlProperty(hWnd, CtrlStartX), GControlProperty(hWnd, CtrlStartY), GControlProperty(hWnd, CtrlTargetX)};
			if (Min < Max) {
				for (int i = 0; i < 4; i++) {
					Val[i] = (Val[i] < Min ? Min : (Val[i] > Max ? Max : Val[i]));
				}
			} else {
				for (int i = 0; i < 4; i++) {
					Val[i] = (Val[i] > Min ? Min : (Val[i] < Max ? Max : Val[i]));
				}
			}
			GControlProperty(hWnd, (LONG32)Val[0], CtrlVal);
			GControlProperty(hWnd, (LONG32)Val[1], CtrlStartX);
			GControlProperty(hWnd, (LONG32)Val[2], CtrlStartY);
			GControlProperty(hWnd, (LONG32)Val[3], CtrlTargetX);
		}
		return 0;
	case GPM_SETMAX:
		{
			GControlProperty(hWnd, (LONG32)lParam, CtrlMax);
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = lParam;
			LONG32 Val[4] = {GControlProperty(hWnd, CtrlVal), GControlProperty(hWnd, CtrlStartX), GControlProperty(hWnd, CtrlStartY), GControlProperty(hWnd, CtrlTargetX)};
			if (Min < Max) {
				for (int i = 0; i < 4; i++) {
					Val[i] = (Val[i] < Min ? Min : (Val[i] > Max ? Max : Val[i]));
				}
			} else {
				for (int i = 0; i < 4; i++) {
					Val[i] = (Val[i] > Min ? Min : (Val[i] < Max ? Max : Val[i]));
				}
			}
			GControlProperty(hWnd, (LONG32)Val[0], CtrlVal);
			GControlProperty(hWnd, (LONG32)Val[1], CtrlStartX);
			GControlProperty(hWnd, (LONG32)Val[2], CtrlStartY);
			GControlProperty(hWnd, (LONG32)Val[3], CtrlTargetX);
		}
		return 0;
	case GPM_SETVALUE1:
		{
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			if (Min < Max) {
				GControlProperty(hWnd, (LONG32)((LONG32)lParam < Min ? Min : ((LONG32)lParam > Max ? Max : (LONG32)lParam)), CtrlVal);
			} else {
				GControlProperty(hWnd, (LONG32)((LONG32)lParam > Min ? Min : ((LONG32)lParam < Max ? Max : (LONG32)lParam)), CtrlVal);
			}
		}
		return 0;
	case GPM_SETVALUE2:
		{
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			if (Min < Max) {
				GControlProperty(hWnd, (LONG32)((LONG32)lParam < Min ? Min : ((LONG32)lParam > Max ? Max : (LONG32)lParam)), CtrlStartX);
			} else {
				GControlProperty(hWnd, (LONG32)((LONG32)lParam > Min ? Min : ((LONG32)lParam < Max ? Max : (LONG32)lParam)), CtrlStartX);
			}
		}
		return 0;
	case GPM_SETVALUE3:
		{
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			if (Min < Max) {
				GControlProperty(hWnd, (LONG32)((LONG32)lParam < Min ? Min : ((LONG32)lParam > Max ? Max : (LONG32)lParam)), CtrlStartY);
			} else {
				GControlProperty(hWnd, (LONG32)((LONG32)lParam > Min ? Min : ((LONG32)lParam < Max ? Max : (LONG32)lParam)), CtrlStartY);
			}
		}
		return 0;
	case GPM_SETVALUE4:
		{
			LONG32 Min = GControlProperty(hWnd, CtrlMin);
			LONG32 Max = GControlProperty(hWnd, CtrlMax);
			if (Min < Max) {
				GControlProperty(hWnd, (LONG32)((LONG32)lParam < Min ? Min : ((LONG32)lParam > Max ? Max : (LONG32)lParam)), CtrlTargetX);
			} else {
				GControlProperty(hWnd, (LONG32)((LONG32)lParam > Min ? Min : ((LONG32)lParam < Max ? Max : (LONG32)lParam)), CtrlTargetX);
			}
		}
		return 0;
	case WM_CLOSE:
		break;
	case WM_DESTROY:
		ControlDestroyed(hWnd);
		break;
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}
