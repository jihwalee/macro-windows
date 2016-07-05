#pragma once
#include "Ini.h"


struct sMacorBitOptions {
	DWORD CHECK_IME_HAN_ENG		: 1;	// 0x01 - IME�� ��/�� ��ȯ��� ����
	DWORD MACRO_RUN				: 1;     
	DWORD ALWAYS_ON_TOP			: 1;     
	DWORD HIDE_MACRO_TEST		: 1;	// 0x08
	DWORD TRAY_AT_STARTUP		: 1;
	DWORD KEY_REENTRANCE		: 1;
	DWORD MACRO_LOG				: 1;
	DWORD MACRO_NOTIFY			: 1;	// 0x80
	DWORD START_STOP_MARK		: 1;
	DWORD RUN_SEL_MACRO			: 1;
	DWORD MACRO_DEBUG_LOG		: 1;
	DWORD RELEASE_ALL_KEYS		: 1;	//        ��ũ�� ����� ��� ���� Ű/��ư ����
};

struct sRecBitOptions {
	DWORD REC_KEYBOARD			: 1;	// 0x01 - Keyboard �Է� ���
	DWORD REC_MOUSE_BUTTON		: 1;	// 0x02 - Mouse Button �Է� ���
	DWORD REC_MOUSE_MOVE		: 1;	// 0x04 - Mouse ��ġ ��ȭ ���
	DWORD REC_MOUSE_WHEEL		: 1;	// 0x08 - Mouse �� ��ȭ ���
	DWORD REC_TIME_DELAY		: 1;	// 0x10 - Keyboard�� Mouse �̺�Ʈ�� �ð� ���� ���
	DWORD REC_MERGE_UP_DOWN		: 1;	// 0x20 - Button Up/Down �̺�Ʈ ��ġ�� ���
	DWORD SCREEN_INDEP_MOUSE	: 1;	// 0x40 - Mouse ��ġ ��ȭ�� ȭ�� ũ�⿡ ���������� ���(0 ~ 65535)
	DWORD MOUSE_POS_ABSOLUTE	: 1;	// 0x80	- Check�϶��� Mouse�� ���� ��ġ�� ���, Uncheck�϶��� ����� ��ġ�� ���
	DWORD REC_MOUSE_BUTTON_POS	: 1;	// 0x100 -Mouse Button �Է� ��Ͻ� Ŀ�� ��ġ�� ���� ���
};

struct sEditBitOptions {
	DWORD START_STOP_KEY		: 1;
	DWORD SELECTED_ITEM			: 1;
};

class CAppIni: public CIni
{
	// ���α׷��� ����Ǳ� ���� ini ���Ͽ� �����Ǵ� ������.
	// ���α׷� ������ ���� ������ ���� ���ȴ�.
public: 
	sMacorBitOptions macroOptions;
	sRecBitOptions   recOptions; 
	sEditBitOptions  editOptions; 

	int  hangulKeyboard;

	int  windowSX;
	int  windowSY;  
	int  debugSX;
	int  debugSY;
	int  statusSX;
	int  statusSY;  

	char macroFileName[MAX_PATH+1];
	char title[MAX_PATH+1];

	int  eventDelay;
	int  keyMacroRun;
	int  keyMacroRecord;
	int  keyMousePosCapture;
	int  keyMacroColorStart;
	int  keyMacroColorStop;

	int  recMouseDistance;
	int  recEventTimeInterval;

	// int runSelMacro;
	int sequenceStart;
	int macroSel;

public:
	CAppIni(const char *ini_path);
	virtual ~CAppIni();

	void LoadData();
	void SaveData();

private:
	void TransferAllData (bool bSave);
};

extern CAppIni g_ini;
