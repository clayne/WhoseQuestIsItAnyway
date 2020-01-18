#include "NotificationManager.h"

#include <cstdio>
#include <ctime>
#include <string>

#include "Settings.h"


NotificationManager* NotificationManager::GetSingleton()
{
	static NotificationManager singleton;
	return &singleton;
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

	auto foundQuestName = false;
	if (!a_quest->fullName.empty()) {
		msg += a_quest->fullName.c_str();
		foundQuestName = true;
	} else if (!a_quest->formEditorID.empty()) {
		msg += a_quest->formEditorID.c_str();
		foundQuestName = true;
	}

	if (Settings::printQuestFormID) {
		if (foundQuestName) {
			msg += " ";
		}
		std::snprintf(_buf, sizeof(_buf), "[0x%08X]", a_quest->GetFormID());
		msg += _buf;
	}

	msg += "}";
	return msg;
}


NotificationManager::NotificationManager() :
	_rng(std::random_device()()),
	_buf{}
{}
