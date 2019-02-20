#include "NotificationManager.h"

#include <cstdio>  // snprintf
#include <cstring>  // memeset
#include <ctime>  // time
#include <string>  // string, to_string

#include "Settings.h"  // Settings


NotificationManager* NotificationManager::GetSingleton()
{
	if (!_singleton) {
		_singleton = new NotificationManager();
	}
	return _singleton;
}


void NotificationManager::Free()
{
	delete _singleton;
	_singleton = 0;
}


std::string NotificationManager::BuildNotification(RE::TESQuest* a_quest)
{
	std::string msg = "$WHQA_Msg";
	if (Settings::useRandomMessages) {
		std::snprintf(_buf, sizeof(_buf), "%02i", (_rng() % Settings::totalMessageCount));
		msg += _buf;
	} else {
		msg += "00";
	}
	msg += "{";

	bool foundQuestName = false;
	if (!a_quest->name.empty()) {
		msg += a_quest->name.c_str();
		foundQuestName = true;
	} else if (!a_quest->editorID.empty()) {
		msg += a_quest->editorID.c_str();
		foundQuestName = true;
	}

	if (Settings::printQuestFormID) {
		if (foundQuestName) {
			msg += " ";
		}
		std::snprintf(_buf, sizeof(_buf), "[0x%08X]", a_quest->formID);
		msg += _buf;
	}

	msg += "}";
	return msg;
}


NotificationManager::NotificationManager() :
	_rng(std::time(0))
{
	std::memset(_buf, 0, sizeof(_buf));
}


NotificationManager::~NotificationManager()
{}


NotificationManager* NotificationManager::_singleton = 0;
