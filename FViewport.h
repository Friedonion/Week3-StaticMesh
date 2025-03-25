#pragma once
#include "Core/HAL/PlatformType.h"
#include "ISlateViewport.h"
#include "Core/Math/FRect.h"

class ID3D11RasterizerState;
class SWindow;

namespace ECameraViewMode { enum class Type : uint8; }
namespace EViewport { enum class Position : uint8 ; }

class FViewport : public ISlateViewport
{
public:
	FViewport(FRect rect);
	virtual ~FViewport();
	//void SetViewport();
	void SetViewportRendering();
	virtual void ChangeMainCamera() override;
	virtual void Resize(FRect rect) override;
	virtual void SetActiveFullViewport() override;
	virtual EViewport::Position GetViewportPos() override;
	virtual ECameraViewMode::Type GetCameraViewMode() override;

	EViewport::Position GetViewportPosType();
	void SetViewportPosType(EViewport::Position pos);
	ECameraViewMode::Type GetCameraType();
	void SetCamera(ECameraViewMode::Type _cameraType);
	const FRect& GetRect();
private:
	SWindow* refSWindow;
	ECameraViewMode::Type cameraType;
	FRect Rect;
	EViewport::Position viewportPos;


	//ID3D11RasterizerState* rasterizerState;
};

