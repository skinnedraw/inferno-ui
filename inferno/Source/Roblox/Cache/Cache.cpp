#define NOMINMAX
#include "Cache.h"
#include <cmath>
#include "algorithm"
#include <Json.hpp>
#include <winhttp.h>
#include "../Security/xorstr.hpp"
#pragma comment(lib, "winhttp.lib")
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
using namespace Clipper2Lib;
namespace BoneUtils
{
    inline PathD BoneHullToPath(const SDK::Cache::Bone bone) {
        PathD path;
        for (int i = 0; i < bone.Hull.HullSize; i++) {
            path.push_back(PointD(bone.Hull.Hull[i].x, bone.Hull.Hull[i].y));
        }
        return path;
    }
    inline PathsD UnionAllHulls(const std::vector<SDK::Cache::Bone>& bones) {
        PathsD input;
        for (auto bone : bones) {
            if (bone.Hull.HullSize >= 3 && bone.Hull.Hull) {
                input.push_back(BoneHullToPath(bone));
            }
        }
        PathsD solution = Union(input, FillRule::NonZero);
        return solution;
    }
    SDK::Cache::Bone InstanceToBone(SDK::Instance Instance)
    {
        SDK::Cache::Bone Bone;
        Bone.Name = Instance.Name();
        Bone.Object = Instance;
        return Bone;
    }
    SDK::Cache::CachedConvexHull CalculateBoneConvexHull(SDK::Cache::Bone& Bone)
    {
        if (!Bone.Object.Address)
            return { nullptr, 0 };
        SDK::Vector3 partpos3d = Bone.WorldPosition;
        SDK::Vector3 partsize = Bone.Size;
        //std::cout << partsize.x << ", " << partsize.y << ", " << partsize.z << std::endl;
        SDK::Matrix3 partrot = Bone.Rotation;
        SDK::Vector3 half_size = { partsize.x / 2, partsize.y / 2, partsize.z / 2 };
        SDK::Vector3 local_corners[8] = {
            {-half_size.x, -half_size.y, -half_size.z}, {half_size.x, -half_size.y, -half_size.z},
            {half_size.x, half_size.y, -half_size.z}, {-half_size.x, half_size.y, -half_size.z},
            {-half_size.x, -half_size.y, half_size.z}, {half_size.x, -half_size.y, half_size.z},
            {half_size.x, half_size.y, half_size.z}, {-half_size.x, half_size.y, half_size.z}
        };
        ImVec2 screen_points[8];
        int valid_points = 0;
        for (int i = 0; i < 8; i++) {
            SDK::Vector3 rotated = {
                partrot.data[0] * local_corners[i].x + partrot.data[1] * local_corners[i].y + partrot.data[2] * local_corners[i].z,
                partrot.data[3] * local_corners[i].x + partrot.data[4] * local_corners[i].y + partrot.data[5] * local_corners[i].z,
                partrot.data[6] * local_corners[i].x + partrot.data[7] * local_corners[i].y + partrot.data[8] * local_corners[i].z
            };
            SDK::Vector3 world_corners = {
                partpos3d.x + rotated.x,
                partpos3d.y + rotated.y,
                partpos3d.z + rotated.z
            };
            SDK::Vector2 screen_pos = SDK::WorldToScreen(world_corners);
            screen_pos = SDK::Vector2(std::floor(screen_pos.x), std::floor(screen_pos.y));
            if (screen_pos.x != -1 && screen_pos.y != -1) {
                screen_points[valid_points++] = { screen_pos.x, screen_pos.y };
            }
        }
        if (valid_points < 3) return { nullptr, 0 };
        std::sort(screen_points, screen_points + valid_points, [](ImVec2 a, ImVec2 b) {
            return a.x < b.x || (a.x == b.x && a.y < b.y);
            });
        ImVec2 hull[16];
        int hull_size = 0;
        for (int i = 0; i < valid_points; ++i) {
            while (hull_size >= 2 && ((hull[hull_size - 1].x - hull[hull_size - 2].x) * (screen_points[i].y - hull[hull_size - 2].y) - (hull[hull_size - 1].y - hull[hull_size - 2].y) * (screen_points[i].x - hull[hull_size - 2].x)) <= 0)
                hull_size--;
            hull[hull_size++] = screen_points[i];
        }
        int lower_hull_size = hull_size;
        for (int i = valid_points - 2; i >= 0; --i) {
            while (hull_size > lower_hull_size && ((hull[hull_size - 1].x - hull[hull_size - 2].x) * (screen_points[i].y - hull[hull_size - 2].y) - (hull[hull_size - 1].y - hull[hull_size - 2].y) * (screen_points[i].x - hull[hull_size - 2].x)) <= 0)
                hull_size--;
            hull[hull_size++] = screen_points[i];
        }
        hull_size--;
        if (hull_size < 3) return { nullptr, 0 };

        ImVec2* dynamic_hull = new ImVec2[hull_size];
        for (int i = 0; i < hull_size; i++) {
            dynamic_hull[i] = hull[i];
        }

        SDK::Cache::CachedConvexHull Hull;
        Hull.HullSize = hull_size;
        Hull.Hull = dynamic_hull;
        return Hull;
    }
    
