#include "skse64_common/BranchTrampoline.h"  // g_branchTrampoline, g_localTrampoline
#include "skse64_common/skse_version.h"  // RUNTIME_VERSION

#include "Hooks.h"  // InstallHooks
#include "Settings.h"  // Settings
#include "version.h"  // VERSION_VERSTRING, VERSION_MAJOR

#include "SKSE/API.h"


extern "C" {
	bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
	{
		SKSE::Logger::OpenRelative(FOLDERID_Documents, L"\\My Games\\Skyrim Special Edition\\SKSE\\WhoseQuestIsItAnyway.log");
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::UseLogStamp(true);

		_MESSAGE("WhoseQuestIsItAnyway v%s", WHQA_VERSION_VERSTRING);

		a_info->infoVersion = SKSE::PluginInfo::kVersion;
		a_info->name = "WhoseQuestIsItAnyway";
		a_info->version = WHQA_VERSION_MAJOR;

		if (a_skse->IsEditor()) {
			_FATALERROR("Loaded in editor, marking as incompatible!\n");
			return false;
		}

		switch (a_skse->RuntimeVersion()) {
		case RUNTIME_VERSION_1_5_73:
		case RUNTIME_VERSION_1_5_80:
			break;
		default:
			_FATALERROR("Unsupported runtime version %08X!\n", a_skse->RuntimeVersion());
			return false;
		}

		return true;
	}


	bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
	{
		_MESSAGE("WhoseQuestIsItAnyway loaded");

		if (!SKSE::Init(a_skse)) {
			return false;
		}

		if (Settings::loadSettings()) {
			_MESSAGE("Settings loaded");
		} else {
			_FATALERROR("Settings failed to load!\n");
			return false;
		}

		if (g_branchTrampoline.Create(1024 * 8)) {
			_MESSAGE("Branch trampoline creation successful");
		} else {
			_FATALERROR("Branch trampoline creation failed!\n");
			return false;
		}

		if (g_localTrampoline.Create(1024 * 8)) {
			_MESSAGE("Local trampoline creation successful");
		} else {
			_FATALERROR("Local trampoline creation failed!\n");
			return false;
		}

		InstallHooks();
		_MESSAGE("Hooks installed");

		return true;
	}
};
