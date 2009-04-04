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
#include "SunProject.h"
#include "Resource.h"
#include <windows.h>
#include <commctrl.h>
#include <string>


LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PrevWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ViewWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BufferWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK NebulaDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GradientDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK NewDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MergeDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AnimDlgProc(HWND, UINT, WPARAM, LPARAM);

SunProjectNebulaParams NebulaParams = {0};
SunProjectNebulaParams FlareParams = {0};
SunProjectNebulaParams * PerlinParams = NULL;
COLORREF CustomColourArray[16] = {0};

HBRUSH hBrushNothing = NULL;

enum StarSettingStates {None = 0, Positioning = 1, StarSizing = 2, CoronaSizing = 3};

static HWND hWndView = NULL;

struct StarStruct {
	enum StarSettingStates SettingState;
	LONG32 XPos;
	LONG32 YPos;
	LONG32 StarRadius;
	LONG32 CoronaRadius;
	COLORREF StarColour;
	BOOL ExtPreviewable;
	BOOL ExtBufferPreview;
	BOOL UseExtBuffer;
	BitmapOps *ExternalBuffer;
};

BitmapOps StarBuff(1, 1, TRUE);

StarStruct CurStar = {None, 0, 0, 0, 0, 0, FALSE, FALSE, TRUE, &StarBuff};
//StarStruct CurStar = {None, 0, 0, 0, 0, 0, FALSE, FALSE, FALSE, NULL};

int MenuSpeed = 20;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	MultiSoftGControlsInit(hInstance);
	WNDCLASSEX WndClass = {0};
	//LOGBRUSH background = {0};
	MSG Msg = {0};

	//background.lbStyle = BS_PATTERN;
	//background.lbHatch = (long)LoadImage(hInstance, MAKEINTRESOURCE(IDB_STARFIELDBACK), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

	WndClass.cbSize = sizeof(WNDCLASSEX);
	//WndClass.hbrBackground = CreateBrushIndirect(&background);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MULTISOFT));
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = MainWndProc;
	WndClass.lpszClassName = "MultiSoftSunProjectMain";
	WndClass.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClassEx(&WndClass);

	//WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpfnWndProc = ViewWndProc;
	WndClass.lpszClassName = "MultiSoftSunProjectView";

	RegisterClassEx(&WndClass);

	WndClass.lpfnWndProc = BufferWndProc;
	WndClass.lpszClassName = "MultiSoftSunProjectBuffer";

	RegisterClassEx(&WndClass);

	HWND hWnd = CreateWindowEx(WS_EX_CONTROLPARENT, "MultiSoftSunProjectMain", "The Sun Project Interactive", WS_POPUP | /*WS_THICKFRAME */WS_DLGFRAME /*| WS_CAPTION*/ | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, GetSystemMetrics(SM_CXSCREEN) / 2 - 320, GetSystemMetrics(SM_CYSCREEN) / 2 - 240, 640, 480, NULL, NULL, hInstance, NULL);
	if (!hWnd) {
		UnregisterClass("MultiSoftSunProjectBuffer", hInstance);
		UnregisterClass("MultiSoftSunProjectView", hInstance);
		UnregisterClass("MultiSoftSunProjectMain", hInstance);
		MultiSoftGControlsDeInit();
		return EXIT_FAILURE;
	}

	//Get the current resolution:
	BOOL ResChanged = FALSE;
	DEVMODE dMode = {0};
	EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &dMode);
	if (dMode.dmBitsPerPel < 24) {
		if (MessageBox(NULL, "This program looks best in true-colour.\nWould you like to change into true colour now?\n\n(The resolution will be reverted when you quit the program)", "Change Resolution?", MB_YESNO | MB_ICONQUESTION) == IDYES) {
			//Try 32BPP:
			dMode.dmBitsPerPel = 32;
			if (ChangeDisplaySettings(&dMode, CDS_TEST) == DISP_CHANGE_SUCCESSFUL) {
				if (ChangeDisplaySettings(&dMode, 0) == DISP_CHANGE_SUCCESSFUL) ResChanged = TRUE;
			} else {
				//Try 24BPP:
				dMode.dmBitsPerPel = 24;
				if (ChangeDisplaySettings(&dMode, CDS_TEST) == DISP_CHANGE_SUCCESSFUL) {
					if (ChangeDisplaySettings(&dMode, 0) == DISP_CHANGE_SUCCESSFUL) ResChanged = TRUE;
				} else {
					MessageBox(NULL, "The resolution wan unable to be changed into true-colour", "Error changing resolution!", MB_ICONEXCLAMATION);
				}
			}
		}
	}


	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	LOGBRUSH  lBrush = {0};
	lBrush.lbStyle = BS_PATTERN;
	lBrush.lbHatch = (LONG)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BRUSHNOTHING), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hBrushNothing = CreateBrushIndirect(&lBrush);

	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		if (Msg.message == WM_QUIT) break;
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		if (!bRunning) break; //Otherwise the program may just keep waiting for another message that just won't come...
	}

	if (ResChanged) ChangeDisplaySettings(NULL, 0);

	DeleteObject(hBrushNothing);

	UnregisterClass("MultiSoftSunProjectBuffer", hInstance);
	UnregisterClass("MultiSoftSunProjectView", hInstance);
	UnregisterClass("MultiSoftSunProjectMain", hInstance);

	MultiSoftGControlsDeInit();
	return Msg.wParam;
}



