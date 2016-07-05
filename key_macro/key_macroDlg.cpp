#include "stdafx.h"
#include "key_macro.h"
#include "key_macroDlg.h"
#include "DialogMacroEdit.h"
#include "DialogMacroRec.h"
#include "DialogConfig.h"
#include "DialogUserLoop.h"
#include "DialogKeyMouseStatus.h"
#include "DialogMacroDebug.h"
#include "macro_def.h"
#include "macro_file.h"
#include "key_hook.h"
#include "virtual_key.h"
#include "TrayIcon.h"
#include "ScopedLock.h"
#include "MmTimer.h"
#include "common.h"
#include "AppIni.h"
#include "log.h"


static bool _macro_changed = false;
static CDialogUserLoop *_userLoopDlg = NULL;

Ckey_macroDlg         *g_macroDlg  = NULL;
CDialogKeyMouseStatus *g_statusDlg = NULL;
CDialogMacroDebug     *g_debugDlg  = NULL;


// Multimedia timer�� Macro ���࿡ �ǽð��� Ȯ��
static class CMmtMacros: public CMmTimer {
public:
	virtual void OnTimer (UINT timerId, UINT msg) 
	{
		// Multimedia timer�� ���ο� thread�� �����ϱ� ������ Windows thread�� 
		// �ڿ� ������ �浹�� �߻��� �� �ִ�. ���� �ڿ��� Lock�� �ɾ�� �Ѵ�.
		if (g_macroDlg) {
			g_macroDlg->PostMessage (WM_MM_TIMER, 0, 0);
		}
	}
} _mmtms;


// Ckey_macroDlg ��ȭ ����
Ckey_macroDlg::Ckey_macroDlg(CWnd* pParent /*=NULL*/)
	: CDialogCommon(Ckey_macroDlg::IDD, pParent)
{
	_cmd_line[0] = '\0';
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	g_macroDlg = this;
	_userLoopDlg = NULL;

	srand((uint)time(0)+(uint)getpid());
}

Ckey_macroDlg::~Ckey_macroDlg()
{
	if (_userLoopDlg) {
		delete _userLoopDlg;
		_userLoopDlg = NULL;
	}
	if (g_statusDlg) {
		delete g_statusDlg;
		g_statusDlg = NULL;
	}
	if (g_debugDlg) {
		delete g_debugDlg;
		g_debugDlg = NULL;
	}

	g_macroDlg = NULL;
}

void Ckey_macroDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MACROS, _listMacros);
	DDX_Control(pDX, IDC_LIST_MACROS2, _listMacros2);
	DDX_Control(pDX, IDC_STATIC_HOMEPAGE, _staticHomepage);
}

BEGIN_MESSAGE_MAP(Ckey_macroDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_INPUT()
	ON_WM_COPYDATA()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_LBN_DBLCLK(IDC_LIST_MACROS, &Ckey_macroDlg::OnLbnDblclkListMacros)
	ON_BN_CLICKED(IDC_RADIO_MACRO_RUN, &Ckey_macroDlg::OnBnClickedRadioRun)
	ON_BN_CLICKED(IDC_RADIO_MACRO_EDIT, &Ckey_macroDlg::OnBnClickedRadioEdit)
	ON_BN_CLICKED(IDOK, &Ckey_macroDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &Ckey_macroDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_ADD_MACRO, &Ckey_macroDlg::OnBnClickedButtonAddMacro)
	ON_BN_CLICKED(IDC_BUTTON_COPY_MACRO, &Ckey_macroDlg::OnBnClickedButtonCopyMacro)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_MACRO, &Ckey_macroDlg::OnBnClickedButtonDeleteMacro)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_MACRO, &Ckey_macroDlg::OnBnClickedButtonEditMacro)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_FILE, &Ckey_macroDlg::OnBnClickedButtonMacroFile)
	ON_BN_CLICKED(IDC_BUTTON_REC_MACRO, &Ckey_macroDlg::OnBnClickedButtonRecMacro)
	ON_BN_CLICKED(IDC_BUTTON_UP2, &Ckey_macroDlg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN2, &Ckey_macroDlg::OnBnClickedButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &Ckey_macroDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG, &Ckey_macroDlg::OnBnClickedButtonConfig)
	ON_BN_CLICKED(ID_QUIT, &Ckey_macroDlg::OnBnClickedQuit)
	ON_BN_CLICKED(IDC_BUTTON_KEY_MOUSE_STATUS, &Ckey_macroDlg::OnBnClickedButtonKeyMouseStatus)
	ON_BN_CLICKED(IDC_BUTTON_MACRO_DEBUG, &Ckey_macroDlg::OnBnClickedButtonMacroDebug)
