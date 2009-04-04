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
