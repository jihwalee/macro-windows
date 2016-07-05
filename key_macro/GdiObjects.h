//!!! �� ������ EveR�� SEROPI �κ����� �������� ����ϴ� �����Դϴ�.
//!!! �ڵ带 ������ �� ���Ǹ� ����մϴ�.

#pragma once

extern HBITMAP HB_LAMP_DIS;
extern HBITMAP HB_LAMP_OFF;
extern HBITMAP HB_LAMP_ON;

extern void InitGdiObjects ();
extern void DeinitGdiObjects ();

extern BOOL IsObjDeleteable(HGDIOBJ hGdiObj);
extern BOOL ReplaceStaticControlBitmap (HWND hDlg, int ControlID, HBITMAP hBitmap);
extern BOOL ReplaceButtonControlBitmap (HWND hDlg, int ControlID, HBITMAP hBitmap);