END_MESSAGE_MAP()


void Ckey_macroDlg::ToggleKeyMessage ()
{
	// ��� �޽����� ����� ����Ѵ�.
	char message[1024+1];
	_snprintf (message, 1024, "%s�� ���� \"��ũ�� ���� ����\"�� \"���� �ߴܰ� ����\" ���¸� ����մϴ�.", GetVkDescFromVK(g_ini.keyMacroRun));
	message[1024] = '\0';
	SetDlgItemText (IDC_STATIC_MESSAGE, message);
}

// Ckey_macroDlg �޽��� ó����
BOOL Ckey_macroDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	SetWindowText (g_ini.title);

	// Ÿ��Ʋ ��(ĸ�ǹ�)���� �����ư�� ��Ȱ��ȭ
	CMenu *pMenu = GetSystemMenu(FALSE);
	pMenu->RemoveMenu(SC_CLOSE, MF_BYCOMMAND);
	
	// Ʈ���� �������� ����ϴ�.
	m_hIcon2 = LoadIcon (AfxGetInstanceHandle(), MAKEINTRESOURCE (IDR_MAINFRAME2));
	TrayIconCreate(m_hWnd, m_hIcon, "��ũ�� ���α׷� - ����Ŭ�� �ϼ���.");

	// set_log_property (g_ini.logFileName, 1000000);

	// �������� ��ġ�� �����Ѵ�.
	DlgInScreen (g_ini.windowSX, g_ini.windowSY);
	SetWindowPos (NULL, g_ini.windowSX, g_ini.windowSY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	_userLoopDlg = new CDialogUserLoop(NULL);
	_userLoopDlg->Create (IDD_DIALOG_USER_LOOP);
	_userLoopDlg->ShowWindow (/*SW_SHOW*/ SW_HIDE);
	//_userLoopDlg->ShowWindow (SW_SHOW /*SW_HIDE*/);

	g_statusDlg = new CDialogKeyMouseStatus (this);
	g_statusDlg->Create (IDD_DIALOG_KEY_MOUSE_STATUS);
	g_statusDlg->ShowWindow (SW_HIDE);
	// g_statusDlg->ShowWindow (SW_SHOW);

	g_debugDlg = new CDialogMacroDebug (this);
	g_debugDlg->Create (IDD_DIALOG_MACRO_DEBUG);
	g_debugDlg->ShowWindow (SW_HIDE);
	// g_debugDlg->ShowWindow (SW_SHOW);

	ToggleKeyMessage ();

	// �������� ���� ��Ʈ���� �����Ѵ�.
	CheckDlgButton (IDC_RADIO_MACRO_RUN,   g_ini.macroOptions.MACRO_RUN ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton (IDC_RADIO_MACRO_EDIT, !g_ini.macroOptions.MACRO_RUN ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemText (IDC_EDIT_MACRO_FILE, g_ini.macroFileName);
	
	SetDlgItemText (IDC_STATIC_VERSION, GetVersionInfo("ProductVersion"));

	_staticHomepage.SetURL("http://blog.daum.net/pg365/250");
	_staticHomepage.SetToolTipText("�������� �湮");
	_staticHomepage.SetLinkCursor(::LoadCursor(0, MAKEINTRESOURCE(IDC_HAND)));

	// ���� Ʈ���� ���������� �ּ�ȭ ������ ������, Ʈ���� ���������� �ּ�ȭ �Ѵ�.
	if (g_ini.macroOptions.TRAY_AT_STARTUP) {
		ShowWindow (SW_SHOWMINIMIZED);
		PostMessage (WM_SHOWWINDOW, FALSE, SW_OTHERUNZOOM);
	}

	// ���Ͽ��� ��ũ�θ� �ҷ��� ����Ʈ ��Ʈ�ѿ� ǥ���Ѵ�.
	LoadMacros (g_ini.macroFileName);
	MacroReset ();

	_listMacros.ResetContent();
	for (uint i=0; i<g_macros.size(); ++i) {
		_listMacros.InsertString (i, g_macros[i].name);
	}
	_listMacros.SetCurSel(g_ini.macroSel);

	_listMacros.ShowWindow (SW_SHOW);
	_listMacros2.ShowWindow (SW_HIDE);
	EnableDlgItem (TRUE);

	SetTextRecCount();
	OnBnClickedRadioRun ();
	OnBnClickedRadioEdit ();

	// Ű����/���콺 �̺�Ʈ �� ��ġ
	const int RID_size = 2;
	RAWINPUTDEVICE rid[RID_size];
	ZeroMemory (rid, sizeof(RAWINPUTDEVICE)*RID_size);

	// Ű����
	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x06;
	rid[0].dwFlags = RIDEV_INPUTSINK;
	rid[0].hwndTarget = m_hWnd;

	// ���콺
	rid[1].usUsagePage = 0x01;
	rid[1].usUsage = 0x02;
	rid[1].dwFlags = RIDEV_INPUTSINK;
	rid[1].hwndTarget = m_hWnd;    //    �޽����� ó���� ���ν����� ������ �ڵ�

	if (!RegisterRawInputDevices (rid, RID_size, sizeof(RAWINPUTDEVICE))) {
		TRACE ("RegisterRawInputDevices Failed");
	}

	SetTimer (1000, 33, NULL);
	SetTimer (1001, 500, NULL);
	SetTimer (1002, 100, NULL);
	SetTimer (1003, 250, NULL);


	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void Ckey_macroDlg::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialog::OnPaint();
	}

	static bool init = false;
	if (!init) { CheckAlwaysontop();  init = true; }
}

HCURSOR Ckey_macroDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL Ckey_macroDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	if (pCopyDataStruct->dwData == 0) {
		strncpy (_cmd_line, (char *)pCopyDataStruct->lpData, min(1024, pCopyDataStruct->cbData));
		
		//TRACE ("%s \n", pCopyDataStruct->lpData);
	}
	
	return CDialog::OnCopyData(pWnd, pCopyDataStruct);
}

