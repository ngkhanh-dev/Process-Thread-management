
// MFCApplication2Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication2.h"
#include "MFCApplication2Dlg.h"
#include "afxdialogex.h"
#include "Windows.h"
#include "tlhelp32.h"
#include "iostream"
#include "string"
#include "vector"
#include "thread"
#include "queue"
#include "condition_variable"
#include "functional"
#include "atomic"
#include "afxwin.h"
#include "atlstr.h"
#include "utility"
#include "cstdlib"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

vector<int> m_checkboxStates;

// Logic
bool checkNotExistProcessId(int x, const vector<int>& a) {
	for (int id : a) {
		if (id == x) return false;
	}
	return true;
}

class ThreadPool {
public:
	ThreadPool(size_t numThreads);
	~ThreadPool();
	void enqueueTask(const function<void()>& task);

private:
	vector<thread> workers;
	queue<function<void()>> tasks;
	mutex queueMutex;
	condition_variable condition;
	atomic<bool> stop;
	void workerThread();
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
	for (size_t i = 0; i < numThreads; ++i) {
		workers.emplace_back(&ThreadPool::workerThread, this);
	}
}

ThreadPool::~ThreadPool() {
	stop.store(true);
	condition.notify_all();
	for (thread& worker : workers) {
		worker.join();
	}
}

void ThreadPool::enqueueTask(const function<void()>& task) {
	{
		unique_lock<mutex> lock(queueMutex);
		tasks.push(task);
	}
	condition.notify_one();
}

void ThreadPool::workerThread() {
	while (!stop.load()) {
		function<void()> task;
		{
			unique_lock<mutex> lock(queueMutex);
			condition.wait(lock, [this] { return stop.load() || !tasks.empty(); });
			if (stop.load() && tasks.empty())
				return;
			task = move(tasks.front());
			tasks.pop();
		}
		task();
	}
}

class LightweightProcess {
public:
	LightweightProcess(int id);
	~LightweightProcess();
	void createThread(const function<void()>& entry);

private:
	int id;
	ThreadPool pool;
};

LightweightProcess::LightweightProcess(int id) : id(id), pool(4) {}

LightweightProcess::~LightweightProcess() {}

void LightweightProcess::createThread(const function<void()>& entry) {
	pool.enqueueTask(entry);
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLvnItemchangedTable(NMHDR* pNMHDR, LRESULT* pResult)
	{
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

		if (pNMListView->uChanged & LVIF_STATE)
		{
			if ((pNMListView->uNewState & LVIS_STATEIMAGEMASK) != (pNMListView->uOldState & LVIS_STATEIMAGEMASK))
			{
				// Checkbox state has changed
				CListCtrl table;
				int nItemCount = table.GetItemCount();
				for (int i = 0; i < nItemCount; ++i)
				{
					if (table.GetCheck(i))
					{
						// Checkbox at index i is checked
						// Perform your logic here
					}
				}
			}
		}

		*pResult = 0;
	}
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TABLE, &CAboutDlg::OnLvnItemchangedTable)
END_MESSAGE_MAP()


// CMFCApplication2Dlg dialog



CMFCApplication2Dlg::CMFCApplication2Dlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MFCAPPLICATION2_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_NEW, btn_new);
	DDX_Control(pDX, IDC_TEXT_EDIT, text_edit);
	DDX_Control(pDX, IDC_BTN_TABLE, btn_table);
	DDX_Control(pDX, IDC_TABLE, table);
	DDX_Control(pDX, IDC_BTN_END, btn_end);

}

BEGIN_MESSAGE_MAP(CMFCApplication2Dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TABLE, &CMFCApplication2Dlg::OnLvnItemchangedList1)
	ON_BN_CLICKED(IDC_BTN_TABLE, &CMFCApplication2Dlg::OnBnClickedBtnTable)
	ON_BN_CLICKED(IDC_BTN_NEW, &CMFCApplication2Dlg::OnBnClickedBtnNew)
	ON_BN_CLICKED(IDC_BTN_END, &CMFCApplication2Dlg::OnBnClickedBtnEnd)
END_MESSAGE_MAP()


// CMFCApplication2Dlg message handlers

BOOL CMFCApplication2Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	table.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
	table.InsertColumn(0, _T("Process ID"), LVCFMT_LEFT, 100);
	table.InsertColumn(1, _T("Thread Quantity"), LVCFMT_LEFT, 150);
	table.InsertColumn(2, _T("Executable"), LVCFMT_LEFT, 250);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCApplication2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCApplication2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCApplication2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}





void CMFCApplication2Dlg::OnBnClickedBtnTable()
{	
	table.DeleteAllItems();
	// TODO: Add your control notification handler code here
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32);
	int i = 0;
	if (Process32First(snapshot, &processEntry)) {
		while (Process32Next(snapshot, &processEntry)) {
			// Process ID
			CString ProcessId;
			ProcessId.Format(_T("%d"), processEntry.th32ProcessID); 
			table.InsertItem(i, ProcessId);
			
			// cnt Threads

			CString ThreadQuantity;
			ThreadQuantity.Format(_T("%d"), processEntry.cntThreads);
			table.SetItemText(i, 1, ThreadQuantity);

			// Executable file
			table.SetItemText(i, 2, processEntry.szExeFile);
			
			i++;
		}
	}
	
}


void CMFCApplication2Dlg::OnBnClickedBtnNew()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString path;
	text_edit.GetWindowTextW(path);
	
	LightweightProcess lightweightProcess(1);
	lightweightProcess.createThread([path]() {
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;

		if (CreateProcess(path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else {
			
		}
		});
	UpdateData(FALSE);
}


void CMFCApplication2Dlg::OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView->uChanged & LVIF_STATE)
	{
		if ((pNMListView->uNewState & LVIS_STATEIMAGEMASK) != (pNMListView->uOldState & LVIS_STATEIMAGEMASK))
		{	
		}
	}

	*pResult = 0;
	UpdateData(FALSE);
}


DWORD CStringToDWORD(const CString& str)
{
	// Convert CString to LPCTSTR (null-terminated string)
	LPCTSTR pszStr = static_cast<LPCTSTR>(str);

	// Convert LPCTSTR to unsigned long (DWORD)
	unsigned long ulValue = _tcstoul(pszStr, nullptr, 10);

	// Cast unsigned long to DWORD
	DWORD dwValue = static_cast<DWORD>(ulValue);

	return dwValue;
}

void CMFCApplication2Dlg::OnBnClickedBtnEnd()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	int nItemCount = table.GetItemCount();
	for (int i = 0; i < nItemCount; i++)
	{
		
		if (table.GetCheck(i))
		{
			CString strValue = table.GetItemText(i, 0);
			DWORD dwResult = CStringToDWORD(strValue);
		
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)dwResult);
					
					TerminateProcess(hProcess, 0);
					CloseHandle(hProcess);
		}
	}
	UpdateData(FALSE);
	
}

