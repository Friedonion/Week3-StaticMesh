﻿#include "UI.h"

#include <algorithm>

#include "Object/Actor/Camera.h"
#include "URenderer.h"
#include "Core/HAL/PlatformMemory.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "Debug/DebugConsole.h"
#include "ImGui/imgui_internal.h"
#include "Object/Actor/Actor.h"
#include "Object/Actor/Sphere.h"
#include "Object/Actor/Cube.h"
#include "Object/Actor/Arrow.h"
#include "Object/Actor/Cone.h"
#include "Object/Actor/Cylinder.h"
#include "Object/Actor/WorldGrid.h"
#include "Object/Actor/Spotlight.h"
#include "Object/Actor/Texture.h"
#include "Static/FEditorManager.h"
#include "Object/World/World.h"
#include "Object/Gizmo/GizmoHandle.h"
#include "Core/Rendering/Particle/Particle.h"
#include "Object/Actor/StaticMesh.h"
#include "Object/Gizmo/Axis.h"
#include "Object/PrimitiveComponent/TextureComponent.h"
#include "Static/Enum.h"
#include "Core//Input/PlayerInput.h"
#include "Object/TObjectIterator.h"
#include "Static/ResourceManager.h"
#include "Static/Util.h"

void UI::Initialize(HWND hWnd, URenderer& Renderer, UINT ScreenWidth, UINT ScreenHeight)
{
    // ImGui 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;




    // 기본 폰트 크기 설정
    io.FontGlobalScale = 1.0f;
    io.DisplaySize = ScreenSize;
    //io.WantSetMousePos = true;
    // ImGui Backend 초기화
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(Renderer.GetDevice(), Renderer.GetDeviceContext());

	ScreenSize = ImVec2(static_cast<float>(ScreenWidth), static_cast<float>(ScreenHeight));
    InitialScreenSize = ScreenSize;
    bIsInitialized = true;
    
    io.DisplaySize = ScreenSize;
    this->Renderer = &Renderer;

    Unselectables.Add(ACamera::StaticClass()->GetName());
    Unselectables.Add(AWorldGrid::StaticClass()->GetName());
    Unselectables.Add(AAxis::StaticClass()->GetName());
    Unselectables.Add(APicker::StaticClass()->GetName());
    Unselectables.Add(AGizmoHandle::StaticClass()->GetName());
}

void UI::Update()
{
    if (!bIsInitialized) return;
    POINT mousePos;
    if (GetCursorPos(&mousePos)) {
        HWND hwnd = GetActiveWindow();
        ScreenToClient(hwnd, &mousePos);
    }

    // ImGui Frame 생성
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    windowWidth = UEngine::Get().GetScreenWidth();
    windowHeight = UEngine::Get().GetScreenHeight();

    if (APlayerInput::Get().GetKeyDown(EKeyCode::F1))
        bShowUI = !bShowUI;

    if (bShowUI)
    {
    
    RenderControlPanel();
    RenderPropertyWindow();
    RenderSceneManager();
    RenderComponentsByActor();
    RenderFNameResolver();
    RenderViewOption();
    Debug::ShowConsole(bWasWindowSizeUpdated);
}

    // ImGui 렌더링
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }


    bWasWindowSizeUpdated = false;
}

void UI::Shutdown()
{
    if (!bIsInitialized) return;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    bIsInitialized = false;
}

void UI::OnUpdateWindowSize(UINT InScreenWidth, UINT InScreenHeight)
{
    // ImGUI 리소스 다시 생성
    ImGui_ImplDX11_InvalidateDeviceObjects();
    ImGui_ImplDX11_CreateDeviceObjects();
   // ImGui 창 크기 업데이트
	ScreenSize = ImVec2(static_cast<float>(InScreenWidth), static_cast<float>(InScreenHeight));

    bWasWindowSizeUpdated = true;
}

