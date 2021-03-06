
// NetClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "NetClient.h"
#include "NetClientDlg.h"
#include "afxdialogex.h"
#include <WinSock2.h>
#include "RSA.h"
#include "AES.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// 忽略4996警告
#pragma warning(disable : 4996)

#pragma warning(disable : 4086)
// 定义网络事件通知消息
#define WM_SOCKET WM_USER + 1	
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CNetClientDlg 对话框



CNetClientDlg::CNetClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_NETCLIENT_DIALOG, pParent)
	, localClientPort(_T(""))
	, sendMessageStr(_T(""))
	, clientIPAddress(_T(""))
	, udpRecvPortStr(_T(""))
	, connectClientAddressStr(_T(""))
	, connectClientPortStr(_T(""))
	, connectServerAddrStr(_T(""))
	, connectServerPortStr(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

BEGIN_MESSAGE_MAP(CNetClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SOCKET, OnServerSocket)
	ON_BN_CLICKED(IDC_BUTTON_CONNET_SERVER, &CNetClientDlg::OnBnClickedButtonConnetServer)
	ON_BN_CLICKED(IDC_BUTTON_SEND_MESSAGE, &CNetClientDlg::OnBnClickedButtonSendMessage)
	ON_BN_CLICKED(IDC_BUTTON_START_UDP_RECV_PORT, &CNetClientDlg::OnBnClickedButtonStartUdpRecvPort)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT_UDP_CLIENT, &CNetClientDlg::OnBnClickedButtonConnectUdpClient)
END_MESSAGE_MAP()


// CNetClientDlg 消息处理程序

void CNetClientDlg::DoDataExchange(CDataExchange * pDX)
{
	DDX_Text(pDX, IDC_STATIC_IP_ADDRESS, clientIPAddress);
	DDX_Control(pDX, IDC_MESSAGESLIST, messagesList);
	DDX_Text(pDX, IDC_EDIT_OWN_PORT, udpRecvPortStr);
	DDX_Text(pDX, IDC_EDIT_CLIENT_ADDRESS, connectClientAddressStr);
	DDX_Text(pDX, IDC_EDIT_CLIENT_PORT, connectClientPortStr);
	DDX_Text(pDX, IDC_EDIT_SEND_MESSAGE, sendMessageStr);
	DDX_Text(pDX, IDC_EDIT_SERVER_ADDRESS, connectServerAddrStr);
	DDX_Text(pDX, IDC_EDIT_SERVER_PORT, connectServerPortStr);
}

BOOL CNetClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	// 初始化Winsock库
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	::WSAStartup(sockVersion, &wsaData);
	connectClientSocket = INVALID_SOCKET;
	connectServerSocket = INVALID_SOCKET;
	// 下面是取得本地IP地址的过程，将它显示在状态栏的第一个分栏中
	// 取得本机名称	
	char szHost[256];
	::gethostname(szHost, 256);
	// 通过本机名称取得地址信息
	HOSTENT* pHost = gethostbyname(szHost);
	if (pHost != NULL)
	{
		CString sIP;

		// 得到第一个IP地址
		in_addr *addr = (in_addr*) *(pHost->h_addr_list);

		// 显示给用户
		sIP.Format(inet_ntoa(addr[0]));
		clientIPAddress = sIP;
		UpdateData(FALSE);
	}
	// TODO: 在此添加额外的初始化代码
	UpdateData(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CNetClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CNetClientDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CNetClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CNetClientDlg::OnBnClickedButtonConnetServer()
{
	UpdateData(TRUE);

	if (connectClientSocket == INVALID_SOCKET)	// 连接服务器
	{
		// 取得服务器地址
		if (connectServerAddrStr.IsEmpty())
		{
			MessageBox("Please Input Server Address");
			return;
		}
		// 取得端口号
		int nPort = atoi(connectServerPortStr);
		if (nPort < 1 || nPort > 65535)
		{
			MessageBox("Port Error");
			return;
		}
		// 试图连接服务器
		if (!ConnectServer(connectServerAddrStr, nPort)) 
		{
			MessageBox("Connect Server Error");
			return;
		}
		connectClientAddressStr = "";
		connectClientPortStr = "";
		RSA rsa = RSA();
		RSA_PARAM rsa_param = rsa.RsaGetParam();
		CString str;
		clientd = rsa_param.d;
		clientn = rsa_param.n;
		str.Format("%I64d", rsa_param.e);
		CString str1;
		str1.Format("%I64d", rsa_param.n);
		CString str2 = str +" "+ str1;
		if (::send(connectServerSocket, str2, str2.GetLength(), 0) != -1) {
			UpdateData(FALSE);
		}
		
	}
	else 
	{
		::closesocket(connectServerSocket);
		::closesocket(connectClientSocket);
		connectServerSocket = INVALID_SOCKET;
		connectClientSocket = INVALID_SOCKET;
		UpdateData(FALSE);
	}
}

void CNetClientDlg::OnBnClickedButtonSendMessage()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	clientkeys;
	AES aes = AES();
	aes.des_setkey((char *)clientkeys);
	char *message;
	message = sendMessageStr.GetBuffer(0);
	aes.des_run(message, message, encrypt);
	sendto(connectClientSocket, sendMessageStr, strlen(sendMessageStr), 0, (struct sockaddr*)&connectClientAddr, sizeof(sockaddr));
}