LRESULT CALLBACK MainWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	static HWND hWndTitle = NULL;
	static HWND hWndNew = NULL;
	static HWND hWndRandom = NULL;
	static HWND hWndUndoRedo = NULL;
	static HWND hWndAbort = NULL;
	static HWND hWndBufferBtn = NULL;
	static HWND hWndBufferCon = NULL;
		static HWND hWndBufferHide = NULL;
		static HWND hWndBufferSave = NULL;
		static HWND hWndBufferLoad = NULL;
		static HWND hWndBufferSwap = NULL;
		static HWND hWndBuffOverlayImage = NULL;
		static HWND hWndBuffUnderlayImage = NULL;
		static HWND hWndBuffMergeImages = NULL;
		static HWND hWndViewBuffer = NULL;
	static HWND hWndViewImage = NULL;
	static HWND hWndOpen = NULL;
	static HWND hWndSave = NULL;
	static HWND hWndHelp = NULL;
	static HWND hWndExit = NULL;
	static HWND hWndBackground = NULL;
	static HWND hWndBackCon = NULL;
		static HWND hWndBackHide = NULL;
		static HWND hWndBackSolid = NULL;
		static HWND hWndBackNebula = NULL;
		static HWND hWndBackStarfield = NULL;
			static HWND hWndBackNebCon = NULL;
			static HWND hWndBackNebGrad = NULL;
			static HWND hWndBackNebAnim = NULL;
			static HWND hWndBackNebRand = NULL;
			static HWND hWndBackNebAdvanced = NULL;
			static HWND hWndBackNebCmdDraw = NULL;
			/*static HWND hWndBackNebLblNumber = NULL;	
			static HWND hWndBackNebScrNumber = NULL;
			static HWND hWndBackNebLblCurrent = NULL;
			static HWND hWndBackNebScrCurrent = NULL;
			static HWND hWndBackNebLblErase = NULL;
			static HWND hWndBackNebOptEraseYes = NULL;
			static HWND hWndBackNebOptEraseUnderlay = NULL;
			static HWND hWndBackNebOptEraseNo = NULL;
			static HWND hWndBackNebCmdMiddleRGB = NULL;
			static HWND hWndBackNebLblSeed = NULL;
			static HWND hWndBackNebTxtSeed = NULL;
			static HWND hWndBackNebLblPersistance = NULL;
			static HWND hWndBackNebTxtPersistance = NULL;
			static HWND hWndBackNebLblOctaves = NULL;
			static HWND hWndBackNebTxtOctaves = NULL;
			static HWND hWndBackNebLblVariance = NULL;
			static HWND hWndBackNebLblRedVariance = NULL;
			static HWND hWndBackNebTxtRedVariance = NULL;
			static HWND hWndBackNebLblGreenVariance = NULL;
			static HWND hWndBackNebTxtGreenVariance = NULL;
			static HWND hWndBackNebLblBlueVariance = NULL;
			static HWND hWndBackNebTxtBlueVariance = NULL;
			static HWND hWndBackNebChkSmooth = NULL;
			static HWND hWndBackNebChkTile = NULL;*/
	static HWND hWndStar = NULL;
	static HWND hWndStarCon = NULL;
		static HWND hWndStarHide = NULL;
		static HWND hWndStarBasics = NULL;
			static HWND hWndStarBasCon = NULL;
			static HWND hWndStarBasPosition = NULL;
			static HWND hWndStarBasStarSize = NULL;
			static HWND hWndStarBasCoronaSize = NULL;
			static HWND hWndStarBasStarColour = NULL;
			static HWND hWndStarBasExport = NULL;
			static HWND hWndStarBasUnlock = NULL;
			static HWND hWndStarBasRandom = NULL;
			static HWND hWndStarBasAdvanced = NULL;
			static HWND hWndStarBasDraw = NULL;
		static HWND hWndStarSurface = NULL;
			static HWND hWndStarSurCon = NULL;
			static HWND hWndStarSurScrAmt = NULL;
			static HWND hWndStarSurPrvAmt = NULL;
			static HWND hWndStarSurRandom = NULL;
			static HWND hWndStarSurUnlock = NULL;
			static HWND hWndStarSurDraw = NULL;
		static HWND hWndStarFlares = NULL;
			static HWND hWndStarFlrCon = NULL;
			static HWND hWndStarFlrGrad = NULL;
			//static HWND hWndStarFlrAnim = NULL;
			static HWND hWndStarFlrRand = NULL;
			static HWND hWndStarFlrAdvanced = NULL;
			static HWND hWndStarFlrUnlock = NULL;
			static HWND hWndStarFlrDraw = NULL;
		static HWND hWndStarClear = NULL;
		static HWND hWndStarFinish = NULL;
	static HWND hWndMiscFilters = NULL;
	static HWND hWndMisFilCon = NULL;
		static HWND hWndMisFilHide = NULL;
		static HWND hWndMisFilSmooth = NULL;

	//static HWND hWndView = NULL;
	static HWND hWndBuffer = NULL;
	static HWND hWndPreview = NULL;
	static HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);

	static BitmapOps WindowBackground(1, 1);
	

	//Visibility flags:
	static BOOL BufferVisible = FALSE;
	static BOOL BackgroundVisible = FALSE;
	static BOOL StarVisible = FALSE;
	static BOOL MiscFXVisible = FALSE;
	static BOOL NebulaVisible = FALSE;
	static BOOL StarBasVisible = FALSE;
	static BOOL StarSurVisible = FALSE;
	static BOOL StarFlrVisible = FALSE;
	switch(Message) {
	case WM_CREATE:
		{
			//Create all other windows and objects:
			//Titlebar:
			hWndTitle = CreateWindowEx(NULL, "MultiSoftGTitle", "The Sun Project Interactive", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 0, 0, hWnd, NULL, hInstance, NULL);
			if (!hWndTitle) return -1;
			//Containers:
			hWndBufferCon = CreateWindowEx(NULL, "MultiSoftGWndCon", "Buffer", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 3/*92*/, 480, /*91*/180, /*289*/166, hWnd, NULL, hInstance, NULL);
			if (!hWndBufferCon) return -1;
			hWndBackCon = CreateWindowEx(NULL, "MultiSoftGWndCon", "Background", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 92, -168, 91, 168, hWnd, NULL, hInstance, NULL);
			if (!hWndBackCon) return -1;
			hWndStarCon = CreateWindowEx(NULL, "MultiSoftGWndCon", "Star", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 92, -250, 91, /*168*/250, hWnd, NULL, hInstance, NULL);
			if (!hWndStarCon) return -1;
			hWndMisFilCon = CreateWindowEx(NULL, "MultiSoftGWndCon", "Misc Filters", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 92, -84, 91, 84, hWnd, NULL, hInstance, NULL);
			if (!hWndMisFilCon) return -1;
			hWndBackNebCon = CreateWindowEx(NULL, "MultiSoftGWndCon", "Nebula", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 640, 24, 440, 129, hWnd, NULL, hInstance, NULL);
			if (!hWndBackNebCon) return -1;
			hWndStarBasCon = CreateWindowEx(NULL, "MultiSoftGWndCon", "Star Basics", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 640, 24, 440, 129, hWnd, NULL, hInstance, NULL);
			if (!hWndStarBasCon) return -1;
			hWndStarSurCon = CreateWindowEx(NULL, "MultiSoftGWndCon", "Star Surface", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 640, 24, 440, 129, hWnd, NULL, hInstance, NULL);
			if (!hWndStarBasCon) return -1;
			hWndStarFlrCon = CreateWindowEx(NULL, "MultiSoftGWndCon", "Star Flares", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 640, 24, 440, 129, hWnd, NULL, hInstance, NULL);
			if (!hWndStarBasCon) return -1;
			//Buttons:
			hWndNew = CreateWindowEx(NULL, "MultiSoftGButton", "Start a new image", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 8, 24, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndNew) return -1;
			hWndRandom = CreateWindowEx(NULL, "MultiSoftGButton", "Generate a new random image", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 8, 65, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndRandom) return -1;
			hWndUndoRedo = CreateWindowEx(NULL, "MultiSoftGButton", "Undo or Redo the last change made to the image", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 8, 106, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndUndoRedo) return -1;
			hWndAbort = CreateWindowEx(NULL, "MultiSoftGButton", "Abort the current drawing operation", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 8, 147, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndAbort) return -1;
			hWndBufferBtn = CreateWindowEx(NULL, "MultiSoftGButton", "View the buffer controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 8, 311/*188*/, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndBufferBtn) return -1;
				hWndBufferHide = CreateWindowEx(NULL, "MultiSoftGButton", "Hide the buffer controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 2, 81, 33, hWndBufferCon, NULL, hInstance, NULL);
				if (!hWndBufferHide) return -1;	
				hWndBufferSave = CreateWindowEx(NULL, "MultiSoftGButton", "Store the image in the buffer", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, /*2*/43, 81, 33, hWndBufferCon, NULL, hInstance, NULL);
				if (!hWndBufferSave) return -1;
				hWndBufferLoad = CreateWindowEx(NULL, "MultiSoftGButton", "Restore the image from the buffer", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, /*43*/84, 81, 33, hWndBufferCon, NULL, hInstance, NULL);
				if (!hWndBufferLoad) return -1;
				hWndBufferSwap = CreateWindowEx(NULL, "MultiSoftGButton", "Swap the image with the buffer", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, /*84*/125, 81, 33, hWndBufferCon, NULL, hInstance, NULL);
				if (!hWndBufferSwap) return -1;
				hWndViewBuffer = CreateWindowEx(NULL, "MultiSoftGButton", "View the buffer in a seperate window", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, /*2*/91, /*248*/2, 81, 33, hWndBufferCon, NULL, hInstance, NULL);
				if (!hWndViewBuffer) return -1;
				hWndBuffOverlayImage = CreateWindowEx(NULL, "MultiSoftGButton", "Underlay the image with the buffer", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, /*2*/91, /*125*/43, 81, 33, hWndBufferCon, NULL, hInstance, NULL);
				if (!hWndBuffOverlayImage) return -1;
				hWndBuffUnderlayImage = CreateWindowEx(NULL, "MultiSoftGButton", "Overlay the image with the buffer", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, /*2*/91, /*166*/84, 81, 33, hWndBufferCon, NULL, hInstance, NULL);
				if (!hWndBuffUnderlayImage) return -1;
				hWndBuffMergeImages = CreateWindowEx(NULL, "MultiSoftGButton", "Merge the image and the buffer", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, /*2*/91, /*207*/125, 81, 33, hWndBufferCon, NULL, hInstance, NULL);
				if (!hWndBuffMergeImages) return -1;
			hWndViewImage = CreateWindowEx(NULL, "MultiSoftGButton", "View the image at its actual size", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 8, 188/*311*/, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndBufferSwap) return -1;
			hWndOpen = CreateWindowEx(NULL, "MultiSoftGButton", "Open an image", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 8, 229/*352*/, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndOpen) return -1;
			hWndSave = CreateWindowEx(NULL, "MultiSoftGButton", "Save the image", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 8, 270/*393*/, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndSave) return -1;
			hWndHelp = CreateWindowEx(NULL, "MultiSoftGButton", "Show the help file", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 8, 393, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndHelp) return -1;
			hWndExit = CreateWindowEx(NULL, "MultiSoftGButton", "Exit the program", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 8, 434, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndExit) return -1;
			hWndBackground = CreateWindowEx(NULL, "MultiSoftGButton", "Display the background controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 97, 24, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndBackground) return -1;
				hWndBackHide = CreateWindowEx(NULL, "MultiSoftGButton", "Hide the background controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 2, 81, 33, hWndBackCon, NULL, hInstance, NULL);
				if (!hWndBackHide) return -1;	
				hWndBackSolid = CreateWindowEx(NULL, "MultiSoftGButton", "Display the solid colour controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 43, 81, 33, hWndBackCon, NULL, hInstance, NULL);
				if (!hWndBackSolid) return -1;
				hWndBackNebula = CreateWindowEx(NULL, "MultiSoftGButton", "Display the nebula drawing controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 84, 81, 33, hWndBackCon, NULL, hInstance, NULL);
				if (!hWndBackNebula) return -1;	
				hWndBackStarfield = CreateWindowEx(NULL, "MultiSoftGButton", "Display the Star - Field drawing controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 125, 81, 33, hWndBackCon, NULL, hInstance, NULL);
				if (!hWndBackStarfield) return -1;
					//Gradient button:
					hWndBackNebGrad = CreateWindowEx(NULL, "MultiSoftGButton", "Show the gradient controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 4, 4, 81, 33, hWndBackNebCon, NULL, hInstance, NULL);
					if (!hWndBackNebGrad) return -1;
					//Animation button:
					hWndBackNebAnim = CreateWindowEx(NULL, "MultiSoftGButton", "Show the animation controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10 - 89, 86, 81, 33, hWndBackNebCon, NULL, hInstance, NULL);
					if (!hWndBackNebAnim) return -1;
					//Random button:
					hWndBackNebRand = CreateWindowEx(NULL, "MultiSoftGButton", "Randomise nebula parameteres", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 4, 81, 33, hWndBackNebCon, NULL, hInstance, NULL);
					if (!hWndBackNebRand) return -1;
					//Advanced button:
					hWndBackNebAdvanced = CreateWindowEx(NULL, "MultiSoftGButton", "Advanced nebula parameters", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 45, 81, 33, hWndBackNebCon, NULL, hInstance, NULL);
					if (!hWndBackNebAdvanced) return -1;
					//Draw button:
					hWndBackNebCmdDraw = CreateWindowEx(NULL, "MultiSoftGButton", "Draw Nebula", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 86, 81, 33, hWndBackNebCon, NULL, hInstance, NULL);
					if (!hWndBackNebCmdDraw) return -1;
					/*//Layer number:
					hWndBackNebLblNumber = CreateWindowEx(NULL, "STATIC", "Layers: 1", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 4, 4, 72, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebLblNumber) return -1;
					hWndBackNebScrNumber = CreateWindowEx(NULL, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS, 4, 20, 72, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebScrNumber) return -1;
					hWndBackNebLblCurrent = CreateWindowEx(NULL, "STATIC", "Current: 1", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 4, 44, 72, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebLblCurrent) return -1;
					hWndBackNebScrCurrent = CreateWindowEx(NULL, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS, 4, 60, 72, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebScrCurrent) return -1;
					//Treatment of previous image:
					hWndBackNebLblErase = CreateWindowEx(NULL, "STATIC", "Previous:", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 84, 4, 80, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebLblErase) return -1;
					hWndBackNebOptEraseYes = CreateWindowEx(NULL, "BUTTON", "Erase", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP | WS_CLIPSIBLINGS, 84, 20, 80, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebOptEraseYes) return -1;
					hWndBackNebOptEraseUnderlay = CreateWindowEx(NULL, "BUTTON", "Underlay", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP | WS_CLIPSIBLINGS, 84, 36, 80, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebOptEraseUnderlay) return -1;
					hWndBackNebOptEraseNo = CreateWindowEx(NULL, "BUTTON", "Overlay", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP | WS_CLIPSIBLINGS, 84, 52, 80, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebOptEraseNo) return -1;
					//Middle Colour
					hWndBackNebCmdMiddleRGB = CreateWindowEx(NULL, "MultiSoftGButton", "Colour...", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | WS_CLIPSIBLINGS, 4, 84, 72, 24, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebCmdMiddleRGB) return -1;
					//Tweaks:
					hWndBackNebLblSeed = CreateWindowEx(NULL, "STATIC", "Seed:", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 172, 4, 72, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebLblSeed) return -1;
					hWndBackNebTxtSeed = CreateWindowEx(NULL, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_TABSTOP | WS_CLIPSIBLINGS, 172, 20, 72, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebTxtSeed) return -1;
					hWndBackNebLblPersistance = CreateWindowEx(NULL, "STATIC", "Persistence:", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 172, 44, 72, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebLblPersistance) return -1;
					hWndBackNebTxtPersistance = CreateWindowEx(NULL, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_TABSTOP | WS_CLIPSIBLINGS, 172, 60, 72, 16, hWndBackNebFrame, NULL, hInstance, NULL);
					if (!hWndBackNebTxtPersistance) return -1;

					//Variances:
					hWndBackNebLblVariance = CreateWindowEx(NULL, "STATIC", "Variance:", WS_CHILD | WS_VISIBLE | SS_RIGHT | WS_CLIPSIBLINGS, 426, 51, 65, 16, hWnd, NULL, hInstance, NULL);
					if (!hWndBackNebLblVariance) return -1;		
					hWndBackNebLblRedVariance = CreateWindowEx(NULL, "STATIC", "Red:", WS_CHILD | WS_VISIBLE | SS_RIGHT | WS_CLIPSIBLINGS, 426, 75, 65, 16, hWnd, NULL, hInstance, NULL);
					if (!hWndBackNebLblRedVariance) return -1;	
					hWndBackNebTxtRedVariance = CreateWindowEx(NULL, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_TABSTOP | WS_CLIPSIBLINGS, 499, 75, 32, 16, hWnd, NULL, hInstance, NULL);
					if (!hWndBackNebTxtRedVariance) return -1;
					hWndBackNebLblGreenVariance = CreateWindowEx(NULL, "STATIC", "Green:", WS_CHILD | WS_VISIBLE | SS_RIGHT | WS_CLIPSIBLINGS, 426, 99, 65, 16, hWnd, NULL, hInstance, NULL);
					if (!hWndBackNebLblGreenVariance) return -1;
					hWndBackNebTxtGreenVariance = CreateWindowEx(NULL, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_TABSTOP | WS_CLIPSIBLINGS, 499, 99, 32, 16, hWnd, NULL, hInstance, NULL);
					if (!hWndBackNebTxtGreenVariance) return -1;
					hWndBackNebLblBlueVariance = CreateWindowEx(NULL, "STATIC", "Blue:", WS_CHILD | WS_VISIBLE | SS_RIGHT | WS_CLIPSIBLINGS, 426, 123, 65, 16, hWnd, NULL, hInstance, NULL);
					if (!hWndBackNebLblBlueVariance) return -1;
					hWndBackNebTxtBlueVariance = CreateWindowEx(NULL, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_TABSTOP | WS_CLIPSIBLINGS, 499, 123, 32, 16, hWnd, NULL, hInstance, NULL);
					if (!hWndBackNebTxtBlueVariance) return -1;
					//*/ 
			hWndStar = CreateWindowEx(NULL, "MultiSoftGButton", "Display Star Drawing controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 97, 65, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndStar) return -1;
				hWndStarHide = CreateWindowEx(NULL, "MultiSoftGButton", "Hide the Star Drawing Controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 2, 81, 33, hWndStarCon, NULL, hInstance, NULL);
				if (!hWndStarHide) return -1;
				hWndStarBasics = CreateWindowEx(NULL, "MultiSoftGButton", "Display the basic star first step controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 43, 81, 33, hWndStarCon, NULL, hInstance, NULL);
				if (!hWndStarBasics) return -1;
					hWndStarBasPosition = CreateWindowEx(NULL, "MultiSoftGButton", "Position the star", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 4, 4, 81, 33, hWndStarBasCon, NULL, hInstance, NULL);
					if (!hWndStarBasPosition) return -1;
					hWndStarBasStarSize = CreateWindowEx(NULL, "MultiSoftGButton", "Set the size of the star", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 4, 45, 81, 33, hWndStarBasCon, NULL, hInstance, NULL);
					if (!hWndStarBasStarSize) return -1;
					hWndStarBasCoronaSize = CreateWindowEx(NULL, "MultiSoftGButton", "Set the size of the corona", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 4, 86, 81, 33, hWndStarBasCon, NULL, hInstance, NULL);
					if (!hWndStarBasCoronaSize) return -1;
					hWndStarBasStarColour = CreateWindowEx(NULL, "MultiSoftGButton", "Change the star colour", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 93, 4, 81, 33, hWndStarBasCon, NULL, hInstance, NULL);
					if (!hWndStarBasStarColour) return -1;
					hWndStarBasExport = CreateWindowEx(NULL, "MultiSoftGButton", "Export the current star", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10 - 89, 4, 81, 33, hWndStarBasCon, NULL, hInstance, NULL);
					if (!hWndStarBasExport) return -1;
					hWndStarBasUnlock = CreateWindowEx(NULL, "MultiSoftGButton", "Unlock the basic star controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10 - 89, 86, 81, 33, hWndStarBasCon, NULL, hInstance, NULL);
					if (!hWndStarBasUnlock) return -1;
					hWndStarBasRandom = CreateWindowEx(NULL, "MultiSoftGButton", "Randomise basic star parameteres", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 4, 81, 33, hWndStarBasCon, NULL, hInstance, NULL);
					if (!hWndStarBasRandom) return -1;
					hWndStarBasAdvanced = CreateWindowEx(NULL, "MultiSoftGButton", "Advanced (basic) star parameters", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 45, 81, 33, hWndStarBasCon, NULL, hInstance, NULL);
					if (!hWndStarBasAdvanced) return -1;
					hWndStarBasDraw = CreateWindowEx(NULL, "MultiSoftGButton", "Draw basic star", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 86, 81, 33, hWndStarBasCon, NULL, hInstance, NULL);
					if (!hWndStarBasDraw) return -1;
				hWndStarSurface = CreateWindowEx(NULL, "MultiSoftGButton", "Display the star surface controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 84, 81, 33, hWndStarCon, NULL, hInstance, NULL);
				if (!hWndStarSurface) return -1;
					hWndStarSurScrAmt = CreateWindowEx(NULL, "MultiSoftGHScroll", "Change the amount of maximum variance on the surface", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 6, 12, 440 - 99 - 4, 16, hWndStarSurCon, NULL, hInstance, NULL);
					if (!hWndStarSurScrAmt) return -1;
					hWndStarSurPrvAmt = CreateWindowEx(NULL, "MultiSoftGWndCon", "Surface variance preview", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 6, 47, 77, 29, hWndStarSurCon, NULL, hInstance, NULL);
					if (!hWndStarSurPrvAmt) return -1;
					hWndStarSurRandom = CreateWindowEx(NULL, "MultiSoftGButton", "Randomise star surface parameters", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 4, 81, 33, hWndStarSurCon, NULL, hInstance, NULL);
					if (!hWndStarSurRandom) return -1;
					hWndStarSurUnlock = CreateWindowEx(NULL, "MultiSoftGButton", "Unlock the star surface controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10 - 89, 86, 81, 33, hWndStarSurCon, NULL, hInstance, NULL);
					if (!hWndStarSurUnlock) return -1;
					hWndStarSurDraw = CreateWindowEx(NULL, "MultiSoftGButton", "Draw the star surface", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 86, 81, 33, hWndStarSurCon, NULL, hInstance, NULL);
					if (!hWndStarSurDraw) return -1;
				hWndStarFlares = CreateWindowEx(NULL, "MultiSoftGButton", "Display the solar flare controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 125, 81, 33, hWndStarCon, NULL, hInstance, NULL);
				if (!hWndStarFlares) return -1;
					//Gradient button:
					hWndStarFlrGrad = CreateWindowEx(NULL, "MultiSoftGButton", "Show the gradient controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 4, 4, 81, 33, hWndStarFlrCon, NULL, hInstance, NULL);
					if (!hWndStarFlrGrad) return -1;
					//Animation button:
					//hWndStarFlrAnim = CreateWindowEx(NULL, "MultiSoftGButton", "Show the animation controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10 - 89, 86, 81, 33, hWndStarFlrCon, NULL, hInstance, NULL);
					//if (!hWndStarFlrAnim) return -1;
					//Random button:
					hWndStarFlrRand = CreateWindowEx(NULL, "MultiSoftGButton", "Randomise flare parameteres", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 4, 81, 33, hWndStarFlrCon, NULL, hInstance, NULL);
					if (!hWndStarFlrRand) return -1;
					//Advanced button:
					hWndStarFlrAdvanced = CreateWindowEx(NULL, "MultiSoftGButton", "Advanced flare parameters", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 45, 81, 33, hWndStarFlrCon, NULL, hInstance, NULL);
					if (!hWndStarFlrAdvanced) return -1;
					hWndStarFlrUnlock = CreateWindowEx(NULL, "MultiSoftGButton", "Unlock the solar flare controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10 - 89, 86, 81, 33, hWndStarFlrCon, NULL, hInstance, NULL);
					if (!hWndStarFlrUnlock) return -1;
					hWndStarFlrDraw = CreateWindowEx(NULL, "MultiSoftGButton", "Draw the solar flares", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 440 - 81 - 10, 86, 81, 33, hWndStarFlrCon, NULL, hInstance, NULL);
					if (!hWndStarFlrDraw) return -1;
				hWndStarClear = CreateWindowEx(NULL, "MultiSoftGButton", "Clear the star buffer and start a new star", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 166, 81, 33, hWndStarCon, NULL, hInstance, NULL);
				if (!hWndStarClear) return -1;
				hWndStarFinish = CreateWindowEx(NULL, "MultiSoftGButton", "Finalise the current star and transfer it to the image", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 207, 81, 33, hWndStarCon, NULL, hInstance, NULL);
				if (!hWndStarFinish) return -1;
			hWndMiscFilters = CreateWindowEx(NULL, "MultiSoftGButton", "Display the Miscellaneous Filters", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 97, 106, 81, 33, hWnd, NULL, hInstance, NULL);
			if (!hWndMiscFilters) return -1;
				hWndMisFilHide = CreateWindowEx(NULL, "MultiSoftGButton", "Hide the Miscellaneous Filters", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 2, 81, 33, hWndMisFilCon, NULL, hInstance, NULL);
				if (!hWndMisFilHide) return -1;
				hWndMisFilSmooth = CreateWindowEx(NULL, "MultiSoftGButton", "Display the smoothing controls", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 2, 43, 81, 33, hWndMisFilCon, NULL, hInstance, NULL);
				if (!hWndMisFilSmooth) return -1;


			SSSOldBitmapPtr = new BitmapOps(640, 480);
			SSSUndoBitmapPtr = new BitmapOps(640, 480);
			SSSBufferBitmapPtr = new BitmapOps(640, 480);
			memset(SSSOldBitmapPtr->PixelData, 0, SSSOldBitmapPtr->UBound());
			memset(SSSUndoBitmapPtr->PixelData, 0, SSSUndoBitmapPtr->UBound());
			memset(SSSBufferBitmapPtr->PixelData, 0, SSSBufferBitmapPtr->UBound());
			/*DrawSolidColour(*SSSOldBitmapPtr, 0, 0, 0);
			DrawSolidColour(*SSSUndoBitmapPtr, 0, 0, 0);
			DrawSolidColour(*SSSBufferBitmapPtr, 0, 0, 0);*/
			if (!SSSOldBitmapPtr) return -1;

			WindowBackground.GetBitmapFromResource(IDB_WINDOWBACKGROUND);

			//Set up the default nebula parameters:
			for (int i = 0; i < 5; i++) {
				NebulaParams.LayerParams[i].RedVariance = 256;
				NebulaParams.LayerParams[i].GreenVariance = 256;
				NebulaParams.LayerParams[i].BlueVariance = 256;
				NebulaParams.LayerParams[i].KeepPrevious = 2;
				NebulaParams.LayerParams[i].Persistance = 0.75;
				NebulaParams.LayerParams[i].FirstOctave = 1;
				NebulaParams.LayerParams[i].Interpolation = 2;
				NebulaParams.LayerParams[i].Smooth = 1;
				NebulaParams.LayerParams[i].NumGradientEntries = 512;
				NebulaParams.LayerParams[i].GradientArray = new COLORREF[NebulaParams.LayerParams[i].NumGradientEntries];
			}
			NebulaParams.LayerParams[0].KeepPrevious = 0;
			NebulaParams.NumLayers = 1;
			
			NebulaParams.Animate = FALSE;
			NebulaParams.BaseFileName = NULL;
			NebulaParams.FileFormat = 0;
			NebulaParams.FramesPerKey = 24;
			NebulaParams.KeyFrames = 10;
			NebulaParams.Loop = FALSE;
			NebulaParams.SmoothT = TRUE;

			//Set up the default star parameters:
			CurStar.CoronaRadius = 15;
			CurStar.StarColour = 0x0080ff;
			CurStar.StarRadius = 75;
			CurStar.XPos = 320;
			CurStar.YPos = 240;

			//Set up the default flare parameters:
			for (i = 0; i < 5; i++) {
				FlareParams.LayerParams[i].RedVariance = 256;
				FlareParams.LayerParams[i].GreenVariance = 256;
				FlareParams.LayerParams[i].BlueVariance = 256;
				FlareParams.LayerParams[i].KeepPrevious = 2;
				FlareParams.LayerParams[i].Persistance = 0.75;
				FlareParams.LayerParams[i].FirstOctave = 1;
				FlareParams.LayerParams[i].Interpolation = 2;
				FlareParams.LayerParams[i].Smooth = 1;
				FlareParams.LayerParams[i].NumGradientEntries = 512;
				FlareParams.LayerParams[i].GradientArray = new COLORREF[NebulaParams.LayerParams[i].NumGradientEntries];
			}
			FlareParams.LayerParams[0].KeepPrevious = 0;
			FlareParams.NumLayers = 1;
			
			FlareParams.Animate = FALSE;
			FlareParams.BaseFileName = NULL;
			FlareParams.FileFormat = 0;
			FlareParams.FramesPerKey = 24;
			FlareParams.KeyFrames = 10;
			FlareParams.Loop = FALSE;
			FlareParams.SmoothT = TRUE;

			//Preview Window:
			WNDCLASSEX WndClass = {0};
			WndClass.cbSize = sizeof(WNDCLASSEX);
			WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			WndClass.hInstance = hInstance;
			WndClass.lpfnWndProc = PrevWndProc;
			WndClass.lpszClassName = "MultiSoftSunProjectPrev";
			WndClass.style = CS_VREDRAW | CS_HREDRAW;

			if (!RegisterClassEx(&WndClass)) return -1;

			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);

			hWndPreview = CreateWindowEx(NULL, "MultiSoftSunProjectPrev", "", WS_CHILD | WS_DLGFRAME | WS_VISIBLE, ClientRect.right - 416, ClientRect.bottom - 316, 406, 306, hWnd, NULL, hInstance, NULL);
			if (!hWndPreview) {
				UnregisterClass("MultiSoftSunProjectPrev", hInstance);
				return -1;
			}

			HWND TempHWnd = NULL;
			//Title bar:
			SendMessage(hWndTitle, GTM_SETHEIGHT, NULL, 16);
			SendMessage(hWndTitle, GTM_SETBARIMAGE, NULL, IDB_TITLEBAR);
			SendMessage(hWndTitle, GTM_SETTEXTCOLOUR, NULL, 0x80);
			//Close Button:
			HWND hWndClose = (HWND)SendMessage(hWndTitle, GTM_GETCLOSEHWND, NULL, NULL);
			SendMessage(hWndClose, GBM_SETOFFIMAGE, NULL, IDB_CLOSEOFF);
			SendMessage(hWndClose, GBM_SETOVERIMAGE, NULL, IDB_CLOSEOVER);
			SendMessage(hWndClose, GBM_SETONIMAGE, NULL, IDB_CLOSEON);
			//Max Button:
			HWND hWndMax = (HWND)SendMessage(hWndTitle, GTM_GETMAXHWND, NULL, NULL);
			ShowWindow(hWndMax, SW_HIDE);
			//Min Button:
			HWND hWndMin = (HWND)SendMessage(hWndTitle, GTM_GETMINHWND, NULL, NULL);
			SendMessage(hWndMin, GBM_SETOFFIMAGE, NULL, IDB_MINOFF);
			SendMessage(hWndMin, GBM_SETOVERIMAGE, NULL, IDB_MINOVER);
			SendMessage(hWndMin, GBM_SETONIMAGE, NULL, IDB_MINON);
			//ToolTip:
			HWND TTHWnd = GetMultiSoftGToolTipHWnd();
			SendMessage(TTHWnd, GTTM_SETIMAGE, NULL, IDB_TOOLTIP);
			SendMessage(TTHWnd, GTTM_SETLEFTIMAGE, NULL, IDB_TOOLTIPLEFT);
			SendMessage(TTHWnd, GTTM_SETRIGHTIMAGE, NULL, IDB_TOOLTIPRIGHT);
			SendMessage(TTHWnd, GTTM_SETTEXTCOLOUR, NULL, 0x80);

			//New button:
			SendMessage(hWndNew, GBM_SETOFFIMAGE, NULL, IDB_NEWOFF);
			SendMessage(hWndNew, GBM_SETOVERIMAGE, NULL, IDB_NEWOVER);
			SendMessage(hWndNew, GBM_SETONIMAGE, NULL, IDB_NEWON);
			SendMessage(hWndNew, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndNew, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Random button:
			SendMessage(hWndRandom, GBM_SETOFFIMAGE, NULL, IDB_RANDOMOFF);
			SendMessage(hWndRandom, GBM_SETOVERIMAGE, NULL, IDB_RANDOMOVER);
			SendMessage(hWndRandom, GBM_SETONIMAGE, NULL, IDB_RANDOMON);
			SendMessage(hWndRandom, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndRandom, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Buffer button:
			SendMessage(hWndBufferBtn, GBM_SETOFFIMAGE, NULL, IDB_BUFFEROFF);
			SendMessage(hWndBufferBtn, GBM_SETOVERIMAGE, NULL, IDB_BUFFEROVER);
			SendMessage(hWndBufferBtn, GBM_SETONIMAGE, NULL, IDB_BUFFERON);
			SendMessage(hWndBufferBtn, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndBufferBtn, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Buffer Window Container:
			SendMessage(hWndBufferCon, GWCM_SETBACKCOLOUR, 0, 0);
				//Hide Buffer button:
				SendMessage(hWndBufferHide, GBM_SETOFFIMAGE, NULL, IDB_HIDEOFF);
				SendMessage(hWndBufferHide, GBM_SETOVERIMAGE, NULL, IDB_HIDEOVER);
				SendMessage(hWndBufferHide, GBM_SETONIMAGE, NULL, IDB_HIDEON);
				SendMessage(hWndBufferHide, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBufferHide, GBM_SETTOOLTIPSTATE, NULL, TRUE);	
				//Buffer Save button:
				SendMessage(hWndBufferSave, GBM_SETOFFIMAGE, NULL, IDB_BUFFERSAVEOFF);
				SendMessage(hWndBufferSave, GBM_SETOVERIMAGE, NULL, IDB_BUFFERSAVEOVER);
				SendMessage(hWndBufferSave, GBM_SETONIMAGE, NULL, IDB_BUFFERSAVEON);
				SendMessage(hWndBufferSave, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBufferSave, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//Buffer Load button:
				SendMessage(hWndBufferLoad, GBM_SETOFFIMAGE, NULL, IDB_BUFFERLOADOFF);
				SendMessage(hWndBufferLoad, GBM_SETOVERIMAGE, NULL, IDB_BUFFERLOADOVER);
				SendMessage(hWndBufferLoad, GBM_SETONIMAGE, NULL, IDB_BUFFERLOADON);
				SendMessage(hWndBufferLoad, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBufferLoad, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//Buffer Swap button:
				SendMessage(hWndBufferSwap, GBM_SETOFFIMAGE, NULL, IDB_BUFFERSWAPOFF);
				SendMessage(hWndBufferSwap, GBM_SETOVERIMAGE, NULL, IDB_BUFFERSWAPOVER);
				SendMessage(hWndBufferSwap, GBM_SETONIMAGE, NULL, IDB_BUFFERSWAPON);
				SendMessage(hWndBufferSwap, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBufferSwap, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//View Buffer button:
				SendMessage(hWndViewBuffer, GBM_SETOFFIMAGE, NULL, IDB_VIEWBUFFEROFF);
				SendMessage(hWndViewBuffer, GBM_SETOVERIMAGE, NULL, IDB_VIEWBUFFEROVER);
				SendMessage(hWndViewBuffer, GBM_SETONIMAGE, NULL, IDB_VIEWBUFFERON);
				SendMessage(hWndViewBuffer, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndViewBuffer, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//Overlay Image button:
				SendMessage(hWndBuffOverlayImage, GBM_SETOFFIMAGE, NULL, IDB_OVERLAYIMAGEOFF);
				SendMessage(hWndBuffOverlayImage, GBM_SETOVERIMAGE, NULL, IDB_OVERLAYIMAGEOVER);
				SendMessage(hWndBuffOverlayImage, GBM_SETONIMAGE, NULL, IDB_OVERLAYIMAGEON);
				SendMessage(hWndBuffOverlayImage, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBuffOverlayImage, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//Underlay Image button:
				SendMessage(hWndBuffUnderlayImage, GBM_SETOFFIMAGE, NULL, IDB_UNDERLAYIMAGEOFF);
				SendMessage(hWndBuffUnderlayImage, GBM_SETOVERIMAGE, NULL, IDB_UNDERLAYIMAGEOVER);
				SendMessage(hWndBuffUnderlayImage, GBM_SETONIMAGE, NULL, IDB_UNDERLAYIMAGEON);
				SendMessage(hWndBuffUnderlayImage, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBuffUnderlayImage, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//Merge Images button:
				SendMessage(hWndBuffMergeImages, GBM_SETOFFIMAGE, NULL, IDB_MERGEIMAGESOFF);
				SendMessage(hWndBuffMergeImages, GBM_SETOVERIMAGE, NULL, IDB_MERGEIMAGESOVER);
				SendMessage(hWndBuffMergeImages, GBM_SETONIMAGE, NULL, IDB_MERGEIMAGESON);
				SendMessage(hWndBuffMergeImages, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBuffMergeImages, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Undo/Redo button:
			SendMessage(hWndUndoRedo, GBM_SETOFFIMAGE, NULL, IDB_UNDOREDOOFF);
			SendMessage(hWndUndoRedo, GBM_SETOVERIMAGE, NULL, IDB_UNDOREDOOVER);
			SendMessage(hWndUndoRedo, GBM_SETONIMAGE, NULL, IDB_UNDOREDOON);
			SendMessage(hWndUndoRedo, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndUndoRedo, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Abort button:
			SendMessage(hWndAbort, GBM_SETOFFIMAGE, NULL, IDB_ABORTOFF);
			SendMessage(hWndAbort, GBM_SETOVERIMAGE, NULL, IDB_ABORTOVER);
			SendMessage(hWndAbort, GBM_SETONIMAGE, NULL, IDB_ABORTON);
			SendMessage(hWndAbort, GBM_SETDISABLEDIMAGE, NULL, IDB_ABORTDISABLED);
			SendMessage(hWndAbort, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndAbort, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			EnableWindow(hWndAbort, FALSE);
			//View Image button:
			SendMessage(hWndViewImage, GBM_SETOFFIMAGE, NULL, IDB_VIEWIMAGEOFF);
			SendMessage(hWndViewImage, GBM_SETOVERIMAGE, NULL, IDB_VIEWIMAGEOVER);
			SendMessage(hWndViewImage, GBM_SETONIMAGE, NULL, IDB_VIEWIMAGEON);
			SendMessage(hWndViewImage, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndViewImage, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Open button:
			SendMessage(hWndOpen, GBM_SETOFFIMAGE, NULL, IDB_OPENOFF);
			SendMessage(hWndOpen, GBM_SETOVERIMAGE, NULL, IDB_OPENOVER);
			SendMessage(hWndOpen, GBM_SETONIMAGE, NULL, IDB_OPENON);
			SendMessage(hWndOpen, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndOpen, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Save button:
			SendMessage(hWndSave, GBM_SETOFFIMAGE, NULL, IDB_SAVEOFF);
			SendMessage(hWndSave, GBM_SETOVERIMAGE, NULL, IDB_SAVEOVER);
			SendMessage(hWndSave, GBM_SETONIMAGE, NULL, IDB_SAVEON);
			SendMessage(hWndSave, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndSave, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Help button:
			SendMessage(hWndHelp, GBM_SETOFFIMAGE, NULL, IDB_HELPOFF);
			SendMessage(hWndHelp, GBM_SETOVERIMAGE, NULL, IDB_HELPOVER);
			SendMessage(hWndHelp, GBM_SETONIMAGE, NULL, IDB_HELPON);
			SendMessage(hWndHelp, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndHelp, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Exit button:
			SendMessage(hWndExit, GBM_SETOFFIMAGE, NULL, IDB_EXITOFF);
			SendMessage(hWndExit, GBM_SETOVERIMAGE, NULL, IDB_EXITOVER);
			SendMessage(hWndExit, GBM_SETONIMAGE, NULL, IDB_EXITON);
			SendMessage(hWndExit, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndExit, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Background Button:
			SendMessage(hWndBackground, GBM_SETOFFIMAGE, NULL, IDB_BACKGROUNDOFF);
			SendMessage(hWndBackground, GBM_SETOVERIMAGE, NULL, IDB_BACKGROUNDOVER);
			SendMessage(hWndBackground, GBM_SETONIMAGE, NULL, IDB_BACKGROUNDON);
			SendMessage(hWndBackground, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndBackground, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Background Window Container:
			SendMessage(hWndBackCon, GWCM_SETBACKCOLOUR, 0, 0);
				//BackHide Button:
				SendMessage(hWndBackHide, GBM_SETOFFIMAGE, NULL, IDB_HIDEOFF);
				SendMessage(hWndBackHide, GBM_SETOVERIMAGE, NULL, IDB_HIDEOVER);
				SendMessage(hWndBackHide, GBM_SETONIMAGE, NULL, IDB_HIDEON);
				SendMessage(hWndBackHide, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBackHide, GBM_SETTOOLTIPSTATE, NULL, TRUE);	
				//BackSolid Button:
				SendMessage(hWndBackSolid, GBM_SETOFFIMAGE, NULL, IDB_SOLIDCOLOUROFF);
				SendMessage(hWndBackSolid, GBM_SETOVERIMAGE, NULL, IDB_SOLIDCOLOUROVER);
				SendMessage(hWndBackSolid, GBM_SETONIMAGE, NULL, IDB_SOLIDCOLOURON);
				SendMessage(hWndBackSolid, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBackSolid, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//BackNebula Button:
				SendMessage(hWndBackNebula, GBM_SETOFFIMAGE, NULL, IDB_NEBULAOFF);
				SendMessage(hWndBackNebula, GBM_SETOVERIMAGE, NULL, IDB_NEBULAOVER);
				SendMessage(hWndBackNebula, GBM_SETONIMAGE, NULL, IDB_NEBULAON);
				SendMessage(hWndBackNebula, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBackNebula, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//Nebula Window Container:
				SendMessage(hWndBackNebCon, GWCM_SETBACKCOLOUR, 0, 0);
					//Nebula gradient Button:
					SendMessage(hWndBackNebGrad, GBM_SETOFFIMAGE, NULL, IDB_GRADIENTOFF);
					SendMessage(hWndBackNebGrad, GBM_SETOVERIMAGE, NULL, IDB_GRADIENTOVER);
					SendMessage(hWndBackNebGrad, GBM_SETONIMAGE, NULL, IDB_GRADIENTON);
					SendMessage(hWndBackNebGrad, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndBackNebGrad, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Nebula animation Button:
					SendMessage(hWndBackNebAnim, GBM_SETOFFIMAGE, NULL, IDB_ANIMATIONOFF);
					SendMessage(hWndBackNebAnim, GBM_SETOVERIMAGE, NULL, IDB_ANIMATIONOVER);
					SendMessage(hWndBackNebAnim, GBM_SETONIMAGE, NULL, IDB_ANIMATIONON);
					SendMessage(hWndBackNebAnim, GBM_SETDISABLEDIMAGE, NULL, IDB_ANIMATIONDISABLED);
					SendMessage(hWndBackNebAnim, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndBackNebAnim, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Random nebula Button:
					SendMessage(hWndBackNebRand, GBM_SETOFFIMAGE, NULL, IDB_RANDOMOFF);
					SendMessage(hWndBackNebRand, GBM_SETOVERIMAGE, NULL, IDB_RANDOMOVER);
					SendMessage(hWndBackNebRand, GBM_SETONIMAGE, NULL, IDB_RANDOMON);
					SendMessage(hWndBackNebRand, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndBackNebRand, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Advanced Nebula parameters Button:
					SendMessage(hWndBackNebAdvanced, GBM_SETOFFIMAGE, NULL, IDB_ADVANCEDOFF);
					SendMessage(hWndBackNebAdvanced, GBM_SETOVERIMAGE, NULL, IDB_ADVANCEDOVER);
					SendMessage(hWndBackNebAdvanced, GBM_SETONIMAGE, NULL, IDB_ADVANCEDON);
					SendMessage(hWndBackNebAdvanced, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndBackNebAdvanced, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Draw Nebula Button:
					SendMessage(hWndBackNebCmdDraw, GBM_SETOFFIMAGE, NULL, IDB_DRAWOFF);
					SendMessage(hWndBackNebCmdDraw, GBM_SETOVERIMAGE, NULL, IDB_DRAWOVER);
					SendMessage(hWndBackNebCmdDraw, GBM_SETONIMAGE, NULL, IDB_DRAWON);
					SendMessage(hWndBackNebCmdDraw, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndBackNebCmdDraw, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//BackStarfield Button:
				SendMessage(hWndBackStarfield, GBM_SETOFFIMAGE, NULL, IDB_STARFIELDOFF);
				SendMessage(hWndBackStarfield, GBM_SETOVERIMAGE, NULL, IDB_STARFIELDOVER);
				SendMessage(hWndBackStarfield, GBM_SETONIMAGE, NULL, IDB_STARFIELDON);
				SendMessage(hWndBackStarfield, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndBackStarfield, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Star Button:
			SendMessage(hWndStar, GBM_SETOFFIMAGE, NULL, IDB_STAROFF);
			SendMessage(hWndStar, GBM_SETOVERIMAGE, NULL, IDB_STAROVER);
			SendMessage(hWndStar, GBM_SETONIMAGE, NULL, IDB_STARON);
			SendMessage(hWndStar, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndStar, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Star Window Container:
			SendMessage(hWndStarCon, GWCM_SETBACKCOLOUR, 0, 0);
				//Hide Button:
				SendMessage(hWndStarHide, GBM_SETOFFIMAGE, NULL, IDB_HIDEOFF);
				SendMessage(hWndStarHide, GBM_SETOVERIMAGE, NULL, IDB_HIDEOVER);
				SendMessage(hWndStarHide, GBM_SETONIMAGE, NULL, IDB_HIDEON);
				SendMessage(hWndStarHide, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndStarHide, GBM_SETTOOLTIPSTATE, NULL, TRUE);	
				//Basics Button:
				SendMessage(hWndStarBasics, GBM_SETOFFIMAGE, NULL, IDB_BASICSOFF);
				SendMessage(hWndStarBasics, GBM_SETOVERIMAGE, NULL, IDB_BASICSOVER);
				SendMessage(hWndStarBasics, GBM_SETONIMAGE, NULL, IDB_BASICSON);
				SendMessage(hWndStarBasics, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndStarBasics, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Basics Window Container:
					SendMessage(hWndStarBasCon, GWCM_SETBACKCOLOUR, 0, 0);
					//Position Button:
					SendMessage(hWndStarBasPosition, GBM_SETOFFIMAGE, NULL, IDB_PLACESTAROFF);
					SendMessage(hWndStarBasPosition, GBM_SETOVERIMAGE, NULL, IDB_PLACESTAROVER);
					SendMessage(hWndStarBasPosition, GBM_SETONIMAGE, NULL, IDB_PLACESTARON);
					SendMessage(hWndStarBasPosition, GBM_SETDISABLEDIMAGE, NULL, IDB_PLACESTARDISABLED);
					SendMessage(hWndStarBasPosition, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarBasPosition, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Star Size Button:
					SendMessage(hWndStarBasStarSize, GBM_SETOFFIMAGE, NULL, IDB_STARSIZEOFF);
					SendMessage(hWndStarBasStarSize, GBM_SETOVERIMAGE, NULL, IDB_STARSIZEOVER);
					SendMessage(hWndStarBasStarSize, GBM_SETONIMAGE, NULL, IDB_STARSIZEON);
					SendMessage(hWndStarBasStarSize, GBM_SETDISABLEDIMAGE, NULL, IDB_STARSIZEDISABLED);
					SendMessage(hWndStarBasStarSize, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarBasStarSize, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Corona Size Button:
					SendMessage(hWndStarBasCoronaSize, GBM_SETOFFIMAGE, NULL, IDB_CORONASIZEOFF);
					SendMessage(hWndStarBasCoronaSize, GBM_SETOVERIMAGE, NULL, IDB_CORONASIZEOVER);
					SendMessage(hWndStarBasCoronaSize, GBM_SETONIMAGE, NULL, IDB_CORONASIZEON);
					SendMessage(hWndStarBasCoronaSize, GBM_SETDISABLEDIMAGE, NULL, IDB_CORONASIZEDISABLED);
					SendMessage(hWndStarBasCoronaSize, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarBasCoronaSize, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Star Colour Button:
					SendMessage(hWndStarBasStarColour, GBM_SETOFFIMAGE, NULL, IDB_STARCOLOUROFF);
					SendMessage(hWndStarBasStarColour, GBM_SETOVERIMAGE, NULL, IDB_STARCOLOUROVER);
					SendMessage(hWndStarBasStarColour, GBM_SETONIMAGE, NULL, IDB_STARCOLOURON);
					SendMessage(hWndStarBasStarColour, GBM_SETDISABLEDIMAGE, NULL, IDB_STARCOLOURDISABLED);
					SendMessage(hWndStarBasStarColour, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarBasStarColour, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Export Star Button:
					SendMessage(hWndStarBasExport, GBM_SETOFFIMAGE, NULL, IDB_EXPORTSTAROFF);
					SendMessage(hWndStarBasExport, GBM_SETOVERIMAGE, NULL, IDB_EXPORTSTAROVER);
					SendMessage(hWndStarBasExport, GBM_SETONIMAGE, NULL, IDB_EXPORTSTARON);
					SendMessage(hWndStarBasExport, GBM_SETDISABLEDIMAGE, NULL, IDB_EXPORTSTARDISABLED);
					SendMessage(hWndStarBasExport, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarBasExport, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					EnableWindow(hWndStarBasExport, FALSE);
					//Unlock Button:
					SendMessage(hWndStarBasUnlock, GBM_SETOFFIMAGE, NULL, IDB_UNLOCKOFF);
					SendMessage(hWndStarBasUnlock, GBM_SETOVERIMAGE, NULL, IDB_UNLOCKOVER);
					SendMessage(hWndStarBasUnlock, GBM_SETONIMAGE, NULL, IDB_UNLOCKON);
					SendMessage(hWndStarBasUnlock, GBM_SETDISABLEDIMAGE, NULL, IDB_UNLOCKDISABLED);
					SendMessage(hWndStarBasUnlock, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarBasUnlock, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					EnableWindow(hWndStarBasUnlock, FALSE);
					//Random Basic Star Button:
					SendMessage(hWndStarBasRandom, GBM_SETOFFIMAGE, NULL, IDB_RANDOMOFF);
					SendMessage(hWndStarBasRandom, GBM_SETOVERIMAGE, NULL, IDB_RANDOMOVER);
					SendMessage(hWndStarBasRandom, GBM_SETONIMAGE, NULL, IDB_RANDOMON);
					SendMessage(hWndStarBasRandom, GBM_SETDISABLEDIMAGE, NULL, IDB_RANDOMDISABLED);
					SendMessage(hWndStarBasRandom, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarBasRandom, GBM_SETTOOLTIPSTATE, NULL, TRUE);		
					//Advanced Star parameters Button:
					SendMessage(hWndStarBasAdvanced, GBM_SETOFFIMAGE, NULL, IDB_ADVANCEDOFF);
					SendMessage(hWndStarBasAdvanced, GBM_SETOVERIMAGE, NULL, IDB_ADVANCEDOVER);
					SendMessage(hWndStarBasAdvanced, GBM_SETONIMAGE, NULL, IDB_ADVANCEDON);
					SendMessage(hWndStarBasAdvanced, GBM_SETDISABLEDIMAGE, NULL, IDB_ADVANCEDDISABLED);
					SendMessage(hWndStarBasAdvanced, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarBasAdvanced, GBM_SETTOOLTIPSTATE, NULL, TRUE);	
					EnableWindow(hWndStarBasAdvanced, FALSE);
					//Draw Basic Star Button:
					SendMessage(hWndStarBasDraw, GBM_SETOFFIMAGE, NULL, IDB_DRAWOFF);
					SendMessage(hWndStarBasDraw, GBM_SETOVERIMAGE, NULL, IDB_DRAWOVER);
					SendMessage(hWndStarBasDraw, GBM_SETONIMAGE, NULL, IDB_DRAWON);
					SendMessage(hWndStarBasDraw, GBM_SETDISABLEDIMAGE, NULL, IDB_DRAWDISABLED);
					SendMessage(hWndStarBasDraw, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarBasDraw, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//Surface Button:
				SendMessage(hWndStarSurface, GBM_SETOFFIMAGE, NULL, IDB_SURFACEOFF);
				SendMessage(hWndStarSurface, GBM_SETOVERIMAGE, NULL, IDB_SURFACEOVER);
				SendMessage(hWndStarSurface, GBM_SETONIMAGE, NULL, IDB_SURFACEON);
				SendMessage(hWndStarSurface, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndStarSurface, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Surface Window Container:
					SendMessage(hWndStarSurCon, GWCM_SETBACKCOLOUR, 0, 0);
					//Variance scrollbar:
					SendMessage(hWndStarSurScrAmt, GSM_SETMIN, 0, 1);
					SendMessage(hWndStarSurScrAmt, GSM_SETMAX, 0, 128);
					SendMessage(hWndStarSurScrAmt, GSM_SETVALUE, 0, 64);
					SendMessage(hWndStarSurScrAmt, GSM_SETTRACKIMAGE, NULL, IDB_HSCROLLTRACK);
					SendMessage(hWndStarSurScrAmt, GSM_SETBARTOPIMAGE, NULL, IDB_HSCROLLBARLEFT);
					SendMessage(hWndStarSurScrAmt, GSM_SETBARBOTTOMIMAGE, NULL, IDB_HSCROLLBARRIGHT);
					EnableWindow(hWndStarSurScrAmt, FALSE);
					TempHWnd = (HWND)SendMessage(hWndStarSurScrAmt, GSM_GETUPLEFTHWND, 0, 0);
					SendMessage(TempHWnd, GBM_SETOFFIMAGE, NULL, IDB_HSCROLLLARROWOFF);
					SendMessage(TempHWnd, GBM_SETOVERIMAGE, NULL, IDB_HSCROLLLARROWOVER);
					SendMessage(TempHWnd, GBM_SETONIMAGE, NULL, IDB_HSCROLLLARROWON);
					SendMessage(TempHWnd, GBM_SETDISABLEDIMAGE, NULL, IDB_HSCROLLLARROWDISABLED);
					TempHWnd = (HWND)SendMessage(hWndStarSurScrAmt, GSM_GETDOWNRIGHTHWND, 0, 0);
					SendMessage(TempHWnd, GBM_SETOFFIMAGE, NULL, IDB_HSCROLLRARROWOFF);
					SendMessage(TempHWnd, GBM_SETOVERIMAGE, NULL, IDB_HSCROLLRARROWOVER);
					SendMessage(TempHWnd, GBM_SETONIMAGE, NULL, IDB_HSCROLLRARROWON);
					SendMessage(TempHWnd, GBM_SETDISABLEDIMAGE, NULL, IDB_HSCROLLRARROWDISABLED);
					//Variance preview area:
					SendMessage(hWndStarSurPrvAmt, GWCM_SETPARENTPAINT, 0, TRUE);
					//Random Star Surface Button:
					SendMessage(hWndStarSurRandom, GBM_SETOFFIMAGE, NULL, IDB_RANDOMOFF);
					SendMessage(hWndStarSurRandom, GBM_SETOVERIMAGE, NULL, IDB_RANDOMOVER);
					SendMessage(hWndStarSurRandom, GBM_SETONIMAGE, NULL, IDB_RANDOMON);
					SendMessage(hWndStarSurRandom, GBM_SETDISABLEDIMAGE, NULL, IDB_RANDOMDISABLED);
					SendMessage(hWndStarSurRandom, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarSurRandom, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					EnableWindow(hWndStarSurRandom, FALSE);
					//Unlock Button:
					SendMessage(hWndStarSurUnlock, GBM_SETOFFIMAGE, NULL, IDB_UNLOCKOFF);
					SendMessage(hWndStarSurUnlock, GBM_SETOVERIMAGE, NULL, IDB_UNLOCKOVER);
					SendMessage(hWndStarSurUnlock, GBM_SETONIMAGE, NULL, IDB_UNLOCKON);
					SendMessage(hWndStarSurUnlock, GBM_SETDISABLEDIMAGE, NULL, IDB_UNLOCKDISABLED);
					SendMessage(hWndStarSurUnlock, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarSurUnlock, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Draw Star Surface Button:
					SendMessage(hWndStarSurDraw, GBM_SETOFFIMAGE, NULL, IDB_DRAWOFF);
					SendMessage(hWndStarSurDraw, GBM_SETOVERIMAGE, NULL, IDB_DRAWOVER);
					SendMessage(hWndStarSurDraw, GBM_SETONIMAGE, NULL, IDB_DRAWON);
					SendMessage(hWndStarSurDraw, GBM_SETDISABLEDIMAGE, NULL, IDB_DRAWDISABLED);
					SendMessage(hWndStarSurDraw, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarSurDraw, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					EnableWindow(hWndStarSurDraw, FALSE);
				//Flares Button:
				SendMessage(hWndStarFlares, GBM_SETOFFIMAGE, NULL, IDB_FLARESOFF);
				SendMessage(hWndStarFlares, GBM_SETOVERIMAGE, NULL, IDB_FLARESOVER);
				SendMessage(hWndStarFlares, GBM_SETONIMAGE, NULL, IDB_FLARESON);
				SendMessage(hWndStarFlares, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndStarFlares, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Flares Window Container:
					SendMessage(hWndStarFlrCon, GWCM_SETBACKCOLOUR, 0, 0);
					//Flare gradient Button:
					SendMessage(hWndStarFlrGrad, GBM_SETOFFIMAGE, NULL, IDB_GRADIENTOFF);
					SendMessage(hWndStarFlrGrad, GBM_SETOVERIMAGE, NULL, IDB_GRADIENTOVER);
					SendMessage(hWndStarFlrGrad, GBM_SETONIMAGE, NULL, IDB_GRADIENTON);
					SendMessage(hWndStarFlrGrad, GBM_SETDISABLEDIMAGE, NULL, IDB_GRADIENTDISABLED);
					SendMessage(hWndStarFlrGrad, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarFlrGrad, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					EnableWindow(hWndStarFlrGrad, FALSE);
					//Flare animation Button:
					/*SendMessage(hWndStarFlrAnim, GBM_SETOFFIMAGE, NULL, IDB_ANIMATIONOFF);
					SendMessage(hWndStarFlrAnim, GBM_SETOVERIMAGE, NULL, IDB_ANIMATIONOVER);
					SendMessage(hWndStarFlrAnim, GBM_SETONIMAGE, NULL, IDB_ANIMATIONON);
					SendMessage(hWndStarFlrAnim, GBM_SETDISABLEDIMAGE, NULL, IDB_ANIMATIONDISABLED);
					SendMessage(hWndStarFlrAnim, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarFlrAnim, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					EnableWindow(hWndStarFlrAnim, FALSE);*/
					//Random Flare Button:
					SendMessage(hWndStarFlrRand, GBM_SETOFFIMAGE, NULL, IDB_RANDOMOFF);
					SendMessage(hWndStarFlrRand, GBM_SETOVERIMAGE, NULL, IDB_RANDOMOVER);
					SendMessage(hWndStarFlrRand, GBM_SETONIMAGE, NULL, IDB_RANDOMON);
					SendMessage(hWndStarFlrRand, GBM_SETDISABLEDIMAGE, NULL, IDB_RANDOMDISABLED);
					SendMessage(hWndStarFlrRand, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarFlrRand, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					EnableWindow(hWndStarFlrRand, FALSE);
					//Advanced Nebula parameters Button:
					SendMessage(hWndStarFlrAdvanced, GBM_SETOFFIMAGE, NULL, IDB_ADVANCEDOFF);
					SendMessage(hWndStarFlrAdvanced, GBM_SETOVERIMAGE, NULL, IDB_ADVANCEDOVER);
					SendMessage(hWndStarFlrAdvanced, GBM_SETONIMAGE, NULL, IDB_ADVANCEDON);
					SendMessage(hWndStarFlrAdvanced, GBM_SETDISABLEDIMAGE, NULL, IDB_ADVANCEDDISABLED);
					SendMessage(hWndStarFlrAdvanced, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarFlrAdvanced, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					EnableWindow(hWndStarFlrAdvanced, FALSE);
					//Unlock Button:
					SendMessage(hWndStarFlrUnlock, GBM_SETOFFIMAGE, NULL, IDB_UNLOCKOFF);
					SendMessage(hWndStarFlrUnlock, GBM_SETOVERIMAGE, NULL, IDB_UNLOCKOVER);
					SendMessage(hWndStarFlrUnlock, GBM_SETONIMAGE, NULL, IDB_UNLOCKON);
					SendMessage(hWndStarFlrUnlock, GBM_SETDISABLEDIMAGE, NULL, IDB_UNLOCKDISABLED);
					SendMessage(hWndStarFlrUnlock, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarFlrUnlock, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					//Draw Solar Flares Button:
					SendMessage(hWndStarFlrDraw, GBM_SETOFFIMAGE, NULL, IDB_DRAWOFF);
					SendMessage(hWndStarFlrDraw, GBM_SETOVERIMAGE, NULL, IDB_DRAWOVER);
					SendMessage(hWndStarFlrDraw, GBM_SETONIMAGE, NULL, IDB_DRAWON);
					SendMessage(hWndStarFlrDraw, GBM_SETDISABLEDIMAGE, NULL, IDB_DRAWDISABLED);
					SendMessage(hWndStarFlrDraw, GBM_AUTOSIZE, 81, 33);
					SendMessage(hWndStarFlrDraw, GBM_SETTOOLTIPSTATE, NULL, TRUE);
					EnableWindow(hWndStarFlrDraw, FALSE);
				//Clear Star Button:
				SendMessage(hWndStarClear, GBM_SETOFFIMAGE, NULL, IDB_CLEARSTAROFF);
				SendMessage(hWndStarClear, GBM_SETOVERIMAGE, NULL, IDB_CLEARSTAROVER);
				SendMessage(hWndStarClear, GBM_SETONIMAGE, NULL, IDB_CLEARSTARON);
				SendMessage(hWndStarClear, GBM_SETDISABLEDIMAGE, NULL, IDB_CLEARSTARDISABLED);
				SendMessage(hWndStarClear, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndStarClear, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				EnableWindow(hWndStarClear, FALSE);
				//Finish Star Button:
				SendMessage(hWndStarFinish, GBM_SETOFFIMAGE, NULL, IDB_FINISHSTAROFF);
				SendMessage(hWndStarFinish, GBM_SETOVERIMAGE, NULL, IDB_FINISHSTAROVER);
				SendMessage(hWndStarFinish, GBM_SETONIMAGE, NULL, IDB_FINISHSTARON);
				SendMessage(hWndStarFinish, GBM_SETDISABLEDIMAGE, NULL, IDB_FINISHSTARDISABLED);
				SendMessage(hWndStarFinish, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndStarFinish, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				EnableWindow(hWndStarFinish, FALSE);
			//Misc Effects Button:
			SendMessage(hWndMiscFilters, GBM_SETOFFIMAGE, NULL, IDB_MISCFILTERSOFF);
			SendMessage(hWndMiscFilters, GBM_SETOVERIMAGE, NULL, IDB_MISCFILTERSOVER);
			SendMessage(hWndMiscFilters, GBM_SETONIMAGE, NULL, IDB_MISCFILTERSON);
			SendMessage(hWndMiscFilters, GBM_AUTOSIZE, 81, 33);
			SendMessage(hWndMiscFilters, GBM_SETTOOLTIPSTATE, NULL, TRUE);
			//Misc Filters Window Container:
			SendMessage(hWndMisFilCon, GWCM_SETBACKCOLOUR, 0, 0);
				//Hide Misc Filters Button:
				SendMessage(hWndMisFilHide, GBM_SETOFFIMAGE, NULL, IDB_HIDEOFF);
				SendMessage(hWndMisFilHide, GBM_SETOVERIMAGE, NULL, IDB_HIDEOVER);
				SendMessage(hWndMisFilHide, GBM_SETONIMAGE, NULL, IDB_HIDEON);
				SendMessage(hWndMisFilHide, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndMisFilHide, GBM_SETTOOLTIPSTATE, NULL, TRUE);
				//Smooth Button:
				SendMessage(hWndMisFilSmooth, GBM_SETOFFIMAGE, NULL, IDB_SMOOTHOFF);
				SendMessage(hWndMisFilSmooth, GBM_SETOVERIMAGE, NULL, IDB_SMOOTHOVER);
				SendMessage(hWndMisFilSmooth, GBM_SETONIMAGE, NULL, IDB_SMOOTHON);
				SendMessage(hWndMisFilSmooth, GBM_AUTOSIZE, 81, 33);
				SendMessage(hWndMisFilSmooth, GBM_SETTOOLTIPSTATE, NULL, TRUE);
		}
		break;
	case WM_COMMAND:
		if ((HWND)lParam == hWndNew) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_NEW), hWnd, (DLGPROC)NewDlgProc) == TRUE) {
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWndStarClear), GBN_CLICKED), (LPARAM)hWndStarClear);
				}
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndBackSolid) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				static COLORREF LastColour = 0;
				CHOOSECOLOR cc = {0};
				cc.lStructSize = sizeof(CHOOSECOLOR);
				cc.hwndOwner = hWnd;
				cc.rgbResult = LastColour;
				cc.lpCustColors = CustomColourArray;
				cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;
				if (ChooseColor(&cc)) {
					LastColour = cc.rgbResult;
					bDrawing = false;
					SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
					if (MessageBox(hWnd, "Draw under the image?", "Underlay?", MB_YESNO | MB_ICONQUESTION) == IDYES) {
						DrawSolidColour(*SSSOldBitmapPtr, GetRValue(LastColour), GetGValue(LastColour), GetBValue(LastColour), 1);
					} else if (MessageBox(hWnd, "Draw over the image?", "Overlay?", MB_YESNO | MB_ICONQUESTION) == IDYES) {
						DrawSolidColour(*SSSOldBitmapPtr, GetRValue(LastColour), GetGValue(LastColour), GetBValue(LastColour), 2);
					} else {
						DrawSolidColour(*SSSOldBitmapPtr, GetRValue(LastColour), GetGValue(LastColour), GetBValue(LastColour), 0);
					}
					if (IsWindow(hWndView))
						InvalidateRect(hWndView, NULL, TRUE);
					InvalidateRect(hWndPreview, NULL, TRUE);
				}
			}
		}
		if ((HWND)lParam == hWndBackNebula) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (NebulaVisible) {
					SendMessage(hWndBackNebCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
					NebulaVisible = FALSE;
				} else {
					if (StarBasVisible) {
						SendMessage(hWndStarBasCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						StarBasVisible = FALSE;
					}
					if (StarSurVisible) {
						SendMessage(hWndStarSurCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						StarSurVisible = FALSE;
					}
					if (StarFlrVisible) {
						SendMessage(hWndStarFlrCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						StarFlrVisible = FALSE;
					}
					SendMessage(hWndBackNebCon, GWCM_MOVETOCOSINE, MAKEWPARAM(184, 24), MAKELPARAM(MenuSpeed, 20));
					NebulaVisible = TRUE;
				}
			}
		}
		if ((HWND)lParam == hWndStarBasics) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (StarBasVisible) {
					SendMessage(hWndStarBasCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
					StarBasVisible = FALSE;
				} else {
					if (NebulaVisible) {
						SendMessage(hWndBackNebCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						NebulaVisible = FALSE;
					}
					if (StarSurVisible) {
						SendMessage(hWndStarSurCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						StarSurVisible = FALSE;
					}
					if (StarFlrVisible) {
						SendMessage(hWndStarFlrCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						StarFlrVisible = FALSE;
					}
					SendMessage(hWndStarBasCon, GWCM_MOVETOCOSINE, MAKEWPARAM(184, 24), MAKELPARAM(MenuSpeed, 20));
					StarBasVisible = TRUE;
				}
			}
		}
		if ((HWND)lParam == hWndStarSurface) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (StarSurVisible) {
					SendMessage(hWndStarSurCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
					StarSurVisible = FALSE;
				} else {
					if (NebulaVisible) {
						SendMessage(hWndBackNebCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						NebulaVisible = FALSE;
					}
					if (StarBasVisible) {
						SendMessage(hWndStarBasCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						StarBasVisible = FALSE;
					}
					if (StarFlrVisible) {
						SendMessage(hWndStarFlrCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						StarFlrVisible = FALSE;
					}
					SendMessage(hWndStarSurCon, GWCM_MOVETOCOSINE, MAKEWPARAM(184, 24), MAKELPARAM(MenuSpeed, 20));
					StarSurVisible = TRUE;
				}
			}
		}
		if ((HWND)lParam == hWndStarFlares) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (StarFlrVisible) {
					SendMessage(hWndStarFlrCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
					StarFlrVisible = FALSE;
				} else {
					if (NebulaVisible) {
						SendMessage(hWndBackNebCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						NebulaVisible = FALSE;
					}
					if (StarBasVisible) {
						SendMessage(hWndStarBasCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						StarBasVisible = FALSE;
					}
					if (StarSurVisible) {
						SendMessage(hWndStarSurCon, GWCM_MOVETOCOSINE, MAKEWPARAM(640, 24), MAKELPARAM(MenuSpeed, 20));
						StarSurVisible = FALSE;
					}
					SendMessage(hWndStarFlrCon, GWCM_MOVETOCOSINE, MAKEWPARAM(184, 24), MAKELPARAM(MenuSpeed, 20));
					StarFlrVisible = TRUE;
				}
			}
		}
		if ((HWND)lParam == hWndBackNebGrad) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				PerlinParams = &NebulaParams;
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_GRADIENT), hWnd, (DLGPROC)GradientDlgProc);
			}
		}
		if ((HWND)lParam == hWndBackNebAnim) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				PerlinParams = &NebulaParams;
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_NEBULAANIM), hWnd, (DLGPROC)AnimDlgProc);
			}
		}
		if ((HWND)lParam == hWndBackNebRand) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				//Set up the nebula parameters:
				srand(GetTickCount());rand(); //Seed the random number generator
				for (int i = 0; i < 5; i++) {
					NebulaParams.LayerParams[i].RedVariance = rand() % 512;
					NebulaParams.LayerParams[i].GreenVariance = rand() % 512;
					NebulaParams.LayerParams[i].BlueVariance = rand() % 512;
					NebulaParams.LayerParams[i].KeepPrevious = 2;
					NebulaParams.LayerParams[i].MiddleColour = RGB(rand() % 128, rand() % 128, rand() % 128);
					NebulaParams.LayerParams[i].Persistance = (double)rand() / RAND_MAX * 0.5 + 0.5;
					NebulaParams.LayerParams[i].FirstOctave = 1;
					NebulaParams.LayerParams[i].Bug = 0;
					NebulaParams.LayerParams[i].Interpolation = 2;
					NebulaParams.LayerParams[i].Octaves = 0;
					NebulaParams.LayerParams[i].Seed = 0;
					NebulaParams.LayerParams[i].Smooth = 1;
					//NebulaParams.LayerParams[i].HTile = 0;
					//NebulaParams.LayerParams[i].VTile = 0;
					NebulaParams.LayerParams[i].UseGradient = rand() % 2;
					//CreatePerlinGradient((NebulaParams.LayerParams[i].Bug ? 2 : 0) | (NebulaParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (NebulaParams.LayerParams[i].HTile ? 4 : 0) | (!NebulaParams.LayerParams[i].Interpolation ? 16 : 0) | (NebulaParams.LayerParams[i].Interpolation == 2 ? 32 : 0), NebulaParams.LayerParams[i].MiddleColour, NebulaParams.LayerParams[i].NumGradientEntries, NebulaParams.LayerParams[i].GradientArray, NebulaParams.LayerParams[i].RedVariance, NebulaParams.LayerParams[i].GreenVariance, NebulaParams.LayerParams[i].BlueVariance, NebulaParams.LayerParams[i].Persistance, NebulaParams.LayerParams[i].Octaves, NebulaParams.LayerParams[i].FirstOctave);
					CreatePerlinGradient(1 | (NebulaParams.LayerParams[i].Bug ? 2 : 0) | (NebulaParams.LayerParams[i].HTile ? 4 : 0) | (!NebulaParams.LayerParams[i].Interpolation ? 16 : 0) | (NebulaParams.LayerParams[i].Interpolation == 2 ? 32 : 0), NebulaParams.LayerParams[i].MiddleColour, NebulaParams.LayerParams[i].NumGradientEntries, NebulaParams.LayerParams[i].GradientArray, NebulaParams.LayerParams[i].RedVariance, NebulaParams.LayerParams[i].GreenVariance, NebulaParams.LayerParams[i].BlueVariance, NebulaParams.LayerParams[i].Persistance, NebulaParams.LayerParams[i].Octaves, NebulaParams.LayerParams[i].FirstOctave);
				}
				NebulaParams.LayerParams[0].KeepPrevious = 0;
				NebulaParams.NumLayers = 1;
				NebulaParams.Animate = FALSE;
				NebulaParams.SmoothT = TRUE;
			}
		}
		if ((HWND)lParam == hWndBackNebAdvanced) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				PerlinParams = &NebulaParams;
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_NEBULA), hWnd, (DLGPROC)NebulaDlgProc);
			}
		}
		if ((HWND)lParam == hWndBackNebCmdDraw) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break; //Otherwise we would defeat the purpose of this variable altogether
				if (NebulaParams.Animate) {
					OPENFILENAME ofn = {0};
					char txFileName[MAX_PATH] = {0};
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hWnd;
					ofn.lpstrFilter = "Microsoft AVI (*.avi)\0*.avi\0Bitmap Stream (*???.bmp)\0*.bmp\0All Files (*.*)\0*.*\0\0";
					ofn.nFilterIndex = 1;
					ofn.lpstrFile = txFileName;
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
					ofn.lpstrDefExt = "avi";
					if (GetSaveFileName(&ofn)) {
						NebulaParams.BaseFileName = txFileName;
						bDrawing = true; //So that the preview window draws the 'Drawing...' text
						if (IsWindow(hWndView))
							InvalidateRect(hWndView, NULL, TRUE);
						InvalidateRect(hWndPreview, NULL, TRUE);
						bDrawing = false;
						EnableWindow(hWndAbort, TRUE);
						SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
						DrawNebula(*SSSOldBitmapPtr, NebulaParams);
						EnableWindow(hWndAbort, FALSE);
					}
				} else {
					bDrawing = true; //So that the preview window draws the 'Drawing...' text
					if (IsWindow(hWndView))
						InvalidateRect(hWndView, NULL, TRUE);
					InvalidateRect(hWndPreview, NULL, TRUE);
					bDrawing = false;
					EnableWindow(hWndAbort, TRUE);
					SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
					DrawNebula(*SSSOldBitmapPtr, NebulaParams);
					EnableWindow(hWndAbort, FALSE);
				}
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndBackStarfield) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				bDrawing = false;
				EnableWindow(hWndAbort, TRUE);
				SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				DrawStarField(*SSSOldBitmapPtr, 0);
				EnableWindow(hWndAbort, FALSE);
				/*SendMessage(hWndPreview, WM_USER, 0, 0);
				SendMessage(hWndView, WM_USER, 0, 0);*/
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndRandom) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break; //Otherwise we would defeat the purpose of this variable altogether
				bDrawing = true; //So that the preview window draws the 'Drawing...' text
				/*SendMessage(hWndPreview, WM_USER, 0, 0);
				SendMessage(hWndView, WM_USER, 0, 0);*/
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
				bDrawing = false; //So that the drawing funciton doesn't abort straight away
				EnableWindow(hWndAbort, TRUE);
				SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				DrawRandomStarCompact(hWnd, *SSSOldBitmapPtr);
				EnableWindow(hWndAbort, FALSE);
				/*SendMessage(hWndPreview, WM_USER, 0, 0);
				SendMessage(hWndView, WM_USER, 0, 0);*/
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndBackground || (HWND)lParam == hWndBackHide) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (BackgroundVisible) {
					SendMessage(hWndBackCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, -168), MAKELPARAM(MenuSpeed, 20));
					BackgroundVisible = FALSE;
				} else {
					if (MiscFXVisible) {
						SendMessage(hWndMisFilCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, -84), MAKELPARAM(MenuSpeed, 20));
						MiscFXVisible = FALSE;
					}
					if (StarVisible) {
						SendMessage(hWndStarCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, -250), MAKELPARAM(MenuSpeed, 20));
						StarVisible = FALSE;
					}
					SendMessage(hWndBackCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, 19), MAKELPARAM(MenuSpeed, 20));
					BackgroundVisible = TRUE;
				}
			}
		}
		if ((HWND)lParam == hWndUndoRedo) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				BitmapOps *TempBitmapOps = SSSOldBitmapPtr;
				LONG32 Width = SSSOldBitmapPtr->Width();
				LONG32 Height = SSSOldBitmapPtr->Height();
				SSSOldBitmapPtr = SSSUndoBitmapPtr;
				SSSUndoBitmapPtr = TempBitmapOps;
				//Re-scale the star position and size:
				CurStar.XPos = (double)CurStar.XPos / (double)Width * (double)SSSOldBitmapPtr->Width() + 0.5;
				CurStar.YPos = (double)CurStar.YPos / (double)Height * (double)SSSOldBitmapPtr->Height() + 0.5;
				CurStar.StarRadius = (double)CurStar.StarRadius / (double)((Width + Height) / 2) * (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) + 0.5;
				CurStar.CoronaRadius = (double)CurStar.CoronaRadius / (double)((Width + Height) / 2) * (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) + 0.5;
				/*SendMessage(hWndPreview, WM_USER, 0, 0);
				SendMessage(hWndView, WM_USER, 0, 0);*/
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndAbort) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				bDrawing = false;
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
				EnableWindow(hWndAbort, FALSE);
			}
		}
		if ((HWND)lParam == hWndBufferBtn || (HWND)lParam == hWndBufferHide) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (BufferVisible) {
					SendMessage(hWndBufferCon, GWCM_MOVETOCOSINE, MAKEWPARAM(3/*92*/, 480), MAKELPARAM(MenuSpeed, 20));
					BufferVisible = FALSE;
				} else {
					SendMessage(hWndBufferCon, GWCM_MOVETOCOSINE, MAKEWPARAM(3/*92*/, 480 - 166 - 8), MAKELPARAM(MenuSpeed, 20));
					BufferVisible = TRUE;
				}
			}
		}
		if ((HWND)lParam == hWndBufferSave) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				SSSBufferBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				if (IsWindow(hWndBuffer))
					InvalidateRect(hWndBuffer, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndBufferLoad) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				bDrawing = false;
				LONG32 Width = SSSOldBitmapPtr->Width();
				LONG32 Height = SSSOldBitmapPtr->Height();
				SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				SSSOldBitmapPtr->CopyBitmapFromBitmapOps(*SSSBufferBitmapPtr);
				//Re-scale the star position and size:
				CurStar.XPos = (double)CurStar.XPos / (double)Width * (double)SSSOldBitmapPtr->Width() + 0.5;
				CurStar.YPos = (double)CurStar.YPos / (double)Height * (double)SSSOldBitmapPtr->Height() + 0.5;
				CurStar.StarRadius = (double)CurStar.StarRadius / (double)((Width + Height) / 2) * (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) + 0.5;
				CurStar.CoronaRadius = (double)CurStar.CoronaRadius / (double)((Width + Height) / 2) * (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) + 0.5;
				/*SendMessage(hWndPreview, WM_USER, 0, 0);
				SendMessage(hWndView, WM_USER, 0, 0);*/
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == (HWND)hWndBufferSwap) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				BitmapOps *TempBitmapOps = SSSOldBitmapPtr;
				LONG32 Width = SSSOldBitmapPtr->Width();
				LONG32 Height = SSSOldBitmapPtr->Height();
				SSSOldBitmapPtr = SSSBufferBitmapPtr;
				SSSBufferBitmapPtr = TempBitmapOps;
				//Re-scale the star position and size:
				CurStar.XPos = (double)CurStar.XPos / (double)Width * (double)SSSOldBitmapPtr->Width() + 0.5;
				CurStar.YPos = (double)CurStar.YPos / (double)Height * (double)SSSOldBitmapPtr->Height() + 0.5;
				CurStar.StarRadius = (double)CurStar.StarRadius / (double)((Width + Height) / 2) * (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) + 0.5;
				CurStar.CoronaRadius = (double)CurStar.CoronaRadius / (double)((Width + Height) / 2) * (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) + 0.5;
				/*SendMessage(hWndPreview, WM_USER, 0, 0);
				SendMessage(hWndView, WM_USER, 0, 0);*/
				if (IsWindow(hWndBuffer))
					InvalidateRect(hWndBuffer, NULL, TRUE);
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndBuffOverlayImage) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				UnderlayBitmapOps(*SSSOldBitmapPtr, *SSSBufferBitmapPtr, *SSSOldBitmapPtr);
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndBuffUnderlayImage) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				UnderlayBitmapOps(*SSSOldBitmapPtr, *SSSOldBitmapPtr, *SSSBufferBitmapPtr);
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndBuffMergeImages) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_MERGE), hWnd, (DLGPROC)MergeDlgProc);
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndViewBuffer) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (IsWindow(hWndBuffer)) {
					SetFocus(hWndBuffer);
				} else {
					hWndBuffer = CreateWindowEx(NULL, "MultiSoftSunProjectBuffer", "Buffer", WS_POPUP | WS_THICKFRAME /*WS_DLGFRAME */| WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, GetSystemMetrics(SM_CXSCREEN) / 2 - 323, GetSystemMetrics(SM_CYSCREEN) / 2 - 243, 646, 486, NULL, NULL, hInstance, NULL);
					if (hWndBuffer) {
						//SendMessage(hWndBuffer, WM_USER, 0, 0);
						ShowWindow(hWndBuffer, SW_SHOW);
						UpdateWindow(hWndBuffer);
					}
				}
			}
		}
		if ((HWND)lParam == hWndStar || (HWND)lParam == hWndStarHide) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (StarVisible) {
					SendMessage(hWndStarCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, -250), MAKELPARAM(MenuSpeed, 20));
					StarVisible = FALSE;
				} else {
					if (BackgroundVisible) {
						SendMessage(hWndBackCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, -168), MAKELPARAM(MenuSpeed, 20));
						BackgroundVisible = FALSE;
					}
					if (MiscFXVisible) {
						SendMessage(hWndMisFilCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, -84), MAKELPARAM(MenuSpeed, 20));
						MiscFXVisible = FALSE;
					}
					SendMessage(hWndStarCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, 19), MAKELPARAM(MenuSpeed, 20));
					StarVisible = TRUE;
				}
			}
		}
		if ((HWND)lParam == hWndStarBasPosition) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (CurStar.SettingState == Positioning) {
					CurStar.SettingState = None;
					if (CurStar.ExtPreviewable) {
						CurStar.ExtBufferPreview = TRUE;
					}
				} else {
					CurStar.SettingState = Positioning;
					CurStar.ExtBufferPreview = FALSE;
				}
				if (CurStar.ExtPreviewable) {
					EnableWindow(hWndStarFinish, TRUE);
				}
				InvalidateRect(hWndPreview, NULL, TRUE);
				if (IsWindow(hWndView) && CurStar.ExtPreviewable)
					InvalidateRect(hWndView, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndStarBasStarSize) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (CurStar.SettingState == StarSizing) {
					CurStar.SettingState = None;
					if (CurStar.ExtPreviewable) {
						EnableWindow(hWndStarFinish, TRUE);
						CurStar.ExtBufferPreview = TRUE;
					}
				} else {
					CurStar.SettingState = StarSizing;
					EnableWindow(hWndStarFinish, FALSE);
					CurStar.ExtBufferPreview = FALSE;
				}
				InvalidateRect(hWndPreview, NULL, TRUE);
				if (IsWindow(hWndView) && CurStar.ExtPreviewable)
					InvalidateRect(hWndView, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndStarBasCoronaSize) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (CurStar.SettingState == CoronaSizing) {
					CurStar.SettingState = None;
					if (CurStar.ExtPreviewable) {
						EnableWindow(hWndStarFinish, TRUE);
						CurStar.ExtBufferPreview = TRUE;
					}
				} else {
					CurStar.SettingState = CoronaSizing;
					EnableWindow(hWndStarFinish, FALSE);
					CurStar.ExtBufferPreview = FALSE;
				}
				InvalidateRect(hWndPreview, NULL, TRUE);
				if (IsWindow(hWndView) && CurStar.ExtPreviewable)
					InvalidateRect(hWndView, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndStarBasStarColour) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				CHOOSECOLOR cc = {0};
				cc.lStructSize = sizeof(CHOOSECOLOR);
				cc.hwndOwner = hWnd;
				cc.rgbResult = CurStar.StarColour;
				cc.lpCustColors = CustomColourArray;
				cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;
				if (ChooseColor(&cc)) {
					CurStar.StarColour = cc.rgbResult;
				}
			}
		}
		if ((HWND)lParam == hWndStarBasExport) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				OPENFILENAME ofn = {0};
				char txFileName[MAX_PATH] = {0};
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.BMP\0All Files (*.*)\0*.*\0\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = txFileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
				ofn.lpstrDefExt = "bmp";
				if (GetSaveFileName(&ofn)) {
					CurStar.ExternalBuffer->DumpImage(txFileName);
				}
			}
		}
		if ((HWND)lParam == hWndStarBasUnlock) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				EnableWindow(hWndStarBasPosition, TRUE);
				EnableWindow(hWndStarBasStarSize, TRUE);
				EnableWindow(hWndStarBasCoronaSize, TRUE);
				EnableWindow(hWndStarBasStarColour, TRUE);
				EnableWindow(hWndStarBasRandom, TRUE);
				//EnableWindow(hWndStarBasAdvanced, TRUE);
				EnableWindow(hWndStarBasDraw, TRUE);
				EnableWindow(hWndStarBasUnlock, FALSE);
			}
		}
		if ((HWND)lParam == hWndStarBasRandom) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				BOOL Update = FALSE;
				if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview) Update = TRUE;
				EnableWindow(hWndStarFinish, FALSE);
				CurStar.ExtBufferPreview = FALSE;
				CurStar.ExtPreviewable = FALSE;
				srand(GetTickCount()); rand();
				LONG32 TempVal = (SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 4;
				if (TempVal) CurStar.StarRadius = rand() % TempVal;
				else CurStar.StarRadius = 1;
				TempVal = (SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 8;
				if (TempVal) CurStar.CoronaRadius = rand() % TempVal + 2;
				else CurStar.CoronaRadius = 2;
				CurStar.StarColour = RGB(rand() % 255, rand() % 255, rand() % 255);
				CurStar.XPos = rand() % SSSOldBitmapPtr->Width();
				CurStar.YPos = rand() % SSSOldBitmapPtr->Height();
				if (CurStar.SettingState || Update) InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndStarBasDraw) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break; //Otherwise we would defeat the purpose of this variable altogether
				bDrawing = true; //So that the preview window draws the 'Drawing...' text
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
				bDrawing = false;
				EnableWindow(hWndAbort, TRUE);
				if (CurStar.UseExtBuffer) {
					EnableWindow(hWndStarClear, TRUE);
					EnableWindow(hWndStarFinish, TRUE);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWndStarSurUnlock), GBN_CLICKED), (LPARAM)hWndStarSurUnlock);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWndStarFlrUnlock), GBN_CLICKED), (LPARAM)hWndStarFlrUnlock);
					CurStar.SettingState = None;
					CurStar.ExtPreviewable = TRUE;
					CurStar.ExtBufferPreview = TRUE;
					CurStar.ExternalBuffer->Resize((CurStar.StarRadius + CurStar.CoronaRadius) * 2, (CurStar.StarRadius + CurStar.CoronaRadius) * 2);
					memset(CurStar.ExternalBuffer->PixelData, 0, CurStar.ExternalBuffer->UBound());
					DrawBasicStar(*CurStar.ExternalBuffer, CurStar.ExternalBuffer->Width() / 2, CurStar.ExternalBuffer->Height() / 2, CurStar.StarRadius, CurStar.CoronaRadius, CurStar.StarColour);
				} else {
					SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
					EnableWindow(hWndStarBasPosition, FALSE);
					EnableWindow(hWndStarBasStarSize, FALSE);
					EnableWindow(hWndStarBasCoronaSize, FALSE);
					EnableWindow(hWndStarBasStarColour, FALSE);
					EnableWindow(hWndStarBasRandom, FALSE);
					//EnableWindow(hWndStarBasAdvanced, FALSE);
					EnableWindow(hWndStarBasDraw, FALSE);
					EnableWindow(hWndStarBasUnlock, TRUE);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWndStarSurUnlock), GBN_CLICKED), (LPARAM)hWndStarSurUnlock);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWndStarFlrUnlock), GBN_CLICKED), (LPARAM)hWndStarFlrUnlock);
					CurStar.SettingState = None;
					DrawBasicStar(*SSSOldBitmapPtr, CurStar.XPos, SSSOldBitmapPtr->Height() - CurStar.YPos, CurStar.StarRadius, CurStar.CoronaRadius, CurStar.StarColour);
				}
				EnableWindow(hWndAbort, FALSE);
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndStarSurScrAmt) {
			if (HIWORD(wParam) == GSN_SCROLL || HIWORD(wParam) == GSN_BARDRAG) {
				InvalidateRect(hWndStarSurPrvAmt, NULL, FALSE);
			}
		}
		if ((HWND)lParam == hWndStarSurPrvAmt) {
			if (HIWORD(wParam) == GWCN_PAINT) {
				LONG32 Variance = SendMessage(hWndStarSurScrAmt, GSM_GETDRAGVALUE, 0, 0);
				RECT ClientRect;
				GetClientRect(hWndStarSurPrvAmt, &ClientRect);
				PAINTSTRUCT ps;
				HDC hDc = BeginPaint(hWndStarSurPrvAmt, &ps);
				HPEN hOldPen = (HPEN)SelectObject(hDc, GetStockObject(NULL_PEN));
				HBRUSH hBrush, hOldBrush;
				LONG32 TempRed, TempGreen, TempBlue;
				//Dark colour:
				TempRed = GetRValue(CurStar.StarColour) - Variance;
				TempGreen = GetGValue(CurStar.StarColour) - Variance;
				TempBlue = GetBValue(CurStar.StarColour) - Variance;
				if (TempRed < 0) TempRed = 0;
				if (TempGreen < 0) TempGreen = 0;
				if (TempBlue < 0) TempBlue = 0;
				hBrush = CreateSolidBrush(RGB(TempRed, TempGreen, TempBlue));
				hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
				Rectangle(hDc, 0, 0, ClientRect.right / 3 + 1, ClientRect.bottom + 1);
				SelectObject(hDc, hOldBrush);
				DeleteObject(hBrush);
				//Actual colour:
				hBrush = CreateSolidBrush(CurStar.StarColour);
				hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
				Rectangle(hDc, ClientRect.right / 3, 0, ClientRect.right * 2 / 3 + 1, ClientRect.bottom + 1);
				SelectObject(hDc, hOldBrush);
				DeleteObject(hBrush);
				//Light colour:
				TempRed = GetRValue(CurStar.StarColour) + Variance;
				TempGreen = GetGValue(CurStar.StarColour) + Variance;
				TempBlue = GetBValue(CurStar.StarColour) + Variance;
				if (TempRed > 255) TempRed = 255;
				if (TempGreen > 255) TempGreen = 255;
				if (TempBlue > 255) TempBlue = 255;
				hBrush = CreateSolidBrush(RGB(TempRed, TempGreen, TempBlue));
				hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
				Rectangle(hDc, ClientRect.right * 2 / 3, 0, ClientRect.right + 1, ClientRect.bottom + 1);
				SelectObject(hDc, hOldBrush);
				DeleteObject(hBrush);
				SelectObject(hDc, hOldPen);
				EndPaint(hWndStarSurPrvAmt, &ps);
			}
		}
		if ((HWND)lParam == hWndStarSurRandom) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				SendMessage(hWndStarSurScrAmt, GSM_SETVALUE, 0, rand() % 64);
				InvalidateRect(hWndStarSurPrvAmt, NULL, FALSE);
			}
		}
		if ((HWND)lParam == hWndStarSurUnlock) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				EnableWindow(hWndStarSurScrAmt, TRUE);
				EnableWindow(hWndStarSurRandom, TRUE);
				EnableWindow(hWndStarSurDraw, TRUE);
				EnableWindow(hWndStarSurUnlock, FALSE);
			}
		}
		if ((HWND)lParam == hWndStarSurDraw) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break; //Otherwise we would defeat the purpose of this variable altogether
				bDrawing = true; //So that the preview window draws the 'Drawing...' text
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
				bDrawing = false;
				EnableWindow(hWndAbort, TRUE);
				if (CurStar.UseExtBuffer) {
					if (!CurStar.ExtPreviewable) {
						EnableWindow(hWndStarClear, TRUE);
						EnableWindow(hWndStarFinish, TRUE);
						CurStar.ExtPreviewable = TRUE;
						CurStar.ExternalBuffer->Resize((CurStar.StarRadius + CurStar.CoronaRadius) * 2, (CurStar.StarRadius + CurStar.CoronaRadius) * 2);
						memset(CurStar.ExternalBuffer->PixelData, 0, CurStar.ExternalBuffer->UBound());
						DrawBasicStar(*CurStar.ExternalBuffer, CurStar.ExternalBuffer->Width() / 2, CurStar.ExternalBuffer->Height() / 2, CurStar.StarRadius, CurStar.CoronaRadius, CurStar.StarColour);
					}
					CurStar.ExtBufferPreview = TRUE;
					DrawStarSurfaceNoise(*CurStar.ExternalBuffer, CurStar.ExternalBuffer->Width() / 2, CurStar.ExternalBuffer->Height() / 2, CurStar.StarRadius, SendMessage(hWndStarSurScrAmt, GSM_GETVALUE, 0, 0), TRUE, 0, TRUE, CurStar.StarColour);
				} else {
					SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
					DrawStarSurfaceNoise(*SSSOldBitmapPtr, CurStar.XPos, SSSOldBitmapPtr->Height() - CurStar.YPos, CurStar.StarRadius, SendMessage(hWndStarSurScrAmt, GSM_GETVALUE, 0, 0), TRUE, 0, TRUE, CurStar.StarColour);
				}
				EnableWindow(hWndAbort, FALSE);
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndStarFlrGrad) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				PerlinParams = &FlareParams;
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_GRADIENT), hWnd, (DLGPROC)GradientDlgProc);
			}
		}
		/*if ((HWND)lParam == hWndStarFlrAnim) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				PerlinParams = &FlareParams;
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_NEBULAANIM), hWnd, (DLGPROC)AnimDlgProc);
			}
		}*/
		if ((HWND)lParam == hWndStarFlrRand) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				//Set up the nebula parameters:
				srand(GetTickCount());rand(); //Seed the random number generator
				for (int i = 0; i < 5; i++) {
					FlareParams.LayerParams[i].RedVariance = (double)rand() / RAND_MAX * (GetRValue(CurStar.StarColour) + 256) / 2;
					FlareParams.LayerParams[i].GreenVariance = (double)rand() / RAND_MAX * (GetGValue(CurStar.StarColour) + 256) / 2;
					FlareParams.LayerParams[i].BlueVariance = (double)rand() / RAND_MAX * (GetBValue(CurStar.StarColour) + 256) / 2;
					FlareParams.LayerParams[i].KeepPrevious = 2;
					//FlareParams.LayerParams[i].MiddleColour = RGB((double)rand() / RAND_MAX * GetRValue(CurStar.StarColour), (double)rand() / RAND_MAX * GetGValue(CurStar.StarColour), (double)rand() / RAND_MAX * GetBValue(CurStar.StarColour));
					FlareParams.LayerParams[i].MiddleColour = RGB(GetRValue(CurStar.StarColour), GetGValue(CurStar.StarColour), GetBValue(CurStar.StarColour));
					FlareParams.LayerParams[i].Persistance = (double)rand() / RAND_MAX * 0.1 + 0.85;
					FlareParams.LayerParams[i].FirstOctave = 1;
					FlareParams.LayerParams[i].Bug = 0;
					FlareParams.LayerParams[i].Interpolation = 2;
					FlareParams.LayerParams[i].Octaves = -2;
					FlareParams.LayerParams[i].Seed = 0;
					FlareParams.LayerParams[i].Smooth = 1;
					//FlareParams.LayerParams[i].HTile = 0;
					//FlareParams.LayerParams[i].VTile = 0;
					FlareParams.LayerParams[i].UseGradient = rand() % 2;
					//CreatePerlinGradient((FlareParams.LayerParams[i].Bug ? 2 : 0) | (FlareParams.LayerParams[i].KeepPrevious != 2 ? 1 : 0) | (FlareParams.LayerParams[i].HTile ? 4 : 0) | (!FlareParams.LayerParams[i].Interpolation ? 16 : 0) | (FlareParams.LayerParams[i].Interpolation == 2 ? 32 : 0), FlareParams.LayerParams[i].MiddleColour, FlareParams.LayerParams[i].NumGradientEntries, FlareParams.LayerParams[i].GradientArray, FlareParams.LayerParams[i].RedVariance, FlareParams.LayerParams[i].GreenVariance, FlareParams.LayerParams[i].BlueVariance, FlareParams.LayerParams[i].Persistance, FlareParams.LayerParams[i].Octaves, FlareParams.LayerParams[i].FirstOctave);
					CreatePerlinGradient(1 | (FlareParams.LayerParams[i].Bug ? 2 : 0) | (FlareParams.LayerParams[i].HTile ? 4 : 0) | (!FlareParams.LayerParams[i].Interpolation ? 16 : 0) | (FlareParams.LayerParams[i].Interpolation == 2 ? 32 : 0), FlareParams.LayerParams[i].MiddleColour, FlareParams.LayerParams[i].NumGradientEntries, FlareParams.LayerParams[i].GradientArray, FlareParams.LayerParams[i].RedVariance, FlareParams.LayerParams[i].GreenVariance, FlareParams.LayerParams[i].BlueVariance, FlareParams.LayerParams[i].Persistance, FlareParams.LayerParams[i].Octaves, FlareParams.LayerParams[i].FirstOctave);
				}
				FlareParams.LayerParams[0].KeepPrevious = 0;
				FlareParams.NumLayers = 1;
				FlareParams.Animate = FALSE;
				FlareParams.SmoothT = TRUE;
			}
		}
		if ((HWND)lParam == hWndStarFlrAdvanced) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				PerlinParams = &FlareParams;
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_NEBULA), hWnd, (DLGPROC)NebulaDlgProc);
			}
		}
		if ((HWND)lParam == hWndStarFlrUnlock) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				EnableWindow(hWndStarFlrGrad, TRUE);
				//EnableWindow(hWndStarFlrAnim, TRUE);
				EnableWindow(hWndStarFlrRand, TRUE);
				EnableWindow(hWndStarFlrAdvanced, TRUE);
				EnableWindow(hWndStarFlrDraw, TRUE);
				EnableWindow(hWndStarFlrUnlock, FALSE);
			}
		}
		if ((HWND)lParam == hWndStarFlrDraw) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break; //Otherwise we would defeat the purpose of this variable altogether
				bDrawing = true; //So that the preview window draws the 'Drawing...' text
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
				bDrawing = false;
				EnableWindow(hWndAbort, TRUE);
				if (CurStar.UseExtBuffer) {
					if (!CurStar.ExtPreviewable) {
						EnableWindow(hWndStarClear, TRUE);
						EnableWindow(hWndStarFinish, TRUE);
						CurStar.ExtPreviewable = TRUE;
						CurStar.ExtBufferPreview = TRUE;
						CurStar.ExternalBuffer->Resize((CurStar.StarRadius + CurStar.CoronaRadius) * 2, (CurStar.StarRadius + CurStar.CoronaRadius) * 2);
						memset(CurStar.ExternalBuffer->PixelData, 0, CurStar.ExternalBuffer->UBound());
						DrawBasicStar(*CurStar.ExternalBuffer, CurStar.ExternalBuffer->Width() / 2, CurStar.ExternalBuffer->Height() / 2, CurStar.StarRadius, CurStar.CoronaRadius, CurStar.StarColour);
					}
					CurStar.ExtBufferPreview = TRUE;
					DrawFlaresPerlinNoise(*CurStar.ExternalBuffer, FlareParams, CurStar.ExternalBuffer->Width() / 2, CurStar.ExternalBuffer->Height() / 2, CurStar.StarRadius, CurStar.CoronaRadius, CurStar.StarColour, 0, TRUE, FALSE);
				} else {
					SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
					DrawFlaresPerlinNoise(*SSSOldBitmapPtr, FlareParams, CurStar.XPos, SSSOldBitmapPtr->Height() - CurStar.YPos, CurStar.StarRadius, CurStar.CoronaRadius, CurStar.StarColour, 0, FALSE, TRUE);
				}
				EnableWindow(hWndAbort, FALSE);
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndStarClear) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				EnableWindow(hWndStarClear, FALSE);
				EnableWindow(hWndStarFinish, FALSE);
				SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWndStarBasUnlock), GBN_CLICKED), (LPARAM)hWndStarBasUnlock);
				EnableWindow(hWndStarSurScrAmt, FALSE);
				EnableWindow(hWndStarSurRandom, FALSE);
				EnableWindow(hWndStarSurDraw, FALSE);
				EnableWindow(hWndStarSurUnlock, TRUE);
				EnableWindow(hWndStarFlrGrad, FALSE);
				//EnableWindow(hWndStarFlrAnim, FALSE);
				EnableWindow(hWndStarFlrRand, FALSE);
				EnableWindow(hWndStarFlrAdvanced, FALSE);
				EnableWindow(hWndStarFlrDraw, FALSE);
				EnableWindow(hWndStarFlrUnlock, TRUE);
				CurStar.ExtPreviewable = FALSE;
				CurStar.ExtBufferPreview = FALSE;
				CurStar.ExternalBuffer->Resize(1, 1);
				//memset(CurStar.ExternalBuffer->PixelData, 0, CurStar.ExternalBuffer->UBound());
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndStarFinish) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				//Copy the star to the image, and stop previewing:
				SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				CurStar.ExtBufferPreview = FALSE;
				LONG32 MinX, MinY, MaxX, MaxY, MaxC;
				LONG32 Width = SSSOldBitmapPtr->Width();
				LONG32 Height = SSSOldBitmapPtr->Height();
				MaxC = CurStar.StarRadius + CurStar.CoronaRadius;
				LONG32 OffX = CurStar.XPos - MaxC;
				LONG32 OffY = Height - CurStar.YPos - MaxC;
				MaxC += MaxC; //Double it
				if (OffX < 0) MinX = -OffX;
				else MinX = 0;
				if (OffY < 0) MinY = -OffY;
				else MinY = 0;
				if (OffX + MaxC > Width)
					MaxX = MaxC - (OffX + MaxC - Width);
				else MaxX = MaxC;
				if (OffY + MaxC > Height)
					MaxY = MaxC - (OffY + MaxC - Height);
				else MaxY = MaxC;
				DWORD i, j, X, Y;
				double Alpha;
				for (Y = MinY; Y < MaxY; Y++) {
					for (X = MinX; X < MaxX; X++) {
						i = CurStar.ExternalBuffer->GetPixelIndex(X, Y);
						j = SSSOldBitmapPtr->GetPixelIndex(X + OffX, Y + OffY);
						Alpha = (double)CurStar.ExternalBuffer->PixelData[i + 3] / 255.0;
						SSSOldBitmapPtr->PixelData[j + 2] = SSSOldBitmapPtr->PixelData[j + 2] * Alpha + CurStar.ExternalBuffer->PixelData[i + 2];
						SSSOldBitmapPtr->PixelData[j + 1] = SSSOldBitmapPtr->PixelData[j + 1] * Alpha + CurStar.ExternalBuffer->PixelData[i + 1];
						SSSOldBitmapPtr->PixelData[j] = SSSOldBitmapPtr->PixelData[j] * Alpha + CurStar.ExternalBuffer->PixelData[i];
						/*SSSOldBitmapPtr->PixelData[j + 2] = CurStar.ExternalBuffer->PixelData[i + 2];
						SSSOldBitmapPtr->PixelData[j + 1] = CurStar.ExternalBuffer->PixelData[i + 1];
						SSSOldBitmapPtr->PixelData[j] = CurStar.ExternalBuffer->PixelData[i];*/
					}
				}
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
			if (HIWORD(wParam) == GBN_ENABLECHANGED) {
				if (IsWindowEnabled((HWND)lParam)) {
					EnableWindow(hWndStarBasExport, TRUE);
				} else {
					EnableWindow(hWndStarBasExport, FALSE);
				}
			}
		}
		if ((HWND)lParam == hWndMiscFilters || (HWND)lParam == hWndMisFilHide) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (MiscFXVisible) {
					SendMessage(hWndMisFilCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, -84), MAKELPARAM(MenuSpeed, 20));
					MiscFXVisible = FALSE;
				} else {
					if (BackgroundVisible) {
						SendMessage(hWndBackCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, -168), MAKELPARAM(MenuSpeed, 20));
						BackgroundVisible = FALSE;
					}
					if (StarVisible) {
						SendMessage(hWndStarCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, -250), MAKELPARAM(MenuSpeed, 20));
						StarVisible = FALSE;
					}
					SendMessage(hWndMisFilCon, GWCM_MOVETOCOSINE, MAKEWPARAM(92, 19), MAKELPARAM(MenuSpeed, 20));
					MiscFXVisible = TRUE;
				}
			}
		}
		if ((HWND)lParam == hWndMisFilSmooth) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				SmoothBitmapOps(*SSSOldBitmapPtr);
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndViewImage) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (IsWindow(hWndView)) {
					SetFocus(hWndView);
				} else {
					hWndView = CreateWindowEx(NULL, "MultiSoftSunProjectView", "Image", WS_POPUP | WS_THICKFRAME /*WS_DLGFRAME */| WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, GetSystemMetrics(SM_CXSCREEN) / 2 - 323, GetSystemMetrics(SM_CYSCREEN) / 2 - 243, 646, 486, NULL, NULL, hInstance, NULL);
					if (hWndView) {
						//SendMessage(hWndView, WM_USER, 0, 0);
						ShowWindow(hWndView, SW_SHOW);
						UpdateWindow(hWndView);
					}
				}
			}
		}
		if ((HWND)lParam == hWndOpen) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				OPENFILENAME ofn = {0};
				char txFileName[MAX_PATH] = {0};
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.BMP\0All Files (*.*)\0*.*\0\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = txFileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
				ofn.lpstrDefExt = "bmp";
				if (GetOpenFileName(&ofn)) {
					LONG32 Width = SSSOldBitmapPtr->Width();
					LONG32 Height = SSSOldBitmapPtr->Height();
					SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
					SSSOldBitmapPtr->LoadImage(txFileName);
					if (CurStar.UseExtBuffer && CurStar.ExtPreviewable) {
						if (MessageBox(hWnd, "Re-scale the current star?\n\nNote that this will also clear the current star.", "Confirm star re-scale", MB_YESNO | MB_ICONQUESTION) == IDYES) {
							//Re-scale the star position and size:
							CurStar.ExtPreviewable = FALSE;
							CurStar.ExtBufferPreview = FALSE;
							CurStar.XPos = (double)CurStar.XPos / (double)Width * (double)SSSOldBitmapPtr->Width() + 0.5;
							CurStar.YPos = (double)CurStar.YPos / (double)Height * (double)SSSOldBitmapPtr->Height() + 0.5;
							CurStar.StarRadius = (double)CurStar.StarRadius / (double)((Width + Height) / 2) * (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) + 0.5;
							CurStar.CoronaRadius = (double)CurStar.CoronaRadius / (double)((Width + Height) / 2) * (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) + 0.5;
						}
					} else {
						//Re-scale the star position and size:
						CurStar.XPos = (double)CurStar.XPos / (double)Width * (double)SSSOldBitmapPtr->Width() + 0.5;
						CurStar.YPos = (double)CurStar.YPos / (double)Height * (double)SSSOldBitmapPtr->Height() + 0.5;
						CurStar.StarRadius = (double)CurStar.StarRadius / (double)((Width + Height) / 2) * (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) + 0.5;
						CurStar.CoronaRadius = (double)CurStar.CoronaRadius / (double)((Width + Height) / 2) * (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) + 0.5;
					}
				}
				if (IsWindow(hWndView))
					InvalidateRect(hWndView, NULL, TRUE);
				InvalidateRect(hWndPreview, NULL, TRUE);
			}
		}
		if ((HWND)lParam == hWndSave) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				if (bDrawing) break;
				OPENFILENAME ofn = {0};
				char txFileName[MAX_PATH] = {0};
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.BMP\0All Files (*.*)\0*.*\0\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = txFileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
				ofn.lpstrDefExt = "bmp";
				if (GetSaveFileName(&ofn)) {
					if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview) {
						if (MessageBox(NULL, "The current star is currently only being PREVIEWED!!!\nWould you like to finalise the star before saving?\n\nNote that clicking no will result in the image being saved WITHOUT the star visible.", "Star not finalised!", MB_YESNO | MB_ICONQUESTION) == IDYES) {
							MainWndProc(hWnd, WM_COMMAND, MAKEWPARAM(GBN_CLICKED, GBN_CLICKED), (LPARAM)hWndStarFinish);
						}
					}
					SSSOldBitmapPtr->DumpImage(txFileName);
				}
			}
		}
		if ((HWND)lParam == hWndHelp) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				//Get the current directory:
				char FileName[MAX_PATH] = {0};
				GetModuleFileName(NULL, FileName, MAX_PATH);
				for (DWORD i = strlen(FileName); i > 0; i--) {
					if (FileName[i] == '\\') {
						FileName[i] = '\0';
						break;
					}
				}
				strcat(FileName, "\\help\\index.html");
				ShellExecute(NULL, NULL, FileName, NULL, NULL, NULL);
			}
		}
		if ((HWND)lParam == hWndExit) {
			if (HIWORD(wParam) == GBN_CLICKED) {
				DestroyWindow(hWnd);
			}
		}
		break;
	case WM_ERASEBKGND:
		WindowBackground.CopyImageArrayToDC(hWnd);
		return 0;
	case WM_NCPAINT:
		return SendMessage(hWndTitle, GTM_NCPARENTPAINT, wParam, lParam);
	case WM_SYSCOMMAND:
	case WM_INITMENU:
	case WM_SIZING:
	case WM_SIZE:
		return SendMessage(hWndTitle, Message, wParam, lParam);
	case WM_CLOSE:
		break;
	case WM_DESTROY:
		bRunning = false;
		UnregisterClass("MultiSoftSunProjectPrev", hInstance);
		if (SSSOldBitmapPtr) delete SSSOldBitmapPtr; SSSOldBitmapPtr = NULL;
		if (SSSUndoBitmapPtr) delete SSSUndoBitmapPtr; SSSUndoBitmapPtr = NULL;
		if (SSSBufferBitmapPtr) delete SSSBufferBitmapPtr; SSSBufferBitmapPtr = NULL;
		for (int i = 0; i < 5; i++) {
			if (NebulaParams.LayerParams[i].GradientArray) delete [] NebulaParams.LayerParams[i].GradientArray; NebulaParams.LayerParams[i].GradientArray = NULL;
			if (FlareParams.LayerParams[i].GradientArray) delete [] FlareParams.LayerParams[i].GradientArray; FlareParams.LayerParams[i].GradientArray = NULL;
		}
		PostQuitMessage(0);
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}

