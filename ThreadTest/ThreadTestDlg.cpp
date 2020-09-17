
// ThreadTestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ThreadTest.h"
#include "ThreadTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CThreadTestDlg �Ի���



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


HANDLE semp[2];//������A ����4�������� //[0] = B,[1] = A
CRITICAL_SECTION m_Section;//��Դ���䣬�����߻�������
HANDLE c2[2];//������֪ͨ������A //[0] = B,[1] = A
HWND hwnd;
HANDLE hThread[6];

// CThreadTestDlg ��Ϣ�������

BOOL CThreadTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	hwnd = this->GetSafeHwnd();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CThreadTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
	dwHwndThreadID = GetWindowThreadProcessId(hWnd, &dwHwndProcessID);		//��ȡ���������Ľ��̺��߳�ID

	if (dwHwndProcessID != GetCurrentProcessId())		//���ڽ��̲��ǵ�ǰ���ý���ʱ������ԭ������
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}

	//���ڽ����ǵ�ǰ����ʱ��
	if (dwHwndThreadID == GetCurrentThreadId())			//�����߳̾��ǵ�ǰ�����̣߳�����ԭ������
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
		//�õ������ö���������
		WaitForSingleObject(c2[6 - nEdit], INFINITE);
		//��������������Ҫ����ö�����
		EnterCriticalSection(&m_Section);
		if (nResources >= strlen(szResources))
		{
			LeaveCriticalSection(&m_Section);//��ռ����Դ��
			break;//��Դ��û�ж�����
		}
		//ȡ����Դ
		sprintf_s(sz, MAX_PATH, "%c", szResources[nResources]);
		nResources++;
		LeaveCriticalSection(&m_Section);//��ռ����Դ��
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
		//�õ���������Դ������
		//�ж���A���ź� ���� B���ź�
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
	//������������������
	//4����������
	//2 - 4 ͬ��
	//2 / 4 ����
	//����������Դ���ж������������ö��������궫��֮��֪ͨ������
	//������A����Դ���ö�����ʱ��������B�Ͳ�Ӧ�����ˣ���Ȼ�����õ�ͬһ��������
	//����һ���������ж�����ʱ�򣬶���֪ͨ4�������߿�ʼ�������Ӧ�þ����ź��������˰ɡ�
	//�������ߵĶ�����������֮�󣬲�����Դ����������ö����������߾���Ҫ֪ͨ�����ߣ����Ӧ�����¼�����
	nResources = 0;
	HWND hEdit = ::GetDlgItem(hwnd, IDC_EDIT1);
	::GetWindowText(hEdit, szResources, MAX_PATH);
	InitializeCriticalSection(&m_Section);
	c2[0] = CreateEvent(
		NULL,   // ��ȫ����
		FALSE,   // ��λ��ʽ
		FALSE,   // ��ʼ״̬
		"eb"   // ��������
	);
	c2[1] = CreateEvent(
		NULL,   // ��ȫ����
		FALSE,   // ��λ��ʽ
		FALSE,   // ��ʼ״̬
		"ea"   // ��������
	);
	semp[0] = CreateSemaphore(NULL          //�ź����İ�ȫ����
		, 0            //�����ź����ĳ�ʼ�������������㵽���ֵ֮���һ��ֵ
		, 4            //�����ź�����������
		, "sb"         //ָ���ź������������
	);
	semp[1] = CreateSemaphore(NULL          //�ź����İ�ȫ����
		, 0            //�����ź����ĳ�ʼ�������������㵽���ֵ֮���һ��ֵ
		, 4            //�����ź�����������
		, "sa"         //ָ���ź������������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	HANDLE h = CreateThread(NULL, 0, WorkThread,NULL, 0, NULL);
}