void UI::RenderControlPanel()
{
    SetWindowLayout(0.4f, 0.3f, 0.f, 0.f);
    ImGui::Begin("Jungle Control Panel");
    ImGui::Text("Hello, Jungle World!");
    ImGui::Text("FPS: %.3f (what is that ms)", ImGui::GetIO().Framerate);

    RenderMemoryUsage();
    RenderPrimitiveSelection();
    RenderCameraSettings();
    RenderWorldGridSetting();
    RenderViewMode();
    RenderShowFlag();
    
    ImGui::End();
}

void UI::RenderWorldGridSetting()
{
    AWorldGrid* WorldGrid = FEditorManager::Get().GetWorldGrid();
    
    if (WorldGrid == nullptr)
        return;

    float Spacing = WorldGrid->GetSpacing();
    
    if (ImGui::DragFloat("Grid Spacing", &Spacing, 0.1f))
    {
        Spacing = std::clamp(Spacing, 0.f, 10.f);

        WorldGrid->SetSpacing(Spacing);
    }
    
    ImGui::Separator();
}

void UI::RenderMemoryUsage()
{
    const uint64 ContainerAllocByte = FPlatformMemory::GetAllocationBytes<EAT_Container>();
    const uint64 ContainerAllocCount = FPlatformMemory::GetAllocationCount<EAT_Container>();
    const uint64 ObjectAllocByte = FPlatformMemory::GetAllocationBytes<EAT_Object>();
    const uint64 ObjectAllocCount = FPlatformMemory::GetAllocationCount<EAT_Object>();
    ImGui::Text(
        "Container Memory Uses: %llubyte, Count: %llu",
        ContainerAllocByte,
        ContainerAllocCount
    );
    ImGui::Text(
        "Object Memory Uses: %llubyte, Count: %llu Objects",
        ObjectAllocByte,
        ObjectAllocCount
    );
    ImGui::Text(
        "Total Memory Uses: %llubyte, Count: %llu",
        ContainerAllocByte + ObjectAllocByte,
        ContainerAllocCount + ObjectAllocCount
    );

    ImGui::Separator();
}

void UI::RenderPrimitiveSelection()
{
    const char* PrimitiveItems[] = {"Sphere", "Cube", "Cylinder", "Cone", "Spotlight", "Texture", "Custom"};

    ImGui::Combo("Primitive", reinterpret_cast<int*>(&CurrentPrimitiveItem), PrimitiveItems, ARRAYSIZE(PrimitiveItems));

    if (ImGui::Button("Spawn"))
    {
        UParticle::Get().Reload(Renderer->GetDevice(), Renderer->GetDeviceContext());
        UWorld* World = UEngine::Get().GetWorld();

        for (int i = 0; i < NumOfSpawn; i++)
        {
            switch (CurrentPrimitiveItem)
            {
            case ESpawnSphere:
                World->SpawnActor<ASphere>();
                break;
            case ESpawnCube:
                World->SpawnActor<ACube>();
                break;
            case ESpawnCylinder:
                World->SpawnActor<ACylinder>();
                break;
            case ESpawnCone:
                World->SpawnActor<ACone>();
                break;
            case ESpawnSpotlight:
                World->SpawnActor<ASpotlight>();
                break;
            case ESpawnTexture:
                World->SpawnActor<ATexture>();
                    break;
            case ESpawnCustom:
                {
                    AStaticMesh* StaticMeshActor = World->SpawnActor<AStaticMesh>();
                    StaticMeshActor->SetObjName("pirate");
                    break;
                }
            default:
                break;
            }
        }
    }
    
    ImGui::SameLine();
    ImGui::InputInt("Number of spawn", &NumOfSpawn, 0);

    ImGui::Separator();

    UWorld* World = UEngine::Get().GetWorld();
    uint32 bufferSize = 100;
    char* SceneNameInput = new char[bufferSize];
    strcpy_s(SceneNameInput, bufferSize, *World->SceneName);
    
	if (ImGui::InputText("Scene Name", SceneNameInput, bufferSize))
	{
		World->SceneName = SceneNameInput;
	}
    
    if (ImGui::Button("New Scene"))
    {
        World->ClearWorld();
    }
    if (ImGui::Button("Save Scene"))
    {
        World->SaveWorld(*World->SceneName);   
    }
    if (ImGui::Button("Load Scene"))
    {
        World->LoadWorld(SceneNameInput);
    }
    ImGui::Separator();
}

