// LookupSocket.cpp: implementation of the CLookupSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <iostream>
#include "clientO2M.h"
#include "LookupSocket.h"
#include "clientO2MDlg.h"
#include <string> 
#include <fstream> 
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
string byteToHexStr(BYTE *byte_arr, int arr_len);

CLookupSocket::CLookupSocket( SOCKET s , WSAEVENT hEventOut, WSAEVENT hEventIn,int equipId)
{
	m_s = s;
	ZeroMemory(&m_oIO, sizeof(m_oIO));	//��Ӧ�ڷ����¼����ص��ṹ
	m_oIO.hEvent = hEventOut;
	ZeroMemory(&m_iIO, sizeof(m_iIO));	//��Ӧ�ڽ����¼����ص��ṹ
	m_iIO.hEvent = hEventIn;
	isLive = true;
	ZeroMemory(&m_hdrRecv, HEADERLEN);	//��ͷ
	//connetTimes = 0;
	//MySQLInterface mysqlUser;
	eid = equipId;					//�豸id
	string serverName = "localhost";
	string userName = "root";
	string userKey = "root";
	string dbName = "unity";

	m_sqlUser.connectMySQL(const_cast<char *>(serverName.c_str()), const_cast<char *>(userName.c_str()), const_cast<char *>(userKey.c_str()), const_cast<char *>(dbName.c_str()), 3306);
	m_dataQue = queue<vector<BYTE>>();  //��ն���
	isLive = true;
	DWORD dwThreadId;
	PTHECLIENT pParam = new THECLIENT;
	//pParam->hServHwnd = this->GetSafeHwnd();
	pParam->pClient = this;
	m_hSaveThread = CreateThread(NULL, 0, SaveData, pParam, 0, &dwThreadId);
}
CLookupSocket::~CLookupSocket()
{
	isLive = false;
	WaitForSingleObject(m_hSaveThread, INFINITE);	//�ȴ������߳��˳�
	closesocket(m_s);
	WSACloseEvent(m_iIO.hEvent);
	WSACloseEvent(m_oIO.hEvent);
	m_s = INVALID_SOCKET;
}

/*
 * ��������
 */
