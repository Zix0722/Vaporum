#include "MapDefinition.hpp"
#include "GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"
std::vector<MapDefinition*> MapDefinition::s_mapDefinitions;


MapDefinition* MapDefinition::GetDefinitionByName(std::string const& name)
{
	for (int defIndex = 0; defIndex < s_mapDefinitions.size(); ++defIndex)
	{
		if (s_mapDefinitions[defIndex]->m_name == name)
		{
			return s_mapDefinitions[defIndex];
		}
	}
	return nullptr;
}


MapDefinition::MapDefinition()
{

}

MapDefinition::~MapDefinition()
{

}

bool MapDefinition::LoadFromXmlElement(XmlElement& element)
{
	m_name = ParseXMLAttribute(element, "name", m_name);
	std::string shader;
	shader = ParseXMLAttribute(element, "overlayShader", shader).c_str();
	if (shader == "Default")
	{
		m_shader = g_theRenderer->GetDefalutShader();
	}
	else
	{
		m_shader = g_theRenderer->CreateShader(shader.c_str());
	}
	m_gridSize = ParseXMLAttribute(element, "gridSize", m_gridSize);
	m_worldBoundsMin = ParseXMLAttribute(element, "worldBoundsMin", m_worldBoundsMin);
	m_worldBoundsMax = ParseXMLAttribute(element, "worldBoundsMax", m_worldBoundsMax);

	//Tiles Loading and Creating
	XmlElement* tilesElement;
	tilesElement = element.FirstChildElement("Tiles");
	const char* test;
	test = tilesElement->FirstChild()->ToText()->Value();
	Strings symbols = SplitStringOnDelimiter(test, '\n', true);
	for (int i = static_cast<int>(symbols.size() - 1); i > -1; i--)
	{
		Strings chars = SplitStringOnDelimiter(symbols[i], ' ', true);
		for (int charIndex = 0; charIndex < chars.size(); charIndex++)
		{
			m_tileSymbols.push_back(chars[charIndex]);
		}
	}
	// player1
	XmlElement* p1Element;
	p1Element = element.FirstChildElement("Units");
	if (!p1Element)
	{
		return true;
	}
	const char* test1;
	test1 = p1Element->FirstChild()->ToText()->Value();
	Strings symbols1 = SplitStringOnDelimiter(test1, '\n', true);
	for (int i = static_cast<int>(symbols1.size() - 1); i > -1; i--)
	{
		Strings chars = SplitStringOnDelimiter(symbols1[i], ' ', true);
		for (int charIndex = 0; charIndex < chars.size(); charIndex++)
		{
			m_player1Units.push_back(chars[charIndex]);
		}
	}

	//p2
	XmlElement* p2Element;
	p2Element = p1Element->NextSiblingElement("Units");
	if (!p2Element)
	{
		return true;
	}
	const char* test2;
	test2 = p2Element->FirstChild()->ToText()->Value();
	Strings symbols2 = SplitStringOnDelimiter(test2, '\n', true);
	for (int i = static_cast<int>(symbols2.size() - 1); i > -1; i--)
	{
		Strings chars = SplitStringOnDelimiter(symbols2[i], ' ', true);
		for (int charIndex = 0; charIndex < chars.size(); charIndex++)
		{
			m_player2Units.push_back(chars[charIndex]);
		}
	}
	
	return true;
}

