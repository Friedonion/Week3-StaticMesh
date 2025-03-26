#pragma once
#include "Core/HAL/PlatformType.h"
class FRect;

namespace ECameraViewMode { enum class Type : uint8; }
namespace EViewport { enum class Position : uint8; }

class ISlateViewport
{
public:
	virtual void ChangeMainCamera() = 0;
	virtual void Resize(FRect rect) = 0;
	virtual void SetActiveFullViewport() = 0;
	virtual EViewport::Position GetViewportPos() = 0;
	virtual ECameraViewMode::Type GetCameraViewMode() = 0;
};

