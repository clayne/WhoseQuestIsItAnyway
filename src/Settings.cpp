#include "Settings.h"


bool Settings::loadSettings(bool a_dumpParse)
{
	setFileName(FILE_NAME);
	return Json2Settings::Settings::loadSettings(a_dumpParse);
}


decltype(Settings::printQuestFormID)	Settings::printQuestFormID("printQuestFormID", false, false);
decltype(Settings::useRandomMessages)	Settings::useRandomMessages("useRandomMessages", false, false);
decltype(Settings::totalMessageCount)	Settings::totalMessageCount("totalMessageCount", false, false);