    void UpdateBoneInfo(SDK::Cache::Bone& Bone)
    {
        if (!Bone.Object.Address)
            return;
        SDK::Primitive Primitive = Bone.Object.Primitive();
        if (Bone.CacheSize)
            Bone.Size = Primitive.Size();
        Bone.WorldPosition = Primitive.Position();
        Bone.Rotation = Primitive.Rotation();
        Bone.ScreenPosition = SDK::WorldToScreen(Bone.WorldPosition);
        Bone.Hull = BoneUtils::CalculateBoneConvexHull(Bone);
    }
}
void SDK::Cache::PlayerCache()
{

    while (true)
    {
        if (!Globals::DataModel.FindFirstChildOfClass("Workspace").Address)
            continue;
        try
        {
            if (Globals::GameID == SDK::Games::BadBusiness)
            {
                std::vector<SDK::Cache::Player> CacheList;
                std::vector<SDK::Instance> Players;
                Players = Globals::Workspace.FindFirstChild("Characters").Children();
                if (Globals::Misc::TeamCheck)
                {
                    Players.clear();
                    for (SDK::Instance Highlight : Globals::LocalPlayer.FindFirstChild("PlayerGui").Children())
                    {
                        if (Highlight.Class() == "Highlight")
                        {
                            SDK::Instance Adornee = SDK::Memory->Read<SDK::Instance>(Highlight.Address + SDK::Offsets::Adornee);
                            if (!Adornee.Address)
                                continue;
                            bool alreadyAdded = false;
                            for (SDK::Instance player : Players) {
                                if (player.Address == Adornee.Address) {
                                    alreadyAdded = true;
                                    break;
                                }
                            }
                            if (!alreadyAdded) {
                                Players.push_back(Adornee);
                            }
                        }
                    }
                }
                for (SDK::Instance Player : Players)
                {
                    if (!Player.Address)
                        continue;
                    SDK::Cache::Player Cache;
                    Cache.PlayerObject = (SDK::Player)Player;
                    Cache.PlayerObjectAddress = Player.Address;
                    Cache.CharacterObject = Player;
                    Cache.CharacterObjectAddress = Cache.CharacterObject.Address;
                   
                    Cache.PlayerName = "Player " + Player.Name();
                    Cache.DisplayName = "Player";
                    Cache.HumanoidRootPart = BoneUtils::InstanceToBone(Cache.CharacterObject.FindFirstChild("Root"));
                    Cache.Humanoid = SDK::Humanoid();
                    Cache.Head = BoneUtils::InstanceToBone(Cache.CharacterObject.FindFirstChild("Body").FindFirstChild("Head"));
                    Cache.RigType = 3;
                    Cache.Health = static_cast<float>(SDK::Memory->Read<double>(Cache.CharacterObject.FindFirstChild("Health").Address + SDK::Offsets::Value));
                  
                    Cache.MaxHealth = static_cast<float>(SDK::Memory->Read<double>(Cache.CharacterObject.FindFirstChild("Health").FindFirstChild("MaxHealth").Address + SDK::Offsets::Value));
                   
                    BoneUtils::UpdateBoneInfo(Cache.Head);
                    BoneUtils::UpdateBoneInfo(Cache.HumanoidRootPart);
                    for (SDK::Instance Bone : Cache.CharacterObject.FindFirstChild("Body").Children())
                    {
                        std::string Class = Bone.Class();

                        if (Class != "Part" && Class != "MeshPart")
                            continue;
                        SDK::Cache::Bone NewBone = BoneUtils::InstanceToBone(Bone);
                        BoneUtils::UpdateBoneInfo(NewBone);

                        Cache.Bones.push_back(NewBone);
                    }
                   
                    float left = FLT_MAX, top = FLT_MAX, right = -FLT_MAX, bottom = -FLT_MAX;
                    for (SDK::Cache::Bone& Bone : Cache.Bones)
                    {
                        if (!Bone.Object.Address || Bone.Name == "HumanoidRootPart") continue;

                        SDK::Vector3 pos = Bone.WorldPosition;
                        float hx = Bone.Size.x * 0.5f, hy = Bone.Size.y * 0.5f, hz = Bone.Size.z * 0.5f;

                        SDK::Vector3 corners[8] = {
                            {pos.x - hx, pos.y - hy, pos.z - hz}, {pos.x + hx, pos.y - hy, pos.z - hz},
                            {pos.x - hx, pos.y + hy, pos.z - hz}, {pos.x + hx, pos.y + hy, pos.z - hz},
                            {pos.x - hx, pos.y - hy, pos.z + hz}, {pos.x + hx, pos.y - hy, pos.z + hz},
                            {pos.x - hx, pos.y + hy, pos.z + hz}, {pos.x + hx, pos.y + hy, pos.z + hz}
                        };

                        for (int i = 0; i < 8; ++i) {
                            SDK::Vector2 screen = SDK::WorldToScreen(corners[i]);
                            if (screen.x != -1 && screen.y != -1) {
                                if (screen.x < left) left = screen.x;
                                if (screen.y < top) top = screen.y;
                                if (screen.x > right) right = screen.x;
                                if (screen.y > bottom) bottom = screen.y;
                            }
                        }
                    }
                    if (left < right && top < bottom) {
                        Cache.BoxMin = ImVec2(std::floor(left), std::floor(top));
                        Cache.BoxMax = ImVec2(std::floor(right), std::floor(bottom));
                    }
                    Cache.HighlightSolution = BoneUtils::UnionAllHulls(Cache.Bones);
                    CacheList.push_back(Cache);
                    if (!Cache.CharacterObject.FindFirstChild("Clothes").Address)
                    {
                        SDK::Cache::LocalPlayer = Cache;
                    }
                }
                for (SDK::Cache::Player& NewPlayer : CacheList)
                {
                    if (SDK::Cache::LocalPlayer.PlayerObjectAddress)
                    {
                        NewPlayer.Distance = NewPlayer.HumanoidRootPart.WorldPosition.distance(SDK::Cache::LocalPlayer.HumanoidRootPart.WorldPosition);
                    }
                }
                SDK::Cache::PlayersMutex.lock();
                SDK::Cache::Players = std::move(CacheList);
                SDK::Cache::PlayersMutex.unlock();
            }
            else if (Globals::GameID == SDK::Games::PhantomForces)
            {
               
                std::vector<SDK::Cache::Player> CacheList;
                std::vector<SDK::Instance> Players;
                std::vector<SDK::Instance> Team1 = Globals::Workspace.FindFirstChild("Players").Children()[0].Children();
                std::vector<SDK::Instance> Team2 = Globals::Workspace.FindFirstChild("Players").Children()[1].Children();
                Players.insert(Players.end(), Team1.begin(), Team1.end());
                Players.insert(Players.end(), Team2.begin(), Team2.end());
                
                Logging::Log(Logging::LogType::WARNING, "Players - %s", std::to_string(Players.size()));

                for (SDK::Instance Player : Players)
                {
                    SDK::Cache::Player Cache;
                    Cache.PlayerName = "Yo";
                    Cache.DisplayName = "YoWSP";
                    Cache.CharacterObject = Player;
                    Cache.CharacterObjectAddress = Player.Address;
                    Cache.PlayerObject = (SDK::Player)Player;
                    Cache.PlayerObjectAddress = Player.Address;
                    Cache.MaxHealth = 100;
                    Cache.Health = 100;
                    Cache.RigType = 0;
                    
                    for (SDK::Instance Bone : Player.Children())
                    {
                        std::string Class = Bone.Class();

                        if (Class != "Part" && Class != "MeshPart" && Class != "TrussPart")
                            continue;
                        SDK::Cache::Bone NewBone = BoneUtils::InstanceToBone(Bone);
                        BoneUtils::UpdateBoneInfo(NewBone);
                        bool IsHead, IsTorso = false;
                        if (Bone.FindFirstChildOfClass("SpotLight").Address)
                        {
                            Cache.HumanoidRootPart = NewBone;
                            NewBone.Size = Vector3(2.0f, 2.0f, 1.0f);
                            NewBone.CacheSize = false;
                            IsTorso = true;
                        }
                        if (Bone.FindFirstChildOfClass("BillboardGui").Address)
                        {
                            Cache.Head = NewBone;
                            std::string Name = SDK::Memory->ReadString(Bone.FindFirstChildOfClass("BillboardGui").FindFirstChildOfClass("TextLabel").Address + 0xdf0);
                            Cache.PlayerName = Name;
                            Cache.DisplayName = Name;
                            NewBone.CacheSize = false;
                            NewBone.Size = SDK::Vector3(1.0f, 1.0f, 1.0f);
                            Cache.Head.WorldPosition = Cache.Head.Object.Primitive().Position();
                            IsHead = true;
                        }
                        if (!(IsHead && IsTorso))
                        {
                           
                            NewBone.CacheSize = false;
                            NewBone.Size = SDK::Vector3(1.0f, 2.0f, 1.0f);
                            if (Bone.FindFirstChildOfClass("BillboardGui").Address)
                            {
                                NewBone.Size = SDK::Vector3(1.0f, 1.0f, 1.0f);
                            }
                        }
                        BoneUtils::UpdateBoneInfo(NewBone);
                        Cache.Bones.push_back(NewBone);
                       
                    }
                    /*
                    for (SDK::Instance Bone : Player.FindFirstChildOfClass("Folder").Children())
                    {
                        std::string Class = Bone.Class();

                        if (Class != "Part" && Class != "MeshPart")
                            continue;
                        SDK::Cache::Bone NewBone = BoneUtils::InstanceToBone(Bone);
                        BoneUtils::UpdateBoneInfo(NewBone);



                        Cache.Bones.push_back(NewBone);
                    }
                    */
                    BoneUtils::UpdateBoneInfo(Cache.Head);
                    BoneUtils::UpdateBoneInfo(Cache.HumanoidRootPart);
                    for (SDK::Instance StupidFag : Globals::Players.Children())
                    {
                        if (StupidFag.Name() == Cache.PlayerName)
                        {
                            Cache.PlayerObject = (SDK::Player)StupidFag;
                            Cache.PlayerObjectAddress = StupidFag.Address;
                        }
                    }
                    Cache.Team = Cache.PlayerObject.Team().Address;
                    if (Globals::Misc::TeamCheck && Cache.Team == Globals::LocalPlayer.Team().Address)
                        continue;
                    float left = FLT_MAX, top = FLT_MAX, right = -FLT_MAX, bottom = -FLT_MAX;
                    for (SDK::Cache::Bone& Bone : Cache.Bones)
                    {
                        if (!Bone.Object.Address || Bone.Name == "HumanoidRootPart") continue;

                        SDK::Vector3 pos = Bone.WorldPosition;
                        float hx = Bone.Size.x * 0.5f, hy = Bone.Size.y * 0.5f, hz = Bone.Size.z * 0.5f;

                        SDK::Vector3 corners[8] = {
                            {pos.x - hx, pos.y - hy, pos.z - hz}, {pos.x + hx, pos.y - hy, pos.z - hz},
                            {pos.x - hx, pos.y + hy, pos.z - hz}, {pos.x + hx, pos.y + hy, pos.z - hz},
                            {pos.x - hx, pos.y - hy, pos.z + hz}, {pos.x + hx, pos.y - hy, pos.z + hz},
                            {pos.x - hx, pos.y + hy, pos.z + hz}, {pos.x + hx, pos.y + hy, pos.z + hz}
                        };

                        for (int i = 0; i < 8; ++i) {
                            SDK::Vector2 screen = SDK::WorldToScreen(corners[i]);
                            if (screen.x != -1 && screen.y != -1) {
                                if (screen.x < left) left = screen.x;
                                if (screen.y < top) top = screen.y;
                                if (screen.x > right) right = screen.x;
                                if (screen.y > bottom) bottom = screen.y;
                            }
                        }
                    }
                    if (left < right && top < bottom) {
                        Cache.BoxMin = ImVec2(std::floor(left), std::floor(top));
                        Cache.BoxMax = ImVec2(std::floor(right), std::floor(bottom));
                    }
                    Cache.HighlightSolution = BoneUtils::UnionAllHulls(Cache.Bones);
                    CacheList.push_back(Cache);
                    
                }
                for (SDK::Cache::Player& NewPlayer : CacheList)
                {
                   
                        NewPlayer.Distance = NewPlayer.Head.Object.Primitive().Position().distance(static_cast<SDK::Camera>(Globals::Workspace.FindFirstChildOfClass("Camera")).CameraPosition());
                    
                }
                if (Globals::Workspace.FindFirstChild("Ignore").FindFirstChildOfClass("Model").Address)
                {
                    SDK::Instance LocalModel = Globals::Workspace.FindFirstChild("Ignore").FindFirstChildOfClass("Model");
                    SDK::Player Player = Globals::LocalPlayer;
                    SDK::Cache::Player Cache;
                    Cache.PlayerObject = Player;
                    Cache.PlayerObjectAddress = Player.Address;
                    Cache.CharacterObject = LocalModel;
                    Cache.CharacterObjectAddress = Cache.CharacterObject.Address;
                   
                    Cache.PlayerName = Player.Name();
                    Cache.DisplayName = Player.DisplayName();
                    Cache.HumanoidRootPart = BoneUtils::InstanceToBone(Cache.CharacterObject.FindFirstChild("HumanoidRootPart"));
                    Cache.Head = BoneUtils::InstanceToBone(Cache.CharacterObject.FindFirstChild("Head"));
                    Cache.Humanoid = static_cast<SDK::Humanoid>(Cache.CharacterObject.FindFirstChildOfClass("Humanoid"));
                    Cache.RigType = Cache.Humanoid.RigType();
                    
                    Cache.Health = Cache.Humanoid.Health();
                    Cache.MaxHealth = Cache.Humanoid.MaxHealth();
                    Cache.Team = Cache.PlayerObject.Team().Address;
                    BoneUtils::UpdateBoneInfo(Cache.Head);
                    BoneUtils::UpdateBoneInfo(Cache.HumanoidRootPart);

                    for (SDK::Instance Bone : Cache.CharacterObject.Children())
                    {
                        std::string Class = Bone.Class();

                        if (Class != "Part" && Class != "MeshPart")
                            continue;
                        SDK::Cache::Bone NewBone = BoneUtils::InstanceToBone(Bone);
                        BoneUtils::UpdateBoneInfo(NewBone);

                        Cache.Bones.push_back(NewBone);
                    }
                    Logging::Log(Logging::LogType::WARNING, "LocalPlayer Bones - %s", std::to_string(Cache.Bones.size()));
                    Logging::Log(Logging::LogType::WARNING, "LocalPlayer Head - 0x%llX", Cache.Head.Object.Address);
                    Logging::Log(Logging::LogType::WARNING, "LocalPlayer HumanoidRootPart - 0x%llX", Cache.HumanoidRootPart.Object.Address);
                    Logging::Log(Logging::LogType::WARNING, "LocalPlayer Humanoid - 0x%llX", Cache.Humanoid.Address);
                    Logging::Log(Logging::LogType::WARNING, "LocalPlayer Health - %s/%s", std::to_string(Cache.Health), std::to_string(Cache.MaxHealth));
                    SDK::Cache::LocalPlayer = Cache;
                    Cache.HighlightSolution = BoneUtils::UnionAllHulls(Cache.Bones);
                    CacheList.push_back(Cache);
                }
                Logging::Log(Logging::LogType::WARNING, "Players Cached - %s", std::to_string(CacheList.size()));
                SDK::Cache::PlayersMutex.lock();
                SDK::Cache::Players = std::move(CacheList);
                SDK::Cache::PlayersMutex.unlock();
                
            }
            else
            {
                std::string LocalPlayerName = Globals::LocalPlayer.Name();
                std::vector<SDK::Cache::Player> CacheList;
                for (SDK::Instance Instance : Globals::Players.Children())
                {
                    SDK::Player Player = (SDK::Player)Instance;
                    SDK::Cache::Player Cache;
                    Cache.PlayerObject = Player;
                    Cache.PlayerObjectAddress = Player.Address;
                    Cache.CharacterObject = Player.Character();
                    Cache.CharacterObjectAddress = Cache.CharacterObject.Address;
                    if (!Cache.CharacterObjectAddress)
                        continue;
                    if (!Cache.PlayerObjectAddress)
                        continue;
                    Cache.PlayerName = Player.Name();
                    Cache.DisplayName = Player.DisplayName();
                    Cache.HumanoidRootPart = BoneUtils::InstanceToBone(Cache.CharacterObject.FindFirstChild("HumanoidRootPart"));
                    Cache.Head = BoneUtils::InstanceToBone(Cache.CharacterObject.FindFirstChild("Head"));
                    Cache.Humanoid = static_cast<SDK::Humanoid>(Cache.CharacterObject.FindFirstChildOfClass("Humanoid"));
                    Cache.RigType = Cache.Humanoid.RigType();
                    if (!Cache.CharacterObject.FindFirstChild("Head").Address)
                        continue;
                    Cache.Health = Cache.Humanoid.Health();
                    Cache.MaxHealth = Cache.Humanoid.MaxHealth();
                    Cache.Team = Cache.PlayerObject.Team().Address;
                   
                    if (Cache.PlayerObjectAddress == Globals::Players.LocalPlayer().Address)
                    {
                       
                        SDK::Cache::LocalPlayer = Cache;
                        auto playergui = Player.FindFirstChildOfClass("PlayerGui");
                        SDK::Crosshairs.clear();
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("MainScreenGui").FindFirstChild("Aim"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("IntellectUi").FindFirstChild("Aim"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("Main Screen").FindFirstChild("Aim"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("Interface").FindFirstChild("Main").FindFirstChild("Aim"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("MouseUnlock").FindFirstChild("Frame"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("Crosshair").FindFirstChild("Main"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("CoreGUI").FindFirstChild("Crosshair"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("Cursor").FindFirstChild("dot"));
                       // SDK::Crosshairs.push_back(playergui.FindFirstChild("ItemGUI").FindFirstChild("Crosshair"));
                    }
                    if (Globals::Misc::TeamCheck && Cache.Team == SDK::Cache::LocalPlayer.Team)
                        continue;
                    if (Globals::Esp::Region)
                    {
                       
                        std::string Flag = SDK::Memory->ReadString(Cache.PlayerObjectAddress + 0xF8);
                        SDK::Cache::Flag FlagObject;
                        FlagObject.Flag = Flag;
                        FlagObject.Color = ImColor(Globals::Esp::Colors::Region[0], Globals::Esp::Colors::Region[1], Globals::Esp::Colors::Region[2], Globals::Esp::Colors::Region[3]);
                        Cache.Flags.push_back(FlagObject);
                    }
                    if (Globals::Esp::Leaderstats)
                    {
                        SDK::Instance LeaderStats = Cache.PlayerObject.FindFirstChild("leaderstats");
                        if (LeaderStats.Address)
                        {
                            for (SDK::Instance Stat : LeaderStats.Children())
                            {
                                if (Stat.Class() == "IntValue")
                                {
                                    int Value = SDK::Memory->Read<int>(Stat.Address + SDK::Offsets::Value);
                                    std::string Name = Stat.Name();
                                    std::string Flag = Name + ": " + std::to_string(Value);
                                    SDK::Cache::Flag FlagObject;
                                    FlagObject.Flag = Flag;
                                    FlagObject.Color = ImColor(Globals::Esp::Colors::LeaderStats[0], Globals::Esp::Colors::LeaderStats[1], Globals::Esp::Colors::LeaderStats[2], Globals::Esp::Colors::LeaderStats[3]);
                                    Cache.Flags.push_back(FlagObject);
                                }
                                if (Stat.Class() == "NumberValue")
                                {
                                    double Value = SDK::Memory->Read<double>(Stat.Address + SDK::Offsets::Value);
                                    std::string Name = Stat.Name();
                                    std::string Flag = Name + ": " + std::to_string(Value);
                                    SDK::Cache::Flag FlagObject;
                                    FlagObject.Flag = Flag;
                                    FlagObject.Color = ImColor(Globals::Esp::Colors::LeaderStats[0], Globals::Esp::Colors::LeaderStats[1], Globals::Esp::Colors::LeaderStats[2], Globals::Esp::Colors::LeaderStats[3]);
                                    Cache.Flags.push_back(FlagObject);
                                }
                                if (Stat.Class() == "BoolValue")
                                {
                                    bool Value = SDK::Memory->Read<bool>(Stat.Address + SDK::Offsets::Value);
                                    std::string Name = Stat.Name();
                                    std::string Flag = Name + ": " + (Value ? "True" : "False");
                                    SDK::Cache::Flag FlagObject;
                                    FlagObject.Flag = Flag;
                                    FlagObject.Color = ImColor(Globals::Esp::Colors::LeaderStats[0], Globals::Esp::Colors::LeaderStats[1], Globals::Esp::Colors::LeaderStats[2], Globals::Esp::Colors::LeaderStats[3]);
                                    Cache.Flags.push_back(FlagObject);
                                }
                                if (Stat.Class() == "StringValue")
                                {
                                    std::string Value = SDK::Memory->ReadString(Stat.Address + SDK::Offsets::Value);
                                    std::string Name = Stat.Name();
                                    std::string Flag = Name + ": " + Value;
                                    SDK::Cache::Flag FlagObject;
                                    FlagObject.Flag = Flag;
                                    FlagObject.Color = ImColor(Globals::Esp::Colors::LeaderStats[0], Globals::Esp::Colors::LeaderStats[1], Globals::Esp::Colors::LeaderStats[2], Globals::Esp::Colors::LeaderStats[3]);
                                    Cache.Flags.push_back(FlagObject);
                                }
                            }
                        }

                    }
                    if (Player.Team().Address && Globals::Esp::Team)
                    {
                        SDK::Cache::Flag Friendly;
                        Friendly.Flag = Player.Team().Name();


                        SDK::Color3 TeamColor = Player.Team().TeamColor();
                        Friendly.Color = ImColor(TeamColor.r, TeamColor.g, TeamColor.b);


                        Cache.Flags.push_back(Friendly);

                    }
                    BoneUtils::UpdateBoneInfo(Cache.Head);
                    BoneUtils::UpdateBoneInfo(Cache.HumanoidRootPart);
                    if (Cache.CharacterObject.FindFirstChildOfClass("Tool").Address)
                        Cache.Weapon = Cache.CharacterObject.FindFirstChildOfClass("Tool").Name();
                    
                    if (std::to_string(Globals::GameID) == "3747388906")
                    {
                       
                        for (SDK::Instance Weapon : Cache.CharacterObject.Children())
                        {
                            if (Weapon.Class() != "Model")
                                continue;
                            std::string Name = Weapon.Name();
                            
                            if (Name.find("Armor") == std::string::npos && Name != "Hair" && Name != "HolsterModel")
                            {
                               
                                Cache.Weapon = Name;
                            }

                        }
                    }
                    for (SDK::Instance Bone : Cache.CharacterObject.Children())
                    {
                        std::string Class = Bone.Class();

                        if (Class != "Part" && Class != "MeshPart")
                            continue;
                        SDK::Cache::Bone NewBone = BoneUtils::InstanceToBone(Bone);
                        BoneUtils::UpdateBoneInfo(NewBone);

                        Cache.Bones.push_back(NewBone);
                    }
                  
                    float left = FLT_MAX, top = FLT_MAX, right = -FLT_MAX, bottom = -FLT_MAX;
                    for (SDK::Cache::Bone& Bone : Cache.Bones)
                    {
                        if (!Bone.Object.Address || Bone.Name == "HumanoidRootPart") continue;

                        SDK::Vector3 pos = Bone.WorldPosition;
                        float hx = Bone.Size.x * 0.5f, hy = Bone.Size.y * 0.5f, hz = Bone.Size.z * 0.5f;

                        SDK::Vector3 corners[8] = {
                            {pos.x - hx, pos.y - hy, pos.z - hz}, {pos.x + hx, pos.y - hy, pos.z - hz},
                            {pos.x - hx, pos.y + hy, pos.z - hz}, {pos.x + hx, pos.y + hy, pos.z - hz},
                            {pos.x - hx, pos.y - hy, pos.z + hz}, {pos.x + hx, pos.y - hy, pos.z + hz},
                            {pos.x - hx, pos.y + hy, pos.z + hz}, {pos.x + hx, pos.y + hy, pos.z + hz}
                        };

                        for (int i = 0; i < 8; ++i) {
                            SDK::Vector2 screen = SDK::WorldToScreen(corners[i]);
                            if (screen.x != -1 && screen.y != -1) {
                                if (screen.x < left) left = screen.x;
                                if (screen.y < top) top = screen.y;
                                if (screen.x > right) right = screen.x;
                                if (screen.y > bottom) bottom = screen.y;
                            }
                        }
                    }
                    if (left < right && top < bottom) {
                        Cache.BoxMin = ImVec2(std::floor(left), std::floor(top));
                        Cache.BoxMax = ImVec2(std::floor(right), std::floor(bottom));
                    }
                    Cache.HighlightSolution = BoneUtils::UnionAllHulls(Cache.Bones);
                    //Cache.Skeleton = BoneUtils::CalculateSkeleton(Cache);
                    CacheList.push_back(Cache);
                    
                }

                for (SDK::Cache::Player& NewPlayer : CacheList)
                {
                    if (NewPlayer.PlayerObjectAddress == Globals::Players.LocalPlayer().Address)
                    {
                       
                        SDK::Cache::LocalPlayer = NewPlayer;
                        auto playergui = NewPlayer.PlayerObject.FindFirstChildOfClass("PlayerGui");
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("MainScreenGui").FindFirstChild("Aim"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("IntellectUi").FindFirstChild("Aim"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("Main Screen").FindFirstChild("Aim"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("Interface").FindFirstChild("Main").FindFirstChild("Aim"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("MouseUnlock").FindFirstChild("Frame"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("Crosshair").FindFirstChild("Main"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("CoreGUI").FindFirstChild("Crosshair"));
                        SDK::Crosshairs.push_back(playergui.FindFirstChild("Cursor").FindFirstChild("dot"));
                    }
                    if (SDK::Cache::LocalPlayer.PlayerObjectAddress)
                    {
                        NewPlayer.Distance = NewPlayer.HumanoidRootPart.WorldPosition.distance(SDK::Cache::LocalPlayer.HumanoidRootPart.WorldPosition);
                    }
                }
                SDK::Cache::PlayersMutex.lock();
                SDK::Cache::Players = std::move(CacheList);
                SDK::Cache::PlayersMutex.unlock();
            }
        }
        catch (...)
        {
            Logging::Log(Logging::LogType::ERR, "Caught crash at cache");
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


void UpdatePositions()
{
    while (true)
    {
        if (!Globals::DataModel.FindFirstChildOfClass("Workspace").Address)
            continue;
        try
        {
            if (SDK::Cache::Players.size() == 0)
                std::this_thread::sleep_for(std::chrono::seconds(1));
            BoneUtils::UpdateBoneInfo(SDK::Cache::LocalPlayer.HumanoidRootPart);
            BoneUtils::UpdateBoneInfo(SDK::Cache::LocalPlayer.Head);
            SDK::Cache::PlayersMutex.lock();
            for (SDK::Cache::Player& Player : SDK::Cache::Players)
            {
                BoneUtils::UpdateBoneInfo(Player.Head);
                BoneUtils::UpdateBoneInfo(Player.HumanoidRootPart);
               
                float left = FLT_MAX, top = FLT_MAX, right = -FLT_MAX, bottom = -FLT_MAX;
                for (SDK::Cache::Bone& Bone : Player.Bones)
                {
                    BoneUtils::UpdateBoneInfo(Bone);
                    if (!Bone.Object.Address || Bone.Name == "HumanoidRootPart") continue;

                    SDK::Vector3 pos = Bone.WorldPosition;
                    float hx = Bone.Size.x * 0.5f, hy = Bone.Size.y * 0.5f, hz = Bone.Size.z * 0.5f;

                    SDK::Vector3 corners[8] = {
                        {pos.x - hx, pos.y - hy, pos.z - hz}, {pos.x + hx, pos.y - hy, pos.z - hz},
                        {pos.x - hx, pos.y + hy, pos.z - hz}, {pos.x + hx, pos.y + hy, pos.z - hz},
                        {pos.x - hx, pos.y - hy, pos.z + hz}, {pos.x + hx, pos.y - hy, pos.z + hz},
                        {pos.x - hx, pos.y + hy, pos.z + hz}, {pos.x + hx, pos.y + hy, pos.z + hz}
                    };

                    for (int i = 0; i < 8; ++i) {
                        SDK::Vector2 screen = SDK::WorldToScreen(corners[i]);
                        if (screen.x != -1 && screen.y != -1) {
                            if (screen.x < left) left = screen.x;
                            if (screen.y < top) top = screen.y;
                            if (screen.x > right) right = screen.x;
                            if (screen.y > bottom) bottom = screen.y;
                        }
                    }
                }
                if (left < right && top < bottom) {
                    Player.BoxMin = ImVec2(std::floor(left), std::floor(top));
                    Player.BoxMax = ImVec2(std::floor(right), std::floor(bottom));
                }
                if (SDK::Cache::LocalPlayer.PlayerObjectAddress)
                {
                    Player.Distance = Player.Head.WorldPosition.distance(static_cast<SDK::Camera>(Globals::Workspace.FindFirstChildOfClass("Camera")).CameraPosition());
                }
                Player.HighlightSolution = BoneUtils::UnionAllHulls(Player.Bones);
                // Player.Skeleton = BoneUtils::CalculateSkeleton(Player);
               
            }
            SDK::Cache::PlayersMutex.unlock();
        }
        catch (...)
        {
            Logging::Log(Logging::LogType::ERR, "Caught crash at cache position update");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
namespace HTTPTHINGS
{
    std::string SecureRequest(const std::string& URL) {
        if (URL.empty()) {
            return "";
        }

        std::string response;
        HINTERNET hSession = NULL;
        HINTERNET hConnect = NULL;
        HINTERNET hRequest = NULL;

        try {
            // Parse URL manually
            std::string url = URL;
            std::string protocol, host, path;
            INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT;
            DWORD flags = 0;

            size_t protocolEnd = url.find("://");
            if (protocolEnd != std::string::npos) {
                protocol = url.substr(0, protocolEnd);
                url = url.substr(protocolEnd + 3);
                if (protocol == "https") {
                    port = INTERNET_DEFAULT_HTTPS_PORT;
                    flags = WINHTTP_FLAG_SECURE;
                }
            }

            size_t pathStart = url.find("/");
            if (pathStart != std::string::npos) {
                host = url.substr(0, pathStart);
                path = url.substr(pathStart);
            } else {
                host = url;
                path = "/";
            }

            size_t portStart = host.find(":");
            if (portStart != std::string::npos) {
                port = (INTERNET_PORT)atoi(host.substr(portStart + 1).c_str());
                host = host.substr(0, portStart);
            }

            // Initialize WinHTTP
            hSession = WinHttpOpen(xorstr_(L"SecureClient/1.0"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) {
                return "";
            }

            WinHttpSetTimeouts(hSession, 10000, 10000, 30000, 30000);

            // Convert to wide string
            std::wstring wHost(host.begin(), host.end());
            std::wstring wPath(path.begin(), path.end());

            // Connect to server
            hConnect = WinHttpConnect(hSession, wHost.c_str(), port, 0);
            if (!hConnect) {
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Create request
            hRequest = WinHttpOpenRequest(hConnect, L"GET", wPath.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
            if (!hRequest) {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Send request
            if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Wait for response
            if (!WinHttpReceiveResponse(hRequest, NULL)) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Check status code
            DWORD statusCode = 0;
            DWORD statusCodeSize = sizeof(statusCode);
            WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &statusCode, &statusCodeSize, NULL);
            
            if (statusCode >= 400) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Read response data
            DWORD bytesAvailable = 0;
            do {
                if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable)) {
                    break;
                }

                if (bytesAvailable == 0) {
                    break;
                }

                char* buffer = new char[bytesAvailable + 1];
                DWORD bytesRead = 0;
                if (WinHttpReadData(hRequest, buffer, bytesAvailable, &bytesRead)) {
                    buffer[bytesRead] = '\0';
                    response.append(buffer, bytesRead);
                }
                delete[] buffer;
            } while (bytesAvailable > 0);

            // Cleanup
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);

            return response;
        }
        catch (const std::exception& e) {
            if (hRequest) WinHttpCloseHandle(hRequest);
            if (hConnect) WinHttpCloseHandle(hConnect);
            if (hSession) WinHttpCloseHandle(hSession);
            return "";
        }
        catch (...) {
            if (hRequest) WinHttpCloseHandle(hRequest);
            if (hConnect) WinHttpCloseHandle(hConnect);
            if (hSession) WinHttpCloseHandle(hSession);
            return "";
        }
    }

    std::vector<unsigned char> HttpGetBinary(const std::string& URL) {
        if (URL.empty()) {
            return {};
        }

        std::vector<unsigned char> response;
        HINTERNET hSession = NULL;
        HINTERNET hConnect = NULL;
        HINTERNET hRequest = NULL;

        try {
            // Parse URL manually
            std::string url = URL;
            std::string protocol, host, path;
            INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT;
            DWORD flags = 0;

            size_t protocolEnd = url.find("://");
            if (protocolEnd != std::string::npos) {
                protocol = url.substr(0, protocolEnd);
                url = url.substr(protocolEnd + 3);
                if (protocol == "https") {
                    port = INTERNET_DEFAULT_HTTPS_PORT;
                    flags = WINHTTP_FLAG_SECURE;
                }
            }

            size_t pathStart = url.find("/");
            if (pathStart != std::string::npos) {
                host = url.substr(0, pathStart);
                path = url.substr(pathStart);
            } else {
                host = url;
                path = "/";
            }

            size_t portStart = host.find(":");
            if (portStart != std::string::npos) {
                port = (INTERNET_PORT)atoi(host.substr(portStart + 1).c_str());
                host = host.substr(0, portStart);
            }

            // Initialize WinHTTP
            hSession = WinHttpOpen(xorstr_(L"SecureClient/1.0"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) {
                return {};
            }

            WinHttpSetTimeouts(hSession, 10000, 10000, 30000, 30000);

            // Convert to wide string
            std::wstring wHost(host.begin(), host.end());
            std::wstring wPath(path.begin(), path.end());

            // Connect to server
            hConnect = WinHttpConnect(hSession, wHost.c_str(), port, 0);
            if (!hConnect) {
                WinHttpCloseHandle(hSession);
                return {};
            }

            // Create request
            hRequest = WinHttpOpenRequest(hConnect, L"GET", wPath.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
            if (!hRequest) {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return {};
            }

            // Send request
            if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return {};
            }

            // Wait for response
            if (!WinHttpReceiveResponse(hRequest, NULL)) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return {};
            }

            // Check status code
            DWORD statusCode = 0;
            DWORD statusCodeSize = sizeof(statusCode);
            WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &statusCode, &statusCodeSize, NULL);
            
            if (statusCode >= 400) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return {};
            }

            // Read response data
            DWORD bytesAvailable = 0;
            do {
                if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable)) {
                    break;
                }

                if (bytesAvailable == 0) {
                    break;
                }

                unsigned char* buffer = new unsigned char[bytesAvailable];
                DWORD bytesRead = 0;
                if (WinHttpReadData(hRequest, buffer, bytesAvailable, &bytesRead)) {
                    response.insert(response.end(), buffer, buffer + bytesRead);
                }
                delete[] buffer;
            } while (bytesAvailable > 0);

            // Cleanup
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);

            return response;
        }
        catch (const std::exception& e) {
            if (hRequest) WinHttpCloseHandle(hRequest);
            if (hConnect) WinHttpCloseHandle(hConnect);
            if (hSession) WinHttpCloseHandle(hSession);
            return {};
        }
        catch (...) {
            if (hRequest) WinHttpCloseHandle(hRequest);
            if (hConnect) WinHttpCloseHandle(hConnect);
            if (hSession) WinHttpCloseHandle(hSession);
            return {};
        }
    }
}

std::string GetHeadshotURL(std::uint64_t userID)
{
    if (userID == 0) {
        return "";
    }

    try {
        std::string apiUrl =
            xorstr_("https://thumbnails.roblox.com/v1/users/avatar-headshot?userIds=")
            + std::to_string(userID)
            + xorstr_("&size=420x420&format=Png&isCircular=false");

        std::string jsonResponse = HTTPTHINGS::SecureRequest(apiUrl);
        if (jsonResponse.empty()) {
            return "";
        }

        nlohmann::json parsed;
        try {
            parsed = nlohmann::json::parse(jsonResponse);
        }
        catch (const nlohmann::json::parse_error& e) {
            return "";
        }

        if (parsed.contains(xorstr_("data")) &&
            parsed[xorstr_("data")].is_array() &&
            !parsed[xorstr_("data")].empty() &&
            parsed[xorstr_("data")][0].contains(xorstr_("imageUrl"))) {

            return parsed[xorstr_("data")][0].value(xorstr_("imageUrl"), "");
        }

        return "";
    }
    catch (const std::exception& e) {
        return "";
    }
    catch (...) {
        return "";
    }
}

ID3D11ShaderResourceView* CreateTextureFromPNG(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    const std::vector<unsigned char>& pngData)
{
    if (!device || pngData.empty()) {
        return nullptr;
    }

    try {
        int width, height, channels;
        unsigned char* imageData = stbi_load_from_memory(
            pngData.data(),
            static_cast<int>(pngData.size()),
            &width,
            &height,
            &channels,
            4
        );

        if (!imageData || width <= 0 || height <= 0) {
            if (imageData) {
                stbi_image_free(imageData);
            }
            return nullptr;
        }

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = imageData;
        initData.SysMemPitch = width * 4;

        ID3D11Texture2D* texture = nullptr;
        HRESULT hr = device->CreateTexture2D(&desc, &initData, &texture);
        if (FAILED(hr)) {
            stbi_image_free(imageData);
            return nullptr;
        }

        ID3D11ShaderResourceView* srv = nullptr;
        hr = device->CreateShaderResourceView(texture, nullptr, &srv);

        if (texture) {
            texture->Release();
        }
        stbi_image_free(imageData);

        if (FAILED(hr)) {
            return nullptr;
        }

        return srv;
    }
    catch (const std::exception& e) {
        return nullptr;
    }
    catch (...) {
        return nullptr;
    }
}

ID3D11ShaderResourceView* LoadAvatarTexture(std::uint64_t userID)
{
    if (userID == 0) {
        return nullptr;
    }

    try {
        std::string imageUrl = GetHeadshotURL(userID);
        if (imageUrl.empty()) {
            return nullptr;
        }

        std::vector<unsigned char> pngData = HTTPTHINGS::HttpGetBinary(imageUrl);
        if (pngData.empty()) {
            return nullptr;
        }

       
        if (!Globals::g_pd3dDevice || !Globals::g_pd3dDeviceContext) {
            return nullptr;
        }

        ID3D11ShaderResourceView* avatarTexture = CreateTextureFromPNG(
            Globals::g_pd3dDevice,
            Globals::g_pd3dDeviceContext,
            pngData
        );

        return avatarTexture;
    }
    catch (const std::exception& e) {
        return nullptr;
    }
    catch (...) {
        return nullptr;
    }
}

void LoadPlayerAvatar(SDK::Cache::Player& Player)
{
    try {
      
        if (Player.PlayerName.empty()) {
            return;
        }

       
        for (const SDK::Cache::PlayerAvatar& AV : SDK::Cache::LoadedAvatars) {
            if (AV.PlayerName == Player.PlayerName) {
                return;
            }
        }

   
        if (!Player.PlayerObjectAddress) {
            return;
        }

       
        if (!SDK::Memory) {
            return;
        }

        std::uint64_t id = 0;
        try {
            id = SDK::Memory->Read<std::uint64_t>(Player.PlayerObjectAddress + SDK::Offsets::UserId);
        }
        catch (...) {
            return;
        }

        if (id == 0) {
            return;
        }

        SDK::Cache::PlayerAvatar A;
        A.PlayerName = Player.PlayerName;
        A.Texture = LoadAvatarTexture(id);

      
        if (A.Texture) {
            SDK::Cache::LoadedAvatars.push_back(A);
        }
    }
    catch (const std::exception& e) {
        if (Logging::Log) {
            Logging::Log(Logging::LogType::ERR, std::string("Exception while loading avatar: " + std::string(e.what())).c_str());
        }
    }
    catch (...) {
        if (Logging::Log) {
            Logging::Log(Logging::LogType::ERR, "Unknown exception while loading avatar");
        }
    }
}

void PlayerAvatarUpdate()
{
    while (true) {
        if (!Globals::DataModel.FindFirstChildOfClass("Workspace").Address)
            continue;
        try {
            if (SDK::Cache::Players.empty()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

          
            std::vector<SDK::Cache::Player> playersCopy;

            try {
                SDK::Cache::PlayersMutex.lock();
                playersCopy = SDK::Cache::Players;
                SDK::Cache::PlayersMutex.unlock();
            }
            catch (...) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            for (SDK::Cache::Player& Player : playersCopy) {
                try {
                    LoadPlayerAvatar(Player);
                }
                catch (const std::exception& e) {
                    if (Logging::Log) {
                        Logging::Log(Logging::LogType::ERR, std::string("Exception in LoadPlayerAvatar: " + std::string(e.what())).c_str());
                    }
                }
                catch (...) {
                    if (Logging::Log) {
                        Logging::Log(Logging::LogType::ERR, "Unknown exception in LoadPlayerAvatar");
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        catch (const std::exception& e) {
            if (Logging::Log) {
                Logging::Log(Logging::LogType::ERR, std::string("Exception in PlayerAvatarUpdate: " + std::string(e.what())).c_str());
            }
            std::this_thread::sleep_for(std::chrono::seconds(5)); 
        }
        catch (...) {
            if (Logging::Log) {
                Logging::Log(Logging::LogType::ERR, "Unknown exception in PlayerAvatarUpdate");
            }
            std::this_thread::sleep_for(std::chrono::seconds(5)); 
        }
    }
}
void SDK::Cache::PlayerInfoUpdate()
{
    std::thread(UpdatePositions).detach();
    std::thread(PlayerAvatarUpdate).detach();
    while (true)
    {
        if (!Globals::DataModel.FindFirstChildOfClass("Workspace").Address)
            continue;
        try
        {

            if (SDK::Cache::Players.size() == 0)
                std::this_thread::sleep_for(std::chrono::seconds(1));
            SDK::Cache::PlayersMutex.lock();
            for (SDK::Cache::Player& Player : SDK::Cache::Players)
            {
               
                if (Globals::GameID == SDK::Games::BadBusiness)
                {
                    Player.Health = static_cast<float>(SDK::Memory->Read<double>(Player.CharacterObject.FindFirstChild("Health").Address + SDK::Offsets::Value));
                    Player.MaxHealth = static_cast<float>(SDK::Memory->Read<double>(Player.CharacterObject.FindFirstChild("Health").FindFirstChild("MaxHealth").Address + SDK::Offsets::Value));
                }
                else if (Globals::GameID == SDK::Games::PhantomForces)
                {
                    Player.Health = 100.0f;
                    Player.MaxHealth = 100.0f;
                }
                else
                {

                    Player.Health = Player.Humanoid.Health();
                    Player.MaxHealth = Player.Humanoid.MaxHealth();
                    if (Player.CharacterObject.FindFirstChildOfClass("Tool").Address)
                        Player.Weapon = Player.CharacterObject.FindFirstChildOfClass("Tool").Name();
                    if (std::to_string(Globals::GameID) == "3747388906")
                    {

                        for (SDK::Instance Weapon : Player.CharacterObject.Children())
                        {
                            if (Weapon.Class() != "Model")
                                continue;
                            std::string Name = Weapon.Name();

                            if (Name.find("Armor") == std::string::npos && Name != "Hair" && Name != "HolsterModel")
                            {

                                Player.Weapon = Name;
                            }

                        }
                    }
                }
               
                
            }
            SDK::Cache::PlayersMutex.unlock();
        }
        catch (...)
        {
            Logging::Log(Logging::LogType::ERR, "Caught crash at cache info update");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }
}
