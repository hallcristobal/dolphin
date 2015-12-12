// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.

#include <polarssl/md5.h>

#include "Common/ChunkFile.h"
#include "Common/CommonPaths.h"
#include "Common/FileUtil.h"
#include "Common/Hash.h"
#include "Common/NandPaths.h"
#include "Common/StringUtil.h"
#include "Common/Thread.h"
#include "Common/Timer.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/CoreTiming.h"
#include "Core/Movie.h"
#include "Core/LUA/Lua.h"
#include "Core/NetPlayProto.h"
#include "Core/State.h"
#include "Core/DSP/DSPCore.h"
#include "Core/HW/DVDInterface.h"
#include "Core/HW/EXI_Device.h"
#include "Core/HW/ProcessorInterface.h"
#include "Core/HW/SI.h"
#include "Core/HW/Wiimote.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"
#include "Core/HW/WiimoteEmu/WiimoteHid.h"
#include "Core/HW/WiimoteEmu/Attachment/Classic.h"
#include "Core/HW/WiimoteEmu/Attachment/Nunchuk.h"
#include "Core/IPC_HLE/WII_IPC_HLE_Device_usb.h"
#include "Core/PowerPC/PowerPC.h"
#include "InputCommon/GCPadStatus.h"
#include "VideoCommon/VideoConfig.h"
#include "Core/Host.h"

#include <lua.hpp> //Dragonbane


//Lua Functions (C)
int ReadValue8(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	u32 address = lua_tointeger(L, 1);

	u8 result = Memory::Read_U8(address);

	lua_pushinteger(L, result); // return value
	return 1; // number of return values
}

int ReadValue16(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	u32 address = lua_tointeger(L, 1);

	u16 result = Memory::Read_U16(address);

	lua_pushinteger(L, result); // return value
	return 1; // number of return values
}

int ReadValue32(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	u32 address = lua_tointeger(L, 1);

	u32 result = Memory::Read_U32(address);

	lua_pushinteger(L, result); // return value
	return 1; // number of return values
}

int ReadValueFloat(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	u32 address = lua_tointeger(L, 1);

	float result = Memory::Read_F32(address);

	lua_pushnumber(L, result); // return value
	return 1; // number of return values
}
int ReadValueString(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 2)
		return 0;

	u32 address = lua_tointeger(L, 1);
	int count = lua_tointeger(L, 2);

	std::string result = Memory::Read_String(address, count);

	lua_pushstring(L, result.c_str()); // return value
	return 1; // number of return values
}

//Write Stuff
int WriteValue8(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 2)
		return 0;

	u32 address = lua_tointeger(L, 1);
	u8 value = lua_tointeger(L, 2);

	Memory::Write_U8(value, address);

	return 0; // number of return values
}

int WriteValue16(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 2)
		return 0;

	u32 address = lua_tointeger(L, 1);
	u16 value = lua_tointeger(L, 2);

	Memory::Write_U16(value, address);

	return 0; // number of return values
}

int WriteValue32(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 2)
		return 0;

	u32 address = lua_tointeger(L, 1);
	u32 value = lua_tointeger(L, 2);

	Memory::Write_U32(value, address);

	return 0; // number of return values
}

int WriteValueFloat(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 2)
		return 0;

	u32 address = lua_tointeger(L, 1);
	double value = lua_tonumber(L, 2);

	Memory::Write_F32((float)value, address);

	return 0; // number of return values
}
int WriteValueString(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 2)
		return 0;

	u32 address = lua_tointeger(L, 1);
	const char* value = lua_tostring(L, 2);

	std::string string = StringFromFormat("%s", value);

	Memory::Write_String(string, address);

	return 0; // number of return values
}

int GetPointerNormal(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	u32 address = lua_tointeger(L, 1);

	u32 pointer = Memory::Read_U32(address);

	if (pointer > 0x80000000)
	{
		pointer -= 0x80000000;
	}
	else
	{
		return 0;
	}

	lua_pushinteger(L, pointer); // return value
	return 1; // number of return values
}

int PressButton(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	const char* button = lua_tostring(L, 1);

	Lua::iPressButton(button);

	return 0; // number of return values
}

int ReleaseButton(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	const char* button = lua_tostring(L, 1);

	Lua::iReleaseButton(button);

	return 0; // number of return values
}

int SetMainStickX(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	int xPos = lua_tointeger(L, 1);

	Lua::iSetMainStickX(xPos);

	return 0;
}
int SetMainStickY(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	int yPos = lua_tointeger(L, 1);

	Lua::iSetMainStickY(yPos);

	return 0;
}

