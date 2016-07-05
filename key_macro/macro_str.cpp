#include "StdAfx.h"
#include "common.h"
#include "macro_def.h"
#include "virtual_key.h"


const char *strKeyboardOption (int flags)
{
	switch (flags) {
	case  0x00: return "";
	case  0x01: return "������";
	case  0x02: return "����";
	case  0x03: return "";
	default:    return "(����)";
	}
}

const char *strMouseOption (int flags)
{
	switch (flags) {
	case  0x00: return "";
	case  0x01: return "������";
	case  0x02: return "����";
	case  0x03: return "Ŭ��";
	default:    return "(����)";
	}
}

const char *GetItemDescription (const sMacroItem &item)
{
	int n = 0;
	const int MAX_DESC = 256;
	static char desc[MAX_DESC+1];
	desc[0] = '\0';

	switch (item.type) {
	case MI_NONE: 
		n += _snprintf (desc+n, MAX_DESC-n, "    (����)");
		break;

	case MI_KEYBD: {
		int index = GetVkIndex(LOBYTE(item.keybd.scan_vk_code));
		n += _snprintf (desc+n, MAX_DESC-n, "    Ű���� %s ", GetVkDesc(index));

		if (item.keybd.flags) { 
			n += _snprintf (desc+n, MAX_DESC-n, "%s ", strKeyboardOption(item.keybd.flags&0x03));
			if (g_ini.macroOptions.CHECK_IME_HAN_ENG) {
				n += sprintf (desc + n, "[%s] ", (item.keybd.flags&0x10) ? "�ѱ�" : "����");
			}
		}
		break; }

	case MI_MOUSE: {
		n += _snprintf (desc+n, MAX_DESC-n, "    ���콺 ");
		if (item.mouse.flags & MOUSEEVENTF_MOVE) {
			if (item.mouse.flags & MOUSEEVENTF_ABSOLUTE) {
				n += _snprintf (desc+n, MAX_DESC-n, "������ġ(%d, %d) %s", 
					item.mouse.x, item.mouse.y, 
					(item.mouse.flags & MOUSEEVENTF_SCREEN_INDEPENDENT_POS) ? "*SI" : "");
			}
			else {
				n += _snprintf (desc+n, MAX_DESC-n, "�����ġ(%d, %d) %s", 
					item.mouse.x, item.mouse.y, 
					(item.mouse.flags & MOUSEEVENTF_SCREEN_INDEPENDENT_POS) ? "*SI" : "");
			}
		}

		if (item.mouse.flags & MOUSEEVENTF_WHEEL) {
			n += _snprintf (desc+n, MAX_DESC-n, "��(%d) ", (int)(short)HIWORD(item.mouse.flags));
		}

		int flags = (item.mouse.flags>>1) & 0x03;
		if (flags) n += _snprintf (desc+n, MAX_DESC-n, "�޹�ư_%s ", strMouseOption(flags));

		flags = (item.mouse.flags>>3) & 0x03;
		if (flags) n += _snprintf (desc+n, MAX_DESC-n, "������ư_%s ", strMouseOption(flags));

		flags = (item.mouse.flags>>5) & 0x03;
		if (flags) n += _snprintf (desc+n, MAX_DESC-n, "�߾ӹ�ư_%s ", strMouseOption(flags));

		flags = (item.mouse.flags>>7) & 0x03;
		if (flags) {
			if (HIWORD(item.mouse.flags) == XBUTTON1) {
				n += _snprintf (desc+n, MAX_DESC-n, "X1��ư_%s ", strMouseOption(flags));
			}
			if (HIWORD(item.mouse.flags) == XBUTTON2) {
				n += _snprintf (desc+n, MAX_DESC-n, "X2��ư_%s ", strMouseOption(flags));
			}
		}
		break; }

	case MI_DELAY:
		if ((item.delay.flags&0xF1) == 0x00) {
			n += _snprintf (desc+n, MAX_DESC-n, "    �ð����� %0.3f ��", (double)item.delay.delay/1000.);
			
			int random_delay = item.delay.xy;
			if (random_delay > 0) {
				random_delay = BOUND (random_delay, 0, 10);
				n += _snprintf (desc+n, MAX_DESC-n, ", ���� ���� +- %d%% �߰�", random_delay*10);
			}
		}
		else if ((item.delay.flags&0xF1) == 0x01){
			int flag = (item.delay.flags>>1) & 0x03;

			if (flag == 3) {
				n += _snprintf (desc+n, MAX_DESC-n, "    �����ð� %d�� %d�� %d�� ����", 
					(item.delay.delay/(60*60*1000))%24, 
					(item.delay.delay/(60*1000))%60, 
					(item.delay.delay/(1000))%60);
			}
			else if (flag == 2) { 
				n += _snprintf (desc+n, MAX_DESC-n, "    �����ð� --�� %d�� %d�� ����", 
					(item.delay.delay/(60*1000))%60, 
					(item.delay.delay/(1000))%60);
			}
			else if (flag == 1) {
				n += _snprintf (desc+n, MAX_DESC-n, "    �����ð� --�� --�� %d�� ����", 
					(item.delay.delay/(1000))%60);
			}
			else if (flag == 0) {
				n += _snprintf (desc+n, MAX_DESC-n, "    �����ð� --�� --�� --�� ����");
			}
		}
		else if ((item.delay.flags&0xF1) == 0x10){
			int index = GetVkIndex(LOBYTE(item.delay.delay));
			n += _snprintf (desc+n, MAX_DESC-n, "    Ű�Է´�� %s %s", 
				GetVkDesc(index), 
				!(item.delay.delay & VK_KEY_UP) ? "����" : "��");
			if (item.delay.delay & VK_MODIFIER_KEY) {
				if (item.delay.delay & VK_KEY_CTRL ) n += _snprintf (desc+n, MAX_DESC-n, ", Ctrl ");
				if (item.delay.delay & VK_KEY_ALT  ) n += _snprintf (desc+n, MAX_DESC-n, ", Alt ");
				if (item.delay.delay & VK_KEY_SHIFT) n += _snprintf (desc+n, MAX_DESC-n, ", Shift ");
			}
		}
		else if ((item.delay.flags&0xF1) == 0x20){
			n += _snprintf (desc+n, MAX_DESC-n, "    ȭ������(%d, %d, %d) ���(%d) ��ġ(%d, %d) ", 
				(int)GetRValue (item.delay.delay), (int)GetGValue (item.delay.delay), (int)GetBValue (item.delay.delay),
				(int)((item.delay.delay>>24)&0xFF),
				(int)(short)LOWORD (item.delay.xy), (int)(short)HIWORD (item.delay.xy));
		}
		break;

	case MI_STRING: {
		char str[64+1];
		strncpy (str, item.string.string, 64);
		str[64] = '\0';

		int comment = item.string.flags & 0x10;
		if (comment) n += _snprintf (desc+n, MAX_DESC-n, "    # %s", str);
		else         n += _snprintf (desc+n, MAX_DESC-n, "    ���ڿ� '%s'", str);

		break; }

	case MI_MCALL: {
		char str[64+1];
		strncpy (str, item.mcall.name, 64);
		str[64] = '\0';

		int call_method = item.mcall.flags & 0x03;
		if      (call_method == 0) n += _snprintf (desc+n, MAX_DESC-n, "    ��ũ�� ���� ȣ�� '%s'", str);
		else if (call_method == 1) n += _snprintf (desc+n, MAX_DESC-n, "    ��ũ�� ���� ȣ�� '%s'", str);
		else if (call_method == 2) n += _snprintf (desc+n, MAX_DESC-n, "    ��ũ�� ���� �ߴ� '%s'", str);
		break; }

	case MI_LOCK: {
		int enter = item.lock.flags & 0x01;
		if (enter) n += _snprintf (desc+n, MAX_DESC-n, "< �������౸������ ��");
		else  	   n += _snprintf (desc+n, MAX_DESC-n, "> �������౸�����κ��� ����");
		break; }
	case MI_LOOP: {
		ushort loop_flag = (item.loop.flags & 0x07);
		if (loop_flag == 0)		 n += _snprintf (desc+n, MAX_DESC-n, "[   �ݺ��������� �� (%d ~ %d : %d) %c", 
									(uint)item.loop.start, (uint)item.loop.stop, (uint)item.loop.step, 
									(int)((item.loop.flags & 0x08) ? '*':' '));
		else if (loop_flag == 1) n += _snprintf (desc+n, MAX_DESC-n, "    �ݺ� ���ڸ� Ű���� �̺�Ʈ�� ���");
		else if (loop_flag == 2) n += _snprintf (desc+n, MAX_DESC-n, "    �ݺ� ���ڸ� ���콺 X ��ġ�� ���");
		else if (loop_flag == 3) n += _snprintf (desc+n, MAX_DESC-n, "    �ݺ� ���ڸ� ���콺 Y ��ġ�� ���");
		else if (loop_flag == 7) n += _snprintf (desc+n, MAX_DESC-n, "]   �ݺ��������� ����");

		break; }
	}
	desc[MAX_DESC] = '\0';

	return desc;
}


