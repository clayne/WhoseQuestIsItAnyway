#include "Settings.h"


bool Settings::loadSettings(bool a_dumpParse)
{
	return Json2Settings::Settings::loadSettings(FILE_NAME, false, a_dumpParse);
}


decltype(Settings::printQuestFormID)	Settings::printQuestFormID("printQuestFormID", false, false);
decltype(Settings::useRandomMessages)	Settings::useRandomMessages("useRandomMessages", false, false);
decltype(Settings::totalMessageCount)	Settings::totalMessageCount("totalMessageCount", false, false);
