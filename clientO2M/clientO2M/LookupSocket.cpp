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
	ZeroMemory(&m_oIO, sizeof(m_oIO));	//对应于发送事件的重叠结构
	m_oIO.hEvent = hEventOut;
	ZeroMemory(&m_iIO, sizeof(m_iIO));	//对应于接收事件的重叠结构
	m_iIO.hEvent = hEventIn;
	isLive = true;
	ZeroMemory(&m_hdrRecv, HEADERLEN);	//包头
	//connetTimes = 0;
	//MySQLInterface mysqlUser;
	eid = equipId;					//设备id
	string serverName = "localhost";
	string userName = "root";
	string userKey = "root";
	string dbName = "unity";

	m_sqlUser.connectMySQL(const_cast<char *>(serverName.c_str()), const_cast<char *>(userName.c_str()), const_cast<char *>(userKey.c_str()), const_cast<char *>(dbName.c_str()), 3306);
	m_dataQue = queue<vector<BYTE>>();  //清空队列
	isLive = true;
	staSolLevFauLas = "00";			//溶剂标志
	staInkLevFauLas = "00";			//墨水标志
	DWORD dwThreadId;
	PTHECLIENT pParam = new THECLIENT;
	//pParam->hServHwnd = this->GetSafeHwnd();
	pParam->pClient = this;
	m_hSaveThread = CreateThread(NULL, 0, SaveData, pParam, 0, &dwThreadId);
}
CLookupSocket::~CLookupSocket()
{
	isLive = false;
	//WaitForSingleObject(m_hSaveThread, INFINITE);	//等待工作线程退出
	DWORD dwRet = 0;
	MSG msg;
	while (TRUE)
	{
		//wait for m_hThread to be over，and wait for
		//QS_ALLINPUT（Any message is in the queue）
		dwRet = MsgWaitForMultipleObjects(1, &m_hSaveThread, FALSE, INFINITE, QS_ALLINPUT);
		switch (dwRet)
		{
		case WAIT_OBJECT_0:
			break; //break the loop
		case WAIT_OBJECT_0 + 1:
			//get the message from Queue
			//and dispatch it to specific window
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			DispatchMessage(&msg);
			continue;
		default:
			break; // unexpected failure
		}
		break;
	}
	CloseHandle(m_hSaveThread);
	closesocket(m_s);
	WSACloseEvent(m_iIO.hEvent);
	WSACloseEvent(m_oIO.hEvent);
	m_s = INVALID_SOCKET;
	m_sqlUser.closeMySQL();
}

/*
 * 发送数据
 */
