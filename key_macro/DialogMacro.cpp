#include "stdafx.h"
#include "key_macro.h"
#include "DialogMacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���α׷��� ����Ǵ� ���� ��ȭ���� �����ۿ� 
// ������ ���� �����ϰ� �ִ� �������� static���� ����
static int _macro_ID = 0;

IMPLEMENT_DYNAMIC(CDialogMacro, CDialog)

CDialogMacro::CDialogMacro(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogMacro::IDD, pParent)
{
	_caller_index = -1;
}

void CDialogMacro::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogMacro, CDialog)
	ON_BN_CLICKED(IDOK, &CDialogMacro::OnBnClickedOk)
END_MESSAGE_MAP()


BOOL CDialogMacro::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (_item.type == MI_NONE) {
		SetWindowText ("��ũ�� ȣ�� �߰�");
		// �⺻�� ����
		_item.type          = MI_MCALL;
		_item.mcall.name    = new char [255+1];
		_item.mcall.name[0] = '\0';
		
		if (0 <= _macro_ID && _macro_ID < (int)g_macros.size()) {
			strncpy (_item.mcall.name, g_macros[_macro_ID].name, 255);
			_item.mcall.name[255] = '\0';
		}
	}
	else {
		SetWindowText ("��ũ�� ȣ�� ����");
	}

	int sel = _item.mcall.flags & 0x03;
	CheckDlgButton (IDC_RADIO_SEQUENCE, sel == 0 ? BST_CHECKED : BST_UNCHECKED);	// 0x00 - ȣ��Ǵ� ��ũ�� ������ ����
	CheckDlgButton (IDC_RADIO_PARALLEL, sel == 1 ? BST_CHECKED : BST_UNCHECKED);	// 0x01 - ȣ��Ǵ� ��ũ�� ���� ����
	CheckDlgButton (IDC_RADIO_STOP,     sel == 2 ? BST_CHECKED : BST_UNCHECKED);	// 0x02 - ���� ���� ��ũ�� �ߴ�

	SendDlgItemMessage (IDC_COMBO_MACRO, WM_SETREDRAW, 0, 0);
	for (uint i=0; i<g_macros.size(); ++i) {
		SendDlgItemMessage (IDC_COMBO_MACRO, CB_INSERTSTRING, i, (LPARAM)g_macros[i].name);
	}
	SendDlgItemMessage (IDC_COMBO_MACRO, WM_SETREDRAW, 1, 0);
	SendDlgItemMessage (IDC_COMBO_MACRO, CB_SETCURSEL, GetMacroID(_item.mcall.name));

	GetDlgItem(IDC_COMBO_MACRO)->SetFocus ();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogMacro::OnBnClickedOk()
{
	int index = SendDlgItemMessage (IDC_COMBO_MACRO, CB_GETCURSEL, 0, 0);
	if (index < 0) {
		AfxMessageBox ("ȣ���� ��ũ�ΰ� ���õ��� �ʾҽ��ϴ�.");
		return;
	}
	if (_caller_index == index) {
		AfxMessageBox ("�ڱ� �ڽ��� ȣ���� �� �����ϴ�.");
		return;
	}

	long flags = 0;
	if      (IsDlgButtonChecked (IDC_RADIO_SEQUENCE) == BST_CHECKED) flags = 0;	// 0x00 - ȣ��Ǵ� ��ũ�� ������ ����
	else if (IsDlgButtonChecked (IDC_RADIO_PARALLEL) == BST_CHECKED) flags = 1;	// 0x01 - ȣ��Ǵ� ��ũ�� ���� ����
	else if (IsDlgButtonChecked (IDC_RADIO_STOP    ) == BST_CHECKED) flags = 2;	// 0x02 - ���� ���� ��ũ�� �ߴ�
	_item.mcall.flags = flags;

	strncpy (_item.mcall.name, g_macros[index].name, 255);
	_item.mcall.name[255] = '\0';


	_macro_ID = index;

	OnOK();
}
