#pragma once

#include <random>  // mt19937
#include <string>  // string

#include "RE/TESQuest.h"  // TESQuest


class NotificationManager
{
public:
	static NotificationManager*	GetSingleton();
	static void					Free();

	std::string					BuildNotification(RE::TESQuest* a_quest);

protected:
	NotificationManager();
	NotificationManager(const NotificationManager&) = delete;
	NotificationManager(NotificationManager&&) = delete;
	~NotificationManager();

	NotificationManager& operator=(const NotificationManager&) = delete;
	NotificationManager& operator=(NotificationManager&&) = delete;


	static NotificationManager*	_singleton;
	std::mt19937				_rng;
	char						_buf[1000];
};