int SetCStickX(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	int xPos = lua_tointeger(L, 1);

	Lua::iSetCStickX(xPos);

	return 0;
}
int SetCStickY(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	int yPos = lua_tointeger(L, 1);

	Lua::iSetCStickY(yPos);

	return 0;
}

int SaveState(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 2)
		return 0;

	bool useSlot = false;

	BOOL Slot = lua_toboolean(L, 1);
	int slotID = 0;
	std::string string = "";

	if (Slot)
	{
		useSlot = true;
		slotID = lua_tointeger(L, 2);
	}
	else
	{
		const char* fileName = lua_tostring(L, 2);
		string = StringFromFormat("%s", fileName);
	}

	Lua::iSaveState(useSlot, slotID, string);

	return 0; // number of return values
}

int LoadState(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 2)
		return 0;

	bool useSlot = false;

	BOOL Slot = lua_toboolean(L, 1);
	int slotID = 0;
	std::string string = "";

	if (Slot)
	{
		useSlot = true;
		slotID = lua_tointeger(L, 2);
	}
	else
	{
		const char* fileName = lua_tostring(L, 2);
		string = StringFromFormat("%s", fileName);
	}

	Lua::iLoadState(useSlot, slotID, string);

	return 0; // number of return values
}


int GetFrameCount(lua_State *L)
{
	int argc = lua_gettop(L);

	lua_pushinteger(L, Movie::g_currentFrame); // return value
	return 1; // number of return values
}

int MsgBox(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
		return 0;

	const char* text = lua_tostring(L, 1);

	int delay = 5000; //Default: 5 seconds

	if (argc == 2)
	{
		delay = lua_tointeger(L, 2);
	}

	std::string message = StringFromFormat("Lua Msg: %s", text);

	Core::DisplayMessage(message, delay);

	return 0; // number of return values
}

int AbortSwim(lua_State *L)
{
	int argc = lua_gettop(L);

	Movie::swimStarted = false;

	return 0; // number of return values
}

void HandleLuaErrors(lua_State *L, int status)
{
	if (status != 0)
	{
		std::string message = StringFromFormat("Lua Error: %s", lua_tostring(L, -1));

		PanicAlertT(message.c_str());

		lua_pop(L, 1); // remove error message
	}
}


namespace Lua
{
	//Dragonbane: Lua Stuff
	static lua_State *luaState;
	static GCPadStatus PadLocal;

	const int m_gc_pad_buttons_bitmask[12] = {
		PAD_BUTTON_DOWN, PAD_BUTTON_UP, PAD_BUTTON_LEFT, PAD_BUTTON_RIGHT, PAD_BUTTON_A, PAD_BUTTON_B,
		PAD_BUTTON_X, PAD_BUTTON_Y, PAD_TRIGGER_Z, PAD_TRIGGER_L, PAD_TRIGGER_R, PAD_BUTTON_START
	};

	StateEvent m_stateData;

	//LUA Stuff
	bool lua_isStateOperation = false;
	bool lua_isStateSaved = false;
	bool lua_isStateLoaded = false;
	bool lua_isStateDone = false;


	void Init()
	{
		
	}
	void Shutdown()
	{


	}

