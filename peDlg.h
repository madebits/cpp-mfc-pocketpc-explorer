// peDlg.h : header file
//

#if !defined(AFX_PEDLG_H__83D3811E_F2A4_4FA4_AC90_97B0E08A5B61__INCLUDED_)
#define AFX_PEDLG_H__83D3811E_F2A4_4FA4_AC90_97B0E08A5B61__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <Tlhelp32.h>
#include "Shellapi.h"

#define APP_NAME _T("Process Explorer")
#define BTN_BACK _T("< Back")
#define URL _T("")

#define MODE_PROCESS    1
#define MODE_MODULE     2
#define MODE_MEMORY     3
#define MODE_WINDOW     4
//#define MODE_FOLDERS     5
#define MODE_THREADS	6

/////////////////////////////////////////////////////////////////////////////
// CPeDlg dialog

class CPeDlg : public CDialog
{
// Construction
public:
	void InsertThreadInfo(THREADENTRY32* me);
	void FillThreads();
	//void InsertSystemFolder(int folder, PTCHAR name);
	//void FillSystemFolders();
	HWND GetSelectedWindowHandle(int nSelected);
	CPeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPeDlg)
	enum { IDD = IDD_PE_DIALOG };
	CStatic	m_count;
	CButton	m_btnRef;
	CListCtrl	m_peList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPeDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonRefresh();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPopKillprocess();
	afx_msg void OnPopViewmodules();
	afx_msg void OnShowmemory();
	afx_msg void OnShowWindows();
	afx_msg void OnShowInfo();
	afx_msg void OnShowProcessWins();
	afx_msg void OnWinActivate();
	afx_msg void OnWinGotoProcess();
	afx_msg void OnShowThreads();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DWORD GetSelectedPID();
	DWORD lastPID;
	BOOL GoToProcess(DWORD pid);
	void FillWindowsList(BOOL filter);
	void EnableAll(BOOL on);
	void ProcessHeapList(HANDLE snapshot, HEAPLIST32* me);
	void InsertMemoryInfo(DWORD headId, HEAPENTRY32* me);
	void FillMemoryList(DWORD pid,  LPCTSTR name);
	void ErrorMessage(LPCTSTR msg);
	void PrepareList(int mode, LPCTSTR name);
	void FillModuleList(DWORD process, LPCTSTR name);
	void InsertModuleInfo(MODULEENTRY32* me );
	CMenu popup;
	CImageList m_images;
	void InsertProcessInfo(PROCESSENTRY32 *pe);
	void FillProcessList();

    int mode;
    HICON IC_MEMBIG;
    HICON IC_MEMDC;
    HICON IC_MEMFIXED;
    HICON IC_MEMFREE;
	//HICON IC_FOLDER;
};

BOOL CALLBACK EnumWindowsProc(HWND win, LPARAM lParam);

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PEDLG_H__83D3811E_F2A4_4FA4_AC90_97B0E08A5B61__INCLUDED_)
