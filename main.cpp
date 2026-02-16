#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <cassert>

#include <stb/stb_image.h>

#include "constants.h"
#include "Platform.h"
#include "Window.h"
#include "math/math.h"
#include "game_algorithms.h"
#include "Level.h"
#include "TextureManager.h"

const std::string BKRED_1 = "./assets/textures/bkred_1.png";
const std::string BRIK_3 = "./assets/textures/brik_3.png";
const std::string BRKS_1 = "./assets/textures/brks_1.png";
const std::string BRKS_00 = "./assets/textures/brks_00.png";
const std::string WALL51_1 = "./assets/textures/wall52_1.png";
const std::string BUILDING_1 = "./assets/textures/building-1.png";
const std::string BUILDING_2 = "./assets/textures/building-2.png";

const std::string LEVEL0 = "./assets/levels/test0.leveldata";
const std::string LEVEL1 = "./assets/levels/test1.leveldata";
const std::string LEVEL2 = "./assets/levels/test2.leveldata";

class Player
{
public:
	float m_ViewAngleDeg;
	float m_MoveAngleDeg;
	float m_Speed = 0.0f;
	Vec2 m_Pos;
};

const int g_CellSize = 64; // TODO: remove
Player g_Player;
int g_FOV = 60;

void SpawnPlayer(Player& player)
{
	ray::LevelPtr level = ray::LevelManager::GetInstance()->GetCurrentLevel();
	const Vec2i levelSizeInCells = level->GetSizeInCells();

	for (int row = 0; row < levelSizeInCells.y; row++)
	{
		for (int col = 0; col < levelSizeInCells.x; col++)
		{
			if (level->GetAt(row, col) == 'P')
			{
				float cellCenter = g_CellSize / 2.0f;
				player.m_Pos.x = g_CellSize * col + cellCenter;
				player.m_Pos.y = g_CellSize * row + cellCenter;
				return;
			}
		}
	}

	std::cout << "Player was not found in the level";
}
 
int8_t ShadeColor(int8_t color, float dist)
{
	const float maxDist = 1024.0f;
	const int8_t colorShift = 100;

	const float factor = std::min(dist, maxDist) / maxDist;
	const int8_t shadedColor = std::max(color - (int8_t)(factor * colorShift), 0);

	return shadedColor;
}

int32_t ExpandToRgba(int8_t value)
{
	return (0xFF000000 | (value << 16) | (value << 8) | value);
}

uint32_t ApplyBrightness(uint32_t color, float intensity)
{
	uint8_t a = (uint8_t)(color >> 24);
	uint8_t r = (uint8_t)(color >> 16);
	uint8_t g = (uint8_t)(color >> 8);
	uint8_t b = (uint8_t)color;

	uint8_t min = 0;
	uint8_t max = 255;
	r = std::clamp((uint8_t)(r * intensity), min, max);
	g = std::clamp((uint8_t)(g * intensity), min, max);
	b = std::clamp((uint8_t)(b * intensity), min, max);

	uint32_t result = (a << 24) | (r << 16) | (g << 8) | b;

	return result;
}

float GetShadingIntensity(float dist)
{
	int8_t maxShade = 90;
	float maxDist = 1024.0f;
	float factor = std::min(dist, maxDist) / maxDist;
	factor *= 1.4f;
	factor = std::min(factor, 1.0f);
	//int8_t intensity = maxShade * factor;
	float intensity = 1.0f - factor;
	return intensity;
}

