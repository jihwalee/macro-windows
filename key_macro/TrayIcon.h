#pragma once

#define WM_TRAY_ICON	(WM_USER+1435)

// Tray�� ������ȭ �Ͽ� �����ֱ����� �Լ��� 
int  TrayIconCreate  (HWND hWnd, HICON hIcon, char *tip);
void TrayIconClose   ();
void TrayIconChange  (HICON hIcon, char *tip);
void TrayIconMessage (char *title, char *info, int timeout);
