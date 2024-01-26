#include "HexTile.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Map.hpp"
#include <stack>

HexTile::HexTile()
{
	m_circumRadius = m_sideLength * 0.5f;
	m_inRadius = static_cast<float>((sqrt(3.0) / 2.0) * m_circumRadius);
}

HexTile::~HexTile()
{

}

void HexTile::Render()
{
	if (m_def->m_isBlocked)
	{
		std::vector<Vertex_PCU> m_highlightedTile;
		Rgba8 colorBackground = Rgba8(0, 0, 0, 255);
		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[0], colorBackground, Vec2::ZERO));
		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[1], colorBackground, Vec2::ZERO));
		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[2], colorBackground, Vec2::ZERO));

		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[0], colorBackground, Vec2::ZERO));
		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[2], colorBackground, Vec2::ZERO));
		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[3], colorBackground, Vec2::ZERO));

		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[0], colorBackground, Vec2::ZERO));
		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[3], colorBackground, Vec2::ZERO));
		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[4], colorBackground, Vec2::ZERO));

		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[0], colorBackground, Vec2::ZERO));
		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[4], colorBackground, Vec2::ZERO));
		m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[5], colorBackground, Vec2::ZERO));


		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->DrawVertexArray((int)m_highlightedTile.size(), m_highlightedTile.data());

		
	}
	if (m_isRenderingAsPath)
	{
		RenderOuterBorder(3.f);
	}
}

bool HexTile::IsPointInHexagon(Vec3 pos)
{
	for (int i = 0; i < m_vertexes.size(); i++)
	{
		Vec3 current = m_vertexes[i];
		Vec3 next = m_vertexes[0];
		if (i == m_vertexes.size() - 1)
		{
			next = m_vertexes[0];
		}
		else
		{
			next = m_vertexes[i + 1];
		}

		Vec3 E = next - current;
		Vec3 T = pos - current;

		Vec3 N = CrossProduct3D(E, T);
		if (N.z < 0.f)
		{
			return false;
		}
	}
	return true;
}

Vec3 HexTile::GetCenterPos() const
{
	return m_worldPos;
}

void HexTile::RenderInnerBorder(Rgba8 color)
{
	if (m_def->m_isBlocked)
	{
		return;
	}
	std::vector<Vertex_PCU> innerVerts;
	Vec3 midPos = m_worldPos;
	Vec3 L = midPos + Vec3(-m_circumRadius * 0.8f, 0.f, 0.f);
	Vec3 R = midPos + Vec3(m_circumRadius * 0.8f, 0.f, 0.f);
	Vec3 LT = midPos + Vec3(-m_circumRadius * 0.5f * 0.8f, m_inRadius * 0.8f, 0.f);
	Vec3 RT = midPos + Vec3(m_circumRadius * 0.5f * 0.8f, m_inRadius * 0.8f, 0.f);
	Vec3 LB = midPos + Vec3(-m_circumRadius * 0.5f * 0.8f, -m_inRadius * 0.8f, 0.f);
	Vec3 RB = midPos + Vec3(m_circumRadius * 0.5f * 0.8f, -m_inRadius * 0.8f, 0.f);

	AddVertsForLineSegment3D(innerVerts, L, LT, 0.05f, color);
	AddVertsForLineSegment3D(innerVerts, LT, RT, 0.05f, color);
	AddVertsForLineSegment3D(innerVerts, RT, R, 0.05f, color);

	AddVertsForLineSegment3D(innerVerts, R, RB, 0.05f, color);
	AddVertsForLineSegment3D(innerVerts, RB, LB, 0.05f, color);
	AddVertsForLineSegment3D(innerVerts, LB, L, 0.05f, color);

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray((int)innerVerts.size(), innerVerts.data());
}

