/*
 * Algorithm for both WallRaycastH and WallRaycastV:
 * 1. Start at an origin on a level given an angle
 * 2. Go to the next horizontal/vertical line (relative to the origin) and find its intersection with the line made up from the origin and angle
 * 3. Check if the intersection point is within the game world. If it isn't, terminate search
 * 4. Check if the game cell, where the point is located, is solid. If it isn't, go to (2). If it is, end search since we have found the closest intersection point 
 */

#include "game_algorithms.h"
#include "Level.h"
#include "math/math.h"
#include <cmath>

bool WallRaycastH(const Vec2& origin, float angle, const Level& level, Vec2& o_HPoint, Vec2i& o_Cell)
{
	const int32_t cellSize = level.GetCellSize();
	const Vec2i originCell = { (int)origin.x / cellSize, (int)origin.y / cellSize};
	const float normAngle = NormalizeAngleRad(angle);
	const bool bQuadrant1 = ANGLE_0 <= normAngle && normAngle < ANGLE_90;
	const bool bQuadrant2 = ANGLE_90 <= normAngle && normAngle < ANGLE_180;
	const bool bQuadrant3 = ANGLE_180 <= normAngle && normAngle < ANGLE_270;
	const bool bQuadrant4 = ANGLE_270 <= normAngle && normAngle < ANGLE_360;
	const bool bDownwardCast = bQuadrant3 || bQuadrant4;
	const float deltaY = (bQuadrant1 || bQuadrant2) ? cellSize : -cellSize;
	const Vec2i& worldSize = level.GetSize();
	float curY = (originCell.y + ((bQuadrant1 || bQuadrant2) ? 1 : 0)) * cellSize;

	const float M = std::tan(normAngle);
	const bool bHorizontalLine = NearZero(M);
	const bool bVerticalLine = NearlyEqual(normAngle, ANGLE_90) || NearlyEqual(normAngle, ANGLE_270);

	if (bHorizontalLine)
		return false;

	const float slope = bVerticalLine ? 0.0f : 1.0f / M;

#if (1)
	while (0 <= curY && curY < worldSize.y)
#else
	while (true)
#endif
	{
		// x = (1/M) * (y - yp) + xp
		float x = slope * (curY - origin.y) + origin.x;
		int cellX = x / cellSize;
		int cellY = curY / cellSize - (bDownwardCast ? 1 : 0);
		Vec2i cell = { cellX, cellY };

		o_HPoint = { x, curY };
		o_Cell = cell;

		if (!level.IsCellWithinBounds(cell))
			return false;

		if (level.IsSolidWall(cell))
			return true;

		curY += deltaY;
	}

	return false;
}

bool WallRaycastV(const Vec2& origin, float angle, const Level& level, Vec2& o_VPoint, Vec2i& o_Cell)
{
	const int32_t cellSize = level.GetCellSize();
	const Vec2i originCell = { (int)origin.x / cellSize, (int)origin.y / cellSize};
	const float normAngle = NormalizeAngleRad(angle);
	const bool bQuadrant1 = ANGLE_0 <= normAngle && normAngle < ANGLE_90;
	const bool bQuadrant2 = ANGLE_90 <= normAngle && normAngle < ANGLE_180;
	const bool bQuadrant3 = ANGLE_180 <= normAngle && normAngle < ANGLE_270;
	const bool bQuadrant4 = ANGLE_270 <= normAngle && normAngle < ANGLE_360;
	const bool bLeftwardCast = bQuadrant2 || bQuadrant3;
	const float deltaX = (bQuadrant1 || bQuadrant4) ? cellSize : -cellSize;
	const Vec2i& worldSize = level.GetSize();
	float curX = (originCell.x + ((bQuadrant1 || bQuadrant4) ? 1 : 0)) * cellSize;

	const float M = std::tan(normAngle);
	const bool bHorizontalLine = NearZero(M);
	const bool bVerticalLine = NearlyEqual(normAngle, ANGLE_90) || NearlyEqual(normAngle, ANGLE_270);

	if (bVerticalLine)
		return false;

	const float slope = bHorizontalLine ? 0.0f : M;
	
#if (1)
	while (0 <= curX && curX < worldSize.x)
#else
	while (true)
#endif
	{
		// y = M * (x - xp) + yp
		float y = slope * (curX - origin.x) + origin.y;	
		int cellX = curX / cellSize - (bLeftwardCast ? 1 : 0);
		int cellY = y / cellSize;
		Vec2i cell = { cellX, cellY };

		o_VPoint = { curX, y };
		o_Cell = cell;

		if (!level.IsCellWithinBounds(cell))
			break;

		if (level.IsSolidWall(cell))
			return true;

		curX += deltaX;
	}

	return false;
}

bool WallRaycast(const Vec2& origin, float angle, const Level& level, Vec2& o_Point, Vec2i& o_Cell)
{
	Vec2 hPoint, vPoint;
	Vec2i hCell, vCell;
	bool bHIntersection = WallRaycastH(origin, angle, level, hPoint, hCell);
	bool bVIntersection = WallRaycastV(origin, angle, level, vPoint, vCell);
	bool found = bHIntersection || bVIntersection;

	if (bHIntersection || bVIntersection)
	{
		float hDist = Distance(origin, hPoint);
		float vDist = Distance(origin, vPoint);
		bool bBothIntersections = bHIntersection && bVIntersection;
		bool hCase = (bBothIntersections && (hDist <  vDist)) || (!bBothIntersections && bHIntersection);
		bool vCase = (bBothIntersections && (hDist >= vDist)) || (!bBothIntersections && bVIntersection);

		found = true;

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
		std::printf("No intersection occurred. Seems like we went out of the world's bounds?\n");
	}

	return found;
}
