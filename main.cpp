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


const char* g_RawLevelData2 =
					"11111"\
					"1   1"\
					"1 P 1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"1   1"\
					"11111";

const char* g_RawLevelData = 
                    "111111111111111111"\
                    "1P  1            1"\
                    "1   1            1"\
                    "1         111111 1"\
                    "1   1     1    1 1"\
                    "11111     1      1"\
                    "1         1    1 1"\
                    "1         1    1 1"\
                    "1              1 1"\
                    "1         1    1 1"\
                    "1         111111 1"\
                    "1                1"\
                    "1  111     111   1"\
                    "1  111     111   1"\
                    "1  111     111   1"\
                    "1                1"\
                    "111111111111111111";
Level* g_Level = nullptr;

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
uint8_t* g_ImgData = nullptr; // TODO: remove
uint32_t g_ImgWidth;
uint32_t g_ImgHeight;
uint32_t g_ImgNrChannels;

void MirrorLevelString(const char* src, char* dst)
{
	for (int row = 0; row < LEVEL_ROWS; ++row)
	{
		for (int col = 0; col < LEVEL_COLS; ++col)
		{
			dst[(LEVEL_ROWS - 1 - row)*LEVEL_COLS + col] = src[row*LEVEL_COLS + col];
		}
	}
}