void DrawWall(uint32_t* framebuffer, ray::TexturePtr texture, float heightInBlocks, const Vec2& scale, const Vec2& offset, int32_t beginY, int32_t endY, uint32_t screenBeginY, uint32_t screenEndY, float xcoord, int32_t strip, float dist)
{
	// (0, 0) is at the top-left of both the screen and texture
	float blockSizeY = ((float)endY - beginY) / heightInBlocks;
	float gameUnitSizeY = blockSizeY / g_CellSize;
	float texWidthInGameUnits = g_CellSize * scale.x;
	float texHeightInGameUnits = blockSizeY * scale.y;
	float textureX = fmodf(xcoord + offset.x, texWidthInGameUnits) / texWidthInGameUnits;

	for (int32_t i = screenBeginY; i < screenEndY; i++)
	{
		float ycoord = i - beginY;
		float offsetY = offset.y * gameUnitSizeY;
		float textureY = fmodf(ycoord + offsetY, texHeightInGameUnits) / texHeightInGameUnits;
		uint32_t color = texture->Sample(textureX, textureY);

		float intensity = GetShadingIntensity(dist);
		//color = ApplyBrightness(color, intensity);

		framebuffer[i * SCREEN_WIDTH + strip] = color;
	}
}

uint32_t GetFloorColor(ray::TexturePtr texture, float worldX, float worldY)
{
	float texHeightRatio = texture->GetHeight() / (float)texture->GetWidth();
	float texWidthUnits = g_CellSize / texHeightRatio;
	float textureX = fmodf((float)worldX, texWidthUnits) / texWidthUnits;
	float textureY = fmodf((float)worldY, g_CellSize) / (float)g_CellSize;

	uint32_t color = texture->Sample(textureX, textureY);

	return color;
}

void DrawFloor(uint32_t* framebuffer, ray::TexturePtr texture, float distToProjPlane, float worldAngle, float localAngle, uint32_t strip, uint32_t wallHeight, uint32_t beginY, uint32_t endY)
{
	for (int i = beginY; i < endY; i++)
	{
		// This code below makes a ray cast for every pixel. There are probably faster ways to do the same thing like horizontal line scan
		// Based on https://permadi.com/1996/05/ray-casting-tutorial-12/
		float halfWallHeight = wallHeight / 2.0f;
		float playerHeight = halfWallHeight;
		float rowDiff = i - SCREEN_HEIGHT / 2;

		// Vertical FOV is unknown but given wallHeight and the distance at which the wall exactly covers the screen's height (which is the same as wallHeight), we can find it
		//float halfVFOV = atanf(halfWallHeight / wallHeight);
		//float halfProjPlaneHeight = tanf(halfVFOV) / distToProjPlane;
		float halfProjPlaneHeight = (halfWallHeight / wallHeight) / distToProjPlane;
		float halfScreenHeight = SCREEN_HEIGHT / 2.0f;
		float heightPerPixel = halfProjPlaneHeight / halfScreenHeight;
		float distToRow = rowDiff * heightPerPixel;

		// distToFloor / distToProjPlane = playerHeight / distToRow
		float distToFloor = (playerHeight / distToRow) * distToProjPlane; // dist from player's feet to point P on floor

		float hyp = distToFloor / cosf(localAngle);

		float dx = hyp * cosf(worldAngle);
		float dy = hyp * sinf(worldAngle);

		float playerX = g_Player.m_Pos.x;
		float playerY = g_Player.m_Pos.y;

		float worldX = playerX + dx;
		float worldY = playerY + dy;

		uint32_t color = GetFloorColor(texture, worldX, worldY);
		//uint32_t color = 0xFFCCCCCC;
		float intensity = GetShadingIntensity(hyp);
		//color = ApplyBrightness(color, intensity);

		framebuffer[i * SCREEN_WIDTH + strip] = color;
	}
}

