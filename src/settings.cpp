#include "Settings.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include "launcher.h"
#include "overlays.h"

namespace sjson{
	bool checkJsonFile();
	void createJsonFile(); 
	
	using json = nlohmann::json;
	json j2 = {
		{"discordRpcOn", true},
		{"lowPowerOn", false},
		{"steeringOn", true},
		{"useDirectInput", false},
		{"dashSteeringSizeMult", 0.5},
		{"dashPreset", 0}
	};

	void Initialize()
	{
		if (!checkJsonFile()) createJsonFile();
		loadJsonFile();
	}
	void loadJsonFile()
	{
		std::ifstream i("config.json");
		j2 = json::parse(i);
		isDiscordRpcOn = j2["discordRpcOn"];
		lowPowerOn = j2["lowPowerOn"];
		steeringOn = j2["steeringOn"];
		dashSteeringSizeMult = j2["dashSteeringSizeMult"];
		dashPreset = j2["dashPreset"];
		useDirectInput = j2["useDirectInput"];
		i.close();
	}
	bool checkJsonFile()
	{
		std::ifstream file;
		file.open("config.json");
		if (file)
		{
			file.close();
			return true;
		}
		file.close();
		return false;
	}
	void createJsonFile()
	{
		std::ofstream file;
		file.open("config.json");
		file << j2.dump(1);
		file.close();
	}
	void save()
	{
		json j2 = {
			{"discordRpcOn", isDiscordRpcOn},
			{"lowPowerOn", lowPowerOn},
			{"steeringOn", steeringOn},
			{"useDirectInput", useDirectInput},
			{"dashSteeringSizeMult", dashSteeringSizeMult},
			{"dashPreset", dashPreset}
		};
		if (!checkJsonFile()) createJsonFile();
		std::ofstream file;
		file.open("config.json");
		file << j2.dump(1);
		file.close();
	}
}
