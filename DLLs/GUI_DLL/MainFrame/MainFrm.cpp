//******************************************************************************
//
// This file is part of the OpenHoldem project
//    Source code:           https://github.com/OpenHoldem/openholdembot/
//    Forums:                http://www.maxinmontreal.com/forums/index.php
//    Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//******************************************************************************
//
// Purpose:
//
//******************************************************************************

#define GUI_DLL_EXPORTS

#include "MainFrm.h"
#include <io.h>
#include <process.h>
#include "OpenHoldemDoc.h"
#include "..\CGUI.h"
#include "..\formula_editor\DialogFormulaScintilla.h"
#include "..\dialog_scraper_output\DialogScraperOutput.h"
#include "..\OpenHoldem_title\COpenHoldemTitle.h"
#include "..\statusbar\COpenHoldemStatusbar.h"
#include "..\..\ConfigurationCheck_DLL\CProblemSolver.h"
#include "..\..\Debug_DLL\debug.h"
#include "..\..\Files_DLL\Files.h"
#include "..\..\Preferences_DLL\Preferences.h"
#include "..\..\Scraper_DLL\CBasicScraper.h"
#include "..\..\SessionCounter_DLL\CSessionCounter.h"
#include "..\..\StringFunctions_DLL\string_functions.h"
#include "..\..\Symbols_DLL\CEngineContainer.h"
#include "..\..\Symbols_DLL\CSymbolEngineReplayFrameController.h"
#include "..\..\Symbols_DLL\CSymbolEngineUserchair.h"
#include "..\..\Symbols_DLL\CSymbolEngineTableLimits.h"
#include "..\..\WindowFunctions_DLL\window_functions.h"
#include "..\formula_editor\DialogFormulaScintilla.h"
#include "..\preferences_dialog\DialogSAPrefs2.h"
#include "..\preferences_dialog\DialogSAPrefs3.h"
#include "..\preferences_dialog\DialogSAPrefs4.h"
#include "..\preferences_dialog\DialogSAPrefs6.h"
#include "..\preferences_dialog\DialogSAPrefs7.h"
#include "..\preferences_dialog\DialogSAPrefs8.h"
#include "..\preferences_dialog\DialogSAPrefs9.h"
#include "..\preferences_dialog\DialogSAPrefs10.h"
#include "..\preferences_dialog\DialogSAPrefs11.h"
#include "..\preferences_dialog\DialogSAPrefs12.h"
#include "..\preferences_dialog\DialogSAPrefs13.h"
#include "..\preferences_dialog\DialogSAPrefs14.h"
#include "..\preferences_dialog\DialogSAPrefs15.h"
#include "..\preferences_dialog\DialogSAPrefs16.h"
#include "..\preferences_dialog\DialogSAPrefs17.h"
#include "..\preferences_dialog\DialogSAPrefs19.h"
#include "..\preferences_dialog\DialogSAPrefs20.h"
#include "..\preferences_dialog\DialogSAPrefs21.h"
#include "..\preferences_dialog\DialogSAPrefs22.h"
#include "..\dialog_scraper_output\DialogScraperOutput.h"
#include "..\..\TableManagement_DLL\CAutoConnector.h"
#include "..\..\TableManagement_DLL\CTableManagement.h"

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()

	// Menu updates
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, &CMainFrame::OnUpdateMenuFileNew)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CMainFrame::OnUpdateMenuFileOpen)
  ON_UPDATE_COMMAND_UI(ID_EDIT_FORMULA, &CMainFrame::OnUpdateMenuFileEdit)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOOTREPLAYFRAME, &CMainFrame::OnUpdateViewShootreplayframe)
  ON_UPDATE_COMMAND_UI(ID_VIEW_SCRAPEROUTPUT, &CMainFrame::OnUpdateViewScraperOutput)

	//  Menu commands
	ON_COMMAND(ID_FILE_OPEN, &CMainFrame::OnFileOpen)
	ON_COMMAND(ID_EDIT_FORMULA, &CMainFrame::OnEditFormula)
	ON_COMMAND(ID_EDIT_PREFERENCES, &CMainFrame::OnEditPreferences)
	ON_COMMAND(ID_EDIT_VIEWLOG, &CMainFrame::OnEditViewLog)
	ON_COMMAND(ID_EDIT_TAGLOG, &CMainFrame::OnEditTagLog)
  ON_COMMAND(ID_EDIT_CLEARLOG, &CMainFrame::OnEditClearLog)
	ON_COMMAND(ID_VIEW_SCRAPEROUTPUT, &CMainFrame::OnScraperOutput)
	ON_COMMAND(ID_VIEW_SHOOTREPLAYFRAME, &CMainFrame::OnViewShootreplayframe)
	ON_COMMAND(ID_HELP_HELP, &CMainFrame::OnHelp)
	ON_COMMAND(ID_HELP_OPEN_PPL, &CMainFrame::OnHelpOpenPPL)
	ON_COMMAND(ID_HELP_FORUMS, &CMainFrame::OnHelpForums)
	ON_COMMAND(ID_HELP_PROBLEMSOLVER, &CMainFrame::OnHelpProblemSolver)

	// Main toolbar 
	ON_BN_CLICKED(ID_MAIN_TOOLBAR_AUTOPLAYER, &CMainFrame::OnAutoplayer)
	ON_BN_CLICKED(ID_MAIN_TOOLBAR_FORMULA, &CMainFrame::OnEditFormula)
	ON_BN_CLICKED(ID_MAIN_TOOLBAR_VALIDATOR, &CMainFrame::OnValidator)
	ON_BN_CLICKED(ID_MAIN_TOOLBAR_TAGLOGFILE, &CMainFrame::OnEditTagLog)
	ON_BN_CLICKED(ID_MAIN_TOOLBAR_SCRAPER_OUTPUT, &CMainFrame::OnScraperOutput)
	ON_BN_CLICKED(ID_MAIN_TOOLBAR_SHOOTFRAME, &CMainFrame::OnViewShootreplayframe)
  ON_BN_CLICKED(ID_MAIN_TOOLBAR_MANUALMODE, &CMainFrame::OnManualMode)
	ON_BN_CLICKED(ID_MAIN_TOOLBAR_HELP, &CMainFrame::OnHelp)
  /*#
	// Hopper
	ON_MESSAGE(WMA_SETWINDOWTEXT, &COpenHoldemHopperCommunication::OnSetWindowText)
	ON_MESSAGE(WMA_DOCONNECT,     &COpenHoldemHopperCommunication::OnConnectMessage)
	ON_MESSAGE(WMA_DODISCONNECT,  &COpenHoldemHopperCommunication::OnDisconnectMessage)
	ON_MESSAGE(WMA_CONNECTEDHWND, &COpenHoldemHopperCommunication::OnConnectedHwndMessage)
	ON_MESSAGE(WMA_SETFLAG,       &COpenHoldemHopperCommunication::OnSetFlagMessage)
	ON_MESSAGE(WMA_RESETFLAG,     &COpenHoldemHopperCommunication::OnResetFlagMessage)
	ON_MESSAGE(WMA_ISREADY,       &COpenHoldemHopperCommunication::OnIsReadyMessage)
  */
	// Flags
	ON_BN_CLICKED(ID_NUMBER0,  &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER1,  &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER2,  &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER3,  &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER4,  &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER5,  &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER6,  &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER7,  &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER8,  &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER9,  &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER10, &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER11, &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER12, &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER13, &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER14, &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER15, &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER16, &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER17, &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER18, &CMainFrame::OnClickedFlags)
	ON_BN_CLICKED(ID_NUMBER19, &CMainFrame::OnClickedFlags)

	ON_WM_TIMER()
  ON_UPDATE_COMMAND_UI(ID_INDICATOR_STATUS_ACTION, OnUpdateStatus)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_STATUS_HANDRANK,OnUpdateStatus)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_STATUS_PRWIN,OnUpdateStatus)
	
	ON_WM_SETCURSOR()
	ON_WM_SYSCOMMAND()

  // OpenHoldem!!!!! -> GUI
  ON_COMMAND(ID_APP_ABOUT, &CGUI::OnAbout)
