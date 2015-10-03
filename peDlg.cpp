// peDlg.cpp : implementation file
//

#include "stdafx.h"
#include "pe.h"
#include "peDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPeDlg dialog

CPeDlg::CPeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPeDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    mode = MODE_PROCESS;

    IC_MEMBIG = AfxGetApp()->LoadIcon(IDI_ICON_MEMBIG);
    IC_MEMDC = AfxGetApp()->LoadIcon(IDI_ICON_MEMDC);
    IC_MEMFIXED = AfxGetApp()->LoadIcon(IDI_ICON_MEMFIXED);
    IC_MEMFREE = AfxGetApp()->LoadIcon(IDI_ICON_MEMFREE);
	//IC_FOLDER = AfxGetApp()->LoadIcon(IDI_ICON_FOLDER);
}

void CPeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPeDlg)
	DDX_Control(pDX, IDC_STATIC_COUNT, m_count);
	DDX_Control(pDX, IDC_BUTTON_REFRESH, m_btnRef);
	DDX_Control(pDX, IDC_LIST, m_peList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPeDlg, CDialog)
	//{{AFX_MSG_MAP(CPeDlg)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnClickList)
	ON_COMMAND(ID_POP_KILLPROCESS, OnPopKillprocess)
	ON_COMMAND(ID_POP_VIEWMODULES, OnPopViewmodules)
	ON_COMMAND(ID_SHOWMEMORY, OnShowmemory)
	ON_COMMAND(ID_SHOWWIND, OnShowWindows)
	ON_COMMAND(ID_SHOWINFO, OnShowInfo)
	ON_COMMAND(ID_SHOWPROCESSWINS, OnShowProcessWins)
	ON_COMMAND(ID_WIN_ACTIVATE, OnWinActivate)
	ON_COMMAND(ID_WIN_GOTOPROC, OnWinGotoProcess)
	ON_COMMAND(ID_SHOW_THREADS, OnShowThreads)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPeDlg message handlers

