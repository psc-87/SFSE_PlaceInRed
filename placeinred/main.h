#pragma once

#include "pattern.h"
#include "sfse.h"
#include <cstring>
#include <vector>
#include <future> // For std::async and std::future


// macros
#define pirlog(fmt, ...) do { const char* thisfunc = __func__; _MESSAGE("[%s] " fmt, thisfunc, ##__VA_ARGS__); DebugLog::flush(); } while (0)
#define consoleprint(fmt, ...) Console_Print(fmt, ##__VA_ARGS__) 

// typedefs
typedef bool (*_ParseParams_Native) (const SCRIPT_PARAMETER* paramInfo, const char* someString, u32* opcodeOffsetPtr, TESObjectREFR* thisObj, void* containingObj, void* scriptObj, void* locals, ...);

// declarations
static bool ExecuteConsole(const SCRIPT_PARAMETER * paramInfo, const char* someString, TESObjectREFR * thisObj, TESObjectREFR * containingObj, Script * script, ScriptLocals * locals, float* result, u32 * opcodeOffsetPtr);

//static std::mutex logMutex;

class PlaceInRed {
private:
		
	SCRIPT_PARAMETER* ConsoleParams;

	bool bDebugPatternTimes = true;
	bool bGameDataIsReady = false; // true when SFSE says its ready
	bool bPlaceInRedEnabled = false; // set when toggling
	bool bObjectSnapEnabled = true;
	bool bGroundSnapEnabled = true;
	bool bSlowRotate = false;
	bool bCSavesPatched = false;
	bool bUnlimitedResources = false;

	f32 fSlowRotateSpeed = 0.2f;
	f32 fRotateSpeedOrig = 5.0f;
	f32 fClosestPointToleranceOrig = -0.04f;
	f32 fClosestPointToleranceLargeOrig = -0.1f;
	f32 fClosestPointToleranceNew = -5.0f;
	f32 fWorkshopGroundHeightForgivenessOrig = 0.2f;
	f32 fWorkshopGroundHeightForgivenessNew = 10.0f;

	u8 AL_OLD[3] = { 0x3C, 0x02, 0x74 }; // cmp al,02; JE
	u8 AL_NEW[3] = { 0xB0, 0x02, 0xEB }; // mov al,02; JMP
	u8 E4_OLD[2] = { 0x75, 0x07 }; // jne 07
	u8 E4_NEW[2] = { 0xEB, 0x07 }; // jmp 07
	u8 E5_OLD[2] = { 0xB0, 0x01 }; // mov al,01
	u8 E5_NEW[2] = { 0xB0, 0x00 }; // mov al,00
	u8 E6_OLD[3] = { 0x8A, 0x45, 0xC0 }; // mov al,[rbp-40]
	u8 E6_NEW[3] = { 0xB0, 0x01, 0x90 }; // mov al,01; nop
	u8 E8_OLD[2] = { 0x74, 0x0E }; // je  0x0E
	u8 E8_NEW[2] = { 0xEB, 0x0E }; // jmp 0x0E
	u8 EB1_OLD[2] = { 0x75, 0x07 }; // jne 07
	u8 EB1_NEW[2] = { 0xEB, 0x00 }; // jmp 00
	u8 EB2_OLD[6] = { 0x0F, 0x84, 0x7C, 0x01, 0x00, 0x00 }; // je  Starfield.exe+1D6A005
	u8 EB2_NEW[6] = { 0xE9, 0x89, 0x01, 0x00, 0x00, 0x90 }; // jmp Starfield.exe+1D6A011; nop
	u8 BL_OLD[6] = { 0x88, 0x9F, 0x8B, 0x01, 0x00, 0x00 }; // mov [rdi+0000018B],bl 
	u8 BL_NEW[6] = { 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 }; // 6 byte nop
	u8 RED_OLD[2] = { 0x75, 0x24 }; // jne 0x24
	u8 RED_NEW[2] = { 0xEB, 0x24 }; // jmp 0x24
	u8 RED2_OLD[3] = { 0x44, 0x8A, 0xF8 }; // mov r15b,al
	u8 RED2_NEW[3] = { 0xB0, 0X01, 0x90 }; // mov al,01; nop
	u8 OSNAP_OLD[6] = { 0x0F, 0x84, 0x82, 0x00, 0x00, 0x00 }; // je Starfield.exe+2DCB47C
	u8 OSNAP_NEW[6] = { 0xE9, 0x83, 0x00, 0x00, 0x00, 0x90 }; // jmp Starfield.exe+2DCB47C
	u8 CSAVES_OLD[2] = { 0x74, 0x25 }; // jne 0x24
	u8 CSAVES_NEW[2] = { 0xEB, 0x6C }; // jmp to end of function
	
	

public:
	u64 start_tickcount = GetTickCount64();
	u64 end_tickcount = 0;

	std::string s_PluginPath_INI = "Data\\SFSE\\Plugins\\PlaceInRed.ini";
	std::string s_PluginPath_Log = "\\My Games\\Starfield\\SFSE\\Logs\\PlaceInRed.txt";
	DebugLog PluginDebugLog;
	PluginHandle pluginHandle = kPluginHandle_Invalid;
	SFSEMessagingInterface* g_messaging = nullptr;
	SFSEMenuInterface* g_menuInterface = nullptr;

