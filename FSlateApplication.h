﻿#pragma once
#include "Source/Core/AbstractClass/Singleton.h"
#include "Source/Core/Container/Array.h"
#include "SWindow.h"
#include "SSplitter.h"

class FViewport;
class FSlateApplication : public TSingleton<FSlateApplication>
{
public:
	void Initialize();
	void Tick();
	void ShutDown();
	FRect GetCurrentWindow();
	FViewport* SNEW(FRect _rect);
private:
	void ProcessMouseButtonDownEvent();
	void ProcessKeyDownEvent();
	void ProcessIsHover();

	TArray<SWindow*> windows;
	SWindow* currentWindow;
};