void UI::RenderCameraSettings()
{
    ImGui::Text("Camera");

    ACamera* Camera = FEditorManager::Get().GetCamera();

    bool IsOrthogonal;
    if (Camera->ProjectionMode == ECameraProjectionMode::Orthographic)
    {
        IsOrthogonal = true;
    }
    else if (Camera->ProjectionMode == ECameraProjectionMode::Perspective)
    {
        IsOrthogonal = false;
    }

    if (ImGui::Checkbox("Orthogonal", &IsOrthogonal))
    {
        if (IsOrthogonal)
        {
            Camera->ProjectionMode = ECameraProjectionMode::Orthographic;
        }
        else
        {
            Camera->ProjectionMode = ECameraProjectionMode::Perspective;
        }
    }

    float FOV = Camera->GetFieldOfView();
    if (ImGui::DragFloat("FOV", &FOV, 0.1f))
    {
        FOV = std::clamp(FOV, 0.01f, 179.99f);
        Camera->SetFieldOfView(FOV);
    }

    float NearFar[2] = { Camera->GetNear(), Camera->GetFar() };
    if (ImGui::DragFloat2("Near, Far", NearFar, 0.1f))
    {
        NearFar[0] = FMath::Max(0.01f, NearFar[0]);
        NearFar[1] = FMath::Max(0.01f, NearFar[1]);

        if (NearFar[0] < NearFar[1])
        {
            Camera->SetNear(NearFar[0]);
            Camera->SetFar(NearFar[1]);
        }
        else
        {
            if (abs(NearFar[0] - Camera->GetNear()) < 0.00001f)
            {
                Camera->SetFar(NearFar[0] + 0.01f);
            }
            else if (abs(NearFar[1] - Camera->GetFar()) < 0.00001f)
            {
                Camera->SetNear(NearFar[1] - 0.01f);
            }
        }
    }
    
    FVector CameraPosition = Camera->GetActorRelativeTransform().GetPosition();
    if (ImGui::DragFloat3("Camera Location", reinterpret_cast<float*>(&CameraPosition), 0.1f))
    {
        FTransform Trans = Camera->GetActorRelativeTransform();
        Trans.SetPosition(CameraPosition);
        Camera->SetActorRelativeTransform(Trans);
    }

    FVector PrevEulerAngle = Camera->GetActorRelativeTransform().GetRotation().GetEuler();
    FVector UIEulerAngle = { PrevEulerAngle.X, PrevEulerAngle.Y, PrevEulerAngle.Z };
    if (ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float*>(&UIEulerAngle), 0.1f))
    {
        FTransform Transform = Camera->GetActorRelativeTransform();

        //FVector DeltaEulerAngle = UIEulerAngle - PrevEulerAngle;
        //Transform.Rotate(DeltaEulerAngle);
        
        UIEulerAngle.Y = FMath::Clamp(UIEulerAngle.Y, -Camera->MaxYDegree, Camera->MaxYDegree);
        Transform.SetRotation(UIEulerAngle);
        Camera->SetActorRelativeTransform(Transform);
    }
    if (ImGui::DragFloat("Camera Speed", &Camera->CameraSpeed, 0.1f))
    {
        Camera->CameraSpeed = FMath::Clamp(Camera->CameraSpeed, .0f, 20.0f);
    }

    float CameraSensitivity = Camera->GetCameraSensitivity();
    if (ImGui::DragFloat("Camera Sensitivity", &CameraSensitivity, 0.1f))
    {
        Camera->SetCameraSensitivity(CameraSensitivity);
    }

    FVector Forward = Camera->GetActorRelativeTransform().GetForward();
    FVector Up = Camera->GetActorRelativeTransform().GetUp();
    FVector Right = Camera->GetActorRelativeTransform().GetRight();

    ImGui::Text("Camera GetForward(): (%.2f %.2f %.2f)", Forward.X, Forward.Y, Forward.Z);
    ImGui::Text("Camera GetUp(): (%.2f %.2f %.2f)", Up.X, Up.Y, Up.Z);
    ImGui::Text("Camera GetRight(): (%.2f %.2f %.2f)", Right.X, Right.Y, Right.Z);
    ImGui::Separator();
}

