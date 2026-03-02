#define NOMINMAX
#include "Aimbot.hpp"
#include "../../Includes.hpp"
#include "hitsounds.h"
struct AimbotPlayer
{
	SDK::Cache::Player Player;
	SDK::Cache::Bone BodyPart;
};

AimbotPlayer SavedPlayer;

AimbotPlayer GetClosest()
{
	try
	{
		POINT cursor_point;
		if (!GetCursorPos(&cursor_point) || !Globals::RobloxWindow)
			return {};

		if (!ScreenToClient(Globals::RobloxWindow, &cursor_point))
			return {};

		SDK::Vector2 cursor = {
			static_cast<float>(cursor_point.x),
			static_cast<float>(cursor_point.y)
		};

		float min_dist = 9e9;
		AimbotPlayer Target;

		//if (!SDK::Cache::LocalPlayer.PlayerObjectAddress || !SDK::Cache::LocalPlayer.HumanoidRootPart.Object.Address)
		//	return {};

		Target.BodyPart = SDK::Cache::LocalPlayer.HumanoidRootPart;
		SDK::Cache::PlayersMutex.lock();
		for (SDK::Cache::Player& Player : SDK::Cache::Players)
		{
			if (!Player.PlayerObjectAddress || !Player.HumanoidRootPart.Object.Address)
				continue;

			if (Player.PlayerObject.Address == SDK::Cache::LocalPlayer.PlayerObject.Address)
				continue;

			

			float Health = 0;
			try {
				Health = Player.Health;
			}
			catch (...) {
				continue;
			}

			if (Health <= 0)
				continue;

			SDK::Vector2 screenPos = Player.HumanoidRootPart.ScreenPosition;
			if (screenPos.x == -1 || screenPos.y == -1)
				continue;

			float Distance = screenPos.distance(cursor);

			if (std::isnan(Distance) || std::isinf(Distance))
				continue;

			if (Globals::Aimbot::EnableFOV && Distance > Globals::Aimbot::FOV)
				continue;

			if (Distance < min_dist)
			{
				min_dist = Distance;

				if (Globals::Aimbot::Hitbox == 0 && Player.Head.Object.Address)
					Target.BodyPart = Player.Head;
				else if (Globals::Aimbot::Hitbox == 1 && Player.HumanoidRootPart.Object.Address)
					Target.BodyPart = Player.HumanoidRootPart;
				else
					continue;

				Target.Player = Player;
			}
		}
		SDK::Cache::PlayersMutex.unlock();
		if (!Target.BodyPart.Object.Address || Target.BodyPart.Object.Address == SDK::Cache::LocalPlayer.HumanoidRootPart.Object.Address)
		{
			SavedPlayer.Player.PlayerObject.Address = 0;
			return {};
		}

		return Target;
	}
	catch (...)
	{
		SavedPlayer.Player.PlayerObject.Address = 0;
		return {};
	}
}

namespace MatrixUtils
{
	static SDK::Vector3 normalize(const SDK::Vector3& vec) {
		try {
			float length = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
			if (length > 0.0001f && !std::isnan(length) && !std::isinf(length)) {
				return { vec.x / length, vec.y / length, vec.z / length };
			}
		}
		catch (...) {}
		return { 0, 0, 1 };
	}

	static SDK::Vector3 crossProduct(const SDK::Vector3& vec1, const SDK::Vector3& vec2) {
		try {
			SDK::Vector3 result = {
				vec1.y * vec2.z - vec1.z * vec2.y,
				vec1.z * vec2.x - vec1.x * vec2.z,
				vec1.x * vec2.y - vec1.y * vec2.x
			};

			if (std::isnan(result.x) || std::isnan(result.y) || std::isnan(result.z) ||
				std::isinf(result.x) || std::isinf(result.y) || std::isinf(result.z)) {
				return { 0, 1, 0 };
			}

			return result;
		}
		catch (...) {
			return { 0, 1, 0 };
		}
	}