LRESULT Ckey_macroDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_MM_TIMER: {
		DWORD current_time = CMmTimer::GetTime ();
		AllMacroStep (current_time - _macro_exec_time);
		_macro_exec_time = current_time;
		break;
	}
	case WM_TRAY_ICON:
		if(LOWORD(lParam) == 0x203){
			// Double click
			PostMessage (WM_SHOWWINDOW, TRUE, SW_OTHERZOOM);
			SetForegroundWindow();
		}
		return 0;
	case WM_SYSCOMMAND:
		if(wParam==SC_MINIMIZE){
			PostMessage (WM_SHOWWINDOW, FALSE, SW_OTHERUNZOOM);
		}
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void Ckey_macroDlg::OnBnClickedQuit()
{
	g_ini.macroSel = _listMacros.GetCurSel ();

	if (_macro_changed) {
		int ret = AfxMessageBox ("��ũ�ΰ� ����Ǿ����ϴ�. �����ұ��?", MB_YESNO|MB_ICONQUESTION);
		if (ret == IDYES) {
			OnBnClickedButtonSave ();
		}
	}

	// ���⼭ ����
	OnOK ();
}


void Ckey_macroDlg::OnBnClickedOk()
{
	// Alt+F4, Esc Ű�� ���� ������ ����Ǵ� �� ����

	// OnOK();
}

void Ckey_macroDlg::OnBnClickedCancel()
{
	// Alt+F4, Esc Ű�� ���� ������ ����Ǵ� �� ����

	// OnCancel();
}