void UI::RenderViewMode() {
    const char* viewModeItems[] = { "Lit", "Unlit", "Wireframe" };
    int currViewMode = static_cast<int>(Renderer->GetCurrentViewMode());

    if (ImGui::Combo("View Mode", &currViewMode, viewModeItems, IM_ARRAYSIZE(viewModeItems))) {
        Renderer->SetViewMode(static_cast<EViewModeIndex>(currViewMode));
    }
    ImGui::Separator();
}

void UI::RenderShowFlag() {
    ImGui::Text("Show Flag");
    bool bShowPrimitives = FEditorManager::Get().IsShowFlagSet(EEngineShowFlags::SF_Primitives);
    if (ImGui::Checkbox("Show Primtives", &bShowPrimitives))
    {
        FEditorManager::Get().SetShowFlag(EEngineShowFlags::SF_Primitives, bShowPrimitives);
        if (!bShowPrimitives)
            FEditorManager::Get().SelectComponent(nullptr);
    }

    bool bShowTextBillboard = FEditorManager::Get().IsShowFlagSet(EEngineShowFlags::SF_BillboardText);
    if (ImGui::Checkbox("Show Billboard Text", &bShowTextBillboard))
    {
        FEditorManager::Get().SetShowFlag(EEngineShowFlags::SF_BillboardText, bShowTextBillboard);
    }
}

