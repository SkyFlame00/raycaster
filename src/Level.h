#pragma once

#include <string>
#include <cstdint>
#include "math/math.h"

class Level
{
public:
	Level(const char* data, const Vec2i& size, int32_t cellSize);

	bool IsPointWithinBounds(const Vec2& point) const;
	bool IsCellWithinBounds(const Vec2i& cell) const;
	bool IsSolidWall(const Vec2i& cell) const;
	bool IsSolidWall(const Vec2& pos) const;
	char GetAt(int x, int y) const;
	Vec2i GetSize() const { return m_Size; }
	int32_t GetCellSize() const { return m_CellSize; }
	float GetWallSize(const Vec2i cell) const;
	float GetBaseWallHeight() const { return m_BaseWallHeight; }

private:
	std::string m_Data;
	Vec2i m_Size;
	int32_t m_CellSize;
	float m_BaseWallHeight;
};

