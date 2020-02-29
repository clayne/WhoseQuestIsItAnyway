#pragma once

#include "Json2Settings.h"


class Settings
{
public:
	using bSetting = Json2Settings::bSetting;
	using iSetting = Json2Settings::iSetting;


	Settings() = delete;

	static bool	loadSettings(bool a_dumpParse = false);


	static bSetting printQuestFormID;
	static bSetting useRandomMessages;
	static iSetting totalMessageCount;

private:
	static constexpr char FILE_NAME[] = "Data\\SKSE\\Plugins\\WhoseQuestIsItAnyway.json";
};
