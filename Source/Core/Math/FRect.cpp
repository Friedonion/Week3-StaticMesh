#include "FRect.h"
#include "Core/HAL/PlatformType.h"
#include "Core/Container/String.h"
FRect::FRect()
{
}

FRect::~FRect()
{
}

FRect::FRect(float minX, float minY, float maxX, float maxY)
{
	Min = FVector2(minX, minY);
	Max = FVector2(maxX, maxY);
}

bool FRect::Contains(const FVector2& mousePos) const
{
	return (mousePos.X >= Min.X && mousePos.X <= Max.X &&
		mousePos.Y >= Min.Y && mousePos.Y <= Max.Y);
}

void FRect::ResizeRatio(float widthRatio, float heightRatio)
{
	Min.X *= widthRatio;
	Max.X *= widthRatio;
	Min.Y *= heightRatio;
	Max.Y *= heightRatio;
}

FString FRect::ToFString() const
{
	return TEXT("Min: (") +
		FString::SanitizeFloat(Min.X) + TEXT(", ") +
		FString::SanitizeFloat(Min.Y) + TEXT("), Max: (") +
		FString::SanitizeFloat(Max.X) + TEXT(", ") +
		FString::SanitizeFloat(Max.Y) + TEXT(")");
}
