#pragma once

namespace Globals
{
	inline HWND RobloxWindow;
	inline bool Watermark;
	inline ID3D11Device* g_pd3dDevice = nullptr;
	inline ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
	namespace Auth
	{
		inline std::string Username = "0";
		inline std::string Password = "0";
	}
	inline bool Vsync = false;
	namespace Esp
	{
		inline bool Enabled = false;
		inline bool Box = false;
		inline bool BoxGlow = false;
		inline bool Name = false;
		inline bool UseDisplayName = false;
		inline bool Distance = false;
		inline bool Tool = false;
		inline bool Chams = false;
		inline bool Avatar = false;
		inline int IconSize = 32;
		inline int OffsetOffBoxAvatar = 11;
		inline int ChamsType = 0;
		inline bool ChamsGlow = false;
		inline bool ChamsOutlines = false;
		inline bool HealthBar = false;
		inline bool HealthNumber = false;
		inline bool HealthBarBackGround = false;
		inline bool HealthBarGlow = false;
		inline int HealthBarColorType = 0;
		inline bool Team = false;
		inline bool Leaderstats = false;
		inline bool Region = false;
		inline bool LocalPlayerESP = false;
		inline int Font = 0;
		inline bool MaxDistance = false;
		inline int MaxDistanceValue = 1000;
		inline bool KeybindList = false;
		inline int ChamsOutlineThickness = 1.0f;
		inline bool InheritBodyColorChams = false;
		inline bool FilterUnknownChamsParts = false;
		inline bool Tracers = false;
		inline bool Skeleton = false;
		inline int TracersPosition = 0;
		inline int ChamsMaterial = 0;
		namespace Colors
		{
			inline float Skeleton[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float Tracers[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float Box[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float BoxGlow[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float Name[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float Distance[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float Tool[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float Chams[4] = { 1.0f,1.0f,1.0f,0.5f };
			inline float ChamsGlow[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float ChamsOutlines[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float HealthBarGlow[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float HealthBar[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float HealthNumber[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float Region[4] = { 1.0f,1.0f,1.0f,1.0f };
			inline float LeaderStats[4] = { 1.0f,1.0f,1.0f,1.0f };
		}
	}
	namespace Misc
	{
		inline bool TeamCheck = false;
		inline bool StreamProof = false;

		inline bool Desync = false;
		inline CKeybind DesyncBind{ "Desync" };

	}
	namespace Aimbot
	{
		namespace Rage
		{
			inline bool SpamTP = false;
			inline bool Orbit = false;
		}
		inline bool Enabled = false;
		inline CKeybind AimbotKey{ "Aimbot" };
		inline int Hitbox = 0;
		inline int AimType = 0;
		inline bool Prediction = false;
		inline float PredictionX = 1;
		inline float PredictionY = 1;
		inline bool EnableFOV = false;
		inline bool DrawFOV = false;
		inline float FovColor[3] = { 1.0f,1.0f,1.0f };
		namespace FovSettings
		{
			inline bool FillFov = false;
			inline float FillColor[4] = { 1.0f,1.0f,1.0f, 0.3f }; 
			
		}
		inline int FOV = 60;
		inline bool Smoothning = false;
		inline float SmoothingValue = 0.1f;
		namespace Target {
			inline bool HaveTarget = false;
			inline std::string Name = "";
			inline std::string DisplayName = "";
			inline std::string Tool = "";
			inline std::string Distance = "";
			inline int Health = 0;
			inline int MaxHealth = 100;
		}
		namespace Hits
		{
			inline bool HitSounds = false;
			inline int HitSound = 0;
			inline bool HitTracers = false;
			inline bool HitMarkers = false;
			inline float HitTracersColor[3] = { 1.0f,1.0f,1.0f };
			inline float HitMarkersColor[3] = { 1.0f,1.0f,1.0f };
		}
		inline bool SilentCrosshair = false;
		inline bool SilentCrosshairTracer = false;
		inline float SilentCrosshairTracerColor[3] = { 1.0f,1.0f,1.0f };
		inline bool IsThereATarget = false;
	}
	namespace TriggerBot
	{
		inline bool Enabled = false;
		inline CKeybind Key{ "TriggerBot" };
		inline int Delay = 0;
		inline int Threshold = 0;
		inline bool ToolCheck = false;
	}
	namespace World
	{
		inline float AmbientColor[3] = { 1.0f,1.0f,1.0f };
		inline float OutdoorAmbientColor[3] = { 1.0f,1.0f,1.0f };
		inline bool LocalPlayerForceField = false;
		namespace Waypoints
		{
			inline bool WaypointName = false;
			inline bool WaypointDistance = false;
			namespace Colors
			{
				inline float WaypointName[4] = { 1.0f,1.0f,1.0f,1.0f };
				inline float WaypointDistance[4] = { 1.0f,1.0f,1.0f,1.0f };
			}
		}
		inline bool CameraFOV = false;
		inline int CameraFOVValue = 90;
		inline bool Zoom = false;
		inline CKeybind ZoomKey{ "Zoom" };
		inline int ZoomValue = 30;
		namespace Movement
		{
			inline bool InfiniteJump = false;
			namespace  InfiniteJumpNameSpace
			{
				inline bool CustomJumpPower = false;
				inline float CustomJumpPowerValue = 50.0f;
			}
			inline bool Fly = false;
			inline CKeybind FlyKey{ "Fly" };
			inline float FlySpeed = 1.0f;

			inline bool Cframe = false;
			inline CKeybind CframeKey{ "CFrame" };
			inline float CframeSpeed = 1.0f;

			inline bool Freecam = false;
			inline CKeybind FreecamKey{ "Freecam" };
			inline float FreecamSpeed = 0.01f;

			inline bool ThirdPerson = false;
			inline CKeybind ThirdPersonKey{ "ThirdPerson" };
			inline float ThirdPersonDistance = 2.5f;

			inline bool Spinbot = false;
			inline int SpinbotSpeed = 5;
			inline bool UpsideDown = false;
			
			
		}
	}
}