void Render(Uint32* framebuffer)
{
	// Define the distance to the projection plane to be 1. Everything else will be calculated from it and FOV
	float distToProjPlane = 1.0f;
	float halfFOVTan = tanf(ToRadians(g_FOV / 2.0f));
	if (NearZero(halfFOVTan)) // TODO: handle differently
		return;
	float halfProjPlaneWidth = halfFOVTan * distToProjPlane;
	float projPlaneWidth = halfProjPlaneWidth * 2;

	// The width of a single column
	float stripWidth = projPlaneWidth / SCREEN_WIDTH;
	ray::LevelPtr level = ray::LevelManager::GetInstance()->GetCurrentLevel();

	for (int strip = 0; strip < SCREEN_WIDTH; strip++)
	{
		// Get an angle at which a ray goes through the strip N to get equally spaced (or linear) columns.
		// Without this, the columns would look slightly non-linear even after the fish-eye effect correction.
		// See https://www.scottsmitelli.com/articles/we-can-fix-your-raycaster/
		float projPlaneX = halfProjPlaneWidth - strip * stripWidth;
		float localAngleRad = atanf(projPlaneX / distToProjPlane); // atan2f isn't needed since we use exactly I and IV quadrants
		float worldAngleRad = localAngleRad + ToRadians(g_Player.m_ViewAngleDeg);
		
		std::vector<ray::WallRaycastHit> hHits;
		std::vector<ray::WallRaycastHit> vHits;

		WallRaycastH(g_Player.m_Pos, worldAngleRad, *level, hHits);
		WallRaycastV(g_Player.m_Pos, worldAngleRad, *level, vHits);

		const bool hasHorIntersection = hHits.size() > 0;
		const bool hasVerIntersection = vHits.size() > 0;
		if (!hasHorIntersection && !hasVerIntersection)
		{
			// sth went wrong: assert and exit
			std::printf("No intersection\n");
			return;
		}

		float maxWallSize = 0.0f;
		std::vector<ray::WallRaycastHit> hVisibleHits;
		for (const ray::WallRaycastHit& hit : hHits)
		{
			float wallSize = level->GetWallSize(hit.cell);

			if (maxWallSize < wallSize)
			{
				maxWallSize = wallSize;
				hVisibleHits.push_back(hit);
			}
		}

		maxWallSize = 0.0f;
		std::vector<ray::WallRaycastHit> vVisibleHits;
		for (const ray::WallRaycastHit& hit : vHits)
		{
			float wallSize = level->GetWallSize(hit.cell);

			if (maxWallSize < wallSize)
			{
				maxWallSize = wallSize;
				vVisibleHits.push_back(hit);
			}
		}

		maxWallSize = 0.0f;
		std::vector<ray::WallRaycastHit> visibleHits;
		auto hIter = hVisibleHits.begin();
		auto vIter = vVisibleHits.begin();
		while (hIter != hVisibleHits.end() || vIter != vVisibleHits.end())
		{
			using HitVector = std::vector<ray::WallRaycastHit>;

			auto nextVisibleHit = [maxWallSize, level](const HitVector& vec, HitVector::iterator iter)
			{
				for (; iter != vec.end(); ++iter)
				{
					const float wallSize = level->GetWallSize(iter->cell);
					if (maxWallSize < wallSize)
						break;
				}
				return iter;
			};

			hIter = nextVisibleHit(hVisibleHits, hIter);
			vIter = nextVisibleHit(vVisibleHits, vIter);

			const bool hDraw = hIter != hVisibleHits.end();
			const bool vDraw = vIter != vVisibleHits.end();
			const float hWallSize = hDraw ? level->GetWallSize(hIter->cell) : 0;
			const float vWallSize = vDraw ? level->GetWallSize(vIter->cell) : 0;
			maxWallSize = std::max(maxWallSize, std::max(hWallSize, vWallSize));

			if (hDraw && vDraw)
			{
				const float hDist = Distance(hIter->point, g_Player.m_Pos);
				const float vDist = Distance(vIter->point, g_Player.m_Pos);

				if (NearlyEqual(hDist, vDist))
					std::printf("Horizontal and vertical hits are at the same distance.\n");

				if (hDist < vDist)
				{
					visibleHits.push_back(*hIter);
					visibleHits.push_back(*vIter);
				}
				else
				{
					visibleHits.push_back(*vIter);
					visibleHits.push_back(*hIter);
				}

				hIter++;
				vIter++;
			}
			else if (hDraw)
			{
				visibleHits.push_back(*hIter);
				hIter++;
			}
			else if (vDraw)
			{
				visibleHits.push_back(*vIter);
				vIter++;
			}
		}

		const float distToProjPlane = 1.0f;
		const float baseWallHeight = level->GetBaseWallHeight();

		uint32_t maxTopWallHeightPx = 0;
		uint32_t maxBottomWallHeightPx = 0;
		for (auto iter = visibleHits.rbegin(); iter != visibleHits.rend(); iter++)
		{
			const ray::WallRaycastHit& hit = *iter;

			// find distance to the closest intersection
			float dist = Distance(hit.point, g_Player.m_Pos);
			int cellX = hit.cell.x;
			int cellY = hit.cell.y;
			bool hCase = hit.isHor;

			// Make a fish eye effect correction
			float correction = cosf(localAngleRad);
			float correctedDist = correction * dist;
			if (1)
				correctedDist = std::round(correctedDist); // round to prevent the "teeth" (a texel issue)
			if (NearZero(correctedDist))
				correctedDist = 1.0f;

			// proj wall height / dist of player to proj plane = wall height / dist to wall
			// dist of player to proj plane = 1
			// proj wall height = wall height / dist to wall
			const float wallHeight = level->GetWallSize(hit.cell);
			float wallHeightNorm = (wallHeight / correctedDist) * distToProjPlane;

			// We ceil to the next integer to avoid an issue with floor texture mapping when a cast ray goes through the wall instead of below it.
			// This is due to one-pixel-difference which occurs due to integer truncation. Say, wallHeight is 5.4, so wallHeightPx will be 5.
			// Then, we'll use 5 in calculations which will produce slightly incorrect results (they become more pronounced with increasing distance) because we should've taken something bigger than 5.4.
			// Rounding (in our case - ceiling) also helps avoid a "teeth" effect when neighboring pixels differ by one texel.
			int wallHeightPx = wallHeightNorm * SCREEN_HEIGHT;

			float heightInBlocks = wallHeight / baseWallHeight;

			float baseWallHeightNorm = (baseWallHeight / correctedDist) * distToProjPlane;
			uint32_t bottomWallHeightPx = std::ceil((baseWallHeightNorm * SCREEN_HEIGHT) / 2.0f);
			uint32_t topWallHeightPx = wallHeightPx - bottomWallHeightPx;
			maxTopWallHeightPx = std::max(maxTopWallHeightPx, topWallHeightPx);
			maxBottomWallHeightPx = std::max(maxBottomWallHeightPx, bottomWallHeightPx);

			int32_t wallBeginY = SCREEN_HEIGHT / 2 - topWallHeightPx;
			int32_t wallEndY = SCREEN_HEIGHT / 2 + bottomWallHeightPx;
			uint32_t screenWallBeginY = static_cast<uint32_t>(std::max(0, wallBeginY));
			uint32_t screenWallEndY = static_cast<uint32_t>(std::min(wallEndY, SCREEN_HEIGHT));
			float xcoord = hCase ? hit.point.x : hit.point.y;

			//ray::TextureManagerPtr textureManager = ray::TextureManager::GetInstance();
			//ray::TexturePtr wallTexture = textureManager->GetTexture(BUILDING_1);
			//if (wallTexture == nullptr)
			//{
			//	return;
			//}

			uint8_t ch = level->GetAt(hit.cell);
			const ray::CellTypeDef* cellTypeDef = level->GetCellTypeDef(ch);
			if (cellTypeDef == nullptr)
			{
				return;
			}

			ray::TexturePtr wallTexture = cellTypeDef->wallTexture;
			if (wallTexture == nullptr)
			{
				return;
			}

			//Vec2 scale = {3.0f, 3.0f};
			Vec2 scale = {3.0f, 3.0f};
			Vec2 offset = {0.0f, 64.0f};
			DrawWall(framebuffer, wallTexture, heightInBlocks, scale, offset, wallBeginY, wallEndY, screenWallBeginY, screenWallEndY, xcoord, strip, correctedDist);
		}

		// draw the ceiling/sky
		const int32_t ceilingHeightPx = std::max(SCREEN_HEIGHT / 2 - (int32_t)maxTopWallHeightPx, 0);
		for (int i = 0; i < ceilingHeightPx; i++)
		{
			Uint32 color = 0xFF000000;
			framebuffer[i * SCREEN_WIDTH + strip] = color;
		}

		ray::TextureManagerPtr textureManager = ray::TextureManager::GetInstance();
		ray::TexturePtr floorTexture = textureManager->GetTexture(WALL51_1);
		if (floorTexture == nullptr)
		{
			return;
		}

		uint32_t floorBeginY = std::min(SCREEN_HEIGHT / 2 + (int32_t)maxBottomWallHeightPx, SCREEN_HEIGHT);
		uint32_t floorEndY = SCREEN_HEIGHT;
		if (floorBeginY < floorEndY)
			DrawFloor(framebuffer, floorTexture, distToProjPlane, worldAngleRad, localAngleRad, strip, baseWallHeight, floorBeginY, floorEndY);
	}
}

