#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "math/math.h"

namespace ray
{
	class Level;
	class LevelManager;

	using LevelPtr = std::shared_ptr<Level>;
	using LevelManagerPtr = std::shared_ptr<LevelManager>;

	class Level
	{
	public:
		Level() = delete;
		Level(const char* data, const Vec2i& size, const Vec2i& sizeInCells, int32_t cellSize);

		bool IsPointWithinBounds(const Vec2& point) const;
		bool IsCellWithinBounds(const Vec2i& cell) const;
		bool IsSolidWall(const Vec2i& cell) const;
		bool IsSolidWall(const Vec2& pos) const;
		char GetAt(int x, int y) const;
		Vec2i GetSize() const { return m_Size; }
		Vec2i GetSizeInCells() const { return m_SizeInCells; }
		int32_t GetCellSize() const { return m_CellSize; }
		float GetWallSize(const Vec2i cell) const;
		float GetBaseWallHeight() const { return m_BaseWallHeight; }

	private:
		std::string m_Data;
		Vec2i m_Size;
		Vec2i m_SizeInCells;
		int32_t m_CellSize;
		float m_BaseWallHeight;
	};

	class LevelManager
	{
	public:
		~LevelManager();

		static LevelManagerPtr GetInstance();
		LevelPtr LoadLevel(const std::string& path);
		void StartLevel(const LevelPtr level);
		LevelPtr GetCurrentLevel();

	private:
		explicit LevelManager();

		std::string MirrorDataString(const std::string& src, uint32_t levelWidth, uint32_t levelHeight);

		static std::shared_ptr<LevelManager> ms_Instance;
		std::unordered_map<std::string, LevelPtr> m_Levels;
		LevelPtr m_CurrentLevel;
	};
}

