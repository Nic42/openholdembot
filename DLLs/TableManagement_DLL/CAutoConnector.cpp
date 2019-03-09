//******************************************************************************
//
// This file is part of the OpenHoldem project
//    Source code:           https://github.com/OpenHoldem/openholdembot/
//    Forums:                http://www.maxinmontreal.com/forums/index.php
//    Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//******************************************************************************
//
// Purpose: automatically connecting to unserved poker-tables,
//   using shared memory and a mutex to synchronize with other instaces.
//
//******************************************************************************

#define TABLE_MANAGEMENT_DLL_EXPORTS

#include "CAutoConnector.h"
#include <afxwin.h>
#include "CSharedMem.h"
#include "CTableManagement.h"
#include "CTablePositioner.h"
#include "..\Debug_DLL\debug.h"
#include "..\GUI_DLL\CGUI.h"
#include "..\GUI_DLL\dialog_scraper_output\DialogScraperOutput.h"
#include "..\GUI_DLL\Toolbar\CFlagsToolbar.h"
#include "..\Numerical_Functions_DLL\NumericalFunctions.h"
#include "..\OpenHoldem_CallBack_DLL\OpenHoldem_CallBack.h"
#include "..\Preferences_DLL\Preferences.h"
#include "..\Scraper_DLL\CBasicScraper.h"
#include "..\Scraper_DLL\CTransform\CTransform.h"
#include "..\StringFunctions_DLL\string_functions.h"
#include "..\TableMapLoader_DLL\CTableMapLoader.h"
#include "..\Tablestate_DLL\TableState.h"
#include "..\WindowFunctions_DLL\window_functions.h"

struct STableList {
  HWND		hwnd;
  int     tablemap_index;
  RECT    attached_rect;
  RECT    crect;
  CString title;
};

CArray <STableList, STableList> g_tlist; 

CAutoConnector::CAutoConnector() {
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] CAutoConnector()\n");
  CString MutexName = CString(Preferences()->mutex_name()) + "AutoConnector";
	_autoconnector_mutex = new CMutex(false, MutexName);
}

CAutoConnector::~CAutoConnector() {
	// Releasing the mutex in case we hold it.
	// If we don't hold it, Unlock() will "fail" silently.
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] ~CAutoConnector()\n");
	_autoconnector_mutex->Unlock();
	if (_autoconnector_mutex != NULL)	{
    write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] ~CAutoConnector() Deleting auto-connector-mutex\n");
		delete _autoconnector_mutex;
		_autoconnector_mutex = NULL;
	}
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] ~CAutoConnector() Marking table as not atached\n");
	set_attached_hwnd(NULL);
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] ~CAutoConnector() Finished\n");
}

bool CAutoConnector::IsConnectedToAnything() {
  HWND table = attached_hwnd();
  bool result = (table != NULL);
  write_log(Preferences()->debug_autoconnector(), 
    "[CAutoConnector] IsConnectedToAnything: %s\n",
    Bool2CString(result));
	return result;
}

bool CAutoConnector::IsConnectedToExistingWindow() {
  if (!IsConnectedToAnything()) {
    return false;
  }
  HWND table = attached_hwnd();
  bool result = IsWindow(table);
  write_log(Preferences()->debug_autoconnector(), 
    "[CAutoConnector] IsConnectedToexistingWindow: %s\n",
    Bool2CString(result));
  return result;
}

bool CAutoConnector::IsConnectedToGoneWindow() {
  if (!IsConnectedToAnything()) {
    return false;
  }
  if (IsConnectedToExistingWindow()) {
    return false;
  }
  write_log(Preferences()->debug_autoconnector(), 
    "[CAutoConnector] IsConnectedToGoneWindow: true\n");
  return true;
}

