#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"

class Ckey_macroApp : public CWinAppEx
{
public:
	Ckey_macroApp();

	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern Ckey_macroApp theApp;