LRESULT CALLBACK PrevWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	static LONG32 OldPosX, OldPosY, OldStarRadiusX, OldStarRadiusY, OldCoronaRadiusX, OldCoronaRadiusY;
	double TempStorage;
	switch(Message) {
	case WM_NCPAINT:
		{
			COLORREF clrFace = 0xCC6600,
				clrDarkShadow = 0x0,
				clrShadow = 0xCC3300,
				clrExtraHighlight = 0xCC6600,
				clrHighlight = 0xCC9900;

			//Borders:
			RECT ClientRect;
			LONG32 BorderSizeX = 0;
			LONG32 BorderSizeY = 0;
			GetClientRect(hWnd, &ClientRect);
			HDC hDc =  GetWindowDC(hWnd);
			if (GetWindowLong(hWnd, GWL_STYLE) & WS_THICKFRAME || GetWindowLong(hWnd, GWL_STYLE) & WS_DLGFRAME || GetWindowLong(hWnd, GWL_STYLE) & DS_MODALFRAME) {
				BorderSizeX = GetSystemMetrics(SM_CXDLGFRAME);
				BorderSizeY = GetSystemMetrics(SM_CYDLGFRAME);
			}
			if (GetWindowLong(hWnd, GWL_STYLE) & WS_BORDER) {
				BorderSizeX = GetSystemMetrics(SM_CXBORDER);
				BorderSizeY = GetSystemMetrics(SM_CYBORDER);
			}
			ClientRect.right += 2 * BorderSizeX;
			ClientRect.bottom += 2 * BorderSizeY;

			if (BorderSizeX == 3 && BorderSizeY == 3) {
				HPEN hOldPen, hPen;
				HBRUSH hOldBrush, hBrush;

				hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
				hOldBrush = (HBRUSH)SelectObject(hDc, hBrush);
				hPen = CreatePen(PS_SOLID, 1, clrFace);
				hOldPen = (HPEN)SelectObject(hDc, hPen);

				//Face:
				Rectangle(hDc, ClientRect.left + 2, ClientRect.top + 2, ClientRect.right - 2, ClientRect.bottom - 2);

				//Highlight:
				SelectObject(hDc, hOldPen);
				DeleteObject(hPen);
				hPen = CreatePen(PS_SOLID, 1, clrHighlight);
				hOldPen = (HPEN)SelectObject(hDc, hPen);
				
				MoveToEx(hDc, ClientRect.right - 3, ClientRect.top + 1, NULL);
				LineTo(hDc, ClientRect.left + 1, ClientRect.top + 1);
				LineTo(hDc, ClientRect.left + 1, ClientRect.bottom - 2);
				
				//Shadow:
				SelectObject(hDc, hOldPen);
				DeleteObject(hPen);
				hPen = CreatePen(PS_SOLID, 1, clrShadow);
				hOldPen = (HPEN)SelectObject(hDc, hPen);
				SelectObject(hDc, hPen);
				
				LineTo(hDc, ClientRect.right - 2, ClientRect.bottom - 2);
				LineTo(hDc, ClientRect.right - 2, ClientRect.top);

				//3D Highlight:
				SelectObject(hDc, hOldPen);
				DeleteObject(hPen);
				hPen = CreatePen(PS_SOLID, 1, clrExtraHighlight);
				hOldPen = (HPEN)SelectObject(hDc, hPen);
				SelectObject(hDc, hPen);
				
				LineTo(hDc, ClientRect.left, ClientRect.top);
				LineTo(hDc, ClientRect.left, ClientRect.bottom - 1);
				
				//Dark Shadow:
				SelectObject(hDc, hOldPen);
				DeleteObject(hPen);
				hPen = CreatePen(PS_SOLID, 1, clrDarkShadow);
				hOldPen = (HPEN)SelectObject(hDc, hPen);
				SelectObject(hDc, hPen);
				
				LineTo(hDc, ClientRect.right - 1, ClientRect.bottom - 1);
				LineTo(hDc, ClientRect.right - 1, ClientRect.top - 1);
				

				//Clean up:
				SelectObject(hDc, hOldBrush);
				//DeleteObject(hBrush);
				SelectObject(hDc, hOldPen);
				DeleteObject(hPen);
			}


			ReleaseDC(hWnd, hDc);
			return TRUE;
		}
	case WM_TIMER:
	case WM_ERASEBKGND:
		{
			if (!bRunning) return 0;
			BitmapOps *DrawBmp = SSSOldBitmapPtr;
			if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview && !bDrawing) {
				DrawBmp = new BitmapOps(1, 1);
				DrawBmp->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				{
					//Copy the star to the preview:
					LONG32 MinX, MinY, MaxX, MaxY, MaxC;
					LONG32 Width = DrawBmp->Width();
					LONG32 Height = DrawBmp->Height();
					MaxC = CurStar.StarRadius + CurStar.CoronaRadius;
					LONG32 OffX = CurStar.XPos - MaxC;
					LONG32 OffY = Height - CurStar.YPos - MaxC;
					MaxC += MaxC; //Double it
					if (OffX < 0) MinX = -OffX;
					else MinX = 0;
					if (OffY < 0) MinY = -OffY;
					else MinY = 0;
					if (OffX + MaxC > Width)
						MaxX = MaxC - (OffX + MaxC - Width);
					else MaxX = MaxC;
					if (OffY + MaxC > Height)
						MaxY = MaxC - (OffY + MaxC - Height);
					else MaxY = MaxC;
					DWORD i, j, X, Y;
					double Alpha;
					for (Y = MinY; Y < MaxY; Y++) {
						for (X = MinX; X < MaxX; X++) {
							i = CurStar.ExternalBuffer->GetPixelIndex(X, Y);
							j = DrawBmp->GetPixelIndex(X + OffX, Y + OffY);
							Alpha = (double)CurStar.ExternalBuffer->PixelData[i + 3] / 255.0;
							DrawBmp->PixelData[j + 2] = DrawBmp->PixelData[j + 2] * Alpha + CurStar.ExternalBuffer->PixelData[i + 2];
							DrawBmp->PixelData[j + 1] = DrawBmp->PixelData[j + 1] * Alpha + CurStar.ExternalBuffer->PixelData[i + 1];
							DrawBmp->PixelData[j] = DrawBmp->PixelData[j] * Alpha + CurStar.ExternalBuffer->PixelData[i];
						}
					}
				}
			}
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			HDC hDc = GetDC(hWnd);
			SetStretchBltMode(hDc, HALFTONE);
			SetBrushOrgEx(hDc, 0, 0, NULL);
			DrawBmp->CopyImageArrayToDC(hWnd, hDc);
			if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview && !bDrawing) {
				delete DrawBmp; DrawBmp = NULL;
			}
			if (bDrawing) {
				//HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, GetStockObject(BLACK_BRUSH));
				//Rectangle(hDc, ClientRect.left, ClientRect.top, ClientRect.right, ClientRect.bottom);
				//SelectObject(hDc, hOldBrush);
				/*SetStretchBltMode(hDc, HALFTONE);
				SetBrushOrgEx(hDc, 0, 0, NULL);
				SSSOldBitmapPtr->CopyImageArrayToDC(hWnd, hDc);*/
				SetTextColor(hDc, RGB(255, 0, 0));
				SetBkMode(hDc, 0);
				char Message[64];
				if (AnimInfo.DrawingAnimation) {
					char tmpChar[16];
					strcpy(Message, "Drawing frame ");
					ltoa(AnimInfo.CurrentFrame, tmpChar, 10);
					strcat(Message, tmpChar);
					strcat(Message, " / ");
					ltoa(AnimInfo.TotalFrames, tmpChar, 10);
					strcat(Message, tmpChar);
					strcat(Message, "...");
				} else {
					 strcpy(Message, "Drawing...");
				}
				SIZE txSize;
				GetTextExtentPoint32(hDc, Message, strlen(Message), &txSize);
				TextOut(hDc, ClientRect.right / 2 - txSize.cx / 2, ClientRect.bottom / 2 - txSize.cy / 2, Message, strlen(Message));
				SetTimer(hWnd, 1, 500, NULL);
			} else {
				KillTimer(hWnd, 1);
				/*SetStretchBltMode(hDc, HALFTONE);
				SetBrushOrgEx(hDc, 0, 0, NULL);
				SSSOldBitmapPtr->CopyImageArrayToDC(hWnd, hDc);*/
			}
			if (CurStar.SettingState == Positioning || CurStar.SettingState == StarSizing || CurStar.SettingState == CoronaSizing) {
				//Reset the 'Old' stuff:
				OldPosX = (double)CurStar.XPos / (double)SSSOldBitmapPtr->Width() * (double)ClientRect.right + 0.5;
				OldPosY = (double)CurStar.YPos / (double)SSSOldBitmapPtr->Height() * (double)ClientRect.bottom + 0.5;
				OldStarRadiusX = (double)CurStar.StarRadius / (double)SSSOldBitmapPtr->Width() * (double)ClientRect.right + 0.5;
				OldStarRadiusY = (double)CurStar.StarRadius / (double)SSSOldBitmapPtr->Height() * (double)ClientRect.bottom + 0.5;
				OldCoronaRadiusX = (double)CurStar.CoronaRadius / (double)SSSOldBitmapPtr->Width() * (double)ClientRect.right + 0.5;
				OldCoronaRadiusY = (double)CurStar.CoronaRadius / (double)SSSOldBitmapPtr->Height() * (double)ClientRect.bottom + 0.5;
				//Draw preview circles:
				HPEN hOldPen;
				HBRUSH hOldBrush;
				hOldBrush = (HBRUSH)SelectObject(hDc, GetStockObject(NULL_BRUSH));
				hOldPen = (HPEN)SelectObject(hDc, GetStockObject(WHITE_PEN));
				SetROP2(hDc, R2_NOT);
				Ellipse(hDc, OldPosX - OldStarRadiusX, OldPosY - OldStarRadiusY, OldPosX + OldStarRadiusX, OldPosY + OldStarRadiusY);
				if (OldCoronaRadiusX > 0)
					Ellipse(hDc, OldPosX - OldStarRadiusX - OldCoronaRadiusX, OldPosY - OldStarRadiusY - OldCoronaRadiusY, OldPosX + OldStarRadiusX + OldCoronaRadiusX, OldPosY + OldStarRadiusY + OldCoronaRadiusY);
				SetROP2(hDc, R2_COPYPEN);
				SelectObject(hDc, hOldPen);
				SelectObject(hDc, hOldBrush);
			}
			ReleaseDC(hWnd, hDc);
		}
		return 0;
	case WM_MOUSEMOVE:
		{
			if (CurStar.SettingState == Positioning || CurStar.SettingState == StarSizing || CurStar.SettingState == CoronaSizing) {
				RECT ClientRect;
				GetClientRect(hWnd, &ClientRect);
				HDC hDcReal = GetDC(hWnd);
				HDC hDc = CreateCompatibleDC(hDcReal);
				HBITMAP hBitmap = CreateCompatibleBitmap(hDcReal, ClientRect.right, ClientRect.bottom);
				HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDc, hBitmap);
				BitBlt(hDc, 0, 0, ClientRect.right, ClientRect.bottom, hDcReal, 0, 0, SRCCOPY);
				HPEN hOldPen;
				HBRUSH hOldBrush;
				hOldBrush = (HBRUSH)SelectObject(hDc, GetStockObject(NULL_BRUSH));
				hOldPen = (HPEN)SelectObject(hDc, GetStockObject(WHITE_PEN));
				SetROP2(hDc, R2_NOT);
				//Clear the existing preview circles:
				Ellipse(hDc, OldPosX - OldStarRadiusX, OldPosY - OldStarRadiusY, OldPosX + OldStarRadiusX, OldPosY + OldStarRadiusY);
				if (OldCoronaRadiusX > 0)
					Ellipse(hDc, OldPosX - OldStarRadiusX - OldCoronaRadiusX, OldPosY - OldStarRadiusY - OldCoronaRadiusY, OldPosX + OldStarRadiusX + OldCoronaRadiusX, OldPosY + OldStarRadiusY + OldCoronaRadiusY);
				//Reset the 'Old' stuff:
				OldPosX = (double)CurStar.XPos / (double)SSSOldBitmapPtr->Width() * (double)ClientRect.right + 0.5;
				OldPosY = (double)CurStar.YPos / (double)SSSOldBitmapPtr->Height() * (double)ClientRect.bottom + 0.5;
				OldStarRadiusX = (double)CurStar.StarRadius / (double)SSSOldBitmapPtr->Width() * (double)ClientRect.right + 0.5;
				OldStarRadiusY = (double)CurStar.StarRadius / (double)SSSOldBitmapPtr->Height() * (double)ClientRect.bottom + 0.5;
				OldCoronaRadiusX = (double)CurStar.CoronaRadius / (double)SSSOldBitmapPtr->Width() * (double)ClientRect.right + 0.5;
				OldCoronaRadiusY = (double)CurStar.CoronaRadius / (double)SSSOldBitmapPtr->Height() * (double)ClientRect.bottom + 0.5;
				if (CurStar.SettingState == Positioning) {
					//Set the new 'Old' stuff:
					OldPosX = (SHORT)LOWORD(lParam);
					OldPosY = (SHORT)HIWORD(lParam);
				} else if (CurStar.SettingState == StarSizing) {
					//OldStarRadiusX = sqrt(pow(LOWORD(lParam) - OldPosX, 2) + pow(HIWORD(lParam) - OldPosY, 2));
					//OldStarRadiusY = sqrt(pow(LOWORD(lParam) - OldPosX, 2) + pow(HIWORD(lParam) - OldPosY, 2));
					//OldStarRadiusX = (double)(SHORT)LOWORD(lParam) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width() + 0.5;
					//OldStarRadiusY = (double)(SHORT)HIWORD(lParam) / (double)ClientRect.bottom * (double)SSSOldBitmapPtr->Height() + 0.5;
					//TempStorage = sqrt(pow(OldStarRadiusX - CurStar.XPos, 2) + pow(OldStarRadiusY - CurStar.YPos, 2));
					TempStorage = sqrt(pow((double)(SHORT)LOWORD(lParam) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width() - CurStar.XPos, 2) + pow((double)(SHORT)HIWORD(lParam) / (double)ClientRect.bottom * (double)SSSOldBitmapPtr->Height() - CurStar.YPos, 2));
					OldStarRadiusX = (double)TempStorage / (double)SSSOldBitmapPtr->Width() * (double)ClientRect.right + 0.5;
					OldStarRadiusY = (double)TempStorage / (double)SSSOldBitmapPtr->Height() * (double)ClientRect.bottom + 0.5;
				} else if (CurStar.SettingState == CoronaSizing) {
					//OldCoronaRadiusX = sqrt(pow(LOWORD(lParam) - OldPosX, 2) + pow(HIWORD(lParam) - OldPosY, 2)) - OldStarRadiusX;
					//OldCoronaRadiusY = sqrt(pow(LOWORD(lParam) - OldPosX, 2) + pow(HIWORD(lParam) - OldPosY, 2)) - OldStarRadiusY;
					//OldCoronaRadiusX = (double)(SHORT)LOWORD(lParam) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width() + 0.5;
					//OldCoronaRadiusY = (double)(SHORT)HIWORD(lParam) / (double)ClientRect.bottom * (double)SSSOldBitmapPtr->Height() + 0.5;
					//TempStorage = sqrt(pow(OldCoronaRadiusX - CurStar.XPos, 2) + pow(OldCoronaRadiusY - CurStar.YPos, 2));
					TempStorage = sqrt(pow((double)(SHORT)LOWORD(lParam) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width() - CurStar.XPos, 2) + pow((double)(SHORT)HIWORD(lParam) / (double)ClientRect.bottom * (double)SSSOldBitmapPtr->Height() - CurStar.YPos, 2));
					OldCoronaRadiusX = (double)(TempStorage - (double)OldStarRadiusX / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width()) / (double)SSSOldBitmapPtr->Width() * (double)ClientRect.right + 0.5;
					OldCoronaRadiusY = (double)(TempStorage - (double)OldStarRadiusY / (double)ClientRect.bottom * (double)SSSOldBitmapPtr->Height()) / (double)SSSOldBitmapPtr->Height() * (double)ClientRect.bottom + 0.5;
				}
				//Draw the new preview circles:
				Ellipse(hDc, OldPosX - OldStarRadiusX, OldPosY - OldStarRadiusY, OldPosX + OldStarRadiusX, OldPosY + OldStarRadiusY);
				if (OldCoronaRadiusX > 0)
					Ellipse(hDc, OldPosX - OldStarRadiusX - OldCoronaRadiusX, OldPosY - OldStarRadiusY - OldCoronaRadiusY, OldPosX + OldStarRadiusX + OldCoronaRadiusX, OldPosY + OldStarRadiusY + OldCoronaRadiusY);
				SetROP2(hDc, R2_COPYPEN);
				SelectObject(hDc, hOldPen);
				BitBlt(hDcReal, 0, 0, ClientRect.right, ClientRect.bottom, hDc, 0, 0, SRCCOPY);
				SelectObject(hDc, hOldBrush);
				SelectObject(hDc, hOldBitmap);
				DeleteObject(hBitmap);
				DeleteDC(hDc);
				ReleaseDC(hWnd, hDcReal);
			}
		}
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		break;
	case WM_LBUTTONUP:
		{
			if (GetCapture() == hWnd) ReleaseCapture();
			if (CurStar.SettingState == Positioning || CurStar.SettingState == StarSizing || CurStar.SettingState == CoronaSizing) {
				RECT ClientRect;
				GetClientRect(hWnd, &ClientRect);
				if (CurStar.SettingState == Positioning) {
					CurStar.XPos = (double)(SHORT)LOWORD(lParam) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width() + 0.5;
					CurStar.YPos = (double)(SHORT)HIWORD(lParam) / (double)ClientRect.bottom * (double)SSSOldBitmapPtr->Height() + 0.5;
				} else if (CurStar.SettingState == StarSizing) {
					//OldStarRadiusX = (double)sqrt(pow(LOWORD(lParam) - OldPosX, 2) + pow(HIWORD(lParam) - OldPosY, 2)) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width() + 0.5;
					//OldStarRadiusY = (double)sqrt(pow(LOWORD(lParam) - OldPosX, 2) + pow(HIWORD(lParam) - OldPosY, 2)) / (double)ClientRect.bottom * (double)SSSOldBitmapPtr->Height() + 0.5;
					//CurStar.StarRadius = (double)sqrt(pow(LOWORD(lParam) - OldPosX, 2) + pow(HIWORD(lParam) - OldPosY, 2)) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width() + 0.5;
					CurStar.StarRadius = sqrt(pow((double)(SHORT)LOWORD(lParam) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width() - CurStar.XPos, 2) + pow((double)(SHORT)HIWORD(lParam) / (double)ClientRect.bottom * (double)SSSOldBitmapPtr->Height() - CurStar.YPos, 2));
				} else if (CurStar.SettingState == CoronaSizing) {
					//OldCoronaRadiusX = (double)(sqrt(pow(LOWORD(lParam) - OldPosX, 2) + pow(HIWORD(lParam) - OldPosY, 2)) - OldStarRadiusX) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width() + 0.5;
					//OldCoronaRadiusY = (double)(sqrt(pow(LOWORD(lParam) - OldPosX, 2) + pow(HIWORD(lParam) - OldPosY, 2)) - OldStarRadiusY) / (double)ClientRect.bottom * (double)SSSOldBitmapPtr->Height() + 0.5;
					//CurStar.CoronaRadius = (double)(sqrt(pow(LOWORD(lParam) - OldPosX, 2) + pow(HIWORD(lParam) - OldPosY, 2)) - OldStarRadiusX) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width();
					CurStar.CoronaRadius = sqrt(pow((double)(SHORT)LOWORD(lParam) / (double)ClientRect.right * (double)SSSOldBitmapPtr->Width() - CurStar.XPos, 2) + pow((double)(SHORT)HIWORD(lParam) / (double)ClientRect.bottom * (double)SSSOldBitmapPtr->Height() - CurStar.YPos, 2)) - CurStar.StarRadius;
					if (CurStar.CoronaRadius < 0) CurStar.CoronaRadius = 0;
				}
				if (CurStar.SettingState == StarSizing || CurStar.SettingState == CoronaSizing) {
					CurStar.ExtPreviewable = FALSE;
				}
				if (CurStar.SettingState == Positioning && CurStar.ExtPreviewable) {
					CurStar.ExtBufferPreview = TRUE;
				}
				CurStar.SettingState = None;
				if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview) {
					InvalidateRect(hWnd, NULL, TRUE);
					if (IsWindow(hWndView))
						InvalidateRect(hWndView, NULL, TRUE);
				} else {
					HDC hDc = GetDC(hWnd);
					HPEN hOldPen;
					HBRUSH hOldBrush;
					hOldBrush = (HBRUSH)SelectObject(hDc, GetStockObject(NULL_BRUSH));
					hOldPen = (HPEN)SelectObject(hDc, GetStockObject(WHITE_PEN));
					SetROP2(hDc, R2_NOT);
					//Clear the existing preview circles:
					Ellipse(hDc, OldPosX - OldStarRadiusX, OldPosY - OldStarRadiusY, OldPosX + OldStarRadiusX, OldPosY + OldStarRadiusY);
					if (OldCoronaRadiusX > 0)
						Ellipse(hDc, OldPosX - OldStarRadiusX - OldCoronaRadiusX, OldPosY - OldStarRadiusY - OldCoronaRadiusY, OldPosX + OldStarRadiusX + OldCoronaRadiusX, OldPosY + OldStarRadiusY + OldCoronaRadiusY);
					SetROP2(hDc, R2_COPYPEN);
					SelectObject(hDc, hOldPen);
					SelectObject(hDc, hOldBrush);
					ReleaseDC(hWnd, hDc);
				}
			}
		}
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}

