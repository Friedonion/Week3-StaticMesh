﻿#pragma once
#define _TCHAR_DEFINED
#include <Windows.h>
#include "ImGui/imgui.h"
#include "Core/Container/Set.h"
#include "Core/Container/Name.h"
#include "Static/Enum.h"

namespace EViewport{ enum class Position : uint8 ;}

class FName;
class AActor;
class URenderer;

enum SpawnType
{
	ESpawnSphere,
	ESpawnCube,
	ESpawnCylinder,
	ESpawnCone,
	ESpawnSpotlight,
	ESpawnTexture,
	ESpawnCustom,
};

class UI
{
    bool bShowUI = true; 

public:
    SpawnType CurrentPrimitiveItem = ESpawnCustom;
	std::string CurrentTextureItem = "Textures/cat.png";
	
    int NumOfSpawn = 1;
	bool bIsInitialized = false;
    
    void Initialize(HWND hWnd, URenderer& Renderer, UINT ScreenWidth, UINT ScreenHeight);
    void Update();
    void Shutdown();

	void OnUpdateWindowSize(UINT InScreenWidth, UINT InScreenHeight);

public:// UIWindows
    void RenderControlPanel();
    void RenderWorldGridSetting();
    void RenderMemoryUsage();
    void RenderPrimitiveSelection();
    void RenderCameraSettings();
	void RenderViewMode();
	void RenderShowFlag();
    void RenderPropertyWindow();
	void RenderSceneManager();
    void RenderComponentsByActor();
    void RenderFNameResolver();
    void RenderViewOption();

private:
	bool bWasWindowSizeUpdated = true;
    ImVec2 ScreenSize;
	ImVec2 InitialScreenSize;
	URenderer* Renderer;
    float windowWidth;
	float windowHeight;

    void SetWindowLayout(float widthRatio, float heightRatio, float posXRatio, float posYRatio);
    TArray<FName> Unselectables;
    void SetViewOption(const char* name, int &current_item,
        EViewport::Position viewportPos);
};