BOOL CPeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CenterWindow(GetDesktopWindow());	// center to the hpc screen

	// TODO: Add extra initialization here

    CRect wrc;
    this->GetWindowRect(&wrc);
    this->m_peList.SetWindowPos(this, wrc.left, wrc.top - 26, wrc.Width(), wrc.Height() - 35, SWP_NOOWNERZORDER);
    
    CRect lrc, brc, crc; 
    this->m_peList.GetWindowRect(&lrc);
    this->m_btnRef.GetWindowRect(&brc);
    this->m_btnRef.SetWindowPos(this, wrc.left + 2, lrc.bottom - 22, brc.Width(), brc.Height(), SWP_NOOWNERZORDER);

    this->m_btnRef.GetWindowRect(&brc);
    this->m_count.GetWindowRect(&crc);
    this->m_count.SetWindowPos(this, brc.right + 5, lrc.bottom - 18, wrc.Width() - brc.Width(), crc.Height(), SWP_NOOWNERZORDER);
 
    popup.LoadMenu(IDR_MENUBAR_POP);

    EnableAll(FALSE);
    FillProcessList();
    EnableAll(TRUE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPeDlg::FillProcessList()
{
    //this->lastPID = this->GetSelectedPID();

	PrepareList(MODE_PROCESS, _T("Process"));

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(snapshot == INVALID_HANDLE_VALUE)
    {
        ErrorMessage(_T("Cannot get process information!"));
        return;
    }
    
    PROCESSENTRY32 me;
    me.dwSize = sizeof(me);
    me.cntUsage = 1;
    me.th32ModuleID = 0;
    
    if(!Process32First(snapshot, &me))
    {
       ErrorMessage(_T("Cannot parse process information!"));
       CloseToolhelp32Snapshot(snapshot);
       return;
    }
   
    InsertProcessInfo(&me);

    while(TRUE)
    {
        me.dwSize = sizeof(me);
        me.cntUsage = 1;
        me.th32ModuleID = 0;
        
        if(!Process32Next(snapshot, &me)) break;
        if(GetLastError() == ERROR_NO_MORE_FILES) break;

        InsertProcessInfo(&me);

        
    }

    if(snapshot != INVALID_HANDLE_VALUE){
       if(!CloseToolhelp32Snapshot(snapshot))
       { 
           ErrorMessage(_T("Cannot free process information!"));
           return;
       }
    }
}

void CPeDlg::InsertProcessInfo(PROCESSENTRY32 *pe)
{
    if(pe == NULL) return;
 
    int imageIndex = -1;
    HICON icon[1];
    ExtractIconEx(pe->szExeFile, 0, NULL, &icon[0], 1); 
    if(icon[0] != NULL)
    {
        imageIndex = this->m_images.Add(icon[0]);
        DestroyIcon(icon[0]);
    }
    
    CString temp;
    temp.Format(_T("%s"), pe->szExeFile);
    int nIndex = this->m_peList.InsertItem(0, temp, imageIndex);
    this->m_peList.SetItemData(nIndex, pe->th32ProcessID);

    temp.Format(_T("%u"), pe->th32ProcessID);
    this->m_peList.SetItemText(nIndex, 1, temp);
    
    temp.Format(_T("%d"), pe->cntThreads);
    this->m_peList.SetItemText(nIndex, 2, temp);

	temp.Format(_T("%08p"), pe->th32MemoryBase);
    this->m_peList.SetItemText(nIndex, 3, temp);

    HANDLE process = OpenProcess(0, FALSE, pe->th32ProcessID);
    if(process != NULL)
    {
   
        TCHAR buff[MAX_PATH];
		::memset(buff, 0, sizeof(TCHAR) * MAX_PATH);
        ::GetModuleFileName((HMODULE)process, (LPWSTR)&buff, MAX_PATH);
        CloseHandle(process);
        temp.Format(_T("%s"), buff);
        this->m_peList.SetItemText(nIndex, 4, temp);
    }
}

void CPeDlg::OnButtonRefresh() 
{
    CWaitCursor wait;
	EnableAll(FALSE);
    FillProcessList();
	GoToProcess(this->lastPID);
    EnableAll(TRUE);
}

void CPeDlg::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int nSelected = this->m_peList.GetSelectionMark();
    if(nSelected < 0) return;
	
	NMLISTVIEW* pInfo;
	CPoint point;
	pInfo = (NMLISTVIEW*)pNMHDR;
	point = pInfo->ptAction;
/*
	if(mode == MODE_FOLDERS)
	{
		CString path = this->m_peList.GetItemText(nSelected, 1);
		CString msg;
		msg.Format(_T("Explore folder:\n%s"), path);
		if(::MessageBox(this->m_hWnd, msg, APP_NAME, MB_ICONQUESTION | MB_YESNO) != IDYES) return;
    	SHELLEXECUTEINFO shei;
		memset(&shei, 0, sizeof(shei));
		shei.cbSize = sizeof(SHELLEXECUTEINFO);
		shei.fMask = NULL;
		shei.hwnd = NULL;
		shei.lpVerb = _T("Explore");
		shei.lpFile = _T("Windows"); //path + _T("\\");
		shei.lpDirectory = _T("\\");
		shei.lpParameters = NULL;
		shei.nShow = SW_MAXIMIZE;
		shei.hInstApp = NULL;
		if(0 == ShellExecuteEx(&shei))
		{
			
			msg.Format(_T("Error!\n"));
			int err = (int)shei.hInstApp;
				if(err == SE_ERR_FNF)
				msg = _T("File not found!");
				else if(err == SE_ERR_PNF)
				msg = _T("Path not found!");
	
				else if(err == SE_ERR_ACCESSDENIED)
				msg = _T("Access denied!");
	
				else if(err == SE_ERR_OOM)
				msg = _T("Out of memory!");
		
				else if(err == SE_ERR_DLLNOTFOUND)
				msg = _T("DLL not found!");
		
				else if(err == SE_ERR_SHARE)
				msg = _T("Resource in use!");
			
				else if(err == SE_ERR_ASSOCINCOMPLETE)
				msg = _T("Not enough information!");
			
				else if(err == SE_ERR_DDETIMEOUT)
				msg = _T("Operation timed out!");
	
				else if(err == SE_ERR_DDEFAIL)
				msg = _T("Operation failed!");
			
				else if(err == SE_ERR_DDEBUSY)
				msg = _T("Operation busy!");
		
				else if(err == SE_ERR_NOASSOC)
					msg = _T("No association!");

				else
					msg = _T("Cannot open folder!");
					

			ErrorMessage(msg);

		}
	}
    else */
	if(mode == MODE_THREADS)
	{
		int nSelected = this->m_peList.GetSelectionMark();
		if(nSelected < 0) return;
		CString msg;
		msg.Format(_T("Switch to current process?\n%s"),
		this->m_peList.GetItemText(nSelected, 3));
		if(::MessageBox(this->m_hWnd, msg, APP_NAME, MB_ICONQUESTION | MB_YESNO) != IDYES) return;
		DWORD pid = this->m_peList.GetItemData(nSelected);
		OnButtonRefresh();

		if(!GoToProcess(pid))
			ErrorMessage(_T("Cannot find process!"));
	}
	else if(mode == MODE_WINDOW)
    {
        CMenu* menu = popup.GetSubMenu(1);

		BOOL enable = FALSE;
		HWND hWin = GetSelectedWindowHandle(nSelected);
		if((hWin != 0) && (::IsWindowVisible(hWin)))
		{
			HWND hParentWin = ::GetParent(hWin);
			if(hParentWin == NULL){
				TCHAR title[MAX_PATH];
				::memset(title, 0, sizeof(TCHAR) * MAX_PATH);
				::GetWindowText(hWin, (LPTSTR)&title, MAX_PATH);
				title[MAX_PATH - 1] = _T('\0');
				CString temp = title;
				if(temp.GetLength() > 0)			
					enable = TRUE;
			}
		}
		menu->EnableMenuItem(ID_WIN_ACTIVATE, MF_BYCOMMAND | (enable ? MF_ENABLED : MF_GRAYED));
		menu->TrackPopupMenu(TPM_LEFTALIGN, pInfo->ptAction.x, pInfo->ptAction.y + 10, this);
    }
    else if(mode == MODE_PROCESS)
    {
        //popup.LoadMenu(IDR_MENUBAR_POP);
        CMenu* menu = popup.GetSubMenu(0);
        menu->TrackPopupMenu(TPM_LEFTALIGN, pInfo->ptAction.x, pInfo->ptAction.y + 10, this);
	    //popup.DestroyMenu();
        //menu->DestroyMenu();
    }
    
    *pResult = 0;
}