BOOL CLookupSocket::Send( void )
{
	DWORD	dwNumberOfBytesSent;	//���������ֽ��� 
	DWORD	dwFlags = 0;			//��־
	int		nErrCode;				//����ֵ

	//if (m_strWord.IsEmpty())
	//{
	//	return TRUE;
	//}

	WSABUF wsaSendBuf[2];
	PACKETHDR hdr;
	hdr.type = COLLECT_CE;					//���ݰ�����
	hdr.len = 1;	//���ݳ���
	wsaSendBuf[0].buf = (char*)&hdr;	//��ͷ������
	wsaSendBuf[0].len = HEADERLEN;		//��ͷ����������
	byte testss=1;

	wsaSendBuf[1].buf = (CHAR*)&testss;	//����
	wsaSendBuf[1].len = hdr.len;		//���ݳ���

	nErrCode = WSASend(m_s,							//��������
						wsaSendBuf,
						2,
						&dwNumberOfBytesSent,
						dwFlags,
						&m_oIO,
						NULL);
	if (SOCKET_ERROR == nErrCode)
	{
		if (WSA_IO_PENDING == WSAGetLastError())
		{
			return TRUE;
		}else
		{
			AfxMessageBox(_T("WSASend��������ʧ�ܣ�"));
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * ��ѯ���ʺ���
 */
BOOL CLookupSocket::Lookup( CString &strWord )
{
	m_strWord.Empty();
	m_strWord = strWord;
	if (m_strWord.IsEmpty())		//���ʲ���
	{
		return TRUE;
	}
	if (FALSE == Send())			//��������
	{
		return FALSE;
	}
	if (FALSE == RecvHeader())		//�������ݰ�ͷ
	{
		return FALSE;
	}
	return TRUE;
}

/*
 * ��������
 */
void CLookupSocket::HandleData()
{
	if (m_hdrRecv.len != 0 )//�������ݰ�ͷ
	{
		RecvPacket();		//���հ���
		ZeroMemory(&m_hdrRecv, HEADERLEN);
	}else	//�������������
	{
			//��ʾ���ʺ���
		//m_pClientDlg->ShowMeaning((TCHAR*)(LPCSTR)m_strMeaning);
		m_strMeaning.ReleaseBuffer();
	}
}

/*
 * ���հ�ͷ
 */
BOOL CLookupSocket::RecvHeader( void )
{
	DWORD	dwBytesRecved;	//�����ֽ���
	DWORD	dwFlags = 0;	//��־
	
	ZeroMemory(&m_hdrRecv, HEADERLEN);//��ͷ��0

	//����������ݻ�����
	WSABUF wsaRecv;
	wsaRecv.buf = (char*)&m_hdrRecv;	
	wsaRecv.len = HEADERLEN;
	//�첽��������
	if(SOCKET_ERROR == WSARecv(m_s, 
								&wsaRecv, 
								1,
								&dwBytesRecved,
								&dwFlags,
								&m_iIO,
								NULL))
	{
		int err = WSAGetLastError();
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			return FALSE;
		}
	}
	if (dwBytesRecved==0)
	{
		return FALSE;
	}
	return TRUE;
}

/*
 * ���հ���
 */
BOOL CLookupSocket::RecvPacket( void )
{
	DWORD	dwBytesRecved;	//�����ֽ���
	DWORD	dwFlags = 0;	//��־
	
	m_strMeaning.Empty();

	//����������ݻ�����
	WSABUF wsaRecv;
	byte *mybyte = new byte[m_hdrRecv.len]();

	//wsaRecv.buf = (char*)m_strMeaning.GetBuffer(m_hdrRecv.len/2);
	wsaRecv.buf = (char*)mybyte;
	//CHAR* mybuf = new CHAR[m_hdrRecv.len]();
	//wsaRecv.buf = mybuf;
	wsaRecv.len = m_hdrRecv.len;
	//�첽��������
	if(SOCKET_ERROR == WSARecv(m_s, 
								&wsaRecv, 
								1,
								&dwBytesRecved,
								&dwFlags,
								&m_iIO,
								NULL))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			return FALSE;
		}
	}
	//if (m_hdrRecv.len >47)
	//{
	//	return FALSE;
	//}

	if (m_hdrRecv.type == COLLECT_CE)
	{
		if (dwBytesRecved==37)
		{
			vector<BYTE> tempvec(mybyte, mybyte + 37);
			m_dataQue.push(tempvec);
		}
	}
	else if (m_hdrRecv.type == PRINTGET_CE)
	{
		vector<BYTE> tempvec(mybyte, mybyte + dwBytesRecved);
		m_codeQue.push(tempvec);
	}
	else if (m_hdrRecv.type == COUNTER_CE)
	{
		vector<BYTE> tempvec(mybyte, mybyte + dwBytesRecved);
		m_counterQue.push(tempvec);
	}
	CString outStr = _T("");
	CString tempstr;
	for (int i = 0; i < wsaRecv.len; i++)
	{
		tempstr = _T("");
		tempstr.Format(_T("%d "), mybyte[i]);
		outStr += tempstr;
	}
	//*m_pClientView->GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(outStr);
	//theApp.outCstr = outStr;
	m_strMeaning = outStr;
	delete[]mybyte;
	return TRUE;
}

//��������//���ܻ��ж��̲߳���MySQL�Ŀ�
DWORD WINAPI CLookupSocket::SaveData(void *pParam)
{
	PTHECLIENT	pThreadParam = (PTHECLIENT)pParam;
	CLookupSocket		*pClientS = pThreadParam->pClient;	//CClientSocket��ָ��
	while (pClientS->isLive)
	{
		if (!pClientS->m_codeQue.empty())
		{
			vector<BYTE> tempVec= pClientS->m_codeQue.front();
			BYTE* tempByte = new BYTE[tempVec.size()];
			copy(tempVec.begin(), tempVec.end(), tempByte);
			string testcodeIn = "insert into code (ctime,cprint,equipment_id) values(now(),x'";
			string tempstr = byteToHexStr(tempByte, tempVec.size());
			testcodeIn = testcodeIn + tempstr + "',"+to_string(pClientS->eid)+");";
			bool testbo=pClientS->m_sqlUser.writeDataToDB(testcodeIn);
			pClientS->m_codeQue.pop();

			if (!pClientS->m_counterQue.empty())//������
			{
				vector<BYTE> tempVec = pClientS->m_counterQue.front();
				vector<int> outVec = pClientS->CounterToInt(tempVec);
				string itable = "insert into data (dtime,dvalue,dcnl,equipment_id) values";
				//itable = itable + "(now()," + to_string(staPrnting) + ",1," + to_string(eid) + "),";
				for (int i=0;i<outVec.size();i++)
				{
					if (i== outVec.size()-1)
					{
						itable = itable + "(now()," + to_string(outVec[i]) + ","+to_string(18+i)+"," + to_string(pClientS->eid) + ");";
					}
					else
					{
						itable = itable + "(now()," + to_string(outVec[i]) + "," + to_string(18 + i) + "," + to_string(pClientS->eid) + "),";
					}
				}
				bool testbo = pClientS->m_sqlUser.writeDataToDB(itable);
				pClientS->m_counterQue.pop();
			}
			Sleep(10);
		}
		else if (!pClientS->m_dataQue.empty())
		{
			pClientS->bytStatus = pClientS->m_dataQue.front();
			DWORD dwThreadId;
			PTHECLIENT pParam = new THECLIENT;
			//pParam->hServHwnd = this->GetSafeHwnd();
			pParam->pClient = pClientS;
			HANDLE m_hWorkerThread = CreateThread(NULL, 0, getstatu, pParam, 0, &dwThreadId);
			pClientS->m_dataQue.pop();
			Sleep(10);
		}
		else 
		{
			Sleep(1000);
		}
	}
	return 0;
}


