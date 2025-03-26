#pragma once
#include "Vector.h"
class FString;

struct FRect
{
	FRect();
	~FRect();
	FRect(float minX, float minY, float maxX, float maxY);
	FVector2 Min;
	FVector2 Max;    

	bool Contains(const FVector2& mousePos) const;
	void ResizeRatio(float widthRatio,float heightRatio);
	FString ToFString() const;
};