END_MESSAGE_MAP()

// CMainFrame construction/destruction
CMainFrame::CMainFrame() {
	_wait_cursor = false;

	_prev_att_rect.bottom = 0;
	_prev_att_rect.left = 0;
	_prev_att_rect.right = 0;
	_prev_att_rect.top = 0;
	_prev_wrect.bottom = 0;
	_prev_wrect.left = 0;
	_prev_wrect.right = 0;
	_prev_wrect.top = 0;
}

CMainFrame::~CMainFrame() {
  /// Delete GUI!!!
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	CString			t = "";
	lpCreateStruct->dwExStyle |= WS_MINIMIZE;
	if (CFrameWnd::OnCreate(lpCreateStruct) == kUndefined)
		return -1;
  GUI()->CreateFlagsToolbar(this);
  GUI()->CreateStatusbar(this);
  /// needed ???
  // Start timer that checks for continued existence of attached HWND 		
  SetTimer(HWND_CHECK_TIMER, 200, 0);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) {
	if ( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	int			max_x = 0, max_y = 0;

	WNDCLASS wnd;
	HINSTANCE hInst = AfxGetInstanceHandle();

	// Set class name
	if (!(::GetClassInfo(hInst, Preferences()->window_class_name(), &wnd))) {
		wnd.style			    = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wnd.lpfnWndProc		= ::DefWindowProc;
		wnd.cbClsExtra		= wnd.cbWndExtra = 0;
		wnd.hInstance		  = hInst;
		wnd.hIcon			    = AfxGetApp()->LoadIcon(IDI_ICON1);
		wnd.hCursor			  = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		wnd.hbrBackground	= (HBRUSH) (COLOR_3DFACE + 1);
		wnd.lpszMenuName	= NULL;
		wnd.lpszClassName	= Preferences()->window_class_name();
    // Fixed size window, not resizable 
    // Because bad-sized windows are annoying
    // and because of potential support for a 4th user-card ;-)
    // http://arstechnica.com/civis/viewtopic.php?f=20&t=848676
    // http://msdn.microsoft.com/en-us/library/aa925944.aspx
    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	  cs.style &= (0xFFFFFFFF ^ WS_SIZEBOX);
	  cs.style |= WS_BORDER;
	  cs.style &= (0xFFFFFFFF ^ WS_MAXIMIZEBOX);

		AfxRegisterClass( &wnd );
	}
	cs.lpszClass = Preferences()->window_class_name();

	// Restore window location and size
  // -32 to avoid placement directly under the taskbar,
  // so that at least a little bit is visible
  // if the values in the ini-file are out of range.
	max_x = GetSystemMetrics(SM_CXSCREEN) - GetSystemMetrics(SM_CXICON - 32);
	max_y = GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYICON - 32);
  // Make sure that our coordinates are not out of screen
  // (too large or even negative)
	cs.x = min(Preferences()->main_x(), max_x);
	cs.y = min(Preferences()->main_y(), max_y);
  cs.x = max(cs.x, 0);
  cs.y = max(cs.y, 0);
  // GUI size
	cs.cx = kMainSizeX;
	cs.cy = kMainSizeY;
	return true;
}