	//Dragonbane: Lua Wrapper Functions
	void iPressButton(const char* button)
	{
		if (!strcmp(button, "A"))
		{
			PadLocal.button |= m_gc_pad_buttons_bitmask[4];
			PadLocal.analogA = 0xFF;
		}
		else if (!strcmp(button, "B"))
		{
			PadLocal.button |= m_gc_pad_buttons_bitmask[5];
			PadLocal.analogB = 0xFF;
		}
		else if (!strcmp(button, "X"))
		{
			PadLocal.button |= m_gc_pad_buttons_bitmask[6];
		}
		else if (!strcmp(button, "Y"))
		{
			PadLocal.button |= m_gc_pad_buttons_bitmask[7];
		}
		else if (!strcmp(button, "Z"))
		{
			PadLocal.button |= m_gc_pad_buttons_bitmask[8];
		}
		else if (!strcmp(button, "L"))
		{
			PadLocal.triggerLeft = 255;
			PadLocal.button |= m_gc_pad_buttons_bitmask[9];
		}
		else if (!strcmp(button, "R"))
		{
			PadLocal.triggerRight = 255;
			PadLocal.button |= m_gc_pad_buttons_bitmask[10];
		}
		else if (!strcmp(button, "Start"))
		{
			PadLocal.button |= m_gc_pad_buttons_bitmask[11];
		}
		else if (!strcmp(button, "D-Up"))
		{
			PadLocal.button |= m_gc_pad_buttons_bitmask[1];
		}
		else if (!strcmp(button, "D-Down"))
		{
			PadLocal.button |= m_gc_pad_buttons_bitmask[0];
		}
		else if (!strcmp(button, "D-Left"))
		{
			PadLocal.button |= m_gc_pad_buttons_bitmask[2];
		}
		else if (!strcmp(button, "D-Right"))
		{
			PadLocal.button |= m_gc_pad_buttons_bitmask[3];
		}		
	}
	void iReleaseButton(const char* button)
	{
		if (!strcmp(button, "A"))
		{
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[4];
			PadLocal.analogA = 0x00;
		}
		else if (!strcmp(button, "B"))
		{
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[5];
			PadLocal.analogB = 0x00;
		}
		else if (!strcmp(button, "X"))
		{
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[6];
		}
		else if (!strcmp(button, "Y"))
		{
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[7];
		}
		else if (!strcmp(button, "Z"))
		{
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[8];
		}
		else if (!strcmp(button, "L"))
		{
			PadLocal.triggerLeft = 0;
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[9];
		}
		else if (!strcmp(button, "R"))
		{
			PadLocal.triggerRight = 0;
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[10];
		}
		else if (!strcmp(button, "Start"))
		{
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[11];
		}
		else if (!strcmp(button, "D-Up"))
		{
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[1];
		}
		else if (!strcmp(button, "D-Down"))
		{
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[0];
		}
		else if (!strcmp(button, "D-Left"))
		{
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[2];
		}
		else if (!strcmp(button, "D-Right"))
		{
			PadLocal.button &= ~m_gc_pad_buttons_bitmask[3];
		}
	}

	void iSetMainStickX(int xVal)
	{
		PadLocal.stickX = xVal;
	}
	void iSetMainStickY(int yVal)
	{
		PadLocal.stickY = yVal;
	}
	void iSetCStickX(int xVal)
	{
		PadLocal.substickX = xVal;
	}
	void iSetCStickY(int yVal)
	{
		PadLocal.substickY = yVal;
	}
	void iSaveState(bool toSlot, int slotID, std::string fileName)
	{
		m_stateData.doSave = true;
		m_stateData.useSlot = toSlot;
		m_stateData.slotID = slotID;
		m_stateData.fileName = fileName;	

		lua_isStateSaved = false;
		lua_isStateLoaded = false;
		lua_isStateDone = false;
		lua_isStateOperation = true;

		Host_UpdateMainFrame();
	}
	void iLoadState(bool fromSlot, int slotID, std::string fileName)
	{
		m_stateData.doSave = false;
		m_stateData.useSlot = fromSlot;
		m_stateData.slotID = slotID;
		m_stateData.fileName = fileName;

		lua_isStateSaved = false;
		lua_isStateLoaded = false;
		lua_isStateDone = false;
		lua_isStateOperation = true;

		Host_UpdateMainFrame();
	}

