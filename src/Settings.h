#pragma once

class Settings
{
public:
	[[nodiscard]] static Settings* GetSingleton()
	{
		static Settings singleton;
		return std::addressof(singleton);
	}

	void Load()
	{
		constexpr auto path = L"Data/SKSE/Plugins/WhoseQuestIsItAnyway.ini";

		CSimpleIniA ini;
		ini.SetUnicode();

		ini.LoadFile(path);

		detail::get_value(ini, printQuestFormID, "Settings", "printQuestFormID", ";Set to `true` to append the form ID of the quest to the quest's name.");
		detail::get_value(ini, useRandomMessages, "Settings", "useRandomMessages", ";Set to `true` to randomly select a message to print as a notification. `false` defaults to message `00`.");
		detail::get_value(ini, totalMessageCount, "Settings", "totalMessageCount", ";Indicates the total number of messages in the random message pool. Max `99`.");

		
		ini.SaveFile(path);
	}

	bool printQuestFormID{ false };
	bool useRandomMessages{ false };
	std::int32_t totalMessageCount{ 5 };


private:
	struct detail
	{
		static void get_value(CSimpleIniA& a_ini, bool& a_value, const char* a_section, const char* a_key, const char* a_comment)
		{
			a_value = a_ini.GetBoolValue(a_section, a_key, a_value);
			a_ini.SetBoolValue(a_section, a_key, a_value, a_comment);
		};

		static void get_value(CSimpleIniA& a_ini, std::int32_t& a_value, const char* a_section, const char* a_key, const char* a_comment)
		{
			try {
				a_value = string::lexical_cast<std::int32_t>(a_ini.GetValue(a_section, a_key, std::to_string(a_value).c_str()));
				a_ini.SetValue(a_section, a_key, std::to_string(a_value).c_str(), a_comment);
			} catch (...) {
			}
		}
	};
};
