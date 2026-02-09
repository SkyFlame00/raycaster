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

namespace ray
{
	void WallRaycastH(const Vec2& origin, float angle, const ray::Level& level, std::vector<WallRaycastHit>& o_Hits)
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
			return;

		const float slope = bVerticalLine ? 0.0f : 1.0f / M;

		while (0 <= curY && curY < worldSize.y)
		{
			// x = (1/M) * (y - yp) + xp
			float x = slope * (curY - origin.y) + origin.x;
			int cellX = x / cellSize;
			int cellY = curY / cellSize - (bDownwardCast ? 1 : 0);
			Vec2i cell = { cellX, cellY };

			//if (!level.IsCellWithinBounds(cell))
			//	return;

			if (!level.IsPointWithinBounds({ x, curY }))
				return;

			if (level.IsSolidWall(cell))
			{
				WallRaycastHit hit;
				hit.point = { x, curY };
				hit.cell = cell;
				hit.isHor = true;
				o_Hits.push_back(hit);
			}

			curY += deltaY;
		}
	}

	void WallRaycastV(const Vec2& origin, float angle, const ray::Level& level, std::vector<WallRaycastHit>& o_Hits)
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
			return;

		const float slope = bHorizontalLine ? 0.0f : M;

		while (0 <= curX && curX < worldSize.x)
		{
			// y = M * (x - xp) + yp
			float y = slope * (curX - origin.x) + origin.y;	
			int cellX = curX / cellSize - (bLeftwardCast ? 1 : 0);
			int cellY = y / cellSize;
			Vec2i cell = { cellX, cellY };

			//if (!level.IsCellWithinBounds(cell))
			//	break;

			if (!level.IsPointWithinBounds({ curX, y }))
				return;

			if (level.IsSolidWall(cell))
			{
				WallRaycastHit hit;
				hit.point = { curX, y };
				hit.cell = cell;
				hit.isHor = false;
				o_Hits.push_back(hit);
			}

			curX += deltaX;
		}
	}

	void WallRaycast(const Vec2& origin, float angle, const ray::Level& level, std::vector<WallRaycastHit>& o_Hits)
	{
		std::vector<WallRaycastHit> hHits;
		std::vector<WallRaycastHit> vHits;

		WallRaycastH(origin, angle, level, hHits);
		WallRaycastV(origin, angle, level, vHits);

		bool bHitH = hHits.size() > 0;
		bool bHitV = vHits.size() > 0;

		if (bHitH || bHitV)
		{
			Vec2 hPoint = bHitH ? hHits[0].point : Vec2{};
			Vec2 vPoint = bHitV ? vHits[0].point : Vec2{};
			Vec2i hCell = bHitH ? hHits[0].cell : Vec2i{};
		   	Vec2i vCell = bHitV ? vHits[0].cell : Vec2i{};
			float hDist = Distance(origin, hPoint);
			float vDist = Distance(origin, vPoint);
			bool bBothIntersections = bHitH && bHitV;
			bool hCase = (bBothIntersections && (hDist <  vDist)) || (!bBothIntersections && bHitH);
			bool vCase = (bBothIntersections && (hDist >= vDist)) || (!bBothIntersections && bHitV);

			// TODO: merge both hHits and vHits into a single vector and sort it based on the point distance and remove the code below

			if (hCase)
			{
				o_Hits.push_back(hHits[0]);
			}
			else if (vCase)
			{
				o_Hits.push_back(vHits[0]);
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
	}
}

