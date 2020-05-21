
// clientO2MDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "clientO2M.h"
#include "clientO2MDlg.h"
#include "afxdialogex.h"
#include "LookupSocket.h"
#include "ClientSocket.h"
#include <iostream>
#include <stdlib.h>
#include "time.h"
//#include "resource.h"
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CclientO2MDlg 对话框



CclientO2MDlg::CclientO2MDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENTO2M_DIALOG, pParent),
	m_bRunning(FALSE),
	m_sListen(INVALID_SOCKET)
{
	//m_sServPort = SERVERPORT;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CclientO2MDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CclientO2MDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CclientO2MDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CclientO2MDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_RESTART, &CclientO2MDlg::OnBnClickedButtonRestart)
END_MESSAGE_MAP()


// CclientO2MDlg 消息处理程序

BOOL CclientO2MDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	//int			reVal;			//返回值	

	//WSADATA wsaData;			//初始化套接字动态库	
	//if ((reVal = WSAStartup(0x0202, &wsaData)) != 0)
	//{
	//	printf("初始化套接字动态库错误%d\n", WSAGetLastError());
	//	return FALSE;
	//}
	//// TODO: 在此添加额外的初始化代码
	//initMysql();
	////m_ctlServIP.SetAddress(192, 168, 0, 100);		//服务器地址
	//m_ctlServIP.SetAddress(127, 0, 0, 1);		//服务器地址
	//if (StartServer())
	//{
	//}
	//else
	//{
	//}
	//typeBut = 0;
	//OnBnClickedButtonStart();
	typeBut = 88;
	m_pThread = AfxBeginThread((AFX_THREADPROC)ButtonThread, this);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CclientO2MDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CclientO2MDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/*
 * 初始化套接字
 */
BOOL CclientO2MDlg::InitSocket(SOCKET &tempSoc)
{


	//创建套接字
	if ((tempSoc = WSASocket(AF_INET, SOCK_STREAM, 0, NULL,
		0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("创建套接字错误%d\n", WSAGetLastError());
		WSACleanup();
		return FALSE;
	}
	return TRUE;
}

//UINT WorkerThread(LPVOID pParam)
/*
 * 工作线程
 */
DWORD WINAPI CclientO2MDlg::WorkerThread(void *pParam)
{
	EQUIPMENT *pClientDlg = (EQUIPMENT*)pParam;
	while (pClientDlg->m_bRunning)
	{

		DWORD dwIndex;				//返回值
		DWORD dwFlags;				//标志
		DWORD dwBytesTraned;		//实际传输的数据	
		LPWSAOVERLAPPED pIO = NULL;	//重叠结构指针
		//等待事件
		if ((dwIndex = WSAWaitForMultipleEvents(pClientDlg->m_nEventTotal,
			pClientDlg->m_arrEvent,
			FALSE,
			THREAD_SLEEP_TIME, TRUE))
			== WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents 失败 %d\n", WSAGetLastError());
			return 0;
		}
		if (WSA_WAIT_TIMEOUT == dwIndex)
		{
			//if (FALSE == pClientDlg->m_pLookupSocket->RecvHeader())		//接收数据包头
			//{
			//	continue;
			//}
			continue;
		}
		//重置事件
		WSAResetEvent(pClientDlg->m_arrEvent[dwIndex - WSA_WAIT_EVENT_0]);

		//pIO = pClientDlg->MapEventToOverlapped(dwIndex - WSA_WAIT_EVENT_0);
		int nIndex = dwIndex - WSA_WAIT_EVENT_0;
		if (0 == nIndex)
		{
			pIO = &pClientDlg->m_pLookupSocket->m_oIO;//发送数据操作重叠结构
		}
		else if (1 == nIndex)
		{
			pIO = &pClientDlg->m_pLookupSocket->m_iIO;//接收数据操作重叠结构
		}
		else
		{
			pIO = NULL;
		}
		ASSERT(NULL != pIO);

		//检查操作完成状态
		BOOL bRet = WSAGetOverlappedResult(pClientDlg->m_sHost,
			pIO,
			&dwBytesTraned,
			TRUE,
			&dwFlags);
		if (bRet == FALSE || dwBytesTraned == 0)//发生错误
		{
			int errInt = WSAGetLastError();
			if (errInt==995)
			{
				continue;
			}
			return -1;

		}
		else
		{
			//接收数据完成
			if (pIO == &pClientDlg->m_pLookupSocket->m_iIO)
			{
				ASSERT(1 == dwIndex);
				pClientDlg->m_pLookupSocket->HandleData();
				//pClientDlg->m_editTest.SetWindowText(pClientDlg->m_pLookupSocket->m_strMeaning);
				//pClientDlg->UpdateData(FALSE);
			}
			else
			{
				//发送数据完毕
			}
		}
	}

	return 0;
}
//接收线程
//UINT ReceiveThread(LPVOID pParam)
DWORD WINAPI CclientO2MDlg::ReceiveThread(void *pParam)
{
	EQUIPMENT *pClientDlg = (EQUIPMENT*)pParam;
		//定义读事件集合
	fd_set fdRead;
	int ret;
	//定义事件等待时间
	TIMEVAL	aTime;
	aTime.tv_sec = 1;
	aTime.tv_usec = 0;
	while (pClientDlg->m_bRunning)
	{
		//if (pClientDlg->m_pLookupSocket->RecvHeader())
		//{
		//	Sleep(2000);
		//}
		//else
		//{
		//	Sleep(1000);
		//}
				//置空fdRead事件为空
		FD_ZERO(&fdRead);
		//给客户端socket设置读事件
		FD_SET(pClientDlg->m_pLookupSocket->m_s, &fdRead);
		//调用select函数，判断是否有读事件发生
		ret = select(0, &fdRead, NULL, NULL, &aTime);

		if (ret == SOCKET_ERROR)
		{
			continue;
		}
		else if (ret > 0)
		{
			//发生读事件
			if (FD_ISSET(pClientDlg->m_pLookupSocket->m_s, &fdRead))
			{
				if (pClientDlg->m_pLookupSocket->RecvHeader())
				{
					Sleep(1);
				}
				else
				{
					pClientDlg->m_bRunning = FALSE;
				}
			}
		}
	}
	return 0;
}
/*
 * 根据序号得到WSAOVERLAPPED指针
 */
WSAOVERLAPPED * CclientO2MDlg::MapEventToOverlapped(int nIndex)
{
	//if (0 == nIndex)
	//{
	//	return &m_pLookupSocket->m_oIO;//发送数据操作重叠结构
	//}
	//else if (1 == nIndex)
	//{
	//	return &m_pLookupSocket->m_iIO;//接收数据操作重叠结构
	//}
	//else
	//{
	//	return NULL;
	//}
	return NULL;
}

void CclientO2MDlg::OnBnClickedButtonStart()
{
	//// TODO: 在此添加控件通知处理程序代码
	//int nRet;			//返回值
	//UpdateData(TRUE);	//获取数据
	//m_sHost = INVALID_SOCKET;
	//InitSocket();								//初始化套接字
	////获取服务器IP
	//DWORD dwServIP;
	//m_ctlServIP.GetAddress(dwServIP);
	//if (0 == dwServIP)
	//{
	//	AfxMessageBox(_T("请填写服务器地址。"));
	//	return;
	//}
	////服务器端口
	//if (0 == m_sServPort)
	//{
	//	AfxMessageBox(_T("请填写服务器端口。"));
	//	return;
	//}
	////服务器地址
	//SOCKADDR_IN	servAddr;
	//servAddr.sin_family = AF_INET;
	//servAddr.sin_addr.S_un.S_addr = htonl(dwServIP);
	////servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//servAddr.sin_port = htons(m_sServPort);
	//int nServLen = sizeof(servAddr);

	//nRet = connect(m_sHost, (SOCKADDR*)&servAddr, nServLen);
	//if (SOCKET_ERROR == nRet)
	//{
	//	AfxMessageBox(_T("连接服务器失败！"));
	//	return;
	//}

	////客户端运行
	//m_bRunning = TRUE;

	////创建事件对象
	//m_arrEvent[m_nEventTotal] = WSACreateEvent();//发送事件out
	//m_nEventTotal++;
	//m_arrEvent[m_nEventTotal] = WSACreateEvent();//接收事件in
	//m_nEventTotal++;
	//m_pLookupSocket = new CLookupSocket(this, m_sHost,
	//	m_arrEvent[m_nEventTotal - 2],
	//	m_arrEvent[m_nEventTotal - 1]);
	////创建工作线程
	//DWORD dwThreadId;
	//m_hWorkerThread = CreateThread(NULL, 0, WorkerThread, this, 0, &dwThreadId);
	//m_hReceiveThread= CreateThread(NULL, 0, ReceiveThread, this, 0, &dwThreadId);
	//有效状态
	//m_ctlWord.EnableWindow(TRUE);
	//(CButton*)GetDlgItem(IDC_LOOKUP_BUTTON)->EnableWindow(TRUE);
	//(CButton*)GetDlgItem(IDOK)->EnableWindow(TRUE);

	////无效状态
	//m_ctlServIP.EnableWindow(FALSE);
	//(CEdit*)GetDlgItem(IDC_CLIENT_PORT_EDIT)->EnableWindow(FALSE);
	//(CButton*)GetDlgItem(IDC_CONNECT_BUTTON)->EnableWindow(FALSE);
	if (typeBut==0)
	{
		typeBut = 1;
		m_pThread = AfxBeginThread((AFX_THREADPROC)ButtonThread, this);
	}


	int			reVal;			//返回值	

	WSADATA wsaData;			//初始化套接字动态库	
	if ((reVal = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("初始化套接字动态库错误%d\n", WSAGetLastError());
		return ;
	}
	// TODO: 在此添加额外的初始化代码
	initMysql();
	//m_ctlServIP.SetAddress(192, 168, 0, 100);		//服务器地址
	//m_ctlServIP.SetAddress(127, 0, 0, 1);		//服务器地址
	if (StartServer())
	{
	}
	else
	{
	}
	//设置启动服务器按钮无效
	(CButton*)GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	typeBut = 0;
}


void CclientO2MDlg::OnBnClickedButtonClose()
{
	// TODO: 在此添加控件通知处理程序代码
	//m_bRunning = FALSE;								//设置客户端运行状态
	//WaitForSingleObject(m_hReceiveThread, INFINITE);	//等待工作线程退出
	////WaitForSingleObject(m_hWorkerThread, INFINITE);	//等待工作线程退出
	//DWORD dwRet = 0;
	//MSG msg;
	//while (TRUE)
	//{
	//	dwRet = WaitForSingleObject(m_hWorkerThread, 50);
	//	if (dwRet== WAIT_TIMEOUT)
	//	{
	//		PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	//		continue;
	//	}
	//	else
	//	{
	//		break;
	//	}
	//}
	//delete m_pLookupSocket;							//删除CLookupSocket指针
	//m_pLookupSocket = NULL;
	//m_nEventTotal=0;

	if (!GetDlgItem(IDC_BUTTON_START)->IsWindowEnabled())
	{
		if (typeBut == 0)
		{
			typeBut = 3;
			m_pThread = AfxBeginThread((AFX_THREADPROC)ButtonThread, this);
		}
		serverRun = FALSE;
		//WaitForSingleObject(m_hFindThread, INFINITE);	//等待发现线程退出
		CloseThread(m_hFindThread);
		CloseHandle(m_hFindThread);
		for (auto iter = theApp.m_equipMap.begin(); iter != theApp.m_equipMap.end();)
		{
			if (iter->second->m_bRunning)
			{
				CloseEquip(iter->second);
			}
			delete iter->second;
			iter->second = NULL;
			iter=theApp.m_equipMap.erase(iter);
		}
		for (auto iterVec = theApp.m_deleteVec.begin(); iterVec != theApp.m_deleteVec.end();)
		{
			delete (*iterVec);
			(*iterVec) = NULL;
			iterVec = theApp.m_deleteVec.erase(iterVec);
		}
		CloseServer();									//释放套接字资源
		(CButton*)GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
		if (typeBut==3)
		{
			typeBut = 0;
		}
		
	}
}

void CclientO2MDlg::initExpert()
{
	string findStr = "SHOW DATABASES LIKE 'expert';";
	vector<vector<string>> tempVec;
	mysqlUser.getDatafromDB(findStr, tempVec);
	if (tempVec.size()>0)
	{
		return;
	}
	//建库
	string creatDb = "expert";
	string ctablestr;
	mysqlUser.createDatabase(creatDb);//创建unity数据库
	//车间表
	ctablestr = "CREATE TABLE IF NOT EXISTS expert_knowledge (id varchar(50) NOT NULL,time DATETIME NOT NULL COMMENT '时间',faultname varchar(50) NOT NULL COMMENT '错误名',faultreason varchar(500) NOT NULL COMMENT '错误原因',faultsolve varchar(500) NOT NULL COMMENT '错误解决',PRIMARY KEY(id))ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	string testinsert;

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Ink temperature sensor fault',now(),'墨水温度传感器故障','1、温度传感器没有密封好。2、焊接时将传感器焊穿','更换传感器外壳');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Printhead temperature sensor fault',now(),'喷头温度传感器故障','1、温度传感器没有密封好。2、焊接时将传感器焊穿','更换传感器外壳');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Pump speed abnormal',now(),'泵速异常','1、泵吸不上油或流量不足。2、电机故障','1、检测墨水液位，及时添加墨水。2、检测电机');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Pressure abnormal',now(),'压力异常','1、喷嘴堵住或供墨过滤器堵塞。2、主过滤器堵塞。3、减压阀不能正常工作。4、供墨泵单向阀有问题或供墨泵泄露。5、压力传感器故障','1、用溶剂或超声波来超洗喷嘴片。2、更换喷嘴体。3、更换主过滤器。4、更换减压阀。5、更换供墨单向阀或更换整体泵。6、更换新的压力传感器');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Ink Visco high',now(),'墨水粘度高','1、墨水粘度过高。2、无法添加溶剂。3、粘度计筒内壁或检测铁柱不光滑导致其下落速度不匀速','1、粘度设定值不合理，根据墨水类型，温度，断点形态来确定合理的值，进行黏度校正。2、先检查溶剂滤片是否堵塞，若堵就对其清洗或更换；再检查溶剂添加电磁阀是否不能正常工作，观察其接线"
		"是否松动或断了，或者是其阀芯粘住生锈卡了，可对其维修；最后检查溶剂泵是否正常工作，看其单向阀是不是不出溶剂了，或其溶剂柱塞是否卡了，或其溶剂泵是否漏了，可对其维修或更换；还有可能是ICU电路板出问题了，使溶剂添加电磁阀不能正常工作，对其进行更换。3、检查检测铁柱和粘度计内壁是否光洁，若检测铁柱生锈可对其适当的打磨，但不可过量打磨，否者黏度检测就不准确；对粘度计内壁进行刷洗');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Ink Visco low',now(),'墨水粘度低','1、墨水粘度值设置不合理。2、墨水箱搅拌滤头堵塞。3、墨水被稀释。4、粘度计进出口、搅拌管路各接头处、主泵接头有泄露。5、粘度计检测铁柱下落不匀速','1、根据温度、断点形态来设定墨水粘度值。2、用超声波超洗墨水箱里的搅拌滤头或更换新的。3、墨水倒出，放在通风处或者用风扇吹，"
		"使墨水中的稀释液挥发。4、 首先可能是频繁的清洗喷头，使清洗液从回收器被吸入墨水箱里或者是手动开清洗导致的，这就不要频繁的清洗喷头，若是喷头堵了，最好用超声波来超洗，不要频繁的用手动清洗、逆清洗，不然墨水就会稀了；再就检查溶剂添加电磁阀、清洗电磁阀是否泄漏或不能正常开关，若有问题就必须进行更换新的。5、 检查各处是否有漏墨，尤其是粘度计的进出口是否有漏气，必要对其进行更换。6、清洁粘度计内壁，或者更换粘度计的检测铁柱');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Viscometer fault',now(),'粘度计故障','1、粘度计进出口、搅拌管路各接头处、循环泵接头有漏气漏墨情况。2、 可能是搅拌滤片堵塞。3、可能循环泵有问题','1、顺序逐一检查其各处是否有漏气漏墨问题，对其有问题处进行更换。2、清洗或更换搅拌滤片。3、检查是不是循环泵柱塞卡了或其膜片漏了，对其进行维修或整体更换泵组件');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Recyle faul',now(),'回收故障','1、墨线位置不正确。2、压力过低。3、回收滤片堵塞或回收管路堵塞。4、回收电磁阀不能正常工作。5、回收单向阀有问题。6、回收泵膜片漏','1、调整墨线位置左右为回收器口正中间，上下为距回收器口上边缘三分之一处。2、墨压应调至40psi，此为最佳运行状态值，过低可能会影响回收效果。3、一般普通墨水很少堵回收"
		"滤片，特殊墨水容易堵，对其进行超洗或更换，把回收滤片拆掉对机器的运行状态没什么影响；若机器一段时间没有用且最后一次还没有正常关机就可能堵回收管路，可以用注射器来打通，从回收过滤器处拆掉回收管，用注射器往里打清洗剂，则会从回收器口处流出，从回收器到回收过滤器这一段管路就打通了；之后手动打开泵、回收电磁阀，用注射器往回收器口里注射清洗剂，则会从墨水箱里的回收管流出，这就整个回收管路都通了。4、一般回收电磁阀是不容易坏的，可能"
		"是其插线松动或阀芯卡了，对其进行拆洗。5、 单向阀可以用洗耳球来检测的，单向阀是下进上出的，用洗耳球对下进行吸气，对上进行吹气，若漏气则说明是坏的，对其维修或更换，最好是更换新的。6、更换膜片或更换整个泵体。7、关闭回收检测，但长时间会对机器造成影响');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Fan fault',now(),'风扇故障','1、风扇连接松动。2、风扇故障。3、电路板问题','1、拔出风扇连接线重新插入。2、更换风扇。3、联系厂家更换电路板');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Phase fault',now(),'相位故障','1、喷嘴驱动设置不正确。2、喷头后盖里进溶剂或受潮了。3、喷嘴微堵。4、喷码机没有接地，受到了干扰。5、喷头抗干扰小板有墨物或坏了。6、墨水黏度不正常或墨水过期。7、回收器、回收检测线及回收系统不好或不稳定。8、电源地线接地是否良好。9、PCU板或PDU板不正常','1、重新设置喷嘴驱动值，这得根据断点的形态"
		"来设置，一次设置在2~5的增加或减少。2、打开喷头后盖，用吹风机吹干。3、用超声波超洗喷嘴片、喷嘴滤片。4、给喷码机良好的接地，并避免或排除周围的干扰。5、可以先用溶剂擦洗抗干扰小板，再用酒精擦洗，最后用吹风机吹干，若还不行就更换新的。6、调整墨水黏度致正常值或更换新墨。7、先检查回收器与回收底座之间连接是否完好，有无漏墨现象，若有就维修或更换；可以在回收底座与喷头之间添加自行车内胎，彻底清洗回收的小白圈；超洗回收过滤器或更换；"
		"再检查回收单向阀工作是否正常，若不正常就维修或更换。8、把电源接地线良好接地。9、更换PDU板或刷改PCU板的PAU、PAU程序，若还不行更换整套的喷头喉管');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('High voltage fault',now(),'高压故障','1、喷头过于潮湿。2、高压板与零极板有墨物或其之间的距离不合适；高压板支架及高压导线有脏墨。3、喷嘴微堵。4、高压电源的高压堡灵敏度调整不合适。5、高压误报。6、高压电源与PDU板之间的4芯控制线虚接。7、高压电源坏掉。8、PDU板或PCU板坏掉','1、清洗喷头并用吹风机吹干（方喷头的可能会是后盖里进"
		"溶剂了，拆掉对其吹干，为了避免以后再进溶剂，可以对其密封巢里添加生胶带，增加密闭性）。2、调整高压板与零极板之间的距离，大概一个十字螺丝刀头直径的距离，把其放在它们之间有点松即可；彻底清洗高压板及支架、高压导线，还有零极板并且吹干。3、用超声波超洗喷嘴片及喷头滤片。4、调整高压电源的高压堡灵敏度致合适位置，调高压堡时要注意其箭头的标示，不要调反了，而且要轻微的调，调的力量过大或调的量很大，会导致高压电源的报废。5、首先可能是"
		"ICU板的芯片造成的，是其程序的问题，更换即可；或者把墨水箱上的两个黑线连接在一起试试。6、重新接好4芯控制线接头。7、先检测一下高压电源是否正常，可以拔掉高压电源的高压输出导线，然后开墨线、高压，让喷码机就绪，若还报高压故障说明高压电源坏了，更换高压电源。8、更换PDU板或PCU板');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Add solvent',now(),'添加溶剂','1、溶剂过少。2、溶剂液位传感器故障','1、添加溶剂。2、进行溶剂液位校准');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Solvent empty',now(),'溶剂空','1、溶剂已用尽。2、溶剂液位传感器故障','1、添加溶剂。2、进行溶剂液位校准');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Solvent overfull',now(),'溶剂过满','1、溶剂过多。2、溶剂液位传感器故障','1、倒出溶剂。2、进行溶剂液位校准');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Add ink',now(),'添加墨水','1、墨水过少。2、墨水液位传感器故障','1、添加墨水。2、进行墨水液位校准');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Ink empty',now(),'墨水空','1、墨水已用尽。2、墨水液位传感器故障','1、添加墨水。2、进行墨水液位校准');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Ink overfull',now(),'墨水过满','1、墨水过多。2、墨水液位传感器故障','1、倒出墨水。2、进行墨水液位校准');";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "insert into expert_knowledge (id,time,faultname,faultreason,faultsolve) values('Much too product',now(),'电眼过快','1、喷印产品速度过快。2、电眼故障','1、降低传送带速度。2、更换电眼');";
	mysqlUser.writeDataToDB(testinsert);
}
void CclientO2MDlg::initMysql()
{
	string serverName = "localhost";
	string userName = "root";
	string userKey = "root";
	string dbName = "mysql";

	mysqlUser.connectMySQL(const_cast<char *>(serverName.c_str()), const_cast<char *>(userName.c_str()), const_cast<char *>(userKey.c_str()), const_cast<char *>(dbName.c_str()), 3306);
	initExpert();//初始化专家知识库
	string creatDb = "unity";
	string ctablestr;
	mysqlUser.createDatabase(creatDb);//创建unity数据库
	//车间表

		//项目表
	ctablestr = "CREATE TABLE IF NOT EXISTS project (pro_id INT NOT NULL AUTO_INCREMENT,pro_time DATETIME NOT NULL,pro_name varchar(50) NOT NULL COMMENT '项目名',pro_explain varchar(200) NOT NULL COMMENT '项目描述',pro_file_name varchar(50) not null,PRIMARY KEY(pro_id))ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//车间
	ctablestr = "CREATE TABLE IF NOT EXISTS workshop (wid INT NOT NULL AUTO_INCREMENT,wtime DATETIME NOT NULL COMMENT '时间',wname varchar(20) NOT NULL COMMENT '车间名',wposition varchar(50) NOT NULL COMMENT '位置',PRIMARY KEY(wid))ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	string testinsert;

	//testinsert = "insert into workshop (wtime,wname,wposition) values(now(),'testworkshop','testPos');";
	//mysqlUser.writeDataToDB(testinsert);
	//生产线
	ctablestr = "CREATE TABLE IF NOT EXISTS productionline (pid INT NOT NULL AUTO_INCREMENT,ptime DATETIME NOT NULL,pname varchar(20) NOT NULL,pposition varchar(50) NOT NULL,workshop_id INT NOT NULL,PRIMARY KEY(pid),FOREIGN KEY(workshop_id) REFERENCES workshop(wid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);

	//testinsert = "insert into productionline (ptime,pname,pposition,workshop_id) values(now(),'testline','testPos',1);";
	//mysqlUser.writeDataToDB(testinsert);
	//设备表
	//cout << "对于设备表：" << endl;
	//cout << "插入数据前，先用inet_aton把ip地址转为整型，可以节省空间。例：( 2, 'babala', inet_aton( '127.0.0.1' ) )" << endl;
	//cout << "显示数据时，使用inet_ntoa把整型的ip地址转为电地址显示即可。为社么这样存 ? ，性能上的提示比直接存储字符串的IP要高出很多。读取select inet_ntoa(eip) as ip from `equipment`" << endl << endl;
	ctablestr = "CREATE TABLE IF NOT EXISTS equipment (eid INT NOT NULL AUTO_INCREMENT,etime DATETIME NOT NULL,etype varchar(20) NOT NULL COMMENT '设备类型',ename varchar(20) NOT NULL COMMENT '设备号',eposition varchar(50) NOT NULL,eip bigint(20) NOT NULL,eport INT NOT NULL,elabname varchar(30) NOT NULL,productionline_id INT NOT NULL,"
		"project_id INT NOT NULL,PRIMARY KEY(eid),FOREIGN KEY(productionline_id) REFERENCES productionline(pid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);

	//testinsert = "insert into equipment (etime,etype,ename,eposition,eip,eport,productionline_id) values(now(),'codeprint','testcodeprint','testPos',inet_aton('127.0.0.1'),8899,1);";
	//mysqlUser.writeDataToDB(testinsert);

	//设备更改表
	ctablestr = "CREATE TABLE IF NOT EXISTS equipchange (ecid INT NOT NULL AUTO_INCREMENT,eid INT NOT NULL,ectype INT NOT NULL,ectime DATETIME NOT NULL,PRIMARY KEY(ecid))ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);

	testinsert = "drop trigger if exists equip_up;";
	mysqlUser.writeDataToDB(testinsert);
	testinsert = "create trigger equip_up after update on equipment for each row insert equipchange(eid,ectype,ectime) values (new.eid,1,now());";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "drop trigger if exists equip_add;";
	mysqlUser.writeDataToDB(testinsert);
	testinsert = "create trigger equip_add after insert on equipment for each row insert equipchange(eid,ectype,ectime) values (new.eid,2,now());";
	mysqlUser.writeDataToDB(testinsert);

	testinsert = "drop trigger if exists equip_del;";
	mysqlUser.writeDataToDB(testinsert);
	testinsert = "create trigger equip_del before  delete on equipment for each row insert equipchange(eid,ectype,ectime) values (old.eid,3,now());";
	mysqlUser.writeDataToDB(testinsert);
	//采集数据表
	ctablestr = "CREATE TABLE IF NOT EXISTS data (did INT NOT NULL AUTO_INCREMENT,dtime DATETIME NOT NULL,dvalue INT NOT NULL,dcnl INT NOT NULL COMMENT '标识',equipment_id INT NOT NULL,PRIMARY KEY(did),FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	testinsert = "CREATE INDEX eid_dcnl_dtime_Index ON `data`(`equipment_id`, `dcnl`, `dtime`);";//创建组合索引
	mysqlUser.writeDataToDB(testinsert);

	//临时数据表
	ctablestr = "CREATE TABLE IF NOT EXISTS tempdata (tid INT NOT NULL AUTO_INCREMENT,tvalue INT NOT NULL,tcnl INT NOT NULL COMMENT '标识',equipment_id INT NOT NULL,PRIMARY KEY(tid),FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	testinsert = "CREATE INDEX eid_dcnl_dtime_Index ON `tempdata`(`equipment_id`, `tcnl`);";//创建组合索引
	mysqlUser.writeDataToDB(testinsert);
	//打印数据表
	//cout << "打印数据表的cprint为blob类型，保存字节" << endl << endl;
	ctablestr = "CREATE TABLE IF NOT EXISTS code (cid INT NOT NULL AUTO_INCREMENT,ctime DATETIME NOT NULL,cprint BLOB(1000) NOT NULL COMMENT '字节数据',equipment_id INT NOT NULL,PRIMARY KEY(cid),FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//testinsert = "insert into code (ctime,cprint,equipment_id) values(now(),x'018006111C070600FFFF1F244C241F00FFFF',1);";
	//mysqlUser.writeDataToDB(testinsert);
	testinsert = "CREATE INDEX eid_dtime_Index ON `code`(`equipment_id`, `ctime`);";//创建组合索引
	mysqlUser.writeDataToDB(testinsert);
	//在线监测，图片保存路径
	ctablestr = "CREATE TABLE IF NOT EXISTS monitor (mid INT NOT NULL AUTO_INCREMENT,mtime DATETIME NOT NULL,mroute varchar(100) NOT NULL COMMENT '图片路径',equipment_id INT NOT NULL,PRIMARY KEY(mid),FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//testinsert = "insert into monitor (mtime,mroute,equipment_id) values(now(),'E:\gitTEST\CodePrinter\1.jpg',1);";
	//mysqlUser.writeDataToDB(testinsert);
	//错误信息
	ctablestr = "CREATE TABLE IF NOT EXISTS fault (fid INT NOT NULL AUTO_INCREMENT,ftime DATETIME NOT NULL,fvalue varchar(100) NOT NULL COMMENT '错误信息',fcnl INT NOT NULL COMMENT '标识',equipment_id INT NOT NULL,PRIMARY KEY(fid),FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
/*	testinsert = "insert into fault (ftime,fvalue,fcnl,equipment_id) values(now(),'ttt',1,1);";
	mysqlUser.writeDataToDB(testinsert)*/;
	//权限表
	ctablestr = "CREATE TABLE IF NOT EXISTS limits (lid INT NOT NULL AUTO_INCREMENT,lname varchar(50) NOT NULL COMMENT '权限名',lexplain varchar(200) NOT NULL COMMENT '解释',PRIMARY KEY(lid))ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	testinsert = "insert limits(lid,lname,lexplain) values (1,'Admin','管理员权限，可执行系统所有操作'),(2,'Control','控制权限，可控制及查看设备，但不可管理用户及项目'),(3,'Check','仅可查看系统状态')";//创建组合索引
	mysqlUser.writeDataToDB(testinsert);
	//testinsert = "insert into limits (lname,lexplain) values('tttt','ttttt');";
	//mysqlUser.writeDataToDB(testinsert);
	//用户表
	ctablestr = "CREATE TABLE IF NOT EXISTS user (uid INT NOT NULL AUTO_INCREMENT,utime DATETIME NOT NULL,name varchar(50) NOT NULL,class varchar(50) NOT NULL,phone varchar(50) NOT NULL,email varchar(50) NOT NULL,password varchar(50) NOT NULL,limit_id INT NOT NULL,PRIMARY KEY(uid),FOREIGN KEY(limit_id) REFERENCES limits(lid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//testinsert = "insert into user (utime,uname,ukey,limit_id) values(now(),'testuser','testuser',1);";
	//mysqlUser.writeDataToDB(testinsert);

	//testinsert = "insert into abuse (user_id,equipment_id) values(1,1);";
	//mysqlUser.writeDataToDB(testinsert);
	//操作表
	ctablestr = "CREATE TABLE IF NOT EXISTS operation (oid INT NOT NULL AUTO_INCREMENT,otime DATETIME NOT NULL,oorder varchar(100) NOT NULL COMMENT '操作指令',user_id INT NOT NULL,project_id INT NOT NULL,PRIMARY KEY(oid),FOREIGN KEY(user_id) REFERENCES user(uid) on delete cascade on update cascade,FOREIGN KEY(project_id) REFERENCES project(pro_id) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);

	//ctablestr = "CREATE TABLE IF NOT EXISTS quality (qid varchar(20) NOT NULL,qvalue varchar(1000) NOT NULL COMMENT 'unity属性',PRIMARY KEY(qid),FOREIGN KEY(qid) REFERENCES user(uid) on delete cascade on update cascade,FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	//mysqlUser.createdbTable(ctablestr);


	//虚拟设备表
	ctablestr = "CREATE TABLE IF NOT EXISTS virtual_device (vd_id INT NOT NULL,vd_time DATETIME NOT NULL,vd_name varchar(50) NOT NULL COMMENT '虚拟设备名',vd_property varchar(1000) NOT NULL COMMENT '属性列表',PRIMARY KEY(vd_id))ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//项目设备
	ctablestr = "CREATE TABLE IF NOT EXISTS pro_equip (pe_id INT NOT NULL AUTO_INCREMENT,pe_time DATETIME NOT NULL,equipment_id INT NOT NULL COMMENT '物理设备id',vir_dev_id INT NOT NULL COMMENT '虚拟设备id',project_id INT NOT NULL COMMENT '项目id',PRIMARY KEY(pe_id),FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade,"
		"FOREIGN KEY(vir_dev_id) REFERENCES virtual_device(vd_id) on delete cascade on update cascade,FOREIGN KEY(project_id) REFERENCES project(pro_id) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//项目清单
	ctablestr = "CREATE TABLE IF NOT EXISTS project_list (pl_id INT NOT NULL AUTO_INCREMENT,user_id INT NOT NULL,project_id INT NOT NULL,PRIMARY KEY(pl_id),FOREIGN KEY(user_id) REFERENCES user(uid) on delete cascade on update cascade,FOREIGN KEY(project_id) REFERENCES project(pro_id) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);

	//设备unity属性表
	ctablestr = "CREATE TABLE IF NOT EXISTS unity_equip (ue_id varchar(20) NOT NULL,pe_time DATETIME NOT NULL,ue_property varchar(1000) NOT NULL COMMENT '属性列表',PRIMARY KEY(ue_id))ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);

	testinsert = "insert into unity_equip (ue_id,pe_time,ue_property) values('CodePrint',now(),'Pressure|Pump speed|Ink temperature|Nozzle temperature|Ink level|Solvent level|Target viscosity|Actual viscosity|High voltage|Modulation voltage|Phase|Ink time|Encoder frequency|Number of columns|Code byte|"
		"Pump speed or pressure mode|Switch pump|Switch nozzle|Ink supply valve|Vent valve|Cleaning valve|Solvent valve|Viscosity valve|Flush valve|Close recycling detection|Close ink line|Add solvent|Test viscosity|Flushing nozzle|The reverse suction nozzle|Ink line calibration|Ink road cycle|High voltage switch|Product testing|Product spray printing|Counter|Switch printing');";
	mysqlUser.writeDataToDB(testinsert);
	//电眼
	testinsert = "insert into unity_equip (ue_id,pe_time,ue_property) values('OptoelectronicSwitch',now(),'Eye level');";
	mysqlUser.writeDataToDB(testinsert);
	//删除30天前数据
	string deleteM = "delete from data where dtime < date_add(curdate(), INTERVAL - 1 month);";
	mysqlUser.createdbTable(deleteM);
	//删除30天前数据
	deleteM = "delete from code where ctime < date_add(curdate(), INTERVAL - 1 month);";
	mysqlUser.createdbTable(deleteM);

	//创建储存过程
	//string sqlStr;
	//sqlStr = "source E:\\gitTEST\\server\\clientO2M\\x64\\Debug\\test.sql;";
	//mysqlUser.writeDataToDB(sqlStr);
	//string saveFun;
	//saveFun = "drop procedure if exists 压力;drop procedure if exists 泵速;drop procedure if exists 墨水温度;drop procedure if exists 喷头温度;drop procedure if exists 墨水液位;";//delimiter $$ create procedure test (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=5); end$$ delimiter ;";
	//saveFun += "drop procedure if exists 溶剂液位;drop procedure if exists 目标粘度;drop procedure if exists 实际粘度;drop procedure if exists 高压;drop procedure if exists 调制电压;";
	//saveFun += "drop procedure if exists 相位;drop procedure if exists 墨水时间;drop procedure if exists 编码器频率;drop procedure if exists 列点数;";
	//saveFun = "drop procedure if exists 压力;";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "delimiter  $$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 压力 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=1); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 泵速 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=2); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 墨水温度 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=3); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 喷头温度 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=4); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 墨水液位 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=5); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 溶剂液位 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=6); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 目标粘度 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=7); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 实际粘度 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=8); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 高压 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=9); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 调制电压 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=10); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 相位 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=11); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 墨水时间 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=12); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 编码器频率 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=13); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "create procedure 列点数 (in eid int) begin select dvalue from data where did in (select max(did) from data where equipment_id=eid and dcnl=14); end$$";
	//mysqlUser.writeDataToDB(saveFun);
	//saveFun = "delimiter ;";
	//mysqlUser.writeDataToDB(saveFun);
//读取设备信息
	ctablestr = "select eid,etype,inet_ntoa(eip) as ip,eport from `equipment`;";
	vector<vector<string>> tempVec;
	if (mysqlUser.getDatafromDB(ctablestr, tempVec))
	{
		for (auto iter1=tempVec.begin();iter1!=tempVec.end();iter1++)
		{
			if ((*iter1)[1]=="CodePrint")
			{
				PEQUIPMENT tempEquip=new EQUIPMENT;						//不知道为啥用指针才行
				tempEquip->m_id = atoi(((*iter1)[0]).c_str());
				tempEquip->m_ip = (*iter1)[2];
				tempEquip->m_sServPort= atoi(((*iter1)[3]).c_str());
				tempEquip->m_bRunning=FALSE;
				tempEquip->m_pLookupSocket=NULL;
				tempEquip->m_nEventTotal=0;
				tempEquip->connetTimes = 0;
				if (StartEquip(tempEquip))
				{
					CString fuckstr = _T("fuck");
					tempEquip->m_pLookupSocket->Lookup(fuckstr);
					theApp.m_equipMap.insert(make_pair(tempEquip->m_id, tempEquip));
				}
				else
				{
					theApp.m_deleteVec.push_back(tempEquip);
				}
			}
		}
	}
	DWORD dwThreadId;//select MAX(id) 
	ctablestr = "select MAX(id) from `equipchange` ;";
	vector<vector<string>> tempcVec;
	if (mysqlUser.getDatafromDB(ctablestr, tempcVec))
	{
		changeId = atoi(((*tempcVec.begin())[0]).c_str());
	}
	else
	{
		changeId = 0;
	}
	serverRun = TRUE;
	m_hFindThread = CreateThread(NULL, 0, FindThread, this, 0, &dwThreadId);
}


//wstring CclientO2MDlg::Utf8ToUnicode(const string& str) {
//	// 预算-缓冲区中宽字节的长度    
//	int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
//	// 给指向缓冲区的指针变量分配内存    
//	wchar_t *pUnicode = (wchar_t*)malloc(sizeof(wchar_t)*unicodeLen);
//	// 开始向缓冲区转换字节    
//	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, pUnicode, unicodeLen);
//	wstring ret_str = pUnicode;
//	free(pUnicode);
//	return ret_str;
//}

bool CclientO2MDlg::StartEquip(PEQUIPMENT tempEquip)
{
	int nRet;			//返回值
	tempEquip->m_sHost = INVALID_SOCKET;
	InitSocket(tempEquip->m_sHost);								//初始化套接字
	//服务器地址
	SOCKADDR_IN	servAddr;
	servAddr.sin_family = AF_INET;
	//servAddr.sin_addr.S_un.S_addr = htonl(dwServIP);
	servAddr.sin_addr.S_un.S_addr = inet_addr(tempEquip->m_ip.c_str());
	servAddr.sin_port = htons(tempEquip->m_sServPort);
	int nServLen = sizeof(servAddr);

	//nRet = connect(tempEquip->m_sHost, (SOCKADDR*)&servAddr, nServLen);
		// 设置为非阻塞的socket  
	int iMode = 1;
	ioctlsocket(tempEquip->m_sHost, FIONBIO, (u_long FAR*)&iMode);


	//// 定义服务端  
	////SOCKADDR_IN addrSrv;
	////addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	////addrSrv.sin_family = AF_INET;
	////addrSrv.sin_port = htons(8888);


	//// 超时时间  
	struct timeval tm;
	tm.tv_sec = 5;
	tm.tv_usec = 0;
	int ret = -1;


	//// 尝试去连接服务端  
	if (-1 != connect(tempEquip->m_sHost, (SOCKADDR*)&servAddr, sizeof(SOCKADDR)))
	{
		ret = 1; // 连接成功  
	}
	else
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(tempEquip->m_sHost, &set);

		if (select(-1, NULL, &set, NULL, &tm) <= 0)
		{
			ret = -1; // 有错误(select错误或者超时)  
		}
		else
		{
			int error = -1;
			int optLen = sizeof(int);
			getsockopt(tempEquip->m_sHost, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen);

			// 之所以下面的程序不写成三目运算符的形式， 是为了更直观， 便于注释  
			if (0 != error)
			{
				ret = -1; // 有错误  
			}
			else
			{
				ret = 1;  // 无错误  
			}
		}
	}


	//// 设回为阻塞socket  
	iMode = 0;
	ioctlsocket(tempEquip->m_sHost, FIONBIO, (u_long FAR*)&iMode); //设置为阻塞模式 
	if (SOCKET_ERROR == ret)
	{
		//AfxMessageBox(_T("连接服务器失败！"));
		return false;
	}

	//if (SOCKET_ERROR == nRet)
	//{
	//	//AfxMessageBox(_T("连接服务器失败！"));
	//	return false;
	//}

	//客户端运行
	tempEquip->m_bRunning = TRUE;
	tempEquip->connetTimes = 0;
	//创建事件对象
	tempEquip->m_arrEvent[tempEquip->m_nEventTotal] = WSACreateEvent();//发送事件out
	tempEquip->m_nEventTotal++;
	tempEquip->m_arrEvent[tempEquip->m_nEventTotal] = WSACreateEvent();//接收事件in
	tempEquip->m_nEventTotal++;
	tempEquip->m_pLookupSocket = new CLookupSocket(tempEquip->m_sHost,
		tempEquip->m_arrEvent[tempEquip->m_nEventTotal - 2],
		tempEquip->m_arrEvent[tempEquip->m_nEventTotal - 1],
		tempEquip->m_id);
	//创建工作线程
	DWORD dwThreadId;
	tempEquip->m_hWorkerThread = CreateThread(NULL, 0, WorkerThread, (void*)tempEquip, 0, &dwThreadId);
	tempEquip->m_hReceiveThread = CreateThread(NULL, 0, ReceiveThread, (void*)tempEquip, 0, &dwThreadId);
	return true;
}

bool CclientO2MDlg::CloseEquip(PEQUIPMENT tempEquip)
{
	tempEquip->m_bRunning = FALSE;								//设置客户端运行状态
	//WaitForSingleObject(tempEquip->m_hReceiveThread, INFINITE);	//等待工作线程退出
	////WaitForSingleObject(m_hWorkerThread, INFINITE);	//等待工作线程退出
	//DWORD dwRet = 0;
	//MSG msg;
	//while (TRUE)
	//{
	//	dwRet = WaitForSingleObject(tempEquip->m_hWorkerThread, 50);
	//	if (dwRet == WAIT_TIMEOUT)
	//	{
	//		PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	//		continue;
	//	}
	//	else
	//	{
	//		break;
	//	}
	//}
	CloseThread(tempEquip->m_hReceiveThread);
	CloseHandle(tempEquip->m_hReceiveThread);
	CloseThread(tempEquip->m_hWorkerThread);
	CloseHandle(tempEquip->m_hWorkerThread);
	delete tempEquip->m_pLookupSocket;							//删除CLookupSocket指针
	tempEquip->m_pLookupSocket = NULL;
	tempEquip->m_nEventTotal = 0;
	//WSACleanup();									//释放套接字资源
	return true;
}

DWORD WINAPI CclientO2MDlg::FindThread(void *pParam)
{
	CclientO2MDlg *pClientDlg = (CclientO2MDlg*)pParam;
	while (pClientDlg->serverRun)
	{
		//掉线处理
		for (auto iter=theApp.m_equipMap.begin();iter!= theApp.m_equipMap.end();)
		{
			if (!iter->second->m_bRunning)
			{
				if (pClientDlg->CloseEquip(iter->second))
				{
					theApp.m_deleteVec.push_back(iter->second);
					iter = theApp.m_equipMap.erase(iter);
				}
				else
				{
					iter++;
				}
			}
			else
			{
				iter++;
			}
		}
		//重连
		for (auto iterVec= theApp.m_deleteVec.begin();iterVec!= theApp.m_deleteVec.end();)
		{
			if(pClientDlg->StartEquip(*iterVec))
			{
				CString fuckstr = _T("request");
				(*iterVec)->m_pLookupSocket->Lookup(fuckstr);
				theApp.m_equipMap.insert(make_pair((*iterVec)->m_id, *iterVec));
				iterVec = theApp.m_deleteVec.erase(iterVec);
			}
			else
			{
				if ((++(*iterVec)->connetTimes) > 0)
				{
					string ctablestr = "select ename from `equipment` where eid =" + to_string((*iterVec)->m_id) + ";";
					vector<vector<string>> tempcVec;
					if (pClientDlg->mysqlUser.getDatafromDB(ctablestr, tempcVec))
					{
						//changeId = atoi(((*tempcVec.begin())[0]).c_str());
						string testinsert = "insert into fault (ftime,fvalue,fcnl,equipment_id) values(now(),'连接设备" + ((*tempcVec.begin())[0]) + "失败！',1," + to_string((*iterVec)->m_id) + ");";
						pClientDlg->mysqlUser.writeDataToDB(testinsert);
					}
					delete (*iterVec);
					(*iterVec) = NULL;
					iterVec = theApp.m_deleteVec.erase(iterVec);
				}
				else
				{
					iterVec++;
				}
			}
		}
		//设备表更改
		string ctablestr = "select id,eid,ectype from `equipchange` where id > ";
		ctablestr += to_string(pClientDlg->changeId) + ";";
		vector<vector<string>> tempVec;
		if (pClientDlg->mysqlUser.getDatafromDB(ctablestr, tempVec))
		{
			for (auto iter1 = tempVec.begin(); iter1 != tempVec.end(); iter1++)
			{
				pClientDlg->changeId = atoi((*iter1)[0].c_str());
				ctablestr = "select eid,etype,inet_ntoa(eip) as ip,eport from `equipment` where eid = "+(*iter1)[1]+";";
				vector<vector<string>> oneVec;
				pClientDlg->mysqlUser.getDatafromDB(ctablestr, oneVec);
				switch (atoi((*iter1)[2].c_str()))
				{
				case 1://更新
				{
					auto upIter = theApp.m_equipMap.find(atoi((*iter1)[1].c_str()));
					auto oneIter = oneVec.begin();
					if (upIter != theApp.m_equipMap.end())
					{
						if (upIter->second->m_ip!= (*oneIter)[2]|| upIter->second->m_sServPort!= atoi(((*oneIter)[3]).c_str()))
						{
							upIter->second->m_bRunning = FALSE;
							if (pClientDlg->CloseEquip(upIter->second))
							{
								delete upIter->second;
								upIter->second = NULL;
								theApp.m_equipMap.erase(upIter);
							}
							if ((*oneIter)[1] == "CodePrint")
							{
								PEQUIPMENT tempEquip = new EQUIPMENT;						//不知道为啥用指针才行
								tempEquip->m_id = atoi(((*oneIter)[0]).c_str());
								tempEquip->m_ip = (*oneIter)[2];
								tempEquip->m_sServPort = atoi(((*oneIter)[3]).c_str());
								tempEquip->m_bRunning = FALSE;
								tempEquip->m_pLookupSocket = NULL;
								tempEquip->m_nEventTotal = 0;
								//pClientDlg->StartEquip(tempEquip);
								//CString fuckstr = _T("fuck");
								//tempEquip->m_pLookupSocket->Lookup(fuckstr);
								//theApp.m_equipMap.insert(make_pair(tempEquip->m_id, tempEquip));
								if (pClientDlg->StartEquip(tempEquip))
								{
									CString fuckstr = _T("fuck");
									tempEquip->m_pLookupSocket->Lookup(fuckstr);
									theApp.m_equipMap.insert(make_pair(tempEquip->m_id, tempEquip));
								}
								else
								{
									theApp.m_deleteVec.push_back(tempEquip);
								}
							}
						}
					}
					else
					{
						if ((*oneIter)[1] == "CodePrint")
						{
							PEQUIPMENT tempEquip = new EQUIPMENT;						//不知道为啥用指针才行
							tempEquip->m_id = atoi(((*oneIter)[0]).c_str());
							tempEquip->m_ip = (*oneIter)[2];
							tempEquip->m_sServPort = atoi(((*oneIter)[3]).c_str());
							tempEquip->m_bRunning = FALSE;
							tempEquip->m_pLookupSocket = NULL;
							tempEquip->m_nEventTotal = 0;
							//pClientDlg->StartEquip(tempEquip);
							//CString fuckstr = _T("fuck");
							//tempEquip->m_pLookupSocket->Lookup(fuckstr);
							//theApp.m_equipMap.insert(make_pair(tempEquip->m_id, tempEquip));
							if (pClientDlg->StartEquip(tempEquip))
							{
								CString fuckstr = _T("fuck");
								tempEquip->m_pLookupSocket->Lookup(fuckstr);
								theApp.m_equipMap.insert(make_pair(tempEquip->m_id, tempEquip));
								for (auto detIter = theApp.m_deleteVec.begin(); detIter != theApp.m_deleteVec.end(); detIter++)
								{
									if ((*detIter)->m_id == atoi((*iter1)[2].c_str()))
									{
										delete (*detIter);
										(*detIter) = NULL;
										theApp.m_deleteVec.erase(detIter);
										break;
									}
								}
							}
							else
							{
								theApp.m_deleteVec.push_back(tempEquip);
							}
						}
					}
				}
				case 2://插入
				{
					auto oneIter = oneVec.begin();
					if (oneIter == oneVec.end())
					{
						break;
					}
					if ((*oneIter)[1] == "CodePrint")
					{
						PEQUIPMENT tempEquip = new EQUIPMENT;						//不知道为啥用指针才行
						tempEquip->m_id = atoi(((*oneIter)[0]).c_str());
						tempEquip->m_ip = (*oneIter)[2];
						tempEquip->m_sServPort = atoi(((*oneIter)[3]).c_str());
						tempEquip->m_bRunning = FALSE;
						tempEquip->m_pLookupSocket = NULL;
						tempEquip->m_nEventTotal = 0;
						//pClientDlg->StartEquip(tempEquip);
						//CString fuckstr = _T("fuck");
						//tempEquip->m_pLookupSocket->Lookup(fuckstr);
						//theApp.m_equipMap.insert(make_pair(tempEquip->m_id, tempEquip));
						if (pClientDlg->StartEquip(tempEquip))
						{
							CString fuckstr = _T("fuck");
							tempEquip->m_pLookupSocket->Lookup(fuckstr);
							theApp.m_equipMap.insert(make_pair(tempEquip->m_id, tempEquip));
						}
						else
						{
							theApp.m_deleteVec.push_back(tempEquip);
						}
					}

					break;
				}
				case 3://删除
				{
					auto delIter = theApp.m_equipMap.find(atoi((*iter1)[1].c_str()));
					if (delIter != theApp.m_equipMap.end())
					{
						delIter->second->m_bRunning = FALSE;
						if (pClientDlg->CloseEquip(delIter->second))
						{
							delete delIter->second;
							delIter->second = NULL;
							theApp.m_equipMap.erase(delIter);
						}
					}
					else
					{
						for (auto iterVec = theApp.m_deleteVec.begin(); iterVec != theApp.m_deleteVec.end(); iterVec++)
						{
							if ((*iterVec)->m_id == atoi((*iter1)[1].c_str()))
							{
								delete (*iterVec);
								(*iterVec) = NULL;
								theApp.m_deleteVec.erase(iterVec);
								break;
							}
						}
					}
					break;
				}
				default:
					break;
				}
			}
		}
		Sleep(1000);
	}
	return 0;
}

BOOL CclientO2MDlg::InitServerSocket(void)
{
	int			reVal;			//返回值
	WSADATA wsaData;			//初始化套接字动态库	
	if ((reVal = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("初始化套接字动态库错误%d\n", WSAGetLastError());
		return FALSE;
	}

	//创建监听套接字
	if ((m_sListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL,
		0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("创建监听套接字错误%d\n", WSAGetLastError());
		WSACleanup();
		return FALSE;
	}
	return TRUE;
}

DWORD WINAPI CclientO2MDlg::ServiceThread(void *pParam)
{
	PTHREADPARAM	pThreadParam = (PTHREADPARAM)pParam;
	HWND			hwnd = pThreadParam->hServHwnd;		//主窗口句柄
	CclientO2MDlg		*pServer = pThreadParam->pServView;	//CServerView类指针

	SOCKET sAccept = INVALID_SOCKET;					//接受套接字
	SOCKET sListen = pServer->m_sListen;				//监听套接字	
	CObList clientList;									//客户端链表

	while (pServer->m_bRunning)
	{
		//接受客户端连接请求
		SOCKADDR addr;
		int len = sizeof(addr);
		sAccept = accept(sListen, (SOCKADDR*)&addr, &len);
		if (INVALID_SOCKET != sAccept)
		{
			//创建客户端对象
			CClientSocket * pClientS = new CClientSocket(sAccept, hwnd);
			if (FALSE == pClientS->RecvHeader())//接收数据
			{
				//发生错误
				delete pClientS;
				pClientS = NULL;
			}
			clientList.AddTail(pClientS);
		}

		//睡眠
		//Sleep(THREAD_SLEEP_TIME);
		DWORD  dwRet = SleepEx(THREAD_SLEEP_TIME, TRUE);
		if (WAIT_IO_COMPLETION == dwRet)//完成例程被调用
		{
			int ss = 1;
			ss++;
		}
		else if (0 == dwRet)			//函数调用超时
		{
			//清理资源
			POSITION pos1 = NULL;
			POSITION pos2 = NULL;
			for (pos1 = clientList.GetHeadPosition(); (pos2 = pos1) != NULL;)
			{
				CClientSocket *pClientS = (CClientSocket*)clientList.GetNext(pos1);
				if (TRUE == pClientS->m_bDead)	//与客户端的连接已经端口
				{
					Sleep(200);
					clientList.RemoveAt(pos2);	//删除节点
					delete pClientS;			//删除指针
					pClientS = NULL;
				}
			}
		}
	}

	//服务器线程退出时，清理资源。删除客户端链表的所用节点。
	POSITION pos = NULL;
	for (pos = clientList.GetHeadPosition(); pos != NULL;)
	{
		CClientSocket *pClientS = (CClientSocket*)clientList.GetNext(pos);
		delete pClientS;
		pClientS = NULL;
	}
	clientList.RemoveAll();

	delete pThreadParam;//删除线程参数
	pThreadParam = NULL;
	return 0;
}

BOOL CclientO2MDlg::StartServer()
{
	SOCKADDR_IN	servAddr;		//服务器地址
	InitServerSocket();
	//绑定套接字
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(SERVERPORT);
	if (bind(m_sListen, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		printf("绑定套接字错误 %d\n", WSAGetLastError());
		closesocket(m_sListen);
	//	WSACleanup();
		return FALSE;
	}

	//监听
	if (listen(m_sListen, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("绑定套接字错误 %d\n", WSAGetLastError());
		closesocket(m_sListen);
	//	WSACleanup();
		return FALSE;
	}

	//设置服务器为运行状态
	m_bRunning = TRUE;

	//设置套结字为非阻塞模式
	u_long ulUnBlocking = 1;
	int ret = ioctlsocket(m_sListen, FIONBIO, &ulUnBlocking);
	if (SOCKET_ERROR == ret)
	{
		printf("ioctlsocket %d\n", WSAGetLastError());
		closesocket(m_sListen);
	//	WSACleanup();
		return FALSE;
	}

	//创建线程参数
	PTHREADPARAM pParam = new THREADPARAM;
	pParam->hServHwnd = this->GetSafeHwnd();
	pParam->pServView = this;

	//创建服务线程
	DWORD dwThreadId;
	m_hThreadService = CreateThread(NULL, 0, ServiceThread, pParam, 0, &dwThreadId);
	return TRUE;
}

BOOL CclientO2MDlg::CloseServer()
{
	m_bRunning = FALSE;								//设置服务器运行状态

	//WaitForSingleObject(m_hThreadService, INFINITE);//等待服务线程退出m_hFindThread
	CloseThread(m_hThreadService);
	CloseHandle(m_hThreadService);					//释放资源

	closesocket(m_sListen);							//关闭监听套接字
	WSACleanup();									//释放套接字资源
	return TRUE;
}

BOOL CclientO2MDlg::CloseThread(HANDLE tempHandle)
{
	DWORD dwRet = 0;
	MSG msg;
	while (TRUE)
	{
		//wait for m_hThread to be over，and wait for
		//QS_ALLINPUT（Any message is in the queue）
		dwRet = MsgWaitForMultipleObjects(1, &tempHandle, FALSE, INFINITE, QS_ALLINPUT);
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
	//CloseHandle(*tempHandle);
	return TRUE;
}

void CclientO2MDlg::OnBnClickedButtonRestart()
{
	// TODO: 在此添加控件通知处理程序代码
	if (typeBut == 0)
	{
		typeBut = 2;
		m_pThread = AfxBeginThread((AFX_THREADPROC)ButtonThread, this);
	}
	OnBnClickedButtonClose();
	OnBnClickedButtonStart();
	typeBut = 0;
}

UINT CclientO2MDlg::ButtonThread(LPVOID *pParam)
{

	CclientO2MDlg* tempDlg = (CclientO2MDlg*)pParam;
	CString tempStr = _T("STARTING");
	int timeRe = 0;
	switch (tempDlg->typeBut)
	{
	case 0:
		//tempDlg->GetDlgItem(IDC_STATIC_TABLE)->SetWindowText(_T("STARTING"));
		//::SetWindowText(::GetDlgItem(tempDlg->m_hWnd, IDC_STATIC_TABLE), _T("STARTING"));
		//tempStr = _T("STARTING");
		//while (tempDlg->GetDlgItem(IDC_BUTTON_START)->IsWindowEnabled())
		//{
		//	if ((timeRe++)<5)
		//	{
		//		tempStr += _T(".");
		//	}
		//	else
		//	{ 
		//		timeRe = 0;
		//		tempStr = _T("STARTING");
		//	}
		//	::SetWindowText(::GetDlgItem(tempDlg->m_hWnd, IDC_STATIC_TABLE), tempStr);
		//	Sleep(50);
		//}
		//::SetWindowText(::GetDlgItem(tempDlg->m_hWnd, IDC_STATIC_TABLE), L"RUNNING");
		break;
	
	case 1:
		tempStr = _T("STARTING");
		while (tempDlg->typeBut == 1)
		{
			if ((timeRe++) < 5)
			{
				tempStr += _T(".");
			}
			else
			{
				timeRe = 0;
				tempStr = _T("STARTING");
			}
			::SetWindowText(::GetDlgItem(tempDlg->m_hWnd, IDC_STATIC_TABLE), tempStr);
			Sleep(50);
		}
		::SetWindowText(::GetDlgItem(tempDlg->m_hWnd, IDC_STATIC_TABLE), L"RUNNING");
		break;
	case 2:
		tempStr = _T("RESTARTING");
		while (tempDlg->typeBut == 2)
		{
			if ((timeRe++) < 5)
			{
				tempStr += _T(".");
			}
			else
			{
				timeRe = 0;
				tempStr = _T("RESTARTING");
			}
			::SetWindowText(::GetDlgItem(tempDlg->m_hWnd, IDC_STATIC_TABLE), tempStr);
			Sleep(50);
		}
		::SetWindowText(::GetDlgItem(tempDlg->m_hWnd, IDC_STATIC_TABLE), L"RUNNING");
		break;
	case 3:
		tempStr = _T("CLOSING");
		while (tempDlg->typeBut == 3)
		{
			if ((timeRe++) < 5)
			{
				tempStr += _T(".");
			}
			else
			{
				timeRe = 0;
				tempStr = _T("CLOSING");
			}
			::SetWindowText(::GetDlgItem(tempDlg->m_hWnd, IDC_STATIC_TABLE), tempStr);
			Sleep(50);
		}
		::SetWindowText(::GetDlgItem(tempDlg->m_hWnd, IDC_STATIC_TABLE), L"CLOSE");
		break;
	default:
		tempDlg->typeBut = 0;
		tempDlg->OnBnClickedButtonStart();
		break;
	}
	//tempDlg->typeBut = 0;
	return 0;
}