// CMainFrame message handlers

void CMainFrame::OnEditFormula() {
  if (GUI()->FormulaEditor()->IsCreated()) {
    GUI()->FormulaEditor()->DestroyEditor();
  } else {
    GUI()->FormulaEditor()->CreateEditor();
  }
}

void CMainFrame::OnEditViewLog() {
  assert(SessionCounter() != nullptr);
  OpenFileInExternalSoftware(LogFilePath(SessionCounter()->session_id()));
}

void CMainFrame::OnEditTagLog() {
	write_log(k_always_log_basic_information,
		"[*** ATTENTION ***] User tagged this situation for review\n");
}

void CMainFrame::OnEditClearLog() {
  clear_log();
}

// Menu -> Edit -> View Scraper Output
void CMainFrame::OnScraperOutput() {
	if (GUI()->DlgScraperOutput()) {
		write_log(Preferences()->debug_gui(), "[GUI] m_DlgScraperOutput = %i\n", GUI()->DlgScraperOutput());
		write_log(Preferences()->debug_gui(), "[GUI] Going to destroy existing scraper output dialog\n");
		BOOL	bWasShown = ::IsWindow(GUI()->DlgScraperOutput()->m_hWnd) && GUI()->DlgScraperOutput()->IsWindowVisible();
		write_log(Preferences()->debug_gui(), "[GUI] Scraper output dialog was visible: %s\n", Bool2CString(bWasShown));
    CDlgScraperOutput::DestroyWindowSafely();
		if (bWasShown) {
			write_log(Preferences()->debug_gui(), "[GUI] Scraper output dialog destroyed; going to return\n");
			return;
		}
	}	else {
		write_log(Preferences()->debug_gui(), "[GUI] Scraper output dialog does not yet exist\n");
	}
  GUI()->CreateDialogScraperOutput(this);
	
}

