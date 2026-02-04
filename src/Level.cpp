#include "Level.h"
#include "constants.h"

Level::Level(const char* data, const Vec2i& size, int32_t cellSize)
	: m_Data(data)
	, m_Size(size)
	, m_CellSize(cellSize)
	, m_BaseWallHeight(64.0f)
{
}

bool Level::IsPointWithinBounds(const Vec2& point) const
{
	return 0 <= point.x && point.x < m_Size.x
		&& 0 <= point.y && point.y < m_Size.y;
}

bool Level::IsCellWithinBounds(const Vec2i& cell) const
{
	return 0 <= cell.x && cell.x <= m_Size.x
		&& 0 <= cell.y && cell.y <= m_Size.y;
}

bool Level::IsSolidWall(const Vec2i& cell) const
{
	const char wallType = GetAt(cell.x, cell.y);
	return wallType == '1' || wallType == '2' || wallType == '3'
		|| wallType == '4' || wallType == '5' || wallType == '9';
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

float Level::GetWallSize(const Vec2i cell) const
{
	const char wallType = GetAt(cell.x, cell.y);
	float size;

	switch (wallType)
	{
		case '1':
			size = m_BaseWallHeight;
			break;
		case '2':
			size = m_BaseWallHeight * 2;
			break;
		case '3':
			size = m_BaseWallHeight * 3;
			break;
		case '4':
			size = m_BaseWallHeight * 4;
			break;
		case '5':
			size = m_BaseWallHeight * 5;
			break;
		case '9':
			size = m_BaseWallHeight * 9;
			break;
		default:
			size = 0.0f;
	}

	return size;
}

