#include "stdafx.h"
#include "macro_def.h"
#include "AppIni.h"
#include <assert.h>
#include "resource.h"
#include "DialogMacroDebug.h"

extern CDialogMacroDebug *g_debugDlg;

static bool _macro_running = false;
int g_critical_section_count  = 0;
int g_sequence_count = 0;
vector<sMacro> g_macros;


void sMacro::make_loop ()
{
	// sLoop �������� �ݺ��������� ��, �ݺ� �������� ���� �̺�Ʈ�� �����Ѵ�.
	// �ݺ� �������� ���� �̺�Ʈ���� �ݺ� ������ �˻�Ǿ�, true�� ��� �ݺ��������� �� �̺�Ʈ ���� index�� �����Ѵ�.
	
	vector<pair<sLoop, int> > stack;

	for (uint i=0; i<_item.size(); i++) {
		sMacroItem &m = _item[i];

		if (m.type == MI_LOOP) {
			int loop_flag = m.loop.flags & 0x07;

			if (loop_flag == 0) {		// �ݺ��������� ��
				stack.push_back (make_pair(m.loop, i+1));	// MI_LOOP �� ĭ �� ��ġ
			}
			else if (loop_flag == 1 ||	// �ݺ� ���ڸ� Ű���� �̺�Ʈ�� ���
				loop_flag == 2 ||		// �ݺ� ���ڸ� ���콺 X ��ġ�� ����
				loop_flag == 3) {		// �ݺ� ���ڸ� ���콺 Y ��ġ�� ����

				if (stack.size() > 0) ; // OK
				else assert (0);
			}
			else if (loop_flag == 7) {	// �ݺ� �������� ����
				if (stack.size() > 0) {
					pair<sLoop, int> top = stack.back();
					
					m.loop.start = top.second;	// �ݺ� ������ true�� ��� ������ index�� �ٲ۴�.
					m.loop.stop  = top.first.stop;	// stop, step ������ �״�� ������ �д�.
					m.loop.step  = top.first.step;
					
					stack.pop_back ();
				}
				else assert (0);
			}
			else assert (0);
		}
	}
}

void sMacro::init_runtime_value () 
{
	index = -1;
	run_count = 0;
	start_color_prev = false;
	stop_color_prev = false;

	str_index = 0;
	time_remain = 0;
	wait_key = 0;
	wait_color = 0;
	wait_xy = 0;

	for (int i=0; i<256; i++) vk_pressed[i] = false;
	cs_enter = false;
	loop_count.clear ();

	make_loop ();
}

void sMacro::start (int index_) 
{
	init_runtime_value();
	index = index_;
	// if (g_ini.macroOptions.MACRO_LOG) log ("'%s' Start", name);
}

void sMacro::stop () 
{
	index = _item.size();
	run_count = HIWORD(options);

	if (g_ini.macroOptions.RELEASE_ALL_KEYS) {
		// ��ũ�� ���� �� ���� Ű�� �ִµ� ���� �ʰ� �׳� ������ ���
		// ��ũ�� ����(stop) �ÿ� ��� ���� Ű�� ��ư�� ������ �Ѵ�.
		for (int vk_code=0; vk_code<256; vk_code++) {
			UINT sc_vk = MapVirtualKey (vk_code, MAPVK_VK_TO_VSC);

			if (vk_pressed[vk_code]) {
				if      (vk_code == VK_LBUTTON)  mouse_event (MOUSEEVENTF_LEFTUP,   0, 0, 0, EX_KEY_MACRO_ITSELF);
				else if (vk_code == VK_RBUTTON)  mouse_event (MOUSEEVENTF_RIGHTUP,  0, 0, 0, EX_KEY_MACRO_ITSELF);
				else if (vk_code == VK_MBUTTON)  mouse_event (MOUSEEVENTF_MIDDLEUP, 0, 0, 0, EX_KEY_MACRO_ITSELF);
				else if (vk_code == VK_XBUTTON1) mouse_event (MOUSEEVENTF_XUP,      0, 0, XBUTTON1, EX_KEY_MACRO_ITSELF);
				else if (vk_code == VK_XBUTTON2) mouse_event (MOUSEEVENTF_XUP,      0, 0, XBUTTON2, EX_KEY_MACRO_ITSELF);
				else keybd_event ((BYTE)vk_code, sc_vk, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, EX_KEY_MACRO_ITSELF);
			}
		}
	}

	if (cs_enter) {
		if (g_critical_section_count > 0) g_critical_section_count--;
		cs_enter = false;
	}
	// if (g_ini.macroOptions.MACRO_LOG) log ("'%s' Stop", name);
}