void HandleInput(float dt)
{
	SDL_PumpEvents();

	const Uint8* state = SDL_GetKeyboardState(nullptr);
	const float rotationSpeed = 180.0f; // 180 degrees per second
	const float moveSpeed = 200.0f * dt;

	g_Player.m_Speed = 0.0f;

	if (state[SDL_SCANCODE_A])
	{
		if (state[SDL_SCANCODE_LALT])
		{
			g_Player.m_Speed = moveSpeed;
			g_Player.m_MoveAngleDeg = NormalizeAngle(g_Player.m_ViewAngleDeg + 90.0f);
		}
		else
		{
			g_Player.m_ViewAngleDeg += dt * rotationSpeed;
			//std::cout << "View angle = " << g_Player.m_ViewAngleDeg << std::endl;
		}
	}

	if (state[SDL_SCANCODE_D])
	{
		if (state[SDL_SCANCODE_LALT])
		{
			g_Player.m_Speed = moveSpeed;
			g_Player.m_MoveAngleDeg = NormalizeAngle(g_Player.m_ViewAngleDeg - 90.0f);
		}
		else
		{
			g_Player.m_ViewAngleDeg -= dt * rotationSpeed;
			//std::cout << "View angle = " << g_Player.m_ViewAngleDeg << std::endl;
		}
	}

	if (state[SDL_SCANCODE_W])
	{
		g_Player.m_Speed = moveSpeed;
		g_Player.m_MoveAngleDeg = g_Player.m_ViewAngleDeg;
	}

	if (state[SDL_SCANCODE_S])
	{
		g_Player.m_Speed = moveSpeed;
		g_Player.m_MoveAngleDeg = NormalizeAngle(g_Player.m_ViewAngleDeg + 180.0f);
	}
}