void Ckey_macroDlg::OnRawInput(UINT nInputcode, HRAWINPUT hRawInput)
{
	// This feature requires Windows XP or greater.
	// The symbol _WIN32_WINNT must be >= 0x0501.

	RAWINPUT rawInput;
    ZeroMemory (&rawInput, sizeof(RAWINPUT));
    
	UINT unSize = sizeof(RAWINPUT);
    ::GetRawInputData ((HRAWINPUT)hRawInput, RID_INPUT, &rawInput, &unSize, sizeof(RAWINPUTHEADER));

	switch (rawInput.header.dwType) {
    case RIM_TYPEKEYBOARD:
		OnRawKeyboard (rawInput.data.keyboard);
        break;
	case RIM_TYPEMOUSE: 
		OnRawMouse (rawInput.data.mouse);
		break; 
    case RIM_TYPEHID:
        break;
    default:
        break;
    }

	CDialog::OnRawInput(nInputcode, hRawInput);
}

void Ckey_macroDlg::OnDestroy()
{
	_userLoopDlg->DestroyWindow ();
	g_statusDlg->DestroyWindow ();
	g_debugDlg->DestroyWindow ();
	
	// ���� ��ũ�ΰ� ���� ���̶�� �������� ��ũ�θ� ��� �ߴ��Ѵ�.
	if (IsMacroRunning()) {
		// g_ini.macroOptions.MACRO_RUN = 0;	// �� �÷��״� �״�� �д�.
								// �ֳ��ϸ� ��ũ�� ���� �߿� â�� ������ 
								// ���� ���۵� �� ��ũ�� ���� ������ ������ �ϱ����ؼ���.
		MacroTerm ();
		_mmtms.Stop ();
	}
	
	RECT rectWnd;
	GetWindowRect (&rectWnd);
	g_ini.windowSX = rectWnd.left;
	g_ini.windowSY = rectWnd.top;

	TrayIconClose();
	CDialog::OnDestroy();
}

void Ckey_macroDlg::StartStopMark ()
{
	// ��ü ȭ���� DC�� ������ ���� �ð��� ���� �ɸ���.
	// MACRO_COLOR_START, MACRO_COLOR_STOP �÷��װ� ������ �ʾҴٸ�,
	// �ƿ� DC�� ������ ��ƾ���� �������� �ʴ´�.
	bool macro_color = false;
	for (uint i=0; i<g_macros.size(); ++i) {
		sMacro &m = g_macros[i];
		if ((m.options & MACRO_COLOR_START) || (m.options & MACRO_COLOR_STOP)) {
			macro_color = true;
			break;
		}
	}
	if (!macro_color) return;

	CDC *dc = CDC::FromHandle(::GetDC(NULL));   
	if (!dc) return;

	CPen pen_blue (PS_SOLID, 2, RGB(0,0,255));
	CPen pen_red (PS_SOLID, 2, RGB(255,0,0));

	HGDIOBJ penOld   = dc->SelectObject (&pen_blue);
	HGDIOBJ brushOld = dc->SelectObject (GetStockObject(NULL_BRUSH));

	for (uint i=0; i<g_macros.size(); ++i) {
		sMacro &m = g_macros[i];
		const int r = 4;

		if (m.options & MACRO_COLOR_STOP) {
			int x = LOWORD(m.stop_xy);
			int y = HIWORD(m.stop_xy);
			dc->SelectObject (&pen_blue);
			dc->Rectangle (x-r, y-r, x+r, y+r);
		}
		if (m.options & MACRO_COLOR_START) {
			int x = LOWORD(m.start_xy);
			int y = HIWORD(m.start_xy);
			dc->SelectObject (&pen_red);
			dc->Rectangle (x-r, y-r, x+r, y+r);
		}
	}

	dc->SelectObject (penOld);
	dc->SelectObject (brushOld);

	ReleaseDC (dc);
}

