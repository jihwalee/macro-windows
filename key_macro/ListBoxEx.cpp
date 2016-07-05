#include "stdafx.h"
#include "ListBoxEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern const char *GetItemDescription (const sMacroItem &item);
vector<sMacroItem> CListBoxEx::clipboard;

CListBoxEx::CListBoxEx ()
{
	ctrl = false;
}

void CListBoxEx::DoDataExchange(CDataExchange* pDX)
{
	CListBox::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CListBoxEx, CListBox)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()


void CListBoxEx::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!ctrl) CListBox::OnChar(nChar, nRepCnt, nFlags);
}

void CListBoxEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//TRACE("%X ", nChar);

	if      (ctrl && nChar == 'A') KeyCtrlA ();
	else if (ctrl && nChar == 'C') KeyCtrlC ();
	else if (ctrl && nChar == 'X') KeyCtrlX ();
	else if (ctrl && nChar == 'V') KeyCtrlV ();
	else if (nChar == VK_DELETE)   KeyDelete ();
	
	if (nChar == VK_CONTROL) ctrl = true;

	CListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CListBoxEx::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_CONTROL) ctrl = false;

	CListBox::OnKeyUp(nChar, nRepCnt, nFlags);
}

int CListBoxEx::GetFirstSel ()
{
	int count = GetSelCount();
	CArray<int,int> arr;

	if (count > 0) {
		arr.SetSize(count);
		GetSelItems(count, arr.GetData()); 

		return arr.GetAt (0);
	}
	else {
		return -1;
	}
}

int CListBoxEx::GetLastSel ()
{
	int count = GetSelCount();
	CArray<int,int> arr;

	if (count > 0) {
		arr.SetSize(count);
		GetSelItems(count, arr.GetData()); 

		return arr.GetAt (arr.GetSize()-1);
	}
	else {
		return -1;
	}
}

void CListBoxEx::KeyCtrlA ()
{
	for (int i=0; i<GetCount(); i++) {
		SetSel (i, TRUE);
	}
}

void CListBoxEx::KeyCtrlC ()
{
	vector<sMacroItem> temp;

	int count = GetSelCount();
	CArray<int,int> arr;

	arr.SetSize(count);
	GetSelItems(count, arr.GetData()); 

	temp.reserve (count);

	for (int i=0; i<arr.GetSize(); i++) {
		int sel = arr.GetAt(i);
		if (1 <= sel) { // ó�� �������� �������� �ʴ´�.
			temp.push_back (_items->at(sel));
		}
	}

	// temp ������ ������ clipboard�� ����
	if (temp.size()) {
		clipboard.swap (temp);
	}
}

void CListBoxEx::KeyCtrlV ()
{
	int sel = GetLastSel ();
	if (sel >= 0) {
		SelItemRange (FALSE, 0, GetCount ());

		sel++; // ���������� ���õ� �׸� �ڿ� �ٿ��ֱ� �Ѵ�.

		for (uint i=0; i<clipboard.size(); i++) {
			_items->insert (_items->begin() + sel + i, clipboard[i]);

			InsertString (sel + i, GetItemDescription (clipboard[i]));
			SetSel (sel + i, TRUE);
		}
	}
}

void CListBoxEx::KeyCtrlX ()
{
	vector<sMacroItem> temp;

	int count = GetSelCount();
	CArray<int,int> arr;

	arr.SetSize(count);
	GetSelItems(count, arr.GetData()); 

	temp.reserve (count);

	// ���� ���� ������ ���� �׸���� �Ųٷ� �����.
	for (int i=arr.GetSize()-1; i>=0; i--) {
		int sel = arr.GetAt(i);
		if (1 <= sel) { // ó�� �������� ������ ���ϵ���!
			DeleteString (sel);

			temp.push_back (_items->at(sel));
			_items->erase (_items->begin() + sel);
		}
	}

	// temp ������ ������ clipboard�� �Ųٷ� ����
	if (temp.size()) {
		clipboard.clear();
		clipboard.reserve (temp.size());
		for (int i=temp.size()-1; i>=0; i--) {
			clipboard.push_back (temp[i]);
		}
	}
}

void CListBoxEx::KeyDelete ()
{
	int count = GetSelCount();
	CArray<int,int> arr;

	arr.SetSize(count);
	GetSelItems(count, arr.GetData()); 

	// ���� ���� ������ ���� �׸���� �Ųٷ� �����.
	for (int i=arr.GetSize()-1; i>=0; i--) {
		int sel = arr.GetAt(i);
		if (1 <= sel) { // ó�� �������� ������ ���ϵ���!
			DeleteString (sel);

			_items->erase (_items->begin() + sel);
		}
	}
}