LRESULT CALLBACK ViewWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	static HWND hWndTitle = NULL;
	static HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	static BOOL Stretch = FALSE;
	static double OffX = 0;
	static double OffY = 16;
	static LONG32 DOffX, DOffY, OldWidth, OldHeight;
	static BOOL Dragging = FALSE;
	switch(Message) {
	case WM_CREATE:
		{
			hWndTitle = CreateWindowEx(NULL, "MultiSoftGTitle", "Image", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 0, 0, hWnd, NULL, hInstance, NULL);
			if (!hWndTitle) return -1;

			if (Stretch) {
				if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview && !bDrawing) {
					SetWindowText(hWnd, "Image (Star Preview, Stretched)");
					SetWindowText(hWndTitle, "Image (Star Preview, Stretched)");
				} else {
					SetWindowText(hWnd, "Image (Stretched)");
					SetWindowText(hWndTitle, "Image (Stretched)");
				}
			} else {
				if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview && !bDrawing) {
					SetWindowText(hWnd, "Image (Star Preview)");
					SetWindowText(hWndTitle, "Image (Star Preview)");
				} else {
					SetWindowText(hWnd, "Image");
					SetWindowText(hWndTitle, "Image");
				}
			}
			OffX = 320 - SSSOldBitmapPtr->Width() / 2;
			OffY = 16 + 232 - SSSOldBitmapPtr->Height() / 2;
			OldWidth = 640; OldHeight = 480;

			//Title bar:
			SendMessage(hWndTitle, GTM_SETHEIGHT, NULL, 16);
			SendMessage(hWndTitle, GTM_SETBARIMAGE, NULL, IDB_TITLEBAR);
			SendMessage(hWndTitle, GTM_SETTEXTCOLOUR, NULL, 0x80);
			//Close Button:
			HWND hWndClose = (HWND)SendMessage(hWndTitle, GTM_GETCLOSEHWND, NULL, NULL);
			SendMessage(hWndClose, GBM_SETOFFIMAGE, NULL, IDB_CLOSEOFF);
			SendMessage(hWndClose, GBM_SETOVERIMAGE, NULL, IDB_CLOSEOVER);
			SendMessage(hWndClose, GBM_SETONIMAGE, NULL, IDB_CLOSEON);
			//Max Button:
			HWND hWndMax = (HWND)SendMessage(hWndTitle, GTM_GETMAXHWND, NULL, NULL);
			SendMessage(hWndMax, GBM_SETOFFIMAGE, NULL, IDB_MAXOFF);
			SendMessage(hWndMax, GBM_SETOVERIMAGE, NULL, IDB_MAXOVER);
			SendMessage(hWndMax, GBM_SETONIMAGE, NULL, IDB_MAXON);
			//Restore Button:
			HWND hWndRestore = (HWND)SendMessage(hWndTitle, GTM_GETRESTOREHWND, NULL, NULL);
			SendMessage(hWndRestore, GBM_SETOFFIMAGE, NULL, IDB_RESTOREOFF);
			SendMessage(hWndRestore, GBM_SETOVERIMAGE, NULL, IDB_RESTOREOVER);
			SendMessage(hWndRestore, GBM_SETONIMAGE, NULL, IDB_RESTOREON);
			//Min Button:
			HWND hWndMin = (HWND)SendMessage(hWndTitle, GTM_GETMINHWND, NULL, NULL);
			SendMessage(hWndMin, GBM_SETOFFIMAGE, NULL, IDB_MINOFF);
			SendMessage(hWndMin, GBM_SETOVERIMAGE, NULL, IDB_MINOVER);
			SendMessage(hWndMin, GBM_SETONIMAGE, NULL, IDB_MINON);
		}
		break;
	case WM_TIMER:
	case WM_ERASEBKGND:
		{
			if (!bRunning) return 0;
			BitmapOps *DrawBmp = SSSOldBitmapPtr;
			if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview && !bDrawing) {
				DrawBmp = new BitmapOps(1, 1);
				DrawBmp->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				{
					//Copy the star to the preview:
					LONG32 MinX, MinY, MaxX, MaxY, MaxC;
					LONG32 Width = DrawBmp->Width();
					LONG32 Height = DrawBmp->Height();
					MaxC = CurStar.StarRadius + CurStar.CoronaRadius;
					LONG32 OffX = CurStar.XPos - MaxC;
					LONG32 OffY = Height - CurStar.YPos - MaxC;
					MaxC += MaxC; //Double it
					if (OffX < 0) MinX = -OffX;
					else MinX = 0;
					if (OffY < 0) MinY = -OffY;
					else MinY = 0;
					if (OffX + MaxC > Width)
						MaxX = MaxC - (OffX + MaxC - Width);
					else MaxX = MaxC;
					if (OffY + MaxC > Height)
						MaxY = MaxC - (OffY + MaxC - Height);
					else MaxY = MaxC;
					DWORD i, j, X, Y;
					double Alpha;
					for (Y = MinY; Y < MaxY; Y++) {
						for (X = MinX; X < MaxX; X++) {
							i = CurStar.ExternalBuffer->GetPixelIndex(X, Y);
							j = DrawBmp->GetPixelIndex(X + OffX, Y + OffY);
							Alpha = (double)CurStar.ExternalBuffer->PixelData[i + 3] / 255.0;
							DrawBmp->PixelData[j + 2] = DrawBmp->PixelData[j + 2] * Alpha + CurStar.ExternalBuffer->PixelData[i + 2];
							DrawBmp->PixelData[j + 1] = DrawBmp->PixelData[j + 1] * Alpha + CurStar.ExternalBuffer->PixelData[i + 1];
							DrawBmp->PixelData[j] = DrawBmp->PixelData[j] * Alpha + CurStar.ExternalBuffer->PixelData[i];
						}
					}
				}
			}
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			HDC hDc = GetDC(hWnd);
			if (!Stretch) {
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, hBrushNothing);//GetStockObject(BLACK_BRUSH));
				HPEN hOldPen = (HPEN)SelectObject(hDc, GetStockObject(NULL_PEN));
				Rectangle(hDc, 0, 0, ClientRect.right + 1, OffY + 1);
				Rectangle(hDc, 0, OffY, OffX + 1, OffY + DrawBmp->Height() + 1);
				Rectangle(hDc, OffX + DrawBmp->Width(), OffY, ClientRect.right + 1, OffY + DrawBmp->Height() + 1);
				Rectangle(hDc, 0, OffY + DrawBmp->Height(), ClientRect.right + 1, ClientRect.bottom + 1);
				SelectObject(hDc, hOldPen);
				SelectObject(hDc, hOldBrush);
			}
			if (bDrawing) {
				/*HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, GetStockObject(BLACK_BRUSH));
				Rectangle(hDc, ClientRect.left, ClientRect.top, ClientRect.right, ClientRect.bottom);
				SelectObject(hDc, hOldBrush);*/
				if (Stretch) {
					SetStretchBltMode(hDc, HALFTONE);
					SetBrushOrgEx(hDc, 0, 0, NULL);
					//DrawBmp->CopyImageArrayToDC(hWnd, hDc);
					DrawBmp->CopyImageArrayToDC(hWnd, hDc, 0, 16, ClientRect.right, ClientRect.bottom - 16);
					char Message[64];
					if (AnimInfo.DrawingAnimation) {
						char tmpChar[16];
						strcpy(Message, "Image (Stretched) - Drawing frame ");
						ltoa(AnimInfo.CurrentFrame, tmpChar, 10);
						strcat(Message, tmpChar);
						strcat(Message, " / ");
						ltoa(AnimInfo.TotalFrames, tmpChar, 10);
						strcat(Message, tmpChar);
						strcat(Message, "...");
					} else {
						 strcpy(Message, "Image (Stretched) - Drawing...");
					}
					SetWindowText(hWnd, Message);
					SetWindowText(hWndTitle, Message);
				} else {
					DrawBmp->CopyImageArrayToDC(hWnd, hDc, OffX, OffY, DrawBmp->Width(), DrawBmp->Height());
					//DrawBmp->CopyImageArrayToDC(hWnd, hDc, 0, 16, DrawBmp->Width(), DrawBmp->Height());
					char Message[64];
					if (AnimInfo.DrawingAnimation) {
						char tmpChar[16];
						strcpy(Message, "Image - Drawing frame ");
						ltoa(AnimInfo.CurrentFrame, tmpChar, 10);
						strcat(Message, tmpChar);
						strcat(Message, " / ");
						ltoa(AnimInfo.TotalFrames, tmpChar, 10);
						strcat(Message, tmpChar);
						strcat(Message, "...");
					} else {
						 strcpy(Message, "Image - Drawing...");
					}
					SetWindowText(hWnd, Message);
					SetWindowText(hWndTitle, Message);
				}
				SetTextColor(hDc, RGB(255, 0, 0));
				SetBkMode(hDc, 0);
				char Message[64];
				if (AnimInfo.DrawingAnimation) {
					char tmpChar[16];
					strcpy(Message, "Drawing frame ");
					ltoa(AnimInfo.CurrentFrame, tmpChar, 10);
					strcat(Message, tmpChar);
					strcat(Message, " / ");
					ltoa(AnimInfo.TotalFrames, tmpChar, 10);
					strcat(Message, tmpChar);
					strcat(Message, "...");
				} else {
					 strcpy(Message, "Drawing...");
				}
				SIZE txSize;
				GetTextExtentPoint32(hDc, Message, strlen(Message), &txSize);
				TextOut(hDc, ClientRect.right / 2 - txSize.cx / 2, ClientRect.bottom / 2 - txSize.cy / 2, Message, strlen(Message));
				SetTimer(hWnd, 1, 500, NULL);
			} else {
				KillTimer(hWnd, 1);
				if (Stretch) {
					SetStretchBltMode(hDc, HALFTONE);
					SetBrushOrgEx(hDc, 0, 0, NULL);
					//DrawBmp->CopyImageArrayToDC(hWnd, hDc);
					DrawBmp->CopyImageArrayToDC(hWnd, hDc, 0, 16, ClientRect.right, ClientRect.bottom - 16);
					if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview && !bDrawing) {
						SetWindowText(hWnd, "Image (Star Preview, Stretched)");
						SetWindowText(hWndTitle, "Image (Star Preview, Stretched)");
					} else {
						SetWindowText(hWnd, "Image (Stretched)");
						SetWindowText(hWndTitle, "Image (Stretched)");
					}
				} else {
					DrawBmp->CopyImageArrayToDC(hWnd, hDc, OffX, OffY, DrawBmp->Width(), DrawBmp->Height());
					//DrawBmp->CopyImageArrayToDC(hWnd, hDc, 0, 16, DrawBmp->Width(), DrawBmp->Height());
					if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview && !bDrawing) {
						SetWindowText(hWnd, "Image (Star Preview)");
						SetWindowText(hWndTitle, "Image (Star Preview)");
					} else {
						SetWindowText(hWnd, "Image");
						SetWindowText(hWndTitle, "Image");
					}
				}
			}
			ReleaseDC(hWnd, hDc);
			if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview && !bDrawing) {
				delete DrawBmp; DrawBmp = NULL;
			}
		}
		return 0;
	case WM_RBUTTONUP:
		if (Dragging) {
			Dragging = FALSE;
			ReleaseCapture();
		}
		if (Stretch) {
			Stretch = FALSE;
			if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview && !bDrawing) {
				SetWindowText(hWnd, "Image (Star Preview)");
				SetWindowText(hWndTitle, "Image (Star Preview)");
			} else {
				SetWindowText(hWnd, "Image");
				SetWindowText(hWndTitle, "Image");
			}
		} else {
			Stretch = TRUE;
			if (CurStar.UseExtBuffer && CurStar.ExtBufferPreview && !bDrawing) {
				SetWindowText(hWnd, "Image (Star Preview, Stretched)");
				SetWindowText(hWndTitle, "Image (Star Preview, Stretched)");
			} else {
				SetWindowText(hWnd, "Image (Stretched)");
				SetWindowText(hWndTitle, "Image (Stretched)");
			}
		}
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_CAPTURECHANGED:
		{
			if (GetCapture() != hWnd) Dragging = FALSE;
		}
		return 0;
	case WM_LBUTTONDOWN:
		if (!Stretch) {
			DOffX = (SHORT)LOWORD(lParam) - OffX;
			DOffY = (SHORT)HIWORD(lParam) - OffY;
			Dragging = TRUE;
			SetCapture(hWnd);
		}
		return 0;
	case WM_MOUSEMOVE:
		if (Dragging) {
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			OffX = (SHORT)LOWORD(lParam) - DOffX;
			OffY = (SHORT)HIWORD(lParam) - DOffY;
			if (OffX + SSSOldBitmapPtr->Width() < 0) {DOffX = (SHORT)LOWORD(lParam) + SSSOldBitmapPtr->Width(); OffX = -SSSOldBitmapPtr->Width();}
			if (OffY + SSSOldBitmapPtr->Height() < 16) {DOffY = (SHORT)HIWORD(lParam) + SSSOldBitmapPtr->Height() - 16; OffY = -SSSOldBitmapPtr->Height();}
			if (OffX > ClientRect.right) {DOffX = (SHORT)LOWORD(lParam) - ClientRect.right; OffX = ClientRect.right;}
			if (OffY > ClientRect.bottom) {DOffY = (SHORT)HIWORD(lParam) - ClientRect.bottom; OffY = ClientRect.bottom;}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case WM_LBUTTONUP:
		if (Dragging) {
			Dragging = FALSE;
			ReleaseCapture();
		}
		return 0;
	case WM_NCPAINT:
		return SendMessage(hWndTitle, GTM_NCPARENTPAINT, wParam, lParam);
	case WM_SIZING:
	case WM_SIZE:
		{
			LRESULT lResult = SendMessage(hWndTitle, Message, wParam, lParam);
			WINDOWPLACEMENT WndPlace;
			WndPlace.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hWnd, &WndPlace);
			if (WndPlace.showCmd != SW_SHOWMINIMIZED) {
				RECT ClientRect;
				GetClientRect(hWnd, &ClientRect);
				OffX = (double)(OffX + SSSOldBitmapPtr->Width() / 2) / (double)OldWidth * (double)ClientRect.right - SSSOldBitmapPtr->Width() / 2;
				OffY = (double)(OffY + SSSOldBitmapPtr->Height() / 2) / (double)OldHeight * (double)ClientRect.bottom - SSSOldBitmapPtr->Height() / 2;
				OldWidth = ClientRect.right; OldHeight = ClientRect.bottom;
				InvalidateRect(hWnd, NULL, TRUE);
			}
			return lResult;
		}
	case WM_SYSCOMMAND:
	case WM_INITMENU:
		return SendMessage(hWndTitle, Message, wParam, lParam);
	case WM_DESTROY:
		KillTimer(hWnd, 1);
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}

