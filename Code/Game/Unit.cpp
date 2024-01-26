#include "Unit.hpp"
#include "Engine/Renderer/Model.hpp"
#include "GameCommon.hpp"
#include "Game/HexTile.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"

Unit::Unit(UnitDefinition* def, OBJLoader* objLoader, Map* map)
	:m_def(def)
	,m_map(map)
{
	m_model = objLoader->LoadOrGetModelFromPath(m_def->m_modelFileName.c_str());
	m_currentHP = m_def->m_health;
}

Unit::~Unit()
{

}

void Unit::Move()
{
	if (m_currentTile)
	{
		Vec3 worldPos = m_currentTile->m_worldPos;
		if (m_model)
		{
			m_overrideTransform = m_model->m_transform;
		}
		m_overrideTransform.t = worldPos;
	}
}

void Unit::Update()
{
	if(m_map->m_game->m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER || m_map->m_game->m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
		{
			switch (m_currentState)
			{
			case UnitState::Unmoved:
				break;
			case UnitState::move:
				MoveBehavior();
				m_map->m_game->m_app->SendRemoteCommond("RemoteCommand Command=\"Move\"");
				break;
			case UnitState::confirmMove:
				ConfirmMove();
				m_map->m_game->m_app->SendRemoteCommond("RemoteCommand Command=\"ConfirmMove\"");
				break;
			case UnitState::attack:
				Attack();
				m_map->m_game->m_app->SendRemoteCommond("RemoteCommand Command=\"Attack\"");
				break;
			case UnitState::confirmAttack:
				ConfirmAttack();
				m_map->m_game->m_app->SendRemoteCommond("RemoteCommand Command=\"ConfirmAttack\"");
				break;
			case UnitState::moved:
				break;
			default:
				break;
			}
		}

	}
	
}

void Unit::Render()
{
	if (m_model)
	{
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->BindShader(m_model->m_shader);
		g_theRenderer->BindTexture(m_model->m_texture);

		Mat44 matrix;
		matrix.SetIJK3D(m_overrideTransform.x, m_overrideTransform.y, m_overrideTransform.z);
		matrix.SetTranslation3D(m_overrideTransform.t);
		matrix.AppendScaleUniform3D(m_overrideTransform.scale);
		if (m_currentState == UnitState::moved)
		{
			g_theRenderer->SetModelConstants(matrix, Rgba8::GREY);
		}
		else
		{
			g_theRenderer->SetModelConstants(matrix, m_Tint);
		}
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);

		if (m_model->m_gpuMesh.m_indexes != nullptr)
		{
			m_model->m_gpuMesh.Render(g_theRenderer, (int)m_model->m_cpuMesh.m_indexes.size());
		}
		else
		{
			m_model->m_cpuMesh.Render(g_theRenderer);
		}
	}

	if (m_targets.size() > 0)
	{
		for (int i = 0; i < m_targets.size(); i++)
		{
			if (m_targets[i] == m_map->m_game->m_curentCursorOverlap)
			{
				m_targets[i]->RenderInnerBorder(Rgba8::RED);
			}
			else
			{
				m_targets[i]->RenderInnerBorder(Rgba8(100,0,0));
			}
		}
	}
}

void Unit::DoCombat(Unit* target)
{
	int my_dmgTaken;
	if (target->m_def->m_groundAttackRangeMax >= m_def->m_groundAttackRangeMax)
	{
		my_dmgTaken = 2 * target->m_def->m_groundAttackDamage / m_def->m_defense;
	}
	else
	{
		my_dmgTaken = 0;
	}
	 
	int his_dmgTaken = 2 * m_def->m_groundAttackDamage / target->m_def->m_defense;
	m_currentHP -= my_dmgTaken;
	target->m_currentHP -= his_dmgTaken;
}

void Unit::MoveBehavior()
{
	if (m_map->m_game->m_curentCursorOverlap && !m_map->m_game->m_curentCursorOverlap->m_def->m_isBlocked)
	{
		if (m_map->m_game->m_curentCursorOverlap->DistTo(m_currentTile) <= m_def->m_movementRange && !m_map->m_game->m_curentCursorOverlap->m_unit)
		{
			m_currentTile->m_unit = nullptr;
			m_currentTile = m_map->m_game->m_curentCursorOverlap;
			m_currentTile->m_unit = this;
			Move();
			m_currentState = UnitState::confirmMove;
		}
	}
}

void Unit::ConfirmMove()
{
	//m_map->m_currentSelectUnit = nullptr;
	m_map->m_startMovingTile = nullptr;
	m_currentState = UnitState::attack;
	m_targets.clear();

	if (m_Tint == Rgba8::BLUE)
	{
		for (int i = 0; i < m_map->m_P2Units.size(); i++)
		{
			Unit*& unit = m_map->m_P2Units[i];
			if (unit)
			{
				if (m_currentTile->DistTo(unit->m_currentTile) <= m_def->m_groundAttackRangeMax && m_currentTile->DistTo(unit->m_currentTile) >= m_def->m_groundAttackRangeMin)
				{
					m_targets.push_back(unit->m_currentTile);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < m_map->m_P1Units.size(); i++)
		{
			Unit*& unit = m_map->m_P1Units[i];
			if (unit)
			{
				if (m_currentTile->DistTo(unit->m_currentTile) <= m_def->m_groundAttackRangeMax && m_currentTile->DistTo(unit->m_currentTile) >= m_def->m_groundAttackRangeMin)
				{
					m_targets.push_back(unit->m_currentTile);
				}
			}
		}
	}

}

void Unit::Attack()
{
	if (m_map->m_game->m_curentCursorOverlap)
	{
		bool isTarget = false;
		for (int i = 0; i < m_targets.size(); i++)
		{
			if (m_targets[i] = m_map->m_game->m_curentCursorOverlap)
			{
				isTarget = true;
				DoCombat(m_targets[i]->m_unit);
				m_targets.clear();
			}
		}

	}
	m_currentState = UnitState::confirmAttack;
}

void Unit::ConfirmAttack()
{
	m_map->m_currentSelectUnit = nullptr;
	m_currentState = UnitState::moved;
}
