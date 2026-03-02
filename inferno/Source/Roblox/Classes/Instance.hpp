#pragma once
#include "../../Includes.hpp"
#include <vector>
#include <string>
namespace SDK
{
	class Instance
	{
	public:
		uintptr_t Address;
		inline std::string Name()
		{
			if (!this->Address)
				return "?";
			uintptr_t StringPointer = SDK::Memory->Read<uintptr_t>(this->Address + SDK::Offsets::Name);
			if (!StringPointer)
				return "?";
			return SDK::Memory->ReadString(StringPointer);
		}
		inline std::string Class()
		{
			if (!this->Address)
				return "?";
			uintptr_t Descriptor = SDK::Memory->Read<uintptr_t>(this->Address + SDK::Offsets::ClassDescriptor);
			if (!Descriptor)
				return "?";
			uintptr_t StringPointer = SDK::Memory->Read<uintptr_t>(Descriptor + 0x8);
			if (!StringPointer)
				return "?";
			return SDK::Memory->ReadString(StringPointer);
		}
		inline SDK::Instance Parent()
		{
			if (!this->Address)
				return SDK::Instance();
			return SDK::Memory->Read<SDK::Instance>(this->Address + SDK::Offsets::Parent);
		}
		inline std::vector<SDK::Instance> Children()
		{
			std::vector<SDK::Instance> Container;

			if (!this->Address)
				return Container;
			auto Start = SDK::Memory->Read<uintptr_t>(this->Address + SDK::Offsets::Children);
			if (!Start)
				return Container;
			auto End = SDK::Memory->Read<uintptr_t>(Start + SDK::Offsets::ChildrenEnd);
			for (auto instances = SDK::Memory->Read<uintptr_t>(Start); instances != End; instances += 16)
				Container.emplace_back(SDK::Memory->Read<SDK::Instance>(instances));

			return Container;
		}
		inline SDK::Instance FindFirstChild(std::string Name)
		{
			if (!this->Address)
				return SDK::Instance();
			if (Name.empty())
				return SDK::Instance();
			std::vector<SDK::Instance> Children = this->Children();
			for (SDK::Instance Child : Children)
			{
				if (!Child.Address)
					continue;
				if (Child.Name() == Name)
					return Child;
			}
			return SDK::Instance();
		}
		inline SDK::Instance FindFirstChildOfClass(std::string Class)
		{
			if (!this->Address)
				return SDK::Instance();
			if (Class.empty())
				return SDK::Instance();
			std::vector<SDK::Instance> Children = this->Children();
			for (SDK::Instance Child : Children)
			{
				if (!Child.Address)
					continue;
				if (Child.Class() == Class)
					return Child;
			}
			return SDK::Instance();
		}
		inline SDK::Primitive Primitive()
		{
			if (!this->Address)
				return SDK::Primitive();
			return SDK::Memory->Read<SDK::Primitive>(this->Address + SDK::Offsets::Primitive);
		}
	};
}