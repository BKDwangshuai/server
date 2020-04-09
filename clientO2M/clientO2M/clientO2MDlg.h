
// clientO2MDlg.h: 头文件
//

#pragma once
#include "MySQLInterface.h"
//UINT WorkerThread(LPVOID pParam);
//UINT ReceiveThread(LPVOID pParam);
class CLookupSocket;
// CclientO2MDlg 对话框
class CclientO2MDlg : public CDialogEx
{
// 构造
public:
	CclientO2MDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENTO2M_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	BOOL			m_bRunning;			//服务器运行状态
	SOCKET			m_sListen;			//监听套结字
	HANDLE			m_hThreadService;	//服务线程句柄

	BOOL	InitServerSocket(void);						//初始化套结字
	BOOL StartServer();								//服务器开始
	BOOL CloseServer();								//服务器结束
	static DWORD WINAPI ServiceThread(void *pParam);//服务线程函数
public:
	bool StartEquip(PEQUIPMENT tempEquip);					//开始设备通讯
	bool CloseEquip(PEQUIPMENT tempEquip);
	BOOL			InitSocket(SOCKET &tempSoc);						//初始化套结字
	static	DWORD WINAPI	WorkerThread(void *pParam);	//工作线程 
	static	DWORD WINAPI	ReceiveThread(void *pParam);	//接收线程 
	WSAOVERLAPPED	*MapEventToOverlapped(int nIndex);		//根据事件句柄序号得到OVERLAPPED指针
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonClose();
	CIPAddressCtrl m_ctlServIP;
	afx_msg void OnBnClickedButtonSend();
	CEdit m_editTest;
	void initMysql();
	afx_msg void OnBnClickedButtonCode();
	//wstring Utf8ToUnicode(const string& str);
	HANDLE			m_hFindThread;					//工作线程句柄
	static	DWORD WINAPI	FindThread(void *pParam);	//查看各设备工作状态线程 
	BOOL serverRun;									//程序运行状态
	int changeId;									//记录最新id
	MySQLInterface mysqlUser;						//MySQL操作
};
