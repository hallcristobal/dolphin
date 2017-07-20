// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.

#pragma once

#include <string>

#include "Common/CommonTypes.h"

//Dragonbane
#include <DolphinWX/MAIN.H>


struct GCPadStatus;
class PointerWrap;
struct wiimote_key;

namespace WiimoteEmu
{
	struct ReportFeatures;
}

// Per-(video )Movie actions

namespace Movie
{

	// Enumerations and structs
	enum PlayMode
	{
		MODE_NONE = 0,
		MODE_RECORDING,
		MODE_PLAYING
	};

	// GameCube Controller State
#pragma pack(push,1)
	struct ControllerState
	{
		bool Start : 1, A : 1, B : 1, X : 1, Y : 1, Z : 1; // Binary buttons, 6 bits
		bool DPadUp : 1, DPadDown : 1,             // Binary D-Pad buttons, 4 bits
			DPadLeft : 1, DPadRight : 1;
		bool L : 1, R : 1;                         // Binary triggers, 2 bits
		bool disc : 1;                           // Checks for disc being changed
		bool reset : 1;                          // Console reset button
		bool loading : 1;                        // Dragonbane: Loading status flag, 1 bit
		bool reserved : 1;                       // Reserved bits used for padding, 1 bit

		u8   TriggerL, TriggerR;               // Triggers, 16 bits
		u8   AnalogStickX, AnalogStickY;       // Main Stick, 16 bits
		u8   CStickX, CStickY;                 // Sub-Stick, 16 bits

		u8 tunerEvent;						   // Dragonbane: Tuner Event, 8 bits

		float LinkX, LinkZ;					   // Dragonbane: Used to detect desyncs, 64 bits
	};
	static_assert(sizeof(ControllerState) == 17, "ControllerState should be 17 bytes"); //Dragonbane
#pragma pack(pop)

#pragma pack(push,1)
	struct ControllerState_Version2
	{
		bool Start : 1, A : 1, B : 1, X : 1, Y : 1, Z : 1; // Binary buttons, 6 bits
		bool DPadUp : 1, DPadDown : 1,             // Binary D-Pad buttons, 4 bits
			DPadLeft : 1, DPadRight : 1;
		bool L : 1, R : 1;                         // Binary triggers, 2 bits
		bool disc : 1;                           // Checks for disc being changed
		bool reset : 1;                          // Console reset button
		bool loading : 1;                        // Dragonbane: Loading status flag, 1 bit
		bool reserved : 1;                       // Reserved bits used for padding, 1 bit

		u8   TriggerL, TriggerR;               // Triggers, 16 bits
		u8   AnalogStickX, AnalogStickY;       // Main Stick, 16 bits
		u8   CStickX, CStickY;                 // Sub-Stick, 16 bits

		float LinkX, LinkZ;					   // Dragonbane: Used to detect desyncs, 64 bits
	};
	static_assert(sizeof(ControllerState_Version2) == 16, "ControllerState should be 16 bytes"); //Dragonbane
#pragma pack(pop)

#pragma pack(push,1)
	struct ControllerState_Version1
	{
		bool Start : 1, A : 1, B : 1, X : 1, Y : 1, Z : 1; // Binary buttons, 6 bits
		bool DPadUp : 1, DPadDown : 1,             // Binary D-Pad buttons, 4 bits
			DPadLeft : 1, DPadRight : 1;
		bool L : 1, R : 1;                         // Binary triggers, 2 bits
		bool disc : 1;                           // Checks for disc being changed
		bool reset : 1;                          // Console reset button
		bool reserved : 2;                       // Reserved bits used for padding, 2 bit

		u8   TriggerL, TriggerR;               // Triggers, 16 bits
		u8   AnalogStickX, AnalogStickY;       // Main Stick, 16 bits
		u8   CStickX, CStickY;                 // Sub-Stick, 16 bits
	};
	static_assert(sizeof(ControllerState_Version1) == 8, "ControllerState should be 8 bytes"); //Dragonbane
#pragma pack(pop)


																							   // Global declarations
	extern bool g_bDiscChange, g_bClearSave, g_bReset;
	extern u64 g_titleID;

	extern u64 g_currentFrame, g_totalFrames;
	extern u64 g_currentLagCount;
	extern u64 g_currentInputCount, g_totalInputCount;
	extern std::string g_discChange;

	extern bool justStoppedRecording; //Dragonbane

	extern bool updateMainFrame;

	//Dragonbane: Auto Roll Stuff
	extern int roll_timer;
	extern bool roll_enabled;
	extern bool first_roll;
	extern bool checkSave, uncheckSave;

	//Dragonbane: Superswim Script
	extern bool swimStarted;
	extern bool swimInProgress;
	extern float swimDestPosX;
	extern float swimDestPosZ;

