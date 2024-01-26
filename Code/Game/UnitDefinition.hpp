#pragma once
#include <string>
#include "Engine/Core/XmlUtils.hpp"


class UnitDefinition
{
public:
	UnitDefinition();
	~UnitDefinition();

	bool LoadFromXmlElement(XmlElement const& element);
	static UnitDefinition* GetDefinitionByName(std::string const& name);
	static UnitDefinition* GetDefinitionBySymbol(std::string const& symbol);
	static std::vector<UnitDefinition*> s_UnitDefinitions;
public:
	std::string m_symbol;
	std::string m_name;
	std::string m_modelFileName;
	std::string m_type;
	int m_groundAttackDamage = 0;
	int m_groundAttackRangeMin = 0;
	int m_groundAttackRangeMax = 0;
	int m_movementRange = 0;
	int m_defense = 0;
	int m_health = 0;
}; 
