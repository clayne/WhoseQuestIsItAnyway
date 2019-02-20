#include "Settings.h"


bool Settings::loadSettings(bool a_dumpParse)
{
	setFileName(FILE_NAME);
	return Json2Settings::Settings::loadSettings(a_dumpParse);
}


bSetting	Settings::printQuestFormID("printQuestFormID", false, false);
bSetting	Settings::useRandomMessages("useRandomMessages", false, false);
iSetting	Settings::totalMessageCount("totalMessageCount", false, false);
