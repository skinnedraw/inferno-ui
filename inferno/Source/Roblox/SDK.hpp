#pragma once
#include "../Includes.hpp"
namespace SDK
{
	inline void Scan()
	{
		Logging::Log(Logging::LogType::INFO, "Scanning... Roblox Base -> 0x%llX", SDK::RobloxBase);
		Globals::DataModel = SDK::FetchDatamodel(); 
		Globals::VisualEngine = SDK::FetchVisualEngine();
		

		Globals::Players = (SDK::Players)Globals::DataModel.FindFirstChildOfClass("Players");
		Globals::Workspace = Globals::DataModel.FindFirstChildOfClass("Workspace");
		Globals::LocalPlayer = Globals::Players.LocalPlayer();
		Globals::MouseService = (SDK::MouseService)Globals::DataModel.FindFirstChildOfClass("MouseService");
		Globals::Lighting = Globals::DataModel.FindFirstChildOfClass("Lighting");
		Globals::GameID = Globals::DataModel.GameID();
		
		Logging::Log(Logging::LogType::INFO, "Scan finished. Datamodel -> 0x%llX", Globals::DataModel.Address);
		Logging::Log(Logging::LogType::INFO, "LocalPlayer -> 0x%llX", Globals::LocalPlayer.Address);
		Logging::Log(Logging::LogType::INFO, "MouseService -> 0x%llX", Globals::MouseService.Address);
		Logging::Log(Logging::LogType::INFO, "Players -> 0x%llX", Globals::Players.Address);
		Logging::Log(Logging::LogType::INFO, "Workspace -> 0x%llX", Globals::Workspace.Address);
		Logging::Log(Logging::LogType::INFO, "VisualEngine -> 0x%llX", Globals::VisualEngine.Address);
		Logging::Log(Logging::LogType::INFO, "Lighting -> 0x%llX", Globals::Lighting.Address);
		Logging::Log(Logging::LogType::INFO, "GameID -> %i", Globals::GameID);

	}
	inline void InitializeAutoRescan()
	{
		while (true)
		{
			if (Globals::DataModel.Address != SDK::FetchDatamodel().Address)
			{
				Logging::Log(Logging::LogType::INFO, "Scene change detected, rescanning.");
				SDK::Scan();
			}
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	}
	inline void SDKCacheUpdate()
	{
		while (true)
		{
			try
			{
				SDK::VisualEngineCache::ViewMatrix = Globals::VisualEngine.ViewMatrix();
			}
			catch (...)
			{
				Logging::Log(Logging::LogType::ERR, "Caught crash at SDKCacheUpdate");
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
	inline void Initialize()
	{
		SDK::Scan();
		std::thread(SDK::InitializeAutoRescan).detach();
		std::thread(SDK::SDKCacheUpdate).detach();
	}
}