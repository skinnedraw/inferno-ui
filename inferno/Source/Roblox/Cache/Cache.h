#pragma once
#include "../../Includes.hpp"
namespace SDK::Cache
{
	void PlayerCache();
	void PlayerInfoUpdate();
	struct CachedConvexHull
	{
		ImVec2* Hull;
		int HullSize;
	};
	struct WayPoint
	{
		std::string Name;
		//SDK::Instance Instance;
		ImVec2 ScreenPosition;
		SDK::Vector3 WorldPosition;
	};
	struct Flag
	{
		ImColor Color;
		std::string Flag;
	};
	struct Bone {
		std::string Name;
		SDK::Instance Object;
		SDK::Vector3 WorldPosition;
		SDK::Vector2 ScreenPosition;
		SDK::Vector3 Size;
		SDK::Matrix3 Rotation;
		CachedConvexHull Hull;
		bool CacheSize = true;

	};
	struct BoneConnection
	{
		ImVec2 Bone1;
		ImVec2 Bone2;
	};
	struct Player
	{
		std::string PlayerName;
		std::string DisplayName;
		float Health;
		float MaxHealth;
		std::vector<SDK::Cache::Bone> Bones;
		SDK::Cache::Bone Head;
		SDK::Cache::Bone HumanoidRootPart;
		SDK::Instance CharacterObject;
		SDK::Player PlayerObject;
		uintptr_t PlayerObjectAddress;
		uintptr_t CharacterObjectAddress;
		SDK::Humanoid Humanoid;
		int RigType;
		int Distance;
		ImVec2 BoxMin;
		ImVec2 BoxMax;
		std::string Weapon;
		std::vector<SDK::Cache::Flag> Flags;
		uintptr_t Team;
		Clipper2Lib::PathsD HighlightSolution;
	};
	inline std::vector<SDK::Cache::Player> Players;
	inline SDK::Cache::Player LocalPlayer;
	inline std::vector<SDK::Cache::WayPoint> Waypoints;
	struct PlayerAvatar {
		ID3D11ShaderResourceView* Texture;
		std::string PlayerName;
	};
	inline std::mutex PlayersMutex;
	inline std::vector<PlayerAvatar> LoadedAvatars;
}