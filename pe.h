// pe.h : main header file for the PE application
//

#if !defined(AFX_PE_H__5DF93A58_C3DB_4737_9E7D_4DCB9F47962C__INCLUDED_)
#define AFX_PE_H__5DF93A58_C3DB_4737_9E7D_4DCB9F47962C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPeApp:
// See pe.cpp for the implementation of this class
//

class CPeApp : public CWinApp
{
public:
	CPeApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPeApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPeApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PE_H__5DF93A58_C3DB_4737_9E7D_4DCB9F47962C__INCLUDED_)