LRESULT CALLBACK BufferWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	static HWND hWndTitle = NULL;
	static HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	static BOOL Stretch = FALSE;
	static double OffX = 0;
	static double OffY = 16;
	static LONG32 DOffX, DOffY, OldWidth, OldHeight;
	static BOOL Dragging = FALSE;
	switch(Message) {
	case WM_CREATE:
		{
			hWndTitle = CreateWindowEx(NULL, "MultiSoftGTitle", "Buffer", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 0, 0, hWnd, NULL, hInstance, NULL);
			if (!hWndTitle) return -1;

			if (Stretch) {
				SetWindowText(hWnd, "Buffer (Stretched)");
				SetWindowText(hWndTitle, "Buffer (Stretched)");
			} else {
				SetWindowText(hWnd, "Buffer");
				SetWindowText(hWndTitle, "Buffer");
			}
			OffX = 320 - SSSBufferBitmapPtr->Width() / 2;
			OffY = 16 + 232 - SSSBufferBitmapPtr->Height() / 2;
			OldWidth = 640; OldHeight = 480;

			//Title bar:
			SendMessage(hWndTitle, GTM_SETHEIGHT, NULL, 16);
			SendMessage(hWndTitle, GTM_SETBARIMAGE, NULL, IDB_TITLEBAR);
			SendMessage(hWndTitle, GTM_SETTEXTCOLOUR, NULL, 0x80);
			//Close Button:
			HWND hWndClose = (HWND)SendMessage(hWndTitle, GTM_GETCLOSEHWND, NULL, NULL);
			SendMessage(hWndClose, GBM_SETOFFIMAGE, NULL, IDB_CLOSEOFF);
			SendMessage(hWndClose, GBM_SETOVERIMAGE, NULL, IDB_CLOSEOVER);
			SendMessage(hWndClose, GBM_SETONIMAGE, NULL, IDB_CLOSEON);
			//Max Button:
			HWND hWndMax = (HWND)SendMessage(hWndTitle, GTM_GETMAXHWND, NULL, NULL);
			SendMessage(hWndMax, GBM_SETOFFIMAGE, NULL, IDB_MAXOFF);
			SendMessage(hWndMax, GBM_SETOVERIMAGE, NULL, IDB_MAXOVER);
			SendMessage(hWndMax, GBM_SETONIMAGE, NULL, IDB_MAXON);
			//Restore Button:
			HWND hWndRestore = (HWND)SendMessage(hWndTitle, GTM_GETRESTOREHWND, NULL, NULL);
			SendMessage(hWndRestore, GBM_SETOFFIMAGE, NULL, IDB_RESTOREOFF);
			SendMessage(hWndRestore, GBM_SETOVERIMAGE, NULL, IDB_RESTOREOVER);
			SendMessage(hWndRestore, GBM_SETONIMAGE, NULL, IDB_RESTOREON);
			//Min Button:
			HWND hWndMin = (HWND)SendMessage(hWndTitle, GTM_GETMINHWND, NULL, NULL);
			SendMessage(hWndMin, GBM_SETOFFIMAGE, NULL, IDB_MINOFF);
			SendMessage(hWndMin, GBM_SETOVERIMAGE, NULL, IDB_MINOVER);
			SendMessage(hWndMin, GBM_SETONIMAGE, NULL, IDB_MINON);
		}
		break;
	case WM_ERASEBKGND:
		{
			if (!bRunning) return 0;
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			HDC hDc = GetDC(hWnd);
			if (Stretch) {
				SetStretchBltMode(hDc, HALFTONE);
				SetBrushOrgEx(hDc, 0, 0, NULL);
				//SSSBufferBitmapPtr->CopyImageArrayToDC(hWnd, hDc);
				SSSBufferBitmapPtr->CopyImageArrayToDC(hWnd, hDc, 0, 16, ClientRect.right, ClientRect.bottom - 16);
			} else {
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDc, hBrushNothing);//GetStockObject(BLACK_BRUSH));
				HPEN hOldPen = (HPEN)SelectObject(hDc, GetStockObject(NULL_PEN));
				Rectangle(hDc, 0, 0, ClientRect.right + 1, OffY + 1);
				Rectangle(hDc, 0, OffY, OffX + 1, OffY + SSSBufferBitmapPtr->Height() + 1);
				Rectangle(hDc, OffX + SSSBufferBitmapPtr->Width(), OffY, ClientRect.right + 1, OffY + SSSBufferBitmapPtr->Height() + 1);
				Rectangle(hDc, 0, OffY + SSSBufferBitmapPtr->Height(), ClientRect.right + 1, ClientRect.bottom + 1);
				SelectObject(hDc, hOldPen);
				SelectObject(hDc, hOldBrush);
				SSSBufferBitmapPtr->CopyImageArrayToDC(hWnd, hDc, OffX, OffY, SSSBufferBitmapPtr->Width(), SSSBufferBitmapPtr->Height());
			}
			ReleaseDC(hWnd, hDc);
		}
		return 0;
	case WM_RBUTTONUP:
		if (Dragging) {
			Dragging = FALSE;
			ReleaseCapture();
		}
		if (Stretch) {
			Stretch = FALSE;
			SetWindowText(hWnd, "Buffer");
			SetWindowText(hWndTitle, "Buffer");
		} else {
			Stretch = TRUE;
			SetWindowText(hWnd, "Buffer (Stretched)");
			SetWindowText(hWndTitle, "Buffer (Stretched)");
		}
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_CAPTURECHANGED:
		{
			if (GetCapture() != hWnd) Dragging = FALSE;
		}
		return 0;
	case WM_LBUTTONDOWN:
		if (!Stretch) {
			DOffX = (SHORT)LOWORD(lParam) - OffX;
			DOffY = (SHORT)HIWORD(lParam) - OffY;
			Dragging = TRUE;
			SetCapture(hWnd);
		}
		return 0;
	case WM_MOUSEMOVE:
		if (Dragging) {
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			OffX = (SHORT)LOWORD(lParam) - DOffX;
			OffY = (SHORT)HIWORD(lParam) - DOffY;
			if (OffX + SSSOldBitmapPtr->Width() < 0) DOffX = (SHORT)LOWORD(lParam) + SSSOldBitmapPtr->Width();
			if (OffY + SSSOldBitmapPtr->Height() < 16) DOffY = (SHORT)HIWORD(lParam) + SSSOldBitmapPtr->Height() - 16;
			if (OffX > ClientRect.right) DOffX = (SHORT)LOWORD(lParam) - ClientRect.right;
			if (OffY > ClientRect.bottom) DOffY = (SHORT)HIWORD(lParam) - ClientRect.bottom;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case WM_LBUTTONUP:
		if (Dragging) {
			Dragging = FALSE;
			ReleaseCapture();
		}
		return 0;
	case WM_NCPAINT:
		return SendMessage(hWndTitle, GTM_NCPARENTPAINT, wParam, lParam);
	case WM_SIZING:
	case WM_SIZE:
		{
			LRESULT lResult = SendMessage(hWndTitle, Message, wParam, lParam);
			WINDOWPLACEMENT WndPlace;
			WndPlace.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hWnd, &WndPlace);
			if (WndPlace.showCmd != SW_SHOWMINIMIZED) {
				RECT ClientRect;
				GetClientRect(hWnd, &ClientRect);
				OffX = (double)(OffX + SSSOldBitmapPtr->Width() / 2) / (double)OldWidth * (double)ClientRect.right - SSSOldBitmapPtr->Width() / 2;
				OffY = (double)(OffY + SSSOldBitmapPtr->Height() / 2) / (double)OldHeight * (double)ClientRect.bottom - SSSOldBitmapPtr->Height() / 2;
				OldWidth = ClientRect.right; OldHeight = ClientRect.bottom;
				InvalidateRect(hWnd, NULL, TRUE);
			}
			return lResult;
		}
	case WM_SYSCOMMAND:
	case WM_INITMENU:
		return SendMessage(hWndTitle, Message, wParam, lParam);
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}

