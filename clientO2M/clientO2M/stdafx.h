
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持


#include <afxsock.h>            // MFC 套接字扩展







#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


/**应用程序定义**/
#include <map>
#include <string>
#include <vector>
using namespace std;
#include "winsock2.h"
#pragma comment(lib, "ws2_32.lib")
#define		SERVERPORT			7788					//服务器端口
#define		MAX_SIZE_BUF		1024						//数据缓冲区长度
#define		THREAD_SLEEP_TIME	100							//线程睡眠时间
#define		MAX_WAIT_EVENTS		WSA_MAXIMUM_WAIT_EVENTS		//最多等待的事件对象数量
#define		HEADERLEN			(sizeof(PACKETHDR))			//包头长度

#define		WM_USER_ADDWORD		(WM_USER + 100)				//添加单词
//服务器与客户端之间数据类型
#define		CONTROL_CE				1					//控制
#define		PRINT_CE					2					//打印数据下发
#define		PRINTGET_CE				3					//打印字节传回
#define		COLLECT_CE				4					//采集数据
#define		COUNTER_CE				5					//计数器
#define		LAB_NAME					6					//lab名
#define		OTHER_CE					10					//其他


#define     SAVETIME					60					//数据一分钟存一次
typedef struct _packethdr
{
	u_short	type;	//类型
	u_short	len;	//数据包长度(包体)
}PACKETHDR, *PPACKETHDR;

class CclientO2MDlg;
typedef struct _threadparam
{
	HWND		hServHwnd;	//主窗口句柄
	CclientO2MDlg *pServView;	//主窗口指针
}THREADPARAM, *PTHREADPARAM;

class CLookupSocket;
typedef struct __threadparamC
{
	CLookupSocket *pClient;
}THECLIENT, *PTHECLIENT;

typedef struct MyStruct
{
	int m_id;									//设备id
	short	m_sServPort;							//端口
	string m_ip;									//ip
	WSAEVENT		m_arrEvent[MAX_WAIT_EVENTS];		//事件对象数组
	DWORD			m_nEventTotal;						//事件总数
	SOCKET			m_sHost;							//主机套接字
	BOOL			m_bRunning;							//客户端运行状态
	HANDLE			m_hWorkerThread;					//工作线程句柄
	HANDLE			m_hReceiveThread;					//工作线程句柄
	CLookupSocket	*m_pLookupSocket;					//CLookupSocket指针
	int			connetTimes;								//断线重连次数
}EQUIPMENT, *PEQUIPMENT;