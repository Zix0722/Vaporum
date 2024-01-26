#include "Map.hpp"
#include "GameCommon.hpp"
#include "HexTile.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "UnitDefinition.hpp"
#include "Unit.hpp"
#include "Engine/Renderer/Model.hpp"
#include "Game.hpp"



Map::Map(Game* owner, std::string mapName)
	:m_game(owner)
{
	m_def = MapDefinition::GetDefinitionByName(mapName);
	InitTiles();
}


Map::~Map()
{

}

void Map::Update()
{
	RemoveUnitsFromMapIfUnderZeroHP();
	for (int i = 0; i < m_P1Units.size(); ++i)
	{
		Unit*& unit = m_P1Units[i];
		if (unit)
		{
			unit->Update();
		}
	}

	for (int i = 0; i < m_P2Units.size(); ++i)
	{
		Unit*& unit = m_P2Units[i];
		if (unit)
		{
			unit->Update();
		}
	}

	if (m_game->m_currentTurn == GameTurn::P1)
	{
		for (int i = 0; i < m_P1Units.size(); ++i)
		{
			Unit*& unit = m_P1Units[i];
			if (unit)
			{
				if (unit->m_currentState == UnitState::move || unit->m_currentState == UnitState::confirmMove)
				{
					if (m_startMovingTile)
					{

					}
					else
					{
						m_startMovingTile = unit->m_currentTile;
					}
					if (unit->m_currentState == UnitState::move || unit->m_currentState == UnitState::confirmMove || unit->m_currentState == UnitState::attack || unit->m_currentState == UnitState::confirmAttack)
					{
						m_currentSelectUnit = unit;

						break;
					}
					break;
				}
				
				
			}
		}
	}
	else
	{
		for (int i = 0; i < m_P2Units.size(); ++i)
		{
			Unit*& unit = m_P2Units[i];
			if (unit)
			{
				if (unit->m_currentState == UnitState::move || unit->m_currentState == UnitState::confirmMove)
				{
					if (m_startMovingTile)
					{

					}
					else
					{
						m_startMovingTile = unit->m_currentTile;
					}
					if (unit->m_currentState == UnitState::move || unit->m_currentState == UnitState::confirmMove || unit->m_currentState == UnitState::attack || unit->m_currentState == UnitState::confirmAttack)
					{
						m_currentSelectUnit = unit;
						break;
					}
					break;
				}
				
				
			}
		}
	}
	
	
}

