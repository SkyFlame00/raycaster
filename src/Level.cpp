#include "Level.h"
#include "constants.h"

Level::Level(const char* data, const Vec2i& size, int32_t cellSize)
	: m_Data(data)
	, m_Size(size)
	, m_CellSize(cellSize)
{
}

bool Level::IsCellWithinBounds(const Vec2i& cell) const
{
	return 0 <= cell.x && cell.x <= m_Size.x
		&& 0 <= cell.y && cell.y <= m_Size.y;
}

bool Level::IsSolidWall(const Vec2i& cell) const
{
	const char wallType = GetAt(cell.x, cell.y);
	return wallType == '1' || wallType == '2';
}

bool Level::IsSolidWall(const Vec2& pos) const
{
	int32_t cellX = pos.x / (float)m_CellSize;
	int32_t cellY = pos.y / (float)m_CellSize;
	return IsSolidWall(Vec2i({ cellX, cellY }));
}

char Level::GetAt(int x, int y) const
{
	return m_Data[LEVEL_COLS*y + x];
}

