#pragma once
#include "MapDefinition.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Core/OBJLoader.hpp"

class Game;
class VertexBuffer;
class IndexBuffer;
class HexTile;
class Unit;

extern OBJLoader* m_OBJLoader;

class Map
{
public:
	Map(Game* owner, std::string mapName);
	Map(std::string mapName);
	~Map();

	void Update();
	void Render() const;

	static Map* LoadMapByName(std::string name);
	HexTile* GetTileByXY(int x, int y);
	void ClearAllPath();
public:
	Game* m_game = nullptr;
	MapDefinition* m_def = nullptr;
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	std::vector<Vertex_PNCU> m_vertexes;
	std::vector<unsigned int> m_indexes;
	
	std::vector<HexTile*> m_tiles;
	float boundsThickness = 0.02f;
	void InitUnits();

	std::vector<Unit*> m_P1Units;
	std::vector<Unit*> m_P2Units;
	Unit* m_currentCursorUnit = nullptr;
	Unit* m_currentSelectUnit = nullptr;
	HexTile* m_startMovingTile = nullptr;
private:
	void InitTiles();
	void RemoveUnitsFromMapIfUnderZeroHP();
};