	bool bAllPatternsFound = false; // set true when all patterns found
	uintptr_t SFBaseAddr = 0;
	
	uintptr_t* RotateFinder = nullptr;
	uintptr_t RotateAddr = 0;
	
	uintptr_t* WSModeFinder = nullptr;
	uintptr_t WSModeAddr = 0;
	
	uintptr_t* ClosestPointFinder = nullptr;
	uintptr_t ClosestPointAddr = 0;
	
	uintptr_t* ClosestPointLargeFinder = nullptr;
	uintptr_t ClosestPointLargeAddr = 0;
	
	uintptr_t* GroundHeightFinder = nullptr;
	uintptr_t GroundHeightAddr = 0;
	
	uintptr_t* gConsoleFinder = nullptr;
	uintptr_t gConsoleAddr = 0;
	uintptr_t* gConsole_vPrintFinder = nullptr;
	uintptr_t gConsole_vPrintAddr = 0;
	
	Script::SCRIPT_FUNCTION* firstConsole = nullptr;
	uintptr_t* firstConsoleFinder = nullptr;
	
	Script::SCRIPT_FUNCTION* firstScript = nullptr;
	uintptr_t* firstScriptFinder = nullptr;
	
	uintptr_t* ParseParams = nullptr;
	_ParseParams_Native ParseParams_Native = nullptr;

	// Simple pointers we patch close to
	uintptr_t* AL = nullptr;
	uintptr_t* E4 = nullptr;     // 00 = good
	uintptr_t* E5 = nullptr;     // 00 = good
	uintptr_t* E6 = nullptr;     // 01 = good
	uintptr_t* E8 = nullptr;     // 00 = good
	uintptr_t* EB = nullptr;     // 02 = good
	uintptr_t* ONE = nullptr;    // 02 = good (starts as 00 or 01)
	uintptr_t* BL = nullptr;     // writes bad places
	uintptr_t* FOURS = nullptr;  // 04 -> 02
	uintptr_t* RED = nullptr;    // red
	uintptr_t* RED2 = nullptr;   // red2
	uintptr_t* TALLRED = nullptr;   // red when too tall
	uintptr_t* RESOURCES = nullptr;   // resources 0x18
	uintptr_t* OSNAP = nullptr;   // objectsnap decision
	uintptr_t* GSNAP = nullptr;   // groundsnap float
	uintptr_t* GSNAP2 = nullptr;   // "bUseAlwaysFollowGround:Workshop"
	uintptr_t* GSNAP3 = nullptr;   // "bWorkshopTempItemFollowGround:Workshop"
	uintptr_t* CSAVES = nullptr; // block C saves
	uintptr_t* HideFirstConsoleMSG = nullptr; // dont show the achievement message on first console run

public:
	// initialize
	PlaceInRed()
	{		
		// console parameters
		ConsoleParams = new SCRIPT_PARAMETER[2];
		ConsoleParams[0].pParamName = "param1";
		ConsoleParams[0].eParamType = 0; // string
		ConsoleParams[0].bOptional = 0;
		ConsoleParams[1].pParamName = "param2";
		ConsoleParams[1].eParamType = 0; // string
		ConsoleParams[1].bOptional = 1;

		
	}

	// deconstruct
	~PlaceInRed()
	{
		delete[] ConsoleParams;
	}

	void CreateLogFile() const
	{
		this->PluginDebugLog.openRelative(0x0005, this->s_PluginPath_Log.c_str());
	}

	// set sf base address
	uintptr_t SetStarfieldBaseAddress()
	{
		if (!SFBaseAddr){
			SFBaseAddr = reinterpret_cast<uintptr_t>(GetModuleHandleA(0));
		}

		return SFBaseAddr;
	}

	std::string& GetPluginINIPath() const {
		static std::string s_configPath = getRuntimeDirectory() + s_PluginPath_INI;
		return s_configPath;
	}

	// Return an ini setting as a std::string
	std::string GetINISettingAsString(const char* section, const char* key) const {
		std::string result;
		const std::string& configPath = GetPluginINIPath();
		if (!configPath.empty()) {
			char resultBuf[2048]{};
			resultBuf[0] = 0;
			u32 resultLen = GetPrivateProfileString(section, key, NULL, resultBuf, sizeof(resultBuf), configPath.c_str());
			result = resultBuf;
		}
		return result;
	}

	// string to float for use with simple ini settings
	f32 StringToFloat(std::string fString, f32 min = 0.001, f32 max = 99999, f32 error = 0)
	{
		f32 theFloat = 0;
		try
		{
			theFloat = std::stof(fString);
		}
		catch (...)
		{
			return error;
		}
		if (theFloat > min && theFloat < max){
			return theFloat;
		} else {
			return error;
		}
	}

	// Read memory in to a data buffer
	bool ReadMemory(uintptr_t addr, void* data, size_t len)
	{
		DWORD oldProtect;
		// Change memory protection to allow read/write/execute access
		if (VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) {
			// Copy memory to buffer
			memcpy(data, (void*)addr, len);

			// Restore the original memory protection
			if (VirtualProtect((void*)addr, len, oldProtect, &oldProtect)) {
				return true;
			}
		}
		return false;
	}

