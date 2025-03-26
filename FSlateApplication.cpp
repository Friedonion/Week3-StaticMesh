﻿#include "FSlateApplication.h"
#include "Source/Core/Rendering/URenderer.h"
#include "Source/Static/FEditorManager.h"
#include "../FViewport.h"
#include "Core/Engine.h"
#include "Source/Core/Input/PlayerInput.h"
#include "Debug/DebugConsole.h"
void FSlateApplication::Initialize()
{
	FVector2 swapSize = UEngine::Get().GetRenderer()->GetSwapChainSize();
	float width = swapSize.X;
	float height = swapSize.Y;
	UEngine::Get().SetScreenWidth(swapSize.X);
	UEngine::Get().SetScreenHeight(swapSize.Y);
}

void FSlateApplication::Tick()
{
	ProcessIsHover();
	ProcessKeyDownEvent();
	ProcessMouseButtonDownEvent();
	FVector2 swapSize = UEngine::Get().GetRenderer()->GetSwapChainSize();
	float width = swapSize.X;
	float height = swapSize.Y;
	//UE_LOG("SwapChain size %f, %f", width, height);
	for (int i = 0; i < windows.Num(); i++)
	{
		FRect rect = windows[i]->Rect;
		//UE_LOG(*rect.ToFString());
	}
}

void FSlateApplication::ShutDown()
{
	if (fullWindow) fullWindow->RestorePrevSize();
	SaveSWindowToJSON();
	SaveSWindowRatioToJSON();

	for (int i = 0; i < windows.Num(); i++)
	{
		SWindow* window = windows[i];
		delete window;
	}
}

FRect FSlateApplication::GetCurrentWindow()
{
	FRect resultRect;
	resultRect.Max = FVector2(0, 0);
	resultRect.Min = FVector2(0, 0);
	if (currentWindow) resultRect = currentWindow->Rect;
	return resultRect;
}

FViewport* FSlateApplication::SNEW(FRect _rect)
{
	FViewport* viewport = new FViewport(_rect);
	SWindow* window = new SWindow();
	window->SetISlateViewport(viewport);
	window->Rect = _rect;
	windows.Add(window);
	return viewport;
}

void FSlateApplication::Add(SWindow* _window)
{
	windows.Add(_window);
}

void FSlateApplication::ResizeScreen(float resizeWidthRatio, float resizeHeightRatio)
{
	for (int i = 0; i < windows.Num(); i++)
	{
		windows[i]->ScreenResize(resizeWidthRatio, resizeHeightRatio);
	}
}

void FSlateApplication::SaveSWindowToJSON()
{
	json::JSON jsonArray = json::Array();
	for (int i = 0; i < windows.Num(); i++)
	{
		jsonArray.append(windows[i]->ToJSON());

	}
	std::ofstream outFile("engine.ini");
	outFile << jsonArray.dump(4); // pretty print
	outFile.close();

}

void FSlateApplication::SaveSWindowRatioToJSON()
{
	json::JSON jsonArray = json::Array();
	for (int i = 0; i < windows.Num(); i++)
	{
		jsonArray.append(windows[i]->ToJSONRatio());

	}
	std::ofstream outFile("engineRatio.ini");
	outFile << jsonArray.dump(4); // pretty print
	outFile.close();
}

SWindow* FSlateApplication::GetClickedWindow()
{
	return clickedWindow;
}

void FSlateApplication::ProcessMouseButtonDownEvent()
{
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(UEngine::Get().GetWindowHandle(), &pt);
	FVector2 MousePos(pt.x, pt.y);

	APlayerInput& Input = APlayerInput::Get();
	FVector MousePrePos = APlayerInput::Get().GetMousePrePos();
	FVector MouseNextPos = APlayerInput::Get().GetMousePos();
	FVector DeltaPos = MouseNextPos - MousePrePos;

	FVector2 MousePos2D(MouseNextPos.X, MouseNextPos.Y);
	FVector2 MousePrePos2D(MousePrePos.X, MousePrePos.Y);
	FVector2 DeltaPos2D(DeltaPos.X, DeltaPos.Y);
	//UE_LOG("MousePrePos %f %f", MousePrePos.X, MousePrePos.Y);
	//UE_LOG("MousePos %f %f", MousePos.X, MousePos.Y);
	//UE_LOG("DeltaPos2D %f %f", DeltaPos2D.X, DeltaPos2D.Y);

	if (Input.GetMouseDown(false))
	{
		for (int i = 0; i < windows.Num(); i++)
		{
			if (windows[i]->isClicked(MousePos))
			{
				clickedWindow = windows[i];
			}
		}
	}
	
	if (Input.IsPressedMouse(false))
	{
		if (clickedWindow&&DeltaPos.X!=0&&DeltaPos.Y!=0)
		{
			clickedWindow->OnMouseDrag(DeltaPos2D);
		}
	}
	else
	{
		if (clickedWindow)
		{
			clickedWindow->OnMouseUp();
			clickedWindow = nullptr;
		}
	}


}

void FSlateApplication::ProcessKeyDownEvent()
{
	APlayerInput& Input = APlayerInput::Get();
	URenderer* renderer = UEngine::Get().GetRenderer();
	
	if (Input.GetKeyDown(EKeyCode::Enter)&&renderer->activeFullViewport==nullptr)
	{
		if (currentWindow == nullptr) return;
		if (dynamic_cast<SSplitter2x2*>(currentWindow)) return;
		currentWindow->SetActiveFullViewport();

		FRect fullrect;
		FVector2 fullScreen = renderer->GetSwapChainSize();
		fullrect.Min.X = 0;
		fullrect.Min.Y = 0;
		fullrect.Max.X = fullScreen.X;
		fullrect.Max.Y = fullScreen.Y;
		currentWindow->Resize(fullrect);
		fullWindow = currentWindow;
	}
	else if (Input.GetKeyDown(EKeyCode::Enter) && renderer->activeFullViewport)
	{
		if (fullWindow==nullptr) return;
		renderer->activeFullViewport = nullptr;
		fullWindow->RestorePrevSize();
		fullWindow = nullptr;
	}
}

void FSlateApplication::ProcessIsHover()
{
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(UEngine::Get().GetWindowHandle(), &pt);
	FVector2 mouse(pt.x, pt.y);
	if (fullWindow)
	{
		fullWindow->isHover(mouse);
		return;
	}
	for (int i = 0; i < windows.Num(); i++)
	{
		if (windows[i]->isHover(mouse))
		{
			SWindow* prevWindow = currentWindow;
			currentWindow = windows[i];
			windows[i]->ChangeMainCamera();
			if (currentWindow != prevWindow)
			{
				currentWindow->OnFocus();
			}
			//FEditorManager::Get().SetCameraIndex(i);
		}
	}
}
