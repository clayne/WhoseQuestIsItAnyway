#include "Settings.h"


bool Settings::loadSettings(bool a_dumpParse)
{
	auto [log, success] = Json2Settings::load_settings(FILE_NAME, a_dumpParse);
	if (!log.empty()) {
		_ERROR("%s", log.c_str());
	}
	return success;
}


decltype(Settings::printQuestFormID) Settings::printQuestFormID("printQuestFormID", false);
decltype(Settings::useRandomMessages) Settings::useRandomMessages("useRandomMessages", false);
decltype(Settings::totalMessageCount) Settings::totalMessageCount("totalMessageCount", 5);