DWORD WINAPI CLookupSocket::getstatu(void *pParam)
{
	PTHECLIENT	pThreadParam = (PTHECLIENT)pParam;
	CLookupSocket		*pClientS = pThreadParam->pClient;	//CClientSocket��ָ��
	
	pClientS->staSysRea = pClientS->GETnBIT_from_bytStatus(0, 0, 1) == _T("1") ? true : false;  //ϵͳ׼����
	pClientS->staSysBus = (pClientS->GETnBIT_from_bytStatus(0, 1, 1) == _T("1") ? true : false);  //ϵͳæ
	pClientS->staBumMod = (pClientS->GETnBIT_from_bytStatus(0, 2, 1) == _T("1") ? true : false); //��ģʽ
	pClientS->staBum = (pClientS->GETnBIT_from_bytStatus(0, 3, 1) == _T("1") ? true : false);   //'�ÿ���
	pClientS->staHarFin = (pClientS->GETnBIT_from_bytStatus(0, 4, 1) == _T("1") ? true : false);  //'��λ����λ���
	pClientS->staCleFauFin = (pClientS->GETnBIT_from_bytStatus(0, 5, 1) == _T("1") ? true : false);// '����������
	pClientS->staNozVal = (pClientS->GETnBIT_from_bytStatus(1, 0, 1) == _T("1") ? true : false);   //'���췧
	pClientS->staFeeVal = (pClientS->GETnBIT_from_bytStatus(1, 1, 1) == _T("1") ? true : false);  //'��ī��
	pClientS->staBleVal = (pClientS->GETnBIT_from_bytStatus(1, 2, 1) == _T("1") ? true : false);  //'������
	pClientS->staFluVal = (pClientS->GETnBIT_from_bytStatus(1, 3, 1) == _T("1") ? true : false);  //'��ϴ��
	pClientS->staSolVal = (pClientS->GETnBIT_from_bytStatus(1, 4, 1) == _T("1") ? true : false);  //'�ܼ���
	pClientS->staVisVal = (pClientS->GETnBIT_from_bytStatus(1, 5, 1) == _T("1") ? true : false);  //'ճ�ȷ�
	pClientS->staWasVal = (pClientS->GETnBIT_from_bytStatus(1, 6, 1) == _T("1") ? true : false);  //'��ϴ��
	pClientS->staInkFloSenOff = (pClientS->GETnBIT_from_bytStatus(2, 0, 1) == _T("1") ? true : false);  //'�ػ���
	pClientS->staCloInkLin = (pClientS->GETnBIT_from_bytStatus(2, 1, 1) == _T("1") ? true : false);  //'��ī��
	pClientS->staAddSol = (pClientS->GETnBIT_from_bytStatus(2, 2, 1) == _T("1") ? true : false); //'����ܼ�
	pClientS->staDetVis = (pClientS->GETnBIT_from_bytStatus(2, 3, 1) == _T("1") ? true : false); //'����ճ��
	pClientS->staWasNoz = (pClientS->GETnBIT_from_bytStatus(2, 4, 1) == _T("1") ? true : false); //'��ϴ��ͷ
	pClientS->staSucNoz = (pClientS->GETnBIT_from_bytStatus(2, 5, 1) == _T("1") ? true : false);//'��������
	pClientS->staAdjInkLin = (pClientS->GETnBIT_from_bytStatus(2, 6, 1) == _T("1") ? true : false); // 'ī·У��
	pClientS->staInkCir = (pClientS->GETnBIT_from_bytStatus(2, 7, 1) == _T("1") ? true : false); //'ī·ѭ��
	pClientS->staInkTemSenFau = (pClientS->GETnBIT_from_bytStatus(3, 0, 1) == _T("1") ? true : false); //'īˮ�¶ȴ���������
	pClientS->staPriHeaTemFau = (pClientS->GETnBIT_from_bytStatus(3, 1, 1) == _T("1") ? true : false);  //'��ͷ�¶ȴ���������
	pClientS->staBumSpeOveFau = (pClientS->GETnBIT_from_bytStatus(3, 2, 1) == _T("1") ? true : false);  //'�ó��ٱ���
	pClientS->staPreOveFau = (pClientS->GETnBIT_from_bytStatus(3, 3, 1) == _T("1") ? true : false); //'��ѹ����
	pClientS->staVisAbnFau = (pClientS->GETnBIT_from_bytStatus(3, 4, 1) == _T("1") ? true : false);  //'ճ���쳣
	pClientS->staVisSenFau = (pClientS->GETnBIT_from_bytStatus(3, 5, 1) == _T("1") ? true : false); //'ճ�ȼƹ���
	pClientS->staInkFloFau = (pClientS->GETnBIT_from_bytStatus(3, 6, 1) == _T("1") ? true : false); //'���չ���
	pClientS->staPriHeaCle = (pClientS->GETnBIT_from_bytStatus(3, 7, 1) == _T("1") ? true : false);  //'���ػ���ϴ
	pClientS->staFanFau = (pClientS->GETnBIT_from_bytStatus(4, 0, 1) == _T("1") ? true : false); // '���ȹ���
	pClientS->staChaFau = (pClientS->GETnBIT_from_bytStatus(4, 1, 1) == _T("1") ? true : false);  //'������
	pClientS->staPhaFau = (pClientS->GETnBIT_from_bytStatus(4, 2, 1) == _T("1") ? true : false);  //'��λ����
	pClientS->staHigVolFau = (pClientS->GETnBIT_from_bytStatus(4, 3, 1) == _T("1") ? true : false); // '��ѹ����
	pClientS->staSolLevFau =to_string(pClientS->GETnBIT_from_bytStatus(4, 5, 1) == _T("1") ? true : false) + to_string(pClientS->GETnBIT_from_bytStatus(4, 4, 1) == _T("1") ? true : false);  //'�ܼ�Һλ״̬
	pClientS->staInkLevFau =to_string(pClientS->GETnBIT_from_bytStatus(4, 7, 1) == _T("1") ? true : false) + to_string(pClientS->GETnBIT_from_bytStatus(4, 6, 1) == _T("1") ? true : false);  //'īˮҺλ״̬
	pClientS->staPrnting = (pClientS->GETnBIT_from_bytStatus(5, 0, 1) == _T("1") ? true : false); // '��ӡ��
	//'staBufOveFau = IIf(pClientS->GETnBIT_from_bytStatus(5, 2, 1)==_T("1"));  // '�ı�buf���
	pClientS->staHigVolSwi = (pClientS->GETnBIT_from_bytStatus(5, 3, 1) == _T("1") ? true : false); //'��ѹ����
	pClientS->staActProSen = (pClientS->GETnBIT_from_bytStatus(5, 4, 1) == _T("1") ? true : false); //'���۵�ǰ��ƽ
	pClientS->staProSenFas = (pClientS->GETnBIT_from_bytStatus(5, 5, 1) == _T("1") ? true : false);  ////'���۹���
	pClientS->staAutModFau = (pClientS->GETnBIT_from_bytStatus(5, 6, 1) == _T("1") ? true : false); // '�Զ�����ʧ��
	pClientS->staValFau = (pClientS->GETnBIT_from_bytStatus(5, 7, 1) == _T("1") ? true : false);  //'������
	pClientS->staPrinted = (pClientS->GETnBIT_from_bytStatus(6, 0, 1) == _T("1") ? true : false); //'��ӡ���
	pClientS->staRemPrinSwi = (pClientS->GETnBIT_from_bytStatus(6, 1, 1) == _T("1") ? true : false); //  'Զ�̴�ӡ����
	//'staBufFul = (pClientS->GETnBIT_from_bytStatus(6, 2, 1)==_T("1"));   //'�ı�buf��
	pClientS->staBufRea = (pClientS->GETnBIT_from_bytStatus(6, 3, 1) == _T("1") ? true : false);  //'��Ϣ׼����
	pClientS->staEncDir = (pClientS->GETnBIT_from_bytStatus(6, 4, 1) == _T("1") ? true : false);  //'����������
	pClientS->staLinFas = (pClientS->GETnBIT_from_bytStatus(6, 5, 1) == _T("1") ? true : false);  //'����������
	pClientS->staPriHeaHot = (pClientS->GETnBIT_from_bytStatus(6, 6, 1) == _T("1") ? true : false);  // '����״̬
	pClientS->staPriHeaHotFau = (pClientS->GETnBIT_from_bytStatus(6, 7, 1) == _T("1") ? true : false); // '���¹���
	if (pClientS->bytStatus[8] * 256 + pClientS->bytStatus[7] > 5000)  //'ʵʱѹ��
	{
		pClientS->staPressure = pClientS->bytStatus[8] * 256 + pClientS->bytStatus[7] - 65536;
	}
	else
	{
		pClientS->staPressure = pClientS->bytStatus[8] * 256 + pClientS->bytStatus[7];
	}

	pClientS->staBumSpe = pClientS->bytStatus[10] * 256 + pClientS->bytStatus[9];     //'ʵʱ����
	pClientS->staPriHeaTem = pClientS->bytStatus[11];                                       //'ʵʱ��ͷ�¶�
	pClientS->staInkTem = (pClientS->bytStatus[13] * 256 + pClientS->bytStatus[12]) / 10;              //'ʵʱīˮ�¶�
	pClientS->staActVis = pClientS->bytStatus[14];                                     //'Ŀǰʵʱճ��
	pClientS->staTarVis = pClientS->bytStatus[15];                                         // 'Ŀ��ο�ճ��
	pClientS->staInkLev = pClientS->bytStatus[16];                                        // 'ʵʱīˮҺλ
	pClientS->staSolLev = pClientS->bytStatus[17];                                          //'ʵʱ�ܼ�Һλ
	pClientS->staHigVol = pClientS->bytStatus[19] * 256 + pClientS->bytStatus[18];                      // 'ʵʱ��ѹ
	pClientS->staPhase = pClientS->bytStatus[20];                                        // 'ʵʱ��λ
	pClientS->staEncFre = pClientS->bytStatus[22] * 256 + pClientS->bytStatus[21];                       //'ʵʱ������Ƶ��
	pClientS->staProCou = pClientS->bytStatus[26] * pow(256, 3) + pClientS->bytStatus[25] * pow(256, 2) + pClientS->bytStatus[24] * 256 + pClientS->bytStatus[23];      // '��Ʒ������
	pClientS->staPriCou = pClientS->bytStatus[30] * pow(256, 3) + pClientS->bytStatus[29] * pow(256, 2) + pClientS->bytStatus[28] * 256 + pClientS->bytStatus[27];      // '��ӡ������
	pClientS->staPixDotNee = pClientS->bytStatus[31];                                  // '�й��ɵ���
	pClientS->staAutModVol = pClientS->bytStatus[32];                                  // '�Զ����ѵ�ѹ

	pClientS->staSetTimeEna = (pClientS->GETnBIT_from_bytStatus(36, 5, 1) == _T("1") ? true : false);   //'ά����īˮʱ����Ĺ��ܿ���
	if (!pClientS->staSetTimeEna)
	{
		pClientS->staInkLifeTime = (pClientS->bytStatus[36] & 31) * pow(256, 3) + pClientS->bytStatus[35] * pow(256, 2) + pClientS->bytStatus[34] * 256 + pClientS->bytStatus[33];   // 'īˮʱ��
		pClientS->staRFID = to_string(pClientS->GETnBIT_from_bytStatus(36, 7, 1) == _T("1") ? true : false) + to_string(pClientS->GETnBIT_from_bytStatus(36, 6, 1) == _T("1") ? true : false); //'RFID״̬
	}
	pClientS->saveDataToMySQL();
	return 0;
}

