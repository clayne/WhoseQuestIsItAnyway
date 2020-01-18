#pragma once

#include <random>
#include <string>

#include "RE/Skyrim.h"


class NotificationManager
{
public:
	static NotificationManager* GetSingleton();

	std::string BuildNotification(RE::TESQuest* a_quest);

protected:
	NotificationManager();
	NotificationManager(const NotificationManager&) = delete;
	NotificationManager(NotificationManager&&) = delete;
	~NotificationManager() = default;

	NotificationManager& operator=(const NotificationManager&) = delete;
	NotificationManager& operator=(NotificationManager&&) = delete;


	std::mt19937 _rng;
	char _buf[1000];
};
