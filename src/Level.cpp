#include "Level.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "constants.h"

namespace ray
{
	namespace
	{
		std::string MirrorDataString(const std::string& src, uint32_t levelWidth, uint32_t levelHeight)
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
	}

	Level::Level(const std::string& data, int32_t cellSize)
	    : m_CellSize(cellSize)
	    , m_BaseWallHeight(64.0f)
	{
		Init(data);
	}

	void Level::Init(const std::string& data)
	{
		ParseLevelData(data);
	}

	bool Level::IsPointWithinBounds(const Vec2& point) const
	{
		return 0 <= point.x && point.x < m_Size.x
			&& 0 <= point.y && point.y < m_Size.y;
	}

	bool Level::IsCellWithinBounds(const Vec2i& cell) const
	{
		return 0 <= cell.x && cell.x < m_SizeInCells.x
			&& 0 <= cell.y && cell.y < m_SizeInCells.y;
	}

	bool Level::IsSolidWall(const Vec2i& cell) const
	{
		const char wallType = GetAt(cell.x, cell.y);

		if (const CellTypeDef* def = GetCellTypeDef(wallType))
		{
			return def->height > 0;
		}

		return false;
		//return wallType == '1' || wallType == '2' || wallType == '3'
		//	|| wallType == '4' || wallType == '5' || wallType == '9'
		//	|| wallType == 'q';
	}

	bool Level::IsSolidWall(const Vec2& pos) const
	{
		int32_t cellX = pos.x / (float)m_CellSize;
		int32_t cellY = pos.y / (float)m_CellSize;
		return IsSolidWall(Vec2i({ cellX, cellY }));
	}

	char Level::GetAt(const Vec2i& vec) const
	{
		return GetAt(vec.x, vec.y);
	}

	char Level::GetAt(int x, int y) const
	{
		return m_MapData[m_SizeInCells.x * y + x];
	}

	char Level::GetAt(const Vec2& vec) const
	{
		int32_t x = static_cast<int32_t>(vec.x) / m_CellSize;
		int32_t y = static_cast<int32_t>(vec.y) / m_CellSize;

		return GetAt(x, y);
	}

	float Level::GetWallSize(const Vec2i cell) const
	{
		const char wallType = GetAt(cell.x, cell.y);
		if (const CellTypeDef* def = GetCellTypeDef(wallType))
		{
			return def->height * m_BaseWallHeight;
		}
		return m_BaseWallHeight;
	}

	void Level::ProcessDefItem(const DefItem& defItem)
	{
		CellTypeDef cellTypeDef;
		TextureManagerPtr textureManager = TextureManager::GetInstance();

		switch (defItem.type)
		{
			case DefItemType::CELL:
				if (!defItem.cellType)
				{
					// report
					return;
				}

				cellTypeDef.cellType = defItem.cellType;
				cellTypeDef.wallTexture = textureManager->GetTexture(defItem.texturePath);
				cellTypeDef.floorTexture = textureManager->GetTexture(defItem.floorTexturePath);
				cellTypeDef.paletteColor = defItem.paletteColor;
				cellTypeDef.height = defItem.height;
				cellTypeDef.offsetX = defItem.offsetX;
				cellTypeDef.offsetY = defItem.offsetY;
				cellTypeDef.scaleX = defItem.scaleX;
				cellTypeDef.scaleY = defItem.scaleY;
				
				m_CellTypeDefinitions[defItem.cellType] = cellTypeDef;

				break;

			case DefItemType::UNDEFINED:
			default:
				// error
				break;
		}
	}

