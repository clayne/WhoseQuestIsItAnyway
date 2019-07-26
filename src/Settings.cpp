#include "Settings.h"


bool Settings::loadSettings(bool a_dumpParse)
{
	return Json2Settings::Settings::loadSettings(FILE_NAME, a_dumpParse);
}


decltype(Settings::printQuestFormID)	Settings::printQuestFormID("printQuestFormID", false);
decltype(Settings::useRandomMessages)	Settings::useRandomMessages("useRandomMessages", false);
decltype(Settings::totalMessageCount)	Settings::totalMessageCount("totalMessageCount", false);