void CMainFrame::OnViewShootreplayframe() {
	EngineContainer()->symbol_engine_replayframe_controller()->ShootReplayFrameIfNotYetDone();
}

void CMainFrame::OnManualMode() {
  // Manualmode usually is in the same directory.
  // No error-checking here~> if it does not work, then we silently fail.
  // http://msdn.microsoft.com/en-us/library/windows/desktop/bb762153%28v=vs.85%29.aspx
  ShellExecute(
    NULL,               // Pointer to parent window; not needed
    "open",             // "open" == "execute" for an executable
    ManualModePath(),
		NULL, 		          // Parameters
		ToolsDirectory(), // Working directory, location of ManualMode
		SW_SHOWNORMAL);		  // Active window, default size
}

void CMainFrame::OnEditPreferences() {
	//CDlgPreferences  myDialog;
	CSAPrefsDialog dlg;

	// the "pages" (all derived from CSAPrefsSubDlg)
	CDlgSAPrefs2 page2;
	CDlgSAPrefs3 page3;
	CDlgSAPrefs4 page4;
	CDlgSAPrefs6 page6;
  CDlgSAPrefs7 page7;
	CDlgSAPrefs8 page8;
  CDlgSAPrefs9 page9;
	CDlgSAPrefs10 page10;
	CDlgSAPrefs11 page11;
	CDlgSAPrefs12 page12;
	CDlgSAPrefs13 page13;
	CDlgSAPrefs14 page14;
	CDlgSAPrefs15 page15;
	CDlgSAPrefs16 page16;
	CDlgSAPrefs17 page17;
	CDlgSAPrefs19 page19;
	CDlgSAPrefs20 page20;
	CDlgSAPrefs21 page21;
	CDlgSAPrefs22 page22;

	// add pages
	dlg.AddPage(page14, "Auto-Connector");
	dlg.AddPage(page2,  "Autoplayer");
  dlg.AddPage(page9, "Auto-starter");
	dlg.AddPage(page10, "Chat");
	dlg.AddPage(page17, "Configuration Check");
	dlg.AddPage(page20, "Debugging");
	dlg.AddPage(page3,  "DLL Extension");
	dlg.AddPage(page15, "GUI");
	dlg.AddPage(page19, "Handhistory Generator");
	dlg.AddPage(page11, "Logging");
	dlg.AddPage(page6,  "Poker Tracker v4");
	dlg.AddPage(page22, "Popup Blocker");
	dlg.AddPage(page16, "Rebuy");
	dlg.AddPage(page8,  "Replay Frames");
	dlg.AddPage(page4,  "Scraper");
	dlg.AddPage(page13, "Stealth");
	dlg.AddPage(page21, "Table Positioner");
	dlg.AddPage(page12, "Validator");	

  // this one will be a child node on the tree
	// (&page3 specifies the parent)
	//dlg.AddPage(dlg4, "Page 4", &page3);

	// the prefs dialog title
	dlg.SetTitle("OpenHoldem Preferences");

	// text drawn on the right side of the shaded
	// page label. this does not change when the
	// pages change, hence "constant".
	dlg.SetConstantText("Preferences");

	// launch it like any other dialog...
	//dlg1.m_csText = m_csStupidCString;
	if (dlg.DoModal()==IDOK) {
		//m_csStupidCString = dlg1.m_csText;
	}
}

