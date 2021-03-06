/*************************************************************
  Bachelor of Software Engineering
  Media Design School
  Auckland
  New Zealand

  (c) 2018 Media Design School

  File Name   :   main.cpp
  Description :   the main script for my paint tool
  Author      :   Henry Oliver
  Mail        :   henry.oliver@mediadesign.school.nz
*******************************************************************/

//#define WIN32_LEAN_AND_MEAN

#include <windows.h>   // Include all the windows headers.
#include <windowsx.h>  // Include useful macros.
#include <sstream>

#include "resource.h"
#include "shape.h"
#include "line.h"
#include "canvas.h"
#include "rectangle.h"
#include "ellipse.h"
#include "polygon.h"
#include "stamp.h"
#include "backBuffer.h"

#define WINDOW_CLASS_NAME L"WINCLASS1"
typedef DWORD COLORREF;
typedef DWORD* LPCOLORREF;

//Global variables
HINSTANCE g_hInstance;
CCanvas* g_pCanvas;
IShape* g_pShape = 0;
CPolygon* g_pPolygon;
HMENU g_hMenu;

COLORREF currentColor;
COLORREF brushcurrentColor;

bool amDrawing = false;
bool warnuserboutweight = false;
int PenWeight = 1;
int PenStyle = 0;
int BrushStyle = 6;
int checkedwp = 0;
int checkedsp = 0;
int checkedsb = 0;

//Enum to decalre the type of tool supported by the application.
enum ESHAPE
{
	FREEHAND = 0,
	LINESHAPE,
	RECTANGLESHAPE,
	ELLIPSESHAPE,
	POLYGONSHAPE,
	STAMP
};

bool VarianceEqual(int a, int b, int variance)
{
	return std::abs(a - b) <= variance;
}


COLORREF _ChooseColor(HWND const _hWnd, const COLORREF _CurrentColor)
{
	CHOOSECOLOR m_Color;
	ZeroMemory(&m_Color, sizeof(m_Color));
	static COLORREF UserCustomColors[16]{};

	m_Color.lStructSize = sizeof(m_Color);
	m_Color.hwndOwner = _hWnd;
	m_Color.lpCustColors = static_cast<LPDWORD>(UserCustomColors);
	m_Color.rgbResult = _CurrentColor;
	m_Color.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&m_Color))
	{
		return m_Color.rgbResult;
	}
	// else
	return _CurrentColor;
}

void GameLoop()
{
	//One frame of game logic occurs here...
}

LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam)
{
	// This is the main message handler of the system.
	PAINTSTRUCT ps; // Used in WM_PAINT.
	HDC hdc;        // Handle to a device context.
	static POINT _MousePos;
	static POINT _StartPos;
	static POINT _EndPos;

	static ESHAPE currentShape = LINESHAPE;
	

	switch (_msg)
	{
	case WM_CLOSE: {
		int msgboxID = MessageBox(_hwnd, L"Are you sure you want to exit?", L"Goodbye", MB_YESNO | MB_ICONEXCLAMATION);
		switch (msgboxID)
		{
		case IDYES:
			PostQuitMessage(0);
			break;
		case IDNO:
			return 0;
		}
		break;
	}

	case WM_CREATE:
	{
		// Do initialization stuff here.

		g_pCanvas = new CCanvas;
		// Return Success.
		return (0);
	}
	break;

	case WM_LBUTTONDOWN: {
		_StartPos = _MousePos;
		switch (currentShape)
		{
		case LINESHAPE:{
			CLine* ptr = new CLine(PenStyle, PenWeight, currentColor, _StartPos.x, _StartPos.y, _EndPos.x, _EndPos.y);
			g_pShape = ptr;
			amDrawing = true;
			break;
		}
		case RECTANGLESHAPE: {
			CRectangle* ptr = new CRectangle(BrushStyle, 3, brushcurrentColor, PenStyle, currentColor, _StartPos.x, _StartPos.y, PenWeight);
			g_pShape = ptr;
			g_pShape->SetStartX(_StartPos.x);
			g_pShape->SetStartY(_StartPos.y);
			amDrawing = true;
			break;
		}

		case ELLIPSESHAPE: {
			CEllipse* ptr = new CEllipse(currentColor,brushcurrentColor, _StartPos.x, _StartPos.y, PenWeight, PenStyle, BrushStyle);
			g_pShape = ptr;
			g_pShape->SetStartX(_StartPos.x);
			g_pShape->SetStartY(_StartPos.y);
			amDrawing = true;
			break;
		}
		case POLYGONSHAPE: {
			if (g_pShape == nullptr) // NEW
			{
				CPolygon* ptr = new CPolygon(BrushStyle, brushcurrentColor, PenStyle, currentColor, PenWeight);
				g_pShape = ptr;
				g_pShape->SetStartX(_StartPos.x);
				g_pShape->SetStartY(_StartPos.y);
				amDrawing = true;
			}
			break;
		}
		
		case STAMP:{
			break;
		}
		default:
			break;
		}
		

		InvalidateRect(_hwnd, nullptr, true);

		break;
	}

	case WM_LBUTTONUP: {
		_EndPos = _MousePos;
		if (g_pShape != nullptr) {
			switch (currentShape)
			{
			case LINESHAPE: {
				g_pShape->SetEndX(_EndPos.x);
				g_pShape->SetEndY(_EndPos.y);
				g_pCanvas->AddShape(g_pShape);
				g_pShape = nullptr;
				break;
			}

			case RECTANGLESHAPE: {
				g_pShape->SetEndX(_EndPos.x);
				g_pShape->SetEndY(_EndPos.y);
				//g_pShape->SetColor(currentColor);
				g_pCanvas->AddShape(g_pShape);
				g_pShape = nullptr;
				break;
			}
			case ELLIPSESHAPE: {
				g_pShape->SetEndX(_EndPos.x);
				g_pShape->SetEndY(_EndPos.y);
				g_pCanvas->AddShape(g_pShape);
				g_pShape = nullptr;
				break;
			}
			case POLYGONSHAPE: {

				// NEW
				CPolygon* ptr = dynamic_cast<CPolygon*>(g_pShape);
				ptr->AddPoint(_EndPos);

				if (ptr->GetSize() > 1 &&
					VarianceEqual(_EndPos.x, ptr->GetStartX(), 5) &&
					VarianceEqual(_EndPos.y, ptr->GetStartY(), 5))
				{
					// finish editing shape
					g_pCanvas->AddShape(g_pShape);
					g_pShape = nullptr;
				}

				break;

			}
			case STAMP: {
				break;
			}
			default:
				break;
			}
		}


		InvalidateRect(_hwnd, nullptr, true);
		break;
	}

	case WM_MOUSEMOVE:
	{
		_MousePos = { GET_X_LPARAM(_lparam),  GET_Y_LPARAM(_lparam) };
		//InvalidateRect(_hwnd, nullptr, true);
		return 0;

		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(_hwnd, &ps);

		if (currentShape == POLYGONSHAPE && g_pShape != nullptr)
		{
			g_pCanvas->AddShape(g_pShape);
			g_pCanvas->Draw(hdc);
			g_pCanvas->PopShape();
		}
		else
		{
			g_pCanvas->Draw(hdc);
		}

		EndPaint(_hwnd, &ps);

		return (0);
	}
	break;
	case WM_COMMAND:
	{
		switch (LOWORD(_wparam))
		{

		case ID_WIDTH_1: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_1, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_ENABLED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_ENABLED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_ENABLED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_ENABLED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_ENABLED);
			checkedwp = ID_WIDTH_1;
			PenWeight = 1;
			break;
		}
		case ID_WIDTH_2: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_2, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_2;
			PenWeight = 2;
			break;
		}
		case ID_WIDTH_3: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_3, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_3;
			PenWeight = 3;
			break;
		}
		case ID_WIDTH_4: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_4, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_4;
			PenWeight = 4;
			break;
		}
		case ID_WIDTH_5: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_5, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_5;
			PenWeight = 5;
			break;
		}
		case ID_WIDTH_6: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_6, MF_CHECKED);
			checkedwp = ID_WIDTH_6;
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			PenWeight = 6;
			break;
		}
		case ID_WIDTH_7: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_7, MF_CHECKED);
			checkedwp = ID_WIDTH_7;
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			PenWeight = 7;
			break;
		}
		case ID_WIDTH_8: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_8, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_8;
			PenWeight = 8;
			break;
		}
		case ID_WIDTH_9: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_9, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_9;
			PenWeight = 9;
			break;
		}
		case ID_WIDTH_10: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_10, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_10;
			PenWeight = 10;
			break;
		}
		case ID_WIDTH_11: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_11, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_11;
			PenWeight = 11;
			break;
		}
		case ID_WIDTH_12: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_12, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_12;
			PenWeight = 12;
			break;
		}
		case ID_WIDTH_13: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_13, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_13;
			PenWeight = 13;
			break;
		}
		case ID_WIDTH_14: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_14, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_14;
			PenWeight = 14;
			break;
		}
		case ID_WIDTH_15: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_15, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_15;
			PenWeight = 15;
			break;
		}
		case ID_WIDTH_16: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_16, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_16;
			PenWeight = 16;
			break;
		}
		case ID_WIDTH_17: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_17, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_17;
			PenWeight = 17;
			break;
		}
		case ID_WIDTH_18: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_18, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_18;
			PenWeight = 18;
			break;
		}
		case ID_WIDTH_19: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_19, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_19;
			PenWeight = 19;
			break;
		}
		case ID_WIDTH_20: {
			CheckMenuItem(g_hMenu, checkedwp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_20, MF_CHECKED);
			EnableMenuItem(g_hMenu, ID_STYLE_SOLID, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_GRAYED);
			EnableMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_GRAYED);
			if (warnuserboutweight == false) {
				MessageBox(_hwnd, L"WARNING LINE STYLING DOESN'T WORK WITH LINE WIDTHS GREATER THAN 1", L"WARNING", MB_OK | MB_ICONWARNING);
				warnuserboutweight = true;
			}
			checkedwp = ID_WIDTH_20;
			PenWeight = 20;
			break;
		}

		case ID_STYLE_SOLID:{
			CheckMenuItem(g_hMenu, checkedsp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_SOLID, MF_UNCHECKED);
			checkedsp = ID_STYLE_DASHDOTDOT;
			PenStyle = 0;
			break;
		}
		case ID_STYLE_DASHED: {
			CheckMenuItem(g_hMenu, checkedsp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_DASHED, MF_CHECKED);
			checkedsp = ID_STYLE_DASHED;
			PenStyle = 1;
			break;
		}
		case ID_STYLE_DOTTED: {
			CheckMenuItem(g_hMenu, checkedsp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_DOTTED, MF_CHECKED);
			checkedsp = ID_STYLE_DOTTED;
			PenStyle = 2;
			break;
		}
		case ID_STYLE_DASHDOT: {
			CheckMenuItem(g_hMenu, checkedsp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_DASHDOT, MF_CHECKED);
			checkedsp = ID_STYLE_DASHDOT;
			PenStyle = 3;
			break;
		}
		case ID_STYLE_DASHDOTDOT: {
			CheckMenuItem(g_hMenu, checkedsp, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_DASHDOTDOT, MF_CHECKED);
			checkedsp = ID_STYLE_DASHDOTDOT;
			PenStyle = 4;
			break;
		}


		case ID_STYLE_SOLIDBRUSH: {
			CheckMenuItem(g_hMenu, checkedsb, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_SOLIDBRUSH, MF_CHECKED);
			checkedsb = ID_STYLE_SOLIDBRUSH;
			BrushStyle = 6; //look at that cheap hack
			break;
		}
		case ID_STYLE_HORIZONTAL: {
			CheckMenuItem(g_hMenu, checkedsb, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_HORIZONTAL, MF_CHECKED);
			checkedsb = ID_STYLE_HORIZONTAL;
			BrushStyle = HS_HORIZONTAL;
			break;
		}
		case ID_STYLE_VERTICAL: {
			CheckMenuItem(g_hMenu, checkedsb, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_VERTICAL, MF_CHECKED);
			checkedsb = ID_STYLE_VERTICAL;
			BrushStyle = HS_VERTICAL;
			break;
		}
		case ID_STYLE_DIAGONAL: {
			CheckMenuItem(g_hMenu, checkedsb, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_DIAGONAL, MF_CHECKED);
			checkedsb = ID_STYLE_DIAGONAL;
			BrushStyle = HS_FDIAGONAL;
			break;
		}
		case ID_STYLE_BDIAGONAL: {
			CheckMenuItem(g_hMenu, checkedsb, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_BDIAGONAL, MF_CHECKED);
			checkedsb = ID_STYLE_BDIAGONAL;
			BrushStyle = HS_BDIAGONAL;
			break;
		}
		case ID_STYLE_CROSS: {
			CheckMenuItem(g_hMenu, checkedsb, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_CROSS, MF_CHECKED);
			checkedsb = ID_STYLE_CROSS;
			BrushStyle = HS_CROSS;
			break;
		}
		case ID_STYLE_DIAGCROSS: {
			CheckMenuItem(g_hMenu, checkedsb, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_STYLE_DIAGCROSS, MF_CHECKED);
			checkedsb = ID_STYLE_DIAGCROSS;
			BrushStyle = HS_DIAGCROSS;
			break;
		}

		case ID_FILE_OPEN:
		{
			MessageBox(_hwnd, L"[INSERT FILE OPEN FEATURE HERE]", L"PLACEHOLDER", MB_OK | MB_ICONWARNING);
			break;
		}
		case ID_FILE_SAVE:
		{
			MessageBox(_hwnd, L"[INSERT FILE SAVE FEATURE HERE]", L"PLACEHOLDER", MB_OK | MB_ICONWARNING);
			break;
		} 
		case ID_FILE_EXIT:
		{
			int msgboxID = MessageBox(_hwnd, L"Are you sure you want to exit?", L"Goodbye", MB_YESNO | MB_ICONEXCLAMATION);
			switch (msgboxID)
			{
			case IDYES:
				PostQuitMessage(0);
				break;
			case IDNO:
				//do nothing
				break;
			}
			break;
		}

		case ID_SHAPE_LINE: {
			if (currentShape == POLYGONSHAPE && g_pShape != nullptr)
			{
				g_pCanvas->AddShape(g_pShape);
				g_pShape = nullptr;
			}
			currentShape = LINESHAPE;
			CheckMenuItem(g_hMenu, ID_SHAPE_POLYGON, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_ELLIPSE, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_R, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_LINE, MF_CHECKED);
			break;
		}

		case ID_SHAPE_R: {
			if (currentShape == POLYGONSHAPE && g_pShape != nullptr)
			{
				g_pCanvas->AddShape(g_pShape);
				g_pShape = nullptr;
			}
			currentShape = RECTANGLESHAPE;
			CheckMenuItem(g_hMenu, ID_SHAPE_POLYGON, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_ELLIPSE, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_R, MF_CHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_LINE, MF_UNCHECKED);
			break;
		}


		case ID_SHAPE_ELLIPSE: {
			if (currentShape == POLYGONSHAPE && g_pShape != nullptr)
			{
				g_pCanvas->AddShape(g_pShape);
				g_pShape = nullptr;
			}
			currentShape = ELLIPSESHAPE;
			CheckMenuItem(g_hMenu, ID_SHAPE_POLYGON, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_ELLIPSE, MF_CHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_R, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_LINE, MF_UNCHECKED);
			break;
		}

		case ID_SHAPE_POLYGON: {
			if (currentShape == POLYGONSHAPE && g_pShape != nullptr)
			{
				g_pCanvas->AddShape(g_pShape);
				g_pShape = nullptr;
			}
			currentShape = POLYGONSHAPE;
			CheckMenuItem(g_hMenu, ID_SHAPE_POLYGON, MF_CHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_ELLIPSE, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_R, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_SHAPE_LINE, MF_UNCHECKED);
			break;
		}

		case ID_PEN_WIDTH: {
			CheckMenuItem(g_hMenu, ID_WIDTH_1, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_2, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_3, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_4, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_5, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_6, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_7, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_8, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_9, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_10, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_11, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_12, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_13, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_14, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_15, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_16, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_17, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_18, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_19, MF_UNCHECKED);
			CheckMenuItem(g_hMenu, ID_WIDTH_20, MF_UNCHECKED);

			break;
		}

		case ID_PEN_COLOR:{
			currentColor = _ChooseColor(_hwnd, currentColor);
			break;
		}

		case ID_PEN_STYLE: {
			MessageBox(_hwnd, L"This is pen, AND HE IS STYLISH", L"Author Information", MB_OK | MB_ICONEXCLAMATION);
			break;
		}

		case ID_BRUSH_COLOR: {
			brushcurrentColor = _ChooseColor(_hwnd, brushcurrentColor);
			break;
		}

		case ID_BRUSH_STYLE: {
			MessageBox(_hwnd, L"This is brush, AND HE IS STYLISH", L"Author Information", MB_OK | MB_ICONEXCLAMATION);
			break;
		}

		case ID_ADD_STAMP: {
			MessageBox(_hwnd, L"This is stamp, AND HE IS A COLLECTOR", L"Author Information", MB_OK | MB_ICONEXCLAMATION);
			break;
		}

		case ID_HELP_ABOUT:
		{
			MessageBox(_hwnd, L"This paint tool was developed by Henry Oliver", L"Author Information", MB_OK | MB_ICONINFORMATION);
			break;
		}
		default:
			break;
		}
		return(0);
	}
	break;
	case WM_DESTROY:
	{
		// Kill the application, this sends a WM_QUIT message.
		PostQuitMessage(0);

		// Return success.
		return (0);
	}
	break;

		default:break;
	} // End switch.

	  // Process any messages that we did not take care of...

	return (DefWindowProc(_hwnd, _msg, _wparam, _lparam));
}


