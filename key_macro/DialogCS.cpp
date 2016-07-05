#include "stdafx.h"
#include "key_macro.h"
#include "DialogCS.h"

// ���α׷��� ����Ǵ� ���� ��ȭ���� �����ۿ� 
// ������ ���� �����ϰ� �ִ� �������� static���� ����
static long _flags = 0x01;


IMPLEMENT_DYNAMIC(CDialogCS, CDialog)

CDialogCS::CDialogCS(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogCS::IDD, pParent)
{
}

void CDialogCS::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogCS, CDialog)
	ON_BN_CLICKED(IDOK, &CDialogCS::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDialogCS::OnBnClickedCancel)
END_MESSAGE_MAP()


BOOL CDialogCS::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (_item.type == MI_NONE) {
		SetWindowText ("�������౸�� �߰�");
		// �⺻�� ����
		_item.type = MI_LOCK;
		_item.lock.flags = _flags;
	}
	else {
		SetWindowText ("�������౸�� ����");
	}

	int lock = _item.lock.flags&0x01;
	CheckDlgButton (IDC_RADIO_CS_LEAVE, !lock ? BST_CHECKED : BST_UNCHECKED);	// 0x00 - ���� ���౸������ ����
	CheckDlgButton (IDC_RADIO_CS_ENTER,  lock ? BST_CHECKED : BST_UNCHECKED);	// 0x01 - ���� ���౸������ ��

	return TRUE;
}

void CDialogCS::OnBnClickedOk()
{
	_flags = 0;
	if (IsDlgButtonChecked (IDC_RADIO_CS_LEAVE)) _flags = 0x00;	// 0x00 - ���� ���౸������ ����
	if (IsDlgButtonChecked (IDC_RADIO_CS_ENTER)) _flags = 0x01;	// 0x01 - ���� ���౸������ ��

	_item.lock.flags = _flags;

	OnOK();
}

void CDialogCS::OnBnClickedCancel()
{
	OnCancel();
}