void HexTile::RenderOuterBorder(float level)
{
	float thickness = 0.05f * level;
	std::vector<Vertex_PCU> innerVerts;
	AddVertsForLineSegment3D(innerVerts, m_vertexes[0], m_vertexes[1], thickness, Rgba8::WHITE);
	AddVertsForLineSegment3D(innerVerts, m_vertexes[1], m_vertexes[2], thickness, Rgba8::WHITE);
	AddVertsForLineSegment3D(innerVerts, m_vertexes[2], m_vertexes[3], thickness, Rgba8::WHITE);

	AddVertsForLineSegment3D(innerVerts, m_vertexes[3], m_vertexes[4], thickness, Rgba8::WHITE);
	AddVertsForLineSegment3D(innerVerts, m_vertexes[4], m_vertexes[5], thickness, Rgba8::WHITE);
	AddVertsForLineSegment3D(innerVerts, m_vertexes[5], m_vertexes[0], thickness, Rgba8::WHITE);

	std::vector<Vertex_PCU> m_highlightedTile;
	Rgba8 colorBackground = Rgba8(255, 255, 255, 25);
	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[0], colorBackground, Vec2::ZERO));
	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[1], colorBackground, Vec2::ZERO));
	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[2], colorBackground, Vec2::ZERO));

	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[0], colorBackground, Vec2::ZERO));
	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[2], colorBackground, Vec2::ZERO));
	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[3], colorBackground, Vec2::ZERO));

	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[0], colorBackground, Vec2::ZERO));
	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[3], colorBackground, Vec2::ZERO));
	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[4], colorBackground, Vec2::ZERO));

	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[0], colorBackground, Vec2::ZERO));
	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[4], colorBackground, Vec2::ZERO));
	m_highlightedTile.emplace_back(Vertex_PCU(m_vertexes[5], colorBackground, Vec2::ZERO));


	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray((int)m_highlightedTile.size(), m_highlightedTile.data());

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray((int)innerVerts.size(), innerVerts.data());
}

int HexTile::DistTo(HexTile* another)
{
	return (abs(x - another->x) + abs(y - another->y) + abs(z - another->z)) / 2;
}

int HexTile::DistToAnother(HexTile* another)
{
	std::vector<std::vector<int>> heatmap(m_GRID_SIZE, std::vector<int>(m_GRID_SIZE, 0));
	std::queue<HexTile*> queue;
	HexTile* centralTile = new HexTile(0, 0);
	queue.push(centralTile);


	for (int i = 0; i < m_map->m_tiles.size(); i++)
	{
		HexTile*& tile = m_map->m_tiles[i];
		if (tile->m_def->m_isBlocked)
		{
			//heatmap[tile->x + m_GRID_SIZE / 2][tile->y + m_GRID_SIZE / 2] = -1;
		}
	}
	
	while (!queue.empty()) {
		HexTile* currentTile = queue.front();
		queue.pop();

		for (int dx = -1; dx <= 1; ++dx) {
			for (int dy = -1; dy <= 1; ++dy) {
				//HexTile* neighborTile = m_map->GetTileByXY(currentTile->x + dx, currentTile->y + dy);
				HexTile* neighborTile = new HexTile(currentTile->x + dx, currentTile->y + dy);
				if (neighborTile->IsValid() && heatmap[neighborTile->x + m_GRID_SIZE / 2][neighborTile->y + m_GRID_SIZE / 2] == 0) {
					int distance = currentTile->DistTo(neighborTile);
					heatmap[neighborTile->x + m_GRID_SIZE / 2][neighborTile->y + m_GRID_SIZE / 2] = distance;
					//neighborTile->m_parent = currentTile;
					queue.push(neighborTile);
				}
			}
		}
	}
	//HighlightPath(another);
	return heatmap[another->x][another->y];
}

void HexTile::HighlightPath(HexTile* destination)
{
	std::stack<HexTile*> pathStack;
	HexTile* currentTile = destination;

	// Trace the path by following parent pointers
	while (currentTile != nullptr) {
		pathStack.push(currentTile);
		currentTile = currentTile->m_parent;
	}

	// Print the path
	while (!pathStack.empty()) {
		HexTile* pathTile = pathStack.top();
		pathTile->m_isRenderingAsPath = true;
		pathStack.pop();
	}
}

bool HexTile::IsValid() const
{
	return x >= -m_GRID_SIZE / 2 && x <= m_GRID_SIZE / 2 && y >= -m_GRID_SIZE / 2 && y <= m_GRID_SIZE / 2;
}