int WINAPI WinMain(HINSTANCE _hInstance,
	HINSTANCE _hPrevInstance,
	LPSTR _lpCmdLine,
	int _nCmdShow)
{
	WNDCLASSEX winclass; // This will hold the class we create.
	HWND hwnd;           // Generic window handle.
	MSG msg;             // Generic message.

	g_hInstance = _hInstance;
	// First fill in the window class structure.
	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = _hInstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// register the window class
	if (!RegisterClassEx(&winclass))
	{
		return (0);
	}

	g_hMenu = LoadMenu(_hInstance, MAKEINTRESOURCE(IDR_MENU1));
	// create the window
	hwnd = CreateWindowEx(NULL, // Extended style.
		WINDOW_CLASS_NAME,      // Class.
		L"Henry's Paint Tool",   // Title.
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		100, 100,                    // Initial x,y.
		1500, 800,                // Initial width, height.
		NULL,                   // Handle to parent.
		g_hMenu,                   // Handle to menu.
		_hInstance,             // Instance of this application.
		NULL);                  // Extra creation parameters.

	if (!(hwnd))
	{
		return (0);
	}



	// Enter main event loop
	while (true)
	{
		// Test if there is a message in queue, if so get it.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Test if this is a quit.
			if (msg.message == WM_QUIT)
			{
				break;
			}

			// Translate any accelerator keys.
			TranslateMessage(&msg);
			// Send the message to the window proc.
			DispatchMessage(&msg);
		}

		// Main game processing goes here.
		GameLoop(); //One frame of game logic occurs here...
	}

	// Return to Windows like this...
	
	return (static_cast<int>(msg.wParam));
}
