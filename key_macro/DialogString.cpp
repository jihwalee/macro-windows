#include "stdafx.h"
#include "key_macro.h"
#include "DialogString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���α׷��� ����Ǵ� ���� ��ȭ���� �����ۿ� 
// ������ ���� �����ϰ� �ִ� �������� static���� ����
static long _flags = 0;


// CDialogString dialog
IMPLEMENT_DYNAMIC(CDialogString, CDialog)

CDialogString::CDialogString(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogString::IDD, pParent)
{
}

void CDialogString::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_STRING, _editString);
}


BEGIN_MESSAGE_MAP(CDialogString, CDialog)
	ON_BN_CLICKED(IDOK, &CDialogString::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDialogString::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK_COMMENT, &CDialogString::OnBnClickedCheckComment)
END_MESSAGE_MAP()


// CDialogString message handlers
BOOL CDialogString::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	if (_item.type == MI_NONE) {
		SetWindowText ("���ڿ� �߰�");
		// �⺻�� ����
		_item.type        = MI_STRING;
		_item.string.length  = 0;
		_item.string.flags   = _flags;
		_item.string.string    = new char [1];
		_item.string.string[0] = '\0';
	}
	else {
		SetWindowText ("���ڿ� ����");
		_editString.SetWindowText (_item.string.string);
	}

	CheckDlgButton (IDC_CHECK_RANDOM_CHAR,     (_item.string.flags & 0x01) ? BST_CHECKED : BST_UNCHECKED);	// 0x01 - ���� ���� ���� ���
	CheckDlgButton (IDC_CHECK_SEQUENCE_NUMBER, (_item.string.flags & 0x02) ? BST_CHECKED : BST_UNCHECKED);	// 0x02 - ���� ���� ���� ���
	CheckDlgButton (IDC_CHECK_COMMENT,		   (_item.string.flags & 0x10) ? BST_CHECKED : BST_UNCHECKED);	// 0x10 - ���ڿ��� �ּ�(comment)���� ���

	OnBnClickedCheckComment();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogString::OnBnClickedOk()
{
	int length = _editString.GetWindowTextLength ();
	if (length <= 0) {
		AfxMessageBox ("���ڿ��� ��� �ֽ��ϴ�.");
		return;
	}

	delete [] _item.string.string;
	_item.string.length = length;
	_item.string.string = new char[length+1];
	
	_editString.GetWindowText (_item.string.string, length+1);
	_item.string.string[length] = '\0';

	_flags = 0;
	if (IsDlgButtonChecked (IDC_CHECK_RANDOM_CHAR)    == BST_CHECKED) _flags |= 0x01;	// 0x01 - ���� ���� ���� ���
	if (IsDlgButtonChecked (IDC_CHECK_SEQUENCE_NUMBER)== BST_CHECKED) _flags |= 0x02;	// 0x02 - ���� ���� ���� ���
	if (IsDlgButtonChecked (IDC_CHECK_COMMENT)        == BST_CHECKED) _flags |= 0x10;	// 0x10 - ���ڿ��� �ּ�(comment)���� ���
	_item.string.flags = _flags;

	OnOK();
}

void CDialogString::OnBnClickedCancel()
{
	OnCancel();
}

void CDialogString::OnBnClickedCheckComment()
{
	bool enable = (IsDlgButtonChecked (IDC_CHECK_COMMENT) != BST_CHECKED);

	GetDlgItem (IDC_CHECK_RANDOM_CHAR)->EnableWindow (enable);
	GetDlgItem (IDC_CHECK_SEQUENCE_NUMBER)->EnableWindow (enable);
}
