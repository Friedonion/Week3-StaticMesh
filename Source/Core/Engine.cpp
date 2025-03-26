﻿#include "Engine.h"
#include <iostream>
#include "Object/ObjectFactory.h"
#include "Object/World/World.h"
#include "Debug/DebugConsole.h"
#include "Object/Gizmo/Axis.h"
#include "Core/Input/PlayerInput.h"
#include "Core/Input/PlayerController.h"
#include "Object/Actor/Camera.h"
#include "Object/Actor/Sphere.h"
#include "Object/Actor/WorldGrid.h"
#include "Static/FEditorManager.h"
#include"Data/ConfigManager.h"
#include "../FSlateApplication.h"
#include "Data/ObjManager.h"
#include "Core/HAL/PlatformType.h"
#include"Static/ResourceManager.h"
#include"Object/Actor/StaticMesh.h"
#include <shellapi.h>
class AArrow;
class APicker;
// ImGui WndProc 정의
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT UEngine::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // ImGui의 메시지를 처리
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
    {
        return true;
    }

    static bool m_isManualResize = false;

    switch (uMsg)
    {
        // 창이 제거될 때 (창 닫기, Alt+F4 등)
    case WM_DESTROY:
        PostQuitMessage(0); // 프로그램 종료
        break;
    case WM_KEYDOWN:
        APlayerInput::Get().KeyDown(static_cast<EKeyCode>(wParam));
        if ((lParam>>30)%2 != 0)
        {
            APlayerInput::Get().KeyOnceUp(static_cast<EKeyCode>( wParam ));
        }
        break;
    case WM_KEYUP:
        APlayerInput::Get().KeyUp(static_cast<EKeyCode>( wParam ));
        break;
    case WM_LBUTTONDOWN:
        APlayerInput::Get().HandleMouseInput(hWnd, lParam, true, false);
        break;
    case WM_LBUTTONUP:
        APlayerInput::Get().HandleMouseInput(hWnd, lParam, false, false);
        break;
    case WM_RBUTTONDOWN:
        APlayerInput::Get().HandleMouseInput(hWnd, lParam, true, true);
        break;
    case WM_RBUTTONUP:
        APlayerInput::Get().HandleMouseInput(hWnd, lParam, false, true);
        break;
    case WM_SIZING:
        m_isManualResize = true;
        break;
    case WM_SIZE:
    if(m_isManualResize){
        UEngine::Get().UpdateWindowSize(LOWORD(lParam), HIWORD(lParam));
        m_isManualResize = false;
    }
		break;   
    case WM_DROPFILES:
{
    HDROP hDrop = (HDROP)wParam;
    
    // 몇 개의 파일이 드롭되었는지 확인
    UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

    for (UINT i = 0; i < fileCount; ++i)
    {
        wchar_t filePath[MAX_PATH];
        DragQueryFileW(hDrop, 0, filePath, MAX_PATH);
        UWorld* World = UEngine::Get().GetWorld();

        std::wstring ws(filePath);
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string path(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &path[0], size_needed, nullptr, nullptr);

        // 액터 스폰 및 obj 로딩
        if (UEngine::Get().GetWorld())
        {
            AStaticMesh* CustomActor = UEngine::Get().GetWorld()->SpawnActor<AStaticMesh>();
            CustomActor->SetObjName(path);
        }
        MessageBox(hWnd, filePath, TEXT(L"드롭된 파일 경로"), MB_OK);
    }

    DragFinish(hDrop); // 메모리 해제
    return 0;
}
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

void UEngine::InitTextures()
{
    for (auto& [type, path] : TexturesToLoad)
    {
        UResourceManager::Get().LoadTexture(path);
    }
}

void UEngine::Initialize(
    HINSTANCE hInstance, const WCHAR* InWindowTitle, const WCHAR* InWindowClassName, int InScreenWidth,
    int InScreenHeight,
    EScreenMode InScreenMode
)
{
    WindowInstance = hInstance;
    WindowTitle = InWindowTitle;
    WindowClassName = InWindowClassName;
    ScreenMode = InScreenMode;
    ScreenWidth = InScreenWidth;
    ScreenHeight = InScreenHeight;
    ObjLoader = std::make_unique<FObjManager>();

    InitWindow(InScreenWidth, InScreenWidth);
    InitRenderer();
    InitWorld();
    InitTextures();
    UResourceManager::Get().Initialize(Renderer->GetDevice(),Renderer->GetDeviceContext());
    
    InitializedScreenWidth = ScreenWidth;
    InitializedScreenHeight = ScreenHeight;
    
    ui.Initialize(WindowHandle, *Renderer, ScreenWidth, ScreenHeight);
    FSlateApplication::Get().Initialize();
	UE_LOG("Engine Initialized!");
}

void UEngine::Run()
{
    // FPS 제한
    constexpr int TargetFPS = 750;
    constexpr double TargetDeltaTime = 1000.0f / TargetFPS; // 1 FPS의 목표 시간 (ms)

    // 고성능 타이머 초기화
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);

    LARGE_INTEGER StartTime;
    QueryPerformanceCounter(&StartTime);
    
    IsRunning = true;
    while (IsRunning)
    {
        // DeltaTime 계산 (초 단위)
        const LARGE_INTEGER EndTime = StartTime;
        QueryPerformanceCounter(&StartTime);

        const float DeltaTime =
            static_cast<float>(StartTime.QuadPart - EndTime.QuadPart) / static_cast<float>(Frequency.QuadPart);

        APlayerInput::Get().PreProcessInput();
        
        // 메시지(이벤트) 처리
        MSG Msg;
        while (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
        {
            // 키 입력 메시지를 번역
            TranslateMessage(&Msg);

            // 메시지를 등록한 Proc에 전달
            DispatchMessage(&Msg);

            if (Msg.message == WM_QUIT)
            {
                IsRunning = false;
                break;
            }

        }

        if (World == nullptr)
        {
            continue;
        }
        
		// Renderer Update
        Renderer->Prepare();
        Renderer->PrepareShader();
        
		// World Update
		if (World)
		{
			World->Tick(DeltaTime);
			World->Render(DeltaTime);
		    World->LateTick(DeltaTime);
		}

        //각 Actor에서 TickActor() -> PlayerTick() -> TickPlayerInput() 호출하는데 지금은 Message에서 처리하고 있다.
        APlayerInput::Get().TickPlayerInput(); //잘못된 위치. 위에 달린 주석대로 처리해야 정상 플레이어 액터 내에서만 처리해야할것같다.
        
        // TickPlayerInput
        FSlateApplication::Get().Tick();
        APlayerController::Get().ProcessPlayerInput(DeltaTime);
        
        if (!IsRunning)
            break;
		// ui Update
        ui.Update();

        Renderer->SwapBuffer();

        // FPS 제한
        double ElapsedTime;
        do
        {
            Sleep(0);

            LARGE_INTEGER CurrentTime;
            QueryPerformanceCounter(&CurrentTime);

            ElapsedTime = static_cast<double>(CurrentTime.QuadPart - StartTime.QuadPart) * 1000.0 / static_cast<double>(Frequency.QuadPart);
        } while (ElapsedTime < TargetDeltaTime);
    }
}


void UEngine::Shutdown()
{
    FSlateApplication::Get().ShutDown();
    Renderer->Release();
    
    ConfigManager::Get().SaveAllConfigs();

#ifdef _DEBUG
    World->SaveWorld(*World->DebugDefaultSceneName);
#else
    World->SaveWorld(*World->DefaultSceneName)
#endif
        UResourceManager::Get().Shutdown();
    ShutdownWindow();
}


void UEngine::SetScreenWidth(float _screenWidth)
{
    ScreenWidth = _screenWidth;
}

void UEngine::SetScreenHeight(float _screenHeight)
{
    ScreenHeight = _screenHeight;
}

void UEngine::InitWindow(int InScreenWidth, int InScreenHeight)
{
    // 윈도우 클래스 등록
    WNDCLASSW wnd_class{};
    wnd_class.lpfnWndProc = WndProc;
    wnd_class.hInstance = WindowInstance;
    wnd_class.lpszClassName = WindowClassName;
    RegisterClassW(&wnd_class);

    // Window Handle 생성
    WindowHandle = CreateWindowExW(
        WS_EX_NOREDIRECTIONBITMAP, WindowClassName, WindowTitle,
        WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        InScreenWidth, InScreenHeight,
        nullptr, nullptr, WindowInstance, nullptr
    );
    DragAcceptFiles(WindowHandle, TRUE);
    // TODO: 전체화면 구현
    if (ScreenMode != EScreenMode::Windowed)
    {
        std::cout << "not implement Fullscreen and Borderless mode." << '\n';
    }

    // 윈도우 포커싱
    ShowWindow(WindowHandle, SW_SHOW);
    SetForegroundWindow(WindowHandle);
    SetFocus(WindowHandle);

    //AllocConsole(); // 콘솔 창 생성

    //// 표준 출력 및 입력을 콘솔과 연결
    //freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    //freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

    //std::cout << "Debug Console Opened!" << '\n';
}

void UEngine::InitRenderer()
{
	// 렌더러 초기화
	Renderer = std::make_unique<URenderer>();
	Renderer->Create(WindowHandle);
	Renderer->CreateShader();
	Renderer->CreateConstantBuffer();
    
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
}

void UEngine::InitWorld()
{
    World = FObjectFactory::ConstructObject<UWorld>();
    
    ACamera* mainCamera = World->SpawnActor<ACamera>();
    mainCamera->SetCameraViewMode(ECameraViewMode::Type::Perspective);
    FEditorManager::Get().SetCamera(mainCamera);

    //Front
    ACamera* frontCamera = World->SpawnActor<ACamera>();
    frontCamera->SetCameraViewMode(ECameraViewMode::Type::Front);
    
    //Top
    ACamera* topCamera = World->SpawnActor<ACamera>();
    topCamera->SetCameraViewMode(ECameraViewMode::Type::Top);
    
    //Right
    ACamera* rightCamera = World->SpawnActor<ACamera>();
    rightCamera->SetCameraViewMode(ECameraViewMode::Type::Right);
    
    //Back
    ACamera* backCamera = World->SpawnActor<ACamera>();
    backCamera->SetCameraViewMode(ECameraViewMode::Type::Back);
    
    //Bottom
    ACamera* bottomCamera = World->SpawnActor<ACamera>();
    bottomCamera->SetCameraViewMode(ECameraViewMode::Type::Bottom);
   
    //Left
    ACamera* leftCamera = World->SpawnActor<ACamera>();
    leftCamera->SetCameraViewMode(ECameraViewMode::Type::Left);
    
    FEditorManager::Get().SetWorldGrid(World->SpawnActor<AWorldGrid>());

#ifdef _DEBUG
    World->LoadWorld(*World->DebugDefaultSceneName);
#else
    World->LoadWorld(*World->ReleaseDefaultSceneName);
#endif
    
    ConfigManager::Get().LoadAllConfigs();
    
    World->SpawnActor<AAxis>();
    World->SpawnActor<APicker>();
   // World->SpawnActor<AArrow>();
	World->BeginPlay();
}

void UEngine::ShutdownWindow()
{
    ui.Shutdown();
    if (WindowHandle != nullptr)
    {
        DestroyWindow(WindowHandle);   
        WindowHandle = nullptr;
    }
    UnregisterClassW(WindowClassName, WindowInstance);
    WindowInstance = nullptr;
    ui.bIsInitialized = false;
}

void UEngine::UpdateWindowSize(UINT InScreenWidth, UINT InScreenHeight)
{
    //화면 최소화 반응 안하기
    if (InScreenHeight == 0 || InScreenWidth == 0)return;
    float resizeWidthRatio = float(InScreenWidth) / ScreenWidth;
    float resizeHeightRatio = float(InScreenHeight) / ScreenHeight;
    UE_LOG("New size %d %d", InScreenWidth, InScreenHeight);
    UE_LOG("Origin size %d %d", ScreenWidth, ScreenHeight);
    UE_LOG("Resize Ratio %f %f", resizeWidthRatio, resizeHeightRatio);
    FSlateApplication::Get().ResizeScreen(resizeWidthRatio, resizeHeightRatio);
	ScreenWidth = InScreenWidth;
	ScreenHeight = InScreenHeight;

    if(Renderer)
    {
        Renderer->OnUpdateWindowSize(ScreenWidth, ScreenHeight);
    }

	if (ui.bIsInitialized)
	{
		ui.OnUpdateWindowSize(ScreenWidth, ScreenHeight);
	}
}

UObject* UEngine::GetObjectByUUID(uint32 InUUID) const
{
    if (const auto Obj = GObjects.Find(InUUID))
    {
        return Obj->get();
    }
    return nullptr;
}
