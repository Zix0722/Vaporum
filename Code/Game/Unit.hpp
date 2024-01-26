#pragma once
#include "UnitDefinition.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Renderer/Model.hpp"

class HexTile;
class Map;

enum class UnitState
{
	Unmoved,
	move,
	confirmMove,
	attack,
	confirmAttack,
	moved,
};

class Unit
{
public:
	Unit(UnitDefinition* def, OBJLoader* objLoader, Map* map);
	~Unit();
	void Move();
	void Update();
	void Render();
	void DoCombat(Unit* target);
	void MoveBehavior();
	void ConfirmMove();
	void Attack();
	void ConfirmAttack();
public:
	Map* m_map = nullptr;
	UnitDefinition* m_def;
	UnitState m_currentState = UnitState::Unmoved;
	Model* m_model = nullptr;
	HexTile* m_currentTile = nullptr;
	modelTransform m_overrideTransform;
	Rgba8 m_Tint = Rgba8::WHITE;
	int m_currentHP = 0;
	std::vector<HexTile*> m_targets;
};