DWORD CPeDlg::GetSelectedPID()
{
	int nSelected = this->m_peList.GetSelectionMark();
    if(nSelected < 0) return 0;
	return this->m_peList.GetItemData(nSelected);
}

BOOL CPeDlg::GoToProcess(DWORD pid)
{
	int count = this->m_peList.GetItemCount();
    for(int i = 0; i < count; ++i)
    {
        DWORD cpid = this->m_peList.GetItemData(i);
		if(cpid == pid)
        {
			this->m_peList.EnsureVisible(i, FALSE);
            this->m_peList.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
            return TRUE;
        }
    }
	return FALSE;
}

void CPeDlg::OnPopKillprocess() 
{
    int nSelected = this->m_peList.GetSelectionMark();
    if(nSelected < 0) return;
    CString strText = this->m_peList.GetItemText(nSelected, 0);
    CString msg;
    msg.Format(_T("Kill process: %s?\n\nWarning: Killing a process may cause unrecoverable loss of data!"), strText);
    
    if(::MessageBox(this->m_hWnd, msg, APP_NAME, MB_ICONWARNING | MB_YESNO) != IDYES) return;
    
	CWaitCursor wait;
	
	DWORD pid = this->m_peList.GetItemData(nSelected);
    HANDLE process = OpenProcess(0, FALSE, pid);
    if(process == NULL)
    {
        ErrorMessage(_T("Cannot find process!"));
        return;
    }
    if(!::TerminateProcess(process, 1))
    {
        ErrorMessage(_T("Cannot kill process!"));
        return;
    }
    CloseHandle(process);
	
    EnableAll(FALSE);
    FillProcessList();
    EnableAll(TRUE);
	
}

void CPeDlg::OnPopViewmodules() 
{
	int nSelected = this->m_peList.GetSelectionMark();
    if(nSelected < 0) return;
    CString strText = this->m_peList.GetItemText(nSelected, 0);
    DWORD pid = this->m_peList.GetItemData(nSelected);
    EnableAll(FALSE);
    FillModuleList(pid, (LPCTSTR)strText);
	EnableAll(TRUE);
}


void CPeDlg::FillModuleList(DWORD process, LPCTSTR name)
{
    this->lastPID = this->GetSelectedPID();

	CWaitCursor wait;
	
	PrepareList(MODE_MODULE, name);

    DWORD flags = TH32CS_SNAPMODULE;
    if((process == 0) && (name == NULL)) flags = TH32CS_SNAPALL;
    
    HANDLE snapshot = CreateToolhelp32Snapshot(flags, process);
    if(snapshot == INVALID_HANDLE_VALUE)
    {
        ErrorMessage(_T("Cannot get module information!"));
        return;
    }
    
    MODULEENTRY32 me;
    me.dwSize = sizeof(me);
    
    if(!Module32First(snapshot, &me))
    {
       if(GetLastError() != ERROR_NO_MORE_FILES)
            ErrorMessage(_T("Cannot parse module information!"));
       CloseToolhelp32Snapshot(snapshot);
       return;
    }
   
    InsertModuleInfo(&me);

    while(TRUE)
    {
        me.dwSize = sizeof(me);
        
        if(!Module32Next(snapshot, &me)) break;
        if(GetLastError() == ERROR_NO_MORE_FILES) break;

        InsertModuleInfo(&me);
    }

    if(snapshot != INVALID_HANDLE_VALUE){
       if(!CloseToolhelp32Snapshot(snapshot))
       { 
           ErrorMessage(_T("Cannot free module information!"));
           return;
       }
    }
}

