#include "stdafx.h"
#include "key_macro.h"
#include "macro_def.h"
#include "DialogMouse.h"
#include "key_hook.h"
#include "virtual_key.h"
#include "Common.h"
#include "Ini.h"


CDialogMouse *g_mouseDlg = NULL;

// ���α׷��� ����Ǵ� ���� ��ȭ���� �����ۿ� 
// ������ ���� �����ϰ� �ִ� �������� static���� ����
static long _flags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
static long _x = 0;
static long _y = 0;

IMPLEMENT_DYNAMIC(CDialogMouse, CDialog)

CDialogMouse::CDialogMouse(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogMouse::IDD, pParent)
{
	g_mouseDlg = this;
}

CDialogMouse::~CDialogMouse()
{
	g_mouseDlg = NULL;
}

void CDialogMouse::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogMouse, CDialog)
	ON_BN_CLICKED(IDOK, &CDialogMouse::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_MOUSE_POS_ABSOLUTE, &CDialogMouse::OnBnClickedCheckMousePosAbsolute)
END_MESSAGE_MAP()


BOOL CDialogMouse::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (_item.type == MI_NONE) {
		SetWindowText ("���콺 �̺�Ʈ �߰�");
		// �⺻�� ����
		_item.type        = MI_MOUSE;
		_item.mouse.flags = _flags;
		_item.mouse.x     = _x;
		_item.mouse.y     = _y;
	}
	else {
		SetWindowText ("���콺 �̺�Ʈ ����");
	}

	char message[256+1];
	_snprintf (message, 256, "%s�� ���� Ŀ�� ��ġ�� ȹ���մϴ�.", GetVkDescFromVK (g_ini.keyMousePosCapture));
	message[256] = '\0';
	SetDlgItemText (IDC_STATIC_MESSAGE, message);

	CheckDlgButton (IDC_CHECK_MOVE,  (_item.mouse.flags & MOUSEEVENTF_MOVE) ? BST_CHECKED : BST_UNCHECKED); 
	SetDlgItemInt (IDC_EDIT_ABSPOS_X, _item.mouse.x);
	SetDlgItemInt (IDC_EDIT_ABSPOS_Y, _item.mouse.y);

	CheckDlgButton (IDC_CHECK_WHEEL, (_item.mouse.flags & MOUSEEVENTF_WHEEL) ? BST_CHECKED : BST_UNCHECKED); 
	SetDlgItemInt  (IDC_EDIT_WHEEL,  (_item.mouse.flags & MOUSEEVENTF_WHEEL) ? (short)HIWORD(_item.mouse.flags) : 0);
	
	int button_select = 0;
	int button_state  = 3;

	if (_item.mouse.flags & (MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP)) {
		button_select = 1;
		button_state  = (_item.mouse.flags >> 1) & 0x03;
	}
	else if (_item.mouse.flags & (MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_RIGHTUP)) { 
		button_select = 2;
		button_state  = (_item.mouse.flags >> 3) & 0x03;
	}
	else if (_item.mouse.flags & (MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MIDDLEUP)) { 
		button_select = 3;
		button_state  = (_item.mouse.flags >> 5) & 0x03;
	}
	else if (_item.mouse.flags & (MOUSEEVENTF_XDOWN|MOUSEEVENTF_XUP)) {
		if      (HIWORD(_item.mouse.flags) == XBUTTON1) button_select = 4;
		else if (HIWORD(_item.mouse.flags) == XBUTTON2) button_select = 5;
		button_state = (_item.mouse.flags >> 7) & 0x03;
	}

	SendDlgItemMessage (IDC_COMBO_BUTTON_SELECT, CB_SETCURSEL, button_select);

	CheckDlgButton (IDC_RADIO_BUTTON_CLICK, (button_state == 3) ? BST_CHECKED : BST_UNCHECKED);	// ���콺 ��ư Ŭ��
	CheckDlgButton (IDC_RADIO_BUTTON_DOWN,  (button_state == 1) ? BST_CHECKED : BST_UNCHECKED);	// ���콺 ��ư ������
	CheckDlgButton (IDC_RADIO_BUTTON_UP,    (button_state == 2) ? BST_CHECKED : BST_UNCHECKED); // ���콺 ��ư ����

	CheckDlgButton (IDC_CHECK_MOUSE_POS_ABSOLUTE, _item.mouse.flags&MOUSEEVENTF_ABSOLUTE               ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton (IDC_CHECK_MOUSE_POS_SCRINDEP, _item.mouse.flags&MOUSEEVENTF_SCREEN_INDEPENDENT_POS ? BST_CHECKED : BST_UNCHECKED);

	OnBnClickedCheckMousePosAbsolute();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogMouse::OnMouseEvent (long mx, long my)
{
	if (IsDlgButtonChecked (IDC_CHECK_MOUSE_POS_SCRINDEP)) {
		// TRACE ("%d %d -- ", mx, my);
		ScreenIndependentMousePos (mx, my);
		// TRACE ("%d %d\n", mx, my);
	}

	SetDlgItemInt (IDC_EDIT_CURPOS_X, mx);
	SetDlgItemInt (IDC_EDIT_CURPOS_Y, my);
}

void CDialogMouse::OnkeyMouseEvent ()
{
	int mx = GetDlgItemInt (IDC_EDIT_CURPOS_X);
	int my = GetDlgItemInt (IDC_EDIT_CURPOS_Y);

	if (IsDlgButtonChecked(IDC_CHECK_MOUSE_POS_ABSOLUTE)) {
		SetDlgItemInt (IDC_EDIT_ABSPOS_X, mx);
		SetDlgItemInt (IDC_EDIT_ABSPOS_Y, my);
	}
	else {
		static int sx = 0;
		static int sy = 0;

		SetDlgItemInt (IDC_EDIT_ABSPOS_X, mx-sx);
		SetDlgItemInt (IDC_EDIT_ABSPOS_Y, my-sy);
		sx = mx; sy = my;
	}
}

void CDialogMouse::OnBnClickedOk()
{
	_flags = 0;

	if (IsDlgButtonChecked (IDC_CHECK_MOVE))  _flags |= MOUSEEVENTF_MOVE;
	if (IsDlgButtonChecked (IDC_CHECK_WHEEL)) _flags |= MOUSEEVENTF_WHEEL;

	_x = GetDlgItemInt (IDC_EDIT_ABSPOS_X);
	_y = GetDlgItemInt (IDC_EDIT_ABSPOS_Y);

	int button_select = SendDlgItemMessage (IDC_COMBO_BUTTON_SELECT, CB_GETCURSEL, 0, 0);

	int button_state = 0;
	if (IsDlgButtonChecked (IDC_RADIO_BUTTON_CLICK))   button_state = 3;	// ���콺 ��ư Ŭ��
	if (IsDlgButtonChecked (IDC_RADIO_BUTTON_DOWN))    button_state = 1;	// ���콺 ��ư ������
	if (IsDlgButtonChecked (IDC_RADIO_BUTTON_UP))      button_state = 2;	// ���콺 ��ư ����

	if (button_select > 0) {
		if (button_select == 1 && button_state == 1) _flags |= MOUSEEVENTF_LEFTDOWN;
		if (button_select == 1 && button_state == 2) _flags |= MOUSEEVENTF_LEFTUP;
		if (button_select == 1 && button_state == 3) _flags |= MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP;

		if (button_select == 2 && button_state == 1) _flags |= MOUSEEVENTF_RIGHTDOWN;
		if (button_select == 2 && button_state == 2) _flags |= MOUSEEVENTF_RIGHTUP;
		if (button_select == 2 && button_state == 3) _flags |= MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_RIGHTUP;

		if (button_select == 3 && button_state == 1) _flags |= MOUSEEVENTF_MIDDLEDOWN;
		if (button_select == 3 && button_state == 2) _flags |= MOUSEEVENTF_MIDDLEUP;
		if (button_select == 3 && button_state == 3) _flags |= MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MIDDLEUP;
	}

	// ���콺�� �� �̺�Ʈ�� X1, X2 ��ư�� ���� ������ �����ϱ� ������ ���� ����� �� ����.
	if (_flags & MOUSEEVENTF_WHEEL) {
		_flags |= MAKELONG(0, GetDlgItemInt (IDC_EDIT_WHEEL));
	}
	else if (button_select > 3) {
		if (button_select == 4 && button_state == 1) _flags |= MAKELONG(MOUSEEVENTF_XDOWN                , XBUTTON1);
		if (button_select == 4 && button_state == 2) _flags |= MAKELONG(MOUSEEVENTF_XUP                  , XBUTTON1);
		if (button_select == 4 && button_state == 3) _flags |= MAKELONG(MOUSEEVENTF_XDOWN|MOUSEEVENTF_XUP, XBUTTON1);

		if (button_select == 5 && button_state == 1) _flags |= MAKELONG(MOUSEEVENTF_XDOWN                , XBUTTON2);
		if (button_select == 5 && button_state == 2) _flags |= MAKELONG(MOUSEEVENTF_XUP                  , XBUTTON2);
		if (button_select == 5 && button_state == 3) _flags |= MAKELONG(MOUSEEVENTF_XDOWN|MOUSEEVENTF_XUP, XBUTTON2);
	}

	if (IsDlgButtonChecked (IDC_CHECK_MOUSE_POS_ABSOLUTE)) _flags |= MOUSEEVENTF_ABSOLUTE;
	if (IsDlgButtonChecked (IDC_CHECK_MOUSE_POS_SCRINDEP)) _flags |= MOUSEEVENTF_SCREEN_INDEPENDENT_POS;

	_item.mouse.flags = _flags;
	_item.mouse.x     = _x;
	_item.mouse.y     = _y;

	// ������ �÷��׸� �˻��� ���� ���� �� �÷��׸� ���� �˻�
	long check_flags = _flags & ~(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_SCREEN_INDEPENDENT_POS);
	if (!check_flags) {
		AfxMessageBox ("���콺�� �ƹ��� ��ɵ� ���õ��� �ʾҽ��ϴ�.");
		return;
	}
	OnOK();
}

void CDialogMouse::OnBnClickedCheckMousePosAbsolute()
{
	if (IsDlgButtonChecked(IDC_CHECK_MOUSE_POS_ABSOLUTE)) {
		SetDlgItemText (IDC_CHECK_MOVE, "������ġ:");
		GetDlgItem(IDC_CHECK_MOUSE_POS_SCRINDEP)->EnableWindow (TRUE);
	}
	else {
		SetDlgItemText (IDC_CHECK_MOVE, "�����ġ:");
		CheckDlgButton (IDC_CHECK_MOUSE_POS_SCRINDEP, BST_UNCHECKED);
		GetDlgItem(IDC_CHECK_MOUSE_POS_SCRINDEP)->EnableWindow (FALSE);
	}
}
