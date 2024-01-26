#include "Button.hpp"
#include "App.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/VertexUtils.hpp"

Button::Button(float length, float width, bool checkCursorOverlap /*= false*/)
{
	m_bounds = AABB2(Vec2(0.f, 0.f), Vec2(length, width));
	m_checkCursor = checkCursorOverlap;
}

Button::~Button()
{

}

void Button::SetPosition(Vec2 pos)
{
	m_bounds.SetCenter(pos);
}

void Button::Update()
{
	Vec2 cursorUV = g_theWindow->GetCursorNormalizeUVs();
	Vec2 cursorPos = App::m_screenCamera->GetPosAtUV(cursorUV);

	//cursorPos = Vec2(g_theInput->GetCursorClientPosition().x, g_theWindow->m_fullScreenDimensions.y - g_theInput->GetCursorClientPosition().y);
	

	if (m_checkCursor)
	{
		if (m_bounds.IsPointInside(cursorPos))
		{
			m_isCusorOn = true;
			m_isCursonInBox = true;
		}
		else
		{
			m_isCursonInBox = false;
		}

		if (m_isCusorOn)
		{
			if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
			{
				OnButtonClick();
			}
		}
	}
}

void Button::Render() const
{
	Vec2 cursorUV = g_theWindow->GetCursorNormalizeUVs();
	Vec2 uv = g_theWindow->GetMouseOffset();
	Vec2 cursorPos = App::m_screenCamera->GetPosAtUV(cursorUV);
	//DebugDrawRing(cursorPos, 10.f, 1.f, Rgba8::RED);

	std::vector<Vertex_PCU> boxVerts;
	std::vector<Vertex_PCU> textVerts;
	
	Rgba8 txtColor, boxColor;

	if (m_isCusorOn)
	{
		txtColor = Rgba8::BLACK;
		boxColor = Rgba8::WHITE;
	}
	else
	{
		txtColor = Rgba8::WHITE;
		boxColor = Rgba8::BLACK;
	}
	if (m_showOutline)
	{
		AddVertsForAABB2(boxVerts, m_bounds, Rgba8::WHITE);
		AABB2 box;
		box.SetDimensions(Vec2(m_bounds.GetDimensions().x - 2.f, m_bounds.GetDimensions().y - 2.f));
		box.SetCenter(m_bounds.GetCenter());
		AddVertsForAABB2(boxVerts, box, Rgba8::BLACK);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray((int)boxVerts.size(), boxVerts.data());
	}


	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	float fontSize;
	if (m_overideFontSize == 0.f)
	{
		fontSize = 35.f;
	}
	else
	{
		fontSize = m_overideFontSize;
	}
	g_gameFont->AddVertsForTextInBox2D(textVerts, m_bounds, fontSize, m_text, txtColor, 1.2f, m_textAlignment);

	AddVertsForAABB2(boxVerts, m_bounds, boxColor);
	if (m_isCusorOn)
	{
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray((int)boxVerts.size(), boxVerts.data());
	}
	
	g_theRenderer->BindTexture(&g_gameFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
}

void Button::OnButtonClick()
{
	if (m_onClickFunction)
	{
		(*m_onClickFunction)(); // Call the member function using the function pointer
	}
}

void Button::BindButtonOnClick(CallbackFunction func)
{
	m_onClickFunction = func;
}

