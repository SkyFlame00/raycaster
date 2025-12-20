float DegToRad(float deg)
{
	return deg * (M_PI / 180.0f);
}

float RadToDeg(float rad)
{
	return rad * (180.0f / M_PI);
}

bool FindVIntersectionPoint(const Vec2& origin, float angleRad, const Vec2& levelSize, int cellSize, Vec2& o_VPoint, IVec2& o_Cell)
{
	bool bLeftwardCast;
	bool bDownwardCast; 
	float curX;
	float curY;
	float deltaX;
	float deltaY;

	angle
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

	float slope = tan(angleRad);
	
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
}