void UI::RenderPropertyWindow()
{
    SetWindowLayout(0.3f, 0.4f, 0.f, 0.6f);
    ImGui::Begin("Properties");

    PickState CurrentPickState = FEditorManager::Get().GetPickState();

    if (CurrentPickState == PickState::Actor)
    {
        AActor* selectedActor = FEditorManager::Get().GetSelectedActor();
        if (selectedActor != nullptr)
        {
            ImGui::Text("Selected : [%4d]%s", selectedActor->GetUUID(), *selectedActor->Name.GetString());
            FTransform selectedTransform = selectedActor->GetActorRelativeTransform();
            float position[] = { selectedTransform.GetPosition().X, selectedTransform.GetPosition().Y, selectedTransform.GetPosition().Z };
            float scale[] = { selectedTransform.GetScale().X, selectedTransform.GetScale().Y, selectedTransform.GetScale().Z };

            if (ImGui::DragFloat3("Translation", position, 0.1f))
            {
                selectedTransform.SetPosition(position[0], position[1], position[2]);
                selectedActor->SetActorRelativeTransform(selectedTransform);
            }

            FVector PrevEulerAngle = selectedTransform.GetRotation().GetEuler();
            FVector UIEulerAngle = PrevEulerAngle;
            if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&UIEulerAngle), 0.1f))
            {
                FVector DeltaEulerAngle = UIEulerAngle - PrevEulerAngle;

                selectedTransform.Rotate(DeltaEulerAngle);
                //UE_LOG("Rotation: %.2f, %.2f, %.2f", DeltaEulerAngle.X, DeltaEulerAngle.Y, DeltaEulerAngle.Z);
                selectedActor->SetActorRelativeTransform(selectedTransform);
            }
            if (ImGui::DragFloat3("Scale", scale, 0.1f))
            {
                selectedTransform.SetScale(scale[0], scale[1], scale[2]);
                selectedActor->SetActorRelativeTransform(selectedTransform);
            }

            if (selectedActor->IsA(AStaticMesh::StaticClass()))
            {
                //StaticMeshComponent 가져와서 현재 풀에 있는 오브젝트 목록 띄워주고 바꿀수있게 해주기
                UStaticMeshComponent* StaticMeshComponent = selectedActor->GetComponentByClass<UStaticMeshComponent>();
                TMap<std::string, TArray<FSubMeshData>>& ObjInfos = UResourceManager::Get().GetMeshDatas();
                //FStaticMesh에 지금 현재 오브젝트 이름이 있다.
                std::string& StaticMeshObjName = StaticMeshComponent->GetObjName();

                if (ImGui::BeginCombo("Object", StaticMeshObjName.c_str()))
                {
                    for (auto& [Key, Item] : ObjInfos)
                    {
                        bool isSelected = Hash(StaticMeshObjName) == Hash(Key);
                        if (ImGui::Selectable(Key.c_str(), isSelected)) {
                            StaticMeshObjName = Key; // 선택된 항목 업데이트
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus(); // 기본 포커스 설정
                        }
                    }
                    ImGui::EndCombo();
                }
                
                StaticMeshComponent->LoadFromObj(StaticMeshObjName);
            }
        }
    }else if (CurrentPickState == PickState::Component)
        {
        UPrimitiveComponent* selectedComponent = FEditorManager::Get().GetSelectedComponent();

        if (selectedComponent)
        {
            ImGui::Text("Selected : [%4d]%s", selectedComponent->GetUUID(), *selectedComponent->Name.GetString());
            FTransform selectedTransform = selectedComponent->GetRelativeTransform();
            float position[] = { selectedTransform.GetPosition().X, selectedTransform.GetPosition().Y, selectedTransform.GetPosition().Z };
            float scale[] = { selectedTransform.GetScale().X, selectedTransform.GetScale().Y, selectedTransform.GetScale().Z };

            if (ImGui::DragFloat3("Translation", position, 0.1f))
            {
                selectedTransform.SetPosition(position[0], position[1], position[2]);
                selectedComponent->SetRelativeTransform(selectedTransform);
            }

            FVector PrevEulerAngle = selectedTransform.GetRotation().GetEuler();
            FVector UIEulerAngle = PrevEulerAngle;
            if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&UIEulerAngle), 0.1f))
            {
                FVector DeltaEulerAngle = UIEulerAngle - PrevEulerAngle;

                selectedTransform.Rotate(DeltaEulerAngle);
                selectedComponent->SetRelativeTransform(selectedTransform);
            }
            if (ImGui::DragFloat3("Scale", scale, 0.1f))
            {
                selectedTransform.SetScale(scale[0], scale[1], scale[2]);
                selectedComponent->SetRelativeTransform(selectedTransform);
            }
            
            FVector4 ActorColor = selectedComponent->GetColor();
            float* ColorArray = reinterpret_cast<float*>(&ActorColor);
            if (ImGui::ColorEdit4("Color", ColorArray))
            {
                selectedComponent->SetColor(ActorColor);
            }
            bool bRender = selectedComponent->GetIsDefaultRendered();
            if (ImGui::Checkbox("Show Primitive", &bRender))
            {
                selectedComponent->SetIsDefaultRendered(bRender);
            }
            if (selectedComponent->IsA(UStaticMeshComponent::StaticClass()))
            {
                //이터레이터 돌면서 각 서브메쉬에 해당하는 머테리얼 변경할 수 있게 드랍박스로 제공
                UStaticMeshComponent* StaticMeshComponent = dynamic_cast<UStaticMeshComponent*>(selectedComponent);
                TArray<FStaticMesh> StaticMeshInfos = StaticMeshComponent->GetRenderUnits();
                
                TMap<std::string, FMaterialData>& MaterialInfos = UResourceManager::Get().GetMaterials();
                TMap<uint32_t, std::string> MaterialItems; //GUID, Path
                
                for (auto& [Key, Item] : MaterialInfos) //GUID로 기존인덱스 설정
                {
                    MaterialItems.Add(Item.GUID, Key);
                }
                
                int SubMeshIndex = 0;
                for (auto& StaticMeshInfo : StaticMeshInfos)
                {
                    char SubMeshName[32];
                    snprintf(SubMeshName, sizeof(SubMeshName), "SubMesh%d", SubMeshIndex++);
                    if (ImGui::BeginCombo(SubMeshName, MaterialItems[StaticMeshInfo.GUID].c_str()))
                    {
                        for (auto& [Key, Item] : MaterialItems)
                        {
                            bool isSelected = (StaticMeshInfo.Material->GUID == Key);
                            if (ImGui::Selectable(MaterialItems[Key].c_str(), isSelected)) {
                                StaticMeshInfo.GUID = Key; // 선택된 항목 업데이트
                            }
                            if (isSelected) {
                                ImGui::SetItemDefaultFocus(); // 기본 포커스 설정
                            }
                        }
                        ImGui::EndCombo();
                    }
                    StaticMeshInfo.Material = &MaterialInfos[MaterialItems[StaticMeshInfo.GUID]];
                }
                StaticMeshComponent->SetRenderUnits(StaticMeshInfos);
            }
        }
    }
    if (FEditorManager::Get().GetGizmoHandle() != nullptr)
    {
        AGizmoHandle* Gizmo = FEditorManager::Get().GetGizmoHandle();
        if (Gizmo->GetGizmoType() == EGizmoType::Translate)
        {
            ImGui::Text("GizmoType: Translate");
        }
        else if (Gizmo->GetGizmoType() == EGizmoType::Rotate)
        {
            ImGui::Text("GizmoType: Rotate");
        }
        else if (Gizmo->GetGizmoType() == EGizmoType::Scale)
        {
            ImGui::Text("GizmoType: Scale");
        }
    }
    
    ImGui::End();
}