void CPeDlg::InsertModuleInfo(MODULEENTRY32 *me)
{
    if(me == NULL) return;
 
    int imageIndex = -1;
    
    HICON icon[1];
    ExtractIconEx(me->szModule, 0, NULL, &icon[0], 1); 
    if(icon[0] != NULL)
    {
        imageIndex = this->m_images.Add(icon[0]);
        DestroyIcon(icon[0]);
    }

    CString temp;
    temp.Format(_T("%s"), me->szModule);
    int nIndex = this->m_peList.InsertItem(0, temp, imageIndex);
    this->m_peList.SetItemData(nIndex, me->th32ModuleID);

    temp.Format(_T("%08p"), me->modBaseAddr);
    this->m_peList.SetItemText(nIndex, 1, temp);
    
    temp.Format(_T("%d / %d"), me->ProccntUsage, me->GlblcntUsage);
    this->m_peList.SetItemText(nIndex, 2, temp);

    //temp.Format(_T("%s"), me->szExePath);
    //this->m_peList.SetItemText(nIndex, 3, temp);

	temp.Format(_T("%u"), me->modBaseSize);
    this->m_peList.SetItemText(nIndex, 3, temp);

    TCHAR buff[MAX_PATH];
	::memset(buff, 0, MAX_PATH * sizeof(TCHAR));
    ::GetModuleFileName(me->hModule, (LPWSTR)&buff, MAX_PATH);
    temp.Format(_T("%s"), buff);
    this->m_peList.SetItemText(nIndex, 4, temp);
}


void CPeDlg::OnShowmemory() 
{
    int nSelected = this->m_peList.GetSelectionMark();
    if(nSelected < 0) return;
    CString strText = this->m_peList.GetItemText(nSelected, 0);
    DWORD pid = this->m_peList.GetItemData(nSelected);
    EnableAll(FALSE);
    FillMemoryList(pid, (LPCTSTR)strText);
    EnableAll(TRUE);
}

long memorySize = 0;

void CPeDlg::FillMemoryList(DWORD process, LPCTSTR name)
{
 	this->lastPID = this->GetSelectedPID();

	memorySize = 0;
	
	CString msg;
    msg.Format(_T("Show memory for %s?\nThis may take some time!"), name);
    if(::MessageBox(this->m_hWnd, msg, APP_NAME, MB_ICONWARNING | MB_YESNO) != IDYES) return;
    
	CWaitCursor wait;
    PrepareList(MODE_MEMORY, name);
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, process);
    if(snapshot == INVALID_HANDLE_VALUE)
    {
        ErrorMessage(_T("Cannot get module information!"));
        return;
    }
    
    HEAPLIST32 me;
    me.dwSize = sizeof(me);
    me.th32ProcessID = process;
    
    if(!Heap32ListFirst(snapshot, &me))
    {
       if(GetLastError() != ERROR_NO_MORE_FILES)
            ErrorMessage(_T("Cannot parse module information!"));
       CloseToolhelp32Snapshot(snapshot);
       return;
    }
   
    ProcessHeapList(snapshot, &me);

    while(TRUE)
    {
        me.dwSize = sizeof(me);
        me.th32ProcessID = process;
        
        if(!Heap32ListNext(snapshot, &me)) break;
        if(GetLastError() == ERROR_NO_MORE_FILES) break;

        ProcessHeapList(snapshot, &me);
    }

    if(snapshot != INVALID_HANDLE_VALUE){
       if(!CloseToolhelp32Snapshot(snapshot))
       { 
           ErrorMessage(_T("Cannot free module information!"));
           return;
       }
    }
}

void CPeDlg::ProcessHeapList(HANDLE snapshot, HEAPLIST32 *list)
{
    if(list == NULL) return;
    
    HEAPENTRY32 me;
    me.dwSize = sizeof(me);

    if(!Heap32First(snapshot, &me, list->th32ProcessID, list->th32HeapID))
    {
       return;
    }

    InsertMemoryInfo(list->th32HeapID, &me);

    while(TRUE)
    {
        me.dwSize = sizeof(me);
        me.th32ProcessID = list->th32ProcessID;
        
        if(!Heap32Next(snapshot, &me)) break;
        if(GetLastError() == ERROR_NO_MORE_FILES) break;

        InsertMemoryInfo(list->th32HeapID, &me);
    }
}

void CPeDlg::InsertMemoryInfo(DWORD headId, HEAPENTRY32 *me)
{
    if(me == NULL) return;
 
    int imageIndex = -1;

    CString ttemp;
    switch(me->dwFlags)
    {
        case LF32_BIGBLOCK: ttemp = _T("BIG"); imageIndex = 0; break;
        case LF32_DECOMMIT: ttemp = _T("DECOMMIT"); imageIndex = 1; break;
        case LF32_FIXED: ttemp = _T("FIXED"); imageIndex = 2; break;
        case LF32_FREE: ttemp = _T("FREE"); imageIndex = 3; break;
        default: ttemp = _T("?"); break;
    }

    CString temp;
    temp.Format(_T("%08p"), me->dwAddress);
    int nIndex = this->m_peList.InsertItem(0, temp, imageIndex);
    this->m_peList.SetItemData(nIndex, (DWORD)me->hHandle);

    //temp.Format(_T("%X"), me->hHandle);
    //this->m_peList.SetItemText(nIndex, 1, temp);
    
    temp.Format(_T("%d"), me->dwBlockSize);
    this->m_peList.SetItemText(nIndex, 1, temp);
	memorySize += me->dwBlockSize;

    temp.Format(_T("%d"), me->dwLockCount);
    this->m_peList.SetItemText(nIndex, 2, temp);

    this->m_peList.SetItemText(nIndex, 3, ttemp);

    //temp.Format(_T("%d"), me->th32HeapID);
    //this->m_peList.SetItemText(nIndex, 5, temp);

}

