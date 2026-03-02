#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <string>

namespace SDK
{
    inline uintptr_t GetProcessId(std::string_view processName)
    {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        HANDLE ss = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        while (Process32Next(ss, &pe)) {
            if (!processName.compare(pe.szExeFile)) {
                CloseHandle(ss);
                return pe.th32ProcessID;
            }
        }
        CloseHandle(ss);
        return 0;
    }

    typedef NTSTATUS(WINAPI* NtReadVirtualMemory_t)(HANDLE, PVOID, PVOID, ULONG, PULONG);
    typedef NTSTATUS(WINAPI* NtWriteVirtualMemory_t)(HANDLE, PVOID, PVOID, ULONG, PULONG);

    inline NtReadVirtualMemory_t GetNtReadVirtualMemorySyscall()
    {
        static uint8_t stub[] = {
            0x4C, 0x8B, 0xD1,
            0xB8, 0x3F, 0x00, 0x00, 0x00,
            0x0F, 0x05,
            0xC3
        };
        void* exec = VirtualAlloc(0, sizeof(stub), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        memcpy(exec, stub, sizeof(stub));
        return (NtReadVirtualMemory_t)exec;
    }

    inline NtWriteVirtualMemory_t GetNtWriteVirtualMemorySyscall()
    {
        static uint8_t stub[] = {
            0x4C, 0x8B, 0xD1,
            0xB8, 0x3A, 0x00, 0x00, 0x00,
            0x0F, 0x05,
            0xC3
        };
        void* exec = VirtualAlloc(0, sizeof(stub), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        memcpy(exec, stub, sizeof(stub));
        return (NtWriteVirtualMemory_t)exec;
    }

    class MemoryClass final
    {
    private:
        NtReadVirtualMemory_t VRead;
        NtWriteVirtualMemory_t VWrite;
    public:
        HANDLE Handle;

        MemoryClass()
        {
            VRead = GetNtReadVirtualMemorySyscall();
            VWrite = GetNtWriteVirtualMemorySyscall();
        }

        template <typename T>
        inline T Read(uintptr_t address)
        {
            T B{};
            VRead(Handle, (void*)address, &B, sizeof(T), nullptr);
            return B;
            
        }

        template <typename T>
        inline void Write(uintptr_t address, T v)
        {
            VWrite(Handle, (void*)address, &v, sizeof(T), nullptr);
        }

        inline std::string ReadStringRaw(uintptr_t address)
        {
            std::string s;
            char c = 0;
            int size = sizeof(c);
            int o = 0;
            s.reserve(204);
            while (o < 200)
            {
                c = this->Read<char>(address + o);
                if (c == 0)
                    break;
                o += size;
                s.push_back(c);
            }
            return s;
        }

        inline std::string ReadString(uintptr_t address)
        {
            int length = this->Read<uintptr_t>(address + 0x18);
            if (length >= 16u)
            {
                uintptr_t n = this->Read<uintptr_t>(address);
                return this->ReadStringRaw(n);
            }
            else
            {
                return this->ReadStringRaw(address);
            }
        }

        inline uintptr_t GetModule(std::string_view MN)
        {
            HMODULE modules[1024];
            DWORD neededmodule;
            if (EnumProcessModules(this->Handle, modules, sizeof(modules), &neededmodule))
            {
                int moduleCount = neededmodule / sizeof(HMODULE);
                for (int i = 0; i < moduleCount; ++i)
                {
                    char buffer[MAX_PATH];
                    if (GetModuleBaseNameA(this->Handle, modules[i], buffer, sizeof(buffer)))
                    {
                        if (!MN.compare(buffer)) {
                            return reinterpret_cast<uintptr_t>(modules[i]);
                        }
                    }
                }
            }
            return 0;
        }
    };

    inline MemoryClass* Memory;
    inline uintptr_t RobloxBase;
}