	// return rel32 from a pattern match
	s32 CalculateRel32(uintptr_t* ptrPattern, s32 r32start, s32 end) const
	{
		if (!ptrPattern) {
			return 0;
		}

		s32 relish32 = *reinterpret_cast<s32*>(uintptr_t(ptrPattern) + r32start);
		return (uintptr_t(ptrPattern) + end + relish32) - SFBaseAddr;
	}

	// these need to wait for gamedata load and are called at that time
	void ReadPluginINISettings()
	{
		

		// Toggle CSaves
		std::string ini_csaves = GetINISettingAsString("Main", "CSAVES_PATCHED");
		if (!bCSavesPatched && ini_csaves == "1") { ToggleCSaves(); }

		// slowrotate speed
		std::string ini_rotateSpeed = GetINISettingAsString("Main", "fSlowRotateSpeed");
		fSlowRotateSpeed = StringToFloat(ini_rotateSpeed, 0.1f, 10.0f, 0.2f);

		// Toggle SlowRotate
		std::string ini_slowRotate = GetINISettingAsString("Main", "bSlowRotate");
		if (bSlowRotate && ini_slowRotate == "1") { ToggleSlowRotate(); }

		// Toggle PlaceInRed
		std::string ini_placeinred = GetINISettingAsString("Main", "PLACEINRED_ENABLED");
		if (!bPlaceInRedEnabled && ini_placeinred == "1") { TogglePlaceInRed(); }

		// Toggle GroundSnap
		std::string ini_groundSnap = GetINISettingAsString("Main", "GROUNDSNAP_ENABLED");
		if (bGroundSnapEnabled && ini_groundSnap == "0") { ToggleGroundSnap(); }

		// Toggle ObjectSnap
		std::string ini_objectSnap = GetINISettingAsString("Main", "OBJECTSNAP_ENABLED");
		if (bObjectSnapEnabled && ini_objectSnap == "0") { ToggleObjectSnap(); }

		pirlog("Done.");

	}

	// Read original game ini settings before we start changing them
	void ReadOriginalGameINISettings()
	{
		if (RotateAddr) {
			ReadMemory(RotateAddr, &fRotateSpeedOrig, sizeof(f32));
		}
		if (ClosestPointAddr) {
			ReadMemory(ClosestPointAddr, &fClosestPointToleranceOrig, sizeof(f32));
		}
		if (ClosestPointLargeAddr) {
			ReadMemory(ClosestPointLargeAddr, &fClosestPointToleranceLargeOrig, sizeof(f32));
		}
		if (GroundHeightAddr) {
			ReadMemory(GroundHeightAddr, &fWorkshopGroundHeightForgivenessOrig, sizeof(f32));
		}

		pirlog("Done.");
	}

	void TogglePlaceInRed() {

		if (bPlaceInRedEnabled)
		{
			safeWriteBuf(ClosestPointAddr, &fClosestPointToleranceOrig, sizeof(f32));
			safeWriteBuf(ClosestPointLargeAddr, &fClosestPointToleranceLargeOrig, sizeof(f32));
			safeWriteBuf(GroundHeightAddr, &fWorkshopGroundHeightForgivenessOrig, sizeof(f32));
			safeWrite8((uintptr_t)WSModeAddr - 0x01, 0x02);
			safeWriteBuf((uintptr_t)AL, AL_OLD, sizeof(AL_OLD));
			safeWriteBuf((uintptr_t)BL, BL_OLD, sizeof(BL_OLD));
			safeWriteBuf((uintptr_t)E4, E4_OLD, sizeof(E4_OLD));
			safeWriteBuf((uintptr_t)E5, E5_OLD, sizeof(E5_OLD));
			safeWriteBuf((uintptr_t)E6, E6_OLD, sizeof(E6_OLD));
			safeWriteBuf((uintptr_t)E8, E8_OLD, sizeof(E8_OLD));
			safeWriteBuf((uintptr_t)EB + 0x07, EB1_OLD, sizeof(EB1_OLD));
			safeWriteBuf((uintptr_t)EB + 0x17, EB2_OLD, sizeof(EB2_OLD));
			safeWrite8((uintptr_t)EB + 0x0F, 0x04);
			safeWrite8((uintptr_t)EB + 0x2F, 0x74);
			safeWrite8((uintptr_t)TALLRED, 0x74);
			safeWrite8((uintptr_t)ONE + 0x06, 0x01);
			safeWrite8((uintptr_t)FOURS + 0x06, 0x04);
			safeWrite8((uintptr_t)FOURS + 0x1D, 0x04);
			safeWriteBuf((uintptr_t)RED, RED_OLD, sizeof(RED_OLD));
			safeWriteBuf((uintptr_t)RED2 + 0x16, RED2_OLD, sizeof(RED2_OLD));
			safeWrite8((uintptr_t)RESOURCES + 0x06, 0x18);
		} else {
			safeWriteBuf(ClosestPointAddr, &fClosestPointToleranceNew, sizeof(f32));
			safeWriteBuf(ClosestPointLargeAddr, &fClosestPointToleranceNew, sizeof(f32));
			safeWriteBuf(GroundHeightAddr, &fWorkshopGroundHeightForgivenessNew, sizeof(f32));
			safeWrite8((uintptr_t)WSModeAddr - 0x01, 0x02);
			safeWriteBuf((uintptr_t)AL, AL_NEW, sizeof(AL_NEW));
			safeWriteBuf((uintptr_t)BL, BL_NEW, sizeof(BL_NEW));
			safeWriteBuf((uintptr_t)E4, E4_NEW, sizeof(E4_NEW));
			safeWriteBuf((uintptr_t)E5, E5_NEW, sizeof(E5_NEW));
			safeWriteBuf((uintptr_t)E6, E6_NEW, sizeof(E6_NEW));
			safeWriteBuf((uintptr_t)E8, E8_NEW, sizeof(E8_NEW));
			safeWriteBuf((uintptr_t)EB + 0x07, EB1_NEW, sizeof(EB1_NEW));
			safeWriteBuf((uintptr_t)EB + 0x17, EB2_NEW, sizeof(EB2_NEW));
			safeWrite8((uintptr_t)EB + 0x0F, 0x02);
			safeWrite8((uintptr_t)EB + 0x2F, 0xEB);
			safeWrite8((uintptr_t)TALLRED, 0xEB);
			safeWrite8((uintptr_t)ONE + 0x06, 0x02);
			safeWrite8((uintptr_t)FOURS + 0x06, 0x02);
			safeWrite8((uintptr_t)FOURS + 0x1D, 0x02);
			safeWriteBuf((uintptr_t)RED, RED_NEW, sizeof(RED_NEW));
			safeWriteBuf((uintptr_t)RED2 + 0x16, RED2_NEW, sizeof(RED2_NEW));
			safeWrite8((uintptr_t)RESOURCES + 0x06, 0x02);
		}

		bPlaceInRedEnabled = !bPlaceInRedEnabled;
		consoleprint(bPlaceInRedEnabled ? "Place in Red - Enabled" : "Place in Red - Disabled");
	}

