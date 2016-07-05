#pragma once
#include <vector>
#include "macro_item.h"
#include "log.h"
#include "AppIni.h"

using namespace std;

extern int g_critical_section_count;
extern int g_sequence_count;

// Macro Start/Stop �ɼ�
#define	MACRO_AUTO_SS			0x0002
#define MACRO_KEY_START			0x0004
#define MACRO_TIMER_SS			0x0008
#define MACRO_COLOR_START		0x0010
#define MACRO_KEY_STOP			0x0020
#define MACRO_COLOR_STOP		0x0040
#define MACRO_COLOR_ONCE		0x0080


class sMacro {
public:
	// ��ũ�ο� ���� ������Ƽ ����
	char name[256];				// ��ũ�� �̸�
	long start_key;				// ��ũ�� ���� ���� ����Ű, VK_KEY_UP, VK_KEY_CTRL, VK_KEY_ALT, VK_KEY_SHIFT, VK_MODIFIER_KEY �ڵ� ����
	long stop_key;				// ��ũ�� �ߴ� ���� ����Ű, VK_KEY_UP, VK_KEY_CTRL, VK_KEY_ALT, VK_KEY_SHIFT, VK_MODIFIER_KEY �ڵ� ����
	long start_time;			// ��ũ�� ���� �ð� ����
	long stop_time;				// ��ũ�� �ߴ� �ð� ����
	long start_color;			// ��ũ�� ���� �� ����
	long stop_color;			// ��ũ�� �ߴ� �� ����
	long tolerance;				// ���۰� �ߴ� ���� ���� ��� ����
	long start_xy;				// ��ũ�� ���� ���� �������� ȭ����� ��ġ
	long stop_xy;				// ��ũ�� �ߴ� ���� �������� ȭ����� ��ġ
	long options;				// ��ũ�� ����, �ߴ� �� ���� �ɼǵ�

	vector<sMacroItem> _item;	// ��ũ�� �׸�(Ű����, ���콺 �̺�Ʈ)��

public:
	// ��ũ�� ����� �ʿ��� ������
	long index;					// ��ũ�� �׸� ��Ͽ��� ����ǰ� �ִ� �׸� ���� index
	long run_count;
	bool start_color_prev;
	bool stop_color_prev;
	long time_remain;
	long wait_key;
	long wait_color;
	long wait_xy;

private:
	long str_index;				// ���ڿ����� ��µǰ� �ִ� ���ڿ� ���� ����
	bool vk_pressed[256];		// �� ��ũ�ο� ���� ���� Virtucal Key code ���
	bool cs_enter;				// �� ��ũ�ΰ� Critical Section�� ��
	vector<int> loop_count;		// sLoop �׸񿡼� ����ϴ� ī��Ʈ ��

	//CDeviceLock lock;
	void make_loop ();
	void init_runtime_value ();

public:
	long ExecTime (int &depth);
	void MacroStep (int dt);

	void start (int index_ = 1); 
	void stop (); 

	bool is_running() 
	{
		return 0 <= index && index < (int)_item.size();
	}

	sMacro () 
		: start_key(0), stop_key(0), 
		start_time(0), stop_time(0), 
		start_color(0), stop_color(0), 
		tolerance (10),
		start_xy(0), stop_xy(0), 
		options(0) 
	{
		init_runtime_value();
		name[0] = '\0';
	}
	
	sMacro (const sMacro &macro) 
	{
		strncpy (name, macro.name, 255);
		name[255] = '\0';
		start_key   = macro.start_key;
		stop_key    = macro.stop_key;
		start_time  = macro.start_time;
		stop_time   = macro.stop_time;
		start_color = macro.start_color;
		stop_color  = macro.stop_color;
		tolerance   = macro.tolerance;
		start_xy    = macro.start_xy;
		stop_xy     = macro.stop_xy;
		options     = macro.options;
		_item       = macro._item;

		init_runtime_value ();
	}

	sMacro &operator = (const sMacro &macro) 
	{
		if (this != &macro) {
			strncpy (name, macro.name, 255);
			name[255] = '\0';
			start_key   = macro.start_key;
			stop_key    = macro.stop_key;
			start_time  = macro.start_time;
			stop_time   = macro.stop_time;
			start_color = macro.start_color;
			stop_color  = macro.stop_color;
			tolerance   = macro.tolerance;
			start_xy    = macro.start_xy;
			stop_xy     = macro.stop_xy;
			options     = macro.options;
			_item       = macro._item;

			init_runtime_value ();
		}
		return *this;
	}
};


extern vector<sMacro> g_macros;

extern void MacroReset ();
extern void MacroInit  ();
extern void MacroTerm  ();

extern void MacroStartStopShortkey (int vk_input);
extern void MacroStartStopColor (CWnd *wnd);
extern void MacroStartStopTimer ();
extern void MacroStart     (int id, int index = 1);
extern void MacroStop      (int id);
extern bool IsMacroRunning ();
extern int  GetMacroID     (const char *name, int except = -1);

extern void AllMacroStep (int dt);
