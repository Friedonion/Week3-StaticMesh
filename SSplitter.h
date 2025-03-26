#pragma once
#include "SWindow.h"

class SSplitter :
    public SWindow
{
public:
    virtual bool isHover(FVector2 coord) const override;
    virtual void OnFocus() override;
    virtual bool isClicked(FVector2 mousePos)  override;
    virtual bool OnMouseDrag(FVector2 mouseDelta) ;
    virtual bool OnMouseUp()  override;
    virtual void ScreenResize(float resizeWidthRatio, float resizeHeightRatio) override;
    virtual json::JSON ToJSON() override;
    SWindow* SideLT;
    SWindow* SideRB;
    FRect Handle;
};


class SSplitter2x2 : public SWindow
{
public:
    virtual bool isHover(FVector2 coord) const override;
    virtual void OnFocus() override;
    virtual bool isClicked(FVector2 mousePos)  override;
    virtual bool OnMouseDrag(FVector2 mouseDelta) ;
    virtual bool OnMouseUp()  override;
    virtual void ScreenResize(float resizeWidthRatio, float resizeHeightRatio) override;
    virtual json::JSON ToJSON() override;
    virtual json::JSON ToJSONRatio() override;
    virtual ECameraViewMode::Type GetCameraViewMode() override;
    SWindow* SideLT;
    SWindow* SideLB;
    SWindow* SideRB;
    SWindow* SideRT;
    FRect horitionalHandle;
    FRect verticalHandle;
private:
    bool bisHoriClicked = false;
    bool bisVertiClicked = false;

    const float minWidth = 300;
    const float minHeight = 300;

    bool ClampVertical(FVector2 mouseDelta);
    bool ClampHorizontal(FVector2 mouseDleta);
};