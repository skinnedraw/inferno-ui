#include "Misc.h"
#include "../../Includes.hpp"
#include <unordered_map>
#include <functional>
#include <chrono>
#define M_PI 3.14159265358979323846
SDK::Vector3 LookVector(const SDK::Matrix3& rotationMatrix) {
	return { rotationMatrix.data[2], rotationMatrix.data[2 + 3], rotationMatrix.data[2 + 6] };
}

SDK::Vector3 RightVector(const SDK::Matrix3& rotationMatrix) {
	return { rotationMatrix.data[0], rotationMatrix.data[0 + 3], rotationMatrix.data[0 + 6] };
}
void FlyHack()
{
	bool AirCheck = false;
	while (true)
	{
		Globals::World::Movement::FlyKey.update();
		if (Globals::World::Movement::Fly && Globals::World::Movement::FlyKey.enabled)
		{
			SDK::Cache::Bone HRP = SDK::Cache::LocalPlayer.HumanoidRootPart;
			if (!HRP.Object.Address)
				continue;
			SDK::Camera Camera = (SDK::Camera)Globals::Workspace.FindFirstChildOfClass("Camera");
			SDK::Matrix3 CamRot = Camera.CameraRotation();
			SDK::Vector3 LookVec = LookVector(CamRot);
			SDK::Vector3 RightVec = RightVector(CamRot);
			SDK::Vector3 Direction(0, 0, 0);
			if (GetAsyncKeyState('W') & 0x8000) {
				Direction = Direction - LookVec;
				AirCheck = true;
			}
			if (GetAsyncKeyState('S') & 0x8000) {
				Direction = Direction + LookVec;
				AirCheck = true;
			}
			if (GetAsyncKeyState('A') & 0x8000) {
				Direction = Direction - RightVec;
				AirCheck = true;
			}
			if (GetAsyncKeyState('D') & 0x8000) {
				Direction = Direction + RightVec;
				AirCheck = true;
			}

			if (Direction.magnitude() > 0) {
				Direction = Direction.normalize();
			}

			if (!AirCheck)
			{

				float loll = 0.0f;
				HRP.Object.Primitive().SetVelocity({ 0,loll,0 });



			}
			else
			{

				HRP.Object.Primitive().SetVelocity(Direction * Globals::World::Movement::FlySpeed * 10);


			}
			AirCheck = false;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

		}
		
	}
}

void CFrame()
{
	while (true)
	{
		Globals::World::Movement::CframeKey.update();
		if (Globals::World::Movement::Cframe && Globals::World::Movement::CframeKey.enabled)
		{
			SDK::Cache::Bone HRP = SDK::Cache::LocalPlayer.HumanoidRootPart;
			SDK::Humanoid Humanoid = SDK::Cache::LocalPlayer.Humanoid;
			if (!HRP.Object.Address)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}
			if (!Humanoid.Address)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}

			SDK::Camera Camera = (SDK::Camera)Globals::Workspace.FindFirstChildOfClass("Camera");
			SDK::Matrix3 CamRot = Camera.CameraRotation();
			SDK::Vector3 LookVec = LookVector(CamRot);
			SDK::Vector3 RightVec = RightVector(CamRot);
			SDK::Vector3 Direction(0, 0, 0);
			if (GetAsyncKeyState('W') & 0x8000) {
				Direction = Direction - LookVec;
				
			}
			if (GetAsyncKeyState('S') & 0x8000) {
				Direction = Direction + LookVec;
				
			}
			if (GetAsyncKeyState('A') & 0x8000) {
				Direction = Direction - RightVec;
				
			}
			if (GetAsyncKeyState('D') & 0x8000) {
				Direction = Direction + RightVec;
			
			}

			if (Direction.magnitude() > 0) {
				Direction = Direction.normalize();
			}
			HRP.Object.Primitive().SetVelocity(SDK::Vector3(Direction.x, 0, Direction.z) * Globals::World::Movement::CframeSpeed * 10);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		
	}
}


