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
	static	DWORD WINAPI	getstatu(void *pParam);	//�����߳�
	string DEC_to_BIN(long long Dec);
	wstring Utf8ToUnicode(const string& str);
public:
	void	HandleData(void);			//��������
	BOOL	Lookup(CString &strWord);	//��ѯ���ʺ���
	string m_filePath;					//�ļ�·��
public:
	string UnicodeToUtf8(const wstring& wstr);
	BOOL SendCstring(const u_short type, const CString &strData);//����Cstring
	BOOL SendFile(const u_short type, string srcFileName);
	BOOL SendLab(CString filePath);
	BOOL	Send(void);					//��������
	BOOL	RecvHeader(void);			//���հ�ͷ
	BOOL	RecvPacket(void);			//���հ���

	MySQLInterface m_sqlUser;		//����MySQL
	void saveDataToMySQL();			//���浽MySQL
	void SaveTempDataToMysql();		//������ʱ����
	void SaveCounterToMysql();		//�������кż�����
	void SaveFaultToMysql();			//���������Ϣ
	string staSolLevFauLas;			//�ܼ���־
	string staInkLevFauLas;			//īˮ��־
	bool boTempUp;					//�Ƿ�Ϊ����
	bool boCounterUP;				//�������Ƿ�Ϊ����
	bool isLive;						//�̻߳���
	static	DWORD WINAPI	SaveData(void *pParam);	//�����߳� 
	HANDLE m_hSaveThread;						//�����߳̾��

	char* ConvertBinaryToString(char* pBinaryData, int nLen);
	vector<int> CounterToInt(vector<BYTE>input);
private:
	CLookupSocket();
	/////////////////////////////���ݱ�����
							//�豸id
	vector<BYTE>bytStatus;
	queue<vector<BYTE>> m_dataQue;//�ɼ�����que
	queue<vector<BYTE>> m_codeQue;//��ӡ����
	queue<vector<BYTE>> m_counterQue;//������
public:
	WSAOVERLAPPED	m_oIO;		//��Ӧ�ڷ����¼����ص��ṹ
	WSAOVERLAPPED	m_iIO;      //��Ӧ�ڽ����¼����ص��ṹ


public:
//	CclientO2MDlg		*m_pClientDlg;				//������ָ��
	SOCKET			m_s;						//�׽���
	CString			m_strWord;					//����
	//CString			m_strMeaning;				//���ʺ���
	PACKETHDR		m_hdrRecv;					//�������ݰ�ͷ
	int eid;
	string lab_name;
	/////////////////////////////
public:
	bool staSysRea, staSysBus, staBumMod, staBum, staHarFin, staCleFauFin;
	bool staNozVal, staFeeVal, staBleVal, staFluVal, staSolVal, staVisVal, staWasVal;
	bool staInkFloSenOff, staCloInkLin, staAddSol, staDetVis, staWasNoz, staSucNoz, staAdjInkLin, staInkCir;
	bool staInkTemSenFau, staPriHeaTemFau, staBumSpeOveFau, staPreOveFau, staVisAbnFau, staVisSenFau, staInkFloFau, staPriHeaCle;
	bool staFanFau, staChaFau, staPhaFau;
	bool staHigVolFau;
	bool isHigh;
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
	//�Ƚ���
	int m_time;
	int staPressure1, staBumSpe1, staHigVol1, staEncFre1;
	BYTE staPriHeaTem1, staActVis1, staTarVis1, staInkLev1, staSolLev1, staPhase1;
	int staInkTem1;
	int staProCou1, staPriCou1, staInkLifeTime1;
	BYTE staPixDotNee1, staAutModVol1;
};

#endif // !defined(AFX_LOOKUPSOCKET_H__EE6B100A_876F_4784_B839_05E139095BA1__INCLUDED_)
