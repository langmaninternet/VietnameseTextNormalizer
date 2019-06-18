
// VietnameseTextNormalizerGUIDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "VietnameseTextNormalizerGUI.h"
#include "VietnameseTextNormalizerGUIDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CVietnameseTextNormalizerGUIDlg dialog



CVietnameseTextNormalizerGUIDlg::CVietnameseTextNormalizerGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VIETNAMESETEXTNORMALIZERGUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVietnameseTextNormalizerGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVietnameseTextNormalizerGUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CVietnameseTextNormalizerGUIDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CVietnameseTextNormalizerGUIDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_TEXTINPUT, &CVietnameseTextNormalizerGUIDlg::OnChangeTextinput)
END_MESSAGE_MAP()


// CVietnameseTextNormalizerGUIDlg message handlers

BOOL CVietnameseTextNormalizerGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVietnameseTextNormalizerGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVietnameseTextNormalizerGUIDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVietnameseTextNormalizerGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVietnameseTextNormalizerGUIDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CVietnameseTextNormalizerGUIDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}

#include "VietnameseTextNormalizer.h"

void CVietnameseTextNormalizerGUIDlg::OnChangeTextinput()
{
	CString inputText;
	GetDlgItem(IDC_TEXTINPUT)->GetWindowText(inputText);

	CString outputText = inputText;


	size_t nChar = inputText.GetLength();
	qwchar* ucs2buffer = (qwchar*)qcalloc(nChar + 10/*safe*/, sizeof(qwchar));
	if (ucs2buffer)
	{
		for (size_t iChar = 0; iChar < nChar; iChar++)
		{
			ucs2buffer[iChar] = (qwchar)inputText[iChar];
		}
		VietnameseTextNormalizer vntObject;
		vntObject.flagStandardTextForNLP = true;
		vntObject.flagStandardTextForASR = true;
		vntObject.Input(ucs2buffer);
		vntObject.Normalize();
		vntObject.GenStandardText();
		if (vntObject.standardText && vntObject.standardTextChange > 0)
		{
			//printf("Normalization : %d change(s) - Utf8 mode\n", vntObject.standardTextChange);
			outputText = L"";
			for (int iChar = 0; iChar < vntObject.standardTextLength; iChar++)
			{
				if (vntObject.standardText[iChar] != 0) outputText += (wchar_t)(vntObject.standardText[iChar]);
			}
		}
		qfree(ucs2buffer);
	}

	GetDlgItem(IDC_TEXTOUTPUT)->SetWindowText(inputText);
}
