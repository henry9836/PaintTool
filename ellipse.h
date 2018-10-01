/*******************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand

(c) 2018 Media Design School

File Name   :   ellipse.h
Description :   this holds important infomation for the ellipse
Author      :   Henry Oliver
Mail        :   henry.oliver@mediadesign.school.nz
***********************************************************************/


#ifndef __ELLIPSE_H__
#define __ELLIPSE_H__

//	Library Includes
#include <windows.h>
#include <windowsx.h>

#include "shape.h"

class CEllipse : public IShape
{
public:
	CEllipse(COLORREF _newColor, int, int);
	CEllipse();
	virtual ~CEllipse();

	virtual void Draw(HDC _hdc);
	void SetColor(COLORREF _newColor);


private:
	COLORREF m_Color;
};

#endif