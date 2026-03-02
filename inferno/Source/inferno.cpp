#include "Includes.hpp"
#include "Security/AntiTamper/AntiTamperLib.h"
#include <filesystem>
#include <fstream>


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
    std::thread(Overlay::Setup).detach();
    
    
    
    
    //std::cout << std::hex << Globals::LocalPlayer.Character().FindFirstChild("Humanoid").Address << std::endl;
    //std::cout << std::hex << Globals::Lighting.Address << std::endl;
    //std::cout << SDK::Memory->Read<int>(Globals::Lighting.Address + SDK::Offsets::ClockTime) << std::endl;
    while (true)
    {
        Globals::RobloxWindow = FindWindowA(0, xorstr_("Roblox"));
        if (Globals::RobloxWindow == NULL) {
            exit(0);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    VMProtectEnd();
}
