
#include "PublicStuff.h"

bool bRunning = true;
bool bDrawing = false;

BitmapOps *SSSOldBitmapPtr = NULL;
BitmapOps *SSSUndoBitmapPtr = NULL;
BitmapOps *SSSBufferBitmapPtr = NULL;

BOOL NoPerlin = FALSE;
BOOL NoFade = FALSE;

bool DoEvents(HWND hWnd = NULL) {
	MSG msg;
	if (!bRunning || !bDrawing) return false;
	while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {bRunning = false; return false;}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (!bRunning || !bDrawing) return false; //In case anything changed...
	return true;
}