long sMacro::ExecTime (int &depth)
{
	// ��ũ�� ȣ���� ��ȯ�� �̷� ���, ���� �ݺ�ȣ��� �� �ִ�.
	// �׷��� depth�� 10���� �����Ѵ�.
	if (++depth > 10) return 100000000;	

	long sum = g_ini.eventDelay*(_item.size() - 1);

	for (uint i=0; i<_item.size(); ++i) {
		sMacroItem &mi = _item[i];

		if (mi.type == MI_DELAY) {
			if (mi.delay.flags == 0) {
				sum += mi.delay.delay;
			}
		}
		else if (mi.type == MI_STRING) {
			sum += g_ini.eventDelay*(mi.string.length - 1);
		}
		else if (mi.type == MI_MCALL) {
			// �� �÷��װ� �� �Ǿ� ������ "���� ����"��
			// ���� ������ �ƴ� �� ���� �ð� ����.
			if (!(mi.mcall.flags & 0x0001)) {	
				int id = GetMacroID (mi.mcall.name);
				if (0 <= id) {
					sum += g_macros[id].ExecTime (depth);
				}
			}
		}
	}
	return sum;
}

void sMacro::MacroStep (int dt)
{
	// � ��ũ�ΰ� CS�� ������ �ִµ�,  
	// �� ��ũ�ΰ� ������ �ִ°��� �ƴ϶�� �������� �ʴ´�.
	if (g_critical_section_count > 0) {
		if (!cs_enter) return;
	}

	int index_ = index;
	sMacroItem &mi = _item[index];

	switch (mi.type) {
	case MI_KEYBD:
		mi.keybd.Step (vk_pressed);
		index++;
		break;
	case MI_MOUSE: 
		mi.mouse.Step (vk_pressed);
		index++;
		break; 
	case MI_DELAY:
		if ((mi.delay.flags&0xF1) == 0x00) {
			if (mi.delay.StepDelay (time_remain, dt)) index++;
		}
		else if ((mi.delay.flags&0xF1) == 0x01) {
			if (mi.delay.StepTime (time_remain, dt)) index++;
		}
		else if ((mi.delay.flags&0xF1) == 0x10){
			if (mi.delay.StepKey (wait_key)) index++;
		}
		else if ((mi.delay.flags&0xF1) == 0x20){
			if (mi.delay.StepColor (wait_color, wait_xy)) index++;
		}
		break;
	case MI_STRING:
		if (mi.string.Step (str_index)) index++;
		break;
	case MI_MCALL:
		if (mi.mcall.Step (str_index)) index++;
		break;
	case MI_LOCK:
		if(mi.lock.Step (cs_enter)) index++;
		break;
	case MI_LOOP:
		if(mi.loop.Step (index, loop_count)) index++;
		break;
	default:
		index++;
		break;
	}

	// ��ũ�ΰ� �ϳ� ����Ǹ�, �̸� Log�� ����Ѵ�.
	if (index_ != index) {
		if (0 <= index_ && index_ < (int)_item.size()) {
			extern const char *GetItemDescription (const sMacroItem &item);
			eMacroItem type = _item[index_].type;

			if (type == MI_KEYBD || type == MI_MOUSE || type == MI_DELAY || type == MI_STRING || type == MI_MCALL) {
				char buff[2048];
				sprintf (buff, "%s: %s\r\n", name, GetItemDescription (_item[index_]));
				g_debugDlg->Log (buff);
			}
		}
	}
}

void AllMacroStep (int dt)
{
	if (!_macro_running) return;

	for (uint i=0; i<g_macros.size(); ++i) {
		sMacro &m = g_macros[i];

		if (0 < m.index && m.index < (int)m._item.size()) {
			m.MacroStep (dt);

			if ((int)m._item.size() <= m.index) {
				m.run_count++;

				int repeat_count = HIWORD(m.options);
				if (repeat_count == 0 || m.run_count < repeat_count) {
					m.index = 1; // ó������ �����
				}
				else {
					m.stop ();
				}
			}
		}
	}
}

