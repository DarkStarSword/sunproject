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
#ifndef __MULTISOFTGCONTROLS_H__
#define __MULTISOFTGCONTROLS_H__

#include <windows.h>


//Call this before attempting to load a dialogue with GControls:
void MultiSoftGControlsInit(HINSTANCE);
//Call this when you no longer need the GControls:
void MultiSoftGControlsDeInit();

/*
Window Classes:
MultiSoftGButton
MultiSoftGTitle
MultiSoftGWndCon
MultiSoftVScroll
MultiSoftHScroll
MultiSoftHProgress

NEVER CREATE A WINDOW OF THIS WINDOW CLASS, ONLY ONE SHOULD EVER EXIST:
MultiSoftGToolTip
*/

//Call this so you can call the tool tip messages:
HWND GetMultiSoftGToolTipHWnd();
//Only use the API's on this if you apsolutely need to for some reason...

//You can use this for transparent blitting:
void TransBlt (HDC hDc, HBITMAP hBitmap, LONG32 DestX, LONG32 DestY, COLORREF TransColour);

//Window messages:
//MultiSoftGButton:
//lParam: COLORREF
#define GBM_SETFACECOLOUR				WM_USER
#define GBM_SETDARKSHADOWCOLOUR			(WM_USER + 1)
#define GBM_SETSHADOWCOLOUR				(WM_USER + 2)
#define GBM_SETEXTRAHIGHLIGHTCOLOUR		(WM_USER + 3)
#define GBM_SETHIGHLIGHTCOLOUR			(WM_USER + 4)
#define GBM_SETTEXTCOLOUR				(WM_USER + 5)
#define GBM_SETACTIVECOLOUR				(WM_USER + 6)
#define GBM_SETDEFAULTCOLOUR			(WM_USER + 7)
//lParam: BOOL
#define GBM_SETTOOLTIPSTATE				(WM_USER + 8) //Default: FALSE
//lParam: ResourceID
#define GBM_SETOFFIMAGE					(WM_USER + 9)
#define GBM_SETOVERIMAGE				(WM_USER + 10)
#define GBM_SETONIMAGE					(WM_USER + 11)
#define GBM_SETDISABLEDIMAGE			(WM_USER + 12)
//wParam: Default x size (If no off image exist, 64 suggested)
//lParam: Default y size (If no off image exist, 24 suggested)
#define GBM_AUTOSIZE					(WM_USER + 13)
				/***** UNSUPPORTED *****/
//lParam: BOOL (Auto unload image state If set to false, I will NOT unload the images!)
/*#define GBM_SETOFFIMAGEAUTOUNLOAD		(WM_USER + 14)
#define GBM_SETOVERIMAGEAUTOUNLOAD		(WM_USER + 15)
#define GBM_SETONIMAGEAUTOUNLOAD		(WM_USER + 16)
#define GBM_SETDIMAGEAUTOUNLOAD			(WM_USER + 17)*/
				/***** UNSUPPORTED *****/

//MultiSoftGTitle:
//lParam: COLORREF
#define GTM_SETFACECOLOUR				WM_USER
#define GTM_SETDARKSHADOWCOLOUR			(WM_USER + 1)
#define GTM_SETSHADOWCOLOUR				(WM_USER + 2)
#define GTM_SETEXTRAHIGHLIGHTCOLOUR		(WM_USER + 3)
#define GTM_SETHIGHLIGHTCOLOUR			(WM_USER + 4)
#define GTM_SETTEXTCOLOUR				(WM_USER + 5)
//lParam: DWORD
#define GTM_SETHEIGHT					(WM_USER + 6)
#define GTM_SETMINWIDTH					(WM_USER + 7)
//Return: (HWND)MultiSoftGButton handle, Send messages as above
#define GTM_GETCLOSEHWND				(WM_USER + 8)
#define GTM_GETMAXHWND					(WM_USER + 9)
#define GTM_GETRESTOREHWND				(WM_USER + 10)
#define GTM_GETMINHWND					(WM_USER + 11)
//lParam: ResourceID
#define GTM_SETBARIMAGE					(WM_USER + 12)
//Pass lParam and wParam from windows:
#define GTM_NCPARENTPAINT				(WM_USER + 13)
//#define GTM_NCCALCPARENTSIZE			(WM_USER + 14)

//MultiSoftGToolTip:
//lParam: LPSTR[128]
//LOWORD(wParam): x
//HIWORD(wParam): y
#define GTTM_SHOWTIP					WM_USER
#define GTTM_HIDETIP					(WM_USER + 1)
//Align Options (Combine X and Y with OR(|)):
#define GTT_XALIGNLEFT					0
#define GTT_XALIGNCENTER				1
#define GTT_XALIGNRIGHT					2
#define GTT_YALIGNTOP					0
#define GTT_YALIGNCENTER				4
#define GTT_YALIGNBOTTOM				8
//lParam: LPRECT
//wParam: Position Align options
#define GTTM_SETHOTSPOT					(WM_USER + 2)
//lParam: ResourceID
#define GTTM_SETIMAGE					(WM_USER + 3)
#define GTTM_SETLEFTIMAGE				(WM_USER + 4)
#define GTTM_SETRIGHTIMAGE				(WM_USER + 5)
//lParam: COLORREF
#define GTTM_SETTEXTCOLOUR				(WM_USER + 6)

