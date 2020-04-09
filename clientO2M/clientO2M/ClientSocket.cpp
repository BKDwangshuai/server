// ClientSocket.cpp: implementation of the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ClientSocket.h"
#include "clientO2M.h"
#include "clientO2MDlg.h"
#include "LookupSocket.h"
#include <iostream>
using namespace std;
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//extern CtestmfcOVERApp theApp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClientSocket::CClientSocket(SOCKET s, HWND  hServHwnd)
:m_bDead(FALSE)
{
	m_s = s;									//�׽���
	ZeroMemory(&m_iIO, sizeof(m_iIO));			//�ص��ṹ
	ZeroMemory(&m_oIO, sizeof(m_oIO));			//�ص��ṹ
	//m_pRecordSet.CreateInstance(__uuidof(Recordset));//������¼��ָ��
	m_hServHwnd = hServHwnd;

}

CClientSocket::~CClientSocket()
{
	closesocket(m_s);
}

/*
 * ��������
 */
BOOL CClientSocket::Send(void)
{
	DWORD	dwBytesRecved;				//�����ֽ���
	DWORD	dwFlags = 0;				//��־
	
	ZeroMemory(&m_oIO, sizeof(m_oIO));	//�ص��ṹ
	m_oIO.hEvent = WSAEVENT(this);
	//test
	//if (m_strMeaning.GetLength()>100)
	//{
	//	m_strMeaning=_T("hello");
	//}
	/*m_strMeaning=_T("A3");*/
	string teststr="asdfg";
	WSABUF wsaSendBuf[2];				//WSABUF�ṹ����
	//�������ݰ�ͷ
	PACKETHDR hdr;
	hdr.type = OTHER_CE;						//���ݰ�����
	hdr.len = m_strMeaning.GetLength()*2;		//���ݵĳ���

	wsaSendBuf[0].buf = (char*)&(hdr);		//�������ݻ���������ͷ������
	wsaSendBuf[0].len = HEADERLEN;			//�������ĳ��ȣ� ��ͷ����������

	//wsaSendBuf[1].buf = const_cast<char*>(teststr.c_str());	//�������ݰ���
	//wsaSendBuf[1].len = hdr.len;		//���ݳ���
	wsaSendBuf[1].buf = (char*)(LPCTSTR)m_strMeaning;	//�������ݰ���
	wsaSendBuf[1].len = m_strMeaning.GetLength()*2;		//���ݳ���
	//�첽�������ݣ�ORoutineΪ�������
	if(SOCKET_ERROR == WSASend(m_s, wsaSendBuf, 2, 
		&dwBytesRecved, dwFlags, &m_oIO, ORoutine))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * ��������
 */
void CClientSocket::HandleData( void )
{
	if (m_hdrRecv.len != 0)					//���հ�ͷ���
	{
		RecvPacket();						//�������ݰ���
		if (m_hdrRecv.type == LAB_NAME)
		{
			CString eid, filePath;
			AfxExtractSubString(eid, (LPCTSTR)m_strWord, 0, '|'); // �õ� abcd
			AfxExtractSubString(filePath, (LPCTSTR)m_strWord, 1, '|'); // �õ� abcd
			auto findIter = theApp.m_equipMap.find(_ttoi(eid));
			if (findIter != theApp.m_equipMap.end())
			{
				(findIter->second->m_pLookupSocket)->SendLab(filePath);
			}
		}
		else
		{
		}
		ZeroMemory(&m_hdrRecv, HEADERLEN);	//��ͷ��0
	}else									//�������������
	{
		//ASSERT(m_hdrRecv.len == 0 && m_hdrRecv.type == ETOC);		


		
		//Ͷ����һ���첽IO
		RecvHeader();
	}
}

/*
 * �������������
 */
void CALLBACK CClientSocket::IRoutine( DWORD Error,  DWORD BytesTransferred,
									  LPWSAOVERLAPPED Overlapped,   DWORD InFlags )
{
	CClientSocket *pClient = NULL;
	pClient = (CClientSocket*)Overlapped->hEvent;	//�ͻ���ָ��
	//��������
	if (Error != 0 || BytesTransferred == 0)		//������
	{
		pClient->m_bDead = TRUE;					//��������ɾ��
		pClient->m_clientLive=false;
		return;
	}
	pClient->HandleData();							//��������
}

/*
 * д�����������
 */
void CALLBACK CClientSocket::ORoutine( DWORD Error,  DWORD BytesTransferred,   
									  LPWSAOVERLAPPED Overlapped,   DWORD InFlags )
{
	CClientSocket *pClient = NULL;					//�ͻ���ָ��
	pClient = (CClientSocket*)Overlapped->hEvent;
	if (Error != 0 || BytesTransferred == 0)		//������
	{
		pClient->m_bDead = TRUE;					//ɾ���ÿͻ���
		pClient->m_clientLive=false;
	}
}

/*
 * �������ݰ�ͷ
 */
BOOL CClientSocket::RecvHeader( void )
{
	DWORD	dwBytesRecved;				//�����ֽ���
	DWORD	dwFlags = 0;				//��־	
		
	ZeroMemory(&m_iIO, sizeof(m_iIO));	//�ص��ṹ
	m_iIO.hEvent = WSAEVENT(this);

	ZeroMemory(&m_hdrRecv, HEADERLEN);	//��ͷ��0

	WSABUF wsaRecv;
	wsaRecv.buf = (char*)&m_hdrRecv;
	wsaRecv.len = HEADERLEN;
	//���հ�ͷ
	if(SOCKET_ERROR == WSARecv(m_s, &wsaRecv, 1, 
		&dwBytesRecved, &dwFlags, &m_iIO, IRoutine))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)//�첽����
		{
			return FALSE;
		}
		m_clientLive=true;
	}
	return TRUE;
}

