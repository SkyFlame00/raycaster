#pragma once

#include <string>
#include <cstdint>
#include "math/math.h"

class Level
{
public:
	Level(const char* data, const Vec2i& size, int32_t cellSize);

	bool IsCellWithinBounds(const Vec2i& cell) const;
	bool IsSolidWall(const Vec2i& cell) const;
	bool IsSolidWall(const Vec2& pos) const;
	char GetAt(int x, int y) const;
	Vec2i GetSize() const { return m_Size; }
	int32_t GetCellSize() const { return m_CellSize; }

private:
	std::string m_Data;
	Vec2i m_Size;
	int32_t m_CellSize;
};

