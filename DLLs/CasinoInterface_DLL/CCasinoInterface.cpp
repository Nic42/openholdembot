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

#include "CCasinoInterface.h"
///#include "CMyMutex.h"
#include "PokerChat.hpp"
#include "SwagAdjustment.h"
#include "low_level\keyboard.h"
#include "low_level\mouse.h"
#include "..\Debug_DLL\debug.h"
#include "..\Numerical_Functions_DLL\Numerical_Functions.h"
#include "..\Preferences_DLL\Preferences.h"
#include "..\Scraper_DLL\CBasicScraper.h"
#include "..\Symbols_DLL\CBetroundCalculator.h"
#include "..\Symbols_DLL\CEngineContainer.h"
#include "..\Symbols_DLL\CFunctionCollection.h"
#include "..\Symbols_DLL\CSymbolEngineCasino.h"
#include "..\Symbols_DLL\CSymbolEngineChipAmounts.h"
#include "..\Symbols_DLL\CSymbolEngineHistory.h"
#include "..\Symbols_DLL\CSymbolEngineRandom.h"
#include "..\Symbols_DLL\CSymbolEngineTime.h"
#include "..\TableManagement_DLL\CTableManagement.h"
#include "..\Tablestate_DLL\TableState.h"
///#include "..\CTableMap\CTableMapAccess.h"
#include "..\WindowFunctions_DLL\window_functions.h"
#include "..\..\OpenHoldem\OpenHoldem.h"
#include "..\..\Shared\MagicNumbers\MagicNumbers.h"

CCasinoInterface::CCasinoInterface() {
	// dummy point for mouse and keyboard DLL
	p_null.x = kUndefined;
	p_null.y = kUndefined;
  Reset();
}

CCasinoInterface::~CCasinoInterface() {
}

void CCasinoInterface::Reset() {
  CString button_name;
  for (int i = 0; i < k_max_number_of_buttons; ++i) {
    button_name.Format("i%cbutton", HexadecimalChar(i)); 
    _technical_autoplayer_buttons[i].Reset();
    _technical_autoplayer_buttons[i].SetTechnicalName(button_name);
  }
  for (int i = 0; i < k_max_betpot_buttons; ++i) {
    button_name.Format("%sbutton", k_betpot_button_name[i]);
    _technical_betpot_buttons[i].Reset();
    _technical_betpot_buttons[i].SetTechnicalName(button_name);
  }
  for (int i = 0; i < k_max_number_of_i86X_buttons; ++i) {
    button_name.Format("i86%dbutton", i);
    _technical_i86X_spam_buttons[i].Reset();
    _technical_i86X_spam_buttons[i].SetTechnicalName(button_name);
  }
}

bool CCasinoInterface::TableLostFocus() {
  HWND foreground_window = GetForegroundWindow();
  HWND connected_window = TableManagement()->AutoConnector()->attached_hwnd();
	bool lost_focus = (foreground_window != connected_window);
  if (lost_focus) {
    CString foreground_title(" ", MAX_WINDOW_TITLE);
    WinGetTitle(foreground_window, foreground_title.GetBuffer());
    CString table_title(" ", MAX_WINDOW_TITLE);
    WinGetTitle(connected_window, table_title.GetBuffer());
    write_log(k_always_log_errors, "[CasinoInterface] WARNING! Lost focus detected\n");
    write_log(k_always_log_errors, "[CasinoInterface] WARNING! Foreground: \"%s\"\n", foreground_title);
    write_log(k_always_log_errors, "[CasinoInterface] WARNING! Connected:  \"%s\"\n", table_title);
  }
  return lost_focus;
}

void CCasinoInterface::ClickRect(RECT rect) {
	write_log(Preferences()->debug_autoplayer(), "[CasinoInterface] Calling mouse.dll to single click button: %d,%d %d,%d\n", 
    rect.left, rect.top, rect.right, rect.bottom);
	MouseClick(TableManagement()->AutoConnector()->attached_hwnd(), rect, MouseLeft, 1);
  EngineContainer()->symbol_engine_time()->UpdateOnAutoPlayerAction();
}

bool CCasinoInterface::ClickButtonSequence(int first_button, int second_button, int delay_in_milli_seconds) {
	if (LogicalAutoplayerButton(first_button)->Click()) {
		Sleep(delay_in_milli_seconds); 
		if (TableLostFocus())	{
			return false;
		}
    return LogicalAutoplayerButton(second_button)->Click();
	}
  return false;
}

