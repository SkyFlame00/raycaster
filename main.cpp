#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <cstdio>

//const int SCREEN_WIDTH = 640;
//const int SCREEN_HEIGHT = 480;
const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 120;
const float REAR_VIEW_DIST_LIMIT = 20.0f;
const float FAR_VIEW_DIST_LIMIT = 450.0f;

constexpr int LEVEL_ROWS = 17;
constexpr int LEVEL_COLS = 18;
const char* g_LevelMirrored = 
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
char g_Level[LEVEL_ROWS*LEVEL_COLS];

struct Vec2
{
	float x, y;
	
	Vec2 operator+(const Vec2& other) const
	{
		return {x + other.x, y + other.y};
	}

	Vec2 operator-(const Vec2& other) const
	{
		return {x - other.x, y - other.y};
	}

	float Length()
	{
		return sqrtf(x*x + y*y);
	}
};

class Player
{
public:
	float m_ViewAngleDeg;
	float m_Speed = 0.0f;
	Vec2 m_Pos;
};

const int g_CellSize = 64;
Vec2 g_WorldSize = { LEVEL_COLS * g_CellSize, LEVEL_ROWS * g_CellSize };
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

char GetCell(int x, int y)
{
	//return g_Level[(LEVEL_ROWS - 1) - y + x];
	return g_Level[LEVEL_COLS*y + x];
}

bool IsSolidWall(int x, int y)
{
	return GetCell(x, y) == '1';
}


float Distance(const Vec2& v1, const Vec2& v2)
{
	return (v1 - v2).Length();
}

