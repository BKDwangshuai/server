
// clientO2M.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号
// CclientO2MApp:
// 有关此类的实现，请参阅 clientO2M.cpp
//

class CclientO2MApp : public CWinApp
{
public:
	CclientO2MApp();
	map<int, PEQUIPMENT> m_equipMap;
	vector<PEQUIPMENT> m_deleteVec;
// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()

	
};

extern CclientO2MApp theApp;
