// Page2.cpp : implementation file
//

#include "stdafx.h"
#include "cvvoip.h"
#include "Page2.h"
#include "voip_flash.h"
#include "voip_flash_mib.h"
#include "voip_flash_tool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPage2 property page

IMPLEMENT_DYNCREATE(CPage2, CPropertyPage)

CPage2::CPage2() : CPropertyPage(CPage2::IDD)
{
	//{{AFX_DATA_INIT(CPage2)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPage2::~CPage2()
{
}

void CPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPage2)
	DDX_Control(pDX, IDC_EDIT3, m_edtKey);
	DDX_Control(pDX, IDC_EDIT2, m_edtOutFile);
	DDX_Control(pDX, IDC_EDIT1, m_edtInFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CPage2)
	ON_BN_CLICKED(IDC_BUTTON1, OnInFIle)
	ON_BN_CLICKED(IDC_BUTTON2, OnOutFile)
	ON_BN_CLICKED(IDC_BUTTON3, OnFileConvert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPage2 message handlers

BOOL CPage2::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPage2::OnInFIle() 
{
	char szFilters[]= "*.dat,*.txt|*.dat;*.txt||";
	CFileDialog fileDlg(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST, szFilters, this);

	fileDlg.m_ofn.lpstrInitialDir = ((CCVVoIPApp *) AfxGetApp())->m_curdir;
	if (fileDlg.DoModal() == IDOK)
	{
		m_edtInFile.SetWindowText(fileDlg.GetPathName());
	}	
}

void CPage2::OnOutFile() 
{
	char szFilters[]= "*.dat,*.txt|*.dat;*.txt||";
	CFileDialog fileDlg(FALSE, NULL, NULL,
		OFN_HIDEREADONLY, szFilters, this);

	fileDlg.m_ofn.lpstrInitialDir = ((CCVVoIPApp *) AfxGetApp())->m_curdir;
	if (fileDlg.DoModal() == IDOK)
	{
		m_edtOutFile.SetWindowText(fileDlg.GetPathName());
	}	
}

void CPage2::OnFileConvert() 
{
	voipCfgAll_t cfg_all;
	char *buf, *text;
	int buf_len, text_len;
	char in_filename[256];
	char out_filename[256];
	char key[32];
	int orig_mode;

	memset(in_filename, 0, sizeof(in_filename));
	memset(out_filename, 0, sizeof(out_filename));
	memset(key, 0, sizeof(key));	

	m_edtInFile.GetWindowText(in_filename, sizeof(in_filename));
	m_edtOutFile.GetWindowText(out_filename, sizeof(out_filename));
	m_edtKey.GetWindowText(key, sizeof(key));

	// read file
	if (flash_voip_read_file(in_filename, &buf, &buf_len) != 0)
	{
		MessageBox("VoIP Converting Error: read failed\n");
		return;
	}

	// decode to text data
	if (flash_voip_decode(buf, buf_len, &text, &text_len) != 0)
	{
		MessageBox("VoIP Converting Error: decode failed\n");
		free(buf);
		return;
	}

	free(buf); // free unused buffer

	orig_mode = (buf_len == text_len) ? 0 : 1;

	// import text for checking
	memset(&cfg_all, 0, sizeof(cfg_all));
	if (flash_voip_import_text(&cfg_all, text, text_len) != 0)
	{
		MessageBox("VoIP Converting Error: import text failed\n");
		free(text);
		return;
	}

	// don't need check feature here
	// it will check different feature on importing to flash
	if ((cfg_all.mode & VOIP_CURRENT_SETTING) &&
		(flash_voip_import_check(&cfg_all.current_setting) < 0))
	{
		MessageBox("VoIP Converting Error: import check failed\n");
		free(text);
		return;
	}
	
	if ((cfg_all.mode & VOIP_DEFAULT_SETTING) &&
		(flash_voip_import_check(&cfg_all.default_setting) < 0))
	{
		MessageBox("VoIP Converting Error: import check failed\n");
		free(text);
		return;
	}
	
	if (orig_mode == 0) // text
	{
		// To config file
		// 1. encode to config format
		if (flash_voip_encode(text, text_len, &buf, &buf_len) != 0)
		{
			MessageBox("VoIP Converting Error: encode failed\n");
			free(text);
			return;
		}

		free(text); // free unused buffer

		// 2. output to config file
		if (flash_voip_write_file(out_filename, buf, buf_len) != 0)
		{
			MessageBox("VoIP Converting Error: write failed\n");
			free(buf);
			return;
		}
		
		free(buf);
		MessageBox("Converting Text->Config done.");
	}
	else
	{
		// output to text file directly
		if (flash_voip_write_file(out_filename, text, text_len) != 0)
		{
			MessageBox("VoIP MIB Convert Error: write failed\n");
			free(text);
			return;
		}

		free(text);
		MessageBox("Converting Config->Text done.");
	}
}
