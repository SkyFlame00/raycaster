#include "Level.h"
#include "constants.h"

Level::Level(const char* data, const IVec2& size, int32_t cellSize)
	: m_Data(data)
	, m_Size(size)
	, m_CellSize(cellSize)
{
}

bool Level::IsCellWithinBounds(const IVec2& cell) const
{
	return 0 <= cell.x && cell.x <= m_Size.x
		&& 0 <= cell.y && cell.y <= m_Size.y;
}

bool Level::IsSolidWall(const IVec2& cell) const
{
	return GetAt(cell.x, cell.y) == '1';
}

bool Level::IsSolidWall(const Vec2& pos) const
{
	int32_t cellX = pos.x / (float)m_CellSize;
	int32_t cellY = pos.y / (float)m_CellSize;
	return IsSolidWall(IVec2({ cellX, cellY }));
}

char Level::GetAt(int x, int y) const
{
	return m_Data[LEVEL_COLS*y + x];
}