SDK::Vector3 UpVector(const SDK::Matrix3& rotationMatrix) {
	return { rotationMatrix.data[1], rotationMatrix.data[1 + 3], rotationMatrix.data[1 + 6] };
}
void FreeCam()
{
	SDK::Vector3 CurrentPos(0, 0, 0);
	bool FirstFrame = true;

	while (true)
	{
		Globals::World::Movement::FreecamKey.update();
		if (Globals::World::Movement::Freecam && Globals::World::Movement::FreecamKey.enabled)
		{
			SDK::Camera Camera = (SDK::Camera)Globals::Workspace.FindFirstChildOfClass("Camera");
			//SDK::Memory->Write<int>(Camera.Address + SDK::Offsets::CameraType, 6);
			SDK::Matrix3 CamRot = Camera.CameraRotation();
			SDK::Vector3 LookVec = LookVector(CamRot);
			SDK::Vector3 RightVec = RightVector(CamRot);
			SDK::Vector3 UpVec = UpVector(CamRot);

			if (FirstFrame)
			{
				CurrentPos = Camera.CameraPosition();
				FirstFrame = false;
			}

			SDK::Vector3 Direction(0, 0, 0);

			if (GetAsyncKeyState('S') & 0x8000) Direction += LookVec;
			if (GetAsyncKeyState('W') & 0x8000) Direction -= LookVec;
			if (GetAsyncKeyState('A') & 0x8000) Direction -= RightVec;
			if (GetAsyncKeyState('D') & 0x8000) Direction += RightVec;
			if (GetAsyncKeyState(VK_SPACE) & 0x8000) Direction += UpVec;
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000) Direction -= UpVec;
			
			if (Direction.magnitude() > 0.0001f)
				Direction = Direction.normalize();

			float Speed = Globals::World::Movement::FreecamSpeed;
			CurrentPos += Direction * Speed;

			Camera.SetCameraPosition(CurrentPos);
		}
		else
		{
			if (FirstFrame == false)
			{
				
			}
			SDK::Camera Camera = (SDK::Camera)Globals::Workspace.FindFirstChildOfClass("Camera");
			FirstFrame = true;
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
}

void ThirdPerson()
{
	SDK::Vector3 SmoothedPos(0, 0, 0);
	bool FirstFrame = true;

	while (true)
	{
		Globals::World::Movement::ThirdPersonKey.update();
		if (Globals::World::Movement::ThirdPerson && Globals::World::Movement::ThirdPersonKey.enabled)
		{
			SDK::Camera Camera = (SDK::Camera)Globals::Workspace.FindFirstChildOfClass("Camera");
			//SDK::Memory->Write<int>(Camera.Address + SDK::Offsets::CameraType, 6);
			SDK::Matrix3 CamRot = Camera.CameraRotation();
			SDK::Vector3 LookVec = LookVector(CamRot);

			SDK::Vector3 Offset = LookVec * 6.0f + SDK::Vector3(0, Globals::World::Movement::ThirdPersonDistance, 0);
			SDK::Vector3 Target = SDK::Cache::LocalPlayer.Head.WorldPosition + Offset;

			if (FirstFrame)
			{
				SmoothedPos = Target;
				FirstFrame = false;
			}

			SmoothedPos = SmoothedPos + (Target - SmoothedPos) * 0.15f;
			Camera.SetCameraPosition(SmoothedPos);
		}
		else
		{
			FirstFrame = true;
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
}

void InfiniteJump()
{
	while (true)
	{
		
		if (Globals::World::Movement::InfiniteJump)
		{
			if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
				SDK::Cache::Bone HRP = SDK::Cache::LocalPlayer.HumanoidRootPart;
				SDK::Humanoid Humanoid = SDK::Cache::LocalPlayer.Humanoid;
				if (!HRP.Object.Address || !Humanoid.Address)
				{
					
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
					continue;
				}
				float YVelocity = SDK::Memory->Read<float>(Humanoid.Address + SDK::Offsets::JumpPower);
				if (Globals::World::Movement::InfiniteJumpNameSpace::CustomJumpPower)
				{
					YVelocity = Globals::World::Movement::InfiniteJumpNameSpace::CustomJumpPowerValue;
				}
				SDK::Vector3 Velocity = HRP.Object.Primitive().Velocity();
				SDK::Vector3 NewVelocity(
					Velocity.x,
					YVelocity,
					Velocity.z
				);
			
				HRP.Object.Primitive().SetVelocity(NewVelocity);
			}
		}
		else
		{
			
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	}
}
SDK::Vector3 CrossProduct(SDK::Vector3 v1, SDK::Vector3 v2)
{
	return SDK::Vector3(
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x
	);
}
void Spinbot()
{
	float Angle = 0.0f;
	bool First = false;
	while (true)
	{
		if (Globals::World::Movement::Spinbot)
		{
			SDK::Cache::Bone HRP = SDK::Cache::LocalPlayer.HumanoidRootPart;
			SDK::Humanoid Humanoid = SDK::Cache::LocalPlayer.Humanoid;
			if (!HRP.Object.Address || !Humanoid.Address)
			{

				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}
				


				
				if (Humanoid.Address)
				{
					//SDK::Memory->Write<bool>(Humanoid.Address + 0x1E1, false);
					
				}


				
				if (HRP.Object.Address)
				{

					Angle += (0.01f * (Globals::World::Movement::SpinbotSpeed * (M_PI / 180.0)));


					SDK::Vector3 look = { sinf(Angle), 0.0f, cosf(Angle) };


					SDK::Vector3 up = { 0.0f, 1.0f, 0.0f };      
					
					if (Globals::World::Movement::UpsideDown)
					{
						up.y = -1;
						look = { -look.x, -look.y, -look.z };
					}
					SDK::Vector3 right = CrossProduct(up, look);
					SDK::Matrix3 rotationMatrix;
					SDK::Matrix3 SIGMA = SDK::Memory->Read<SDK::Matrix3>(HRP.Object.Primitive().Address + SDK::Offsets::Rotation);
					rotationMatrix.data[0] = right.x;
					rotationMatrix.data[3] = right.y;
					rotationMatrix.data[6] = right.z;


					rotationMatrix.data[1] = up.x;
					rotationMatrix.data[4] = up.y;
					rotationMatrix.data[7] = up.z;


					rotationMatrix.data[2] = look.x;
					rotationMatrix.data[5] = look.y;
					rotationMatrix.data[8] = look.z;



					SDK::Memory->Write<SDK::Matrix3>(HRP.Object.Primitive().Address + SDK::Offsets::Rotation, rotationMatrix);
				}
			
		}
		else
		{
			Angle = 0.0f;
			if (First)
			{
				SDK::Humanoid Humanoid = SDK::Cache::LocalPlayer.Humanoid;

				if (Humanoid.Address)
				{
					//SDK::Memory->Write<bool>(Humanoid.Address + 0x1E1, true);

				}
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
		}
		

		//std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
	}
}

void Zoom()
{
	float OldFOV = 0;
	bool FirstFrame = false;
	while (true)
	{
		Globals::World::ZoomKey.update();
		if (Globals::World::Zoom && Globals::World::ZoomKey.enabled)
		{
			
			
			FirstFrame = true;
			if (Globals::Workspace.FindFirstChildOfClass("Camera").Address)
				SDK::Memory->Write<float>(Globals::Workspace.FindFirstChildOfClass("Camera").Address + SDK::Offsets::FOV, (Globals::World::ZoomValue * 0.0174533f));
			
		}
		else
		{
			if (FirstFrame)
			{
				if (Globals::Workspace.FindFirstChildOfClass("Camera").Address)
					SDK::Memory->Write<float>(Globals::Workspace.FindFirstChildOfClass("Camera").Address + SDK::Offsets::FOV, OldFOV);
				FirstFrame = false;
			}
			if (Globals::Workspace.FindFirstChildOfClass("Camera").Address)
				OldFOV = SDK::Memory->Read<float>(Globals::Workspace.FindFirstChildOfClass("Camera").Address + SDK::Offsets::FOV);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}
void SpamTP()
{
	while (true)
	{
		if (Globals::Aimbot::Rage::SpamTP && Globals::Aimbot::Target::HaveTarget)
		{
			
			SDK::Cache::Player Target;
			SDK::Cache::PlayersMutex.lock();
			for (SDK::Cache::Player Player : SDK::Cache::Players)
			{
				if (Player.PlayerName == Globals::Aimbot::Target::Name)
				{
					Target = Player;
					break;
				}
			}
			SDK::Cache::PlayersMutex.unlock();
			if (Target.CharacterObjectAddress != 0)
			{
				SDK::Cache::Bone HRP = SDK::Cache::LocalPlayer.HumanoidRootPart;
				SDK::Cache::Bone HRPT = Target.HumanoidRootPart;
				if (!HRP.Object.Address || !HRPT.Object.Address)
				{

					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}
				if (Globals::Aimbot::Target::HaveTarget)
					HRP.Object.Primitive().SetPosition(HRPT.Object.Primitive().Position());
				
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}
void Orbit()
{
	
	auto start_time = std::chrono::high_resolution_clock::now();
	float radius = 1.0f;
	float orbitSpeed = 100.0f;
	float heightOffset = 10.0f;
	bool targetAcquired = false;
	bool firstFrame = false;
	while (true)
	{
		if (Globals::Aimbot::Rage::Orbit && Globals::Aimbot::Target::HaveTarget)
		{
			
			SDK::Cache::Player Target;
			SDK::Cache::PlayersMutex.lock();
			for (SDK::Cache::Player Player : SDK::Cache::Players)
			{
				if (Player.PlayerName == Globals::Aimbot::Target::Name)
				{
					Target = Player;
					break;
				}
			}
			SDK::Cache::PlayersMutex.unlock();
			if (Target.CharacterObjectAddress != 0)
			{
				SDK::Cache::Bone HRP = SDK::Cache::LocalPlayer.HumanoidRootPart;
				SDK::Cache::Bone HRPT = Target.HumanoidRootPart;

				if (!HRP.Object.Address || !HRPT.Object.Address)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}
				float speed = (float)30;
				float offset = (float)10;
				float radius = (float)10;
				SDK::Vector3 target_pos = HRPT.Object.Primitive().Position();
				auto current_time = std::chrono::high_resolution_clock::now();
				float time_elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(current_time - start_time).count();
				float angle = speed * time_elapsed;
				float new_x = target_pos.x + radius * cos(angle);
				float new_z = target_pos.z + radius * sin(angle);
				SDK::Vector3 new_pos = { new_x, target_pos.y + offset, new_z };
				HRP.Object.Primitive().SetPosition(new_pos);
			}

		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}



void Desync()
{
	bool First = false;
	while (true)
	{
		Globals::Misc::DesyncBind.update();
		if (Globals::Misc::DesyncBind.enabled && Globals::Misc::Desync)
		{
			if (!First)
			{
				Globals::Misc::DesyncedPostion = SDK::Cache::LocalPlayer.HumanoidRootPart.WorldPosition;
				First = true;
			}
			
			SDK::Memory->Write<int>(SDK::RobloxBase + SDK::Offsets::Senders::PhysicsSenderMaxBandwidthBps, 0);
		}
		else
		{
			if (First)
			{
				SDK::Memory->Write<int>(SDK::RobloxBase + SDK::Offsets::Senders::PhysicsSenderMaxBandwidthBps, 1000);
				First = false;
			}
			Globals::Misc::DesyncedPostion = SDK::Vector3(-1, -1, -1);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}


void Hacks::Misc::Setup()
{
	std::thread(FlyHack).detach();
	std::thread(Zoom).detach();
	std::thread(CFrame).detach();
	std::thread(FreeCam).detach();
	std::thread(ThirdPerson).detach();
	std::thread(InfiniteJump).detach();
	std::thread(Spinbot).detach();
	std::thread(SpamTP).detach();
	std::thread(Orbit).detach();
	std::thread(Desync).detach();
	

}
