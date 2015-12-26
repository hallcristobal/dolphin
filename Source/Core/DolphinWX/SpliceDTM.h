// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.

#pragma once

//Dragonbane

#include <wx/defs.h>
#include <wx/dialog.h>
#include <wx/gdicmn.h>
#include <wx/string.h>
#include <wx/translation.h>
#include <wx/windowid.h>
#include <wx/artprov.h>
#include <wx/stattext.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>

#include "Common/CommonTypes.h"
#include <string>



class wxWindow;
class wxButton;
class wxFilePickerCtrl;

class DTMSplicer : public wxDialog
{
private:

	DECLARE_EVENT_TABLE();

protected:

	wxStaticText* m_staticText;
	wxFilePickerCtrl *m_DTMPath;
	wxStaticText* m_staticText2;
	wxTextCtrl* fromFrame;
	wxStaticText* m_staticText3;
	wxTextCtrl* toFrame;
	wxStaticText* m_staticText4;
	wxTextCtrl* bytesPerFrame;
	wxStaticText* m_staticText5;
	wxFilePickerCtrl *m_savePath;
	wxButton* m_button_start;

public:

	DTMSplicer(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Attach DTM by DB"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style  = wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP);
	void OnButtonPressed(wxCommandEvent& event);
	void OnCloseWindow(wxCloseEvent& event);
};