BOOL CMainFrame::DestroyWindow() {
  KillTimers();
	// Save window position
  WINDOWPLACEMENT wp;
	GetWindowPlacement(&wp); 		
	Preferences()->SetValue(k_prefs_main_x, wp.rcNormalPosition.left); 		
 	Preferences()->SetValue(k_prefs_main_y, wp.rcNormalPosition.top);
  write_log(Preferences()->debug_gui(), "[GUI] Going to delete the GUI\n");
  write_log(Preferences()->debug_gui(), "[GUI] this = [%i]\n", this);
  // All OK here
  assert(AfxCheckMemory());
  // http://www.maxinmontreal.com/forums/viewtopic.php?f=111&t=20459
  // probably caused by incorrect order of deletion,
  // caused by incorrect position of StopThreads and KillTimers.
  bool success = CFrameWnd::DestroyWindow(); 
  write_log(Preferences()->debug_gui(), "[GUI] Window deleted\n");
  return success;
}

void CMainFrame::OnFileOpen() {
  COpenHoldemDoc *pDoc = (COpenHoldemDoc *)this->GetActiveDocument();   
  if (!pDoc->SaveModified()) {
    return;        // leave the original one
  }
	CFileDialog			cfd(true);
  cfd.m_ofn.lpstrInitialDir = "";
  // http://msdn.microsoft.com/en-us/library/windows/desktop/ms646839%28v=vs.85%29.aspx
  cfd.m_ofn.lpstrFilter = "OpenHoldem Formula Files (*.ohf, *.oppl, *.txt)\0*.ohf;*.oppl;*.txt\0All files (*.*)\0*.*\0\0";
	cfd.m_ofn.lpstrTitle = "Select Formula file to OPEN";
	if (cfd.DoModal() == IDOK) {				
		pDoc->OnOpenDocument(cfd.GetPathName());
		pDoc->SetPathName(cfd.GetPathName());
		///AfxGetApp()->StoreLastRecentlyUsedFileList();
	}
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent) {
  write_log(Preferences()->debug_timers(), "[GUI] CMainFrame::OnTimer()\n");
  // There was a race-condition in this function during termination 
  // if OnTimer was in progress and TableManagement()->AutoConnector() became dangling.
  // This is probably fixed, as we now kill the timers
  // before we delete singleton, but we keep these safety-meassures.
  // It is OK to skip CWnd::OnTimer(nIDEvent); if we terminate.
  if (GUI()->FlagsToolbar() == NULL) {
    return;
  }
  if (GUI()->OpenHoldemStatusbar() == NULL) {
    return;
  }
  if (TableManagement()->AutoConnector() == NULL) {
    return;
  }
  if (nIDEvent == HWND_CHECK_TIMER) {
    write_log(Preferences()->debug_timers(), "[GUI] OnTimer checking table connection\n");
    // Important: check is_conected first.
    // Checking only garbage HWND, then disconnecting
    // can lead to freezing if it colludes with Connect()
 	  if (TableManagement()->AutoConnector()->IsConnectedToGoneWindow()) {
 	    // Table disappeared 		
      write_log(Preferences()->debug_timers(), "[GUI] OnTimer found disappeared window()\n");
 	    TableManagement()->AutoConnector()->Disconnect("table disappeared"); 		 		
    }
 	} else if (nIDEvent == ENABLE_BUTTONS_TIMER) {
		// Autoplayer
		// Since OH 4.0.5 we support autoplaying immediatelly after connection
		// without the need to know the userchair to act on secondary formulas.
    write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_E\n");
		if (TableManagement()->AutoConnector()->IsConnectedToAnything()) 	{
      write_log(Preferences()->debug_timers(), "[GUI] OnTimer enabling buttons\n");
      write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_F\n");
			GUI()->FlagsToolbar()->EnableButton(ID_MAIN_TOOLBAR_AUTOPLAYER, true);
      write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_G\n");
      GUI()->FlagsToolbar()->EnableButton(ID_MAIN_TOOLBAR_SHOOTFRAME, true);
      write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_L\n");
		}	else {
      write_log(Preferences()->debug_timers(), "[GUI] OnTimer disabling buttons\n");
      write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_H\n");
			GUI()->FlagsToolbar()->EnableButton(ID_MAIN_TOOLBAR_AUTOPLAYER, false);
      write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_I\n");
      GUI()->FlagsToolbar()->EnableButton(ID_MAIN_TOOLBAR_SHOOTFRAME, false);
      write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_N\n");
		}
    write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_O\n");
	}	else if (nIDEvent == UPDATE_STATUS_BAR_TIMER) {
    write_log(Preferences()->debug_timers(), "[GUI] OnTimer updating statusbar\n");
    write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_P\n");
		GUI()->OpenHoldemStatusbar()->OnUpdateStatusbar();
    write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_Q\n");
	}
  write_log(Preferences()->debug_alltherest(), "[GUI] location Johnny_R\n");
	CWnd::OnTimer(nIDEvent); 
}

