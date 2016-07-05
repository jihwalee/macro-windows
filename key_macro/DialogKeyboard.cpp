#include "stdafx.h"
#include "key_macro.h"
#include "virtual_key.h"
#include "DialogKeyboard.h"
#include "key_hook.h"
#include "AppIni.h"

CDialogKeyboard  *g_keyDlg = NULL;

// ���α׷��� ����Ǵ� ���� ��ȭ���� �����ۿ� 
// ������ ���� �����ϰ� �ִ� �������� static���� ����
static long _vk_code = 0x30;
static long _flags = 0x03;
// ������ ���, ����ڰ� ������ �� ����.
static bool _extended = false;	// Extended key(Function Ű, ���� Ű�е��� Ű)���� ǥ��


IMPLEMENT_DYNAMIC(CDialogKeyboard, CDialog)

CDialogKeyboard::CDialogKeyboard(CWnd* pParent /*=NULL*/)
	: CDialogCommon(CDialogKeyboard::IDD, pParent)
{
	g_keyDlg = this;
}

CDialogKeyboard::~CDialogKeyboard()
{
	g_keyDlg = NULL;
}

void CDialogKeyboard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogKeyboard, CDialog)
	ON_BN_CLICKED(IDOK, &CDialogKeyboard::OnBnClickedOk)
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CDialogKeyboard::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (_item.type == MI_NONE) {
		SetWindowText ("Ű���� �̺�Ʈ �߰�");
		// �⺻�� ����
		_item.type             = MI_KEYBD;
		_item.keybd.scan_vk_code = MAKEWORD(_vk_code, 0);
		_item.keybd.flags        = _flags;
	}
	else {
		SetWindowText ("Ű���� �̺�Ʈ ����");
	}

	FillComboBoxVkList (IDC_COMBO_KEYBOARD, _item.keybd.scan_vk_code);

	CheckDlgButton (IDC_RADIO_KEY_3,  ((_item.keybd.flags&0x03) == 0x03) ? BST_CHECKED : BST_UNCHECKED);	// ������ ����
	CheckDlgButton (IDC_RADIO_KEY_1,  ((_item.keybd.flags&0x03) == 0x01) ? BST_CHECKED : BST_UNCHECKED);	// ������
	CheckDlgButton (IDC_RADIO_KEY_2,  ((_item.keybd.flags&0x03) == 0x02) ? BST_CHECKED : BST_UNCHECKED);	// ����
	CheckDlgButton (IDC_CHECK_HANGUL, ((_item.keybd.flags&0x10) == 0x10) ? BST_CHECKED : BST_UNCHECKED);	// �ѱ� �Է� ����
	_extended = ((_item.keybd.flags&0x10) == 0x20);														// Ȯ��Ű

	GetDlgItem(IDC_CHECK_HANGUL)->EnableWindow (g_ini.macroOptions.CHECK_IME_HAN_ENG);

	GetDlgItem(IDC_COMBO_KEYBOARD)->SetFocus ();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogKeyboard::OnKeyboardEvent (BYTE vk_code, bool vk_pressed)
{
	if (vk_code && vk_pressed) {
		// Ű���� �̺�Ʈ�� �ٷ� ó������ �ʰ� timer�� ����Ͽ� 30ms �Ŀ�
		// �Էµ� ����Ű�� ComboBox �׸��� �����Ѵ�: OnTimer() �Լ����� nIDEvent == 1000 ����
		_vk_input = vk_code;

		SetTimer (1000, 30, NULL);
	}
}

void CDialogKeyboard::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1000) {
		if (GetFocus() == GetDlgItem(IDC_COMBO_KEYBOARD)) {
			SendDlgItemMessage (IDC_COMBO_KEYBOARD, CB_SETCURSEL, GetVkIndex (_vk_input));
		}
		KillTimer (1000);
	}

	CDialog::OnTimer(nIDEvent);
}

void CDialogKeyboard::OnBnClickedOk()
{
	// flags�� ���� ����:
	// bit 0, 1 - 1�϶� key �����⸸, 2�϶� key ���⸸, 3�϶� key ������ ���� ����

	int index = SendDlgItemMessage (IDC_COMBO_KEYBOARD, CB_GETCURSEL, 0, 0);
	if (index != -1) {
		_vk_code = GetVkCode(index);
	}

	if      (IsDlgButtonChecked (IDC_RADIO_KEY_3)) _flags = 0x03;	// ������ ����
	else if (IsDlgButtonChecked (IDC_RADIO_KEY_1)) _flags = 0x01;	// ������
	else if (IsDlgButtonChecked (IDC_RADIO_KEY_2)) _flags = 0x02;	// ����

	if (IsDlgButtonChecked (IDC_CHECK_HANGUL)) _flags |= 0x10;	// �ѱ��Է»��� üũ
	if (_extended)                             _flags |= 0x20;	// Extended key���� ǥ��

	_item.keybd.scan_vk_code = MAKEWORD (_vk_code, 0);	// ������ ���, Scan code�� �Ҿ������.
	_item.keybd.flags        = _flags;

	OnOK();
}