void PhysicsFrame(float dt)
{
	if (0)
	{
		float speed = g_Player.m_Speed;
		float normAngleDeg = NormalizeAngle(g_Player.m_MoveAngleDeg);
		float angleRad = ToRadians(normAngleDeg);
		float dx = speed * cosf(angleRad);
		float dy = speed * sinf(angleRad);
		Vec2 pos = g_Player.m_Pos;
		Vec2 projPoint = { pos.x + dx, pos.y + dy };
		g_Player.m_Pos = projPoint;
		return;
	}

	float speed = g_Player.m_Speed;
	ray::LevelPtr level = ray::LevelManager::GetInstance()->GetCurrentLevel();
	if (!IsZero(speed))
	{
		float normAngleDeg = NormalizeAngle(g_Player.m_MoveAngleDeg);
		float angleRad = ToRadians(normAngleDeg);
		float dx = speed * cosf(angleRad);
		float dy = speed * sinf(angleRad);
		Vec2 pos = g_Player.m_Pos;
		Vec2 projPoint = { pos.x + dx, pos.y + dy };
		
		std::vector<ray::WallRaycastHit> hits;
		WallRaycast(pos, angleRad, *level, hits);

		const bool found = hits.size() > 0;
		if (!found)
		{
			// report an error
			return;
		}

		Vec2 wallPoint = hits[0].point;
		float distToProjPoint = (projPoint - pos).Length();
		float distToWallPoint = (wallPoint - pos).Length();

		float x = cosf(angleRad);
		float y = sinf(angleRad);
		Vec2 dir = { x, y };
		Vec2 dstPoint;
		if (distToProjPoint < distToWallPoint)
		{
			dstPoint = projPoint;
			// TODO: this should be removed once we start using a circle as the collider
			if (std::abs(distToProjPoint - distToWallPoint) < 5.0f)
				dstPoint = dstPoint - (dir * 5.0f); // move the player back a bit
		}
		else
		{
			// TODO: this should be removed once we start using a circle as the collider
			dstPoint = wallPoint - (dir * 5.0f); // move the player back a bit
		}

		g_Player.m_Pos = dstPoint;

		// TODO: remove debug code below
		if (0)
		{

			bool bPrevSolidWall = level->IsSolidWall(pos);
			int cellSize = level->GetCellSize();
			Vec2i oldCell = { (int)(pos.x / cellSize), (int)(pos.y / cellSize) };
			Vec2i newCell = { (int)(g_Player.m_Pos.x / cellSize), (int)(g_Player.m_Pos.y / cellSize) };

			bool bSolidWall = level->IsSolidWall(g_Player.m_Pos);
			if (bSolidWall)
				std::printf("We're inside a solid wall");
		}
	}
}