void CMainFrame::OnAutoplayer() {
  bool is_autoplaying = GUI()->FlagsToolbar()->IsAutoplayerButtonChecked();
	// Toggle the autoplayer-state
  GUI()->FlagsToolbar()->CheckButton(ID_MAIN_TOOLBAR_AUTOPLAYER, !is_autoplaying);
}

void CMainFrame::OnValidator() {
	if (GUI()->FlagsToolbar()->IsButtonChecked(ID_MAIN_TOOLBAR_VALIDATOR)) {
		GUI()->FlagsToolbar()->CheckButton(ID_MAIN_TOOLBAR_VALIDATOR, true);
    ///EngineContainer-> new symbol-engine!
		///p_validator->SetEnabledManually(true);
	}	else {
		GUI()->FlagsToolbar()->CheckButton(ID_MAIN_TOOLBAR_VALIDATOR, false);
		///p_validator->SetEnabledManually(false);
	}
}

void CMainFrame::OnUpdateStatus(CCmdUI *pCmdUI) {
	GUI()->OpenHoldemStatusbar()->OnUpdateStatusbar();
}

BOOL CMainFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {
	if (_wait_cursor)	{
		RestoreWaitCursor();
		return TRUE;
	}
	return CFrameWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CMainFrame::OnClickedFlags() {
	GUI()->FlagsToolbar()->OnClickedFlags();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Menu updaters

void CMainFrame::OnUpdateMenuFileNew(CCmdUI* pCmdUI) {
	pCmdUI->Enable(!GUI()->FlagsToolbar()->IsAutoplayerButtonChecked());
}

void CMainFrame::OnUpdateMenuFileOpen(CCmdUI* pCmdUI) {
	pCmdUI->Enable(!GUI()->FlagsToolbar()->IsAutoplayerButtonChecked());
}

void CMainFrame::OnUpdateMenuFileEdit(CCmdUI* pCmdUI) {
	pCmdUI->Enable(!GUI()->FlagsToolbar()->IsAutoplayerButtonChecked());
}

void CMainFrame::OnUpdateDllLoadspecificfile(CCmdUI *pCmdUI) {
	pCmdUI->Enable(!GUI()->FlagsToolbar()->IsAutoplayerButtonChecked());
}

void CMainFrame::OnUpdateViewShootreplayframe(CCmdUI *pCmdUI) {
	pCmdUI->Enable(TableManagement()->AutoConnector()->IsConnectedToAnything());
}

void CMainFrame::OnUpdateViewScraperOutput(CCmdUI *pCmdUI) {
	pCmdUI->Enable(TableManagement()->AutoConnector()->IsConnectedToAnything());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Other functions

void CMainFrame::KillTimers() { 		
  // It is very important that we kill all timers as early as possible
  // on termination. otherwise the timer-functions might access
  // objects loke the auto_connector that already are destructed,
  // thus causing a memory-access-error.
  // http://www.maxinmontreal.com/forums/viewtopic.php?f=111&t=20459
 	CFrameWnd::KillTimer(HWND_CHECK_TIMER);
  CFrameWnd::KillTimer(ENABLE_BUTTONS_TIMER);
  CFrameWnd::KillTimer(UPDATE_STATUS_BAR_TIMER);
}

void CMainFrame::ResetDisplay() {
	InvalidateRect(NULL, true); 
}

void CMainFrame::OnHelp() {
  OpenFileInExternalSoftware(OpenHoldemManualpath());
}

void CMainFrame::OnHelpOpenPPL() {
  OpenFileInExternalSoftware(OpenPPLManualpath());
}

void CMainFrame::OnHelpForums() {
	ShellExecute(NULL, "open", "http://www.maxinmontreal.com/forums/index.php", "", "", SW_SHOWDEFAULT);
}

void CMainFrame::OnHelpProblemSolver() {
	CProblemSolver my_problem_solver;
	my_problem_solver.TryToDetectBeginnersProblems();
}