void CAutoConnector::Check_TM_Against_All_Windows_Or_TargetHWND(int tablemap_index, HWND targetHWnd) {
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Check_TM_Against_All_Windows(..)\n");
  if (targetHWnd == NULL) {
		EnumWindows(EnumProcTopLevelWindowList, (LPARAM) tablemap_index);
  } else {
		EnumProcTopLevelWindowList(targetHWnd, (LPARAM) tablemap_index);
  }
}

void CAutoConnector::CheckIfWindowMatchesMoreThanOneTablemap(HWND hwnd) {
  // In OpenHoldem 9.0.1 we replaced clientsize in favour of clientsizemin/max
  // and targetsize. This caused the problem that some people used a very large
  // range of clientsizemin/max in combination with a very common titletext
  // like "Poker". As a consequence some tablemaps connected to nearly every
  // window (not even table).
  // To solve this problem we now detect if a table could be served 
  // by more than one tablemap. For performance reasons we do this exactly once
  // per table at connection.
  // http://www.maxinmontreal.com/forums/viewtopic.php?f=110&t=19407&start=90#p138038
  int num_loaded_tablemaps = NumberOfTableMapsLoaded();
  int num_matching_tablemaps = 0;
  CString matching_tablemaps = "";
  for (int i=0; i<num_loaded_tablemaps; ++i) {
    // See if it matches the specified table map
    if (Check_TM_Against_Single_Window(i, hwnd)) {
      ++num_matching_tablemaps;
      // Build "list" of matching tablemaps
      matching_tablemaps += "42";/// OpenHoldem()->TablemapLoader()->GetTablemapPathToLoad(i);
      matching_tablemaps += "\n";
    }
  }
  if (num_matching_tablemaps > 1) {
    char text[MAX_WINDOW_TITLE] = { 0 };
    GetWindowText(hwnd, text, sizeof(text));
    CString title = text;
    CString error_message;
    error_message.Format("%s%s%s%s%s%s%s%s%s",
      "More than one tablemap fits to the same table\n\n",
      matching_tablemaps,
      "\nTable: ", 
      title,
      "\n\nThese tablemaps need to be adapted:\n",
      "  * clientsizemin/max\n",
      "  * titletext(s)\n",
      "  * and/or tablepoints\n",
      "to make the tablemap-selection-process unambiguous.");
    MessageBox_Error_Warning(error_message);
  }
}

void CAutoConnector::set_attached_hwnd(const HWND table) {
  CSLock lock(m_critsec);
  _attached_hwnd = table;
  assert(TableManagement()->SharedMem() != NULL);
  TableManagement()->SharedMem()->MarkPokerWindowAsAttached(table);
}

BOOL CALLBACK EnumProcTopLevelWindowList(HWND hwnd, LPARAM lparam) {
	CString			title = "", winclass = "";
	char				text[MAX_WINDOW_TITLE] = {0};
	RECT				crect = {0};
	STableList	tablelisthold;
	int					tablemap_index = (int)(lparam);

	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] EnumProcTopLevelWindowList(..)\n");
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Tablemap nr. %d\n", tablemap_index);
  if (!IsWindowVisible(hwnd)) {
    return true;
  }
  // Since OH 11.1.0 We do no longer check for (GetParent(hwnd) != NULL) 
  // because we want OpenHoldem to be able to connect to popups
  // e.g. to click a confirmation-button
  // or maybe even do more complicated hopper-tasks in the future.
  write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] EnumProcTopLevelWindowList(..) found a window candidate...\n");
	// See if it matches the currently loaded table map
  if (Check_TM_Against_Single_Window(tablemap_index, hwnd)) { 
		// Filter out served tables already here,
		// otherwise the other list used in the dialog
		// to select windows manually will cause us lots of headaches,
		// as the lists will be of different size 
		// and the indexes will not match.
    if (TableManagement()->SharedMem()->PokerWindowAttached(hwnd)) {
      write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Window candidate already served: [%d]\n", hwnd);
    } else if (WinIsOpenHoldem(hwnd)) { // !!! refactore, does not belong to popup-handler
      write_log(Preferences()->debug_popup_blocker(), "[CAutoConnector] Window belongs to OpenHoldem\n");
		}	else {
			write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Adding window candidate to the list: [%d]\n", hwnd);
			tablelisthold.hwnd = hwnd;
      tablelisthold.tablemap_index = tablemap_index;
			g_tlist.Add(tablelisthold);
		}
	}
  return true;  // keep processing through entire list of windows
}