	//Dragonbane: Custom Lua Scripts
	void ExecuteScripts(GCPadStatus* PadStatus)
	{
		if (!Core::IsRunningAndStarted())
			return;

		std::string gameID = SConfig::GetInstance().m_LocalCoreStartupParameter.GetUniqueID();
		u32 isLoadingAdd;
		u32 eventFlagAdd;
		u32 charPointerAdd;
		bool isTP = false;

		if (!gameID.compare("GZ2E01"))
		{
			eventFlagAdd = 0x40b16d;
			isLoadingAdd = 0x450ce0;

			isTP = true;
		}
		else if (!gameID.compare("GZ2P01"))
		{
			eventFlagAdd = 0x40d10d;
			isLoadingAdd = 0x452ca0;

			isTP = true;
		}

		//TWW Stuff
		bool isTWW = false;

		if (!gameID.compare("GZLJ01"))
		{
			isLoadingAdd = 0x3ad335;
			eventFlagAdd = 0x3bd3a2;
			charPointerAdd = 0x3ad860;

			isTWW = true;
		}

		//Superswim Script
		if (Movie::swimStarted && !Movie::swimInProgress) //Start Superswim
		{
			luaState = luaL_newstate();

			luaL_openlibs(luaState);

			//Reset vars
			lua_isStateOperation = false;
			lua_isStateSaved = false;
			lua_isStateLoaded = false;

			//For Button manipulation
			memset(&PadLocal, 0, sizeof(PadLocal));
			PadLocal = *PadStatus;

			//Make functions available to Lua programs
			lua_register(luaState, "ReadValue8", ReadValue8);
			lua_register(luaState, "ReadValue16", ReadValue16);
			lua_register(luaState, "ReadValue32", ReadValue32);
			lua_register(luaState, "ReadValueFloat", ReadValueFloat);
			lua_register(luaState, "ReadValueString", ReadValueString);
			lua_register(luaState, "GetPointerNormal", GetPointerNormal);

			lua_register(luaState, "WriteValue8", WriteValue8);
			lua_register(luaState, "WriteValue16", WriteValue16);
			lua_register(luaState, "WriteValue32", WriteValue32);
			lua_register(luaState, "WriteValueFloat", WriteValueFloat);
			lua_register(luaState, "WriteValueString", WriteValueString);

			lua_register(luaState, "PressButton", PressButton);
			lua_register(luaState, "ReleaseButton", ReleaseButton);
			lua_register(luaState, "SetMainStickX", SetMainStickX);
			lua_register(luaState, "SetMainStickY", SetMainStickY);
			lua_register(luaState, "SetCStickX", SetCStickX);
			lua_register(luaState, "SetCStickY", SetCStickY);

			lua_register(luaState, "SaveState", SaveState);
			lua_register(luaState, "LoadState", LoadState);

			lua_register(luaState, "GetFrameCount", GetFrameCount);
			lua_register(luaState, "MsgBox", MsgBox);
			lua_register(luaState, "AbortSwim", AbortSwim);

			std::string file = File::GetExeDirectory() + "\\Scripts\\Superswim.lua";

			int status = luaL_dofile(luaState, file.c_str());

			if (status == 0)
			{
				//Execute Start function
				lua_getglobal(luaState, "startSwim");

				lua_pushnumber(luaState, Movie::swimDestPosX);
				lua_pushnumber(luaState, Movie::swimDestPosZ);

				status = lua_pcall(luaState, 2, LUA_MULTRET, 0);
			}

			if (status != 0)
			{
				HandleLuaErrors(luaState, status);
				lua_close(luaState);

				Movie::swimStarted = false;
				return;
			}

			Movie::swimInProgress = true;
			*PadStatus = PadLocal;
		}
		else if (!Movie::swimStarted && Movie::swimInProgress) 	//Cancel Superswim
		{
			lua_getglobal(luaState, "cancelSwim");

			int status = lua_pcall(luaState, 0, LUA_MULTRET, 0);

			if (status != 0)
			{
				HandleLuaErrors(luaState, status);
			}

			lua_close(luaState);

			Movie::swimInProgress = false;
			*PadStatus = PadLocal;

			return;
		}
		else if (Movie::swimStarted && Movie::swimInProgress)
		{
			//Call Update function
			lua_getglobal(luaState, "updateSwim");

			int status = lua_pcall(luaState, 0, LUA_MULTRET, 0);

			if (status != 0)
			{
				HandleLuaErrors(luaState, status);

				lua_close(luaState);

				Movie::swimInProgress = false;
				Movie::swimStarted = false;
				return;
			}

			//LUA Callbacks
			if (lua_isStateOperation)
			{
				if (lua_isStateSaved)
				{
					//Saved State Callback
					lua_getglobal(luaState, "onStateSaved");

					int status = lua_pcall(luaState, 0, LUA_MULTRET, 0);

					if (status != 0)
					{
						HandleLuaErrors(luaState, status);

						lua_close(luaState);

						Movie::swimInProgress = false;
						Movie::swimStarted = false;
					}

					lua_isStateOperation = false;
					lua_isStateSaved = false;
					lua_isStateLoaded = false;

					return;
				}
				else if (lua_isStateLoaded)
				{
					//Loaded State Callback
					lua_getglobal(luaState, "onStateLoaded");

					int status = lua_pcall(luaState, 0, LUA_MULTRET, 0);

					if (status != 0)
					{
						HandleLuaErrors(luaState, status);

						lua_close(luaState);

						Movie::swimInProgress = false;
						Movie::swimStarted = false;
					}

					lua_isStateOperation = false;
					lua_isStateSaved = false;
					lua_isStateLoaded = false;

					return;
				}
			}

			*PadStatus = PadLocal;
		}
	}
}
