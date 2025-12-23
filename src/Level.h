#pragma once

#include <string>
#include <cstdint>
#include "math/math.h"

class Level
{
public:
	Level(const char* data, const IVec2& size, int32_t cellSize);

	bool IsCellWithinBounds(const IVec2& cell) const;
	bool IsSolidWall(const IVec2& cell) const;
	bool IsSolidWall(const Vec2& pos) const;
	char GetAt(int x, int y) const;
	int32_t GetCellSize() const { return m_CellSize; }

private:
	std::string m_Data;
	IVec2 m_Size;
	int32_t m_CellSize;
};