LRESULT CALLBACK NebulaDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	BOOL RequireReLoad = FALSE;
	static int CurrentLayer = 0;
	switch(Message) {
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hWnd, IDC_SLDNUMBER), TBM_SETRANGEMAX, TRUE, 4);
		SendMessage(GetDlgItem(hWnd, IDC_SLDCURRENT), TBM_SETRANGEMAX, TRUE, 4);

		SendMessage(GetDlgItem(hWnd, IDC_SLDNUMBER), TBM_SETPOS, TRUE, (*PerlinParams).NumLayers - 1);
		SendMessage(GetDlgItem(hWnd, IDC_SLDCURRENT), TBM_SETPOS, TRUE, CurrentLayer);

		RequireReLoad = TRUE;
		break;
	case WM_HSCROLL:
		{
			if ((HWND)lParam == GetDlgItem(hWnd, IDC_SLDCURRENT)) {
				char txWindowText[5];
				GetWindowText(GetDlgItem(hWnd, IDC_TXTRED), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].RedVariance = strtol(txWindowText, NULL, 0);
				GetWindowText(GetDlgItem(hWnd, IDC_TXTGREEN), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].GreenVariance = strtol(txWindowText, NULL, 0);
				GetWindowText(GetDlgItem(hWnd, IDC_TXTBLUE), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].BlueVariance = strtol(txWindowText, NULL, 0);
				GetWindowText(GetDlgItem(hWnd, IDC_TXTPERSISTENCE), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].Persistance = strtol(txWindowText, NULL, 0) / 100.0;
				GetWindowText(GetDlgItem(hWnd, IDC_TXTSEED), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].Seed = strtol(txWindowText, NULL, 0);
				GetWindowText(GetDlgItem(hWnd, IDC_TXTOCTAVES), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].Octaves = strtol(txWindowText, NULL, 0);
				GetWindowText(GetDlgItem(hWnd, IDC_TXTFIRSTOCTAVE), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].FirstOctave = strtol(txWindowText, NULL, 0);
				if (SendMessage(GetDlgItem(hWnd, IDC_OPTPREVIOUSOVERLAY), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].KeepPrevious = 2;
				else if (SendMessage(GetDlgItem(hWnd, IDC_OPTPREVIOUSUNDERLAY), BM_GETCHECK, 0 ,0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].KeepPrevious = 1;
				else
					(*PerlinParams).LayerParams[CurrentLayer].KeepPrevious = 0;
				if (SendMessage(GetDlgItem(hWnd, IDC_OPTINTERPOLATELINEAR), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].Interpolation = 0;
				else if (SendMessage(GetDlgItem(hWnd, IDC_OPTINTERPOLATECOSINE), BM_GETCHECK, 0 ,0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].Interpolation = 1;
				else
					(*PerlinParams).LayerParams[CurrentLayer].Interpolation = 2;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKBUG), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].Bug = TRUE;
				else
					(*PerlinParams).LayerParams[CurrentLayer].Bug = FALSE;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKTILEH), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].HTile = TRUE;
				else
					(*PerlinParams).LayerParams[CurrentLayer].HTile = FALSE;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKTILEV), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].VTile = TRUE;
				else
					(*PerlinParams).LayerParams[CurrentLayer].VTile = FALSE;
				
				CurrentLayer = SendMessage(GetDlgItem(hWnd, IDC_SLDCURRENT), TBM_GETPOS, 0, 0);

				RequireReLoad = TRUE;
			} else if ((HWND)lParam == GetDlgItem(hWnd, IDC_SLDNUMBER)) {
				
			}
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_COLOUR:
			{
				CHOOSECOLOR cc = {0};
				cc.lStructSize = sizeof(CHOOSECOLOR);
				cc.hwndOwner = hWnd;
				cc.rgbResult = (*PerlinParams).LayerParams[CurrentLayer].MiddleColour;
				cc.lpCustColors = CustomColourArray;
				cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;
				if (ChooseColor(&cc)) {
					(*PerlinParams).LayerParams[CurrentLayer].MiddleColour = cc.rgbResult;
				}
			}
			break;
		case IDC_APPLYALL:
			{
				int BugState = 0;
				int SmoothState = 1;///////////
				int HTileState = 0;
				int VTileState = 0;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKBUG), BM_GETCHECK, 0, 0) == BST_CHECKED)
					BugState = 1;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKTILEH), BM_GETCHECK, 0, 0) == BST_CHECKED)
					HTileState = 1;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKTILEV), BM_GETCHECK, 0, 0) == BST_CHECKED)
					VTileState = 1;
				for (int i = 0; i < 5; i++) {
					(*PerlinParams).LayerParams[i].Bug = BugState;
					(*PerlinParams).LayerParams[i].Smooth = SmoothState;
					(*PerlinParams).LayerParams[i].HTile = HTileState;
					(*PerlinParams).LayerParams[i].VTile = VTileState;
				}
			}
			break;
		case IDC_LASTSEEDS:
			{
				for (int i = 0; i < 5; i++) {
					(*PerlinParams).LayerParams[i].Seed = (*PerlinParams).LayerParams[i].LastSeed;
				}
				char txWindowText[5];
				ltoa((LONG32)((*PerlinParams).LayerParams[CurrentLayer].LastSeed), txWindowText, 10);
				SetWindowText(GetDlgItem(hWnd, IDC_TXTSEED), txWindowText);
			}
			break;
		case IDC_RANDOMISE:
			{
				//Set up the default nebula parameters:
				srand(GetTickCount());rand(); //Seed the random number generator
				for (int i = 0; i < 5; i++) {
					(*PerlinParams).LayerParams[i].RedVariance = rand() % 512;
					(*PerlinParams).LayerParams[i].GreenVariance = rand() % 512;
					(*PerlinParams).LayerParams[i].BlueVariance = rand() % 512;
					(*PerlinParams).LayerParams[i].KeepPrevious = rand() % 2 + 1;
					(*PerlinParams).LayerParams[i].MiddleColour = RGB(rand() % 128, rand() % 128, rand() % 128);
					(*PerlinParams).LayerParams[i].Persistance = (double)rand() / RAND_MAX * 0.5 + 0.5;
					(*PerlinParams).LayerParams[i].FirstOctave = 1;
					(*PerlinParams).LayerParams[i].Bug = 0;
					(*PerlinParams).LayerParams[i].Interpolation = 2;
					(*PerlinParams).LayerParams[i].Octaves = 0;
					(*PerlinParams).LayerParams[i].Seed = 0;
					(*PerlinParams).LayerParams[i].Smooth = 1;
					(*PerlinParams).LayerParams[i].HTile = 0;
					(*PerlinParams).LayerParams[i].VTile = 0;
				}
				(*PerlinParams).LayerParams[0].KeepPrevious = 0;
				(*PerlinParams).NumLayers = rand() % 5 + 1;
				(*PerlinParams).Animate = FALSE;
				(*PerlinParams).SmoothT = TRUE;
				SendMessage(GetDlgItem(hWnd, IDC_SLDNUMBER), TBM_SETPOS, TRUE, (*PerlinParams).NumLayers - 1);
				RequireReLoad = TRUE;
			}
			break;
		case IDC_DEFAULTS:
			{
				//Set up the default nebula parameters:
				for (int i = 0; i < 5; i++) {
					(*PerlinParams).LayerParams[i].RedVariance = 256;
					(*PerlinParams).LayerParams[i].GreenVariance = 256;
					(*PerlinParams).LayerParams[i].BlueVariance = 256;
					(*PerlinParams).LayerParams[i].KeepPrevious = 2;
					(*PerlinParams).LayerParams[i].MiddleColour = 0;
					(*PerlinParams).LayerParams[i].Persistance = 0.75;
					(*PerlinParams).LayerParams[i].FirstOctave = 1;
					(*PerlinParams).LayerParams[i].Bug = 0;
					(*PerlinParams).LayerParams[i].Interpolation = 2;
					(*PerlinParams).LayerParams[i].Octaves = 0;
					(*PerlinParams).LayerParams[i].Seed = 0;
					(*PerlinParams).LayerParams[i].Smooth = 1;
					(*PerlinParams).LayerParams[i].HTile = 0;
					(*PerlinParams).LayerParams[i].VTile = 0;
				}
				(*PerlinParams).LayerParams[0].KeepPrevious = 0;
				(*PerlinParams).NumLayers = 1;
				(*PerlinParams).Animate = FALSE;
				(*PerlinParams).SmoothT = TRUE;
				(*PerlinParams).BaseFileName = NULL;
				(*PerlinParams).FileFormat = 0;
				(*PerlinParams).FramesPerKey = 24;
				(*PerlinParams).KeyFrames = 12;
				(*PerlinParams).Loop = FALSE;
				SendMessage(GetDlgItem(hWnd, IDC_SLDNUMBER), TBM_SETPOS, TRUE, (*PerlinParams).NumLayers - 1);
				RequireReLoad = TRUE;
			}
			break;
		case IDC_GRADIENT:
			{
				SendMessage(hWnd, WM_HSCROLL, IDC_SLDCURRENT, (LPARAM)GetDlgItem(hWnd, IDC_SLDCURRENT));
				HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
				DialogBox(hInstance, MAKEINTRESOURCE(IDD_GRADIENT), hWnd, (DLGPROC)GradientDlgProc);
			}
			break;
		case IDC_CLOSE:
			{
				(*PerlinParams).NumLayers = SendMessage(GetDlgItem(hWnd, IDC_SLDNUMBER), TBM_GETPOS, 0, 0) + 1;
				
				char txWindowText[5];
				GetWindowText(GetDlgItem(hWnd, IDC_TXTRED), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].RedVariance = strtol(txWindowText, NULL, 0);
				GetWindowText(GetDlgItem(hWnd, IDC_TXTGREEN), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].GreenVariance = strtol(txWindowText, NULL, 0);
				GetWindowText(GetDlgItem(hWnd, IDC_TXTBLUE), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].BlueVariance = strtol(txWindowText, NULL, 0);
				GetWindowText(GetDlgItem(hWnd, IDC_TXTPERSISTENCE), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].Persistance = strtol(txWindowText, NULL, 0) / 100.0;
				GetWindowText(GetDlgItem(hWnd, IDC_TXTSEED), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].Seed = strtol(txWindowText, NULL, 0);
				GetWindowText(GetDlgItem(hWnd, IDC_TXTOCTAVES), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].Octaves = strtol(txWindowText, NULL, 0);
				GetWindowText(GetDlgItem(hWnd, IDC_TXTFIRSTOCTAVE), txWindowText, 7);
				(*PerlinParams).LayerParams[CurrentLayer].FirstOctave = strtol(txWindowText, NULL, 0);
				if (SendMessage(GetDlgItem(hWnd, IDC_OPTPREVIOUSOVERLAY), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].KeepPrevious = 2;
				else if (SendMessage(GetDlgItem(hWnd, IDC_OPTPREVIOUSUNDERLAY), BM_GETCHECK, 0 ,0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].KeepPrevious = 1;
				else
					(*PerlinParams).LayerParams[CurrentLayer].KeepPrevious = 0;
				if (SendMessage(GetDlgItem(hWnd, IDC_OPTINTERPOLATELINEAR), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].Interpolation = 0;
				else if (SendMessage(GetDlgItem(hWnd, IDC_OPTINTERPOLATECOSINE), BM_GETCHECK, 0 ,0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].Interpolation = 1;
				else
					(*PerlinParams).LayerParams[CurrentLayer].Interpolation = 2;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKBUG), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].Bug = TRUE;
				else
					(*PerlinParams).LayerParams[CurrentLayer].Bug = FALSE;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKTILEH), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].HTile = TRUE;
				else
					(*PerlinParams).LayerParams[CurrentLayer].HTile = FALSE;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKTILEV), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).LayerParams[CurrentLayer].VTile = TRUE;
				else
					(*PerlinParams).LayerParams[CurrentLayer].VTile = FALSE;

				HWND hWndParent = GetParent(hWnd);
				EndDialog(hWnd, 1);
			}
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	}
	if (RequireReLoad) {
		char txWindowText[5];
		ltoa((*PerlinParams).LayerParams[CurrentLayer].RedVariance, txWindowText, 10);
		SetWindowText(GetDlgItem(hWnd, IDC_TXTRED), txWindowText);
		ltoa((*PerlinParams).LayerParams[CurrentLayer].GreenVariance, txWindowText, 10);
		SetWindowText(GetDlgItem(hWnd, IDC_TXTGREEN), txWindowText);
		ltoa((*PerlinParams).LayerParams[CurrentLayer].BlueVariance, txWindowText, 10);
		SetWindowText(GetDlgItem(hWnd, IDC_TXTBLUE), txWindowText);
		//ltoa(((*PerlinParams).LayerParams[CurrentLayer].Persistance * 100.0), txWindowText, 10);
		//casting a double to a long loses data (eg. 116 -> 115 -> 114 -> 113 -> 112), so use sprintf instead, then get rid of the '.':
		sprintf(txWindowText, "%f", (*PerlinParams).LayerParams[CurrentLayer].Persistance * 100.0);
		for (int i = 0; i < strlen(txWindowText); ++i)
			if (txWindowText[i] == '.')
				txWindowText[i] = '\0';
		SetWindowText(GetDlgItem(hWnd, IDC_TXTPERSISTENCE), txWindowText);
		ltoa((*PerlinParams).LayerParams[CurrentLayer].Seed, txWindowText, 10);
		SetWindowText(GetDlgItem(hWnd, IDC_TXTSEED), txWindowText);
		ltoa((*PerlinParams).LayerParams[CurrentLayer].Octaves, txWindowText, 10);
		SetWindowText(GetDlgItem(hWnd, IDC_TXTOCTAVES), txWindowText);
		ltoa((*PerlinParams).LayerParams[CurrentLayer].FirstOctave, txWindowText, 10);
		SetWindowText(GetDlgItem(hWnd, IDC_TXTFIRSTOCTAVE), txWindowText);
		SendMessage(GetDlgItem(hWnd, IDC_OPTPREVIOUSERASE), BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hWnd, IDC_OPTPREVIOUSUNDERLAY), BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hWnd, IDC_OPTPREVIOUSOVERLAY), BM_SETCHECK, BST_UNCHECKED, 0);
		if ((*PerlinParams).LayerParams[CurrentLayer].KeepPrevious == 2) {
			SendMessage(GetDlgItem(hWnd, IDC_OPTPREVIOUSOVERLAY), BM_SETCHECK, BST_CHECKED, 0);
		} else if ((*PerlinParams).LayerParams[CurrentLayer].KeepPrevious == 1) {
			SendMessage(GetDlgItem(hWnd, IDC_OPTPREVIOUSUNDERLAY), BM_SETCHECK, BST_CHECKED, 0);
		} else {
			SendMessage(GetDlgItem(hWnd, IDC_OPTPREVIOUSERASE), BM_SETCHECK, BST_CHECKED, 0);
		}
		SendMessage(GetDlgItem(hWnd, IDC_OPTINTERPOLATELINEAR), BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hWnd, IDC_OPTINTERPOLATECOSINE), BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hWnd, IDC_OPTINTERPOLATECUBIC), BM_SETCHECK, BST_UNCHECKED, 0);
		if ((*PerlinParams).LayerParams[CurrentLayer].Interpolation == 0) {
			SendMessage(GetDlgItem(hWnd, IDC_OPTINTERPOLATELINEAR), BM_SETCHECK, BST_CHECKED, 0);
		} else if ((*PerlinParams).LayerParams[CurrentLayer].Interpolation == 1) {
			SendMessage(GetDlgItem(hWnd, IDC_OPTINTERPOLATECOSINE), BM_SETCHECK, BST_CHECKED, 0);
		} else {
			SendMessage(GetDlgItem(hWnd, IDC_OPTINTERPOLATECUBIC), BM_SETCHECK, BST_CHECKED, 0);
		}
		if ((*PerlinParams).LayerParams[CurrentLayer].Bug == TRUE) {
			SendMessage(GetDlgItem(hWnd, IDC_CHKBUG), BM_SETCHECK, BST_CHECKED, 0);
		} else {
			SendMessage(GetDlgItem(hWnd, IDC_CHKBUG), BM_SETCHECK, BST_UNCHECKED, 0);
		}
		if ((*PerlinParams).LayerParams[CurrentLayer].HTile == TRUE) {
			SendMessage(GetDlgItem(hWnd, IDC_CHKTILEH), BM_SETCHECK, BST_CHECKED, 0);
		} else {
			SendMessage(GetDlgItem(hWnd, IDC_CHKTILEH), BM_SETCHECK, BST_UNCHECKED, 0);
		}
		if ((*PerlinParams).LayerParams[CurrentLayer].VTile == TRUE) {
			SendMessage(GetDlgItem(hWnd, IDC_CHKTILEV), BM_SETCHECK, BST_CHECKED, 0);
		} else {
			SendMessage(GetDlgItem(hWnd, IDC_CHKTILEV), BM_SETCHECK, BST_UNCHECKED, 0);
		}
	}
	return 0;
}

