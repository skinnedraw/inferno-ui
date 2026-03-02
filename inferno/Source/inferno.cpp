#include "Includes.hpp"
#include "Security/AntiTamper/AntiTamperLib.h"
#include <filesystem>
#include <fstream>
#include <Json.hpp>


int main()
{

    VMProtectBeginUltra(xorstr_("Main()"));
    
    
    InitializeAntiTamper();
    
    
    
    
    
    system(xorstr_("cls"));
    if (!std::filesystem::exists("C:\\Athena")) {
        std::filesystem::create_directory("C:\\Athena");
    }
    
    if (!std::filesystem::exists("C:\\Athena\\Configs")) {
        std::filesystem::create_directory("C:\\Athena\\Configs");
    }
    Logging::Log(Logging::LogType::INFO, xorstr_("Hello World!"));
    SDK::MemoryClass* Memory = new SDK::MemoryClass();
    Memory->Handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, 0, SDK::GetProcessId(xorstr_("RobloxPlayerBeta.exe")));
    SDK::Memory = Memory;
    SDK::RobloxBase = Memory->GetModule(xorstr_("RobloxPlayerBeta.exe"));
    SDK::Initialize();
    std::thread(Overlay::Setup).detach();
    std::thread(SDK::Cache::PlayerCache).detach();
    std::thread(SDK::Cache::PlayerInfoUpdate).detach();
    std::thread(Hacks::Aimbot::Setup).detach();
    std::thread(Hacks::Misc::Setup).detach();
    
    
    
    
    //std::cout << std::hex << Globals::LocalPlayer.Character().FindFirstChild("Humanoid").Address << std::endl;
    //std::cout << std::hex << Globals::Lighting.Address << std::endl;
    //std::cout << SDK::Memory->Read<int>(Globals::Lighting.Address + SDK::Offsets::ClockTime) << std::endl;
    while (true)
    {
        Globals::RobloxWindow = FindWindowA(0, xorstr_("Roblox"));
        if (Globals::RobloxWindow == NULL) {
            exit(0);
        }
        SDK::VisualEngineCache::Dimensions = Globals::VisualEngine.Dimensions();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    VMProtectEnd();
}
