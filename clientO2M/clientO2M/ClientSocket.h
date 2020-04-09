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
	CClientSocket(SOCKET s, HWND  hServHwnd);		//���캯��
	virtual ~CClientSocket();						//��������

public:
	BOOL	RecvHeader(void);						//���հ�ͷ			
	BOOL	RecvPacket(void);						//���հ���
	BOOL	Send(void);								//��������
	void	HandleData(void);						//��������
	wstring Utf8ToUnicode(const string& str);
	void	Lookup(char* pWord, CString &strMeaning,//��ѯ���ʺ���
					CString &strMeaning2);		
	//�������������
	static	void CALLBACK IRoutine(DWORD Error,
									DWORD BytesTransferred,
									LPWSAOVERLAPPED Overlapped,
									DWORD InFlags);
	//д�����������
	static	void CALLBACK ORoutine(DWORD Error, 
									DWORD BytesTransferred,
									LPWSAOVERLAPPED Overlapped,
									DWORD InFlags);
public:
	SOCKET			m_s;						//�׽���
	WSAOVERLAPPED	m_iIO;						//in WSAOVERLAPPED�ṹ
	WSAOVERLAPPED	m_oIO;						//out WSAOVERLAPPED�ṹ
	PACKETHDR		m_hdrRecv;					//�������ݰ�ͷ
	CString			m_strMeaning;				//���ʺ���
	CString			m_strWord;					//����
	BOOL			m_bDead;					//�ö����Ƿ�����
	//_RecordsetPtr	m_pRecordSet;				//��¼������
	HWND			m_hServHwnd;				//�����ھ��
	bool            m_clientLive;               //�Ƿ����
private:
	CClientSocket();
	
};

#endif // !defined(AFX_CLIENTSOCKET_H__5D24D54E_F093_4CEC_BA08_8CDB8DFB6AEA__INCLUDED_)