LRESULT CALLBACK GradientDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	static BitmapOps GradientBmp(512, 1);
	switch (Message) {
	case WM_INITDIALOG:
		{
			SendMessage(GetDlgItem(hWnd, IDC_GRADIENTDISPLAY), GWCM_SETBACKCOLOUR, 0, 0);
			SendMessage(GetDlgItem(hWnd, IDC_GRADIENTDISPLAY), GWCM_SETPARENTPAINT, 0, TRUE);
			if ((*PerlinParams).LayerParams[0].UseGradient) {
				SendMessage(GetDlgItem(hWnd, IDC_CHKUSEGRAD), BM_SETCHECK, BST_CHECKED, 0);
			} else {
				SendMessage(GetDlgItem(hWnd, IDC_CHKUSEGRAD), BM_SETCHECK, BST_UNCHECKED, 0);
			}
			for (DWORD Counter = 0; Counter < 512; Counter++) {
				GradientBmp.SetPColour(Counter, 0, GetRValue((*PerlinParams).LayerParams[0].GradientArray[Counter]), GetGValue((*PerlinParams).LayerParams[0].GradientArray[Counter]), GetBValue((*PerlinParams).LayerParams[0].GradientArray[Counter]));
			}
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_GENERATE:
			{
				srand(GetTickCount()); rand();
				CreatePerlinGradient(1 | 64, RGB(rand() % 128, rand() % 128, rand() % 128), 512, (*PerlinParams).LayerParams[0].GradientArray, rand() % 512, rand() % 512, rand() % 512, (double)rand() / RAND_MAX * 0.5 + 0.5, 0, 1);
				for (DWORD Counter = 0; Counter < 512; Counter++) {
					GradientBmp.SetPColour(Counter, 0, GetRValue((*PerlinParams).LayerParams[0].GradientArray[Counter]), GetGValue((*PerlinParams).LayerParams[0].GradientArray[Counter]), GetBValue((*PerlinParams).LayerParams[0].GradientArray[Counter]));
				}
				//Check the box:
				SendMessage(GetDlgItem(hWnd, IDC_CHKUSEGRAD), BM_SETCHECK, BST_CHECKED, 0);
				//Update the preview window:
				InvalidateRect(GetDlgItem(hWnd, IDC_GRADIENTDISPLAY), NULL, FALSE);
			}
			break;
		case IDC_GENERATENEBPARAMS:
			{
				srand(GetTickCount()); rand();
				CreatePerlinGradient(1 | ((*PerlinParams).LayerParams[0].Bug ? 2 : 0) | ((*PerlinParams).LayerParams[0].HTile ? 4 : 0) | (!(*PerlinParams).LayerParams[0].Interpolation ? 16 : 0) | ((*PerlinParams).LayerParams[0].Interpolation == 2 ? 32 : 0), (*PerlinParams).LayerParams[0].MiddleColour, 512, (*PerlinParams).LayerParams[0].GradientArray, (*PerlinParams).LayerParams[0].RedVariance, (*PerlinParams).LayerParams[0].GreenVariance, (*PerlinParams).LayerParams[0].BlueVariance, (*PerlinParams).LayerParams[0].Persistance, (*PerlinParams).LayerParams[0].Octaves, (*PerlinParams).LayerParams[0].FirstOctave);
				for (DWORD Counter = 0; Counter < 512; Counter++) {
					GradientBmp.SetPColour(Counter, 0, GetRValue((*PerlinParams).LayerParams[0].GradientArray[Counter]), GetGValue((*PerlinParams).LayerParams[0].GradientArray[Counter]), GetBValue((*PerlinParams).LayerParams[0].GradientArray[Counter]));
				}
				//Check the box:
				SendMessage(GetDlgItem(hWnd, IDC_CHKUSEGRAD), BM_SETCHECK, BST_CHECKED, 0);
				//Update the preview window:
				InvalidateRect(GetDlgItem(hWnd, IDC_GRADIENTDISPLAY), NULL, FALSE);
			}
			break;
		case IDC_GRADIENTDISPLAY:
			if (HIWORD(wParam) == GWCN_PAINT) {	
				//HWND hWndC = GetDlgItem(hWnd, IDC_GRADIENTDISPLAY);
				//GradientBmp.CopyImageArrayToDC(hWndC);
				PAINTSTRUCT ps;
				HDC hDc = BeginPaint((HWND)lParam, &ps);
				GradientBmp.CopyImageArrayToDC((HWND)lParam, hDc);
				EndPaint((HWND)lParam, &ps);
			}
			break;
		case IDCLOSE:
			{
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKUSEGRAD), BM_GETCHECK, 0, 0) == BST_CHECKED) {
					(*PerlinParams).LayerParams[0].UseGradient = TRUE;
				} else {
					(*PerlinParams).LayerParams[0].UseGradient = FALSE;
				}
			}
			EndDialog(hWnd, 0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	}
	return 0;
}