void UI::RenderSceneManager()
{
    static int selectedBefore = -1;
    static int selected = -1; 
    const TArray<AActor*>& ActorArray = UEngine::Get().GetWorld()->GetActors();
    uint32 NumActors = ActorArray.Num();

    if (NumActors == 0)
        return;

    // Scene Manager 창을 독립된 OS 창으로 만들기 위한 설정
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse; 
    flags |= ImGuiWindowFlags_MenuBar;           

    // 최초 위치/크기 지정 (윈도우 밖으로)
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::SetNextWindowPos(ImVec2(1500, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiCond_FirstUseEver);
    }

    ImGui::Begin("Scene Manager", nullptr, flags);

    if (ImGui::TreeNode("Primitives"))
    {
        for (int n = 0; n < NumActors; n++)
        {
            char buf[64];
            sprintf_s(buf, "%s", *ActorArray[n]->Name.GetString());

            if (Unselectables.Find(ActorArray[n]->Name) > -1)
                continue;

            if (ImGui::Selectable(buf, selected == n && selectedBefore != selected))
                selected = n;
        }
        ImGui::TreePop();
    }

    if (selected > -1 && selectedBefore != selected)
    {
        if (selected < NumActors)
        {
            FEditorManager::Get().SelectActor(ActorArray[selected]);
        }
    }
    selectedBefore = selected;

    ImGui::End();
}


void ShowComponentsRecursive(USceneComponent* Component, uint32 uniqueID)
{
    if (!Component) return;
    
    FString NodeID = FString("##") + FString::FromInt(uniqueID);
    const TSet<USceneComponent*>& Children = Component->GetAttachChildren();
    if (ImGui::TreeNode(*NodeID, "[%4d]%s(%d)", Component->GetUUID(), *Component->Name.GetString(), Children.Num()))
    {
        uint32 id = 0;
        FTransform T = Component->GetComponentTransform();
        FVector P = T.GetPosition();
        ImGui::Text("CompPos : %f %f %f", P.X, P.Y, P.Z);
        for (USceneComponent* Child : Children)
        {
            ShowComponentsRecursive(Child, 128*uniqueID + (id++));
        }

        ImGui::TreePop();
    }
}