	//Dragonbane: Video Comparison Stuff
	extern bool cmp_requested;
	extern bool cmp_isRunning;
	extern bool cmp_leftFinished;
	extern bool cmp_rightFinished;
	extern bool cmp_movieFinished;
	extern bool cmp_loadState;
	extern bool cmp_justFinished;
	extern std::string cmp_currentMovie;
	extern std::string cmp_leftMovie;
	extern std::string cmp_rightMovie;
	extern std::string cmp_leftTitle;
	extern std::string cmp_rightTitle;
	extern std::string cmp_outputPath;
	extern std::string cmp_currentBranch;
	extern int cmp_width;
	extern int cmp_height;
	extern u64 cmp_startTimerFrame;
	extern u64 cmp_curentBranchFrame;

	//Dragonbane: Tuner Events
	extern u8 tunerActionID;
	extern u8 tunerExecuteID;
	extern u8 tunerStatus;


#pragma pack(push,1)
	struct DTMHeader
	{
		u8 filetype[4];         // Unique Identifier (always "DTM"0x1A)

		u8 gameID[6];           // The Game ID
		bool bWii;              // Wii game

		u8  numControllers;     // The number of connected controllers (1-4)

		bool bFromSaveState;    // false indicates that the recording started from bootup, true for savestate
		u64 frameCount;         // Number of frames in the recording
		u64 inputCount;         // Number of input frames in recording
		u64 lagCount;           // Number of lag frames in the recording
		u64 uniqueID;           // (not implemented) A Unique ID comprised of: md5(time + Game ID)
		u32 numRerecords;       // Number of rerecords/'cuts' of this TAS
		u8  author[32];         // Author's name (encoded in UTF-8)

		u8  videoBackend[16];   // UTF-8 representation of the video backend
		u8  audioEmulator[16];  // UTF-8 representation of the audio emulator
		u8  md5[16];            // MD5 of game iso

		u64 recordingStartTime; // seconds since 1970 that recording started (used for RTC)

		bool bSaveConfig;       // Loads the settings below on startup if true
		bool bSkipIdle;
		bool bDualCore;
		bool bProgressive;
		bool bDSPHLE;
		bool bFastDiscSpeed;
		u8   CPUCore;           // 0 = interpreter, 1 = JIT, 2 = JITIL
		bool bEFBAccessEnable;
		bool bEFBCopyEnable;
		bool bCopyEFBToTexture;
		bool bSkipEFBCopyToRam; // 5.0
		bool bEFBCopyCacheEnable;
		bool bEFBEmulateFormatChanges;
		bool bUseXFB;
		bool bUseRealXFB;
		u8   memcards;
		bool bClearSave;        // Create a new memory card when playing back a movie if true
		u8   bongos;
		u8   numGBAs;			//Dragonbane
		bool bSyncGPU;
		bool bNetPlay;	
		bool bPAL60;
		u8   language;
		u8   reserved[9];      // Padding for any new config options
		u8   discChange[40];    // Name of iso file to switch to, for two disc games.
		u8   revision[20];      // Git hash
		u32  DSPiromHash;
		u32  DSPcoefHash;
		u64  tickCount;	        // Number of ticks in the recording
		u8   reserved2[11];     // Make heading 256 bytes, just because we can
	};
	static_assert(sizeof(DTMHeader) == 256, "DTMHeader should be 256 bytes");
#pragma pack(pop)

#pragma pack(push,1)
	struct DTMHeader_Version12
	{
		u8 filetype[4];         // Unique Identifier (always "DTM"0x1A)

		u8 gameID[6];           // The Game ID
		bool bWii;              // Wii game

		u8  numControllers;     // The number of connected controllers (1-4)

		bool bFromSaveState;    // false indicates that the recording started from bootup, true for savestate
		u64 frameCount;         // Number of frames in the recording
		u64 inputCount;         // Number of input frames in recording
		u64 lagCount;           // Number of lag frames in the recording
		u64 uniqueID;           // (not implemented) A Unique ID comprised of: md5(time + Game ID)
		u32 numRerecords;       // Number of rerecords/'cuts' of this TAS
		u8  author[32];         // Author's name (encoded in UTF-8)

		u8  videoBackend[16];   // UTF-8 representation of the video backend
		u8  audioEmulator[16];  // UTF-8 representation of the audio emulator
		u8  md5[16];            // MD5 of game iso

		u64 recordingStartTime; // seconds since 1970 that recording started (used for RTC)

