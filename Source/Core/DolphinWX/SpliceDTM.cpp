// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.

#include <wx/bitmap.h>
#include <wx/defs.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/gdicmn.h>
#include <wx/image.h>
#include <wx/mstream.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/translation.h>
#include <wx/windowid.h>
#include <wx/msgdlg.h>
#include <array>
#include <wx/filepicker.h>
#include <wx/utils.h>

#include "Common/Common.h"
#include "DolphinWX/SpliceDTM.h"
#include "Core/Core.h"
#include "Core/HW/Memmap.h"
#include "Common/IniFile.h"

#include "Common/StringUtil.h"
#include "Common/FileUtil.h"
#include "DiscIO/Filesystem.h"

#include "DiscIO/FileSystemGCWii.h"
#include "DiscIO/Volume.h"
#include "DiscIO/VolumeCreator.h"
#include "Core/ConfigManager.h"
#include "Core/Movie.h"

#include "DolphinWX/ISOFile.h"
#include "DolphinWX/ISOProperties.h"
#include "DolphinWX/WxUtils.h"

//Dragonbane
BEGIN_EVENT_TABLE(DTMSplicer, wxDialog)

EVT_BUTTON(1, DTMSplicer::OnButtonPressed)

END_EVENT_TABLE()


DTMSplicer::DTMSplicer(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	//Text
	m_staticText = new wxStaticText(this, wxID_ANY, wxT("DTM to Attach:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText->Wrap(-1);
	m_staticText->SetFont(wxFont(12, 74, 90, 92, false, wxT("Arial")));
	bSizer1->Add(m_staticText, 0, wxALL, 5);

	//Dragonbane: Get last chosen Path
	IniFile settingsIni;
	IniFile::Section* iniPathSection;
	std::string lastAddedDTMPath = "";
	std::string lastSavedDTMPath = "";
	bool fileLoaded = false;

	fileLoaded = settingsIni.Load(File::GetUserPath(F_DOLPHINCONFIG_IDX));

	//Get Path
	if (fileLoaded)
	{
		iniPathSection = settingsIni.GetOrCreateSection("RememberedPaths");
		iniPathSection->Get("LastAttachedDTMPath", &lastAddedDTMPath, "");
		iniPathSection->Get("LastAttachedDTMSavedPath", &lastSavedDTMPath, "");
	}

	//File Picker
	m_DTMPath = new wxFilePickerCtrl(this, wxID_ANY,
		lastAddedDTMPath, _("Select A Recording File to Attach"),
		("Dolphin TAS Movies (*.dtm)") + wxString::Format("|*.dtm|%s", wxGetTranslation(wxALL_FILES)), 
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_OPEN);

	bSizer1->Add(m_DTMPath, 0, wxALL, 5);

	//Text 2
	m_staticText2 = new wxStaticText(this, wxID_ANY, wxT("From Visual Frame:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText2->Wrap(-1);
	m_staticText2->SetFont(wxFont(12, 74, 90, 92, false, wxT("Arial")));
	bSizer1->Add(m_staticText2, 0, wxALL, 5);

	//Textbox From Visual
	fromVisualFrame = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(fromVisualFrame, 0, wxALL, 5);

	//Text 3
	m_staticText3 = new wxStaticText(this, wxID_ANY, wxT("Until Visual Frame:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText3->Wrap(-1);
	m_staticText3->SetFont(wxFont(12, 74, 90, 92, false, wxT("Arial")));
	bSizer1->Add(m_staticText3, 0, wxALL, 5);

	//Textbox To Visual
	toVisualFrame = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(toVisualFrame, 0, wxALL, 5);

	//Text 4
	m_staticText4 = new wxStaticText(this, wxID_ANY, wxT("From Input Frame:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText4->Wrap(-1);
	m_staticText4->SetFont(wxFont(12, 74, 90, 92, false, wxT("Arial")));
	bSizer1->Add(m_staticText4, 0, wxALL, 5);

	//Textbox From Input
	fromInputFrame = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(fromInputFrame, 0, wxALL, 5);

	//Text 5
	m_staticText5 = new wxStaticText(this, wxID_ANY, wxT("Until Input Frame:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText5->Wrap(-1);
	m_staticText5->SetFont(wxFont(12, 74, 90, 92, false, wxT("Arial")));
	bSizer1->Add(m_staticText5, 0, wxALL, 5);

	//Textbox To Input
	toInputFrame = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(toInputFrame, 0, wxALL, 5);

	//Text 6
	m_staticText6 = new wxStaticText(this, wxID_ANY, wxT("Bytes per Input:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText6->Wrap(-1);
	m_staticText6->SetFont(wxFont(12, 74, 90, 92, false, wxT("Arial")));
	bSizer1->Add(m_staticText6, 0, wxALL, 5);

	//Textbox Bytes per Input Frame
	bytesPerFrame = new wxTextCtrl(this, wxID_ANY, "17", wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(bytesPerFrame, 0, wxALL, 5);

	//Text 7
	m_staticText7 = new wxStaticText(this, wxID_ANY, wxT("Output Path:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText7->Wrap(-1);
	m_staticText7->SetFont(wxFont(12, 74, 90, 92, false, wxT("Arial")));
	bSizer1->Add(m_staticText7, 0, wxALL, 5);

	//File Picker 2
	m_savePath = new wxFilePickerCtrl(this, wxID_ANY,
		lastSavedDTMPath, _("Save New DTM As"),
		("Dolphin TAS Movies (*.dtm)") + wxString::Format("|*.dtm|%s", wxGetTranslation(wxALL_FILES)),
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL | wxFLP_SAVE);


	bSizer1->Add(m_savePath, 0, wxALL, 5);

	bSizer1->AddSpacer(10);

	m_button_start = new wxButton(this, 1, "Attach DTM", wxDefaultPosition, wxDefaultSize);
	m_button_start->Enable();

	bSizer1->Add(m_button_start, 1, wxALIGN_CENTER, 5);

	bSizer1->AddSpacer(10);

	SetSizer(bSizer1);
	Layout();

	bSizer1->Fit(this);

	Centre(wxBOTH);

	Bind(wxEVT_CLOSE_WINDOW, &DTMSplicer::OnCloseWindow, this);
}
void DTMSplicer::OnButtonPressed(wxCommandEvent& event)
{	
	if (event.GetId() == 1) //Attach Button Pressed
	{
		if (!Core::IsRunningAndStarted())
		{
			wxMessageBox("Game not running!");
			return;
		}

		if (!Movie::IsPlayingInput())
		{
			wxMessageBox("Make sure a DTM you want to attach to is currently playing!");
			return;
		}

		if (Core::GetState() != Core::EState::CORE_PAUSE)
		{
			wxMessageBox("Make sure Emulation is paused before you attempt to attach a DTM!");
			return;
		}

		std::string path1 = WxStrToStr(m_DTMPath->GetPath());
		std::string path2 = WxStrToStr(m_savePath->GetPath());

		if (path1.empty() || path2.empty())
		{
			wxMessageBox("Empty paths!");
			return;
		}

		if (!File::Exists(path1))
		{
			wxMessageBox("DTM File doesn't exist!");
			return;
		}

		if (path1 == path2)
		{
			wxMessageBox("Paths are identical!");
			return;
		}

		//Dragonbane: Save last chosen Paths
		IniFile settingsIni;
		IniFile::Section* iniPathSection;
		bool fileLoaded = false;

		fileLoaded = settingsIni.Load(File::GetUserPath(F_DOLPHINCONFIG_IDX));

		//Save Path
		if (fileLoaded)
		{
			iniPathSection = settingsIni.GetOrCreateSection("RememberedPaths");

			std::string file, legalPathname, extension;

			SplitPathEscapeChar(WxStrToStr(path1), &legalPathname, &file, &extension);
			iniPathSection->Set("LastAttachedDTMPath", legalPathname);

			SplitPathEscapeChar(WxStrToStr(path2), &legalPathname, &file, &extension);
			iniPathSection->Set("LastAttachedDTMSavedPath", legalPathname);

			settingsIni.Save(File::GetUserPath(F_DOLPHINCONFIG_IDX));
		}


		std::string fromVisualFrameStr = WxStrToStr(fromVisualFrame->GetValue());
		std::string toVisualFrameStr = WxStrToStr(toVisualFrame->GetValue());

		std::string fromInputFrameStr = WxStrToStr(fromInputFrame->GetValue());
		std::string toInputFrameStr = WxStrToStr(toInputFrame->GetValue());

		std::string bytesPerFrameStr = WxStrToStr(bytesPerFrame->GetValue());

		u64 fromVisualFrameInt = 0;
		u64 toVisualFrameInt = 0;

		u64 fromInputFrameInt = 0;
		u64 toInputFrameInt = 0;

		int bytesPerFrameInt = 17;

		if (!fromVisualFrameStr.empty())
		{
			fromVisualFrameInt = _strtoui64(fromVisualFrameStr.c_str(), NULL, 10);
		}

		if (!toVisualFrameStr.empty())
		{
			toVisualFrameInt = _strtoui64(toVisualFrameStr.c_str(), NULL, 10);
		}


		if (!fromInputFrameStr.empty())
		{
			fromInputFrameInt = _strtoui64(fromInputFrameStr.c_str(), NULL, 10);
		}

		if (!toInputFrameStr.empty())
		{
			toInputFrameInt = _strtoui64(toInputFrameStr.c_str(), NULL, 10);
		}


		if (!bytesPerFrameStr.empty())
		{
			bytesPerFrameInt = atoi(bytesPerFrameStr.c_str());
		}

		if (bytesPerFrameInt > 17 || bytesPerFrameInt < 8)
		{
			wxMessageBox("Only supports DTMs between 8 and 17 bytes per input frame!");
			return;
		}

		Movie::AttachDTM(path1, fromVisualFrameInt, toVisualFrameInt, fromInputFrameInt, toInputFrameInt, bytesPerFrameInt, path2);

		wxMessageBox("Process completed!");
	}
}

void DTMSplicer::OnCloseWindow(wxCloseEvent& event)
{
	if (event.CanVeto())
	{
		event.Skip(false);
		Show(false);
	}
}