void CAutoConnector::FailedToConnectBecauseNoWindowInList() {
	TableManagement()->SharedMem()->RememberTimeOfLastFailedAttemptToConnect();
	GoIntoPopupBlockingMode();
}

void CAutoConnector::FailedToConnectProbablyBecauseAllTablesAlreadyServed() {
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Attempt to connect did fail\n");
	TableManagement()->SharedMem()->RememberTimeOfLastFailedAttemptToConnect();
	GoIntoPopupBlockingMode();
}

void CAutoConnector::GoIntoPopupBlockingMode() {
	// We have a free instance that has nothing to do.
	// Care about potential popups here, once per auto-connector-heartbeat.
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Not connected. Going into popup-blocking mode.\n");
	if (TableManagement()->SharedMem()->AnyWindowAttached())	{
		// Only handle popups if at least one bot is connected to a table.
		// Especially stop popup-handling if the last table got closed
		// to allow "normal" human work again.
		popup_handler.HandleAllWindows();
	} else {
    write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] No table connected at all. No need for popup-blocking.\n");
  }
}

bool CAutoConnector::Connect(HWND targetHWnd) {
	int					line = 0, ret = 0;
	char				title[MAX_WINDOW_TITLE] = {0};
	int					SelectedItem = kUndefined;
	CString			current_path = "";
	BOOL				bFound = false;
  // Potential race-condition, as some objects
  // (especially GUI objects) get created by another thread.
  // We just skip connection if OH is not yet initialized.
  // http://www.maxinmontreal.com/forums/viewtopic.php?f=156&t=19706
  // 
  // We have to check and return very early, we must not do this
  // after locking the mutex, otherwiese we block other instances forever.
  // http://www.maxinmontreal.com/forums/viewtopic.php?f=110&t=19407&p=140417#p140417
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Connect(..)\n");
  ASSERT(_autoconnector_mutex->m_hObject != NULL); 
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Locking autoconnector-mutex\n");
	if (!_autoconnector_mutex->Lock(500))	{
		write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Could not grab mutex; early exit\n");
		return false; 
	}
  // Clear global list for holding table candidates
	g_tlist.RemoveAll();
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Number of tablemaps loaded: %i\n",
    NumberOfTableMapsLoaded());
	for (int tablemap_index=0; tablemap_index<NumberOfTableMapsLoaded(); tablemap_index++) {
		write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Going to check TM nr. %d out of %d\n", 
			tablemap_index, NumberOfTableMapsLoaded());
		Check_TM_Against_All_Windows_Or_TargetHWND(tablemap_index, targetHWnd);
	}
	// Put global candidate table list in table select dialog variables
	int n_window_candidates = (int) g_tlist.GetSize();
  write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Number of table candidates: %i\n", 
    n_window_candidates);
	if (n_window_candidates == 0) {
		FailedToConnectBecauseNoWindowInList();
	}	else 	{
		SelectedItem = SelectTableMapAndWindowAutomatically();
		if (SelectedItem == kUndefined) {
			FailedToConnectProbablyBecauseAllTablesAlreadyServed();
		}	else {
			write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Window [%d] selected\n", g_tlist[SelectedItem].hwnd);
      // Load correct tablemap, and save hwnd/rect/numchairs of table that we are "attached" to
			set_attached_hwnd(g_tlist[SelectedItem].hwnd);
      CheckIfWindowMatchesMoreThanOneTablemap(attached_hwnd());
      CString tablemap_to_load = TableMapLoader()->GetTablemapPathToLoad(g_tlist[SelectedItem].tablemap_index);
			write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Selected tablemap: %s\n", tablemap_to_load);
			BasicScraper()->Tablemap()->LoadTablemap(tablemap_to_load);
			write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Tablemap successfully loaded\n");
			write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Scraper-bitmaps created\n");
      // Clear scraper fields
			TableState()->Reset();
      ///CasinoInterface()->Reset();
			write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Table state cleared\n");
      // Reset symbols
			///EngineContainer()->UpdateOnConnection();
      write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] UpdateOnConnection executed (during connection)\n");
			write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Going to continue with scraper output and scraper DLL\n");
      // log OH title bar text and table reset
      TableManagement()->TablePositioner()->ResizeToTargetSize();
      TableManagement()->TablePositioner()->PositionMyWindow();
		}
	}
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Unlocking autoconnector-mutex\n");
	_autoconnector_mutex->Unlock();
	return (SelectedItem != kUndefined);
}

