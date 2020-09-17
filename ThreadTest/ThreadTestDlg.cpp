
// ThreadTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ThreadTest.h"
#include "ThreadTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CThreadTestDlg 对话框



CThreadTestDlg::CThreadTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_THREADTEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CThreadTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CThreadTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CThreadTestDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()

int nResources;
char szResources[MAX_PATH] = { 0 };


HANDLE semp[2];//生产者A 控制4个消费者 //[0] = B,[1] = A
CRITICAL_SECTION m_Section;//资源分配，生产者互相争抢
HANDLE c2[2];//消费者通知生产者A //[0] = B,[1] = A
HWND hwnd;
HANDLE hThread[6];

// CThreadTestDlg 消息处理程序

BOOL CThreadTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	// TODO: 在此添加额外的初始化代码
	hwnd = this->GetSafeHwnd();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CThreadTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CThreadTestDlg::OnPaint()
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
HCURSOR CThreadTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


int GetWindowTextSafe(HWND hWnd, LPTSTR lpString, int nMaxCount)
{
	if (NULL == hWnd || FALSE == IsWindow(hWnd) || NULL == lpString || 0 == nMaxCount)
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}
	DWORD dwHwndProcessID = 0;
	DWORD dwHwndThreadID = 0;
	dwHwndThreadID = GetWindowThreadProcessId(hWnd, &dwHwndProcessID);		//获取窗口所属的进程和线程ID

	if (dwHwndProcessID != GetCurrentProcessId())		//窗口进程不是当前调用进程时，返回原本调用
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}

	//窗口进程是当前进程时：
	if (dwHwndThreadID == GetCurrentThreadId())			//窗口线程就是当前调用线程，返回原本调用
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}

#ifndef _UNICODE
	WCHAR *lpStringUnicode = new WCHAR[nMaxCount];
	InternalGetWindowText(hWnd, lpStringUnicode, nMaxCount);
	int size = WideCharToMultiByte(CP_ACP, 0, lpStringUnicode, -1, NULL, 0, NULL, NULL);
	if (size <= nMaxCount)
	{
		size = WideCharToMultiByte(CP_ACP, 0, lpStringUnicode, -1, lpString, size, NULL, NULL);
		if (NULL != lpStringUnicode)
		{
			delete[]lpStringUnicode;
			lpStringUnicode = NULL;
		}
		return size;
	}
	if (NULL != lpStringUnicode)
	{
		delete[]lpStringUnicode;
		lpStringUnicode = NULL;
	}
	return 0;

#else
	return InternalGetWindowText(hWnd, lpString, nMaxCount);
#endif
}


DWORD WINAPI Producer(LPVOID lp)
{
	char sz[MAX_PATH] = { 0 };
	DWORD nEdit = (DWORD)lp;
	while (true)
	{
		//得到可以拿东西的令牌
		WaitForSingleObject(c2[6 - nEdit], INFINITE);
		//告诉其它的我需要这个拿东西了
		EnterCriticalSection(&m_Section);
		if (nResources >= strlen(szResources))
		{
			LeaveCriticalSection(&m_Section);//不占用资源池
			break;//资源池没有东西了
		}
		//取出资源
		sprintf_s(sz, MAX_PATH, "%c", szResources[nResources]);
		nResources++;
		LeaveCriticalSection(&m_Section);//不占用资源池
		HWND hEdit = GetDlgItem(hwnd, IDC_EDIT1 + nEdit);
		Sleep(500);
		SetWindowText(hEdit, sz);
		ReleaseSemaphore(semp[6 - nEdit], 1, NULL);
	}
	return 0;
}
DWORD WINAPI Consumer(LPVOID lp)
{
	char sz[MAX_PATH] = { 0 };
	DWORD nEdit = (DWORD)lp;
	char sznow[MAX_PATH] = { 0 };
	while (true)
	{
		//得到可以抢资源的令牌
		//判断是A的信号 还是 B的信号
		int nIndex = WaitForMultipleObjects(2, semp, FALSE, INFINITE);
		if (nIndex == WAIT_OBJECT_0 + 1)
		{
			HWND hEdit = GetDlgItem(hwnd, IDC_EDIT1 + nEdit);
			HWND hEdit1 = GetDlgItem(hwnd, IDC_EDIT_A);
			GetWindowText(hEdit1, sznow, MAX_PATH);
			strcat_s(sz, sznow);
			Sleep(1000);
			SetWindowText(hEdit1, "0");
			SetWindowText(hEdit, sz);
			SetEvent(c2[1]);
		}
		else if (nIndex == WAIT_OBJECT_0)
		{
			HWND hEdit = GetDlgItem(hwnd, IDC_EDIT1 + nEdit);
			HWND hEdit1 = GetDlgItem(hwnd, IDC_EDIT_B);
			GetWindowText(hEdit1, sznow, MAX_PATH);
			strcat_s(sz, sznow);
			Sleep(1500);
			SetWindowText(hEdit1, "0");
			SetWindowText(hEdit, sz);
			SetEvent(c2[0]);
		}
	}
	return 0;
}