void MacroReset ()
{
	_macro_running  = false;
	g_critical_section_count  = 0;
	g_sequence_count = g_ini.sequenceStart;
}

void MacroInit ()
{
	_macro_running  = true;
	g_critical_section_count  = 0;
	g_sequence_count = g_ini.sequenceStart;

	for (uint i=0; i<g_macros.size(); ++i) {
		sMacro &m = g_macros[i];

		// ��ũ�� ����� �Բ� �ڵ� ������ �����Ǿ� ����.
		// ��ũ�θ� �����Ѵ�.
		if (m.options & MACRO_AUTO_SS) m.start (1);
	}
}

void MacroTerm ()
{
	_macro_running  = false;
	g_critical_section_count  = 0;
	g_sequence_count = g_ini.sequenceStart;

	for (uint i=0; i<g_macros.size(); ++i) {
		if (g_macros[i].is_running()) g_macros[i].stop ();
	}

	if (g_ini.macroOptions.RELEASE_ALL_KEYS) {
		// ��� ���� Ű�� ���� �Ѵ�.
		GetKeyState(0); // Windows - Dev Center ���򸻿� ���ϸ�... 
		// GetKeyboardState()�� �����ϱ� ���� GetKeyState() �� ȣ���ؾ� ���� �����Ѵٰ� ��.

		BYTE keyState[256];
		GetKeyboardState((LPBYTE)&keyState);

		for (int vk_code=0; vk_code<256; vk_code++) {
			if (keyState[vk_code] & 1){
				UINT sc_vk = MapVirtualKey (vk_code, MAPVK_VK_TO_VSC);

				keybd_event (vk_code, sc_vk, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, EX_KEY_MACRO_ITSELF);
			}
		}
	}
}

bool CheckSameKey (int macro_vk, int vk_input)
{
#define VK_CTRL_ALT_SHIFT		(VK_KEY_CTRL | VK_KEY_ALT | VK_KEY_SHIFT)

	if (LOBYTE(macro_vk)       != LOBYTE(vk_input))				 return false;
	if ((macro_vk & VK_KEY_UP) != (vk_input & VK_KEY_UP))		 return false;
	
	// Modifier key�� üũ �Ǿ��ִ� ��츸 Ctrl, Alt, Shift Ű�� üũ�Ѵ�.
	if (macro_vk & VK_MODIFIER_KEY) {
		if ((macro_vk & VK_CTRL_ALT_SHIFT) != (vk_input & VK_CTRL_ALT_SHIFT))  return false;	
	}	
	return true;
}

void MacroStartStopShortkey (int vk_input)
{
	if (!_macro_running) return;

	for (uint i=0; i<g_macros.size(); ++i) {
		sMacro &m = g_macros[i];

		bool start = false;	// ��ũ�ΰ� ���۵Ǿ��� ���� �ٷ� ������ �� ������ �Ѵ�.

		if (m.options & MACRO_KEY_START) {
			// ��ũ�ΰ� �������¿��� ����Ű�� ������ ��, ù _item���� �����Ѵ�.
			if (!m.is_running() && CheckSameKey (m.start_key, vk_input)) {
				m.start (1);
				start = true;
			}
		}
		if (m.options & MACRO_KEY_STOP) {
			// ��ũ�ΰ� ������¿��� ����Ű�� ������ ��, ���� ���·� �����.
			if (m.is_running() && CheckSameKey (m.stop_key, vk_input)){
				if (!start) m.stop ();
			}
		}

		// wait_key�� 0�� �ƴ� ���, Ư�� Ű���� �Է��� ��ٸ��� ���̴�.
		if (m.is_running () && m.wait_key > 0) {
			if (CheckSameKey (m.wait_key, vk_input)) {
				m.wait_key = -1; // Ű�� �Էµ�
			}
		}
	}
}

bool ColorCompare (COLORREF c1, COLORREF c2, int tolerance)
{
	int r1 = GetRValue (c1);
	int g1 = GetGValue (c1);
	int b1 = GetBValue (c1);
	int r2 = GetRValue (c2);
	int g2 = GetGValue (c2);
	int b2 = GetBValue (c2);

	return 
		(r1 - tolerance <= r2 && r2 <= r1 + tolerance) &&
		(g1 - tolerance <= g2 && g2 <= g1 + tolerance) &&
		(b1 - tolerance <= b2 && b2 <= b1 + tolerance) ;
}

