#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <cstdio>

#include "./src/constants.h"
#include "./src/math/math.h"
#include "./src/game_algorithms.h"
#include "./src/Level.h"

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
//char g_Level[LEVEL_ROWS*LEVEL_COLS];
Level* g_Level = nullptr;

class Player
{
public:
	float m_ViewAngleDeg;
	float m_Speed = 0.0f;
	Vec2 m_Pos;
};

const int g_CellSize = 64; // TODO: remove
//Vec2 g_WorldSize = { LEVEL_COLS * g_CellSize, LEVEL_ROWS * g_CellSize };
Player g_Player;
int g_FOV = 60;

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

//bool IsSolidWall(int x, int y)
//{
//	return GetCell(x, y) == '1';
//}



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
 
float NormalizeAngle(float angle)
{
	angle = fmodf(angle, 360.0f);
	angle += (angle < 0 ? 360.0f : 0.0f);
	return angle;
}

//bool IsWithinWorld(int cellX, int cellY)
//{
//	return 0 <= cellX && cellX <= g_WorldSize.x
//		&& 0 <= cellY && cellY <= g_WorldSize.y;
//}

void Render(Uint32* buf)
{
	for (int strip = 0; strip < SCREEN_WIDTH; strip++)
	{
		// Since strips are drawn from left to right, we should start with
		// adding half FOV to the player's angle and then subtract arcLength one-by-one
		float startAngle = g_Player.m_ViewAngleDeg + (g_FOV / 2);
		float arcLength = static_cast<float>(g_FOV) / SCREEN_WIDTH;
		float angle = startAngle - strip * arcLength;
		int playerCellX = g_Player.m_Pos.x / g_CellSize;
		int playerCellY = g_Player.m_Pos.y / g_CellSize;
		
		angle = NormalizeAngle(angle);
		float angleRad = DegreesToRadians(angle);

		// find intersection points
		Vec2 hPoint, vPoint;
		IVec2 hCell, vCell;
		bool hasHorIntersection = FindHIntersectionPoint(g_Player.m_Pos, angleRad, *g_Level, hPoint, hCell);
		bool hasVerIntersection = FindVIntersectionPoint(g_Player.m_Pos, angleRad, *g_Level, vPoint, vCell);
		int d_TargetStrip = 0;
		if (!hasHorIntersection && !hasVerIntersection)
		{
			// sth went wrong: assert and exit
			if (strip == d_TargetStrip)
			{
				std::printf("No intersection. HorPoint: (%d, %d), VerPoint: (%d, %d)\n",
					hCell.x, hCell.y, vCell.x, vCell.y);
			}
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

		bool d_HasIntersection = hasHorIntersection || hasVerIntersection;
		if (d_HasIntersection && strip == d_TargetStrip)
		{
			std::printf("IsHor=%d. Hor: (%d, %d). Ver: (%d, %d)\n", hCase, hCell.x, hCell.y, vCell.x, vCell.y);
		}


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

		if (1 && strip == d_TargetStrip)
		{
			std::printf("view angle: %.2f. Dist: %.2f\n", angle, dist);
		}

		// draw the strip
		float heightRatio = 0.0f;
		if (dist <= REAR_VIEW_DIST_LIMIT)
		{
			heightRatio = 1.0f;
		}
		else if (dist >= FAR_VIEW_DIST_LIMIT)
		{
			heightRatio = 0.0f;
		}
		else
		{
			// distNorm is in [0; 1] where 0 indicates that the object is on the rear view plane and 1 - on the far view plane.
			// We need to inverse this number to get the correct height ratio (the smaller the distance to the object, the higher the height ratio should be)
			float distNorm = (dist - REAR_VIEW_DIST_LIMIT) / (FAR_VIEW_DIST_LIMIT - REAR_VIEW_DIST_LIMIT);
			heightRatio = 1.0f - distNorm;
		}

		int wallHeightPx = heightRatio * SCREEN_HEIGHT;
		int ceilingHeightPx = (SCREEN_HEIGHT - wallHeightPx) / 2;
		int floorHeightPx = (SCREEN_HEIGHT - wallHeightPx) / 2;

		// draw the ceiling/sky
		for (int i = 0; i < ceilingHeightPx; i++)
		{
			Uint32 color = 0xFF000000;
			buf[i * SCREEN_WIDTH + strip] = color;
		}

		// draw the wall
		for (int i = ceilingHeightPx; i < ceilingHeightPx + wallHeightPx; i++)
		{
			Uint32 color = 0xFFD6D6D6;
			//if (strip < 20)
			//{
			//	color = 0xFFFF0000;
			//}
			
			//color = hCase ? 0xFF9B9DC7 : 0xFF6B6D8A;
			color = hCase ? 0xFFA1A1A1 : 0xFF696969;
			
			buf[i * SCREEN_WIDTH + strip] = color;
		}

		// draw the floor
		for (int i = ceilingHeightPx + wallHeightPx; i < SCREEN_HEIGHT; i++)
		{
			//Uint32 color = 0xFF555555;
			//Uint32 color = 0xFF3E3E4F;
			Uint32 color = 0xFF404040;
			buf[i * SCREEN_WIDTH + strip] = color;
		}
	}
}

void HandleInput(float dt)
{
	SDL_PumpEvents();

	const Uint8* state = SDL_GetKeyboardState(nullptr);
	const float rotationSpeed = 180.0f; // 180 degrees per second

	if (state[SDL_SCANCODE_A])
	{
		g_Player.m_ViewAngleDeg += dt * rotationSpeed;
		std::cout << "View angle = " << g_Player.m_ViewAngleDeg << std::endl;
	}

	if (state[SDL_SCANCODE_D])
	{
		g_Player.m_ViewAngleDeg -= dt * rotationSpeed;
		std::cout << "View angle = " << g_Player.m_ViewAngleDeg << std::endl;
	}

	g_Player.m_Speed = 0.0f;

	const float moveSpeed = 200.0f * dt;
	if (state[SDL_SCANCODE_W])
	{
		g_Player.m_Speed = moveSpeed;
	}

	if (state[SDL_SCANCODE_S])
	{
		//g_Player.m_Speed = -moveSpeed;
	}
}

void PhysicsFrame(float dt)
{
	float speed = g_Player.m_Speed;
	if (speed > 0.0f)
	{
		float normAngleDeg = NormalizeAngle(g_Player.m_ViewAngleDeg);
		float angleRad = DegreesToRadians(normAngleDeg);
		float dx = speed * cosf(angleRad);
		float dy = speed * sinf(angleRad);
		Vec2 pos = g_Player.m_Pos;
		Vec2 projPoint = { pos.x + dx, pos.y + dy };
		Vec2 wallPoint;
		IVec2 wallCell;
		
		bool found = FindIntersectionPoint(pos, angleRad, *g_Level, wallPoint, wallCell);
		if (!found)
		{
			// report an error
			return;
		}

		Vec2 dstPoint;
		float distToProjPoint = std::abs(projPoint.Length() - pos.Length());
		float distToWallPoint = std::abs(wallPoint.Length() - pos.Length());
		if (distToProjPoint < distToWallPoint)
		{
			dstPoint = projPoint;
		}
		else
		{
			dstPoint = wallPoint;
		}

		g_Player.m_Pos = dstPoint;
	}
}

int main()
{
	std::cout << "Starting program..." << std::endl;
	if (SDL_Init(SDL_INIT_VIDEO != 0))
	{
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	if (!window)
	{
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
	{
		std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!texture)
	{
		std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
		SDL_DestroyRenderer(renderer);
	        SDL_DestroyWindow(window);
       		SDL_Quit();
	        return 1;
	}
	bool running = true;
	SDL_Event event;
	int frame = 0;

	char levelData[LEVEL_ROWS*LEVEL_COLS];
	IVec2 levelSize = { LEVEL_COLS * g_CellSize, LEVEL_ROWS * g_CellSize };
	MirrorLevelString(g_RawLevelData, levelData);
	g_Level = new Level(levelData, levelSize, (int32_t)g_CellSize);

	SpawnPlayer(g_Player, LEVEL_ROWS, LEVEL_COLS);

	Uint32 lastTime = SDL_GetTicks();
	while (running)
	{
		//while (SDL_PollEvent(&event))
		//{
	        //	if (event.type == SDL_QUIT)
		//		running = false;
		//}

		Uint32 currentTime = SDL_GetTicks();
		float dt = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;

		HandleInput(dt);
		PhysicsFrame(dt);

		void* pixels;
		int pitch; // it's measured in bytes
		SDL_LockTexture(texture, nullptr, &pixels, &pitch);

		//std::cout << "Pitch = " << pitch << std::endl;
		//
		//for (int y = 0; y < SCREEN_HEIGHT; ++y)
		//{
		//	for (int x = 0; x < SCREEN_WIDTH; ++x)
		//	{
		//		Uint8 r = (x + frame) % 256;
		//		Uint8 g = (y + frame) % 256;
		//		Uint8 b = ((x + y) / 2 + frame) % 256;
		//		buf[y * (pitch / 4) + x] = (255 << 24) | (r << 16) | (g << 8) | b;
		//	}
		//}

		Uint32* buf = static_cast<Uint32*>(pixels);

		for (int y = 0; y < SCREEN_HEIGHT; y++)
		{
			for (int x = 0; x < SCREEN_WIDTH; x++)
			{
				buf[y*SCREEN_WIDTH + x] = 0x00000000;
			}
		}

		
	//	for (int y = 0; y < SCREEN_HEIGHT; y++)
	//	{
	//		for (int x = 0; x < SCREEN_WIDTH; x++)
	//		{
	//			int color = 0;
	//			if (y < (SCREEN_HEIGHT/2))
	//			{
	//				color = 0xFFFF0000;
	//			}
	//			else
	//			{
	//				color = 0xFF00FF00;
	//			}
	//			buf[y*SCREEN_WIDTH + x] = color;
	//		}
	//	}

		Render(buf);

		SDL_UnlockTexture(texture);

       		SDL_RenderClear(renderer);
       		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
       		SDL_RenderPresent(renderer);

	        frame++;
	       	//SDL_Delay(16); // ~60 FPS
    	}

	delete g_Level; // TODO: remove raw pointes

    	SDL_DestroyTexture(texture);
    	SDL_DestroyRenderer(renderer);
    	SDL_DestroyWindow(window);
    	SDL_Quit();

	return 0;
}