LRESULT CNetClientDlg::OnServerSocket(WPARAM wParam, LPARAM lParam)
{
	// 取得有事件发生的套节字句柄
	SOCKET s = wParam;
	// 查看是否出错
	if (WSAGETSELECTERROR(lParam))
	{
		if (connectServerSocket != SOCKET_ERROR)
			OnBnClickedButtonConnetServer();
		UpdateData(FALSE);
		return 0;
	}
	// 处理发生的事件
	switch (WSAGETSELECTEVENT(lParam))
	{
		case FD_CONNECT:	// 套节字正确的连接到服务器
		{
			// 设置用户界面
			GetDlgItem(IDC_BUTTON_CONNET_SERVER)->SetWindowText("exit Server");
			GetDlgItem(IDC_EDIT_SERVER_ADDRESS)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_SERVER_PORT)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_CLIENT_ADDRESS)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_CLIENT_PORT)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_CONNET_SERVER)->EnableWindow(TRUE);
			connectClientAddressStr = "";
			connectClientPortStr = "";
			UpdateData(FALSE);
		}
		break;

		case FD_READ:		// 套接字接受到对方发送过来的数据包
		{
			// 从服务器接受数据
			char szText[1024] = { 0 };
			::recv(s, szText, 1024, 0);
			// 显示给用户
			// 当前公钥
			CString field;

			CArray<CString, CString> v;

			int index = 0;
			// last argument is the delimitter
			while (AfxExtractSubString(field, CString(szText), index, _T(' ')))
			{
				v.Add(field);
				++index;
			}
			servere = atoll(v.GetAt(0));
			servern = atoll(v.GetAt(1));

			char *AESKEYS = "program";
			clientkeys = (unsigned char*)AESKEYS;
			unsigned __int64 pEnc[8];
			RSA rsa = RSA();
			CString RSAkeys = "encrypt";
			for (unsigned long i = 0; i < 8; i++)
			{
				unsigned long long xxx;
				int xxxx = AESKEYS[i];
				xxx = xxxx;
				pEnc[i] = rsa.PowMod(xxx, servere, servern);
				CString str1;
				str1.Format("%I64d", pEnc[i]);
				RSAkeys = RSAkeys + " " + str1;
			}
			sendto(connectClientSocket, RSAkeys, strlen(RSAkeys), 0, (struct sockaddr*)&connectClientAddr, sizeof(sockaddr));
		}
		break;

		case FD_CLOSE:
			OnBnClickedButtonConnetServer();
			break;
		}

	return 0;
}

UINT ServerRecvThread(LPVOID lpParm)
{
	CNetClientDlg *dlg = (CNetClientDlg*)lpParm;
	char gcInBuffer[1027];
	int lenth;
	int size = sizeof(sockaddr_in);
	CString strReceive, tempStr;
	bool bSendEnable = false;
	while (!dlg->m_bTerminateThread)
	{
		if ((lenth = recvfrom(dlg->udpRecvSocket, gcInBuffer, 1024, 0, (struct sockaddr *)&dlg->udpRecvAddr, &size))>0)
		{
			CString sPeerIP = ::inet_ntoa((dlg->udpRecvAddr).sin_addr);
			DWORD dwIP = ::inet_addr(sPeerIP);
			// 获取主机名称，注意其中第一个参数的转化
			hostent* pHost = ::gethostbyaddr((LPSTR)&dwIP, 4, AF_INET);
			char szHostName[256];
			strncpy(szHostName, pHost->h_name, 256);
			gcInBuffer[lenth] = '\r';
			gcInBuffer[lenth + 1] = '\n';
			gcInBuffer[lenth + 2] = '\0';
			CString str = CString(gcInBuffer);
			if (str.Find("encrypt") == -1) {
				AES aes = AES();
				char *content = "program";
				aes.des_setkey(content);
				aes.des_run(gcInBuffer, gcInBuffer, decrypt);
				CString strItem = CString(szHostName) + "[" + sPeerIP + "]: " + CString(gcInBuffer);
				dlg->messagesList.InsertString(0, strItem);
			}
			else {
				CString field;
				unsigned __int64 pEnc[8];
				CArray<CString, CString> v;
				int index = 0;
				// last argument is the delimitter
				while (AfxExtractSubString(field, CString(gcInBuffer), index, _T(' ')))
				{
					v.Add(field);
					++index;
				}
				pEnc[0] = atoll(v.GetAt(1));
				pEnc[1] = atoll(v.GetAt(2));
				pEnc[2] = atoll(v.GetAt(3));
				pEnc[3] = atoll(v.GetAt(4));
				pEnc[4] = atoll(v.GetAt(5));
				pEnc[5] = atoll(v.GetAt(6));
				pEnc[6] = atoll(v.GetAt(7));
				pEnc[7] = atoll(v.GetAt(8));
				unsigned char  pDec[8];
				RSA rsa = RSA();
				for (unsigned long i = 0; i < 8; i++)
				{
					pDec[i] = rsa.PowMod(pEnc[i], dlg->clientd, dlg->clientn);
				}
				dlg->serverkeys = pDec;
			}
		}
	}
	return 0;
}