	void ToggleObjectSnap()
	{
		if (bObjectSnapEnabled) {
			safeWriteBuf((uintptr_t)OSNAP, OSNAP_NEW, sizeof(OSNAP_NEW));
		} else { 
			safeWriteBuf((uintptr_t)OSNAP, OSNAP_OLD, sizeof(OSNAP_OLD)); 
		}
		bObjectSnapEnabled = !bObjectSnapEnabled;
		consoleprint(bObjectSnapEnabled ? "ObjectSnap - Enabled" : "ObjectSnap - Disabled");
	}

	void ToggleGroundSnap()
	{
		if (bGroundSnapEnabled){
			safeWrite8((uintptr_t)GSNAP + 0x01, 0x00);
			safeWrite8((uintptr_t)GSNAP2 + 0x08, 0x00);
			//safeWrite8((uintptr_t)GSNAP3 + 0x09, 0x93);
		} else {
			safeWrite8((uintptr_t)GSNAP + 0x01, 0x0F);
			safeWrite8((uintptr_t)GSNAP2 + 0x08, 0x25);
			//safeWrite8((uintptr_t)GSNAP3 + 0x09, 0x94);
		}
		bGroundSnapEnabled = !bGroundSnapEnabled;
		consoleprint(bGroundSnapEnabled ? "GroundSnap - Enabled" : "GroundSnap - Disabled");
	}

	void ToggleSlowRotate() {

		if (bSlowRotate) {
			safeWriteBuf(RotateAddr, &fRotateSpeedOrig, sizeof(f32));
		}
		else {
			safeWriteBuf(RotateAddr, &fSlowRotateSpeed, sizeof(f32));
		}

		bSlowRotate = !bSlowRotate;
		consoleprint(bSlowRotate ? "Slow Rotate - Enabled (%f)" : "Slow Rotate - Disabled", fSlowRotateSpeed);
	}
	
	void ToggleCSaves() {

		if (bCSavesPatched) {
			safeWriteBuf((uintptr_t)CSAVES, &CSAVES_OLD, sizeof(CSAVES_OLD));
			safeWrite8((uintptr_t)HideFirstConsoleMSG + 0x05, 0x84);
			
		} else {
			safeWriteBuf((uintptr_t)CSAVES, &CSAVES_NEW, sizeof(CSAVES_NEW));
			safeWrite8((uintptr_t)HideFirstConsoleMSG + 0x05, 0x83);
		}

		bCSavesPatched = !bCSavesPatched;
		consoleprint(bCSavesPatched ? "C Saves Patched" : "C Saves Unpatched (game default)");
	}

	bool PatchConsole(const char* hijacked_cmd_fullname) const
	{
		if (!firstConsoleFinder || !firstConsole) {
			pirlog("failed! firstConsole is null.");
			return false;
		}

		const char* cmdname = hijacked_cmd_fullname;

		for (Script::SCRIPT_FUNCTION* iter = firstConsole; iter->eOutput < (0x0245 + 0x0100); iter++) {
			if (!iter->pExecuteFunction) {
				continue;
			}

			if (!strcmp(iter->pFunctionName, cmdname)) {

				// Found the command
				Script::SCRIPT_FUNCTION& consoleCMD = *iter;

				// Start patching
				consoleCMD.pExecuteFunction = ExecuteConsole;
				consoleCMD.pFunctionName = "placeinred";
				consoleCMD.pShortName = "pir";
				consoleCMD.pHelpString = "pir [option]: pir 1, pir 2, pir toggle, etc.";
				consoleCMD.pParameters = &ConsoleParams[0];
				consoleCMD.sParamCount = 2;

				pirlog("Patched %s at %p", cmdname, (uintptr_t)iter);
				return true;
			}
		}
		pirlog("Failed to patch '%s'. Another plugin may have used it.", hijacked_cmd_fullname);
		return false;
	}