void SpawnPlayer(Player& player, int rows, int cols)
{
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < cols; col++)
		{
			if (GetCell(col, row) == 'P')
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

bool IsWithinWorld(int cellX, int cellY)
{
	return 0 <= cellX && cellX <= g_WorldSize.x
		&& 0 <= cellY && cellY <= g_WorldSize.y;
}

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

		// find intersection points
		Vec2 HorPoint;
		Vec2 VerPoint;
		int hCellX, hCellY;
		int vCellX, vCellY;
		bool hasVerIntersection = false;
		bool hasHorIntersection = false;
		int d_TargetStrip = 0;
		int d_LastVerCellX = -1;
		int d_LastVerCellY = -1;
		int d_LastHorCellX = -1;
		int d_LastHorCellY = -1;
		{
			bool bLeftwardCast;
			bool bDownwardCast; 
			float curX;
			float curY;
			float deltaX;
			float deltaY;
			// TODO: reduce the ifs below. I believe, we can calculate this from trigonometric functions without having to look at each individual quadrant.
			if (0.0f <= angle && angle < 90.0f)
			{
				bLeftwardCast = false;
				bDownwardCast = false;
				curX = (playerCellX + 1) * g_CellSize;
				curY = (playerCellY + 1) * g_CellSize;
				deltaX = g_CellSize;
				deltaY = g_CellSize;
			}
			else if (90.0f <= angle && angle < 180.0f)
			{
				bLeftwardCast = true;
				bDownwardCast = false;
				curX = playerCellX * g_CellSize;
				curY = (playerCellY + 1) * g_CellSize;
				deltaX = -g_CellSize;
				deltaY =  g_CellSize;	
			}
			else if (180.0f <= angle && angle < 270.0f)
			{
				bLeftwardCast = true;
				bDownwardCast = true;
				curX = playerCellX * g_CellSize;
				curY = playerCellY * g_CellSize;
				deltaX = -g_CellSize;
				deltaY = -g_CellSize;
			}
			else if (270.0f <= angle && angle < 360.0f)
			{
				bLeftwardCast = false;
				bDownwardCast = true;
				curX = (playerCellX + 1) * g_CellSize;
				curY = playerCellY * g_CellSize;
				deltaX =  g_CellSize;
				deltaY = -g_CellSize;
			}
			else
			{
				// report error
				return;
			}
			float rad = DegToRad(angle);
			float slope = tan(rad);
			
			while (true)
			{
				// y = M * (x - xp) + yp
				// where x is curX and (xp; yp) is the player's pos
				float y = slope * (curX - g_Player.m_Pos.x) + g_Player.m_Pos.y;	
				int cellX = curX / g_CellSize - (bLeftwardCast ? 1 : 0);
				int cellY = y / g_CellSize;
				bool bWithinWorld = IsWithinWorld(cellX, cellY);

				d_LastVerCellX = cellX;
				d_LastVerCellY = cellY;
				if (bWithinWorld)
				{
					bool bSolidWall = IsSolidWall(cellX, cellY);
					if (bSolidWall)
					{
						hasVerIntersection = true;
						VerPoint = { curX, y };
						vCellX = cellX;
						vCellY = cellY;
						break;
					}	
					else
					{
						curX += deltaX;
					}
				}
				else
				{
					break;
				}
			}

			while (true)
			{
				float x = (1.0f / slope) * (curY - g_Player.m_Pos.y) + g_Player.m_Pos.x;
				int cellX = x / g_CellSize;
				int cellY = curY / g_CellSize - (bDownwardCast ? 1 : 0);
				bool bWithinWorld = IsWithinWorld(cellX, cellY);

				d_LastHorCellX = cellX;
				d_LastHorCellY = cellY;

				if (!bWithinWorld)
					break;

				bool bSolidWall = IsSolidWall(cellX, cellY);
				if (bSolidWall)
				{
					hasHorIntersection = true;
					HorPoint = { x, curY };
					hCellX = cellX;
					hCellY = cellY;
					break;
				}
				else
				{
					curY += deltaY;
				}
			}


			if (!hasHorIntersection && !hasVerIntersection)
			{
				// sth went wrong: assert and exit
				if (strip == d_TargetStrip)
				{
					std::printf("No intersection. HorPoint: (%d, %d), VerPoint: (%d, %d)\n",
						d_LastHorCellX, d_LastHorCellY, d_LastVerCellX, d_LastVerCellY);
					std::cout << "No intersection occurred!\n";
				}
				return;
			}
		}
		
		// find distance to the closest intersection
		float dist = 0.0f;
		float horPointDist = Distance(HorPoint, g_Player.m_Pos);
		float verPointDist = Distance(VerPoint, g_Player.m_Pos); 
		int cellX = 0;
		int cellY = 0;
		bool bothIntersections = hasHorIntersection && hasVerIntersection;
		bool hCase = (bothIntersections && (horPointDist <  verPointDist)) || (!bothIntersections && hasHorIntersection);
		bool vCase = (bothIntersections && (horPointDist >= verPointDist)) || (!bothIntersections && hasVerIntersection);

		bool d_HasIntersection = hasHorIntersection || hasVerIntersection;
		if (d_HasIntersection && strip == d_TargetStrip)
		{
			//std::printf("IsHor=%d. Hor: (%d, %d). Ver: (%d, %d)\n", hCase, d_LastHorCellX, d_LastHorCellY, d_LastVerCellX, d_LastVerCellY);
		}


		if (hCase)
		{
			dist = horPointDist;
			cellX = hCellX;
			cellY = hCellY;
		}
		else if (vCase)
		{
			dist = verPointDist;
			cellX = vCellX;
			cellY = vCellY;
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

	const float moveSpeed = 30.0f; // 30 units per second
	if (state[SDL_SCANCODE_W])
	{
		g_Player.m_Speed = moveSpeed;
	}

	if (state[SDL_SCANCODE_S])
	{
		g_Player.m_Speed = -moveSpeed;
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

	MirrorLevelString(g_LevelMirrored, g_Level);
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

    	SDL_DestroyTexture(texture);
    	SDL_DestroyRenderer(renderer);
    	SDL_DestroyWindow(window);
    	SDL_Quit();

	return 0;
}