//MultiSoftGWndCon:
//lParam: COLORREF
#define GWCM_SETFACECOLOUR				WM_USER
#define GWCM_SETDARKSHADOWCOLOUR		(WM_USER + 1)
#define GWCM_SETSHADOWCOLOUR			(WM_USER + 2)
#define GWCM_SETEXTRAHIGHLIGHTCOLOUR	(WM_USER + 3)
#define GWCM_SETHIGHLIGHTCOLOUR			(WM_USER + 4)
#define GWCM_SETBACKCOLOUR				(WM_USER + 5)
//lParam: BOOL
#define GWCM_SETPARENTPAINT				(WM_USER + 6) //Default: FALSE
//LOWORD(lParam): Frames
//HIWORD(lParam): Frames per Second
//LOWORD(wParam): New x
//HIWORD(wParam): New y
#define GWCM_MOVETOCOSINE				(WM_USER + 7)

//MultiSoftVScroll & MultiSoftHScroll:
//lParam: Value (Automatically checks against range)
#define GSM_SETVALUE					WM_USER
//Return: (LONG32)Value
#define GSM_GETVALUE					(WM_USER + 1)
//lParam: New range (Min can be more than max - It reverses the scrollbar)
#define GSM_SETMIN						(WM_USER + 2)
#define GSM_SETMAX						(WM_USER + 3)
#define GSM_SETSMALLUPDATE				(WM_USER + 4)
#define GSM_SETLARGEUPDATE				(WM_USER + 5)
//lParam: New size
#define GSM_SETUPLEFTSIZE				(WM_USER + 6)
#define GSM_SETDOWNRIGHTSIZE			(WM_USER + 7)
//Return: (HWND)MultiSoftGButton handle, Send messages as above
#define GSM_GETUPLEFTHWND				(WM_USER + 8)
#define GSM_GETDOWNRIGHTHWND			(WM_USER + 9)
//lParam: COLORREF
#define GSM_SETFACECOLOUR				(WM_USER + 10)
#define GSM_SETDARKSHADOWCOLOUR			(WM_USER + 11)
#define GSM_SETSHADOWCOLOUR				(WM_USER + 12)
#define GSM_SETEXTRAHIGHLIGHTCOLOUR		(WM_USER + 13)
#define GSM_SETHIGHLIGHTCOLOUR			(WM_USER + 14)
#define GSM_SETARROWCOLOUR				(WM_USER + 15)
#define GSM_SETTRACKCOLOUR				(WM_USER + 16)
#define GSM_SETTRACKHIGHLIGHTCOLOUR		(WM_USER + 17)//Unsupported
//lParam: ResourceID
#define GSM_SETTRACKIMAGE				(WM_USER + 18)
#define GSM_SETTRACKHIGHLIGHTIMAGE		(WM_USER + 19)//Unsupported
#define GSM_SETBARTOPIMAGE				(WM_USER + 20)
#define GSM_SETBARIMAGE					(WM_USER + 21)
#define GSM_SETBARBOTTOMIMAGE			(WM_USER + 22)
//Return: (LONG32)Value (Use when GSN_BARDRAG is recieved, otherwise returns current value)
#define GSM_GETDRAGVALUE				(WM_USER + 23)

//MultiSoftHProgress & MultiSoftVProgress:
//lParam: Value (Automatically checks against range)
#define GPM_SETVALUE1					WM_USER
#define GPM_SETVALUE2					(WM_USER + 1)
#define GPM_SETVALUE3					(WM_USER + 2)
#define GPM_SETVALUE4					(WM_USER + 3)
//lParam: New range (Min can be more than max - It reverses the progressbar)
#define GPM_SETMIN						(WM_USER + 4)
#define GPM_SETMAX						(WM_USER + 5)
//lParam: ResourceID
#define GPM_SETBARIMAGE0				(WM_USER + 6) //Background image
#define GPM_SETBARIMAGE1				(WM_USER + 7)
#define GPM_SETBARIMAGE2				(WM_USER + 8)
#define GPM_SETBARIMAGE3				(WM_USER + 9)
#define GPM_SETBARIMAGE4				(WM_USER + 10)
//lParam: COLORREF
//(| with 0xff000000 to enable bitmap transparency colour,
//& with 0xffffff to disable it. If there is no bitmap loaded,
//these colours specify the flat bar colour.)
#define GPM_SETCOLOUR0					(WM_USER + 11) //Cannot be transparent!
#define GPM_SETCOLOUR1					(WM_USER + 12)
#define GPM_SETCOLOUR2					(WM_USER + 13)
#define GPM_SETCOLOUR3					(WM_USER + 14)
#define GPM_SETCOLOUR4					(WM_USER + 15)


//Notifications:
//MultiSoftGButton:
#define GBN_CLICKED						WM_USER
#define GBN_LBUTTONDOWN					(WM_USER + 1)
#define GBN_LBUTTONUP					(WM_USER + 2)
#define GBN_ENABLECHANGED				(WM_USER + 3)
//#define GBN_ENABLESTATE
//MultiSoftGWndCon:
#define GWCN_PAINT						WM_USER //GWCM_SETPARENTPAINT must be set
#define GWCN_CLICKED					(WM_USER + 1)
#define GWCN_LBUTTONDOWN				(WM_USER + 2)
#define GWCN_LBUTTONUP					(WM_USER + 3)
#define GWCN_RBUTTONDOWN				(WM_USER + 4)
#define GWCN_RBUTTONUP					(WM_USER + 5)
#define GWCN_MBUTTONDOWN				(WM_USER + 6)
#define GWCN_MBUTTONUP					(WM_USER + 7)
#define GWCN_MOUSEMOVE					(WM_USER + 8)
//MultiSoftVScroll & MultiSoftHScroll:
#define GSN_SCROLL						WM_USER
#define GSN_BARDRAG						(WM_USER + 1)

#endif