CString CLookupSocket::GETnBIT_from_bytStatus(int I, int m, int n)
{
	string tempCstr = "";
	tempCstr = "00000000" + DEC_to_BIN(bytStatus[I]);

	//tempCstr=tempCstr.Mid(tempCstr.GetLength()-m,n);

	CString cstringStr = Utf8ToUnicode(tempCstr).c_str();
	int dd = cstringStr.GetLength();
	return cstringStr.Mid(cstringStr.GetLength() - m - 1, n);
}

string CLookupSocket::DEC_to_BIN(long long Dec)
{
	string DecToBin;
	while (Dec > 0)
	{
		if (Dec % 2 > 0)
		{
			DecToBin = to_string(Dec % 2) + DecToBin;
		}
		else
		{
			DecToBin = to_string(0) + DecToBin;
		}
		Dec = Dec / 2;
	}
	return DecToBin;
}

wstring CLookupSocket::Utf8ToUnicode(const string& str) {
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

void CLookupSocket::saveDataToMySQL()
{
	string itable = "insert into data (dtime,dvalue,dcnl,equipment_id) values";
	itable = itable + "(now()," + to_string(staPrnting) + ",1,"+to_string(eid)+"),";
	itable = itable + "(now()," + to_string(staProCou) + ",2," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staPriCou) + ",3," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staPressure) + ",4," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staBumSpe) + ",5," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staInkTem) + ",6," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staPriHeaTem) + ",7," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staInkLev) + ",8," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staSolLev) + ",9," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staTarVis) + ",10," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staActVis) + ",11," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staHigVol) + ",12," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staAutModVol) + ",13," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staPhase) + ",14," + to_string(eid) + "),";
	itable = itable + "(now()," + to_string(staInkLifeTime) + ",15," + to_string(eid) + "),";//īˮʱ��
	itable = itable + "(now()," + to_string(staEncFre) + ",16," + to_string(eid) + "),";//������Ƶ��
	itable = itable + "(now()," + to_string(staPixDotNee) + ",17," + to_string(eid) + ");";//�е���
	bool testbo = m_sqlUser.writeDataToDB(itable);
}