BOOL CLookupSocket::Send( void )
{
	DWORD	dwNumberOfBytesSent;	//发送数据字节数 
	DWORD	dwFlags = 0;			//标志
	int		nErrCode;				//返回值

	//if (m_strWord.IsEmpty())
	//{
	//	return TRUE;
	//}

	WSABUF wsaSendBuf[2];
	PACKETHDR hdr;
	hdr.type = COLLECT_CE;					//数据包类型
	hdr.len = 1;	//数据长度
	wsaSendBuf[0].buf = (char*)&hdr;	//包头缓冲区
	wsaSendBuf[0].len = HEADERLEN;		//包头缓冲区长度
	byte testss=1;

	wsaSendBuf[1].buf = (CHAR*)&testss;	//包体
	wsaSendBuf[1].len = hdr.len;		//数据长度

	nErrCode = WSASend(m_s,							//发送数据
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
			AfxMessageBox(_T("WSASend函数调用失败！"));
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * 查询单词含义
 */
BOOL CLookupSocket::Lookup( CString &strWord )
{
	m_strWord.Empty();
	m_strWord = strWord;
	if (m_strWord.IsEmpty())		//单词不空
	{
		return TRUE;
	}
	if (FALSE == Send())			//发送数据
	{
		return FALSE;
	}
	if (FALSE == RecvHeader())		//接收数据包头
	{
		return FALSE;
	}
	return TRUE;
}

/*
 * 处理数据
 */
void CLookupSocket::HandleData()
{
	if (m_hdrRecv.len != 0 )//接收数据包头
	{
		RecvPacket();		//接收包体
		ZeroMemory(&m_hdrRecv, HEADERLEN);
	}else	//接收数包体完毕
	{
			//显示单词含义
		//m_pClientDlg->ShowMeaning((TCHAR*)(LPCSTR)m_strMeaning);
		//m_strMeaning.ReleaseBuffer();
	}
}

/*
 * 接收包头
 */
BOOL CLookupSocket::RecvHeader( void )
{
	DWORD	dwBytesRecved;	//接收字节数
	DWORD	dwFlags = 0;	//标志
	
	ZeroMemory(&m_hdrRecv, HEADERLEN);//包头置0

	//定义接收数据缓冲区
	WSABUF wsaRecv;
	wsaRecv.buf = (char*)&m_hdrRecv;	
	wsaRecv.len = HEADERLEN;
	//异步接收数据
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
 * 接收包体
 */
BOOL CLookupSocket::RecvPacket( void )
{
	DWORD	dwBytesRecved;	//接收字节数
	DWORD	dwFlags = 0;	//标志
	
	//m_strMeaning.Empty();

	//定义接收数据缓冲区
	WSABUF wsaRecv;
	byte *mybyte = new byte[m_hdrRecv.len]();

	//wsaRecv.buf = (char*)m_strMeaning.GetBuffer(m_hdrRecv.len/2);
	wsaRecv.buf = (char*)mybyte;
	//CHAR* mybuf = new CHAR[m_hdrRecv.len]();
	//wsaRecv.buf = mybuf;
	wsaRecv.len = m_hdrRecv.len;
	//异步接收数据
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
	//CString outStr = _T("");
	//CString tempstr;
	//for (int i = 0; i < wsaRecv.len; i++)
	//{
	//	tempstr = _T("");
	//	tempstr.Format(_T("%d "), mybyte[i]);
	//	outStr += tempstr;
	//}
	////*m_pClientView->GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(outStr);
	////theApp.outCstr = outStr;
	//m_strMeaning = outStr;
	delete[]mybyte;
	return TRUE;
}


DWORD WINAPI CLookupSocket::SaveData(void *pParam)
{
	PTHECLIENT	pThreadParam = (PTHECLIENT)pParam;
	CLookupSocket		*pClientS = pThreadParam->pClient;	//CClientSocket类指针
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
			CloseHandle(m_hWorkerThread);
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
	CLookupSocket		*pClientS = pThreadParam->pClient;	//CClientSocket类指针
	
	pClientS->staSysRea = pClientS->GETnBIT_from_bytStatus(0, 0, 1) == _T("1") ? true : false;  //系统准备好
	pClientS->staSysBus = (pClientS->GETnBIT_from_bytStatus(0, 1, 1) == _T("1") ? true : false);  //系统忙
	pClientS->staBumMod = (pClientS->GETnBIT_from_bytStatus(0, 2, 1) == _T("1") ? true : false); //泵模式
	pClientS->staBum = (pClientS->GETnBIT_from_bytStatus(0, 3, 1) == _T("1") ? true : false);   //'泵开关
	pClientS->staHarFin = (pClientS->GETnBIT_from_bytStatus(0, 4, 1) == _T("1") ? true : false);  //'下位机复位完成
	pClientS->staCleFauFin = (pClientS->GETnBIT_from_bytStatus(0, 5, 1) == _T("1") ? true : false);// '清除故障完成
	pClientS->staNozVal = (pClientS->GETnBIT_from_bytStatus(1, 0, 1) == _T("1") ? true : false);   //'喷嘴阀
	pClientS->staFeeVal = (pClientS->GETnBIT_from_bytStatus(1, 1, 1) == _T("1") ? true : false);  //'供墨阀
	pClientS->staBleVal = (pClientS->GETnBIT_from_bytStatus(1, 2, 1) == _T("1") ? true : false);  //'排气阀
	pClientS->staFluVal = (pClientS->GETnBIT_from_bytStatus(1, 3, 1) == _T("1") ? true : false);  //'清洗阀
	pClientS->staSolVal = (pClientS->GETnBIT_from_bytStatus(1, 4, 1) == _T("1") ? true : false);  //'溶剂阀
	pClientS->staVisVal = (pClientS->GETnBIT_from_bytStatus(1, 5, 1) == _T("1") ? true : false);  //'粘度阀
	pClientS->staWasVal = (pClientS->GETnBIT_from_bytStatus(1, 6, 1) == _T("1") ? true : false);  //'冲洗阀
	pClientS->staInkFloSenOff = (pClientS->GETnBIT_from_bytStatus(2, 0, 1) == _T("1") ? true : false);  //'关回收
	pClientS->staCloInkLin = (pClientS->GETnBIT_from_bytStatus(2, 1, 1) == _T("1") ? true : false);  //'关墨线
	pClientS->staAddSol = (pClientS->GETnBIT_from_bytStatus(2, 2, 1) == _T("1") ? true : false); //'添加溶剂
	pClientS->staDetVis = (pClientS->GETnBIT_from_bytStatus(2, 3, 1) == _T("1") ? true : false); //'测试粘度
	pClientS->staWasNoz = (pClientS->GETnBIT_from_bytStatus(2, 4, 1) == _T("1") ? true : false); //'冲洗喷头
	pClientS->staSucNoz = (pClientS->GETnBIT_from_bytStatus(2, 5, 1) == _T("1") ? true : false);//'反吸喷嘴
	pClientS->staAdjInkLin = (pClientS->GETnBIT_from_bytStatus(2, 6, 1) == _T("1") ? true : false); // '墨路校正
	pClientS->staInkCir = (pClientS->GETnBIT_from_bytStatus(2, 7, 1) == _T("1") ? true : false); //'墨路循环
	pClientS->staInkTemSenFau = (pClientS->GETnBIT_from_bytStatus(3, 0, 1) == _T("1") ? true : false); //'墨水温度传感器故障
	pClientS->staPriHeaTemFau = (pClientS->GETnBIT_from_bytStatus(3, 1, 1) == _T("1") ? true : false);  //'喷头温度传感器故障
	pClientS->staBumSpeOveFau = (pClientS->GETnBIT_from_bytStatus(3, 2, 1) == _T("1") ? true : false);  //'泵超速保护
	pClientS->staPreOveFau = (pClientS->GETnBIT_from_bytStatus(3, 3, 1) == _T("1") ? true : false); //'过压保护
	pClientS->staVisAbnFau = (pClientS->GETnBIT_from_bytStatus(3, 4, 1) == _T("1") ? true : false);  //'粘度异常
	pClientS->staVisSenFau = (pClientS->GETnBIT_from_bytStatus(3, 5, 1) == _T("1") ? true : false); //'粘度计故障
	pClientS->staInkFloFau = (pClientS->GETnBIT_from_bytStatus(3, 6, 1) == _T("1") ? true : false); //'回收故障
	pClientS->staPriHeaCle = (pClientS->GETnBIT_from_bytStatus(3, 7, 1) == _T("1") ? true : false);  //'开关机清洗
	pClientS->staFanFau = (pClientS->GETnBIT_from_bytStatus(4, 0, 1) == _T("1") ? true : false); // '风扇故障
	pClientS->staChaFau = (pClientS->GETnBIT_from_bytStatus(4, 1, 1) == _T("1") ? true : false);  //'充电故障
	pClientS->staPhaFau = (pClientS->GETnBIT_from_bytStatus(4, 2, 1) == _T("1") ? true : false);  //'相位故障
	pClientS->staHigVolFau = (pClientS->GETnBIT_from_bytStatus(4, 3, 1) == _T("1") ? true : false); // '高压故障
	pClientS->staSolLevFau =to_string(pClientS->GETnBIT_from_bytStatus(4, 5, 1) == _T("1") ? true : false) + to_string(pClientS->GETnBIT_from_bytStatus(4, 4, 1) == _T("1") ? true : false);  //'溶剂液位状态
	pClientS->staInkLevFau =to_string(pClientS->GETnBIT_from_bytStatus(4, 7, 1) == _T("1") ? true : false) + to_string(pClientS->GETnBIT_from_bytStatus(4, 6, 1) == _T("1") ? true : false);  //'墨水液位状态
	pClientS->staPrnting = (pClientS->GETnBIT_from_bytStatus(5, 0, 1) == _T("1") ? true : false); // '打印中
	//'staBufOveFau = IIf(pClientS->GETnBIT_from_bytStatus(5, 2, 1)==_T("1"));  // '文本buf溢出
	pClientS->staHigVolSwi = (pClientS->GETnBIT_from_bytStatus(5, 3, 1) == _T("1") ? true : false); //'高压开关
	pClientS->staActProSen = (pClientS->GETnBIT_from_bytStatus(5, 4, 1) == _T("1") ? true : false); //'电眼当前电平
	pClientS->staProSenFas = (pClientS->GETnBIT_from_bytStatus(5, 5, 1) == _T("1") ? true : false);  ////'电眼过快
	pClientS->staAutModFau = (pClientS->GETnBIT_from_bytStatus(5, 6, 1) == _T("1") ? true : false); // '自动分裂失败
	pClientS->staValFau = (pClientS->GETnBIT_from_bytStatus(5, 7, 1) == _T("1") ? true : false);  //'阀故障
	pClientS->staPrinted = (pClientS->GETnBIT_from_bytStatus(6, 0, 1) == _T("1") ? true : false); //'打印完成
	pClientS->staRemPrinSwi = (pClientS->GETnBIT_from_bytStatus(6, 1, 1) == _T("1") ? true : false); //  '远程打印开关
	//'staBufFul = (pClientS->GETnBIT_from_bytStatus(6, 2, 1)==_T("1"));   //'文本buf满
	pClientS->staBufRea = (pClientS->GETnBIT_from_bytStatus(6, 3, 1) == _T("1") ? true : false);  //'信息准备好
	pClientS->staEncDir = (pClientS->GETnBIT_from_bytStatus(6, 4, 1) == _T("1") ? true : false);  //'编码器方向
	pClientS->staLinFas = (pClientS->GETnBIT_from_bytStatus(6, 5, 1) == _T("1") ? true : false);  //'编码器过快
	pClientS->staPriHeaHot = (pClientS->GETnBIT_from_bytStatus(6, 6, 1) == _T("1") ? true : false);  // '恒温状态
	pClientS->staPriHeaHotFau = (pClientS->GETnBIT_from_bytStatus(6, 7, 1) == _T("1") ? true : false); // '恒温故障
	if (pClientS->bytStatus[8] * 256 + pClientS->bytStatus[7] > 5000)  //'实时压力
	{
		pClientS->staPressure = pClientS->bytStatus[8] * 256 + pClientS->bytStatus[7] - 65536;
	}
	else
	{
		pClientS->staPressure = pClientS->bytStatus[8] * 256 + pClientS->bytStatus[7];
	}

	pClientS->staBumSpe = pClientS->bytStatus[10] * 256 + pClientS->bytStatus[9];     //'实时泵速
	pClientS->staPriHeaTem = pClientS->bytStatus[11];                                       //'实时喷头温度
	pClientS->staInkTem = (pClientS->bytStatus[13] * 256 + pClientS->bytStatus[12]) / 10;              //'实时墨水温度
	pClientS->staActVis = pClientS->bytStatus[14];                                     //'目前实时粘度
	pClientS->staTarVis = pClientS->bytStatus[15];                                         // '目标参考粘度
	pClientS->staInkLev = pClientS->bytStatus[16];                                        // '实时墨水液位
	pClientS->staSolLev = pClientS->bytStatus[17];                                          //'实时溶剂液位
	pClientS->staHigVol = pClientS->bytStatus[19] * 256 + pClientS->bytStatus[18];                      // '实时高压
	pClientS->staPhase = pClientS->bytStatus[20];                                        // '实时相位
	pClientS->staEncFre = pClientS->bytStatus[22] * 256 + pClientS->bytStatus[21];                       //'实时编码器频率
	pClientS->staProCou = pClientS->bytStatus[26] * pow(256, 3) + pClientS->bytStatus[25] * pow(256, 2) + pClientS->bytStatus[24] * 256 + pClientS->bytStatus[23];      // '产品计数器
	pClientS->staPriCou = pClientS->bytStatus[30] * pow(256, 3) + pClientS->bytStatus[29] * pow(256, 2) + pClientS->bytStatus[28] * 256 + pClientS->bytStatus[27];      // '打印计数器
	pClientS->staPixDotNee = pClientS->bytStatus[31];                                  // '列构成点数
	pClientS->staAutModVol = pClientS->bytStatus[32];                                  // '自动分裂电压

	pClientS->staSetTimeEna = (pClientS->GETnBIT_from_bytStatus(36, 5, 1) == _T("1") ? true : false);   //'维护、墨水时间更改功能开放
	if (!pClientS->staSetTimeEna)
	{
		pClientS->staInkLifeTime = (pClientS->bytStatus[36] & 31) * pow(256, 3) + pClientS->bytStatus[35] * pow(256, 2) + pClientS->bytStatus[34] * 256 + pClientS->bytStatus[33];   // '墨水时间
		pClientS->staRFID = to_string(pClientS->GETnBIT_from_bytStatus(36, 7, 1) == _T("1") ? true : false) + to_string(pClientS->GETnBIT_from_bytStatus(36, 6, 1) == _T("1") ? true : false); //'RFID状态
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
	// 预算-缓冲区中宽字节的长度    
	int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	// 给指向缓冲区的指针变量分配内存    
	wchar_t *pUnicode = (wchar_t*)malloc(sizeof(wchar_t)*unicodeLen);
	// 开始向缓冲区转换字节    
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, pUnicode, unicodeLen);
	wstring ret_str = pUnicode;
	free(pUnicode);
	return ret_str;
}

void CLookupSocket::SaveTempDataToMysql()
{
	if (!boTempUp)
	{
		string itable = "insert into tempdata (tvalue,tcnl,equipment_id) values";
		itable = itable + "(" + to_string(staBumMod) + ",1," + to_string(eid) + "),";//'泵速或者压力模式
		itable = itable + "(" + to_string(staBum) + ",2," + to_string(eid) + "),";//开关泵
		itable = itable + "(" + to_string(staNozVal) + ",3," + to_string(eid) + "),";//开关喷嘴
		itable = itable + "(" + to_string(staFeeVal) + ",4," + to_string(eid) + "),";//开关供墨阀
		itable = itable + "(" + to_string(staBleVal) + ",5," + to_string(eid) + "),";//开关排气阀
		itable = itable + "(" + to_string(staFluVal) + ",6," + to_string(eid) + "),";//开关清洗阀
		itable = itable + "(" + to_string(staSolVal) + ",7," + to_string(eid) + "),";//开关溶剂阀
		itable = itable + "(" + to_string(staVisVal) + ",8," + to_string(eid) + "),";//开关粘度阀
		itable = itable + "(" + to_string(staWasVal) + ",9," + to_string(eid) + "),";//开关冲洗阀
		itable = itable + "(" + to_string(staInkFloSenOff) + ",10," + to_string(eid) + "),";//关回收检测
		itable = itable + "(" + to_string(staCloInkLin) + ",11," + to_string(eid) + "),";//关闭墨线
		itable = itable + "(" + to_string(staAddSol) + ",12," + to_string(eid) + "),";//开添加溶剂
		itable = itable + "(" + to_string(staDetVis) + ",13," + to_string(eid) + "),";//开测试粘度
		itable = itable + "(" + to_string(staWasNoz) + ",14," + to_string(eid) + "),";//开冲洗喷嘴
		itable = itable + "(" + to_string(staSucNoz) + ",15," + to_string(eid) + "),";//开反吸喷嘴
		itable = itable + "(" + to_string(staAdjInkLin) + ",16," + to_string(eid) + "),";//开墨线校准
		itable = itable + "(" + to_string(staInkCir) + ",17," + to_string(eid) + "),";//开墨路循环
		itable = itable + "(" + to_string(staHigVolSwi) + ",18," + to_string(eid) + "),";//高压开关
		itable = itable + "(" + to_string(staProCou) + ",19," + to_string(eid) + "),";
		itable = itable + "(" + to_string(staPriCou) + ",20," + to_string(eid) + "),";
		boTempUp = m_sqlUser.writeDataToDB(itable);
	} 
	else
	{
		string itable = "update tempdata set tvalue = ( case";
			//when tcnl=1 then 99 when tcnl=2 then 88 end )where tcnl in (1,2) and equipment_id=1;"
		itable += " when tcnl=1 then " + to_string(staBumMod);
		itable += " when tcnl=2 then " + to_string(staBum);
		itable += " when tcnl=3 then " + to_string(staNozVal);
		itable += " when tcnl=4 then " + to_string(staFeeVal);
		itable += " when tcnl=5 then " + to_string(staBleVal);
		itable += " when tcnl=6 then " + to_string(staFluVal);
		itable += " when tcnl=7 then " + to_string(staSolVal);
		itable += " when tcnl=8 then " + to_string(staVisVal);
		itable += " when tcnl=9 then " + to_string(staWasVal);
		itable += " when tcnl=10 then " + to_string(staInkFloSenOff);
		itable += " when tcnl=11 then " + to_string(staCloInkLin);
		itable += " when tcnl=12 then " + to_string(staAddSol);
		itable += " when tcnl=13 then " + to_string(staDetVis);
		itable += " when tcnl=14 then " + to_string(staWasNoz);
		itable += " when tcnl=15 then " + to_string(staSucNoz);
		itable += " when tcnl=16 then " + to_string(staAdjInkLin);
		itable += " when tcnl=17 then " + to_string(staInkCir);
		itable += " when tcnl=18 then " + to_string(staHigVolSwi);
		itable += " when tcnl=19 then " + to_string(staProCou);
		itable += " when tcnl=20 then " + to_string(staPriCou);
		itable += " end )where tcnl in (1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20) and equipment_id=" + to_string(eid) + ";";
	}
}

void CLookupSocket::SaveCounterToMysql()
{
	if (!boCounterUP)
	{
		if (!m_counterQue.empty())//计数器
		{
			vector<BYTE> tempVec = m_counterQue.front();
			vector<int> outVec = CounterToInt(tempVec);
			string itable = "insert into tempdata (tvalue,tcnl,equipment_id) values";
			//itable = itable + "(now()," + to_string(staPrnting) + ",1," + to_string(eid) + "),";
			for (int i = 0; i < outVec.size(); i++)
			{
				if (i == outVec.size() - 1)
				{
					itable = itable + "(" + to_string(outVec[i]) + "," + to_string(21 + i) + "," + to_string(eid) + ");";
				}
				else
				{
					itable = itable + "(" + to_string(outVec[i]) + "," + to_string(21 + i) + "," + to_string(eid) + "),";
				}
			}
			boCounterUP = m_sqlUser.writeDataToDB(itable);
			m_counterQue.pop();
		}
	} 
	else
	{
		if (!m_counterQue.empty())//计数器
		{
			vector<BYTE> tempVec = m_counterQue.front();
			vector<int> outVec = CounterToInt(tempVec);
			string itable = "update tempdata set tvalue = ( case";
			//itable = itable + "(now()," + to_string(staPrnting) + ",1," + to_string(eid) + "),";
			for (int i = 0; i < outVec.size(); i++)
			{
				itable += " when tcnl=" + to_string(21 + i) + " then " + to_string(outVec[i]);
			}

			itable += " end )where tcnl in (21,22,23,24) and equipment_id="+to_string(eid)+";";
			boCounterUP = m_sqlUser.writeDataToDB(itable);
			m_counterQue.pop();
		}
	}
}
void CLookupSocket::saveDataToMySQL()
{
	string itable = "insert into data (dtime,dvalue,dcnl,equipment_id) values";
	itable = itable + "(now()," + to_string(staPrnting) + ",1,"+to_string(eid)+"),";
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
	itable = itable + "(now()," + to_string(staInkLifeTime) + ",15," + to_string(eid) + "),";//墨水时间
	itable = itable + "(now()," + to_string(staEncFre) + ",16," + to_string(eid) + "),";//编码器频率
	itable = itable + "(now()," + to_string(staPixDotNee) + ",17," + to_string(eid) + ");";//列点数
	bool testbo = m_sqlUser.writeDataToDB(itable);

	SaveTempDataToMysql();
	SaveCounterToMysql();

}

void CLookupSocket::SaveFaultToMysql()
{
	//墨水温度传感器故障
	if (staInkTemSenFau == true && staInkTemSenFauLas == false)
	{
		/*staInkTemSenFauLas = true;*/
		//staInkTemSenFauLas = true;
		//CString csMsg;
		//csMsg.Format(_T("Ink temperature sensor fault"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Yellow" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," +"'Ink temperature sensor fault'" + ",1," + to_string(eid) + ");";//列点数
		staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
	}
	else if (staInkTemSenFau == false && staInkTemSenFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Ink temperature sensor fault';";
		if(m_sqlUser.writeDataToDB(upTable))
			staInkTemSenFauLas = false;
	}

	//喷头温度传感器故障
	if (staPriHeaTemFau == true && staPriHeaTemFauLas == false)
	{
		/*	staPriHeaTemFauLas = true;*/
		//staPriHeaTemFauLas = true;
		//CString csMsg;
		//csMsg.Format(_T("Printhead temperature sensor fault"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Yellow" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Printhead temperature sensor fault'" + ",1," + to_string(eid) + ");";//列点数
		staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
	}
	else if (staPriHeaTemFau == false && staPriHeaTemFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Printhead temperature sensor fault';";
		if (m_sqlUser.writeDataToDB(upTable))
			staPriHeaTemFauLas = false;
	}

	//泵超速保护
	if (staBumSpeOveFau == true && staBumSpeOveFauLas == false)
	{
		//staBumSpeOveFauLas = true;
		//staBumSpeOveFauLas = true;
		//缺starting up

		/*m_Onoff->SetWindowText(_T("Starting Up"));*/

		//ctr0X03bit0 = 0;
		//download_inksystem_control03();
		//ctr0X00bit5 = 0;
		//ctr0X00bit3 = 0;
		//ctr0X00bit2 = 0;
		//ctr0X00bit1 = 1;
		//ctr0X00bit0 = 0;
		//download_inksystem_control00();
		//CString csMsg;
		//csMsg.Format(_T("Pump speed abnormal"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Red" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Pump speed abnormal'" + ",1," + to_string(eid) + ");";//列点数
		staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
	}
	else if (staBumSpeOveFau == false && staBumSpeOveFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Pump speed abnormal';";
		if (m_sqlUser.writeDataToDB(upTable))
			staBumSpeOveFauLas = false;
	}


	//过压保护
	if (staPreOveFau == true && staPreOveFauLas == false)
	{
		////staPreOveFauLas = true;
		//staPreOveFauLas = true;
		////缺starting up

		//ctr0X03bit0 = 0;
		//download_inksystem_control03();
		//ctr0X00bit5 = 0;
		//ctr0X00bit3 = 0;
		//ctr0X00bit2 = 0;
		//ctr0X00bit1 = 1;
		//ctr0X00bit0 = 0;
		//download_inksystem_control00();
		//CString csMsg;
		//csMsg.Format(_T("Pressure abnormal"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Red" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Pressure abnormal'" + ",1," + to_string(eid) + ");";//列点数
		staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
	}
	else if (staPreOveFau == false && staPreOveFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Pressure abnormal';";
		if (m_sqlUser.writeDataToDB(upTable))
			staPreOveFauLas = false;
	}


	//粘度异常
	if (staVisAbnFau == true && staVisAbnFauLas == false)
	{
		//staVisAbnFauLas = true;
		//staVisAbnFauLas = true;
		//CString csMsg;
		//csMsg.Format(_T("Ink Visco abnormal"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Yellow" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Ink Visco abnormal'" + ",1," + to_string(eid) + ");";//列点数
		staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
	}
	else if (staVisAbnFau == false && staVisAbnFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Ink Visco abnormal';";
		if (m_sqlUser.writeDataToDB(upTable))
			staVisAbnFauLas = false;
	}


	//粘度计故障
	if (staVisSenFau == true && staVisSenFauLas == false)
	{
		//staVisSenFauLas = true;
		//staVisSenFauLas = true;
		//CString csMsg;
		//csMsg.Format(_T("Viscometer fault"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Yellow" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Viscometer fault'" + ",1," + to_string(eid) + ");";//列点数
		staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
	}
	else if (staVisSenFau == false && staVisSenFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Viscometer fault';";
		if (m_sqlUser.writeDataToDB(upTable))
			staVisSenFauLas = false;
	}


	//回收故障
	if (staInkFloSenOff == true)
	{
		staInkFloFau = true;
		//m_Ink->GetDlgItem(IDC_INKFLOW_EDIT)->SetWindowText(_T("Disable"));
	}
	else
	{
		if (staInkFloFau == true && staInkFloFauLas == false)
		{
			////staInkFloFauLas = true;
			//staInkFloFauLas = true;
			////缺starting up

			//ctr0X03bit0 = 0;
			//download_inksystem_control03();
			//ctr0X00bit5 = 0;
			//ctr0X00bit3 = 0;
			//ctr0X00bit2 = 0;
			//ctr0X00bit1 = 1;
			//ctr0X00bit0 = 0;
			//download_inksystem_control00();
			//CString csMsg;
			//csMsg.Format(_T("Recyle fault"));
			//string m_tmpt;
			//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Red" + "               ";
			//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
			//faultList.AddString(csMsg);
			//faultList.SetFont(theApp.m_StaticFont);
			//m_Ink->GetDlgItem(IDC_INKFLOW_EDIT)->SetWindowText(_T("Abnormal"));
			string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
			itable = itable + "(now()," + "'Recyle fault'" + ",1," + to_string(eid) + ");";//列点数
			staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
		}

		else if (staInkFloFau == false)
		{
			if (staInkFloFauLas == true)
			{
				string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Recyle fault';";
				if (m_sqlUser.writeDataToDB(upTable))
					staInkFloFauLas = false;

			}
			//m_Ink->GetDlgItem(IDC_INKFLOW_EDIT)->SetWindowText(_T("Normal"));
		}
	}


	//风扇故障
	if (staFanFau == true && staFanFauLas == false)
	{
		////staFanFauLas = true;
		//staFanFauLas = true;
		////缺starting up

		//ctr0X03bit0 = 0;
		//download_inksystem_control03();
		//ctr0X00bit5 = 0;
		//ctr0X00bit3 = 0;
		//ctr0X00bit2 = 0;
		//ctr0X00bit1 = 1;
		//ctr0X00bit0 = 0;
		//download_inksystem_control00();
		//CString csMsg;
		//csMsg.Format(_T("Fan fault"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Red" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Fan fault'" + ",1," + to_string(eid) + ");";//列点数
		staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
	}
	else if (staFanFau == false && staFanFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Fan fault';";
		if (m_sqlUser.writeDataToDB(upTable))
			staFanFauLas = false;
	}


	//充电故障
	if (staChaFau == true && staChaFauLas == false)
	{
		//staChaFauLas = true;
		//staChaFauLas = true;
		//m_PicHead.SetMachineStatus(_T("充电故障"));//Charge fault

		//ctr0X03bit0 = 0;
		//download_inksystem_control03();
		//CString csMsg;
		//csMsg.Format(_T("充电故障"));//Charge fault
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Yellow" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Charge fault'" + ",1," + to_string(eid) + ");";//列点数
		staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
	}
	else if (staChaFau == false && staChaFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Charge fault';";
		if (m_sqlUser.writeDataToDB(upTable))
			staChaFauLas = false;
	}


	//相位故障
	if (staPhaFau == true && staPhaFauLas == false)
	{
		//staPhaFauLas = true;
		//staPhaFauLas = true;
		//m_PicHead.SetMachineStatus(_T("相位故障"));//Phase fault

		//ctr0X03bit0 = 0;
		//download_inksystem_control03();
		//CString csMsg;
		//csMsg.Format(_T("相位故障"));//Phase fault
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Yellow" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Phase fault'" + ",1," + to_string(eid) + ");";//列点数
		staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
	}
	else if (staPhaFau == false && staPhaFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Phase fault';";
		if (m_sqlUser.writeDataToDB(upTable))
			staPhaFauLas = false;
	}


	//高压故障
	if (staHigVolFau == true && staHigVolFauLas == false)
	{
		//staHigVolFauLas = true;
		//staHigVolFauLas = true;
		////缺starting up


		//ctr0X03bit0 = 0;
		//download_inksystem_control03();
		//ctr0X00bit5 = 0;
		//ctr0X00bit3 = 0;
		//ctr0X00bit2 = 0;
		//ctr0X00bit1 = 1;
		//ctr0X00bit0 = 0;
		//download_inksystem_control00();
		//CString csMsg;
		//csMsg.Format(_T("高压故障"));//High voltage faul
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Red" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'High voltage fault'" + ",1," + to_string(eid) + ");";//列点数
		staInkTemSenFauLas = m_sqlUser.writeDataToDB(itable);
	}
	else if (staHigVolFau == false && staHigVolFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='High voltage fault';";
		if (m_sqlUser.writeDataToDB(upTable))
			staHigVolFauLas = false;
	}


	//溶剂液位状态
	if (staSolLevFau == "00" && staSolLevFauLas != "00" && staInkLevFau == "00")
	{
		//theApp.myTimClass.staSolLevFauLas = "00";
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and (fvalue='Add solvent' or fvalue='Solvent empty' or fvalue='Solvent overfull');";
		if (m_sqlUser.writeDataToDB(upTable))
			staSolLevFauLas = "00";
		//m_PicHead.SetBlueAlarm(false);
	}
	else if (staSolLevFau == "01" && staSolLevFauLas != "01")
	{
		//theApp.myTimClass.staSolLevFauLas = "01";
		//staSolLevFauLas = "01";
		//CString csMsg;
		//csMsg.Format(_T("Add solvent"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Blue" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		///*picAlarmBlue.Tag = "im003"
		//picAlarmBlue.Image = My.Resources.ResourceBng.im003*/
		//m_PicHead.SetBlueAlarm(true);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Add solvent'" + ",1," + to_string(eid) + ");";//列点数
		if(m_sqlUser.writeDataToDB(itable))
			staSolLevFauLas = "01";
	}
	else if (staSolLevFau == "10" && staSolLevFauLas != "10")
	{
		//theApp.myTimClass.staSolLevFauLas = "10";
		//staSolLevFauLas = "10";
		//m_PicHead.SetBlueAlarm(true);
		///*picAlarmBlue.Tag = "im003"
		//picAlarmBlue.Image = My.Resources.ResourceBng.im003*/
		//CString csMsg;
		//csMsg.Format(_T("Solvent empty"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Blue" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Solvent empty'" + ",1," + to_string(eid) + ");";//列点数
		if (m_sqlUser.writeDataToDB(itable))
			staSolLevFauLas = "10";
	}
	else if (staSolLevFau == "11" && staSolLevFauLas != "11")
	{
		//theApp.myTimClass.staSolLevFauLas = "11";
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Solvent overfull'" + ",1," + to_string(eid) + ");";//列点数
		if (m_sqlUser.writeDataToDB(itable))
			staSolLevFauLas = "11";
		//m_PicHead.SetBlueAlarm(true);
		///*picAlarmBlue.Tag = "im003"
		//picAlarmBlue.Image = My.Resources.ResourceBng.im003*/
		//CString csMsg;
		//csMsg.Format(_T("Solvent overfull"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Blue" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
	}

	//墨水液位状态
	if (staInkLevFau == "00" && staInkLevFauLas != "00" && staSolLevFau == "00")
	{
		//theApp.myTimClass.staInkLevFauLas = "00";
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and (fvalue='Add ink' or fvalue='Ink empty' or fvalue='Ink overfull');";
		if (m_sqlUser.writeDataToDB(upTable))
			staInkLevFauLas = "00";
		//m_PicHead.SetBlueAlarm(false);
		///*picAlarmBlue.Tag = "im004"
		//picAlarmBlue.Image = My.Resources.ResourceBng.im004*/
		//theApp.myTimClass.staInkEmpFau = false;
		//theApp.myTimClass.staInkOveFau = false;
	}
	else if (staInkLevFau == "01" && staInkLevFauLas != "01")
	{
		//theApp.myTimClass.staInkLevFauLas = "01";
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Add ink'" + ",1," + to_string(eid) + ");";//列点数
		if (m_sqlUser.writeDataToDB(itable))
			staInkLevFauLas = "01";
		//m_PicHead.SetBlueAlarm(true);
		///*picAlarmBlue.Tag = "im003"
		//picAlarmBlue.Image = My.Resources.ResourceBng.im003*/
		//theApp.myTimClass.staInkEmpFau = false;
		//theApp.myTimClass.staInkOveFau = false;
		//CString csMsg;
		//csMsg.Format(_T("Add ink"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Blue" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
	}
	else if (staInkLevFau == "10" && staInkLevFauLas != "10")
	{
		//theApp.myTimClass.staInkLevFauLas = "10";
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Ink empty'" + ",1," + to_string(eid) + ");";//列点数
		if (m_sqlUser.writeDataToDB(itable))
			staInkLevFauLas = "10";
		//缺starting up

		//ctr0X00bit5 = 0;
		//ctr0X00bit3 = 0;
		//ctr0X00bit2 = 0;
		//ctr0X00bit1 = 1;
		//ctr0X00bit0 = 0;
		//download_inksystem_control00();
		//theApp.myTimClass.staInkEmpFau = true;
		//theApp.myTimClass.staInkOveFau = false;
		//CString csMsg;
		//csMsg.Format(_T("Ink empty"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Blue" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
	}
	else if (staInkLevFau == "11" && staInkLevFauLas != "11")
	{
		//theApp.myTimClass.staInkLevFauLas = "11";
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Ink overfull'" + ",1," + to_string(eid) + ");";//列点数
		if (m_sqlUser.writeDataToDB(itable))
			staInkLevFauLas = "11";
		//缺starting up

		//ctr0X00bit5 = 0;
		//ctr0X00bit3 = 0;
		//ctr0X00bit2 = 0;
		//ctr0X00bit1 = 1;
		//ctr0X00bit0 = 0;
		//download_inksystem_control00();
		//theApp.myTimClass.staInkOveFau = true;
		//theApp.myTimClass.staInkEmpFau = false;
		//CString csMsg;
		//csMsg.Format(_T("Ink overfull"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Blue" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);

	}

	//电眼过快
	if (staProSenFas == true && staProSenFasLas == false)
	{
		//staProSenFasLas = true;
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Much too product'" + ",1," + to_string(eid) + ");";//列点数
		if (m_sqlUser.writeDataToDB(itable))
			staProSenFasLas = true;
		//CString csMsg;
		//csMsg.Format(_T("Much too product"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Yellow" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
	}
	else if (staProSenFas == false && staProSenFasLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Much too product';";
		if (m_sqlUser.writeDataToDB(upTable))
			staProSenFasLas = false;
	}

	//自动分裂失败
	if (staAutModFau == true && staAutModFauLas == false)
	{
		//staAutModFauLas = true;
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Auto modulation fault'" + ",1," + to_string(eid) + ");";//列点数
		if (m_sqlUser.writeDataToDB(itable))
			staAutModFauLas = true;
		//m_PicHead.SetMachineStatus(_T("Fault Condition"));

		//ctr0X03bit0 = 0;
		//download_inksystem_control03();
		//CString csMsg;
		//csMsg.Format(_T("Auto modulation fault"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Yellow" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
	}
	else if (staAutModFau == false && staAutModFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Auto modulation fault';";
		if (m_sqlUser.writeDataToDB(upTable))
			staAutModFauLas = false;
	}

	//阀故障
	if (staValFau == true && staValFauLas == false)
	{
		//staValFauLas = true;
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Valve fault'" + ",1," + to_string(eid) + ");";//列点数
		if (m_sqlUser.writeDataToDB(itable))
			staValFauLas = true;
		//缺starting up

		//ctr0X03bit0 = 0;
		//download_inksystem_control03();
		//ctr0X00bit5 = 0;
		//ctr0X00bit3 = 0;
		//ctr0X00bit2 = 0;
		//ctr0X00bit1 = 1;
		//ctr0X00bit0 = 0;
		//download_inksystem_control00();
		//CString csMsg;
		//csMsg.Format(_T("Valve fault"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Red" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
	}
	else if (staValFau == false && staValFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Valve fault';";
		if (m_sqlUser.writeDataToDB(upTable))
			staValFauLas = false;
	}

	//编码器过快
	if (staLinFas == true && staLinFasLas == false)
	{
		//staLinFasLas = true;

		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Too fast'" + ",1," + to_string(eid) + ");";//列点数
		if (m_sqlUser.writeDataToDB(itable))
			staLinFasLas = true;
		//CString csMsg;
		//csMsg.Format(_T("Too fast"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Yellow" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
	}
	else if (staLinFas == false && staLinFasLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Too fast';";
		if (m_sqlUser.writeDataToDB(upTable))
			staLinFasLas = false;
	}

	//恒温故障
	if (staPriHeaHotFau == true && staPriHeaHotFauLas == false)
	{
		//staPriHeaHotFauLas = true;
		string itable = "insert into fault (ftime,fvalue,fcnl,equipment_id) values";
		itable = itable + "(now()," + "'Constant temperature fault'" + ",1," + to_string(eid) + ");";//列点数
		if (m_sqlUser.writeDataToDB(itable))
			staPriHeaHotFauLas = true;
		//CString csMsg;
		//csMsg.Format(_T("Constant temperature fault"));
		//string m_tmpt;
		//m_tmpt = m_currentDate + "               " + m_currentTime + "               " + "Yellow" + "               ";
		//csMsg = theApp.myModuleMain.string2CString(m_tmpt) + csMsg;
		//faultList.AddString(csMsg);
		//faultList.SetFont(theApp.m_StaticFont);
	}
	else if (staPriHeaHotFau == false && staPriHeaHotFauLas == true)
	{
		string upTable = "update fault set fcnl=3 where equipment_id=" + to_string(eid) + " and fvalue='Constant temperature fault';";
		if (m_sqlUser.writeDataToDB(upTable))
			staPriHeaHotFauLas = false;
	}
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
		int value = byte_arr[i]; //直接将unsigned char赋值给整型的值，系统会正动强制转换
		int v1 = value / 16;
		int v2 = value % 16;

		//将商转成字母
		if (v1 >= 0 && v1 <= 9)
			hex1 = (char)(48 + v1);
		else
			hex1 = (char)(55 + v1);

		//将余数转成字母
		if (v2 >= 0 && v2 <= 9)
			hex2 = (char)(48 + v2);
		else
			hex2 = (char)(55 + v2);

		//将字母连接成串
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

//发送cstring
BOOL CLookupSocket::SendCstring(const u_short type, const CString &strData)
{
	//ASSERT(!strData.IsEmpty());

	//int			nErrCode;				//返回值	
	//PACKETHDR	packetHdr;				//定义包头

	//packetHdr.type = type;				//类型
	//packetHdr.len = strData.GetLength();//数据长度

	////发送包头
	//nErrCode = send(m_s, (char*)&packetHdr, PACKETHDRLEN, 0);
	//if (SOCKET_ERROR == nErrCode)
	//{
	//	AfxMessageBox("发送用户列表错误！");
	//	return FALSE;
	//}
	////发送包体
	//nErrCode = send(m_s, strData, packetHdr.len, 0);
	//if (SOCKET_ERROR == nErrCode)
	//{
	//	AfxMessageBox("发送用户列表错误！");
	//	return FALSE;
	//}
	//return TRUE;

	DWORD	dwNumberOfBytesSent;	//发送数据字节数 
	DWORD	dwFlags = 0;			//标志
	int		nErrCode;				//返回值

	//if (m_strWord.IsEmpty())
	//{
	//	return TRUE;
	//}
	string fileName = UnicodeToUtf8(LPCWSTR(strData));

	WSABUF wsaSendBuf[2];
	PACKETHDR hdr;
	hdr.type = type;					//数据包类型
	hdr.len = fileName.length();	//数据长度
	wsaSendBuf[0].buf = (char*)&hdr;	//包头缓冲区
	wsaSendBuf[0].len = HEADERLEN;		//包头缓冲区长度
	//USES_CONVERSION;
	//const char* fileName = W2A(strData);//CString To ConstChar
	
	wsaSendBuf[1].buf = (CHAR*)fileName.c_str();	//包体/////有问题，需转换
	wsaSendBuf[1].len = hdr.len;		//数据长度

	nErrCode = WSASend(m_s,							//发送数据
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
			AfxMessageBox(_T("WSASend函数调用失败！"));
			return FALSE;
		}
	}
	return TRUE;
}


string CLookupSocket::UnicodeToUtf8(const wstring& wstr) {
	// 预算-缓冲区中多字节的长度    
	int ansiiLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	// 给指向缓冲区的指针变量分配内存    
	char *pAssii = (char*)malloc(sizeof(char)*ansiiLen);
	// 开始向缓冲区转换字节    
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, pAssii, ansiiLen, nullptr, nullptr);
	string ret_str = pAssii;
	free(pAssii);
	return ret_str;
}

BOOL CLookupSocket::SendFile(const u_short type, string srcFileName)
{
	DWORD	dwNumberOfBytesSent;	//发送数据字节数 
	DWORD	dwFlags = 0;			//标志
	int		nErrCode;				//返回值

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
		hdr.type = type;					//数据包类型
		hdr.len = readLen;	//数据长度
		wsaSendBuf[0].buf = (char*)&hdr;	//包头缓冲区
		wsaSendBuf[0].len = HEADERLEN;		//包头缓冲区长度
		//USES_CONVERSION;
		//const char* fileName = W2A(strData);//CString To ConstChar

		wsaSendBuf[1].buf = buffer;	//包体/////有问题，需转换
		wsaSendBuf[1].len = hdr.len;		//数据长度

		nErrCode = WSASend(m_s,							//发送数据
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
				AfxMessageBox(_T("WSASend函数调用失败！"));
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