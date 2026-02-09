#include "Level.h"

#include <iostream>
#include <fstream>

#include "constants.h"

namespace ray
{
	Level::Level(const char* data, const Vec2i& size, const Vec2i& sizeInCells, int32_t cellSize)
		: m_Data(data)
	    , m_Size(size)
	    , m_SizeInCells(sizeInCells)
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

	// ===========================================================

	std::shared_ptr<LevelManager> LevelManager::ms_Instance = nullptr;

	LevelManager::LevelManager()
		: m_Levels(0)
		, m_CurrentLevel(nullptr)
	{
	}

	LevelManager::~LevelManager()
	{
	}

	LevelManagerPtr LevelManager::GetInstance()
	{
		if (ms_Instance == nullptr)
		{
			ms_Instance = std::shared_ptr<LevelManager>(new LevelManager());
		}

		return ms_Instance;
	}

	LevelPtr LevelManager::LoadLevel(const std::string& path)
	{
		std::ifstream file(path);

		if (!file.is_open())
		{
			std::printf("Unable to open file %s", path.c_str());
			return nullptr;
		}

		std::string data;
		std::string line;
		uint32_t levelWidth = 0;
		uint32_t levelHeight = 0;
		while (std::getline(file, line))
		{
			data += line;
			levelWidth = std::max(levelWidth, static_cast<uint32_t>(line.length()));
			levelHeight++;
		}

		file.close();

		data = MirrorDataString(data, levelWidth, levelHeight);

		uint32_t cellSize = 64;
		Vec2i levelSizeInCells = { (int32_t)levelWidth, (int32_t)levelHeight };
		Vec2i levelSize = { (int32_t)(levelWidth * cellSize), (int32_t)(levelHeight * cellSize) };
		LevelPtr level = std::make_shared<Level>(data.c_str(), levelSize, levelSizeInCells, cellSize);

		m_Levels[path] = level;

		return level;
	}

	std::string LevelManager::MirrorDataString(const std::string& src, uint32_t levelWidth, uint32_t levelHeight)
	{
		std::string dst;
		dst.resize(src.length());

		for (uint32_t row = 0; row < levelHeight; ++row)
		{
			for (uint32_t col = 0; col < levelWidth; ++col)
			{
				dst[(levelHeight - 1 - row) * levelWidth + col] = src[row * levelWidth + col];
			}
		}

		return dst;
	}

	void LevelManager::StartLevel(const LevelPtr level)
	{
		m_CurrentLevel = level;
	}

	LevelPtr LevelManager::GetCurrentLevel()
	{
		return m_CurrentLevel;
	}
}