void Map::Render() const
{
	for (int i = 0; i < m_tiles.size(); ++i)
	{
		HexTile* const& tile = m_tiles[i];
		if (tile)
		{
			tile->Render();
		}
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindShader(m_def->m_shader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->DrawVertexArray(m_vertexBuffer, (int)m_indexes.size(), m_indexBuffer);

	//-----------------------------------------------------------------------------------
	if (m_startMovingTile)
	{
		m_currentSelectUnit->m_currentTile->RenderOuterBorder(1.5f);
		m_currentSelectUnit->m_currentTile->RenderInnerBorder(Rgba8::BLUE);
		for (int i = 0; i < m_tiles.size(); i++)
		{
			HexTile* const& tile = m_tiles[i];
			if (tile)
			{
				if (m_startMovingTile->DistTo(tile) <= m_currentSelectUnit->m_def->m_movementRange)
				{
					if (tile->m_worldPos.x >= m_def->m_worldBoundsMin.x && tile->m_worldPos.y >= m_def->m_worldBoundsMin.y
						&& tile->m_worldPos.x <= m_def->m_worldBoundsMax.x && tile->m_worldPos.y <= m_def->m_worldBoundsMax.y)
					{
						if (!tile->m_def->m_isBlocked)
						{
							tile->RenderOuterBorder(1.f);
						}
					}
				}
			}
		}
	}

	//-----------------------------------------------------------------------------------
	for (int i = 0; i < m_P1Units.size(); ++i)
	{
		Unit* const& unit = m_P1Units[i];
		if (unit)
		{
			unit->Render();
		}
	}

	for (int i = 0; i < m_P2Units.size(); ++i)
	{
		Unit* const& unit = m_P2Units[i];
		if (unit)
		{
			unit->Render();
		}
	}
}



Map* Map::LoadMapByName(std::string name)
{
	MapDefinition* def = MapDefinition::GetDefinitionByName(name);
	if (def)
	{
		return new Map(nullptr, name);
	}
	return nullptr;
}

HexTile* Map::GetTileByXY(int x, int y)
{
	for (int i = 0; i < m_tiles.size(); i++)
	{
		HexTile*& tile = m_tiles[i];
		if (tile->x == x && tile->y == y)
		{
			return tile;
		}
	}
	return nullptr;
}

void Map::ClearAllPath()
{
	for (int i = 0; i < m_tiles.size(); i++)
	{
		HexTile*& tile = m_tiles[i];
		tile->m_isRenderingAsPath = false;
	}
}

void Map::InitUnits()
{
	m_P1Units.clear();
	m_P2Units.clear();
	IntVec2 size = m_def->m_gridSize;
	int tileNum = size.x * size.y;
	for (int tileIndex = 0; tileIndex < tileNum; ++tileIndex)
	{
		std::string symbol = m_def->m_player1Units[tileIndex];
		UnitDefinition* def = UnitDefinition::GetDefinitionBySymbol(symbol);
		if (!def)
		{
			continue;
		}
		Unit* newUnit = new Unit(def, Game::m_OBJLoader, this);
		m_P1Units.push_back(newUnit);

		newUnit->m_currentTile = m_tiles[tileIndex];
		m_tiles[tileIndex]->m_unit = newUnit;
		newUnit->Move();
		newUnit->m_Tint = Rgba8::BLUE;
	}

	for (int tileIndex = 0; tileIndex < tileNum; ++tileIndex)
	{
		std::string symbol = m_def->m_player2Units[tileIndex];
		UnitDefinition* def = UnitDefinition::GetDefinitionBySymbol(symbol);
		if (!def)
		{
			continue;
		}
		Unit* newUnit = new Unit(def, Game::m_OBJLoader, this);
		m_P2Units.push_back(newUnit);

		newUnit->m_currentTile = m_tiles[tileIndex];
		m_tiles[tileIndex]->m_unit = newUnit;
		newUnit->Move();
		newUnit->m_Tint = Rgba8::RED;
	}
}

void Map::InitTiles()
{
	IntVec2 size = m_def->m_gridSize;
	int tileNum = size.x * size.y;
	m_tiles.reserve(tileNum);

	Vec3 m_startWorldPos = Vec3(0.f, 0.f, 0.f);

	for (int tileIndex = 0; tileIndex < tileNum; ++tileIndex)
	{
		HexTile* newTile = new HexTile();
		newTile->m_GRID_SIZE = m_def->m_gridSize.x;
		newTile->m_map = this;
		std::string symbol = m_def->m_tileSymbols[tileIndex];
		newTile->m_def = TileDefinition::GetDefinitionBySymbol(symbol);
		IntVec2 currentCoords;
		currentCoords.x = tileIndex % size.x;
		currentCoords.y = tileIndex / size.x;

		Vec3 IOffset = Vec3(0.866666f * newTile->m_inRadius * 2.f, 0.5f * newTile->m_inRadius * 2.f, 0.f);
		Vec3 JOffset = Vec3(0.f, newTile->m_inRadius * 2.f, 0.f);
		newTile->m_worldPos = m_startWorldPos + IOffset * (float)currentCoords.x + JOffset * (float)currentCoords.y;
		newTile->m_coord = currentCoords;
		newTile->x = currentCoords.x;
		newTile->y = currentCoords.y;
		newTile->z = - currentCoords.x - currentCoords.y;
		Vec3 midPos = newTile->m_worldPos;
		Vec3 L = midPos + Vec3(-newTile->m_circumRadius, 0.f, 0.f);
		Vec3 R = midPos + Vec3(newTile->m_circumRadius, 0.f, 0.f);
		Vec3 LT = midPos + Vec3(-newTile->m_circumRadius * 0.5f, newTile->m_inRadius, 0.f);
		Vec3 RT = midPos + Vec3(newTile->m_circumRadius * 0.5f, newTile->m_inRadius, 0.f);
		Vec3 LB = midPos + Vec3(-newTile->m_circumRadius * 0.5f, -newTile->m_inRadius, 0.f);
		Vec3 RB = midPos + Vec3(newTile->m_circumRadius * 0.5f, -newTile->m_inRadius, 0.f);

		newTile->m_vertexes.push_back(R);
		newTile->m_vertexes.push_back(RT);
		newTile->m_vertexes.push_back(LT);

		newTile->m_vertexes.push_back(L);
		newTile->m_vertexes.push_back(LB);
		newTile->m_vertexes.push_back(RB);

		m_tiles.push_back(newTile);

		if (newTile->m_def->m_name == "Dirt")
		{
			if (newTile->m_worldPos.x >= m_def->m_worldBoundsMin.x && newTile->m_worldPos.y >= m_def->m_worldBoundsMin.y
				&& newTile->m_worldPos.x <= m_def->m_worldBoundsMax.x && newTile->m_worldPos.y <= m_def->m_worldBoundsMax.y)
			{
				AddVertsForLineSegment3D(m_vertexes, m_indexes, L, LT, boundsThickness, Rgba8::WHITE);
				AddVertsForLineSegment3D(m_vertexes, m_indexes, LT, RT, boundsThickness, Rgba8::WHITE);
				AddVertsForLineSegment3D(m_vertexes, m_indexes, RT, R, boundsThickness, Rgba8::WHITE);
				
				AddVertsForLineSegment3D(m_vertexes, m_indexes, R, RB, boundsThickness, Rgba8::WHITE);
				AddVertsForLineSegment3D(m_vertexes, m_indexes, RB, LB, boundsThickness, Rgba8::WHITE);
				AddVertsForLineSegment3D(m_vertexes, m_indexes, LB, L, boundsThickness, Rgba8::WHITE);
			}
		}
		else
		{

		}
	}


	m_indexBuffer = g_theRenderer->CreateIndexBuffer(m_indexes.size() * sizeof(unsigned int));
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(m_vertexes.size() * sizeof(Vertex_PNCU), sizeof(Vertex_PNCU));

	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), (size_t)m_vertexes.size() * m_vertexBuffer->GetStride(), m_vertexBuffer);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), (size_t)m_indexes.size() * m_indexBuffer->GetStride(), m_indexBuffer);
}

void Map::RemoveUnitsFromMapIfUnderZeroHP()
{
	for (int i = 0; i < m_P1Units.size(); ++i)
	{
		Unit* const& unit = m_P1Units[i];
		if (unit)
		{
			if (unit->m_currentHP <= 0)
			{
				unit->m_currentTile->m_unit = nullptr;
				m_P1Units[i] = nullptr;
			}
		}
	}

	for (int i = 0; i < m_P2Units.size(); ++i)
	{
		Unit* const& unit = m_P2Units[i];
		if (unit)
		{
			if (unit->m_currentHP <= 0)
			{
				unit->m_currentTile->m_unit = nullptr;
				m_P2Units[i] = nullptr;
			}
		}
	}
}

