// ClientSocket.h: interface for the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTSOCKET_H__5D24D54E_F093_4CEC_BA08_8CDB8DFB6AEA__INCLUDED_)
#define AFX_CLIENTSOCKET_H__5D24D54E_F093_4CEC_BA08_8CDB8DFB6AEA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CclientO2MDlg;

class CClientSocket : public CObject  
{
public:
	CClientSocket(SOCKET s, HWND  hServHwnd);		//构造函数
	virtual ~CClientSocket();						//析构函数

public:
	BOOL	RecvHeader(void);						//接收包头			
	BOOL	RecvPacket(void);						//接收包体
	BOOL	Send(void);								//发送数据
	void	HandleData(void);						//处理数据
	wstring Utf8ToUnicode(const string& str);
	void	Lookup(char* pWord, CString &strMeaning,//查询单词含义
					CString &strMeaning2);		
	//读操作完成例程
	static	void CALLBACK IRoutine(DWORD Error,
									DWORD BytesTransferred,
									LPWSAOVERLAPPED Overlapped,
									DWORD InFlags);
	//写操作完成例程
	static	void CALLBACK ORoutine(DWORD Error, 
									DWORD BytesTransferred,
									LPWSAOVERLAPPED Overlapped,
									DWORD InFlags);
public:
	SOCKET			m_s;						//套接字
	WSAOVERLAPPED	m_iIO;						//in WSAOVERLAPPED结构
	WSAOVERLAPPED	m_oIO;						//out WSAOVERLAPPED结构
	PACKETHDR		m_hdrRecv;					//接收数据包头
	CString			m_strMeaning;				//单词含义
	CString			m_strWord;					//单词
	BOOL			m_bDead;					//该对象是否死亡
	//_RecordsetPtr	m_pRecordSet;				//记录集对象
	HWND			m_hServHwnd;				//主窗口句柄
	bool            m_clientLive;               //是否活着
private:
	CClientSocket();
	
};

#endif // !defined(AFX_CLIENTSOCKET_H__5D24D54E_F093_4CEC_BA08_8CDB8DFB6AEA__INCLUDED_)
