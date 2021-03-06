
// NetServerDlg.h: 头文件
//

#pragma once
#include "afxwin.h"
#include <map>
#include <WinSock2.h>
#include <cstring>
using namespace std;
#define MAX_SOCKET 56	// 定义此服务器所能接受的最大客户量

// CNetServerDlg 对话框
class CNetServerDlg : public CDialog
{
	// 构造
	public:
		CNetServerDlg(CWnd* pParent = NULL);	// 标准构造函数
		
		CString serverPortStr;
		
		map<CString, CString> messagesMap;

		sockaddr_in m_clientAddr;
		// 若为true则终止线程
		bool m_bTerminateThread;
		SOCKET recieveSocket;
		// 客户连接列表
		SOCKET client[MAX_SOCKET];	// 套节字数组
		int clientcount;			// 上述数组的大小
		CWinThread* currentThread;

		afx_msg void OnBnClickedButtonStartServer();

		BOOL CloseAllSocket();

		BOOL CreateAndListen(int nPort);
		

	// 对话框数据
	#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_NETSERVER_DIALOG };
	#endif

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


	// 实现
	protected:
		HICON m_hIcon;

		// 生成的消息映射函数
		virtual BOOL OnInitDialog();
		afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
		afx_msg void OnPaint();
		afx_msg HCURSOR OnQueryDragIcon();
		DECLARE_MESSAGE_MAP();
public:
	CListBox clientkeys;
	CString iPAddressStr;
	afx_msg void OnEnChangeEditServerPort();
	LRESULT OnSocket(WPARAM wParam, LPARAM lParam);
	BOOL AddClient(SOCKET s);
	void RemoveClient(SOCKET s);
};