	// switch with string
	static constexpr unsigned int ConsoleSwitch(const char* s, int off = 0) {
		return !s[off] ? 5381 : (ConsoleSwitch(s, off + 1) * 33) ^ s[off];
	}

	// clean help string for logs
	std::string CleanHelpString(const char* str)
	{
		size_t len = strlen(str);
		std::string newStr;
		newStr.reserve(len); // Reserve space for efficiency

		const char* src = str;
		while (*src)
		{
			// exclude pipes and special chars
			if ((*src > 0x19) && (*src <= 0x7E) && (*src != 0x7C))
			{
				newStr += *src;
			}
			src++;
		}

		return newStr;
	}

	// run function based on param 1
	void RunConsoleFunction(const char* p1)
	{
		switch (ConsoleSwitch(p1))
		{
		case ConsoleSwitch("1"): TogglePlaceInRed(); return;
		case ConsoleSwitch("toggle"): TogglePlaceInRed(); return;

		case ConsoleSwitch("2"): ToggleObjectSnap(); return;
		case ConsoleSwitch("osnap"): ToggleObjectSnap(); return;

		case ConsoleSwitch("3"): ToggleGroundSnap(); return;
		case ConsoleSwitch("gsnap"): ToggleGroundSnap(); return;

		case ConsoleSwitch("4"): ToggleSlowRotate(); return;
		case ConsoleSwitch("slow"): ToggleSlowRotate(); return;

		case ConsoleSwitch("c"): ToggleCSaves(); return;
		case ConsoleSwitch("7"): ToggleCSaves(); return;

		default: consoleprint("pir - invalid option specified"); return;
		}
	}

	// dump all script commands to the log file
	void DumpScripts(bool includeParams = false)
	{
		if (firstScript == nullptr)
		{
			pirlog("failed! firstScript is a nullptr");
			return;
		}

		// log all console commands
		for (Script::SCRIPT_FUNCTION* iter = firstScript; iter->eOutput < (0x03C0 + 0x1000); ++iter)
		{

			if (!iter->pExecuteFunction) { continue; }

			// remove new lines and pipes from help string
			std::string cleanhelp = CleanHelpString(iter->pHelpString);
			u16 paramCount = iter->sParamCount;
			pirlog("%u|%p|%s|%s|%u|%s", iter->eOutput - 0x100, (uintptr_t)iter->pExecuteFunction, iter->pFunctionName, iter->pShortName, paramCount, cleanhelp);

			// log parameters optionally
			if (includeParams) {
				if (iter->pParameters && paramCount > 0) // remove false if you want it
				{
					char allParams[8192] = { 0 };
					for (u16 i = 0; i < paramCount; ++i)
					{

						SCRIPT_PARAMETER* param = &iter->pParameters[i];
						u32 pType = param->eParamType;
						const char* pName = param->pParamName ? param->pParamName : "(null)";

						// write the param details to a temp buffer
						char tempbuffer[256];
						snprintf(tempbuffer, sizeof(tempbuffer), "%02u: %s", i, pName);
						// concat
						strncat_s(allParams, tempbuffer, sizeof(allParams) - strlen(allParams) - 1);
					}

					pirlog("%s", allParams);
				}
			}
		}
	}

	// dump all console commands to the log file
	void DumpCMDs(bool includeParams = false)
	{
		if (firstConsole == nullptr)
		{
			pirlog("failed! firstConsole is a nullptr");
			return;
		}

		for (Script::SCRIPT_FUNCTION* iter = firstConsole; iter->eOutput < (0x0245 + 0x100); ++iter)
		{
			if (!iter->pExecuteFunction) { continue; }

			
			std::string cleanhelp = CleanHelpString(iter->pHelpString);
			u16 paramCount = iter->sParamCount;
			pirlog("%u|%p|%s|%s|%u|%s", iter->eOutput - 0x100, (uintptr_t)iter->pExecuteFunction, iter->pFunctionName, iter->pShortName, paramCount, cleanhelp);

			// log parameters optionally
			if (includeParams)
			{
				if (iter->pParameters && paramCount > 0)
				{
					char allParams[8192] = { 0 };
					for (u16 i = 0; i < paramCount; ++i)
					{

						SCRIPT_PARAMETER* param = &iter->pParameters[i];
						u32 pType = param->eParamType;
						const char* pName = param->pParamName ? param->pParamName : "(null)";

						// write the param details to a temp buffer
						char tempbuffer[256];
						snprintf(tempbuffer, sizeof(tempbuffer), "%02u: %s", i, pName);
						// concat
						strncat_s(allParams, tempbuffer, sizeof(allParams) - strlen(allParams) - 1);
					}

					pirlog("%s", allParams);
				}
			}
		}
	}

