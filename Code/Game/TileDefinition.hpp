#pragma once
#include <string>
#include "Engine/Core/XmlUtils.hpp"


class TileDefinition
{
public:
	TileDefinition();
	~TileDefinition();

	bool LoadFromXmlElement(XmlElement const& element);
	static TileDefinition* GetDefinitionByName(std::string const& name);
	static TileDefinition* GetDefinitionBySymbol(std::string const& symbol);
	static std::vector<TileDefinition*> s_TileDefinitions;
public:
	std::string m_symbol;
	std::string m_name;
	bool m_isBlocked;

};