#include "TileDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::vector<TileDefinition*> TileDefinition::s_TileDefinitions;

TileDefinition::TileDefinition()
{

}

TileDefinition::~TileDefinition()
{

}

bool TileDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXMLAttribute(element, "name", m_name);
	std::string str_symbol;
	str_symbol = ParseXMLAttribute(element, "symbol", str_symbol);
	m_symbol = str_symbol;
	//DebuggerPrintf("%s\n", m_symbol);
	m_isBlocked = ParseXMLAttribute(element, "isBlocked", m_isBlocked);

	
	return true;
}

TileDefinition* TileDefinition::GetDefinitionByName(std::string const& name)
{
	for (int defIndex = 0; defIndex < s_TileDefinitions.size(); ++defIndex)
	{
		if (s_TileDefinitions[defIndex]->m_name == name)
		{
			return s_TileDefinitions[defIndex];
		}
	}
	return nullptr;
}

TileDefinition* TileDefinition::GetDefinitionBySymbol(std::string const& symbol)
{
	for (int defIndex = 0; defIndex < s_TileDefinitions.size(); ++defIndex)
	{
		if (s_TileDefinitions[defIndex]->m_symbol == symbol)
		{
			return s_TileDefinitions[defIndex];
		}
	}
	return nullptr;
}

