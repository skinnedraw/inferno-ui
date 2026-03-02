#include "Explorer.h"
#include "../../Includes.hpp"
#include "ClassImages.h"
#include <unordered_map>
void RecursiveDraw(SDK::Instance instance, std::string parentPath);
ID3D11ShaderResourceView* ClassImagesPallete = nullptr;
bool FirstTime = true;
void Hacks::Explorer::Render()
{
    if (FirstTime)
    {
        int width, height, channels;
        unsigned char* imageData = stbi_load_from_memory(
            ClassImagesRawData,
            sizeof(ClassImagesRawData),
            &width,
            &height,
            &channels,
            4
        );
        if (imageData && width > 0 && height > 0) {
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
            if (SUCCEEDED(Globals::g_pd3dDevice->CreateTexture2D(&desc, &initData, &texture))) {
                Globals::g_pd3dDevice->CreateShaderResourceView(texture, nullptr, &ClassImagesPallete);
                if (texture) texture->Release();
            }
            stbi_image_free(imageData);
        }
        FirstTime = false;
    }
    RecursiveDraw(Globals::DataModel, "NULL");
}
void renderClassIcon(const char* className, ImVec2 size = ImVec2(16, 16)) {

    static std::unordered_map<std::string, int> classIconMap = {
     {"Default", 0},
     {"Part", 1},
     {"Model", 2},
     {"Model", 4},
     {"Camera", 5},
     {"Script", 6},
     {"ImageFrame", 7},
     {"Mesh", 8},
     {"SpecialMesh", 8},
     {"Humanoid", 9},
     {"Texture", 10},
     {"Sound", 11},
     {"Players", 21},
     {"SpotLight", 13},
     {"SurfaceLight", 14},
     {"RocketPropulsion", 15},
     {"Tool", 17},
     {"LocalScript", 18},
     {"Workspace", 19},
     {"Player", 12},
     {"Folder", 77},
     {"MeshPart", 1},


    };

    int iconIndex = 0;
    auto it = classIconMap.find(className);
    if (it != classIconMap.end()) {
        iconIndex = it->second;
    }
    else {

        iconIndex = classIconMap["Default"];
    }


    float iconWidth = 16.0f / (float)2352.0f;
    float startU = iconIndex * iconWidth;
    float endU = startU + iconWidth;


    ImGui::Image(
        (ImTextureID)(uintptr_t)ClassImagesPallete,
        size,
        ImVec2(startU, 0),
        ImVec2(endU, 1.0f)
    );
}
bool isValid(const std::string& str) {
    bool startsWithNumber = std::isdigit(str[0]);
    bool containsSymbol = false;

    for (char ch : str) {
        if (!std::isalnum(ch) && ch != '_') {
            containsSymbol = true;
            break;
        }
    }

    return startsWithNumber || containsSymbol;
}

void RecursiveDraw(SDK::Instance instance, std::string parentPath)
{
    std::string instancePath;
    std::string istanceName = instance.Name();
    if (parentPath == "NULL") {
        instancePath = "game:GetService(\"";
    }
    else {
        instancePath = (parentPath == "game:GetService(\"") ? parentPath + istanceName + "\")" : (!isValid(istanceName)) ? parentPath + "[\"" + istanceName + "\"]" : parentPath + "." + instance.Name();
    }


    auto children = instance.Children();


    if (!children.empty()) {
        std::string unique_id = istanceName + " [" + instance.Class() + "]" + "##" + std::to_string(instance.Address);


        renderClassIcon(instance.Class().c_str());

        ImGui::SameLine();
        if (ImGui::TreeNode(unique_id.c_str())) {

            for (SDK::Instance child : children) {
                RecursiveDraw(child, instancePath);
            }
            ImGui::TreePop();
        }
    }
    else {
        std::string unique_id = istanceName + " [" + instance.Class() + "]" + "##" + std::to_string(instance.Address);


        renderClassIcon(instance.Class().c_str());

        ImGui::SameLine();
        ImGui::Selectable(unique_id.c_str(), false);
    }


    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
    {
        std::string popup_id = "Context Menu##" + std::to_string(instance.Address);
        ImGui::OpenPopup(popup_id.c_str());
        if (ImGui::BeginPopup(popup_id.c_str()))
        {
            if (ImGui::MenuItem("Copy Path"))
            {
                ImGui::SetClipboardText(instancePath.c_str());
            }
            ImGui::EndPopup();
        }
    }
}
