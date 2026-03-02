#include "Esp.h"
#include "../../Includes.hpp"
#include <unordered_map>
ImColor FTC(float* color) {
    return ImColor(color[0], color[1], color[2], color[3]);
}
ImColor FTCNoAlpha(float* color) {
    return ImColor(color[0], color[1], color[2]);
}
struct PlayerESPState {
    float smoothHealth = 0.0f;
};

std::unordered_map<std::uintptr_t, PlayerESPState> playerESPStates;
struct HitMarker
{
    Hacks::Esp::HitMarker3D Start;
    Hacks::Esp::HitMarker3D End;
    std::chrono::steady_clock::time_point TimeInserted;
};
std::vector<HitMarker> RegisteredHits = {};
void Hacks::Esp::InsertHitMarker(HitMarker3D Start, HitMarker3D End)
{
    HitMarker Hit;
    SDK::Vector2 One = SDK::WorldToScreen(SDK::Vector3(Start.x, Start.y, Start.z));
    SDK::Vector2 Two = SDK::WorldToScreen(SDK::Vector3(End.x, End.y, End.z));
    Hit.Start = Start;
    Hit.End = End;
    Hit.TimeInserted = std::chrono::steady_clock::now();
    RegisteredHits.push_back(Hit);
}
void RenderHits(ImDrawList* Draw)
{
    if (RegisteredHits.size() == 0)
        return;

    std::chrono::steady_clock::time_point CurrentTime = std::chrono::steady_clock::now();

    for (int i = 0; i < RegisteredHits.size(); ++i)
    {
        std::chrono::milliseconds ElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - RegisteredHits[i].TimeInserted);
        if (ElapsedTime.count() >= 10000) {
            RegisteredHits.erase(RegisteredHits.begin() + i);
            --i;
            continue;
        }
        SDK::Vector2 One = SDK::WorldToScreen(SDK::Vector3(RegisteredHits[i].Start.x, RegisteredHits[i].Start.y, RegisteredHits[i].Start.z));
        SDK::Vector2 Two = SDK::WorldToScreen(SDK::Vector3(RegisteredHits[i].End.x, RegisteredHits[i].End.y, RegisteredHits[i].End.z));
        if (One.x == -1 || Two.x == -1)
            continue;
        if (Globals::Aimbot::Hits::HitTracers)
        {
            Draw->AddLine(
                { One.x, One.y },
                { Two.x, Two.y },
                ImColor(0, 0, 0),
                3.0f
            );
            Draw->AddLine(
                { One.x, One.y },
                { Two.x, Two.y },
                FTCNoAlpha(Globals::Aimbot::Hits::HitTracersColor),
                2.0f
            );
        }
        float gap = 2.0f;
        float half = 6.0f;

        if (Globals::Aimbot::Hits::HitMarkers)
        {
            Draw->AddLine(
                { Two.x - half, Two.y - half },
                { Two.x - gap,  Two.y - gap },
                ImColor(0, 0, 0),
                2.0f
            );
            Draw->AddLine(
                { Two.x - half, Two.y - half },
                { Two.x - gap,  Two.y - gap },
                FTCNoAlpha(Globals::Aimbot::Hits::HitMarkersColor)
            );
            Draw->AddLine(
                { Two.x + gap,  Two.y + gap },
                { Two.x + half, Two.y + half },
                ImColor(0, 0, 0),
                2.0f
            );
            Draw->AddLine(
                { Two.x + gap,  Two.y + gap },
                { Two.x + half, Two.y + half },
                FTCNoAlpha(Globals::Aimbot::Hits::HitMarkersColor)
            );
            Draw->AddLine(
                { Two.x - half, Two.y + half },
                { Two.x - gap,  Two.y + gap },
                ImColor(0, 0, 0),
                2.0f
            );
            Draw->AddLine(
                { Two.x - half, Two.y + half },
                { Two.x - gap,  Two.y + gap },
                FTCNoAlpha(Globals::Aimbot::Hits::HitMarkersColor)
            );
            Draw->AddLine(
                { Two.x + gap,  Two.y - gap },
                { Two.x + half, Two.y - half },
                ImColor(0, 0, 0),
                2.0f
            );
            Draw->AddLine(
                { Two.x + gap,  Two.y - gap },
                { Two.x + half, Two.y - half },
                FTCNoAlpha(Globals::Aimbot::Hits::HitMarkersColor)
            );
        }
    }
}
void Hacks::Esp::Render()
{
    if (!Globals::DataModel.FindFirstChildOfClass("Workspace").Address)
        return;
    POINT cursor_point;
    GetCursorPos(&cursor_point);
    ScreenToClient(Globals::RobloxWindow, &cursor_point);
    ImDrawList* Draw = ImGui::GetBackgroundDrawList();
    if (!Draw) return;

    Draw->Flags = ImDrawFlags_None | ImDrawListFlags_AntiAliasedLines;
    RenderHits(Draw);
    if (SDK::Cache::Waypoints.size() != 0)
    {
        for (SDK::Cache::WayPoint& W : SDK::Cache::Waypoints)
        {
            SDK::Vector2 Screen = SDK::WorldToScreen(W.WorldPosition);
            W.ScreenPosition = { Screen.x, Screen.y };
            if (Globals::World::Waypoints::WaypointName)
            {
                Draw->AddText(
                    W.ScreenPosition,
                    FTC(Globals::World::Waypoints::Colors::WaypointName),
                    W.Name.c_str()
                );
            }
            if (Globals::World::Waypoints::WaypointDistance)
            {
                Draw->AddText(
                    W.ScreenPosition + ImVec2(0,1 + ImGui::CalcTextSize((std::to_string((int)SDK::Cache::LocalPlayer.HumanoidRootPart.WorldPosition.distance(W.WorldPosition)) + "m").c_str()).y),
                    FTC(Globals::World::Waypoints::Colors::WaypointDistance),
                    (std::to_string((int)SDK::Cache::LocalPlayer.HumanoidRootPart.WorldPosition.distance(W.WorldPosition)) + "m").c_str()
                );
            }
        }
    }
    if (Globals::Aimbot::DrawFOV)
    {
        Draw->AddCircle(
            { static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) },
            Globals::Aimbot::FOV,
            ImColor(0, 0, 0),
            0.0f,
            3.0f
        );
        Draw->AddCircle(
            { static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) },
            Globals::Aimbot::FOV,
            FTCNoAlpha(Globals::Aimbot::FovColor),
            0.0f,
            2.0f
        );
        if (Globals::Aimbot::FovSettings::FillFov)
        {
            Draw->AddCircleFilled({ static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) }, Globals::Aimbot::FOV - 1, FTC(Globals::Aimbot::FovSettings::FillColor));
        }
    }
    if (Globals::Misc::DesyncedPostion != SDK::Vector3(-1, -1, -1))
    {
        SDK::Vector2 S = SDK::WorldToScreen(Globals::Misc::DesyncedPostion);
        Draw->AddText({ S.x, S.y }, ImColor(255, 255, 255), "Desync");
    }
    if (!Globals::Esp::Enabled)
        return;
    SDK::Cache::PlayersMutex.lock();
    for (auto& Player : SDK::Cache::Players)
    {
        if (!Player.CharacterObjectAddress ||
            
            Player.Head.ScreenPosition.x == -1 ||
            Player.Health <= 0)
            continue;
        if (!Globals::Esp::LocalPlayerESP && Player.PlayerObjectAddress == SDK::Cache::LocalPlayer.PlayerObjectAddress)
            continue;
        
        bool InVisible = false;
        for (const auto& Bone : Player.Bones)
        {
            if (Bone.ScreenPosition.x == -1 || Bone.ScreenPosition.x == 0 || Bone.ScreenPosition.x < 50)
            {
                InVisible = true;
                break;
            }
        }

        if (InVisible)
            continue;
        if (Globals::Esp::MaxDistance && Player.Distance > Globals::Esp::MaxDistanceValue)
            continue;
        if (Globals::Esp::Box)
        {
            Draw->AddRect(Player.BoxMin, Player.BoxMax, FTC(Globals::Esp::Colors::Box));
            Draw->AddRect(Player.BoxMin + ImVec2(1, 1), Player.BoxMax - ImVec2(1, 1), ImColor(0, 0, 0));
            Draw->AddRect(Player.BoxMin - ImVec2(1, 1), Player.BoxMax + ImVec2(1, 1), ImColor(0, 0, 0));
            
        }
        if (Globals::Esp::BoxGlow) {
            Draw->AddShadowRect(Player.BoxMin + ImVec2(2, 2), Player.BoxMax - ImVec2(2, 2), FTC(Globals::Esp::Colors::BoxGlow), 50.0f, { 0,0 });
        }
        
        if (Globals::Esp::Tracers)
        {
            ImVec2 StartPostion = { static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) };
            if (Globals::Esp::TracersPosition == 2)
            {
                StartPostion = ImVec2(SDK::VisualEngineCache::Dimensions.x / 2, 0);
            }
            if (Globals::Esp::TracersPosition == 1)
            {
                StartPostion = ImVec2(SDK::VisualEngineCache::Dimensions.x / 2, SDK::VisualEngineCache::Dimensions.y);
            }
            Draw->AddLine(
                { Player.Head.ScreenPosition.x,Player.Head.ScreenPosition.y },
                StartPostion,
                ImColor(0,0,0),
                3.0f
            );
            Draw->AddLine(
                { Player.Head.ScreenPosition .x,Player.Head.ScreenPosition .y},
                StartPostion,
                FTC(Globals::Esp::Colors::Tracers),
                2.0f
            );
        }
        


        if (Globals::Esp::Name)
        {
            std::string Name = Globals::Esp::UseDisplayName ? Player.DisplayName : Player.PlayerName;
            if (!Name.empty())
            {
                ImVec2 Size = ImGui::CalcTextSize(Name.c_str());
                ImVec2 NamePosition = ImVec2(Player.BoxMin.x + (Player.BoxMax.x - Player.BoxMin.x) / 2 - Size.x / 2, Player.BoxMin.y - 1 - Size.y);
                
                Draw->AddText(NamePosition, FTC(Globals::Esp::Colors::Name), Name.c_str());
            }
        }
        if (Globals::Esp::Distance)
        {
            std::string DistanceText = std::to_string(Player.Distance) + "m";
            ImVec2 Size = ImGui::CalcTextSize(DistanceText.c_str());
            ImVec2 TextPosition = ImVec2(Player.BoxMin.x + (Player.BoxMax.x - Player.BoxMin.x) / 2 - Size.x / 2, Player.BoxMax.y + 1);

            if (Globals::Esp::Tool && !Player.Weapon.empty()) {
                TextPosition = ImVec2(Player.BoxMin.x + (Player.BoxMax.x - Player.BoxMin.x) / 2 - Size.x / 2, Player.BoxMax.y + ImGui::CalcTextSize(Player.Weapon.c_str()).y + 2);
            }

            Draw->AddText(TextPosition, FTC(Globals::Esp::Colors::Distance), DistanceText.c_str());
        }
        if (Globals::Esp::Tool && !Player.Weapon.empty())
        {
            ImVec2 Size = ImGui::CalcTextSize(Player.Weapon.c_str());
            ImVec2 TextPosition = ImVec2(Player.BoxMin.x + (Player.BoxMax.x - Player.BoxMin.x) / 2 - Size.x / 2, Player.BoxMax.y + 1);
            Draw->AddText(TextPosition, FTC(Globals::Esp::Colors::Tool), Player.Weapon.c_str());
        }
        if (Globals::Esp::Avatar)
        {
            for (SDK::Cache::PlayerAvatar Avatar : SDK::Cache::LoadedAvatars)
            {
                if (Avatar.PlayerName == Player.PlayerName)
                {
                    int iconSize = Globals::Esp::IconSize;
                    float offsetAboveBox = Globals::Esp::OffsetOffBoxAvatar;

                    float centerX = (Player.BoxMin.x + Player.BoxMax.x) / 2.0f;

                    ImVec2 iconMin = ImVec2(
                        centerX - iconSize / 2.0f,
                        Player.BoxMin.y - iconSize - offsetAboveBox
                    );

                    ImVec2 iconMax = ImVec2(
                        centerX + iconSize / 2.0f,
                        Player.BoxMin.y - offsetAboveBox
                    );

                    ImGui::GetBackgroundDrawList()->AddImage(
                        (ImTextureID)Avatar.Texture,
                        iconMin,
                        iconMax
                    );
                    break;
                }
            }
        }
        if (Globals::Esp::Chams || Globals::Esp::ChamsOutlines || Globals::Esp::ChamsGlow)
        {
            if (Globals::Esp::ChamsType == 0)
            {
                std::vector<std::string> KnownChams = {

                    "Right Arm", "Left Arm", "Right Leg", "Left Leg", "Head", "Torso",

                    "Head",
                    "UpperTorso", "LowerTorso",
                    "LeftUpperArm", "LeftLowerArm", "LeftHand",
                    "RightUpperArm", "RightLowerArm", "RightHand",
                    "LeftUpperLeg", "LeftLowerLeg", "LeftFoot",
                    "RightUpperLeg", "RightLowerLeg", "RightFoot"
                };

                for (const auto& Bone : Player.Bones)
                {
                    if (Globals::Esp::FilterUnknownChamsParts)
                    {
                        bool ShouldContinue = true;
                        for (std::string Known : KnownChams)
                        {
                            if (Known == Bone.Name)
                                ShouldContinue = false;
                        }
                        if (ShouldContinue)
                            continue;
                    }

                    ImVec2* Hull = Bone.Hull.Hull;
                    int HullSize = Bone.Hull.HullSize;
                    if (!Hull || HullSize <= 0 || Bone.Name == "HumanoidRootPart")
                        continue;
                    if (Globals::Esp::ChamsOutlines)
                    {
                        for (int i = 0; i < HullSize; i++)
                        {
                            if (!Hull[i].x)
                                continue;
                            Draw->AddLine(Hull[i], Hull[(i + 1) % HullSize], FTC(Globals::Esp::Colors::ChamsOutlines), Globals::Esp::ChamsOutlineThickness);
                        }
                    }
                }
                for (const auto& Bone : Player.Bones)
                {
                    if (Globals::Esp::FilterUnknownChamsParts)
                    {
                        bool ShouldContinue = true;
                        for (std::string Known : KnownChams)
                        {
                            if (Known == Bone.Name)
                                ShouldContinue = false;
                        }
                        if (ShouldContinue)
                            continue;
                    }
                    ImVec2* Hull = Bone.Hull.Hull;
                    int HullSize = Bone.Hull.HullSize;
                    if (!Hull || HullSize <= 0 || Bone.Name == "HumanoidRootPart")
                        continue;



                    if (Globals::Esp::Chams)
                    {
                        ImColor color = FTC(Globals::Esp::Colors::Chams);
                        if (Globals::Esp::InheritBodyColorChams)
                        {
                            ImU32 V = SDK::Memory->Read<ImU32>(Bone.Object.Address + SDK::Offsets::MeshPartColor3);
                            ImVec4 codlor = ImGui::ColorConvertU32ToFloat4(V);
                            color = ImColor(codlor.x, codlor.y, codlor.z, Globals::Esp::Colors::Chams[3]);
                        }
                        Draw->AddConvexPolyFilled(Hull, HullSize, color);
                    }
                    if (Globals::Esp::ChamsGlow)
                    {
                        Draw->AddShadowConvexPoly(Hull, HullSize, FTC(Globals::Esp::Colors::ChamsGlow), 30.0f, { 0,0 });
                    }
                }
            }
            else if (Globals::Esp::ChamsType == 1)
            {
               
                if (Globals::Esp::Chams)
                {
                    for (const auto& path : Player.HighlightSolution) {
                        if (path.size() < 2) continue;

                        std::vector<ImVec2> points;
                        points.reserve(path.size());
                        for (auto& p : path)
                            points.push_back(ImVec2((float)p.x, (float)p.y));
                        ImGui::GetBackgroundDrawList()->AddConcavePolyFilled(points.data(), (int)points.size(), FTC(Globals::Esp::Colors::Chams));
                    }
                }
                
                if (Globals::Esp::ChamsOutlines)
                {
                    for (const auto& path : Player.HighlightSolution) {
                        if (path.size() < 2) continue;

                        std::vector<ImVec2> points;
                        points.reserve(path.size());
                        for (auto& p : path)
                            points.push_back(ImVec2((float)p.x, (float)p.y));
                        ImGui::GetBackgroundDrawList()->AddPolyline(points.data(), (int)points.size(), FTC(Globals::Esp::Colors::ChamsOutlines), ImDrawFlags_Closed, Globals::Esp::ChamsOutlineThickness);
                    }
                }
            }
            
            
            
        }
        if (Globals::Esp::Skeleton)
        {
            if (Player.RigType == 0)
            {
                ImVec2 Head, Torso, RightArm, LeftArm, RightLeg, LeftLeg;
                for (SDK::Cache::Bone Bone : Player.Bones)
                {
                    if (Bone.Name == "Head")
                        Head = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "Torso")
                        Torso = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "Left Leg")
                        LeftLeg = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "Right Leg")
                        RightLeg = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "Right Arm")
                        RightArm = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "Left Arm")
                        LeftArm = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                }

                if (Head.x != -1 && Torso.x != -1)
                    Draw->AddLine(Head, Torso, FTC(Globals::Esp::Colors::Skeleton));
                if (RightArm.x != -1 && Torso.x != -1)
                    Draw->AddLine(RightArm, Torso, FTC(Globals::Esp::Colors::Skeleton));
                if (LeftArm.x != -1 && Torso.x != -1)
                    Draw->AddLine(LeftArm, Torso, FTC(Globals::Esp::Colors::Skeleton));
                if (RightLeg.x != -1 && Torso.x != -1)
                    Draw->AddLine(RightLeg, Torso, FTC(Globals::Esp::Colors::Skeleton));
                if (LeftLeg.x != -1 && Torso.x != -1)
                    Draw->AddLine(LeftLeg, Torso, FTC(Globals::Esp::Colors::Skeleton));
            }

            if (Player.RigType == 1)
            {
                ImVec2 Head, UpperTorso, LowerTorso;
                ImVec2 LeftUpperArm, LeftLowerArm, LeftHand;
                ImVec2 RightUpperArm, RightLowerArm, RightHand;
                ImVec2 LeftUpperLeg, LeftLowerLeg, LeftFoot;
                ImVec2 RightUpperLeg, RightLowerLeg, RightFoot;

                for (SDK::Cache::Bone Bone : Player.Bones)
                {
                    if (Bone.Name == "Head")
                        Head = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "UpperTorso")
                        UpperTorso = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "LowerTorso")
                        LowerTorso = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };

                    if (Bone.Name == "LeftUpperArm")
                        LeftUpperArm = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "LeftLowerArm")
                        LeftLowerArm = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "LeftHand")
                        LeftHand = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };

                    if (Bone.Name == "RightUpperArm")
                        RightUpperArm = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "RightLowerArm")
                        RightLowerArm = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "RightHand")
                        RightHand = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };

                    if (Bone.Name == "LeftUpperLeg")
                        LeftUpperLeg = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "LeftLowerLeg")
                        LeftLowerLeg = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "LeftFoot")
                        LeftFoot = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };

                    if (Bone.Name == "RightUpperLeg")
                        RightUpperLeg = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "RightLowerLeg")
                        RightLowerLeg = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                    if (Bone.Name == "RightFoot")
                        RightFoot = { Bone.ScreenPosition.x, Bone.ScreenPosition.y };
                }

                auto Line = [&](ImVec2 a, ImVec2 b)
                    {
                        if (a.x != -1 && b.x != -1)
                            Draw->AddLine(a, b, FTC(Globals::Esp::Colors::Skeleton));
                    };

                Line(Head, UpperTorso);
                Line(UpperTorso, LowerTorso);

                Line(UpperTorso, LeftUpperArm);
                Line(LeftUpperArm, LeftLowerArm);
                Line(LeftLowerArm, LeftHand);

                Line(UpperTorso, RightUpperArm);
                Line(RightUpperArm, RightLowerArm);
                Line(RightLowerArm, RightHand);

                Line(LowerTorso, LeftUpperLeg);
                Line(LeftUpperLeg, LeftLowerLeg);
                Line(LeftLowerLeg, LeftFoot);

                Line(LowerTorso, RightUpperLeg);
                Line(RightUpperLeg, RightLowerLeg);
                Line(RightLowerLeg, RightFoot);
            }
        } // shit code
        if (Globals::Esp::HealthBar) {
            float rawHealth = Player.Health;
            float playerMaxHealth = Player.MaxHealth;
            
            PlayerESPState& state = playerESPStates[Player.PlayerObjectAddress];
            state.smoothHealth += (rawHealth - state.smoothHealth) * 0.01f;
            state.smoothHealth = ImClamp(state.smoothHealth, 0.0f, playerMaxHealth);
            double playerHealth = state.smoothHealth;
            double healthPercentage = (playerMaxHealth > 0.0) ? (playerHealth / playerMaxHealth) : 0.0;
            float healthBarHeight = Player.BoxMax.y - Player.BoxMin.y;
            float filledHealthBarHeight = healthBarHeight * healthPercentage;
            float emptyBarHeight = healthBarHeight - filledHealthBarHeight;
            float filledHealthMaxY = Player.BoxMin.y + emptyBarHeight;
            ImVec2 health_min = ImVec2(std::floor(Player.BoxMin.x - 5.0f), std::floor(filledHealthMaxY));
            ImVec2 health_max = ImVec2(std::floor(Player.BoxMin.x - 3.0f), std::floor(Player.BoxMax.y));

            if (Globals::Esp::HealthBarBackGround) {
                Draw->AddRectFilled({ health_min.x, Player.BoxMin.y }, { health_max.x, health_max.y }, ImColor(0, 0, 0, 187));
            }

            ImColor Color = FTC(Globals::Esp::Colors::HealthBar);
            if (Globals::Esp::HealthBarColorType == 1) {
                Color = ImColor(94, 235, 52);
                if (healthPercentage < 0.2f) Color = ImColor(235, 52, 52);
                else if (healthPercentage < 0.4f) Color = ImColor(235, 168, 52);
                else if (healthPercentage < 0.7f) Color = ImColor(192, 235, 52);
            }

            Draw->AddRectFilled(health_min, health_max, Color);

            if (Globals::Esp::HealthBarGlow) {
                Draw->AddShadowRect(health_min, health_max, Color, 50.0f, { 0,0 });
            }

            
             if (Globals::Esp::HealthBarBackGround) {
                 Draw->AddRect({ health_min.x - 1, Player.BoxMin.y - 1 }, { health_max.x + 1, health_max.y + 1 }, ImColor(0, 0, 0));
             }
             else {
                 Draw->AddRect(health_min - ImVec2(1, 1), health_max + ImVec2(1, 1), ImColor(0, 0, 0));
             }
            

            if (Globals::Esp::HealthNumber && (int)std::ceil(playerHealth) != playerMaxHealth) {
                std::string healthText = std::to_string((int)std::ceil(playerHealth)) + "%";
                ImVec2 healthTextPos = health_min - ImVec2(ImGui::CalcTextSize(healthText.c_str()).x - 2, 4);

               
                Draw->AddText(healthTextPos, FTC(Globals::Esp::Colors::HealthNumber), healthText.c_str());
                
            }
        }
        {
            int offset = 0;
            
            for (SDK::Cache::Flag Flag : Player.Flags) {
                Draw->AddText(ImVec2(Player.BoxMax.x + 3, Player.BoxMin.y + offset), Flag.Color, Flag.Flag.c_str());
                offset += ImGui::CalcTextSize(Flag.Flag.c_str()).y + 2;
            }
        }
    }
    SDK::Cache::PlayersMutex.unlock();
}