void CNetClientDlg::OnBnClickedButtonStartUdpRecvPort()
{
	// TODO: 在此添加控件通知处理程序代码
	// TODO: 在此添加控件通知处理程序代码
	//读取服务器地址以及端口号  
	UpdateData(TRUE);
	int udpPort;
	udpPort = atoi(udpRecvPortStr);
	//MessageBox(m_strTempString);  
	if (udpPort < 1 || udpPort > 65535) {
		MessageBox("Port Error");
		return;
	}
	////socket  
	if ((udpRecvSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		MessageBox("ERROR: Create Server Socket Error!");
		return;
	}
	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(udpPort);
	(serAddr.sin_addr).s_addr = htonl(INADDR_ANY);
	if ((bind(udpRecvSocket, (LPSOCKADDR)&serAddr, sizeof(serAddr))) == SOCKET_ERROR)
	{
		MessageBox("ERROR: Bind Socket Error!");
		exit(-1);
	}
	//创建线程等待  
	m_bTerminateThread = false;
	currentThread = AfxBeginThread(ServerRecvThread, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	GetDlgItem(IDC_BUTTON_START_UDP_RECV_PORT)->SetWindowText("close Own Port");
}

void CNetClientDlg::OnBnClickedButtonConnectUdpClient()
{
	UpdateData(TRUE);
	if (connectClientSocket == INVALID_SOCKET) {
		GetDlgItem(IDC_BUTTON_CONNECTOTHERS_ADDRESS)->SetWindowText("start Port");
		// TODO: 在此添加控件通知处理程序代码
		if (connectClientAddressStr.IsEmpty())
		{
			MessageBox("Please Input Address!");
			return;
		}
		// 取得端口号
		int uPort = atoi(connectClientPortStr);
		if (uPort < 1 || uPort>65535) {
			MessageBox("Port Error！");
			return;
		}
		// TODO: 在此添加控件通知处理程序代码
		connectClientSocket = socket(AF_INET, SOCK_DGRAM, 0);
		connectClientAddr.sin_family = AF_INET;
		//MessageBox(tempStr);  
		connectClientAddr.sin_port = htons(uPort);
		connectClientAddr.sin_addr.s_addr = inet_addr(connectClientAddressStr);

		if (::send(connectServerSocket, connectClientAddressStr, connectClientAddressStr.GetLength(), 0) == -1) {
			MessageBox("Message Is Empty!");
		}
		GetDlgItem(IDC_BUTTON_CONNECTOTHERS_ADDRESS)->SetWindowText("exit Client");
	}
	else {
		GetDlgItem(IDC_BUTTON_CONNECTOTHERS_ADDRESS)->SetWindowText("connect Client");
	}
	
	UpdateData(FALSE);
}

BOOL CNetClientDlg::ConnectServer(LPCTSTR pszRemoteAddr, u_short nPort)
{
	// 创建套接字
	connectServerSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectServerSocket == INVALID_SOCKET)
	{
		return FALSE;
	}

	// 设置socket为窗口通知消息类型
	::WSAAsyncSelect(connectServerSocket, m_hWnd,
		WM_SOCKET, FD_CONNECT | FD_CLOSE | FD_WRITE | FD_READ);

	// 假定szAddr是IP地址
	ULONG uAddr = ::inet_addr(pszRemoteAddr);
	if (uAddr == INADDR_NONE)
	{
		// 不是IP地址，就认为这是主机名称
		// 从主机名取得IP地址
		hostent* pHost = ::gethostbyname(pszRemoteAddr);
		if (pHost == NULL)
		{
			::closesocket(connectServerSocket);
			connectServerSocket = INVALID_SOCKET;
			return FALSE;
		}
		// 得到以网络字节顺序排列的IP地址
		uAddr = ((struct in_addr*)*(pHost->h_addr_list))->s_addr;
	}
	// 填写服务器地址信息
	sockaddr_in remote;
	remote.sin_addr.S_un.S_addr = uAddr;
	remote.sin_family = AF_INET;
	remote.sin_port = htons(nPort);

	// 连接到远程机
	::connect(connectServerSocket, (sockaddr*)&remote, sizeof(sockaddr));

	return TRUE;
}
