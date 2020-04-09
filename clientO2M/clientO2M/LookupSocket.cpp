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
	DWORD dwThreadId;
	PTHECLIENT pParam = new THECLIENT;
	//pParam->hServHwnd = this->GetSafeHwnd();
	pParam->pClient = this;
	m_hSaveThread = CreateThread(NULL, 0, SaveData, pParam, 0, &dwThreadId);
}
CLookupSocket::~CLookupSocket()
{
	isLive = false;
	WaitForSingleObject(m_hSaveThread, INFINITE);	//等待工作线程退出
	closesocket(m_s);
	WSACloseEvent(m_iIO.hEvent);
	WSACloseEvent(m_oIO.hEvent);
	m_s = INVALID_SOCKET;
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
		m_strMeaning.ReleaseBuffer();
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
	
	m_strMeaning.Empty();

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

//保存数据//可能会有多线程操作MySQL的坑
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

			if (!pClientS->m_counterQue.empty())//计数器
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
	itable = itable + "(now()," + to_string(staInkLifeTime) + ",15," + to_string(eid) + "),";//墨水时间
	itable = itable + "(now()," + to_string(staEncFre) + ",16," + to_string(eid) + "),";//编码器频率
	itable = itable + "(now()," + to_string(staPixDotNee) + ",17," + to_string(eid) + ");";//列点数
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