void CPeDlg::PrepareList(int mode, LPCTSTR name)
{
    this->mode = mode;

    int split = 4;
    
    if(this->m_peList.GetItemCount() != 0) this->m_peList.DeleteAllItems();
    
    for(int i = 5; i >= 0; i--)
	{    
		this->m_peList.DeleteColumn(i);
	}

    CRect wrc;
    this->GetWindowRect(&wrc);
	CString tname;
    
    switch(mode)
    {
		/*
	case MODE_FOLDERS:
		this->SetWindowText(_T("System Folders"));
        this->m_btnRef.SetWindowText(BTN_BACK);
		this->m_peList.InsertColumn(0, _T("Folder"), LVCFMT_LEFT);
        this->m_peList.InsertColumn(1, _T("Path"), LVCFMT_LEFT);
		split = 2;
		break;
		*/
    case MODE_PROCESS:
        this->SetWindowText(APP_NAME);
        this->m_btnRef.SetWindowText(_T("Refresh"));

        this->m_peList.InsertColumn(0, name, LVCFMT_LEFT);
        this->m_peList.InsertColumn(1, _T("PID"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(2, _T("Threads"), LVCFMT_RIGHT);
		this->m_peList.InsertColumn(3, _T("Base Address"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(4, _T("Path"), LVCFMT_LEFT);

		split = 5;
 
        break;
    case MODE_MODULE:
        this->SetWindowText(_T("Module Explorer"));
        this->m_btnRef.SetWindowText(BTN_BACK);

        this->m_peList.InsertColumn(0, (name != NULL ? name : _T("Module")), LVCFMT_LEFT);
        this->m_peList.InsertColumn(1, _T("Base Address"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(2, _T("Local / System Usage"), LVCFMT_RIGHT);
		this->m_peList.InsertColumn(3, _T("Size (bytes)"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(4, _T("Path"), LVCFMT_LEFT);

		split = 5;

        break;
    case MODE_WINDOW:

        this->SetWindowText(_T("Window Explorer"));
        this->m_btnRef.SetWindowText(BTN_BACK);

        this->m_peList.InsertColumn(0, name, LVCFMT_LEFT);
        this->m_peList.InsertColumn(1, _T("Visible"), LVCFMT_LEFT);
		this->m_peList.InsertColumn(2, _T("Class"), LVCFMT_LEFT);
        this->m_peList.InsertColumn(3, _T("Handle"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(4, _T("Size WxH"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(5, _T("Process ID"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(6, _T("EXE Path"), LVCFMT_LEFT);

        split = 7;

        break;
    case MODE_MEMORY:

        this->SetWindowText(_T("Memory Explorer"));
        this->m_btnRef.SetWindowText(BTN_BACK);

        tname = (name != NULL ? name : _T("Heap"));
        tname += _T(" Address");

        this->m_peList.InsertColumn(0, tname, LVCFMT_LEFT);
        //this->m_peList.InsertColumn(1, _T("Handle"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(1, _T("Size Bytes"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(2, _T("Lock Count"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(3, _T("Status"), LVCFMT_LEFT);
        //this->m_peList.InsertColumn(5, _T("TH Id"), LVCFMT_RIGHT);

        split = 4;

        break;
	case MODE_THREADS:

        this->SetWindowText(_T("Thread Explorer"));
        this->m_btnRef.SetWindowText(BTN_BACK);

        this->m_peList.InsertColumn(0, _T("Process"), LVCFMT_LEFT);
        this->m_peList.InsertColumn(1, _T("Thread ID"), LVCFMT_RIGHT);
		this->m_peList.InsertColumn(2, _T("Usage Count"), LVCFMT_RIGHT);
        //this->m_peList.InsertColumn(3, _T("Owner ID"), LVCFMT_RIGHT);
        this->m_peList.InsertColumn(3, _T("Current Process"), LVCFMT_LEFT);
        this->m_peList.InsertColumn(4, _T("Priority"), LVCFMT_RIGHT);

        split = 5;

        break;
    }
    
    int width = wrc.Width() / split;
    if(width <= 10) width = 20;
    for(i = 0; i < split; ++i)
    {
        this->m_peList.SetColumnWidth(i, (i == (split - 1)) ? width - 20 : width);
    }

    this->m_images.DeleteImageList();
    this->m_images.Create(16, 16, ILC_COLOR, 0, 8);
    this->m_peList.SetImageList(&this->m_images, LVSIL_SMALL);

    if(mode == MODE_MEMORY)
    {
        this->m_images.Add(IC_MEMBIG);
        this->m_images.Add(IC_MEMDC);
        this->m_images.Add(IC_MEMFIXED);
        this->m_images.Add(IC_MEMFREE);
    }
	/*
	else if(mode == MODE_FOLDERS)
	{
		this->m_images.Add(IC_FOLDER);
	}
	*/
}

void CPeDlg::ErrorMessage(LPCTSTR msg)
{
    ::MessageBox(this->m_hWnd, msg, APP_NAME, MB_ICONERROR);
}

void CPeDlg::EnableAll(BOOL on)
{
    CString s = _T("");
    if(on)
	{ 
		s.Format(_T("%d item(s)"), this->m_peList.GetItemCount());
		if((mode == MODE_MEMORY) && (memorySize > 0))
		{
			CString temp;
			temp.Format(_T(" %ld byte(s)"), memorySize);
			s += temp;
		}
		else if(mode == MODE_PROCESS)
		{
			s += _T(" ");
			s += URL;
		}
	}
    this->m_count.SetWindowText(s);
    this->m_count.ShowWindow(on ? SW_SHOW : SW_HIDE);
    this->m_btnRef.EnableWindow(on);
}

void CPeDlg::OnShowProcessWins() 
{
	EnableAll(FALSE);
    FillWindowsList(TRUE);
    EnableAll(TRUE);
}

void CPeDlg::OnShowWindows() 
{
    EnableAll(FALSE);
    FillWindowsList(FALSE);
    EnableAll(TRUE);
}

bool showHidden;
DWORD filterPID;
BOOL useFilterPID;

void CPeDlg::FillWindowsList(BOOL filter)
{
    
    CWaitCursor wait;
	this->lastPID = this->GetSelectedPID();
	useFilterPID = FALSE;
	if(filter)
	{
		filterPID = this->lastPID;
		useFilterPID = TRUE;
	}

	showHidden = TRUE;
    if(::MessageBox(this->m_hWnd, _T("Show only visible windows?"), APP_NAME, MB_ICONQUESTION | MB_YESNO) == IDYES)
        showHidden = FALSE;
    
    PrepareList(MODE_WINDOW, _T("Window"));

    LPARAM lParam = (LPARAM)&this->m_peList;
    if(!EnumWindows((WNDENUMPROC)&EnumWindowsProc, lParam))
    {
        ErrorMessage(_T("Cannot collect application information!"));
        return;
    }

}

BOOL CALLBACK EnumWindowsProc(HWND win, LPARAM lParam)
{
    if(win == NULL) return TRUE;

    if(!showHidden && !IsWindowVisible(win)) return TRUE;

	DWORD pid = 0;
    GetWindowThreadProcessId(win, &pid);
	if(useFilterPID && (pid != filterPID)) return TRUE;

    CListCtrl* list = (CListCtrl*)lParam;
    TCHAR title[256];
	::memset(title, 0, sizeof(TCHAR) * MAX_PATH);
    ::GetWindowText(win, (LPTSTR)&title, 256);
    title[255] = _T('\0');
    CString temp = title;

    if(!showHidden && (temp.GetLength() == 0)) return TRUE;

    if(temp.GetLength() == 0) temp = _T("?");

    RECT wr;
    int w = 0, h = 0;
    if(GetWindowRect(win, &wr))
    {
        w = abs(wr.left - wr.right);
        h = abs(wr.top - wr.bottom);
        if(!showHidden && (w == 0) && (h == 0)) return TRUE;
    }

    CString path;

    int imageIndex = -1;
    HANDLE process = OpenProcess(0, FALSE, pid);
    if(process != NULL)
    {
   
        TCHAR buff[MAX_PATH];
		::memset(buff, 0, sizeof(TCHAR) * MAX_PATH);
        ::GetModuleFileName((HMODULE)process, (LPWSTR)&buff, MAX_PATH);
        CloseHandle(process);
        path.Format(_T("%s"), buff);
        HICON icon[1];
        ExtractIconEx(path, 0, NULL, &icon[0], 1); 
        if(icon[0] != NULL)
        {
            CImageList* imgList = list->GetImageList(LVSIL_SMALL);
            if(imgList != NULL)  imageIndex = imgList->Add(icon[0]);
            DestroyIcon(icon[0]);
        }
    }
        
    int nIndex = list->InsertItem(0, temp, imageIndex);
    list->SetItemData(nIndex, pid);

    temp = _T("Hidden");
    if(IsWindowVisible(win)) temp = _T("Visible");
    list->SetItemText(nIndex, 1, temp);

	temp = _T("");
	TCHAR buff[MAX_PATH];
	::memset(buff, 0 , sizeof(TCHAR) * MAX_PATH);
	if(::GetClassName(win, buff, MAX_PATH))
	{
		temp.Format(_T("%s"), buff);
	}
    list->SetItemText(nIndex, 2, temp);

    temp.Format(_T("%X"), win);
    list->SetItemText(nIndex, 3, temp);

    temp.Format(_T("%dx%d"), w, h);
    list->SetItemText(nIndex, 4, temp);

    temp.Format(_T("%u"), pid);
    list->SetItemText(nIndex, 5, temp);

    list->SetItemText(nIndex, 6, path);
 
    return TRUE;
}


void CPeDlg::OnShowInfo() 
{
    CString msg;
    MEMORYSTATUS ms;
    ms.dwLength = sizeof(ms);

    GlobalMemoryStatus(&ms);

    msg.Format(_T("\nLoad\t%d%%\n\nPhysical total\t%u\nPhysical avail.\t%u\nVirtual total\t%u\nVirtual avail.\t%u\nPagefile total\t%u\nPagefile avail.\t%u\n\n(sizes are in bytes)\n\nPocketPC Process Explorer\nVersion 1.02 (c) 2005\n%s"),
        ms.dwMemoryLoad, ms.dwTotalPhys, ms.dwAvailPhys,
        ms.dwTotalVirtual, ms.dwAvailVirtual,
        ms.dwTotalPageFile, ms.dwAvailPageFile, URL);

    ::MessageBox(this->m_hWnd, (LPCTSTR)msg, _T("System Memory Report"), MB_OK | MB_ICONINFORMATION);
}

BOOL CPeDlg::DestroyWindow() 
{
	this->popup.DestroyMenu();
	DestroyIcon(IC_MEMBIG);
	DestroyIcon(IC_MEMDC);
	DestroyIcon(IC_MEMFIXED);
	DestroyIcon(IC_MEMFREE);
	//DestroyIcon(IC_FOLDER);
	
	return CDialog::DestroyWindow();
}

HWND CPeDlg::GetSelectedWindowHandle(int nSelected)
{
	if(nSelected < 0) return 0;
	CString strWin = this->m_peList.GetItemText(nSelected, 3);
	strWin = _T("0x") + strWin;
	PTCHAR dummy;
	return (HWND)_tcstol(strWin, &dummy , 16);
}

void CPeDlg::OnWinActivate() 
{
	int nSelected = this->m_peList.GetSelectionMark();
    if(nSelected < 0) return;
	HWND hWin = GetSelectedWindowHandle(nSelected);
	if(!::SetForegroundWindow(hWin))
	{
		ErrorMessage(_T("Cannot activate window!"));
	}
}

void CPeDlg::OnWinGotoProcess() 
{
	int nSelected = this->m_peList.GetSelectionMark();
    if(nSelected < 0) return;
	CString msg;
	msg.Format(_T("Switch to process of [%s]?\n%s"),
	this->m_peList.GetItemText(nSelected, 0),
	this->m_peList.GetItemText(nSelected, 6));
	if(::MessageBox(this->m_hWnd, msg, APP_NAME, MB_ICONQUESTION | MB_YESNO) != IDYES) return;
	DWORD pid = this->m_peList.GetItemData(nSelected);
	OnButtonRefresh();

	if(!GoToProcess(pid))
		ErrorMessage(_T("Cannot find process!"));
}

/*
void CPeDlg::OnAbout() 
{
	::MessageBox(this->m_hWnd, _T("PocketPC Process Explorer"), APP_NAME, MB_ICONINFORMATION | MB_OK);
}


void CPeDlg::OnShowFolders() 
{
	EnableAll(FALSE);
    FillSystemFolders();
    EnableAll(TRUE);
}

void CPeDlg::FillSystemFolders()
{
	this->lastPID = this->GetSelectedPID();
	CWaitCursor wait;
	PrepareList(MODE_FOLDERS, NULL);

	InsertSystemFolder(CSIDL_APPDATA, _T("Application Data"));
	InsertSystemFolder(CSIDL_BITBUCKET, _T("Recycle Bin"));
	InsertSystemFolder(CSIDL_CONTROLS, _T("Control Panel"));
	InsertSystemFolder(CSIDL_DESKTOPDIRECTORY, _T("Desktop"));
	InsertSystemFolder(CSIDL_FAVORITES , _T("Favorites"));
	InsertSystemFolder(CSIDL_NETHOOD, _T("Network"));
	InsertSystemFolder(CSIDL_PERSONAL, _T("My Documents"));
	InsertSystemFolder(CSIDL_PROGRAMS, _T("Programs"));
	InsertSystemFolder(CSIDL_RECENT, _T("Recent"));
	InsertSystemFolder(CSIDL_STARTMENU, _T("Start Menu"));
	InsertSystemFolder(CSIDL_STARTUP, _T("Start Up"));
	InsertSystemFolder(CSIDL_TEMPLATES , _T("Templates"));

}

void CPeDlg::InsertSystemFolder(int folder, PTCHAR name)
{
	TCHAR path[MAX_PATH];
	::memset(path, 0, sizeof(TCHAR) * MAX_PATH);
	if(!SHGetSpecialFolderPath(this->m_hWnd, path, folder, FALSE)) return;

	int imageIndex = 0;

	CString temp = path;
	int nIndex = this->m_peList.InsertItem(0, name , imageIndex);
    this->m_peList.SetItemText(nIndex, 1, temp);
}
*/

void CPeDlg::OnShowThreads() 
{
	EnableAll(FALSE);
    FillThreads();
    EnableAll(TRUE);
}

void CPeDlg::FillThreads()
{
	this->lastPID = this->GetSelectedPID();
	CWaitCursor wait;
	PrepareList(MODE_THREADS, NULL);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if(snapshot == INVALID_HANDLE_VALUE)
    {
        ErrorMessage(_T("Cannot get process information!"));
        return;
    }
    
    THREADENTRY32 me;
    me.dwSize = sizeof(me);
    
    if(!Thread32First(snapshot, &me))
    {
       ErrorMessage(_T("Cannot parse thread information!"));
       CloseToolhelp32Snapshot(snapshot);
       return;
    }
   
    InsertThreadInfo(&me);

    while(TRUE)
    {
        me.dwSize = sizeof(me);
        
        if(!Thread32Next(snapshot, &me)) break;
        if(GetLastError() == ERROR_NO_MORE_FILES) break;

        InsertThreadInfo(&me);

        
    }

    if(snapshot != INVALID_HANDLE_VALUE){
       if(!CloseToolhelp32Snapshot(snapshot))
       { 
           ErrorMessage(_T("Cannot free process information!"));
           return;
       }
    }
}

void CPeDlg::InsertThreadInfo(THREADENTRY32 *me)
{
	if(me == NULL) return;
    int imageIndex = -1;
     
    CString temp, path, processName;

	temp = _T("");
	HANDLE process = OpenProcess(0, FALSE, me->th32OwnerProcessID);
    if(process != NULL)
    {
   
        TCHAR buff[MAX_PATH];
		::memset(buff, 0, MAX_PATH * sizeof(TCHAR));
        ::GetModuleFileName((HMODULE)process, (LPWSTR)&buff, MAX_PATH);
        CloseHandle(process);
        path.Format(_T("%s"), buff);
		temp = path;
		int i = path.ReverseFind(_T('\\'));
		if(i >= 0)
		{
			i = path.GetLength() - i - 1;
			if(i > 0)
			temp = path.Right(i);
		}	
		HICON icon[1];
		ExtractIconEx(buff, 0, NULL, &icon[0], 1); 
		if(icon[0] != NULL)
		{
			imageIndex = this->m_images.Add(icon[0]);
			DestroyIcon(icon[0]);
		}
    }
    int nIndex = this->m_peList.InsertItem(0, temp, imageIndex);
    this->m_peList.SetItemData(nIndex, me->th32CurrentProcessID);
	processName = temp;

    temp.Format(_T("%u"), me->th32ThreadID);
    this->m_peList.SetItemText(nIndex, 1, temp);
    
    temp.Format(_T("%u"), me->cntUsage);
    this->m_peList.SetItemText(nIndex, 2, temp);

	//temp.Format(_T("%u"), me->th32OwnerProcessID);
    //this->m_peList.SetItemText(nIndex, 3, temp);

	temp = processName;
	if(me->th32OwnerProcessID != me->th32CurrentProcessID)
	{
		temp.Format(_T("%u"), me->th32CurrentProcessID);
		HANDLE process = OpenProcess(0, FALSE, me->th32CurrentProcessID);
		
		if(process != NULL){
			TCHAR buff[MAX_PATH];
			::memset(buff, 0, MAX_PATH * sizeof(TCHAR));
			::GetModuleFileName((HMODULE)process, (LPWSTR)&buff, MAX_PATH);
			CloseHandle(process);
			temp.Format(_T("%s"), buff);
			int i = temp.ReverseFind(_T('\\'));
			if(i >= 0)
			{
				i = temp.GetLength() - i - 1;
				if(i > 0)
				temp = temp.Right(i);
				temp = _T("!") + temp;
			}
		}
	}
    this->m_peList.SetItemText(nIndex, 3, temp);
	temp.Format(_T("%u"), (me->tpBasePri + me->tpDeltaPri));
    this->m_peList.SetItemText(nIndex, 4, temp);
}