	// every pointer the plugin uses is valid
	bool AreAllPointersValid() const
	{
		return this->GSNAP3 && this->GSNAP2 && this->RotateFinder && this->GSNAP &&
			this->OSNAP && this->E6 && this->RED2 && this->AL && this->BL &&
			this->EB && this->ONE && this->FOURS && this->E5 && this->RED &&
			this->RESOURCES && this->WSModeFinder && this->GroundHeightFinder &&
			this->HideFirstConsoleMSG && this->ClosestPointFinder &&
			this->ClosestPointLargeFinder && this->E4 && this->E8 &&
			this->CSAVES && this->gConsoleFinder && this->firstConsoleFinder &&
			this->firstScriptFinder && this->ParseParams;
	}

	// Wrapper to handle asynchronous Utility::pattern
	template <typename T, size_t N>
	std::future<void> FindPatternAsync(T& ptr_address, const char(&pattern)[N])
	{
		return std::async(std::launch::async, [&ptr_address, &pattern]{
			ptr_address = Utility::pattern(pattern).count(1).get(0).get<uintptr_t>();
		});
	}

	// set up the plugin
	void PluginInit()
	{
		std::vector<std::future<void>> vecFutures;

		vecFutures.emplace_back(FindPatternAsync(this->AL, "3C 02 74 2A 88 87 8B 01 00 00 45 33 C9"));
		vecFutures.emplace_back(FindPatternAsync(this->BL, "88 9F ? ? 00 00 48 8B 8F ? ? 00 00 48 8B 01 FF 90"));
		vecFutures.emplace_back(FindPatternAsync(this->CSAVES, "74 25 48 8B 05 ? ? ? ? 48 85 C0 74 09 F6 80 ? ? ? ? 04 75 09 80"));
		vecFutures.emplace_back(FindPatternAsync(this->ClosestPointFinder, "C5 FA 10 35 ? ? ? ? 48 8B 01 4C 8B A8 ? ? ? ? 49 8B 45 00 49"));
		vecFutures.emplace_back(FindPatternAsync(this->ClosestPointLargeFinder, "C5 FA 10 35 ? ? ? ? 4C 8B AD ? ? ? ? 49 8B 45 00 33 FF C5"));
		vecFutures.emplace_back(FindPatternAsync(this->E4, "75 07 44 88 8B ? ? 00 00 44 8A 83 ? ? 00 00 8A 45 ? 45 84 C0"));
		vecFutures.emplace_back(FindPatternAsync(this->E5, "B0 01 88 83 ? ? 00 00 4C 8B ? ? 4C 8B ? ? EB 06 8A 83 ? ? ? ? 84 C0"));
		vecFutures.emplace_back(FindPatternAsync(this->E6, "8A 45 C0 88 83 ? ? 00 00 4C 8B 8B ? ? 00 00 49 83 C1 08 C5"));
		vecFutures.emplace_back(FindPatternAsync(this->E8, "74 0E 8A 85 ? ? ? ? 88 87 ? ? 00 00 EB 03 44"));
		vecFutures.emplace_back(FindPatternAsync(this->EB, "80 BB ? ? 00 00 02 75 07 C6 83 ? ? 00 00 04 44 ? ? ? ? 00 00 0F 84"));
		vecFutures.emplace_back(FindPatternAsync(this->TALLRED, "74 09 41 C6 84 24 8A 01 00 00 01 C5 FA 10 75 70 C5 F8 2E FE"));
		vecFutures.emplace_back(FindPatternAsync(this->FOURS, "C6 83 ? ? 00 00 04 44 38 93 ? ? 00 00 74 11 C6 83 ? ? 00 00 01 C6 83 ? ? 00 00 04"));
		vecFutures.emplace_back(FindPatternAsync(this->GSNAP, "72 0F 48 8B 87 ? ? 00 00 80 ? ? 01 8A C3 75 02 B0 01 84 C9"));
		vecFutures.emplace_back(FindPatternAsync(this->GSNAP2, "44 38 1D ? ? ? ? 75 25 83 B9 ? ? 00 00 03 75 05 41 B2 01 EB 17 48 8B 0D"));
		vecFutures.emplace_back(FindPatternAsync(this->GSNAP3, "80 3D ? ? ? ? 00 41 0F 94 C0 49 8B 56 50 48 8D 8D ? ? 00 00"));
		vecFutures.emplace_back(FindPatternAsync(this->GroundHeightFinder, "C5 FA 10 05 ? ? ? ? C5 FA 58 48 08 48 8B 47 10 41"));
		vecFutures.emplace_back(FindPatternAsync(this->HideFirstConsoleMSG, "80 79 72 00 0F 84 A7 00 00 00 48 8B 1D ? ? ? ? 48 C7 44 24 20 00 00 00 00"));
		vecFutures.emplace_back(FindPatternAsync(this->ONE, "C6 83 ? ? 00 00 01 89 BB ? ? 00 00 8B 05 ? ? ? ? 89 83 ? 00 00 00 C7"));
		vecFutures.emplace_back(FindPatternAsync(this->OSNAP, "0F 84 82 00 00 00 C5 FA 10 35 ? ? ? ? 48 8B 0B E8 ? ? ? ? C5"));
		vecFutures.emplace_back(FindPatternAsync(this->ParseParams, "4C 8B DC 48 83 EC 48 49 8D 43 40 49 89 43 F0"));
		vecFutures.emplace_back(FindPatternAsync(this->RED, "75 24 48 8D 4D ? E8 ? ? ? ? 84 C0 75 17 44 38 93 ? ? ? ? 74 09 44"));
		vecFutures.emplace_back(FindPatternAsync(this->RED2, "C5 D0 55 C2 C5 F8 59 E7 C5 D8 59 C1 C5 F8 11 45 D0 E8"));
		vecFutures.emplace_back(FindPatternAsync(this->RESOURCES, "C6 ? ? ? 00 00 18 EB 34 44 38 ? ? ? 00 00 74 09 C6 83 ? ? 00 00 1F"));
		vecFutures.emplace_back(FindPatternAsync(this->RotateFinder, "C5 FA 10 0D ? ? ? ? C5 F2 59 15 ? ? ? ? C5 FA 5D 1D ? ? ? ? 84"));
		vecFutures.emplace_back(FindPatternAsync(this->WSModeFinder, "C6 05 ? ? ? ? 01 49 8B CE E8 ? ? ? ? 4C 8D ? ? ? 49 8B ? ? 49 8B"));
		vecFutures.emplace_back(FindPatternAsync(this->firstConsoleFinder, "4B 8B 94 3E ? ? ? ? 48 85 D2 74 22 48 8B CB FF 15"));
		vecFutures.emplace_back(FindPatternAsync(this->firstScriptFinder, "4A 8B 94 3E ? ? ? ? 48 8B CB FF 15 ? ? ? ? 85 C0 74 19"));
		vecFutures.emplace_back(FindPatternAsync(this->gConsoleFinder, "48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? E8 ? ? ? ? C5 F8 28 74 24 ? 32 C0"));
		vecFutures.emplace_back(FindPatternAsync(this->gConsole_vPrintFinder, "74 14 48 8B 54 24 30 4C 8B C3 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 83 C4 20 5B C3"));


		// Wait for all async tasks to complete
		for (auto& future : vecFutures) {
			future.get();
		}

		if (AreAllPointersValid() == true)
		{
			bAllPatternsFound = true;
		}

		if (E4) { ReadMemory((uintptr_t(E4)), &E4_OLD, sizeof(E4_OLD)); }
		if (E5) { ReadMemory((uintptr_t(E5)), &E5_OLD, sizeof(E5_OLD)); }
		if (E6) { ReadMemory((uintptr_t(E6)), &E6_OLD, sizeof(E6_OLD)); }
		if (E8) { ReadMemory((uintptr_t(E8)), &E8_OLD, sizeof(E8_OLD)); }
		if (BL) { ReadMemory((uintptr_t(BL)), &BL_OLD, sizeof(BL_OLD)); }
		if (EB) {
			ReadMemory((uintptr_t(EB) + 0x07), &EB1_OLD, sizeof(EB1_OLD));
			ReadMemory((uintptr_t(EB) + 0x17), &EB2_OLD, sizeof(EB2_OLD));
		}

		if (ClosestPointFinder) {
			s32 ClosestPointr32 = CalculateRel32(ClosestPointFinder, 0x04, 0x08);
			ClosestPointAddr = SFBaseAddr + ClosestPointr32;
		}

		if (ClosestPointLargeFinder) {
			s32 ClosestPointLarger32 = CalculateRel32(ClosestPointLargeFinder, 0x04, 0x08);
			ClosestPointLargeAddr = SFBaseAddr + ClosestPointLarger32;
		}

		if (GroundHeightFinder) {
			s32 GroundHeightr32 = CalculateRel32(GroundHeightFinder, 0x04, 0x08);
			GroundHeightAddr = SFBaseAddr + GroundHeightr32;
		}

		if (WSModeFinder) {
			s32 wsmoderel32 = CalculateRel32(WSModeFinder, 0x02, 0x07);
			WSModeAddr = SFBaseAddr + wsmoderel32;
		}

		if (RotateFinder) {
			s32 Rotater32 = CalculateRel32(RotateFinder, 0x04, 0x08);
			RotateAddr = SFBaseAddr + Rotater32;
		}

		if (gConsoleFinder) {
			s32 gConsoler32 = CalculateRel32(gConsoleFinder, 0x03, 0x07);
			gConsoleAddr = SFBaseAddr + gConsoler32;
		}

		if (gConsole_vPrintFinder) {
			s32 vPrintFinder32 = CalculateRel32(gConsole_vPrintFinder, 0x12, 0x16);
			gConsole_vPrintAddr = SFBaseAddr + vPrintFinder32;
		}

		if (firstConsoleFinder) {
			s32 fconsoler32 = 0;
			ReadMemory(uintptr_t(firstConsoleFinder) + 0x04, &fconsoler32, sizeof(s32));
			firstConsole = RelocPtr<Script::SCRIPT_FUNCTION>(fconsoler32);
		}

		if (firstScriptFinder) {
			s32 fscriptr32 = 0;
			ReadMemory(uintptr_t(firstScriptFinder) + 0x04, &fscriptr32, sizeof(s32));
			firstScript = RelocPtr<Script::SCRIPT_FUNCTION>(fscriptr32);
		}

		if (ParseParams) {
			s32 ParseParamsr32 = uintptr_t(ParseParams) - SFBaseAddr;
			ParseParams_Native = RelocAddr<_ParseParams_Native>(ParseParamsr32);
		}

		pirlog("Done.");
	}