void Ckey_macroDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1000) {
		ulong run = 0;
		static ulong run_prev = 0;

		for (uint i=0; i<g_macros.size(); ++i) {
			_listMacros2.SetSel (i, g_macros[i].is_running());
			if (i < 32 && g_macros[i].is_running()) run |= 1 << i;
		}
		if (g_ini.macroOptions.MACRO_NOTIFY) {
			if (run != run_prev) {
				int n = 0;
				char message[2560] = "";
				for (uint i=0; i<g_macros.size(); ++i) {
					if (i < 32 && g_macros[i].is_running()) {
						n += _snprintf (message + n, 2560 - n, "%s\n", g_macros[i].name);
					}
				}
				if (n==0) _snprintf (message + n, 2560 - n, "(none)\n");
				TrayIconMessage ("Running Macros", message, 1000);
				run_prev = run;
			}
		}
	}
	else if (nIDEvent == 1001) {
		if (g_ini.macroOptions.MACRO_NOTIFY) {
			static int count = 0;

			if (IsMacroRunning ()) {
				count++;
				if (count%2) {
					SetIcon (m_hIcon2, FALSE);
					SetIcon (m_hIcon2, TRUE);
					TrayIconChange (m_hIcon2, "Ű����/���콺 ��ũ�� ���α׷� - ��ũ�� ���� ���Դϴ�.");
				}
				else {
					SetIcon (m_hIcon, FALSE);
					SetIcon (m_hIcon, TRUE);
					TrayIconChange (m_hIcon, "Ű����/���콺 ��ũ�� ���α׷� - ��ũ�� ���� ���Դϴ�.");
				}
			}
			else {
				if (count%2) {
					SetIcon (m_hIcon, FALSE);
					SetIcon (m_hIcon, TRUE);
					TrayIconChange (m_hIcon, "Ű����/���콺 ��ũ�� ���α׷� - ����Ŭ�� �ϼ���.");
				}
			}
		}
		if (IsMacroRunning() && g_ini.macroOptions.START_STOP_MARK) {
			StartStopMark ();
		}

	}
	else if (nIDEvent == 1002) {
		MacroStartStopTimer ();
	}
	else if (nIDEvent == 1003) {
		MacroStartStopColor (this);
	}

	CDialog::OnTimer(nIDEvent);
}

void Ckey_macroDlg::OnKeyMouseEvent (BYTE vk_code, bool vk_pressed, bool key)
{
	if (vk_code == g_ini.keyMacroRun && vk_pressed == true) {
		if (IsDlgButtonChecked (IDC_RADIO_MACRO_EDIT)) {
			CheckDlgButton (IDC_RADIO_MACRO_RUN , BST_CHECKED);
			CheckDlgButton (IDC_RADIO_MACRO_EDIT, BST_UNCHECKED);
			OnBnClickedRadioRun();
		}
		else if (IsDlgButtonChecked (IDC_RADIO_MACRO_RUN)) {
			CheckDlgButton (IDC_RADIO_MACRO_RUN , BST_UNCHECKED);
			CheckDlgButton (IDC_RADIO_MACRO_EDIT, BST_CHECKED);
			OnBnClickedRadioEdit();
		}
	}

	int vk_input = vk_code;
	vk_input |= GetCtrlAltShiftKeyState ();
	if (!vk_pressed) vk_input |= VK_KEY_UP;

	MacroStartStopShortkey (vk_input);
}

