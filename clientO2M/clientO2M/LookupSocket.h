// LookupSocket.h: interface for the CLookupSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOOKUPSOCKET_H__EE6B100A_876F_4784_B839_05E139095BA1__INCLUDED_)
#define AFX_LOOKUPSOCKET_H__EE6B100A_876F_4784_B839_05E139095BA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <queue>
#include <iostream>
#include "MySQLInterface.h"
using namespace std;
//class CclientO2MDlg;
//struct PACKETHDR;
class CLookupSocket : public CObject  
{
public:
	//CLookupSocket(CclientO2MDlg *pDlg, SOCKET s, WSAEVENT hEventOut, WSAEVENT hEventIn);
	CLookupSocket(SOCKET s, WSAEVENT hEventOut, WSAEVENT hEventIn,int equipId);
	virtual ~CLookupSocket();
	CString GETnBIT_from_bytStatus(int I, int m, int n);
	static	DWORD WINAPI	getstatu(void *pParam);	//保存线程
	string DEC_to_BIN(long long Dec);
	wstring Utf8ToUnicode(const string& str);
public:
	void	HandleData(void);			//处理数据
	BOOL	Lookup(CString &strWord);	//查询单词含义
	string m_filePath;					//文件路径
public:
	string UnicodeToUtf8(const wstring& wstr);
	BOOL SendCstring(const u_short type, const CString &strData);//发送Cstring
	BOOL SendFile(const u_short type, string srcFileName);
	BOOL SendLab(CString filePath);
	BOOL	Send(void);					//发送数据
	BOOL	RecvHeader(void);			//接收包头
	BOOL	RecvPacket(void);			//接收包体

	MySQLInterface m_sqlUser;		//访问MySQL
	void saveDataToMySQL();			//保存到MySQL
	bool isLive;						//线程活着
	static	DWORD WINAPI	SaveData(void *pParam);	//保存线程 
	HANDLE m_hSaveThread;						//保存线程句柄

	char* ConvertBinaryToString(char* pBinaryData, int nLen);
	vector<int> CounterToInt(vector<BYTE>input);
private:
	CLookupSocket();
	/////////////////////////////数据保存用
							//设备id
	vector<BYTE>bytStatus;
	queue<vector<BYTE>> m_dataQue;//采集数据que
	queue<vector<BYTE>> m_codeQue;//打印数据
	queue<vector<BYTE>> m_counterQue;//计数器
public:
	WSAOVERLAPPED	m_oIO;		//对应于发送事件的重叠结构
	WSAOVERLAPPED	m_iIO;      //对应于接收事件的重叠结构


public:
//	CclientO2MDlg		*m_pClientDlg;				//主窗口指针
	SOCKET			m_s;						//套接字
	CString			m_strWord;					//单词
	CString			m_strMeaning;				//单词含义
	PACKETHDR		m_hdrRecv;					//接收数据包头
	int eid;
	/////////////////////////////
public:
	bool staSysRea, staSysBus, staBumMod, staBum, staHarFin, staCleFauFin;
	bool staNozVal, staFeeVal, staBleVal, staFluVal, staSolVal, staVisVal, staWasVal;
	bool staInkFloSenOff, staCloInkLin, staAddSol, staDetVis, staWasNoz, staSucNoz, staAdjInkLin, staInkCir;
	bool staInkTemSenFau, staPriHeaTemFau, staBumSpeOveFau, staPreOveFau, staVisAbnFau, staVisSenFau, staInkFloFau, staPriHeaCle;
	bool staFanFau, staChaFau, staPhaFau;
	bool staHigVolFau;
	string staSolLevFau, staInkLevFau, staRFID;
	bool staPrnting, staHigVolSwi, staActProSen, staProSenFas, staAutModFau, staValFau;
	bool staPrinted, staRemPrinSwi, staBufFul, staBufRea, staEncDir, staLinFas, staPriHeaHot, staPriHeaHotFau, staSetTimeEna;
	int staPressure, staBumSpe, staHigVol, staEncFre;
	BYTE staPriHeaTem, staActVis, staTarVis, staInkLev, staSolLev, staPhase;
	int staInkTem;
	int staProCou, staPriCou, staInkLifeTime;
	BYTE staPixDotNee, staAutModVol;
	bool staFanFauLas, staChaFauLas, staPhaFauLas, staHigVolFauLas;
	bool staProSenFasLas, staAutModFauLas, staValFauLas, staLinFasLas, staPriHeaHotFauLas;
	bool staInkTemSenFauLas, staPriHeaTemFauLas, staBumSpeOveFauLas, staPreOveFauLas, staVisAbnFauLas, staVisSenFauLas, staInkFloFauLas;
};

#endif // !defined(AFX_LOOKUPSOCKET_H__EE6B100A_876F_4784_B839_05E139095BA1__INCLUDED_)
