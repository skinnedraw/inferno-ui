#pragma once
#include "../../Includes.hpp"
namespace SDK
{
	class Camera : public SDK::Instance
	{
	public:
		SDK::Matrix3 CameraRotation()
		{
			if (!this->Address)
				return SDK::Matrix3();
			return SDK::Memory->Read<SDK::Matrix3>(this->Address + SDK::Offsets::CameraRotation);
		}
		SDK::Vector3 CameraPosition()
		{
			if (!this->Address)
				return SDK::Vector3();
			return SDK::Memory->Read<SDK::Vector3>(this->Address + SDK::Offsets::CameraPos);
		}
		void SetCameraRotation(SDK::Matrix3 Value)
		{
			if (!this->Address)
				return;
			SDK::Memory->Write<SDK::Matrix3>(this->Address + SDK::Offsets::CameraRotation, Value);
		}
		void SetCameraPosition(SDK::Vector3 Value)
		{
			if (!this->Address)
				return;
			SDK::Memory->Write<SDK::Vector3>(this->Address + SDK::Offsets::CameraPos, Value);
		}
		void SpectateHumanoid(SDK::Humanoid Human)
		{
			if (!this->Address)
				return;
			SDK::Memory->Write<uintptr_t>(this->Address + SDK::Offsets::CameraSubject, Human.Address);
		}
	};
}