void MacroStartStopColor (CWnd *wnd)
{
	if (!_macro_running) return;

	CDC *dc = CDC::FromHandle(::GetDC(NULL));  
	if (!dc) return;

	for (uint i=0; i<g_macros.size(); ++i) {
		sMacro &m = g_macros[i];
		bool repeat = !(m.options & MACRO_COLOR_ONCE);
		bool start = false;

		if (m.options & MACRO_COLOR_START) {
			int x = LOWORD (m.start_xy);
			int y = HIWORD (m.start_xy);

			COLORREF scr_color = dc->GetPixel (x, y);
			bool start_color = ColorCompare (m.start_color, scr_color, m.tolerance);

			if (!m.is_running () && ((repeat && start_color) || (start_color && !m.start_color_prev))) {
				m.start (1);
				start = true;
			}
			m.start_color_prev = start_color;
		}
		if (m.options & MACRO_COLOR_STOP) {
			int x = LOWORD (m.stop_xy);
			int y = HIWORD (m.stop_xy);

			COLORREF scr_color = dc->GetPixel (x, y);
			bool stop_color  = ColorCompare (m.stop_color, scr_color, m.tolerance);

			if (m.is_running () && ((repeat && stop_color) || (stop_color && !m.stop_color_prev))) {
				if (!start) m.stop ();
			}
			m.stop_color_prev  = stop_color;
		}

		// wait_color�� 0�� �ƴ� ���, ȭ���� Ư�� ������ ���� �ٲ�⸦ ��ٸ��� ���̴�.
		if (m.is_running () && (m.wait_color > 0) && m.wait_xy) {
			int x = (int)(short)LOWORD (m.wait_xy);
			int y = (int)(short)HIWORD (m.wait_xy);
			int col = m.wait_color&0x00FFFFFF;
			int tol = (m.wait_color >> 24)&0xFF;

			COLORREF scr_color = dc->GetPixel (x, y);
			if (ColorCompare (col, scr_color, tol)) {
				m.wait_color = -1;
			}
		}
	}
	wnd->ReleaseDC (dc);
}

long GetCurrentTime_ (int flag)
{
	time_t current_time = time(NULL);
	struct tm *ts = localtime (&current_time);

	if (flag == 3) {
		return ts->tm_hour*60*60 + ts->tm_min*60 + ts->tm_sec;
	}
	else if (flag == 2) {
		return ts->tm_min*60 + ts->tm_sec;
	}
	else if (flag == 1) {
		return ts->tm_sec;
	}
	else {
		return 0;
	}
}

void MacroStartStopTimer ()
{
	if (!_macro_running) return;

	for (uint i=0; i<g_macros.size(); ++i) {
		sMacro &m = g_macros[i];

		if (m.options & MACRO_TIMER_SS) {
			int flag  = (m.options>>8)&0x03;
			long curt = GetCurrentTime_ (flag);
			
			bool ss = (m.start_time < m.stop_time) ? 
				(m.start_time <= curt && curt <= m.stop_time):
				(m.start_time <= curt || curt <= m.stop_time);

			// ��ũ�ΰ� �������¿��� ����-���� �ð� ������ ����
			if (!m.is_running() &&  ss)     m.start (1);
			// ��ũ�ΰ� ������¿��� ����-���� �ð� ������ ���
			else if (m.is_running() && !ss)  m.stop ();
		}
	}
}


void MacroStart (int id, int index)
{
	if (0 <= id && id < (int)g_macros.size()) {
		sMacro &m = g_macros[id];

		// ��ũ�ΰ� ����� �����̰ų� �������� �����̸� ù _item���� �����Ѵ�.
		if (!m.is_running()) m.start (index);
	}
}

void MacroStop (int id)
{
	if (0 <= id && id < (int)g_macros.size()) {
		sMacro &m = g_macros[id];

		// ���� ���� ���̸� ���� �ܰ踦 ������ ������.
		if (m.is_running()) m.stop ();
	}
}

bool IsMacroRunning ()
{
	return _macro_running;
}

int GetMacroID (const char *name, int except)
{
	for (uint i=0; i<g_macros.size(); ++i) {
		if (except != i && strcmp (g_macros[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}
