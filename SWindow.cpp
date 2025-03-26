#include "SWindow.h"
#include "Source\Debug\DebugConsole.h"
#include "ISlateViewport.h"
#include "Object/Actor/Camera.h"
SWindow::SWindow()
{
}

SWindow::SWindow(FRect _rect)
{
	Rect = _rect;
}

SWindow::~SWindow()
{
}

void SWindow::SetISlateViewport(ISlateViewport* _viewport)
{
	viewport = _viewport;
}

void SWindow::AttachViewportCamera()
{
	viewport->ChangeMainCamera();
}

bool SWindow::isHover(FVector2 coord) const
{
	float x = coord.X;
	float y = coord.Y;
	if (Rect.Contains(coord))
	{
		//viewport->ChangeMainCamera();
		return true;
	}
	return false;
}

void SWindow::OnFocus()
{

	//viewport->ChangeMainCamera();
}


bool SWindow::isClicked(FVector2 mousePos) 
{
	viewport->ChangeMainCamera();
	return Rect.Contains(mousePos);
}

bool SWindow::OnMouseDrag(FVector2 mouseDelta)
{
	return false;
}

bool SWindow::OnMouseUp()
{
	return false;
}

void SWindow::Resize(const FRect& _rect)
{
	prevRect = Rect;
	Rect = _rect;
	if(viewport) viewport->Resize(_rect);
	UE_LOG("Resize Min (%f %f), Max (%f %f)",
		Rect.Min.X, Rect.Min.Y, Rect.Max.X, Rect.Max.Y);
}

void SWindow::ScreenResize(float resizeWidthRatio, float resizeHeightRatio)
{
	prevRect = Rect;
	Rect.ResizeRatio(resizeWidthRatio, resizeHeightRatio);
	if(viewport) viewport->Resize(Rect);
}

json::JSON SWindow::ToJSON()
{
	json::JSON j;
	j["MinX"] = Rect.Min.X;
	j["MinY"] = Rect.Min.Y;
	j["MaxX"] = Rect.Max.X;
	j["MaxY"] = Rect.Max.Y;
	
	if (viewport)
	{
		j["CameraType"] = static_cast<int>(viewport->GetCameraViewMode());
		j["ViewportPos"] = static_cast<int>(viewport->GetViewportPos());
	}
	j["type"] = "SWindow";
	return j;
}

json::JSON SWindow::ToJSONRatio()
{
	FVector2 swapSize = UEngine::Get().GetRenderer()->GetSwapChainSize();
	float width = swapSize.X;
	float height = swapSize.Y;

	json::JSON j;
	j["MinX"] = Rect.Min.X/width;
	j["MinY"] = Rect.Min.Y/height;
	j["MaxX"] = Rect.Max.X/width;
	j["MaxY"] = Rect.Max.Y/height;

	if (viewport)
	{
		j["CameraType"] = static_cast<int>(viewport->GetCameraViewMode());
		j["ViewportPos"] = static_cast<int>(viewport->GetViewportPos());
	}
	j["type"] = "SWindow";
	return j;
}

void SWindow::SetActiveFullViewport()
{
	if(viewport) viewport->SetActiveFullViewport();
}

void SWindow::RestorePrevSize()
{
	viewport->Resize(prevRect);
	FRect tmp = Rect;
	Rect = prevRect;
	prevRect = tmp;
}

void SWindow::ChangeMainCamera()
{
	if(viewport!=nullptr) viewport->ChangeMainCamera();
}

ECameraViewMode::Type SWindow::GetCameraViewMode()
{
	if (viewport) return viewport->GetCameraViewMode();
	else return ECameraViewMode::Type::None;
}