	bool Level::ParseLevelData(const std::string& data)
	{
		enum class ParsingStage
		{
			PARSING_HEADER,
			PARSING_DEF_ITEM,
			PARSING_BODY
		};

		ParsingStage stage = ParsingStage::PARSING_HEADER;
		bool ok = true;
		DefItem defItem;
		uint32_t nline = 0;
		std::string errorMessage;
		std::size_t pos = 0;
		std::size_t cols = m_SizeInCells.y;
		std::size_t len = data.length();
		int32_t levelWidth = 0;
		int32_t levelHeight = 0;
		std::string mapData;
		std::string line;
		std::istringstream stringstream{ data };

		defItem.Reset(); // TODO: make a normal constructor instead of relying on Reset

		while (std::getline(stringstream, line))
		{
			auto isEmptyString = [](const std::string& str)
			{
				for (char ch : str)
					if (!std::isspace(ch))
						return false;
				return true;
			};

			if (isEmptyString(line))
			{
			}	
			else if (stage == ParsingStage::PARSING_HEADER)
			{
				if (line == "HEADER")
				{
					stage = ParsingStage::PARSING_DEF_ITEM;
				}
				else
				{
					ok = false;
					errorMessage = "There should be the \"HEADER\" token in the beginning of the file";
				}
			}
			else if (stage == ParsingStage::PARSING_DEF_ITEM)
			{
				if (line == "BODY")
				{
					stage = ParsingStage::PARSING_BODY;
					ProcessDefItem(defItem);
					// mapData.reserve(...) - reserve space ahead of time
				}
				else
				{
					uint32_t nhyphen = 0;
					for (char c : line)
					{
						if (c != '-')
							break;
						nhyphen++;
					}

					const bool bItemLevel0 = nhyphen == 1;
					const bool bItemLevel1 = nhyphen == 2;
					if (bItemLevel0)
					{
						if (defItem.writing)
						{
							ProcessDefItem(defItem);
							defItem.Reset();
						}

						if (line == "-cell")
						{
							defItem.writing = true;
							defItem.type = DefItemType::CELL;
						}
						else
						{
							ok = false;
							errorMessage = "Unrecognized item of level 0";
						}
					}
					else if (bItemLevel1)
					{
						// TODO: rewrite this function to extract the first token and its value instead of guessing
						auto parseToken = [](const std::string& line, const std::string& token, std::string& stringValue)
						{
							bool bStartsWith = true;
							for (size_t i = 0; i < token.length(); i++)
							{
								if (line[i] != token[i])
								{
									bStartsWith = false;
									break;
								}
							}

							if (bStartsWith)
							{
								if (line[token.length()] == '=')
								{
									stringValue = line.substr(token.length() + 1);
									return true;
								}
								else
								{
									// error
								}
							}

							return false;
						};

						const std::string TEXTURE_PATH_TOKEN = "--texture_path";
						const std::string HEIGHT_TOKEN = "--height";
						const std::string FLOOR_TEXTURE_PATH_TOKEN = "--floor_texture_path";
						const std::string COORDS_TOKEN = "--coords";
						const std::string CHAR_TOKEN = "--char";
						const std::string OFFSET_X_TOKEN = "--offset_x";
						const std::string OFFSET_Y_TOKEN = "--offset_y";
						const std::string SCALE_X_TOKEN = "--scale_x";
						const std::string SCALE_Y_TOKEN = "--scale_y";
						const std::string PALETTE_COLOR = "--palette_color";
						std::string stringValue;
						if (parseToken(line, TEXTURE_PATH_TOKEN, stringValue))
						{
							defItem.texturePath = stringValue;
						}
						else if (parseToken(line, HEIGHT_TOKEN, stringValue))
						{
							defItem.height = static_cast<uint32_t>(std::stoi(stringValue));
						}
						else if (parseToken(line, FLOOR_TEXTURE_PATH_TOKEN, stringValue))
						{
							defItem.floorTexturePath = stringValue;
						}
						else if (parseToken(line, COORDS_TOKEN, stringValue))
						{
							// find delimiter and extract two numbers
						}
						else if (parseToken(line, CHAR_TOKEN, stringValue))
						{
							defItem.cellType = stringValue[0];
						}
						else if (parseToken(line, OFFSET_X_TOKEN, stringValue))
						{
							defItem.offsetX = std::stoi(stringValue);
						}
						else if (parseToken(line, OFFSET_Y_TOKEN, stringValue))
						{
							defItem.offsetY = std::stoi(stringValue);
						}
						else if (parseToken(line, SCALE_X_TOKEN, stringValue))
						{
							defItem.scaleX = std::stof(stringValue);
						}
						else if (parseToken(line, SCALE_Y_TOKEN, stringValue))
						{
							defItem.scaleY = std::stof(stringValue);
						}
						else if (parseToken(line, PALETTE_COLOR, stringValue))
						{
							defItem.paletteColor = stringValue;
						}
						else
						{
							ok = false;
							errorMessage = "Unable to parse item of level 2";
						}
					}
					else
					{
						ok = false;
						errorMessage = "Unrecognized pattern encountered when parsing header";
					}
				}
			}
			else if (stage == ParsingStage::PARSING_BODY)
			{
				// determine player spawn
				// save map data to in the level instance
				mapData += line;
				levelWidth = std::max(levelWidth, (int32_t)line.length());
				levelHeight++;

				for (char ch : line)
				{
					if (ch == 'P')
					{
						// place player
						break;
					}
				}
			}

			if (!ok)
				break;

			nline++;
			pos += cols;
		}

		m_MapData = MirrorDataString(mapData, levelWidth, levelHeight);
		m_SizeInCells = { levelWidth, levelHeight };
		m_Size = { levelWidth * static_cast<int32_t>(m_CellSize), levelHeight * static_cast<int32_t>(m_CellSize) };

		if (!ok)
		{
			std::printf("ParseLevelData: error at line %d - %s", nline, errorMessage.c_str());
		}

		return ok;
	}

	const CellTypeDef* Level::GetCellTypeDef(uint8_t ch) const
	{
		auto iter = m_CellTypeDefinitions.find(ch);
		if (iter != m_CellTypeDefinitions.end())
			return &iter->second;
		return nullptr;
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
			//data += line;
			data += (line + '\n');
			levelWidth = std::max(levelWidth, static_cast<uint32_t>(line.length()));
			levelHeight++;
		}

		file.close();

		//data = MirrorDataString(data, levelWidth, levelHeight);

		uint32_t cellSize = 64;
		//Vec2i levelSizeInCells = { (int32_t)levelWidth, (int32_t)levelHeight };
		//Vec2i levelSize = { (int32_t)(levelWidth * cellSize), (int32_t)(levelHeight * cellSize) };
		//LevelPtr level = std::make_shared<Level>(data, levelSize, levelSizeInCells, cellSize);
		LevelPtr level = std::make_shared<Level>(data, cellSize);

		m_Levels[path] = level;

		return level;
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

