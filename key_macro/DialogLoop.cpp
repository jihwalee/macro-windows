#include "stdafx.h"
#include "key_macro.h"
#include "DialogLoop.h"


// ���α׷��� ����Ǵ� ���� ��ȭ���� �����ۿ� 
// ������ ���� �����ϰ� �ִ� �������� static���� ����
static ushort _flags = 0x00;
static long   _start = 0;
static long   _stop  = 10;
static short  _step  = 1;


IMPLEMENT_DYNAMIC(CDialogLoop, CDialog)

CDialogLoop::CDialogLoop(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogLoop::IDD, pParent)
{
}

void CDialogLoop::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogLoop, CDialog)
	ON_BN_CLICKED(IDOK, &CDialogLoop::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDialogLoop::OnBnClickedCancel)
END_MESSAGE_MAP()



BOOL CDialogLoop::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (_item.type == MI_NONE) {
		SetWindowText ("�ݺ����� �߰�");
		// �⺻�� ����
		_item.type = MI_LOOP;
		_item.loop.flags = _flags;
		_item.loop.start = _start;
		_item.loop.stop  = _stop;
		_item.loop.step  = _step;
	}
	else {
		SetWindowText ("�ݺ����� ����");
	}

	CheckDlgButton (IDC_RADIO_LOOP_ENTER,   ((_item.loop.flags&0x07) == 0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton (IDC_RADIO_LOOP_PRINT,   ((_item.loop.flags&0x07) == 1) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton (IDC_RADIO_LOOP_MOUSE_X, ((_item.loop.flags&0x07) == 2) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton (IDC_RADIO_LOOP_MOUSE_Y, ((_item.loop.flags&0x07) == 3) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton (IDC_RADIO_LOOP_LEAVE,   ((_item.loop.flags&0x07) == 7) ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton (IDC_CHECK_USER_SET,     (_item.loop.flags&0x08) ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemInt (IDC_EDIT_LOOP_START, _item.loop.start);
	SetDlgItemInt (IDC_EDIT_LOOP_STOP , _item.loop.stop );
	SetDlgItemInt (IDC_EDIT_LOOP_STEP , _item.loop.step );

	return TRUE;
}

void CDialogLoop::OnBnClickedOk()
{
	_flags = 0;
	if      (IsDlgButtonChecked (IDC_RADIO_LOOP_ENTER))   _flags = 0;
	else if (IsDlgButtonChecked (IDC_RADIO_LOOP_PRINT))   _flags = 1;
	else if (IsDlgButtonChecked (IDC_RADIO_LOOP_MOUSE_X)) _flags = 2;
	else if (IsDlgButtonChecked (IDC_RADIO_LOOP_MOUSE_Y)) _flags = 3;
	else if (IsDlgButtonChecked (IDC_RADIO_LOOP_LEAVE))   _flags = 7;

	// ���� �ϼ����� ���� ���
	//if (IsDlgButtonChecked (IDC_CHECK_USER_SET)) {
	//	_flags |= 0x08;
	//}

	_start = GetDlgItemInt (IDC_EDIT_LOOP_START);
	_stop  = GetDlgItemInt (IDC_EDIT_LOOP_STOP );
	_step  = GetDlgItemInt (IDC_EDIT_LOOP_STEP );

	if (_flags == 0) {
		if (_start == _stop);
		else if (_start < _stop) {
			if (_step <= 0) {
				AfxMessageBox ("�ݺ������� ������ 0���� Ŀ�� �մϴ�.");
				return;
			}
		}
		else {
			if (_step >= 0) {
				AfxMessageBox ("�ݺ������� ������ 0���� �۾ƾ� �մϴ�.");
				return;
			}
		}
	}

	_item.loop.flags = _flags;
	_item.loop.start = _start;
	_item.loop.stop  = _stop;
	_item.loop.step  = _step;

	OnOK();
}

void CDialogLoop::OnBnClickedCancel()
{
	OnCancel();
}
