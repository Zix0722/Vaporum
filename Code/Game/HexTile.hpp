#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "TileDefinition.hpp"
#include "Engine/Math/Vec3.hpp"
#include <queue>

class Unit;
class Map;
class HexTile
{
public:
	HexTile();
	HexTile(int x, int y) : x(x), y(y), z(-x - y) {};
	~HexTile();

	void Render();
	bool IsPointInHexagon(Vec3 pos);
	Vec3 GetCenterPos() const;
	void RenderInnerBorder(Rgba8 color);
	void RenderOuterBorder(float level);
	int DistTo(HexTile* another);
	int DistToAnother(HexTile* another);
	void HighlightPath(HexTile* destination);

	bool IsValid() const;
public:
	IntVec2 m_coord;
	Map* m_map = nullptr;
	HexTile* m_parent = nullptr;
	int m_GRID_SIZE = 0;
	Vec3 m_worldPos;
	TileDefinition* m_def = nullptr;
	float m_inRadius;
	float m_circumRadius;
	float m_sideLength = 1.08f;
	bool m_isRenderingAsPath = false;
	Unit* m_unit = nullptr;
	std::vector<Vec3> m_vertexes;

	int x, y, z;

};