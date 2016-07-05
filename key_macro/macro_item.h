#pragma once
#include <vector>
#include "common.h"

using namespace std;


#define MOUSEEVENTF_SCREEN_INDEPENDENT_POS         0x0200

#define MACRO_ITEM_SIZE 3

struct sKeybd {
	// Hi  byte - Scan code
	// Low byte - Virtual key code
	long scan_vk_code;
	// 0x01 - ������
	// 0x02 - ����
	// 0x03 - ������ ����
	// 0x10 - �ѱ� �Է� ����
	// 0x20 - Extended key�� ����
	ulong flags;

	void Step (bool vk_pressed[256]);
};

struct sMouse {
	// ���콺 ��ġ(������ġ or �����ġ)
	long x, y;
	// Low WORD:
	//	 MOUSEEVENTF_MOVE        0x0001 // mouse move 
	//	 MOUSEEVENTF_LEFTDOWN    0x0002 // left button down 
	//	 MOUSEEVENTF_LEFTUP      0x0004 // left button up 
	//	 MOUSEEVENTF_RIGHTDOWN   0x0008 // right button down 
	//	 MOUSEEVENTF_RIGHTUP     0x0010 // right button up 
	//	 MOUSEEVENTF_MIDDLEDOWN  0x0020 // middle button down 
	//	 MOUSEEVENTF_MIDDLEUP    0x0040 // middle button up 
	//	 MOUSEEVENTF_XDOWN       0x0080 // x button down (XBUTTON 1,2�� High byte���� ����)
	//	 MOUSEEVENTF_XUP         0x0100 // x button down 
	//	 MOUSEEVENTF_WHEEL       0x0800 // wheel button rolled 
	//   MOUSEEVENTF_ABSOLUTE    0x8000 // absolute move 
	//	 MOUSEEVENTF_SCREEN_INDEPENDENT_POS 0x0200
	// High WORD:
	//	 XBUTTON1, XBUTTON2, ���� ���� ��
	ulong flags;

	void Step (bool vk_pressed[256]);
};

struct sDelay {
	// �ð� ���� �� (����: msec)
	long delay;		// �����ð�, �ð�, Ű��, RGB��
	// 0x00 - �ð�����(delay) ����
	// 0x01 - �����ð�(time) ����
	// 0x02 - �����ð��� ������ ����
	// 0x04 - �����ð��� ��/�� ������ ����
	// 0x06 - �����ð��� ��/��/�ð� ������ ����
	// 0x10 - Ű�Է� ���
	// 0x20 - ȭ�� �� ��ȯ ���
	ulong flags;	// 
	ulong xy;		// ���� ���� �ð�(0~10), ȭ�� ���νĿ��� ȭ�� ��ġ
	//long maxdelay = 20;	// �ִ�����

	bool StepMaxDelay(long &time_remain, int dt);
	bool StepDelay (long &time_remain, int dt);
	bool StepTime  (long &time_remain, int dt);
	bool StepKey   (long &wait_key);
	bool StepColor (long &wait_color, long &wait_xy);

private:
	int GetTimeDelay (int flag, int delay);
	int GetRandomDelay (int delay, int random);
};

struct sString {
	long length;
	// 0x01 - ���� ���� ���� ���
	// 0x02 - ���� ���� ���� ���
	// 0x10 - ���ڿ��� �ּ�(comment)���� ���
	ulong flags;
	// �ּ� or ���ڿ��� ���� ������
	char *string;

	bool Step (long &str_index);
	void KeybdEventNum (int count);

private:
	inline BYTE GetUpperRand () { return 'A' + rand()%26; }
	inline BYTE GetLowerRand () { return 'a' + rand()%26; }
	inline BYTE GetNumberRand() { return '0' + rand()%10; }
	
	void KeybdEvent (BYTE ch);
	void KeybdEventHangul (WORD ch);
	BYTE GetVK (uchar ch, bool *shift_key);
};

struct sMCall {
	char *name;
	// 0x00 - ȣ��Ǵ� ��ũ�� ������ ����
	// 0x01 - ȣ��Ǵ� ��ũ�� ���� ����
	// 0x02 - ���� ���� ��ũ�� �ߴ�
	ulong flags;

	bool Step (long &str_index);
};

struct sLock {
	// 0x00 - ���� ���౸������ ����
	// 0x01 - ���� ���౸������ ��
	ulong flags;

	bool Step (bool &cs_enter);
};

struct sLoop {
	// 0x00 - �ݺ��������� ��
	// 0x01 - �ݺ� ���ڸ� Ű���� �̺�Ʈ�� ���
	// 0x02 - �ݺ� ���ڸ� ���콺 x ��ġ�� ����
	// 0x03 - �ݺ� ���ڸ� ���콺 y ��ġ�� ����
	// 0x07 - �ݺ��������� ����
	ushort flags;
	short  step;	// ����
	long   start;	// �ݺ����� ���� ����
	long   stop;	// �ݺ����� ���� ����

	bool Step (long &index, vector<int> &stack);
};

enum eMacroItem { MI_NONE = 0, MI_KEYBD, MI_MOUSE, MI_DELAY, MI_STRING, MI_MCALL, MI_LOCK, MI_LOOP };

struct sMacroItem {
	eMacroItem type;

	union {
		sKeybd keybd;
		sMouse mouse;
		sDelay delay;
		sString string;
		sMCall mcall;
		sLock  lock;
		sLoop  loop;
		long   data[MACRO_ITEM_SIZE];
	};

	sMacroItem () : type(MI_NONE) 
	{ 
		for (int i = 0; i < MACRO_ITEM_SIZE; i++)
			data[i] = 0;
	}
	
	~sMacroItem () 
	{
		Delete ();
	}

	sMacroItem (const sMacroItem &mi) 
	{
		Copy (mi);
	}
	
	sMacroItem &operator = (const sMacroItem &mi) 
	{
		if (this != &mi) {
			Delete ();
			Copy (mi);
		}
		return *this;
	}

private:
	void Delete (); 
	void Copy (const sMacroItem &mi); 
};