	static SDK::Matrix3 lerp_rmatrix3(const SDK::Matrix3& a, const SDK::Matrix3& b, float t)
	{
		try {
			SDK::Matrix3 result{};
			t = std::max(0.0f, std::min(1.0f, t));

			for (int i = 0; i < 9; ++i) {
				float lerped = a.data[i] + (b.data[i] - a.data[i]) * t;
				if (std::isnan(lerped) || std::isinf(lerped)) {
					result.data[i] = a.data[i];
				}
				else {
					result.data[i] = lerped;
				}
			}
			return result;
		}
		catch (...) {
			return a;
		}
	}

	static SDK::Matrix3 LookAt(const SDK::Vector3& cameraPosition, const SDK::Vector3& targetPosition) {
		try {
			SDK::Vector3 direction = {
				targetPosition.x - cameraPosition.x,
				targetPosition.y - cameraPosition.y,
				targetPosition.z - cameraPosition.z
			};

			if (std::isnan(direction.x) || std::isnan(direction.y) || std::isnan(direction.z) ||
				std::isinf(direction.x) || std::isinf(direction.y) || std::isinf(direction.z)) {
				return SDK::Matrix3{};
			}

			SDK::Vector3 forward = normalize(direction);
			SDK::Vector3 right = normalize(crossProduct({ 0, 1, 0 }, forward));
			SDK::Vector3 up = crossProduct(forward, right);

			SDK::Matrix3 lookAtMatrix{};
			lookAtMatrix.data[0] = -right.x;   lookAtMatrix.data[1] = up.x;   lookAtMatrix.data[2] = -forward.x;
			lookAtMatrix.data[3] = right.y;    lookAtMatrix.data[4] = up.y;   lookAtMatrix.data[5] = -forward.y;
			lookAtMatrix.data[6] = -right.z;   lookAtMatrix.data[7] = up.z;   lookAtMatrix.data[8] = -forward.z;

			for (int i = 0; i < 9; i++) {
				if (std::isnan(lookAtMatrix.data[i]) || std::isinf(lookAtMatrix.data[i])) {
					return SDK::Matrix3{};
				}
			}

			return lookAtMatrix;
		}
		catch (...) {
			return SDK::Matrix3{};
		}
	}
}
bool IsMouseInBox(ImVec2 mousePos, ImVec2 boxMin, ImVec2 boxMax) {
	return (mousePos.x >= boxMin.x && mousePos.x <= boxMax.x &&
		mousePos.y >= boxMin.y && mousePos.y <= boxMax.y);
}
void TriggerBot()
{
	bool wasTargeting = false;
	while (true)
	{
		Globals::TriggerBot::Key.update();
		if (Globals::TriggerBot::Enabled && Globals::TriggerBot::Key.enabled)
		{
			if (Globals::TriggerBot::ToolCheck && SDK::Cache::LocalPlayer.Weapon.empty()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}
			POINT cursor_point;
			GetCursorPos(&cursor_point);
			ScreenToClient(Globals::RobloxWindow, &cursor_point);
			ImVec2 MousePos = {
					static_cast<float>(cursor_point.x),
					static_cast<float>(cursor_point.y)
			};

			bool isTargeting = false;
			SDK::Cache::PlayersMutex.lock();
			for (SDK::Cache::Player Player : SDK::Cache::Players)
			{
				if (!Player.PlayerObjectAddress || Player.Health <= 0)
					continue;
				if (Player.PlayerObject.Address == SDK::Cache::LocalPlayer.PlayerObject.Address)
					continue;
				if (Player.BoxMin.x == 0 && Player.BoxMin.y == 0 && Player.BoxMax.x == 0 && Player.BoxMax.y == 0)
					continue;

				if (IsMouseInBox(MousePos, Player.BoxMin, Player.BoxMax))
				{
					isTargeting = true;
					break;
				}
			}
			SDK::Cache::PlayersMutex.unlock();
			if (isTargeting && !wasTargeting)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(Globals::TriggerBot::Delay));
				INPUT inputs[2] = {};
				inputs[0].type = INPUT_MOUSE;
				inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				inputs[1].type = INPUT_MOUSE;
				inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
				SendInput(2, inputs, sizeof(INPUT));
			}

			wasTargeting = isTargeting;
			std::this_thread::sleep_for(std::chrono::milliseconds(Globals::TriggerBot::Threshold + 1));
		}
		else
		{
			wasTargeting = false;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}
