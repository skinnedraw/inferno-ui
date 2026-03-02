#pragma once
#include "../../Includes.hpp"
#include <vector>
#include <string>
namespace SDK
{
	class Primitive
	{
	public:
		uintptr_t Address;
		SDK::Vector3 Position()
		{
			if (!this->Address)
				return SDK::Vector3(0, 0, 0);
			return SDK::Memory->Read<SDK::Vector3>(this->Address + SDK::Offsets::Position);
		}
		SDK::Vector3 Size()
		{
			if (!this->Address)
				return SDK::Vector3(0, 0, 0);
			return SDK::Memory->Read<SDK::Vector3>(this->Address + SDK::Offsets::PartSize);
		}
		SDK::Vector3 Velocity()
		{
			if (!this->Address)
				return SDK::Vector3(0, 0, 0);
			return SDK::Memory->Read<SDK::Vector3>(this->Address + SDK::Offsets::Velocity);
		}
		SDK::Matrix3 Rotation()
		{
			if (!this->Address)
				return SDK::Matrix3();
			return SDK::Memory->Read<SDK::Matrix3>(this->Address + SDK::Offsets::Rotation);
		}
		void SetRotation(SDK::Matrix3 Value)
		{
			if (!this->Address)
				return;
			SDK::Memory->Write<SDK::Matrix3>(this->Address + SDK::Offsets::Rotation, Value);
		}
		void SetPosition(SDK::Vector3 Value)
		{
			if (!this->Address)
				return;
			SDK::Memory->Write<SDK::Vector3>(this->Address + SDK::Offsets::Position, Value);
		}
		void SetSize(SDK::Vector3 Value)
		{
			if (!this->Address)
				return;
			SDK::Memory->Write<SDK::Vector3>(this->Address + SDK::Offsets::PartSize, Value);
		}
		void SetVelocity(SDK::Vector3 Value)
		{
			if (!this->Address)
				return;
			SDK::Memory->Write<SDK::Vector3>(this->Address + SDK::Offsets::Velocity, Value);
		}
		int Material()
		{
			if (!this->Address)
				return 0;
			return SDK::Memory->Read<int>(this->Address + SDK::Offsets::MaterialType);
		}
		void SetMaterial(int16_t Material)
		{
			
			if (!this->Address)
				return;
			
			SDK::Memory->Write<int16_t>(this->Address + SDK::Offsets::MaterialType, static_cast<int16_t>(Material));
			
		}
		float Transparency()
		{
			if (!this->Address)
				return 0.0f;
			return SDK::Memory->Read<float>(this->Address + SDK::Offsets::Transparency);
		}
		void SetTransparency(float Value)
		{
			if (!this->Address)
				return;
			SDK::Memory->Write<float>(this->Address + SDK::Offsets::Transparency, Value);
		}

	};
}