﻿#include "SSplitter.h"
#include "Source\Debug\DebugConsole.h"
#include "Object/Actor/Camera.h"

bool SSplitter::isHover(FVector2 coord) const
{
	return false;
}

void SSplitter::OnFocus()
{
    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
}


bool SSplitter::isClicked(FVector2 mousePos) 
{
    return false;
}

bool SSplitter::OnMouseDrag(FVector2 mouseDelta) 
{
    return false;
}

bool SSplitter::OnMouseUp() 
{
    return false;
}

void SSplitter::ScreenResize(float resizeWidthRatio, float resizeHeightRatio)
{
}

json::JSON SSplitter::ToJSON()
{
    return json::JSON();
}


bool SSplitter2x2::isHover(FVector2 coord) const
{
    bool inHorizontal = horitionalHandle.Contains(coord);

    bool inVertical = verticalHandle.Contains(coord);

    /*
    UE_LOG("horitionalHandle (%f, %f) (%f, %f)", 
        horitionalHandle.Min.X, horitionalHandle.Min.Y,
        horitionalHandle.Max.X, horitionalHandle.Max.Y);

    UE_LOG("VerticalHandle (%f, %f) (%f, %f)",
        verticalHandle.Min.X, verticalHandle.Min.Y,
        verticalHandle.Max.X, verticalHandle.Max.Y);
    */
    return inHorizontal || inVertical;
}

void SSplitter2x2::OnFocus()
{
    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
}


bool SSplitter2x2::isClicked(FVector2 mousePos) 
{
    if (horitionalHandle.Contains(mousePos))
    {
        bisHoriClicked = true;
    }
    else if (verticalHandle.Contains(mousePos))
    {
        bisVertiClicked = true;
    }
    return bisHoriClicked || bisVertiClicked;
}

bool SSplitter2x2::OnMouseDrag(FVector2 mouseDelta) 
{

    if (bisHoriClicked&&ClampHorizontal(mouseDelta))
    {

        horitionalHandle.Max.X += mouseDelta.X;
        horitionalHandle.Min.X += mouseDelta.X;

        SideLT->Rect.Max.X += mouseDelta.X;
        SideLT->Resize(SideLT->Rect);

        SideLB->Rect.Max.X += mouseDelta.X;
        SideLB->Resize(SideLB->Rect);

        SideRT->Rect.Min.X += mouseDelta.X;
        SideRT->Resize(SideRT->Rect);

        SideRB->Rect.Min.X += mouseDelta.X;
        SideRB->Resize(SideRB->Rect);

        //UE_LOG("Hori Splitter Clicked");
    }
    else if (bisVertiClicked&&ClampVertical(mouseDelta))
    {
        verticalHandle.Max.Y += mouseDelta.Y;
        verticalHandle.Min.Y += mouseDelta.Y;

        SideLT->Rect.Max.Y += mouseDelta.Y;
        SideLT->Resize(SideLT->Rect);

        SideRT->Rect.Max.Y += mouseDelta.Y;
        SideRT->Resize(SideRT->Rect);

        SideLB->Rect.Min.Y += mouseDelta.Y;
        SideLB->Resize(SideLB->Rect);

        SideRB->Rect.Min.Y += mouseDelta.Y;
        SideRB->Resize(SideRB->Rect);

        //UE_LOG("Verti Splitter Clicked");
    }
    return false;
}

bool SSplitter2x2::OnMouseUp() 
{
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    bisHoriClicked = false;
    bisVertiClicked = false;
    return false;
}

void SSplitter2x2::ScreenResize(float resizeWidthRatio, float resizeHeightRatio)
{
    horitionalHandle.ResizeRatio(resizeWidthRatio, resizeHeightRatio);
    verticalHandle.ResizeRatio(resizeWidthRatio, resizeHeightRatio);
}

json::JSON SSplitter2x2::ToJSON()
{
    json::JSON j;
    j["VMinX"] = verticalHandle.Min.X;
    j["VMinY"] = verticalHandle.Min.Y;
    j["VMaxX"] = verticalHandle.Max.X;
    j["VMaxY"] = verticalHandle.Max.Y;

    j["HMinX"] = horitionalHandle.Min.X;
    j["HMinY"] = horitionalHandle.Min.Y;
    j["HMaxX"] = horitionalHandle.Max.X;
    j["HMaxY"] = horitionalHandle.Max.Y;
    j["type"] = "SSplitter2*2";
    return j;
}

json::JSON SSplitter2x2::ToJSONRatio()
{
    FVector2 swapSize = UEngine::Get().GetRenderer()->GetSwapChainSize();
    float width = swapSize.X;
    float height = swapSize.Y;

    json::JSON j;
    j["VMinX"] = verticalHandle.Min.X/width;
    j["VMinY"] = verticalHandle.Min.Y/height;
    j["VMaxX"] = verticalHandle.Max.X/width;
    j["VMaxY"] = verticalHandle.Max.Y/height;

    j["HMinX"] = horitionalHandle.Min.X/width;
    j["HMinY"] = horitionalHandle.Min.Y/height;
    j["HMaxX"] = horitionalHandle.Max.X/width;
    j["HMaxY"] = horitionalHandle.Max.Y/height;
    j["type"] = "SSplitter2*2";
    return j;

}

ECameraViewMode::Type SSplitter2x2::GetCameraViewMode()
{
    return ECameraViewMode::Type::None;
}

bool SSplitter2x2::ClampVertical(FVector2 mouseDelta)
{
    FVector2 swapSize = UEngine::Get().GetRenderer()->GetSwapChainSize();
    float width = swapSize.X;
    float height = swapSize.Y;

    if ((verticalHandle.Min.Y + mouseDelta.Y) < minHeight) return false;
    if ((verticalHandle.Max.Y + mouseDelta.Y) > height - minHeight) return false;

    return true;
}

bool SSplitter2x2::ClampHorizontal(FVector2 mouseDleta)
{
    FVector2 swapSize = UEngine::Get().GetRenderer()->GetSwapChainSize();
    float width = swapSize.X;
    float height = swapSize.Y;

    float nextMousePos = horitionalHandle.Min.X + mouseDleta.X;
    if (nextMousePos < minWidth) return false;
    if (nextMousePos > width - minWidth) return false;

    return true;
}