DWORD WINAPI WorkThread(LPVOID lp)
{
	//两个缓冲区是生产者
	//4个是消费者
	//2 - 4 同步
	//2 / 4 互斥
	//输入数后，资源池有东西，生产者拿东西，拿完东西之后，通知消费者
	//生产者A向资源池拿东西的时候，生产者B就不应该拿了，不然可能拿到同一个东西，
	//任意一个生产者有东西的时候，都会通知4个消费者开始抢。这个应该就用信号量控制了吧。
	//当生产者的东西被消费了之后，才在资源池里面继续拿东西，消费者就需要通知生产者，这个应该用事件控制
	nResources = 0;
	HWND hEdit = ::GetDlgItem(hwnd, IDC_EDIT1);
	::GetWindowText(hEdit, szResources, MAX_PATH);
	InitializeCriticalSection(&m_Section);
	c2[0] = CreateEvent(
		NULL,   // 安全属性
		FALSE,   // 复位方式
		FALSE,   // 初始状态
		"eb"   // 对象名称
	);
	c2[1] = CreateEvent(
		NULL,   // 安全属性
		FALSE,   // 复位方式
		FALSE,   // 初始状态
		"ea"   // 对象名称
	);
	semp[0] = CreateSemaphore(NULL          //信号量的安全特性
		, 0            //设置信号量的初始计数。可设置零到最大值之间的一个值
		, 4            //设置信号量的最大计数
		, "sb"         //指定信号量对象的名称
	);
	semp[1] = CreateSemaphore(NULL          //信号量的安全特性
		, 0            //设置信号量的初始计数。可设置零到最大值之间的一个值
		, 4            //设置信号量的最大计数
		, "sa"         //指定信号量对象的名称
	);
	hThread[0] = CreateThread(NULL, 0, Producer, (LPVOID)5, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, Producer, (LPVOID)6, 0, NULL);

	hThread[2] = CreateThread(NULL, 0, Consumer, (LPVOID)1, 0, NULL);
	hThread[3] = CreateThread(NULL, 0, Consumer, (LPVOID)2, 0, NULL);
	hThread[4] = CreateThread(NULL, 0, Consumer, (LPVOID)3, 0, NULL);
	hThread[5] = CreateThread(NULL, 0, Consumer, (LPVOID)4, 0, NULL);

	SetEvent(c2[0]);
	SetEvent(c2[1]);

	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	TerminateThread(hThread[2], -1);
	TerminateThread(hThread[3], -1);
	TerminateThread(hThread[4], -1);
	TerminateThread(hThread[5], -1);

	WaitForMultipleObjects(6, hThread, TRUE, INFINITE);
	DeleteCriticalSection(&m_Section);
	return 0;
}

void CThreadTestDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	HANDLE h = CreateThread(NULL, 0, WorkThread,NULL, 0, NULL);
}