	// log memory addresses to file
	void LogMemory() const {
		pirlog("\n----------------------------------------------------------\n"
			"Base        : %p Starfield.exe+0x0\n"
			"ConsoleMSG  : %p Starfield.exe+0x%08X\n"
			"CPT         : %p Starfield.exe+0x%08X\n"
			"CPTLarge    : %p Starfield.exe+0x%08X\n"
			"GroundHeight: %p Starfield.exe+0x%08X\n"
			"Rotate      : %p Starfield.exe+0x%08X\n"
			"AL          : %p Starfield.exe+0x%08X\n"
			"E4          : %p Starfield.exe+0x%08X\n"
			"E5          : %p Starfield.exe+0x%08X\n"
			"E6          : %p Starfield.exe+0x%08X\n"
			"E8          : %p Starfield.exe+0x%08X\n"
			"EB          : %p Starfield.exe+0x%08X\n"
			"BL          : %p Starfield.exe+0x%08X\n"
			"ONE         : %p Starfield.exe+0x%08X\n"
			"RED         : %p Starfield.exe+0x%08X\n"
			"RED2        : %p Starfield.exe+0x%08X\n"
			"FOURS       : %p Starfield.exe+0x%08X\n"
			"RESOURCES   : %p Starfield.exe+0x%08X\n"
			"OSNAP       : %p Starfield.exe+0x%08X\n"
			"GSNAP       : %p Starfield.exe+0x%08X\n"
			"GSNAP2      : %p Starfield.exe+0x%08X\n"
			"GSNAP3      : %p Starfield.exe+0x%08X\n"
			"WSMode      : %p Starfield.exe+0x%08X\n"
			"CPREVENT    : %p Starfield.exe+0x%08X\n"
			"ParseParams : %p Starfield.exe+0x%08X\n"
			"gConsole    : %p Starfield.exe+0x%08X\n"
			"vPrint      : %p Starfield.exe+0x%08X\n"
			"fconsole    : %p Starfield.exe+0x%08X\n"
			"fscript     : %p Starfield.exe+0x%08X\n"
			"----------------------------------------------------------",
			SFBaseAddr,
			HideFirstConsoleMSG, (uintptr_t)HideFirstConsoleMSG - SFBaseAddr,
			ClosestPointFinder, ClosestPointAddr - SFBaseAddr,
			ClosestPointLargeFinder, ClosestPointLargeAddr - SFBaseAddr,
			GroundHeightFinder, GroundHeightAddr - SFBaseAddr,
			RotateFinder, RotateAddr - SFBaseAddr,
			AL, (uintptr_t)AL - SFBaseAddr,
			E4, (uintptr_t)E4 - SFBaseAddr,
			E5, (uintptr_t)E5 - SFBaseAddr,
			E6, (uintptr_t)E6 - SFBaseAddr,
			E8, (uintptr_t)E8 - SFBaseAddr,
			EB, (uintptr_t)EB - SFBaseAddr,
			BL, (uintptr_t)BL - SFBaseAddr,
			ONE, (uintptr_t)ONE - SFBaseAddr,
			RED, (uintptr_t)RED - SFBaseAddr,
			RED2, (uintptr_t)RED2 - SFBaseAddr,
			FOURS, (uintptr_t)FOURS - SFBaseAddr,
			RESOURCES, (uintptr_t)RESOURCES - SFBaseAddr,
			OSNAP, (uintptr_t)OSNAP - SFBaseAddr,
			GSNAP, (uintptr_t)GSNAP - SFBaseAddr,
			GSNAP2, (uintptr_t)GSNAP2 - SFBaseAddr,
			GSNAP3, (uintptr_t)GSNAP3 - SFBaseAddr,
			WSModeFinder, WSModeAddr - SFBaseAddr,
			CSAVES, (uintptr_t)CSAVES - SFBaseAddr,
			ParseParams, (uintptr_t)ParseParams_Native - SFBaseAddr,
			gConsoleFinder, gConsoleAddr - SFBaseAddr,
			gConsole_vPrintFinder, gConsole_vPrintAddr - SFBaseAddr,
			firstConsoleFinder, (uintptr_t)firstConsoleFinder - SFBaseAddr,
			firstScriptFinder, (uintptr_t)firstScriptFinder - SFBaseAddr);
	}


};



/*
------------------------------------------------------------------------------------------------------------------------
relocptr: A pointer to another object you know the type of
if (firstConsole.ptr) {
	ReadMemory(uintptr_t(firstConsole.ptr) + 0x04, &firstConsole.r32, sizeof(s32));
	firstConsole.scriptfunction = RelocPtr<Script::SCRIPT_FUNCTION>(firstConsole.r32);
} 
------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------
relocaddr: A directly referenced address where you know the type typedef(call) or type(object) located at that address.
if (ParseParams.ptr) {
	ParseParams.r32 = uintptr_t(ParseParams.ptr) - RelocationManager::s_baseAddr;
	ParseParams_Native = RelocAddr<_ParseParams_Native>(ParseParams.r32);
}
------------------------------------------------------------------------------------------------------------------------

//debug menus
//g_menuInterface->RegisterMenuMovieCreated(toolz::DebugMenuOpen);
static void DebugMenuOpen(IMenu * menu)
{
	const char* mName = menu->GetName() ? menu->GetName() : "null menu";
	const char* mPath = menu->GetRootPath() ? menu->GetRootPath() : "null rootpath";
	pirlog("%s %s %p", mName, mPath, (uintptr_t)menu);
}
*/