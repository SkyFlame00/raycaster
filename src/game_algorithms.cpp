#include "game_algorithms.h"
#include "Level.h"
#include "math/math.h"
#include <cmath>

bool FindHIntersectionPoint(const Vec2& origin, float angleRad, const Level& level, Vec2& o_HPoint, IVec2& o_Cell)
{
	// Algorithm:
	// 1. Start at a certain position (origin) given a view angle (angleRad)
	// 2. Go to the next horizontal line and find its intersection with a line made up from the position and view angle
	// 3. Check if the intersection point is within the game world
	// 4. If so, check if the game cell (on which our current horizontal line is lying) is solid. Otherwise, end search
	// 5. If so, we have found the closest horizontal intersection point. Otherwise, continue search
	bool bLeftwardCast, bDownwardCast; 
	float curX, curY;
	float deltaX, deltaY;
	IVec2 originCell;
	int32_t cellSize = level.GetCellSize();
	float angle = RadiansToDegrees(angleRad);

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
		curX = originCell.x * cellSize;
		curY = originCell.y * cellSize;
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
		return false;
	}

	float slope = tan(angleRad);
	bool found = false;

	while (true)
	{
		// x = (1/M) * (y - yp) + xp
		float x = (1.0f / slope) * (curY - origin.y) + origin.x;
		int cellX = x / cellSize;
		int cellY = curY / cellSize - (bDownwardCast ? 1 : 0);
		IVec2 cell = { cellX, cellY };

		o_HPoint = { x, curY };
		o_Cell = cell;

		bool bWithinWorld = level.IsCellWithinBounds(cell);
		if (!bWithinWorld)
			break;

		bool bSolidWall = level.IsSolidWall(cell);
		if (bSolidWall)
		{
			found = true;
			break;
		}
		else
		{
			curY += deltaY;
		}
	}

	return found;
}

bool FindVIntersectionPoint(const Vec2& origin, float angleRad, const Level& level, Vec2& o_VPoint, IVec2& o_Cell)
{
	// Algorithm:
	// 1. Start at a certain position (origin) given a view angle (angleRad)
	// 2. Go to the next vertical line and find its intersection with a line made up from the position and view angle
	// 3. Check if the intersection point is within the game world
	// 4. If so, check if the game cell (on which our current vertical line is lying) is solid. Otherwise, end search
	// 5. If so, we have found the closest vertical intersection point. Otherwise, continue search
	bool bLeftwardCast, bDownwardCast; 
	float curX, curY;
	float deltaX, deltaY;
	IVec2 originCell;
	int32_t cellSize = level.GetCellSize();
	float angle = RadiansToDegrees(angleRad);

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
		curX = originCell.x * cellSize;
		curY = originCell.y * cellSize;
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
		return false;
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
		IVec2 cell = { cellX, cellY };

		o_VPoint = { curX, y };
		o_Cell = cell;

		bool bWithinWorld = level.IsCellWithinBounds(cell);
		if (!bWithinWorld)
			break;

		bool bSolidWall = level.IsSolidWall(cell);
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

bool FindIntersectionPoint(const Vec2& origin, float angleRad, const Level& level, Vec2& o_Point, IVec2& o_Cell)
{
	Vec2 hPoint, vPoint;
	IVec2 hCell, vCell;
	bool bHIntersection = FindHIntersectionPoint(origin, angleRad, level, hPoint, hCell);
	bool bVIntersection = FindVIntersectionPoint(origin, angleRad, level, vPoint, vCell);
	bool found = bHIntersection || bVIntersection;

	if (bHIntersection || bVIntersection)
	{
		float hDist = Distance(origin, hPoint);
		float vDist = Distance(origin, vPoint);
		bool bBothIntersections = bHIntersection && bVIntersection;
		bool hCase = (bBothIntersections && (hDist <  vDist)) || (!bBothIntersections && bHIntersection);
		bool vCase = (bBothIntersections && (hDist >= vDist)) || (!bBothIntersections && bVIntersection);

		found = true;

		//std::printf("hCase: %d\n", hCase);
		//std::printf("hPt.x: %.1f, hPt.y: %.1f, hCellX: %d, hCellY: %d, hDist: %.1f\n", hPoint.x, hPoint.y, hCell.x, hCell.y, hDist);
		//std::printf("vPt.x: %.1f, vPt.y: %.1f, vCellX: %d, vCellY: %d, vDist: %.1f\n", vPoint.x, vPoint.y, vCell.x, vCell.y, vDist);

		if (hCase)
		{
			o_Point = hPoint;
			o_Cell = hCell;
		}
		else if (vCase)
		{
			o_Point = vPoint;
			o_Cell = vCell;
		}
		else
		{
			std::printf("Neither hCase nor vCase took place\n");
		}	
	}
	else
	{
		std::printf("No intersection occurred. Seems like we went out of the world's bounds\n");
	}

	return found;
}
