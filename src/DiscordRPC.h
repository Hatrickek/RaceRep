#pragma once
#include "discordrpc/include/discord_register.h"
#include "discordrpc/include/discord_rpc.h"
#include <Windows.h>
#include <string>
namespace DiscordRPC
{
	void Initialize();
	void Update();
	//void Shutdown();
	//void ClearPresence();
};

#define DISCORD_APP_ID "967938935034634301"
