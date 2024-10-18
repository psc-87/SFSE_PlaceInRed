#include "main.h"

static PlaceInRed pir;

// executed every time the console command runs
static bool ExecuteConsole(const SCRIPT_PARAMETER* paramInfo, const char* someString, TESObjectREFR* thisObj, TESObjectREFR* containingObj, Script* script, ScriptLocals* locals, float* result, u32* opcodeOffsetPtr)
{
	if (pir.ParseParams_Native)
	{
		char p1[1024] = {};
		char p2[1024] = {};
		bool bParseSuccess = pir.ParseParams_Native(paramInfo, someString, opcodeOffsetPtr, thisObj, containingObj, script, locals, &p1, &p2);

		if (bParseSuccess && p1[0])
		{
			pir.RunConsoleFunction(p1);
			return true;
		}
	}

	return false;
}

// sfse message interface handler
static void MessageHandler(SFSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{

	// case SFSEMessagingInterface::kMessage_PostLoad: 
	// break;
	// case SFSEMessagingInterface::kMessage_PostPostLoad: 
	// break;

	case SFSEMessagingInterface::kMessage_PostDataLoad:
		pir.PatchConsole("GameComment");

		break;

	// case SFSEMessagingInterface::kMessage_PostPostDataLoad:	break;

	default: break;
	}
}

// init sfse stuff and return false if anything fails
static bool InitSFSE(const SFSEInterface* sfse)
{
	// get a plugin handle
	pir.pluginHandle = sfse->GetPluginHandle();
	if (!pir.pluginHandle) {
		pirlog("failed to get a plugin handle!");
		return false;
	}

	// set messaging interface
	pir.g_messaging = (SFSEMessagingInterface*)sfse->QueryInterface(kInterface_Messaging);
	if (!pir.g_messaging) {
		pirlog("failed to set messaging interface!");
		return false;
	}

	// register listener
	if (pir.g_messaging->RegisterListener(pir.pluginHandle, "SFSE", MessageHandler) == false) {
		pirlog("failed to set g_messaging!");
		return false;
	}

	pir.g_menuInterface = (SFSEMenuInterface*)sfse->QueryInterface(kInterface_Menu);
	if (!pir.g_menuInterface) {
		pirlog("failed to set g_menuInterface!");
		return false;
	}

	return true;

}


extern "C"{

	// SFSEPlugin_Load
	__declspec(dllexport) bool SFSEPlugin_Load(const SFSEInterface* sfse)
	{
		
		pir.SetStarfieldBaseAddress();
		pir.CreateLogFile();


		// init SFSE stuff
		if (!InitSFSE(sfse)) {
			pirlog("Plugin load failed during InitSFSE!");
			return false;
		}
		
		pir.PluginInit();
		pir.LogMemory();

		// check pointers
		if (pir.bAllPatternsFound){

			pir.ReadPluginINISettings();
			pir.ReadOriginalGameINISettings();
			pir.end_tickcount = GetTickCount64() - pir.start_tickcount;
			pirlog("completed in %llums.", pir.end_tickcount);
			return true;

		} else {
			pirlog("plugin load failed! Couldn't find required memory patterns.");
			return false;
		}
	}

	// SFSEPlugin_Version
	__declspec(dllexport) SFSEPluginVersionData SFSEPlugin_Version =
	{
		SFSEPluginVersionData::kVersion,
		1, // version
		"Place in Red", //plugin name
		"psc87", // plugin author
		0,	// not address independent
		0,	// not structure independent
		{ RUNTIME_VERSION_1_14_70, 1
		},
		0,	// works with any version of the script extender
		0, 0,	// reserved
	};
};