bool CCasinoInterface::CloseWindow() {
	// Hard-coded click to the "X" at the top-right
	// of the title-bar
	// ToDo (maybe): make it work for different settings.

	RECT table_size, close_region;
	// http://msdn.microsoft.com/en-us/library/ms633503.aspx
	GetClientRect(TableManagement()->AutoConnector()->attached_hwnd(), &table_size);

	close_region.top    = -3;
	close_region.bottom = -15;
	close_region.left   = table_size.right - 18;
	close_region.right  = table_size.right -  6;
	
	write_log(Preferences()->debug_autoplayer(), "[CasinoInterface] f$close is true.\n");
	write_log(Preferences()->debug_autoplayer(), "[CasinoInterface] preparing to execute f$close.\n");
	ClickRect(close_region);

	return true;
}

void CCasinoInterface::PressTabToSwitchOHReplayToNextFrame() {
  RECT	rect_somewhere = {1, 1, 2, 2};
	POINT	cur_pos = {0};

  assert(EngineContainer()->symbol_engine_casino()->ConnectedToOHReplay());
  SendString(TableManagement()->AutoConnector()->attached_hwnd(), 
    rect_somewhere, "\t", false);
}

bool CCasinoInterface::EnterChatMessage(CString &message) {
	RECT			rect_chatbox;
	POINT			cur_pos = {0};

	/*#if (!BasicScraper()->TablemapAccess()->GetTableMapRect("chatbox", &rect_chatbox)) {
		write_log(Preferences()->debug_autoplayer(), "[CasinoInterface] Can't chat. No region defined.\n");
		return false;
	}*/
	write_log(Preferences()->debug_autoplayer(), "[CasinoInterface] Sending chat-message: %s\n", message);
	SendString(TableManagement()->AutoConnector()->attached_hwnd(), rect_chatbox, message, false);

	// Clear old chat_message to allow new ones.
	///_the_chat_message = NULL;
	ComputeFirstPossibleNextChatTime();
	return true;
}

int CCasinoInterface::NumberOfVisibleAutoplayerButtons() {
  int result = LogicalAutoplayerButton(k_autoplayer_function_fold)->IsClickable()
    + LogicalAutoplayerButton(k_autoplayer_function_call)->IsClickable()
    + LogicalAutoplayerButton(k_autoplayer_function_check)->IsClickable()
    + LogicalAutoplayerButton(k_autoplayer_function_raise)->IsClickable()
    + LogicalAutoplayerButton(k_autoplayer_function_allin)->IsClickable();
  write_log(Preferences()->debug_autoplayer(), "[CasinoInterface] %i autoplayer buttons visible.\n", result);
  return result;
}

bool CCasinoInterface::HandleInterfacebuttonsI86(void) {
  for (int i = 0; i<k_max_number_of_i86X_buttons; ++i) {
    if (CasinoInterface()->_technical_i86X_spam_buttons[i].IsClickable()) {
      ///CMyMutex mutex;
      ///if (!mutex.IsLocked()) return false;
      write_log(Preferences()->debug_autoplayer(), "[CasinoInterface] Clicking i86X (%d) button.\n", i);
      return CasinoInterface()->_technical_i86X_spam_buttons[i].Click();
    }
  }
 write_log(Preferences()->debug_autoplayer(), "[CasinoInterface] No interface button (i86X) to be handled.\n");
  return false;
}

bool CCasinoInterface::EnterBetsizeForAllin() {
  write_log(Preferences()->debug_autoplayer(), "[CasinoInterface] Going to enter betsize allin\n");
	double betsize_for_allin = TableState()->User()->_bet.GetValue()
	  + TableState()->User()->_balance.GetValue(); 
  return EnterBetsize(betsize_for_allin);
}

bool CCasinoInterface::EnterBetsize(double total_betsize_in_dollars) {
  if (_betsize_input_box.IsReadyToBeUsed()) {
    return _betsize_input_box.EnterBetsize(total_betsize_in_dollars);
  }
  return false;
}

bool CCasinoInterface::UseSliderForAllin() {
  return _allin_slider.SlideAllin();
}

bool CCasinoInterface::IsMyTurn() {
  return (NumberOfVisibleAutoplayerButtons() >= k_min_buttons_needed_for_my_turn);
}

