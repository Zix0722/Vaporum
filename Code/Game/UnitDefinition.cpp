#include "UnitDefinition.hpp"

std::vector<UnitDefinition*> UnitDefinition::s_UnitDefinitions;

UnitDefinition::UnitDefinition()
{

}

UnitDefinition::~UnitDefinition()
{

}

bool UnitDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXMLAttribute(element, "name", m_name);
	std::string str_symbol;
	str_symbol = ParseXMLAttribute(element, "symbol", str_symbol);
	m_symbol = str_symbol.c_str();
	std::string modelFileStr;
	m_modelFileName = ParseXMLAttribute(element, "modelFilename", modelFileStr);
	m_type = ParseXMLAttribute(element, "type", modelFileStr);
	m_groundAttackDamage = ParseXMLAttribute(element, "groundAttackDamage", 0);
	m_groundAttackRangeMin = ParseXMLAttribute(element, "groundAttackRangeMin", 0);
	m_groundAttackRangeMax = ParseXMLAttribute(element, "groundAttackRangeMax", 0);
	m_movementRange = ParseXMLAttribute(element, "movementRange", 0);
	m_defense = ParseXMLAttribute(element, "defense", 0);
	m_health = ParseXMLAttribute(element, "health", 0);
	return true;
}

UnitDefinition* UnitDefinition::GetDefinitionByName(std::string const& name)
{
	for (int defIndex = 0; defIndex < s_UnitDefinitions.size(); ++defIndex)
	{
		if (s_UnitDefinitions[defIndex]->m_name == name)
		{
			return s_UnitDefinitions[defIndex];
		}
	}
	return nullptr;
}

UnitDefinition* UnitDefinition::GetDefinitionBySymbol(std::string const& symbol)
{
	for (int defIndex = 0; defIndex < s_UnitDefinitions.size(); ++defIndex)
	{
		if (s_UnitDefinitions[defIndex]->m_symbol == symbol)
		{
			return s_UnitDefinitions[defIndex];
		}
	}
	return nullptr;
}