int main()
{
	std::shared_ptr<ray::Platform> platform = ray::Platform::CreatePlatform();
	if (platform == nullptr)
		return 1;

	std::shared_ptr<ray::Window> window = platform->CreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
	if (window == nullptr)
		return 1;

	bool running = true;
	int frame = 0;

	float targetFPS = 18.0f;
	uint64_t targetFrameDur = (uint64_t)(1000.0f / targetFPS);
	uint64_t lastTime = platform->GetElapsedMs();

	ray::TextureManagerPtr textureManager = ray::TextureManager::GetInstance();
	textureManager->LoadTexture(BKRED_1);
	textureManager->LoadTexture(BRIK_3);
	textureManager->LoadTexture(BRKS_1);
	textureManager->LoadTexture(BRKS_00);
	textureManager->LoadTexture(WALL51_1);
	textureManager->LoadTexture(BUILDING_1);
	textureManager->LoadTexture(BUILDING_2);

	ray::LevelManagerPtr levelManager = ray::LevelManager::GetInstance();
	ray::LevelPtr level0 = levelManager->LoadLevel(LEVEL0);
	ray::LevelPtr level2 = levelManager->LoadLevel(LEVEL2);
	levelManager->StartLevel(level2);

	SpawnPlayer(g_Player);

	while (running)
	{
		uint64_t currentTime = platform->GetElapsedMs();
		float dt = (currentTime - lastTime) / 1000.0f;
		uint64_t diff = currentTime - lastTime;
		lastTime = currentTime;

		uint64_t start = platform->GetPerformanceCounter();

		HandleInput(dt);
		PhysicsFrame(dt);

		uint32_t* framebuffer = window->GetFramebuffer();


		for (int y = 0; y < SCREEN_HEIGHT; y++)
		{
			for (int x = 0; x < SCREEN_WIDTH; x++)
			{
				framebuffer[y*SCREEN_WIDTH + x] = 0x00000000;
			}
		}

		Render(framebuffer);

		window->Draw();

		uint64_t end = platform->GetPerformanceCounter();

		frame++;

		uint64_t perf = platform->GetElapsedMs() - lastTime;

		//std::printf("FPS: %.2f\n", 1000.0f / (float)diff);
		double p = (double)(end - start) / (double)platform->GetPerformanceFrequency();
		std::printf("FPS: %.10f\n", 1.0f/p);

		if (perf < targetFrameDur)
		{
			SDL_Delay(targetFrameDur - perf);
		}
	}

	return 0;
}