const char *GetMacroStartCond(DWORD options, int start_key, int stop_key)
{
	int n = 0;
	const int MAX_DESC = 256;
	static char desc[MAX_DESC+1];
	desc[0] = '\0';

	if (options & (MACRO_AUTO_SS|MACRO_KEY_START|MACRO_TIMER_SS|MACRO_COLOR_START)) {
		n += _snprintf (desc+n, MAX_DESC-n, "����: ");
		if (options & MACRO_AUTO_SS)		n += _snprintf (desc+n, MAX_DESC-n, "�ڵ�,");
		if (options & MACRO_KEY_START)		n += _snprintf (desc+n, MAX_DESC-n, "%s%s%s%sŰ,", 
			(((VK_MODIFIER_KEY | VK_KEY_CTRL ) & start_key) == (VK_MODIFIER_KEY | VK_KEY_CTRL )) ? "Ctrl "  : "",
			(((VK_MODIFIER_KEY | VK_KEY_ALT  ) & start_key) == (VK_MODIFIER_KEY | VK_KEY_ALT  )) ? "Alt "   : "",
			(((VK_MODIFIER_KEY | VK_KEY_SHIFT) & start_key) == (VK_MODIFIER_KEY | VK_KEY_SHIFT)) ? "Shift " : "",
			GetVkNameFromVK (0xFF & start_key) );
		if (options & MACRO_TIMER_SS)		n += _snprintf (desc+n, MAX_DESC-n, "�ð�,");
		if (options & MACRO_COLOR_START)	n += _snprintf (desc+n, MAX_DESC-n, "��,");

		if (options & MACRO_KEY_STOP)		n += _snprintf (desc+n, MAX_DESC-n, "����: %s%s%s%sŰ,", 
			(((VK_MODIFIER_KEY | VK_KEY_CTRL ) & stop_key) == (VK_MODIFIER_KEY | VK_KEY_CTRL )) ? "Ctrl "  : "",
			(((VK_MODIFIER_KEY | VK_KEY_ALT  ) & stop_key) == (VK_MODIFIER_KEY | VK_KEY_ALT  )) ? "Alt "   : "",
			(((VK_MODIFIER_KEY | VK_KEY_SHIFT) & stop_key) == (VK_MODIFIER_KEY | VK_KEY_SHIFT)) ? "Shift " : "",
			GetVkNameFromVK (0xFF & stop_key) );
		if (options & MACRO_COLOR_STOP)	n += _snprintf (desc+n, MAX_DESC-n, "��,");
		desc[n-1] = '\0';
	}
	return desc;
}