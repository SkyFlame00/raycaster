#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "math/math.h"
#include "TextureManager.h"

namespace ray
{
	class Level;
	class LevelManager;

	using LevelPtr = std::shared_ptr<Level>;
	using LevelManagerPtr = std::shared_ptr<LevelManager>;

	enum class DefItemType
	{
		UNDEFINED,
		CELL
	};

	struct DefItem
	{
		// TODO: it might make sense to use unions here

		DefItemType type;
		bool writing;
		uint8_t cellType;
		std::string texturePath;
		std::string floorTexturePath;
		uint32_t height;

		void Reset()
		{
			type = DefItemType::UNDEFINED;
			writing = false;
			cellType = 0;
			texturePath = "";
			floorTexturePath = "";
			height = 0u;
		}
	};

	struct CellTypeDef
	{
		CellTypeDef()
			: cellType(0)
			, wallTexture(nullptr)
			, floorTexture(nullptr)
			, height(0u)
		{}

		uint8_t cellType;
		TexturePtr wallTexture;
		TexturePtr floorTexture;
		uint32_t height;
	};

	struct CellDefinition
	{
	};

	class Level
	{
	public:
		Level() = delete;
		Level(const std::string& data, int32_t cellSize);

		void Init(const std::string& data);
		bool IsPointWithinBounds(const Vec2& point) const;
		bool IsCellWithinBounds(const Vec2i& cell) const;
		bool IsSolidWall(const Vec2i& cell) const;
		bool IsSolidWall(const Vec2& pos) const;
		char GetAt(const Vec2i& vec) const;
		char GetAt(int x, int y) const;
		Vec2i GetSize() const { return m_Size; }
		Vec2i GetSizeInCells() const { return m_SizeInCells; }
		int32_t GetCellSize() const { return m_CellSize; }
		float GetWallSize(const Vec2i cell) const;
		float GetBaseWallHeight() const { return m_BaseWallHeight; }
		bool ParseLevelData(const std::string& data);
		void ProcessDefItem(const DefItem& defItem);
		const CellTypeDef* GetCellTypeDef(uint8_t ch) const;

	private:
		std::string m_MapData;
		Vec2i m_Size;
		Vec2i m_SizeInCells;
		int32_t m_CellSize;
		float m_BaseWallHeight;
		std::unordered_map<uint8_t, CellTypeDef> m_CellTypeDefinitions;
		//std::unordered_map<std::string, CellDef> m_CellDefinitions;
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