		bool bSaveConfig;       // Loads the settings below on startup if true
		bool bSkipIdle;
		bool bDualCore;
		bool bProgressive;
		bool bPAL60; // 5.0
		bool bDSPHLE;
		bool bFastDiscSpeed;
		u8   CPUCore;           // 0 = interpreter, 1 = JIT, 2 = JITIL
		bool bEFBAccessEnable;
		bool bEFBCopyEnable;
		bool bSkipEFBCopyToRam; // 5.0
		bool bCopyEFBToTexture;
		bool bEFBCopyCacheEnable;
		bool bEFBEmulateFormatChanges;
		bool bUseXFB;
		bool bUseRealXFB;
		u8   memcards;
		bool bClearSave;        // Create a new memory card when playing back a movie if true
		u8   bongos;
		bool bSyncGPU;
		bool bNetPlay;
		u8	 language; // 5.0
		u8   reserved[10];      // Padding for any new config options
		u8   discChange[40];    // Name of iso file to switch to, for two disc games.
		u8   revision[20];      // Git hash
		u32  DSPiromHash;
		u32  DSPcoefHash;
		u64  tickCount;	        // Number of ticks in the recording
		u8   reserved2[11];     // Make heading 256 bytes, just because we can
	};
	static_assert(sizeof(DTMHeader_Version12) == 256, "DTMHeader should be 256 bytes");
#pragma pack(pop)


	void FrameUpdate();
	void InputUpdate();
	void Init();

	void SetPolledDevice();

	bool IsRecordingInput();
	bool IsRecordingInputFromSaveState();
	bool IsJustStartingRecordingInputFromSaveState();
	bool IsJustStartingPlayingInputFromSaveState();
	bool IsPlayingInput();
	bool IsMovieActive();
	bool IsReadOnly();
	u64  GetRecordingStartTime();

	//Dragonbane
	bool VerifyRecording(const std::string& moviename, const std::string& statename, bool fromStart);
	void CancelVerifying();
	void CancelRecording();
	bool IsAutoSave();
	bool SaveMemCard();
	bool IsMovieFromSaveState(const std::string& moviename);
	bool AutoVerify();
	void RequestVideoComparison(const std::string& leftMovie, const std::string& rightMovie, const std::string& leftMovieTitle, const std::string& rightMovieTitle, int width, int height, const std::string& savePath);
	bool StartVideoComparison();
	u64 GetDTMComparisonLength(const std::string& movie);
	void RenderComparisonVideo(bool schedule);
	bool GetNextComparisonMovie(bool update);
	void CancelComparison();
	void AttachDTM(const std::string& dtmFile, u64 fromVisualFrame, u64 toVisualFrame, u64 fromInputFrame, u64 toInputFrame, int bytesPerFrame, const std::string& outputFile);

	bool IsConfigSaved();
	bool IsDualCore();
	bool IsProgressive();
	bool IsPAL60();
	bool IsSkipIdle();
	bool IsDSPHLE();
	bool IsFastDiscSpeed();
	int  GetCPUMode();
	u8   GetLanguage();
	bool IsStartingFromClearSave();
	bool IsUsingMemcard(int memcard);
	bool IsSyncGPU();
	void SetGraphicsConfig();
	void GetSettings();
	bool IsNetPlayRecording();

	bool IsUsingPad(int controller);
	bool IsUsingWiimote(int wiimote);
	bool IsUsingBongo(int controller);
	bool IsUsingGBA(int controller); //Dragonbane
	void ChangePads(bool instantly = false);
	void ChangeWiiPads(bool instantly = false);

	void DoFrameStep();
	void SetFrameStopping(bool bEnabled);
	void SetReadOnly(bool bEnabled);

	void SetFrameSkipping(unsigned int framesToSkip);
	void FrameSkipping();

	bool BeginRecordingInput(int controllers);
	void RecordInput(GCPadStatus* PadStatus, int controllerID);
	void RecordWiimote(int wiimote, u8 *data, u8 size);

	bool PlayInput(const std::string& filename);
	void LoadInput(const std::string& filename);
	void ReadHeader();
	void PlayController(GCPadStatus* PadStatus, int controllerID);
	bool PlayWiimote(int wiimote, u8* data, const struct WiimoteEmu::ReportFeatures& rptf, int ext, const wiimote_key key);
	void EndPlayInput(bool cont);
	void SaveRecording(const std::string& filename);
	void DoState(PointerWrap &p);
	void CheckMD5();
	void GetMD5();
	void Shutdown();
	void CheckPadStatus(GCPadStatus* PadStatus, int controllerID);
	void CheckWiimoteStatus(int wiimote, u8* data, const struct WiimoteEmu::ReportFeatures& rptf, int ext, const wiimote_key key);

	std::string GetInputDisplay();

	// Done this way to avoid mixing of core and gui code
	typedef void(*GCManipFunction)(GCPadStatus*, int);
	typedef void(*WiiManipFunction)(u8*, WiimoteEmu::ReportFeatures, int, int, wiimote_key);

	void SetGCInputManip(GCManipFunction);
	void SetWiiInputManip(WiiManipFunction);
	void CallGCInputManip(GCPadStatus* PadStatus, int controllerID);
	void CallWiiInputManip(u8* core, WiimoteEmu::ReportFeatures rptf, int controllerID, int ext, const wiimote_key key);
}