/*
 * �������ݰ���
 */
BOOL CClientSocket::RecvPacket( void )
{
	DWORD	dwBytesRecved;				//�����ֽ���
	DWORD	dwFlags = 0;				//��־
			
	ZeroMemory(&m_iIO, sizeof(m_iIO));	//�ص��ṹ
	m_iIO.hEvent = WSAEVENT(this);		//thisָ��

	WSABUF wsaRecv;						//�������ݻ�����
	CHAR *myBuff = new CHAR[m_hdrRecv.len];
	wsaRecv.buf = myBuff;
	wsaRecv.len = m_hdrRecv.len;
	//�첽��������
	if(SOCKET_ERROR == WSARecv(m_s, &wsaRecv, 1, 
		&dwBytesRecved, &dwFlags, &m_iIO, IRoutine))
	{
		int err = WSAGetLastError();
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			return FALSE;
		}
	}
	m_strWord = Utf8ToUnicode(myBuff).c_str();
	delete[]myBuff;
	return TRUE;	
}

/*
 * ��ѯ����
 */
//void CClientSocket::Lookup( char* pWord, CString &strMeaning1, CString &strMeaning2)
//{
//	BOOL bFinder = TRUE;								//�ҵ��õ���
//
//	CString strSQL = "select * from EToC where word = ";//SQL���
//	strSQL += "'";
//	CString strTemp = pWord;
//	strTemp  += "'";
//	strSQL += strTemp;			//select * from EToC where word = '***'
//
//	try
//	{
//		//��ѯ���ʵĺ���
//		m_pRecordSet->Open(_variant_t(strSQL), theApp.m_pConnection.GetInterfacePtr(),
//			adOpenStatic  , 
//			adLockOptimistic,
//			adCmdText);
//
//		int n = m_pRecordSet->GetRecordCount();	//�ж��Ƿ��ҵ��õ���
//		if (0 == n)								//�õ���δ���������ݿ���
//		{
//				strMeaning1 = "�ڴʵ���û�в��ҵ��õ��ʣ�";
//				bFinder = FALSE;
//		}else									//�õ��ʴ��������ݿ���
//		{	
//			//����1
//			_variant_t var;
//			var = m_pRecordSet->GetCollect("meaning1");			
//			if (var.vt != VT_NULL)
//			{
//				strMeaning1 = (LPCSTR)_bstr_t(var);
//			}else
//			{
//				strMeaning1 = "";
//			}
//
//			//����2
//			var = m_pRecordSet->GetCollect("meaning2");			
//			if (var.vt != VT_NULL)
//			{
//				strMeaning2 = (LPCSTR)_bstr_t(var);
//			}else
//			{
//				strMeaning2 = "";
//			}
//		}	
//		m_pRecordSet->Close();				//�ر�
//	}
//	catch(_com_error *e)
//	{
//		AfxMessageBox(e->ErrorMessage());
//		return ;
//	}
//	catch(...)
//	{
//		AfxMessageBox("�������ݿ������ʱ�����쳣��");
//		return;
//	}
//
//	//������Ϣ������δ֪����
//	if (FALSE == bFinder)
//	{
//		SendMessage(m_hServHwnd, WM_USER_ADDWORD, (WPARAM)pWord, NULL);
//	}
//}

wstring CClientSocket::Utf8ToUnicode(const string& str) {
	// Ԥ��-�������п��ֽڵĳ���    
	int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	// ��ָ�򻺳�����ָ����������ڴ�    
	wchar_t *pUnicode = (wchar_t*)malloc(sizeof(wchar_t)*unicodeLen);
	// ��ʼ�򻺳���ת���ֽ�    
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, pUnicode, unicodeLen);
	wstring ret_str = pUnicode;
	free(pUnicode);
	return ret_str;
}