void UI::RenderComponentsByActor()
{
    const TArray<AActor*>& ActorArray = UEngine::Get().GetWorld()->GetActors();
    uint32 NumActors = ActorArray.Num();

    SetWindowLayout(0.3f, 0.2f, 1.f, 0.4f);

    if (NumActors > 0) {
        static int selected = -1;
        ImGui::Begin("Components Tree");
        if (ImGui::TreeNode("[UUID]Actors/Components(NumChilds)"))
        {
            for (int n = 0; n < NumActors; n++)
            {
                ImGui::PushID(n);
                if (ImGui::TreeNode("", "[%4d]%s", ActorArray[n]->GetUUID(), *ActorArray[n]->Name.GetString()))
                {
                    if (USceneComponent* RootComponent = ActorArray[n]->GetRootComponent())
                    {
                        ShowComponentsRecursive(RootComponent, 0);
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();

            }
            ImGui::TreePop();
        }
        ImGui::End();
    }
}

void UI::RenderFNameResolver()
{
    SetWindowLayout(0.3f, 0.1f, 1.f, 0.6f);

    ImGui::Begin("FName Resolver");

    static char id[8];
    ImGui::InputText("FName DisplayIndex", id, 8);
    static FString Name;
    if (ImGui::Button("Resolve"))
    {
        Name = FNamePool::ResolveDisplay(atoi(id));
    }
    ImGui::Text("%s", *Name);
    ImGui::End();
}

void UI::RenderViewOption()
{
    URenderer* renderer = UEngine::Get().GetRenderer();
    if (renderer->activeFullViewport) return;

    float width = 100;
    float height = 35;

    ImVec2 offset = ImGui::GetMainViewport()->Pos; // 윈도우 창 이동 오프셋

    // LT
    {
        FViewport* viewportLT = renderer->GetMultiViewport(EViewport::Position::LT);
        if (!viewportLT) return;
        static int current_itemLT = static_cast<int>(viewportLT->GetCameraType());
        const FRect& rectLT = viewportLT->GetRect();
        ImGui::SetNextWindowPos(ImVec2(rectLT.Max.X - width + offset.x, rectLT.Min.Y + offset.y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        SetViewOption("LT", current_itemLT, EViewport::Position::LT);
    }

    // RT
    {
        FViewport* viewportRT = renderer->GetMultiViewport(EViewport::Position::RT);
        static int current_itemRT = static_cast<int>(viewportRT->GetCameraType());
        const FRect& rectRT = viewportRT->GetRect();
        ImGui::SetNextWindowPos(ImVec2(rectRT.Min.X + offset.x, rectRT.Min.Y + offset.y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        SetViewOption("RT", current_itemRT, EViewport::Position::RT);
    }

    // LB
    {
        FViewport* viewportLB = renderer->GetMultiViewport(EViewport::Position::LB);
        static int current_itemLB = static_cast<int>(viewportLB->GetCameraType());
        const FRect& rectLB = viewportLB->GetRect();
        ImGui::SetNextWindowPos(ImVec2(rectLB.Max.X-width + offset.x, rectLB.Min.Y + offset.y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        SetViewOption("LB", current_itemLB, EViewport::Position::LB);
    }

    // RB
    {
        FViewport* viewportRB = renderer->GetMultiViewport(EViewport::Position::RB);
        static int current_itemRB = static_cast<int>(viewportRB->GetCameraType());
        const FRect& rectRB = viewportRB->GetRect();
        ImGui::SetNextWindowPos(ImVec2(rectRB.Min.X + offset.x, rectRB.Min.Y + offset.y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        SetViewOption("RB", current_itemRB, EViewport::Position::RB);
    }
}

void UI::SetViewOption(const char* name, int& current_item, EViewport::Position viewportPos)
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin(name, nullptr, flags);
    
    const char* items[] = { "Front", "Back", "Top", "Bottom", "Left", "Right", "Perspective" };

    if (ImGui::Combo(name, &current_item, items, IM_ARRAYSIZE(items))) {
        ECameraViewMode::Type newViewMode = static_cast<ECameraViewMode::Type>(current_item);
        UEngine::Get().GetRenderer()->ChangeViewportCameraType(viewportPos, newViewMode);
    }

    ImGui::End();
}



void UI::SetWindowLayout(float widthRatio, float heightRatio, float posXRatio, float posYRatio)
{
    float controllWindowWidth = static_cast<float>(windowWidth) * widthRatio;
    float controllWindowHeight = static_cast<float>(windowHeight) * heightRatio;
    float controllWindowPosX = (static_cast<float>(windowWidth) - controllWindowWidth) * posXRatio;
    float controllWindowPosY = (static_cast<float>(windowHeight) - controllWindowHeight) * posYRatio;

    //ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
    ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Once);
}