char* CLookupSocket::ConvertBinaryToString(char* pBinaryData, int nLen)
{
	static char s_BinaryData[10240];
	mysql_real_escape_string(&m_sqlUser.mysqlInstance, s_BinaryData, pBinaryData, nLen);
	return s_BinaryData;
}

string byteToHexStr(BYTE *byte_arr, int arr_len)
{
	string  hexstr ;
	for (int i = 0; i < arr_len; i++)
	{
		char hex1;
		char hex2;
		int value = byte_arr[i]; //ֱ�ӽ�unsigned char��ֵ�����͵�ֵ��ϵͳ������ǿ��ת��
		int v1 = value / 16;
		int v2 = value % 16;

		//����ת����ĸ
		if (v1 >= 0 && v1 <= 9)
			hex1 = (char)(48 + v1);
		else
			hex1 = (char)(55 + v1);

		//������ת����ĸ
		if (v2 >= 0 && v2 <= 9)
			hex2 = (char)(48 + v2);
		else
			hex2 = (char)(55 + v2);

		//����ĸ���ӳɴ�
		hexstr = hexstr + hex1 + hex2;
	}
	return hexstr;
}

vector<int> CLookupSocket::CounterToInt(vector<BYTE>input)
{
	vector<int> outVec;
	outVec.push_back(input.size() / 4);
	for (int i=0;i< outVec[0];i++)
	{
		outVec.push_back((int)(input[4*i] | input[4 * i+1] << 8 | input[4 * i+2] << 16 | input[4 * i+3] << 24));
	}
	return outVec;
}