void CAutoConnector::Disconnect(CString reason_for_disconnection) {
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Disconnect()\n");
  if (!IsConnectedToAnything()) {
    // Be extra safe.
    // This stupid error happened, when OnTimer() only checked if the window 
    // still existed, but not if we were connected at all.
    // Then Diconnect() plus Connect() lead to freezing.
    write_log(k_always_log_errors, "[CAutoConnector] ERROR: Disconnect() called while not connected\n");
    return;
  }
  // First close scraper-output-dialog,
  // as an updating dialog without a connected table can crash.
  ///CDlgScraperOutput::DestroyWindowSafely();
  // Make sure autoplayer is off
  write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Stopping autoplayer\n");
	// Wait for mutex - "forever" if necessary, as we have to clean up.
	ASSERT(_autoconnector_mutex->m_hObject != NULL); 
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Locking autoconnector-mutex\n");
  _autoconnector_mutex->Lock(INFINITE); 
	///EngineContainer()->UpdateOnDisconnection();
	// Clear "attached" info
	set_attached_hwnd(NULL);
	// Release mutex as soon as possible, after critical work is done
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Unlocking autoconnector-mutex\n");
	_autoconnector_mutex->Unlock();	
	// Delete bitmaps
	///BasicScraper()->DeleteBitmaps();
  // Clear scraper fields
	TableState()->Reset();
  ///CasinoInterface()->Reset();
	// Reset symbols
	///EngineContainer()->UpdateOnConnection();
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] UpdateOnConnection executed (disconnection)\n");
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Going to continue with window title\n");
  CString message;
  message.Format("DISCONNECTION -- %s", reason_for_disconnection);
	///WriteLogTableReset(message);
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Disconnect done\n");
}

int CAutoConnector::SelectTableMapAndWindowAutomatically() {
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] SelectTableMapAndWindowAutomatically(..)\n");
  int n_window_candidates = (int)g_tlist.GetSize();
	for (int i=0; i<n_window_candidates; ++i) {
		if (!TableManagement()->SharedMem()->PokerWindowAttached(g_tlist[i].hwnd))	{
			write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] Chosen (table, TM)-pair in list: %d\n", i);
			return i;
		}
	}
	// No appropriate table found
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] No appropriate table found.\n");
	return kUndefined;
}

double CAutoConnector::SecondsSinceLastFailedAttemptToConnect() {
	time_t last_failed_attempt_to_connect = TableManagement()->SharedMem()->GetTimeOfLastFailedAttemptToConnect(); 
	time_t CurrentTime;
	time(&CurrentTime);
	double _TimeSincelast_failed_attempt_to_connect = difftime(CurrentTime, last_failed_attempt_to_connect);
	write_log(Preferences()->debug_autoconnector(), "[CAutoConnector] TimeSincelast_failed_attempt_to_connect %f\n", _TimeSincelast_failed_attempt_to_connect);
	return _TimeSincelast_failed_attempt_to_connect;
}