void HitSounds()
{
	auto lastClickTime = std::chrono::steady_clock::now();
	while (true)
	{
		
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		{
			lastClickTime = std::chrono::steady_clock::now();
		}
		if (duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastClickTime).count() > 50)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}
		AimbotPlayer Player = GetClosest();
		
		static float previousHealth = -1.0f;
		
		auto name = Player.Player.PlayerName;
		float currentHealth = Player.Player.Health;
		
		int healthPercentage = static_cast<int>(currentHealth);

		if (previousHealth < 0.0f) {
			previousHealth = currentHealth;
		}

		if (currentHealth < previousHealth && (previousHealth - currentHealth) > 5) {
			float damageDealt = previousHealth - currentHealth;
			//std::string hitNotification = name + (" has been shot for ") + std::to_string(static_cast<int>(damageDealt)) + (" health");
			//std::cout << "[atlanta.vip] " << hitNotification << std::endl;
			bool playerDied = (currentHealth <= 0);
			if (Globals::Aimbot::Hits::HitTracers || Globals::Aimbot::Hits::HitMarkers)
			{
				Hacks::Esp::HitMarker3D Position1 = { SDK::Cache::LocalPlayer.Head.WorldPosition.x,SDK::Cache::LocalPlayer.Head.WorldPosition.y,SDK::Cache::LocalPlayer.Head.WorldPosition.z };
				Hacks::Esp::HitMarker3D Pos2 = { Player.Player.Head.WorldPosition.x,Player.Player.Head.WorldPosition.y,Player.Player.Head.WorldPosition.z };
				Hacks::Esp::InsertHitMarker(Position1, Pos2);
			}
			if (Globals::Aimbot::Hits::HitSounds)
			{
				if (Globals::Aimbot::Hits::HitSound == 0) {
					PlaySoundA(reinterpret_cast<char*>(amongus), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 1) {
					PlaySoundA(reinterpret_cast<char*>(skeet_sound), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 2) {
					PlaySoundA(reinterpret_cast<char*>(beep), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 3) {
					PlaySoundA(reinterpret_cast<char*>(bonk), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 4) {
					PlaySoundA(reinterpret_cast<char*>(bubble), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 5) {
					PlaySoundA(reinterpret_cast<char*>(cod), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 6) {
					PlaySoundA(reinterpret_cast<char*>(csgo), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 7) {
					PlaySoundA(reinterpret_cast<char*>(fairy), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 8) {
					PlaySoundA(reinterpret_cast<char*>(fatality), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 9) {
					PlaySoundA(reinterpret_cast<char*>(osu), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 10) {
					PlaySoundA(reinterpret_cast<char*>(rust), NULL, SND_ASYNC | SND_MEMORY);
				}
				else if (Globals::Aimbot::Hits::HitSound == 11) {
					PlaySoundA(reinterpret_cast<char*>(bameware), NULL, SND_ASYNC | SND_MEMORY);
				}
			}
		}

		previousHealth = currentHealth;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
void Hacks::Aimbot::Setup()
{
	std::thread(TriggerBot).detach();
	std::thread(HitSounds).detach();
	while (true)
	{
		Globals::Aimbot::AimbotKey.update();
		
		if (Globals::Aimbot::Enabled && Globals::Aimbot::AimbotKey.enabled)
		{
			POINT cursor_point;
			GetCursorPos(&cursor_point);
			ScreenToClient(Globals::RobloxWindow, &cursor_point);
			
			if (SavedPlayer.Player.PlayerObject.Address == 0 || !SavedPlayer.BodyPart.Object.Address)
			{
				SavedPlayer = GetClosest();
				if (!SavedPlayer.BodyPart.Object.Address) {
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					continue;
				}
			}

			if (!Globals::Workspace.Address) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			SDK::Instance CameraInstance = Globals::Workspace.FindFirstChildOfClass("Camera");
			if (!CameraInstance.Address) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			SDK::Camera Camera = (SDK::Camera)CameraInstance;

			SDK::Vector3 PartPosition;
			try {
				PartPosition = SavedPlayer.BodyPart.Object.Primitive().Position();
			}
			catch (...) {
				SavedPlayer.Player.PlayerObject.Address = 0;
				Globals::Aimbot::Target::HaveTarget = false;
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}
			Globals::Aimbot::Target::HaveTarget = true;
			Globals::Aimbot::Target::Name = SavedPlayer.Player.PlayerName;
			Globals::Aimbot::Target::DisplayName = SavedPlayer.Player.DisplayName;
			Globals::Aimbot::Target::Tool = SavedPlayer.Player.Weapon;
			Globals::Aimbot::Target::Distance = std::to_string(SavedPlayer.Player.Distance) + "m";
			Globals::Aimbot::Target::Health = SavedPlayer.Player.Health;
			Globals::Aimbot::Target::MaxHealth = SavedPlayer.Player.MaxHealth;


			if (Globals::Aimbot::Prediction)
			{
				SDK::Vector3 Velocity = SavedPlayer.BodyPart.Object.Primitive().Velocity();
				Velocity = SDK::Vector3(Velocity.x / (15.0f - Globals::Aimbot::PredictionX), Velocity.y / (15.0f - Globals::Aimbot::PredictionY),0);
				PartPosition = PartPosition + Velocity;
			}
			SDK::Vector2 PartPosition2D = SDK::WorldToScreen(PartPosition);
			if (PartPosition2D.x == -1 || PartPosition2D.y == -1)
			{
				//SavedPlayer.Player.PlayerObject.Address = 0;
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}

			if (SavedPlayer.BodyPart.Object.Address == SDK::Cache::LocalPlayer.HumanoidRootPart.Object.Address) {
				SavedPlayer.Player.PlayerObject.Address = 0;
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}
			SDK::Vector2 relative = { 0, 0 };
			relative.x = (PartPosition2D.x - cursor_point.x) * 0.1f / 0.1f;
			relative.y = (PartPosition2D.y - cursor_point.y) * 0.1f / 0.1f;
			if (Globals::Aimbot::Smoothning)
			{

				relative.x = (PartPosition2D.x - cursor_point.x) * 0.1f / (Globals::Aimbot::SmoothingValue);
				relative.y = (PartPosition2D.y - cursor_point.y) * 0.1f / (Globals::Aimbot::SmoothingValue);
			}
			
			if (Globals::Aimbot::AimType == 0)
			{
				if (relative.x != -1 && relative.y != -1) {
					INPUT input{};
					input.mi.time = 0;
					input.type = INPUT_MOUSE;
					input.mi.mouseData = 0;
					input.mi.dx = relative.x;
					input.mi.dy = relative.y;
					input.mi.dwFlags = MOUSEEVENTF_MOVE;
					SendInput(1, &input, sizeof(input));
					std::this_thread::sleep_for(std::chrono::milliseconds(2));
				}
			}
			if (Globals::Aimbot::AimType == 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				SDK::Matrix3 CameraRotation;
				SDK::Vector3 CameraPosition;

				try {
					CameraRotation = Camera.CameraRotation();
					CameraPosition = Camera.CameraPosition();
				}
				catch (...) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}

				SDK::Matrix3 Matrix = MatrixUtils::LookAt(CameraPosition, PartPosition);
				float lerpFactor = 1.0f; 
				if (Globals::Aimbot::Smoothning) {
					
					lerpFactor = 1.0f - Globals::Aimbot::SmoothingValue;
					Matrix = MatrixUtils::lerp_rmatrix3(CameraRotation, Matrix, lerpFactor);
				}

				 

				try {
					Camera.SetCameraRotation(Matrix);
				}
				catch (...) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}
				
			}
			if (Globals::Aimbot::AimType == 2)
			{
				Globals::Aimbot::IsThereATarget = true;
				Globals::Aimbot::SilentMousePosition = PartPosition2D;
				Globals::MouseService.SilentAim(
					PartPosition2D,
					{
					static_cast<float>(cursor_point.x),
					static_cast<float>(cursor_point.y)
					}
				);
			}
		}
		else
		{
			SavedPlayer.Player.PlayerObject.Address = 0;
			Globals::Aimbot::IsThereATarget = false;
			Globals::Aimbot::Target::HaveTarget = false;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

	}
}