LRESULT CALLBACK NewDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hWnd, IDC_DIMENSIONS), CB_ADDSTRING, 0, (LPARAM)"Custom");
		SendMessage(GetDlgItem(hWnd, IDC_DIMENSIONS), CB_ADDSTRING, 0, (LPARAM)"400 x 300");
		SendMessage(GetDlgItem(hWnd, IDC_DIMENSIONS), CB_ADDSTRING, 0, (LPARAM)"640 x 480 - Standard resolution");
		SendMessage(GetDlgItem(hWnd, IDC_DIMENSIONS), CB_ADDSTRING, 0, (LPARAM)"800 x 600 - Standard resolution");
		SendMessage(GetDlgItem(hWnd, IDC_DIMENSIONS), CB_ADDSTRING, 0, (LPARAM)"1024 x 768 - Standard resolution");
		//SendMessage(GetDlgItem(hWnd, IDC_DIMENSIONS), CB_ADDSTRING, 0, (LPARAM)"468 x 60 - Web banner");
		//Select the 'Custom' entry:
		SendMessage(GetDlgItem(hWnd, IDC_DIMENSIONS), CB_SETCURSEL, 0, 0);
		//Place the current dimensions in the edit controls:
		char TempString[5];
		SendMessage(GetDlgItem(hWnd, IDC_WIDTH), WM_SETTEXT, 0, (LPARAM)ltoa(SSSOldBitmapPtr->Width(), TempString, 10));
		SendMessage(GetDlgItem(hWnd, IDC_HEIGHT), WM_SETTEXT, 0, (LPARAM)ltoa(SSSOldBitmapPtr->Height(), TempString, 10));
		//Check the clear buffer control:
		//SendMessage(GetDlgItem(hWnd, IDC_CLEARBUFFER), BM_SETCHECK, BST_CHECKED, 0);
		SetFocus(GetDlgItem(hWnd, IDC_DIMENSIONS));
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_DIMENSIONS:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				//Find the two dimensions
				LRESULT CurSel = SendMessage(GetDlgItem(hWnd, IDC_DIMENSIONS), CB_GETCURSEL, 0, 0);
				switch (CurSel) {
				case 1:
					SendMessage(GetDlgItem(hWnd, IDC_WIDTH), WM_SETTEXT, 0, (LPARAM)"400");
					SendMessage(GetDlgItem(hWnd, IDC_HEIGHT), WM_SETTEXT, 0, (LPARAM)"300");
					break;
				case 2:
					SendMessage(GetDlgItem(hWnd, IDC_WIDTH), WM_SETTEXT, 0, (LPARAM)"640");
					SendMessage(GetDlgItem(hWnd, IDC_HEIGHT), WM_SETTEXT, 0, (LPARAM)"480");
					break;
				case 3:
					SendMessage(GetDlgItem(hWnd, IDC_WIDTH), WM_SETTEXT, 0, (LPARAM)"800");
					SendMessage(GetDlgItem(hWnd, IDC_HEIGHT), WM_SETTEXT, 0, (LPARAM)"600");
					break;
				case 4:
					SendMessage(GetDlgItem(hWnd, IDC_WIDTH), WM_SETTEXT, 0, (LPARAM)"1024");
					SendMessage(GetDlgItem(hWnd, IDC_HEIGHT), WM_SETTEXT, 0, (LPARAM)"768");
					break;
				/*case 5:
					SendMessage(GetDlgItem(hWnd, IDC_WIDTH), WM_SETTEXT, 0, (LPARAM)"468");
					SendMessage(GetDlgItem(hWnd, IDC_HEIGHT), WM_SETTEXT, 0, (LPARAM)"60");
					break;*/
				}
				SendMessage(GetDlgItem(hWnd, IDC_DIMENSIONS), CB_SETCURSEL, CurSel, 0);
			}
			break;
		case IDC_WIDTH:
		case IDC_HEIGHT:
			if (HIWORD(wParam) == EN_UPDATE) {
				SendMessage(GetDlgItem(hWnd, IDC_DIMENSIONS), CB_SETCURSEL, 0, 0);
			}
			break;
		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		case IDOK:
			//Get the dimensions:
			char TempString[5] = {0};
			GetWindowText(GetDlgItem(hWnd, IDC_WIDTH), TempString, 16);
			LONG32 Width = atol(TempString);
			GetWindowText(GetDlgItem(hWnd, IDC_HEIGHT), TempString, 16);
			LONG32 Height = atol(TempString);
			//Ensure the dimensions are valid:
			if (Width < 1 || Height < 1) {
				MessageBox(NULL, "Invalid dimensions have magically appeared somehow,\nWould you please take the liberty to make them valid?\n\n\t\tPlease?", "Invalid Dimensions", MB_ICONEXCLAMATION);
			} else {
				bDrawing = false;
				//SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
				//Re-scale the star position and size:
				CurStar.XPos = (double)CurStar.XPos / (double)SSSOldBitmapPtr->Width() * (double)Width + 0.5;
				CurStar.YPos = (double)CurStar.YPos / (double)SSSOldBitmapPtr->Height() * (double)Height + 0.5;
				CurStar.StarRadius = (double)CurStar.StarRadius / (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) * (double)((Width + Height) / 2) + 0.5;
				CurStar.CoronaRadius = (double)CurStar.CoronaRadius / (double)((SSSOldBitmapPtr->Width() + SSSOldBitmapPtr->Height()) / 2) * (double)((Width + Height) / 2) + 0.5;
				//Resize all of the bitmaps:
				SSSOldBitmapPtr->Resize(Width, Height);
				SSSUndoBitmapPtr->Resize(Width, Height);
				//If the buffer is to be cleared, resize and clear it:
				/*if (SendMessage(GetDlgItem(hWnd, IDC_CLEARBUFFER), BM_GETCHECK, 0, 0) == BST_CHECKED) {
					SSSBufferBitmapPtr->Resize(Width, Height);
					memset(SSSBufferBitmapPtr->PixelData, 0, SSSBufferBitmapPtr->UBound());
				}*/
				//If the buffer is to be resized, resize it:
				if (SendMessage(GetDlgItem(hWnd, IDC_RESIZEBUFFER), BM_GETCHECK, 0, 0) == BST_CHECKED) {
					//Create a new, temporary buffer:
					BitmapOps *tmpBuffer = new BitmapOps(Width, Height);
					tmpBuffer->ResizeBitmapFromBitmapOps(*SSSBufferBitmapPtr);
					SSSBufferBitmapPtr->CopyBitmapFromBitmapOps(*tmpBuffer);
					if (tmpBuffer) delete tmpBuffer;
				}
				//Blank the bitmaps:
				memset(SSSOldBitmapPtr->PixelData, 0, SSSOldBitmapPtr->UBound());
				memset(SSSUndoBitmapPtr->PixelData, 0, SSSUndoBitmapPtr->UBound());
				EndDialog(hWnd, TRUE);
			}
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	}
	return 0;
}



LRESULT CALLBACK MergeDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	static BitmapOps SmPreviewImage1(80, 60); //Image
	static BitmapOps SmPreviewImage2(80, 60); //Buffer
	static BitmapOps SmPreviewImage3(80, 60); //Preview
	switch(Message) {
	case WM_INITDIALOG:
		{
			SendMessage(GetDlgItem(hWnd, IDC_MERGEAMOUNT), SBM_SETRANGE, 0, 100);
			SendMessage(GetDlgItem(hWnd, IDC_MERGEAMOUNT), SBM_SETPOS, 50, TRUE);
			SendMessage(GetDlgItem(hWnd, IDC_MERGEPREVIEW), GWCM_SETPARENTPAINT, 0, TRUE);
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			SetWindowPos(GetDlgItem(hWnd, IDC_MERGEPREVIEW), NULL, ClientRect.right / 2 - 43, ClientRect.bottom / 2 - 33, 86, 66, SWP_NOZORDER);
			//Update the preview bitmaps:
			SmPreviewImage1.ResizeBitmapFromBitmapOps(*SSSOldBitmapPtr);
			SmPreviewImage2.ResizeBitmapFromBitmapOps(*SSSBufferBitmapPtr);
			SmPreviewImage3.FadeBitmaps(SmPreviewImage1, SmPreviewImage2, 0.5);
			SetFocus(GetDlgItem(hWnd, IDC_MERGEAMOUNT));
		}
		break;
	case WM_HSCROLL:
		{
			//The following code is modified from MSDN,
			//it will become obsolete once my graphical
			//horizontal scrollbar is finished.
			int nScrollCode = (int)LOWORD(wParam);
			int nPos = (short int)HIWORD(wParam);
			SCROLLINFO si = {sizeof(SCROLLINFO), SIF_PAGE|SIF_POS|SIF_RANGE|SIF_TRACKPOS, 0, 0, 0, 0, 0};
			GetScrollInfo (GetDlgItem(hWnd, IDC_MERGEAMOUNT), SB_CTL, &si);
			int nNewPos = si.nPos;
			switch (nScrollCode) {
			case SB_LINELEFT:
				nNewPos--;
				break;
			case SB_LINERIGHT:
				nNewPos++;
				break;
			case SB_PAGELEFT:
				nNewPos -= 5;
				break;
			case SB_PAGERIGHT:
				nNewPos += 5;
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				nNewPos = nPos;// + si.nMin; //Required if si.nMin != 0
				break;
			}
			si.fMask = SIF_POS;
			si.nPos = nNewPos;
			SetScrollInfo (GetDlgItem(hWnd, IDC_MERGEAMOUNT), SB_CTL, &si, TRUE);

			if (nNewPos < 0) nNewPos = 0;
			if (nNewPos > 100) nNewPos = 100;
			//Update the labels:
			char tmpNum[4];
			char tmpText[16] = "Image ";
			strcat(tmpText, itoa(100 - nNewPos, tmpNum, 10));
			strcat(tmpText, "%");
			SetWindowText(GetDlgItem(hWnd, IDC_LBLIMAGE), tmpText);
			strcpy(tmpText, itoa(nNewPos, tmpNum, 10));
			strcat(tmpText, "% Buffer");
			SetWindowText(GetDlgItem(hWnd, IDC_LBLBUFFER), tmpText);
			//Update the preview bitmap:
			SmPreviewImage3.FadeBitmaps(SmPreviewImage1, SmPreviewImage2, (double)nNewPos / 100.0);
			//Update the preview window:
			InvalidateRect(GetDlgItem(hWnd, IDC_MERGEPREVIEW), NULL, FALSE);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_MERGEPREVIEW:
			if (HIWORD(wParam) == GWCN_PAINT) {
				PAINTSTRUCT ps;
				HDC hDc = BeginPaint((HWND)lParam, &ps);
				SmPreviewImage3.CopyImageArrayToDC((HWND)lParam, hDc);
				EndPaint((HWND)lParam, &ps);
			}
			break;
		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		case IDOK:
			SSSUndoBitmapPtr->CopyBitmapFromBitmapOps(*SSSOldBitmapPtr);
			MergeBitmapOps(*SSSOldBitmapPtr, *SSSOldBitmapPtr, *SSSBufferBitmapPtr, (double)SendMessage(GetDlgItem(hWnd, IDC_MERGEAMOUNT), SBM_GETPOS, 0, 0) / 100.0);
			EndDialog(hWnd, 0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	}
	return 0;
}


LRESULT CALLBACK AnimDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
	case WM_INITDIALOG:
		{
			if ((*PerlinParams).Animate)
				SendMessage(GetDlgItem(hWnd, IDC_CHKANIMATE), BM_SETCHECK, BST_CHECKED, 0);
			else
				SendMessage(GetDlgItem(hWnd, IDC_CHKANIMATE), BM_SETCHECK, BST_UNCHECKED, 0);
			if ((*PerlinParams).Loop)
				SendMessage(GetDlgItem(hWnd, IDC_CHKLOOP), BM_SETCHECK, BST_CHECKED, 0);
			else
				SendMessage(GetDlgItem(hWnd, IDC_CHKLOOP), BM_SETCHECK, BST_UNCHECKED, 0);
			if ((*PerlinParams).SmoothT)
				SendMessage(GetDlgItem(hWnd, IDC_CHKSMOOTH), BM_SETCHECK, BST_CHECKED, 0);
			else
				SendMessage(GetDlgItem(hWnd, IDC_CHKSMOOTH), BM_SETCHECK, BST_UNCHECKED, 0);
			char tmpText[8];
			ltoa((*PerlinParams).KeyFrames, tmpText, 10);
			SetWindowText(GetDlgItem(hWnd, IDC_KEYFRAMES), tmpText);
			ltoa((*PerlinParams).FramesPerKey, tmpText, 10);
			SetWindowText(GetDlgItem(hWnd, IDC_FRAMESPERKEY), tmpText);
			SetFocus(GetDlgItem(hWnd, IDC_CHKANIMATE));
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		case IDOK:
			{
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKANIMATE), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).Animate = TRUE;
				else
					(*PerlinParams).Animate = FALSE;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKLOOP), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).Loop = TRUE;
				else
					(*PerlinParams).Loop = FALSE;
				if (SendMessage(GetDlgItem(hWnd, IDC_CHKSMOOTH), BM_GETCHECK, 0, 0) == BST_CHECKED)
					(*PerlinParams).SmoothT = TRUE;
				else
					(*PerlinParams).SmoothT = FALSE;
				char tmpText[8];
				GetWindowText(GetDlgItem(hWnd, IDC_KEYFRAMES), tmpText, 8);
				(*PerlinParams).KeyFrames = atol(tmpText);
				GetWindowText(GetDlgItem(hWnd, IDC_FRAMESPERKEY), tmpText, 8);
				(*PerlinParams).FramesPerKey = atol(tmpText);
				if ((*PerlinParams).Animate && !(*PerlinParams).KeyFrames) MessageBox(NULL, "With no key frames, no frames will be produced!", "No key frames", MB_ICONEXCLAMATION);
				else if ((*PerlinParams).Animate && !(*PerlinParams).FramesPerKey) MessageBox(NULL, "With no frames per key frame, no frames will be produced!", "No frames", MB_ICONEXCLAMATION);
				else if ((*PerlinParams).Animate && (*PerlinParams).Loop && (*PerlinParams).KeyFrames < 4) MessageBox(NULL, "At least 4 key frames are required to loop properly", "Not enough key frames for loop", MB_ICONEXCLAMATION);
				EndDialog(hWnd, 0);
			}
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	}
	return 0;
}