//����cstring
BOOL CLookupSocket::SendCstring(const u_short type, const CString &strData)
{
	//ASSERT(!strData.IsEmpty());

	//int			nErrCode;				//����ֵ	
	//PACKETHDR	packetHdr;				//�����ͷ

	//packetHdr.type = type;				//����
	//packetHdr.len = strData.GetLength();//���ݳ���

	////���Ͱ�ͷ
	//nErrCode = send(m_s, (char*)&packetHdr, PACKETHDRLEN, 0);
	//if (SOCKET_ERROR == nErrCode)
	//{
	//	AfxMessageBox("�����û��б����");
	//	return FALSE;
	//}
	////���Ͱ���
	//nErrCode = send(m_s, strData, packetHdr.len, 0);
	//if (SOCKET_ERROR == nErrCode)
	//{
	//	AfxMessageBox("�����û��б����");
	//	return FALSE;
	//}
	//return TRUE;

	DWORD	dwNumberOfBytesSent;	//���������ֽ��� 
	DWORD	dwFlags = 0;			//��־
	int		nErrCode;				//����ֵ

	//if (m_strWord.IsEmpty())
	//{
	//	return TRUE;
	//}
	string fileName = UnicodeToUtf8(LPCWSTR(strData));

	WSABUF wsaSendBuf[2];
	PACKETHDR hdr;
	hdr.type = type;					//���ݰ�����
	hdr.len = fileName.length();	//���ݳ���
	wsaSendBuf[0].buf = (char*)&hdr;	//��ͷ������
	wsaSendBuf[0].len = HEADERLEN;		//��ͷ����������
	//USES_CONVERSION;
	//const char* fileName = W2A(strData);//CString To ConstChar
	
	wsaSendBuf[1].buf = (CHAR*)fileName.c_str();	//����/////�����⣬��ת��
	wsaSendBuf[1].len = hdr.len;		//���ݳ���

	nErrCode = WSASend(m_s,							//��������
		wsaSendBuf,
		2,
		&dwNumberOfBytesSent,
		dwFlags,
		&m_oIO,
		NULL);
	if (SOCKET_ERROR == nErrCode)
	{
		if (WSA_IO_PENDING == WSAGetLastError())
		{
			return TRUE;
		}
		else
		{
			AfxMessageBox(_T("WSASend��������ʧ�ܣ�"));
			return FALSE;
		}
	}
	return TRUE;
}


