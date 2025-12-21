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
	// Algorithm:
	// 1. Start at a certain position (origin) given a view angle (angleRad)
	// 2. Go to the next vertical line and find its intersection with a line made up from the position and view angle
	// 3. Check if the intersection point is within the game world
	// 4. If so, check if the game cell (on which our current vertical line is lying) is solid. Otherwise, end search
	// 5. If so, we have found the closest vertical intersection so. Otherwise, continue search
	bool bLeftwardCast, bDownwardCast; 
	float curX, curY;
	float deltaX, deltaY;
	IVec2 originCell;

	originCell.x = origin.x / cellSize;
	originCell.y = origin.y / cellSize;

	// TODO: reduce the ifs below. I believe, we can calculate this from trigonometric functions without having to look at each individual quadrant.
	if (0.0f <= angle && angle < 90.0f)
	{
		bLeftwardCast = false;
		bDownwardCast = false;
		curX = (originCell.x + 1) * cellSize;
		curY = (originCell.y + 1) * cellSize;
		deltaX = cellSize;
		deltaY = cellSize;
	}
	else if (90.0f <= angle && angle < 180.0f)
	{
		bLeftwardCast = true;
		bDownwardCast = false;
		curX = originCell.x * cellSize;
		curY = (originCell.y + 1) * cellSize;
		deltaX = -cellSize;
		deltaY =  cellSize;	
	}
	else if (180.0f <= angle && angle < 270.0f)
	{
		bLeftwardCast = true;
		bDownwardCast = true;
		curX = originCellX * cellSize;
		curY = originCellY * cellSize;
		deltaX = -cellSize;
		deltaY = -cellSize;
	}
	else if (270.0f <= angle && angle < 360.0f)
	{
		bLeftwardCast = false;
		bDownwardCast = true;
		curX = (originCell.x + 1) * cellSize;
		curY = originCell.y * cellSize;
		deltaX =  cellSize;
		deltaY = -cellSize;
	}
	else
	{
		// report error
		return;
	}

	float slope = tan(angleRad);
	bool found = false;
	
	// TODO: this while should be re-written using a finite for-loop
	while (true)
	{
		// y = M * (x - xp) + yp
		float y = slope * (curX - origin.x) + origin.y;	
		int cellX = curX / cellSize - (bLeftwardCast ? 1 : 0);
		int cellY = y / cellSize;

		o_VPoint = { curX, y };
		o_Cell = { cellX, cellY };

		bool bWithinWorld = IsWithinWorld(cellX, cellY);
		if (!bWithinWorld)
			break;

		bool bSolidWall = IsSolidWall(cellX, cellY);
		if (bSolidWall)
		{
			found = true;
			break;
		}	
		else
		{
			curX += deltaX;
		}
	}

	return found;
}