CAutoplayerButton* CCasinoInterface::LogicalAutoplayerButton(int autoplayer_function_code) {
  switch (autoplayer_function_code) {
    // Betpot_buttons
  case k_autoplayer_function_betpot_2_1:
    return &_technical_betpot_buttons[0];
  case k_autoplayer_function_betpot_1_1:
    return &_technical_betpot_buttons[1];
  case k_autoplayer_function_betpot_3_4:
    return &_technical_betpot_buttons[2];
  case k_autoplayer_function_betpot_2_3:
    return &_technical_betpot_buttons[3];
  case k_autoplayer_function_betpot_1_2:
    return &_technical_betpot_buttons[4];
  case k_autoplayer_function_betpot_1_3:
    return &_technical_betpot_buttons[5];
  case k_autoplayer_function_betpot_1_4:
    return &_technical_betpot_buttons[6];
  default:
    // The i86-autoplayer-buttons are flexible
    // and have to be searched by label.
    //
    // Search clickable buttons first.
    // The old code could return a non-clickable button
    // if multiple buttons of the same type were present.
    // http://www.maxinmontreal.com/forums/viewtopic.php?f=124&t=18915
    for (int i = 0; i < k_max_number_of_buttons; ++i) {
      if (_technical_autoplayer_buttons[i].IsButtonType(autoplayer_function_code)
        && _technical_autoplayer_buttons[i].IsClickable()) {
        return &_technical_autoplayer_buttons[i];
      }
    }
    // Search again,
    // this time accepting any button, including non-clickable ones.
    for (int i = 0; i < k_max_number_of_buttons; ++i) {
      if (_technical_autoplayer_buttons[i].IsButtonType(autoplayer_function_code)) {
        return &_technical_autoplayer_buttons[i];
      }
    }
  }
  // k_autoplayer_function_beep and invalid input
  return &_non_clickable_fake_button;
}

CAutoplayerButton* CCasinoInterface::BetsizeConfirmationButton() {
  // Last hard-coded default: i3-button
  return &_technical_autoplayer_buttons[3];
}

bool CCasinoInterface::AllinOptionAvailable() {
  if (LogicalAutoplayerButton(k_autoplayer_function_allin)->IsClickable()) {
    return true;
  }
  if (BetsizeConfirmationButton()->IsClickable()) {
    return true;
  }
  if (_allin_slider.SlideAllinPossible()) {
    return true;
  }
  return false;
}

void CCasinoInterface::SendKey(const char ascii_key) {
  RECT r_null;
  // !! kUndefined causes SendKey to return early
  // !! keyboard-DLL is a mess.
  r_null.bottom = kUndefinedZero;
  r_null.left = kUndefinedZero;
  r_null.right = kUndefinedZero;
  r_null.top = kUndefinedZero;
  POINT	cur_pos = { 0 };
  // Using the SendString function to send a single character
  char input[2];
  input[0] = ascii_key;
  input[1] = '\0';
  SendString(TableManagement()->AutoConnector()->attached_hwnd(), r_null, input, false);
}

bool CCasinoInterface::ConnectedToManualMode() {
  const int k_max_length_of_classname = 50;
  char classname[k_max_length_of_classname] = "";
  ///GetClassName(TableManagement()->AutoConnector()->attached_hwnd(), classname, k_max_length_of_classname);
  return (strcmp(classname, "OpenHoldemManualMode") == 0);
}

bool CCasinoInterface::ConnectedToOHReplay() {
  const int k_max_length_of_classname = 50;
  char classname[k_max_length_of_classname] = "";
  ///GetClassName(TableManagement()->AutoConnector()->attached_hwnd(), classname, k_max_length_of_classname);
  return (strcmp(classname, "OHREPLAY") == 0);
}

bool CCasinoInterface::ConnectedToOfflineSimulation() {
  return (ConnectedToManualMode()
    || ConnectedToOHReplay()
    || ConnectedToDDPoker()
    || SitenameContainsCasinoIdentifier("pokeracademy")
    || SitenameContainsCasinoIdentifier("pokerth")
    || SitenameContainsCasinoIdentifier("pokersnowie"));
}

bool CCasinoInterface::ConnectedToRealCasino() {
  return (!ConnectedToOfflineSimulation());
}

bool CCasinoInterface::ConnectedToBring() {
  const int k_max_length_of_classname = 50;
  char classname[k_max_length_of_classname] = "";
  ///GetClassName(TableManagement()->AutoConnector()->attached_hwnd(), classname, k_max_length_of_classname);
  return (strcmp(classname, "BRING") == 0);
}

bool CCasinoInterface::ConnectedToDDPoker() {
  return SitenameContainsCasinoIdentifier("ddpoker");
}

bool CCasinoInterface::SitenameContainsCasinoIdentifier(const char *casino) {
  CString sitename = BasicScraper()->Tablemap()->sitename();
  sitename.MakeLower();
  return (sitename.Find(casino) >= 0);
}