string CLookupSocket::UnicodeToUtf8(const wstring& wstr) {
	// Ԥ��-�������ж��ֽڵĳ���    
	int ansiiLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	// ��ָ�򻺳�����ָ����������ڴ�    
	char *pAssii = (char*)malloc(sizeof(char)*ansiiLen);
	// ��ʼ�򻺳���ת���ֽ�    
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, pAssii, ansiiLen, nullptr, nullptr);
	string ret_str = pAssii;
	free(pAssii);
	return ret_str;
}

BOOL CLookupSocket::SendFile(const u_short type, string srcFileName)
{
	DWORD	dwNumberOfBytesSent;	//���������ֽ��� 
	DWORD	dwFlags = 0;			//��־
	int		nErrCode;				//����ֵ

	const int bufferSize = 1024;
	char buffer[bufferSize] = { 0 };
	int readLen = 0;
	//string srcFileName = "in";
	ifstream srcFile;
	srcFile.open(srcFileName.c_str(), ios::binary);
	if (!srcFile) {
		return FALSE;
	}
	while (!srcFile.eof()) {
		srcFile.read(buffer, bufferSize);
		readLen = srcFile.gcount();

		WSABUF wsaSendBuf[2];
		PACKETHDR hdr;
		hdr.type = type;					//���ݰ�����
		hdr.len = readLen;	//���ݳ���
		wsaSendBuf[0].buf = (char*)&hdr;	//��ͷ������
		wsaSendBuf[0].len = HEADERLEN;		//��ͷ����������
		//USES_CONVERSION;
		//const char* fileName = W2A(strData);//CString To ConstChar

		wsaSendBuf[1].buf = buffer;	//����/////�����⣬��ת��
		wsaSendBuf[1].len = hdr.len;		//���ݳ���

		nErrCode = WSASend(m_s,							//��������
			wsaSendBuf,
			2,
			&dwNumberOfBytesSent,
			dwFlags,
			&m_oIO,
			NULL);
		if (SOCKET_ERROR == nErrCode)
		{
			srcFile.close();
			if (WSA_IO_PENDING == WSAGetLastError())
			{
				return TRUE;
			}
			else
			{
				AfxMessageBox(_T("WSASend��������ʧ�ܣ�"));
				return FALSE;
			}
		}

	}
	srcFile.close();
	return TRUE;
}

BOOL CLookupSocket::SendLab(CString filePath)
{
	int findPos = filePath.ReverseFind('\\');
	CString sendStr = filePath.Right(filePath.GetLength() - findPos - 1);
	if(!SendCstring(LAB_NAME, sendStr))
		return FALSE;
	Sleep(10);
	m_filePath = UnicodeToUtf8(LPCWSTR(filePath));
	if (!SendFile(PRINT_CE, m_filePath))
		return FALSE;
	return TRUE;
}