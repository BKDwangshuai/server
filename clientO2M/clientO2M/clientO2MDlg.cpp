
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
			theApp.m_equipMap.erase(iter);
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


void CclientO2MDlg::initMysql()
{
	string serverName = "localhost";
	string userName = "root";
	string userKey = "root";
	string dbName = "mysql";

	mysqlUser.connectMySQL(const_cast<char *>(serverName.c_str()), const_cast<char *>(userName.c_str()), const_cast<char *>(userKey.c_str()), const_cast<char *>(dbName.c_str()), 3306);
	string creatDb = "unity";
	string ctablestr;
	mysqlUser.createDatabase(creatDb);//创建unity数据库
	//车间表
	ctablestr = "CREATE TABLE IF NOT EXISTS workshop (wid INT NOT NULL AUTO_INCREMENT,wtime DATETIME NOT NULL COMMENT '时间',wname varchar(20) DEFAULT NULL COMMENT '车间名',wposition varchar(50) DEFAULT NULL COMMENT '位置',PRIMARY KEY(wid))ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	string testinsert;

	//testinsert = "insert into workshop (wtime,wname,wposition) values(now(),'testworkshop','testPos');";
	//mysqlUser.writeDataToDB(testinsert);
	//生产线
	ctablestr = "CREATE TABLE IF NOT EXISTS productionline (pid INT NOT NULL AUTO_INCREMENT,ptime DATETIME NOT NULL,pname varchar(20) DEFAULT NULL,pposition varchar(50) DEFAULT NULL,workshop_id INT NOT NULL,PRIMARY KEY(pid),FOREIGN KEY(workshop_id) REFERENCES workshop(wid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);

	//testinsert = "insert into productionline (ptime,pname,pposition,workshop_id) values(now(),'testline','testPos',1);";
	//mysqlUser.writeDataToDB(testinsert);
	//设备表
	//cout << "对于设备表：" << endl;
	//cout << "插入数据前，先用inet_aton把ip地址转为整型，可以节省空间。例：( 2, 'babala', inet_aton( '127.0.0.1' ) )" << endl;
	//cout << "显示数据时，使用inet_ntoa把整型的ip地址转为电地址显示即可。为社么这样存 ? ，性能上的提示比直接存储字符串的IP要高出很多。读取select inet_ntoa(eip) as ip from `equipment`" << endl << endl;
	ctablestr = "CREATE TABLE IF NOT EXISTS equipment (eid INT NOT NULL AUTO_INCREMENT,etime DATETIME NOT NULL,etype varchar(20) DEFAULT NULL COMMENT '设备类型',ename varchar(20) DEFAULT NULL COMMENT '设备号',eposition varchar(50) DEFAULT NULL,eip bigint(20) DEFAULT NULL,eport INT DEFAULT NULL,productionline_id INT NOT NULL,PRIMARY KEY(eid),FOREIGN KEY(productionline_id) REFERENCES productionline(pid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);

	//testinsert = "insert into equipment (etime,etype,ename,eposition,eip,eport,productionline_id) values(now(),'codeprint','testcodeprint','testPos',inet_aton('127.0.0.1'),8899,1);";
	//mysqlUser.writeDataToDB(testinsert);

	//设备更改表
	ctablestr = "CREATE TABLE IF NOT EXISTS equipchange (id INT NOT NULL AUTO_INCREMENT,eid INT NOT NULL,ectype INT NOT NULL,ectime DATETIME NOT NULL,PRIMARY KEY(id))ENGINE=InnoDB DEFAULT CHARSET=utf8;";
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
	ctablestr = "CREATE TABLE IF NOT EXISTS tempdata (tid INT NOT NULL AUTO_INCREMENT,tvalue INT NOT NULL,tcnl INT NOT NULL COMMENT '标识',equipment_id INT NOT NULL,PRIMARY KEY(did),FOREIGN KEY(equipment_id) REFERENCES equipment(tid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	testinsert = "CREATE INDEX eid_dcnl_dtime_Index ON `tempdata`(`equipment_id`, `tcnl`);";//创建组合索引
	mysqlUser.writeDataToDB(testinsert);
	//打印数据表
	//cout << "打印数据表的cprint为blob类型，保存字节" << endl << endl;
	ctablestr = "CREATE TABLE IF NOT EXISTS code (cid INT NOT NULL AUTO_INCREMENT,ctime DATETIME NOT NULL,cprint BLOB(1000) DEFAULT NULL COMMENT '字节数据',equipment_id INT NOT NULL,PRIMARY KEY(cid),FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//testinsert = "insert into code (ctime,cprint,equipment_id) values(now(),x'018006111C070600FFFF1F244C241F00FFFF',1);";
	//mysqlUser.writeDataToDB(testinsert);
	testinsert = "CREATE INDEX eid_dtime_Index ON `code`(`equipment_id`, `ctime`);";//创建组合索引
	mysqlUser.writeDataToDB(testinsert);
	//在线监测，图片保存路径
	ctablestr = "CREATE TABLE IF NOT EXISTS monitor (mid INT NOT NULL AUTO_INCREMENT,mtime DATETIME NOT NULL,mroute varchar(100) DEFAULT NULL COMMENT '图片路径',equipment_id INT NOT NULL,PRIMARY KEY(mid),FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//testinsert = "insert into monitor (mtime,mroute,equipment_id) values(now(),'E:\gitTEST\CodePrinter\1.jpg',1);";
	//mysqlUser.writeDataToDB(testinsert);
	//错误信息
	ctablestr = "CREATE TABLE IF NOT EXISTS fault (fid INT NOT NULL AUTO_INCREMENT,ftime DATETIME NOT NULL,fvalue varchar(100) DEFAULT NULL COMMENT '错误信息',fcnl INT NOT NULL COMMENT '标识',equipment_id INT NOT NULL,PRIMARY KEY(fid),FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
/*	testinsert = "insert into fault (ftime,fvalue,fcnl,equipment_id) values(now(),'ttt',1,1);";
	mysqlUser.writeDataToDB(testinsert)*/;
	//权限表
	ctablestr = "CREATE TABLE IF NOT EXISTS limits (lid INT NOT NULL AUTO_INCREMENT,lname varchar(20) DEFAULT NULL COMMENT '权限名',lexplain varchar(100) DEFAULT NULL COMMENT '解释',PRIMARY KEY(lid))ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//testinsert = "insert into limits (lname,lexplain) values('tttt','ttttt');";
	//mysqlUser.writeDataToDB(testinsert);
	//用户表
	ctablestr = "CREATE TABLE IF NOT EXISTS user (uid INT NOT NULL AUTO_INCREMENT,utime DATETIME NOT NULL,uname varchar(20) DEFAULT NULL,ukey varchar(20) DEFAULT NULL,limit_id INT NOT NULL,PRIMARY KEY(uid),FOREIGN KEY(limit_id) REFERENCES limits(lid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//testinsert = "insert into user (utime,uname,ukey,limit_id) values(now(),'testuser','testuser',1);";
	//mysqlUser.writeDataToDB(testinsert);
	//用户权限使用表
	ctablestr = "CREATE TABLE IF NOT EXISTS abuse (aid INT NOT NULL AUTO_INCREMENT,user_id INT NOT NULL,equipment_id INT NOT NULL,PRIMARY KEY(aid),FOREIGN KEY(user_id) REFERENCES user(uid) on delete cascade on update cascade,FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);
	//testinsert = "insert into abuse (user_id,equipment_id) values(1,1);";
	//mysqlUser.writeDataToDB(testinsert);
	//操作表
	ctablestr = "CREATE TABLE IF NOT EXISTS operation (oid INT NOT NULL AUTO_INCREMENT,otime DATETIME NOT NULL,order varchar(100) NOT NULL COMMENT '操作指令',user_id INT NOT NULL,equipment_id INT NOT NULL,PRIMARY KEY(oid),FOREIGN KEY(user_id) REFERENCES user(uid) on delete cascade on update cascade,FOREIGN KEY(equipment_id) REFERENCES equipment(eid) on delete cascade on update cascade)ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	mysqlUser.createdbTable(ctablestr);

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


	// 定义服务端  
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(8888);


	// 超时时间  
	struct timeval tm;
	tm.tv_sec = 5;
	tm.tv_usec = 0;
	int ret = -1;


	// 尝试去连接服务端  
	if (-1 != connect(tempEquip->m_sHost, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)))
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


	// 设回为阻塞socket  
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

				switch (atoi((*iter1)[2].c_str()))
				{
				case 1://更新
				{
					auto upIter = theApp.m_equipMap.find(atoi((*iter1)[1].c_str()));
					if (upIter != theApp.m_equipMap.end())
					{
						upIter->second->m_bRunning = FALSE;
						if (pClientDlg->CloseEquip(upIter->second))
						{
							delete upIter->second;
							upIter->second = NULL;
							theApp.m_equipMap.erase(upIter);
						}
					}
				}
				case 2://插入
				{
					auto oneIter = oneVec.begin();
					if (oneIter == oneVec.end())
					{
						continue;
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

					for (auto detIter= theApp.m_deleteVec.begin();detIter!=theApp.m_deleteVec.end();detIter++)
					{
						if ((*detIter)->m_id== atoi((*iter1)[2].c_str()))
						{
							delete (*detIter);
							(*detIter) = NULL;
							theApp.m_deleteVec.erase(detIter);
							break;
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