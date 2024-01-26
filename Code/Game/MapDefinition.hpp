#pragma once
#include <string>
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"

class Shader;

class MapDefinition
{
public:
	MapDefinition();
	~MapDefinition();

	bool LoadFromXmlElement(XmlElement& element);
	static MapDefinition* GetDefinitionByName(std::string const& name);
	static std::vector<MapDefinition*> s_mapDefinitions;
public:
	std::string m_name;
	Shader* m_shader = nullptr;
	IntVec2 m_gridSize;
	Vec3 m_worldBoundsMin;
	Vec3 m_worldBoundsMax;

	//Tiles
	std::vector<std::string> m_tileSymbols;
	std::vector<std::string> m_player1Units;
	std::vector<std::string> m_player2Units;
};