void Ckey_macroDlg::OnBnClickedButtonMacroFile()
{
	if (_macro_changed) {
		int ret = AfxMessageBox ("��ũ�ΰ� ����Ǿ����ϴ�. �����ұ��?", MB_YESNO|MB_ICONQUESTION);
		if (ret == IDYES) {
			OnBnClickedButtonSave ();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	char szFilter[] = "Macro file (*.m)|*.m|All Files(*.*)|*.*||";
	CFileDialog dlg(TRUE, NULL, g_ini.macroFileName, OFN_HIDEREADONLY, szFilter);
	
	if(IDOK == dlg.DoModal()) {
		char curPath[MAX_PATH+1] = "";
		GetCurrentDirectory (MAX_PATH, curPath);
		curPath[MAX_PATH] = '\0';

		CString fileNameNew = "";

		if (strnicmp(curPath, (LPCSTR)dlg.GetPathName (), strlen(curPath)) == 0) {
			fileNameNew = (LPCSTR)dlg.GetPathName () + strlen(curPath) + 1;
		}
		else {
			fileNameNew = dlg.GetPathName();
		}

		if (dlg.GetFileExt ().GetLength () == 0) {
			fileNameNew += ".m";
		}

		g_macros.clear ();
		LoadMacros ((LPCSTR)fileNameNew);
		MacroReset ();

		_listMacros.ResetContent();
		for (uint i=0; i<g_macros.size(); ++i) {
			_listMacros.InsertString (i, g_macros[i].name);
		}
		_listMacros.SetCurSel(0);

		strncpy (g_ini.macroFileName, (LPCSTR)fileNameNew, MAX_PATH);
		g_ini.macroFileName[MAX_PATH] = '\0';
		SetDlgItemText (IDC_EDIT_MACRO_FILE, g_ini.macroFileName);
	}
}

void Ckey_macroDlg::EnableDlgItem(BOOL enable)
{
	// GetDlgItem (IDC_BUTTON_SAVE)        ->EnableWindow (enable);
	GetDlgItem (IDC_BUTTON_REC_MACRO)   ->EnableWindow (enable);
	GetDlgItem (IDC_BUTTON_ADD_MACRO)   ->EnableWindow (enable);
	GetDlgItem (IDC_BUTTON_EDIT_MACRO)  ->EnableWindow (enable);
	GetDlgItem (IDC_BUTTON_COPY_MACRO)  ->EnableWindow (enable);
	GetDlgItem (IDC_BUTTON_DELETE_MACRO)->EnableWindow (enable);
	GetDlgItem (IDC_BUTTON_MACRO_FILE)  ->EnableWindow (enable);
	GetDlgItem (IDC_BUTTON_UP2)         ->EnableWindow (enable);
	GetDlgItem (IDC_BUTTON_DOWN2)       ->EnableWindow (enable);
	GetDlgItem (IDC_BUTTON_CONFIG)      ->EnableWindow (enable);
	GetDlgItem (IDC_BUTTON_MACRO_DEBUG) ->EnableWindow (!enable);
}

void Ckey_macroDlg::OnBnClickedButtonSave()
{
	GetDlgItemText (IDC_EDIT_MACRO_FILE, g_ini.macroFileName, MAX_PATH);
	g_ini.macroFileName[MAX_PATH] = '\0';
	
	SaveMacro (g_ini.macroFileName);
	_macro_changed = false;
}

void Ckey_macroDlg::OnBnClickedButtonConfig()
{
	CDialogConfig dlg;
	dlg.DoModal ();

	SetWindowText (g_ini.title);
	CheckAlwaysontop();
	ToggleKeyMessage ();
}

void Ckey_macroDlg::OnBnClickedRadioRun()
{
	extern const char *GetMacroStartCond(DWORD options, int start_key, int stop_key);

	if (!IsMacroRunning() && IsDlgButtonChecked (IDC_RADIO_MACRO_RUN)) {
		g_ini.macroOptions.MACRO_RUN = 1;

		MacroInit ();
		EnableDlgItem (FALSE);
		
		_listMacros2.ResetContent();
		
		char buff[256+1];
		for (uint i=0; i<g_macros.size(); ++i) {
			const char *ssc = GetMacroStartCond(g_macros[i].options, g_macros[i].start_key, g_macros[i].stop_key);
			if (*ssc) {
				_snprintf (buff, 256, "%s - %s", g_macros[i].name, ssc);
				buff[256] = '\0';
			}
			else {
				_snprintf (buff, 256, "%s", g_macros[i].name);
			}
			_listMacros2.InsertString (i, buff);
		}

		_listMacros.ShowWindow (SW_HIDE);
		_listMacros2.ShowWindow (SW_SHOW);

		_mmtms.Start (g_ini.eventDelay);

		if (g_ini.macroOptions.RUN_SEL_MACRO) {
			int sel = _listMacros.GetCurSel ();
			MacroStart (sel);
		}
	}
}

void Ckey_macroDlg::OnBnClickedRadioEdit()
{
	if (IsMacroRunning() && IsDlgButtonChecked (IDC_RADIO_MACRO_EDIT)) {
		g_ini.macroOptions.MACRO_RUN = 0;
		MacroTerm ();
		EnableDlgItem (TRUE);

		_listMacros.ShowWindow (SW_SHOW);
		_listMacros2.ShowWindow (SW_HIDE);

		g_debugDlg->ShowWindow (SW_HIDE);

		//KillTimer (1000);
		_mmtms.Stop ();
	}
}

void Ckey_macroDlg::CheckAlwaysontop()
{
	SetWindowPos ((g_ini.macroOptions.ALWAYS_ON_TOP)? &wndTopMost : &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void Ckey_macroDlg::SetTextRecCount()
{
	char text[256+1];
	_snprintf (text, 256, "%d�� ��ũ�� ��ϵ�.", g_macros.size());
	text[256] = '\0';

	SetDlgItemText (IDC_STATIC_REC_COUNT2, text);
}

void Ckey_macroDlg::OnBnClickedButtonAddMacro()
{
	CDialogMacroEdit dlg;

	sMacro temp;
	dlg._macro = &temp;
	dlg._macro_index = g_macros.size();

	if (dlg.DoModal () == IDOK) {
		int count = _listMacros.GetCount ();

		_listMacros.InsertString (count, temp.name);
		_listMacros.SetCurSel (count);

		g_macros.push_back (temp);
		_macro_changed = true;
	}

	SetTextRecCount();
}

void Ckey_macroDlg::OnBnClickedButtonEditMacro()
{
	int sel = _listMacros.GetCurSel ();
	if (sel < 0) return;

	CDialogMacroEdit dlg;

	sMacro temp = g_macros[sel];
	dlg._macro = &temp;
	dlg._macro_index = sel;

	if (dlg.DoModal () == IDOK) {
		_listMacros.DeleteString (sel);
		_listMacros.InsertString (sel, temp.name);
		_listMacros.SetCurSel (sel);

		g_macros[sel] = *dlg._macro;
		_macro_changed = true;
	}
}

void Ckey_macroDlg::OnBnClickedButtonRecMacro()
{
	CDialogMacroRec dlg;

	sMacro temp;
	dlg._macro = &temp;

	if (dlg.DoModal () == IDOK) {
		int count = _listMacros.GetCount ();

		_listMacros.InsertString (count, temp.name);
		_listMacros.SetCurSel (count);

		g_macros.push_back (temp);
		_macro_changed = true;
	}

	SetTextRecCount();
}

void Ckey_macroDlg::OnBnClickedButtonCopyMacro()
{
	int sel = _listMacros.GetCurSel ();
	if (sel < 0) return;

	int count = _listMacros.GetCount ();
	sMacro macro = g_macros[sel];

	int name_id = 0;
	do {
		_snprintf (macro.name, 255, "Macro Copy %d", ++name_id);
		macro.name[255] = '\0';
	} while (GetMacroID(macro.name) >= 0);

	_listMacros.InsertString (count, macro.name);
	_listMacros.SetCurSel (count);

	g_macros.push_back (macro);
	_macro_changed = true;

	SetTextRecCount();
}

void Ckey_macroDlg::OnBnClickedButtonDeleteMacro()
{
	int sel = _listMacros.GetCurSel ();
	if (sel < 0) return;

	_listMacros.DeleteString (sel);
	_listMacros.SetCurSel (sel);

	g_macros.erase (g_macros.begin() + sel);
	_macro_changed = true;

	SetTextRecCount();
}

void Ckey_macroDlg::OnLbnDblclkListMacros()
{
	OnBnClickedButtonEditMacro();
}

void Ckey_macroDlg::OnBnClickedButtonUp()
{
	int sel = _listMacros.GetCurSel ();
	if (1 <= sel) {
	   _listMacros.DeleteString (sel);
	   _listMacros.InsertString (sel - 1, g_macros[sel].name);
	   _listMacros.SetCurSel (sel - 1);

	   std::swap (g_macros[sel], g_macros[sel - 1]);
		_macro_changed = true;
	}
}

void Ckey_macroDlg::OnBnClickedButtonDown()
{
	int sel = _listMacros.GetCurSel ();
	if (sel < _listMacros.GetCount () - 1) { 
	   _listMacros.DeleteString (sel);
	   _listMacros.InsertString (sel + 1, g_macros[sel].name);
	   _listMacros.SetCurSel (sel + 1);

		std::swap (g_macros[sel], g_macros[sel + 1]);
		_macro_changed = true;
	}
}

void Ckey_macroDlg::OnBnClickedButtonKeyMouseStatus()
{
	g_statusDlg->ShowWindow (g_statusDlg->IsWindowVisible () ? SW_HIDE : SW_SHOW);
}

void Ckey_macroDlg::OnBnClickedButtonMacroDebug()
{
	g_debugDlg->ShowWindow (g_debugDlg->IsWindowVisible () ? SW_HIDE : SW_SHOW);
}