void SpawnPlayer(Player& player, int rows, int cols)
{
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < cols; col++)
		{
			if (g_Level->GetAt(col, row) == 'P')
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

uint32_t TextureSample(uint8_t* imgData, uint32_t imgWidth, uint32_t imgHeight, uint32_t imgNrChannels, float normX, float normY)
{
	uint32_t x = normX * imgWidth;
	uint32_t y = normY * imgHeight;
	uint32_t offset = (y * imgWidth + x) * imgNrChannels;
	uint8_t r = imgData[offset + 0];
	uint8_t g = imgData[offset + 1];
	uint8_t b = imgData[offset + 2];
	uint8_t a = imgData[offset + 3];
	uint32_t color = (a << 24) | (r << 16) | (g << 8) | b;

	return color;
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

void DrawWall(uint32_t* framebuffer, float heightRatio, const Vec2& hPoint, const Vec2& vPoint, bool hCase, int32_t strip, float dist)
{
	// if texture should be stretched in y-dimension (height), then we need to determine how we should stretch it in x-dimension
	float texHeightRatio = g_ImgHeight / (float)g_ImgWidth;
	float texWidthUnits = g_CellSize / texHeightRatio;
	uint32_t wallHeightPx = std::ceil(SCREEN_HEIGHT * heightRatio); // to get the same wallHeightPx as in Render to avoid artifacts
	int32_t offset = (SCREEN_HEIGHT - (int32_t)wallHeightPx) / 2;
	uint32_t beginY = offset >= 0 ? offset : 0;
	uint32_t endY = std::min(beginY + wallHeightPx, (uint32_t)SCREEN_HEIGHT);
	float textureX;
	if (hCase)
	{
		textureX = fmodf(hPoint.x, texWidthUnits) / texWidthUnits;
	}
	else
	{
		textureX = fmodf(vPoint.y, texWidthUnits) / texWidthUnits;
	}
	
	for (int32_t i = beginY; i < endY; i++)
	{
		float textureY = (i - offset) / (float)wallHeightPx;
		uint32_t color = TextureSample(g_ImgData, g_ImgWidth, g_ImgHeight, g_ImgNrChannels, textureX, textureY);

		if (0 && i==0)
		{
			// This is a test on the wall/texture "teeth"
			std::printf("s: %d, c: %x, off: %d, (%u, %u), (%.10f, %.10f), wh=%u, hr=%.10f\n",
					strip, color, offset, beginY, endY, textureX, textureY, wallHeightPx, heightRatio);
		}

		if (hCase)
		{
			// apply horizontal wall shading
			//color = ApplyBrightness(color, -25);
		}

		float intensity = GetShadingIntensity(dist);
		color = ApplyBrightness(color, intensity);

		framebuffer[i * SCREEN_WIDTH + strip] = color;
	}
}

uint8_t* g_FloorImgData = nullptr;
uint32_t g_FloorImgWidth;
uint32_t g_FloorImgHeight;
uint32_t g_FloorImgNrChannels;
uint32_t GetFloorColor(float worldX, float worldY)
{
	float texHeightRatio = g_FloorImgHeight / (float)g_FloorImgWidth;
	float texWidthUnits = g_CellSize / texHeightRatio;
	float textureX = fmodf((float)worldX, texWidthUnits) / texWidthUnits;
	float textureY = fmodf((float)worldY, g_CellSize) / (float)g_CellSize;

	uint32_t color = TextureSample(g_FloorImgData, g_FloorImgWidth, g_FloorImgHeight, g_FloorImgNrChannels, textureX, textureY);

	return color;
}

void DrawFloor(uint32_t* framebuffer, float distToProjPlane, float worldAngle, float localAngle, uint32_t strip, uint32_t wallHeight, uint32_t beginY, uint32_t endY)
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

		uint32_t color = GetFloorColor(worldX, worldY);
		//uint32_t color = 0xFFCCCCCC;
		float intensity = GetShadingIntensity(hyp);
		color = ApplyBrightness(color, intensity);

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

	for (int strip = 0; strip < SCREEN_WIDTH; strip++)
	{
		// Get an angle at which a ray goes through the strip N to get equally spaced (or linear) columns.
		// Without this, the columns would look slightly non-linear even after the fish-eye effect correction.
		// See https://www.scottsmitelli.com/articles/we-can-fix-your-raycaster/
		float projPlaneX = halfProjPlaneWidth - strip * stripWidth;
		float localAngleRad = atanf(projPlaneX / distToProjPlane); // atan2f isn't needed since we use exactly I and IV quadrants
		float worldAngleRad = localAngleRad + ToRadians(g_Player.m_ViewAngleDeg);
		
		Vec2 hPoint, vPoint;
		Vec2i hCell, vCell;
		bool hasHorIntersection = WallRaycastH(g_Player.m_Pos, worldAngleRad, *g_Level, hPoint, hCell);
		bool hasVerIntersection = WallRaycastV(g_Player.m_Pos, worldAngleRad, *g_Level, vPoint, vCell);
		if (!hasHorIntersection && !hasVerIntersection)
		{
			// sth went wrong: assert and exit
			std::printf("No intersection. HorPoint: (%d, %d), VerPoint: (%d, %d)\n",
					hCell.x, hCell.y, vCell.x, vCell.y);
			return;
		}
		
		// find distance to the closest intersection
		float dist = 0.0f;
		float horPointDist = Distance(hPoint, g_Player.m_Pos);
		float verPointDist = Distance(vPoint, g_Player.m_Pos); 
		int cellX = 0;
		int cellY = 0;
		bool bothIntersections = hasHorIntersection && hasVerIntersection;
		bool hCase = (bothIntersections && (horPointDist <  verPointDist)) || (!bothIntersections && hasHorIntersection);
		bool vCase = (bothIntersections && (horPointDist >= verPointDist)) || (!bothIntersections && hasVerIntersection);

		if (hCase)
		{
			dist = horPointDist;
			cellX = hCell.x;
			cellY = hCell.y;
		}
		else if (vCase)
		{
			dist = verPointDist;
			cellX = vCell.x;
			cellY = vCell.y;
		}
		else
		{
			std::printf("Neither hCase nor vCase occurred\n");
		}

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
		const float distToProjPlane = 1.0f;
		const float wallHeight = 80.0f;
		float heightRatio = (wallHeight / correctedDist) * distToProjPlane;
		float heightRatioRaw = heightRatio;
		heightRatio = std::clamp(heightRatio, 0.0f, 1.0f);

		const float baseHeight = 80.0f;

		// We ceil to the next integer to avoid an issue with floor texture mapping when a cast ray goes through the wall instead of below it.
		// This is due to one-pixel-difference which occurs due to integer truncation. Say, wallHeight is 5.4, so wallHeightPx will be 5.
		// Then, we'll use 5 in calculations which will produce slightly incorrect results (they become more pronounced with increasing distance) because we should've taken something bigger than 5.4.
		// Rounding (in our case - ceiling) also helps avoid a "teeth" effect when neighboring pixels differ by one texel.
		int wallHeightPx = std::ceil(heightRatio * SCREEN_HEIGHT);
		int floorHeightPx = std::floor((SCREEN_HEIGHT - (float)wallHeightPx) / 2.0f);
		int ceilingHeightPx = std::ceil((SCREEN_HEIGHT - (float)wallHeightPx) / 2.0f);

		// draw the ceiling/sky
		for (int i = 0; i < ceilingHeightPx; i++)
		{
			Uint32 color = 0xFF000000;
			framebuffer[i * SCREEN_WIDTH + strip] = color;
		}

		DrawWall(framebuffer, heightRatioRaw, hPoint, vPoint, hCase, strip, correctedDist);

		DrawFloor(framebuffer, distToProjPlane, worldAngleRad, localAngleRad, strip, wallHeight, ceilingHeightPx + wallHeightPx, SCREEN_HEIGHT);
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
	if (!IsZero(speed))
	{
		float normAngleDeg = NormalizeAngle(g_Player.m_MoveAngleDeg);
		float angleRad = ToRadians(normAngleDeg);
		float dx = speed * cosf(angleRad);
		float dy = speed * sinf(angleRad);
		Vec2 pos = g_Player.m_Pos;
		Vec2 projPoint = { pos.x + dx, pos.y + dy };
		Vec2 wallPoint;
		Vec2i _wallCell;
		
		bool found = WallRaycast(pos, angleRad, *g_Level, wallPoint, _wallCell);
		if (!found)
		{
			// report an error
			return;
		}

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

			bool bPrevSolidWall = g_Level->IsSolidWall(pos);
			int cellSize = g_Level->GetCellSize();
			Vec2i oldCell = { (int)(pos.x / cellSize), (int)(pos.y / cellSize) };
			Vec2i newCell = { (int)(g_Player.m_Pos.x / cellSize), (int)(g_Player.m_Pos.y / cellSize) };

			bool bSolidWall = g_Level->IsSolidWall(g_Player.m_Pos);
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

	char levelData[LEVEL_ROWS*LEVEL_COLS];
	Vec2i levelSize = { LEVEL_COLS * g_CellSize, LEVEL_ROWS * g_CellSize };
	MirrorLevelString(g_RawLevelData, levelData);
	g_Level = new Level(levelData, levelSize, (int32_t)g_CellSize);

	SpawnPlayer(g_Player, LEVEL_ROWS, LEVEL_COLS);

	float targetFPS = 18.0f;
	uint64_t targetFrameDur = (uint64_t)(1000.0f / targetFPS);
	uint64_t lastTime = platform->GetElapsedMs();

	// load test image
	//const char* texturePath = "./assets/textures/bkred_1.png";
	//const char* texturePath = "./assets/textures/brik_3.png";
	//const char* texturePath = "./assets/textures/brks_1.png";
	const char* texturePath = "./assets/textures/brks_00.png";
	int32_t imgWidth;
	int32_t imgHeight;
	int32_t nrChannels;
	uint8_t* imgData = stbi_load(texturePath, &imgWidth, &imgHeight, &nrChannels, 0);
	assert(imgData);

	g_ImgData = imgData;
	g_ImgWidth = imgWidth;
	g_ImgHeight = imgHeight;
	g_ImgNrChannels = nrChannels;

	//const char* floorTexturePath = "./assets/textures/wood1.png";
	const char* floorTexturePath = "./assets/textures/wall52_1.png";
	//g_FloorImgData = stbi_load(floorTexturePath, &g_FloorImgWidth, &g_FloorImgHeight, &g_FloorImgNrChannels, 0);
	imgData = stbi_load(floorTexturePath, &imgWidth, &imgHeight, &nrChannels, 0);
	assert(imgData);

	g_FloorImgData = imgData;
	g_FloorImgWidth = imgWidth;
	g_FloorImgHeight = imgHeight;
	g_FloorImgNrChannels = nrChannels;

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

		if (0)
		{
			for (int32_t y = 0; y < imgHeight; y++)
			{
				for (int32_t x = 0; x < imgWidth; x++)
				{
					uint32_t offset = (y * imgWidth + x) * nrChannels;
					uint8_t r = imgData[offset];
					uint8_t g = imgData[offset+1];
					uint8_t b = imgData[offset+2];
					uint8_t a = imgData[offset+3];

					int32_t color = (a << 24) | (r << 16) | (g << 8) | b;
					//uint32_t color = 0xFF000000 | (b << 16) | (g << 8) | r;

					framebuffer[y * SCREEN_WIDTH + x] = color;
				}
			}
		}

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

	stbi_image_free(imgData);
	delete g_Level; // TODO: remove raw pointes

	return 0;
}

