
// TestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Test.h"
#include "TestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CTestDlg dialog




CTestDlg::CTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_INSTALL, &CTestDlg::OnBnClickedBtnInstall)
	ON_BN_CLICKED(IDC_BTN_REMOVE, &CTestDlg::OnBnClickedBtnRemove)
	ON_BN_CLICKED(IDC_BTN_SETINFO, &CTestDlg::OnBnClickedBtnSetinfo)
END_MESSAGE_MAP()


// CTestDlg message handlers

BOOL CTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	LoadLibraryA("ESPI11.dll");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTestDlg::OnPaint()
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
HCURSOR CTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTestDlg::OnBnClickedBtnInstall()
{
	// TODO: Add your control notification handler code here
	typedef BOOL  (_stdcall *IsProcotolInstalled)(__in const GUID& stLayerGuid);
	typedef DWORD (_stdcall *InstallProtocol)(\
		__in const GUID& stLayerGuid, \
		__in const GUID& stChainGuid, \
		__in const WCHAR* pwszProtocolDllPath, \
		__in const WCHAR* pwszProtocolName, \
		__in int nProtocolType\
		);
	typedef void  (__stdcall *SetLayerGUID)(__in GUID& stLayerGuid);

	HMODULE hModule = LoadLibraryA("ESPI11.DLL");
	if (hModule == NULL)
	{
		MessageBox(_T("load espi11 failed,errorcode:"));
		return;
	}
	IsProcotolInstalled pfnIsHaveInstalled = (IsProcotolInstalled)GetProcAddress(hModule,"IsProcotolInstalled");
	if (pfnIsHaveInstalled == NULL)
	{
		MessageBox(_T("get IsHaveInstalled addr failed."));
		return;
	}
	InstallProtocol pfnInstallESPI11Proto  = (InstallProtocol)GetProcAddress(hModule,"InstallProtocol");
	if (pfnInstallESPI11Proto == NULL)
	{
		MessageBox(_T("get InstallESPI11Proto addr failed,errorcode:"));
		return;
	}

	/////call//////
	GUID stGuid = { 0xd0d86306, 0x49bf, 0x49f6, { 0x9e, 0xf7, 0x3c, 0xf2, 0x38, 0x82, 0x92, 0x81 } };
	GUID stChainGuid = { 0x6a7ce70, 0xf0fd, 0x4500, { 0x9d, 0x43, 0xd0, 0xa6, 0xf8, 0xa4, 0xe6, 0x77 } };
	wchar_t wszDllPath[MAX_PATH] = {0};
	if (GetModuleFileNameW(hModule,wszDllPath,_countof(wszDllPath)) == 0)
	{
		MessageBox(_T("get module file path failed,errorcode:"));
		return;
	}
	wchar_t wszProtocolName[] = L"zggprotocol";
	int nProtocolType = IPPROTO_TCP;
	if (pfnIsHaveInstalled(stGuid))
	{
		MessageBox(_T("have installed"));
		return;
	}
	if(pfnInstallESPI11Proto(stGuid,stChainGuid,wszDllPath,wszProtocolName,nProtocolType) != ERROR_SUCCESS)
	{
		MessageBox(_T("install failed"));
		return;
	}

	MessageBox(_T("install success"));
}

void CTestDlg::OnBnClickedBtnRemove()
{
	// TODO: Add your control notification handler code here
	typedef BOOL  (_stdcall *RemoveProtocol)(\
		__in const GUID& stLayerGuid, \
		__in const GUID& stChainGuid\
		);
	HMODULE hModule = LoadLibraryA("ESPI11.DLL");
	if (hModule == NULL)
	{
		MessageBox(_T("load espi11 failed,errorcode:"));
		return;
	}
	RemoveProtocol pfnRemoveESPI11Proto = (RemoveProtocol)GetProcAddress(hModule,"RemoveProtocol");
	if (pfnRemoveESPI11Proto == NULL)
	{
		MessageBox(_T("get RemoveESPI11Proto addr failed,"));
		return;
	}
	GUID stGuid = { 0xd0d86306, 0x49bf, 0x49f6, { 0x9e, 0xf7, 0x3c, 0xf2, 0x38, 0x82, 0x92, 0x81 } };
	GUID stChainGuid = { 0x6a7ce70, 0xf0fd, 0x4500, { 0x9d, 0x43, 0xd0, 0xa6, 0xf8, 0xa4, 0xe6, 0x77 } };
	if (!pfnRemoveESPI11Proto(stGuid,stChainGuid))
	{
		MessageBox(_T("remove failed."));
		return;
	}
	MessageBox(_T("remove success"));

}

void CTestDlg::OnBnClickedBtnSetinfo()
{
	// TODO: Add your control notification handler code here
	GUID stGuid = { 0xd0d86306, 0x49bf, 0x49f6, { 0x9e, 0xf7, 0x3c, 0xf2, 0x38, 0x82, 0x92, 0x81 } };
	GUID stChainGuid = { 0x6a7ce70, 0xf0fd, 0x4500, { 0x9d, 0x43, 0xd0, 0xa6, 0xf8, 0xa4, 0xe6, 0x77 } };

	HMODULE hModule = LoadLibraryA("ESPI11.DLL");
	if (hModule == NULL)
	{
		MessageBox(_T("load espi11 failed,errorcode:"));
		return;
	}
	typedef void  (__stdcall *SetLayerGUID)(__in GUID& stLayerGuid);
	SetLayerGUID pfnSetLayerGUID = (SetLayerGUID)GetProcAddress(hModule,"SetLayerGUID");
	if (pfnSetLayerGUID == NULL)
	{
		MessageBox(_T("get SetLayerGUID addr failed"));
		return;
	}
	pfnSetLayerGUID(stGuid);

	MessageBox(_T("set guid success"));
}
