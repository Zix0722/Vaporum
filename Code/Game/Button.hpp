#pragma once
#include "Engine/Math/AABB2.hpp"
#include <string>
class Texture;


typedef void (*CallbackFunction)();

class Button
{
public:
	Button(float length, float width, bool checkCursorOverlap = false);
	~Button();

	void SetPosition(Vec2 pos);
	void Update();
	void Render() const;

	void OnButtonClick();
	void BindButtonOnClick(CallbackFunction func);
public:
	bool m_checkCursor = false;
	bool m_showOutline = false;
	AABB2 m_bounds;
	Texture* m_textrue = nullptr;
	bool m_isCusorOn = false;
	std::string m_text;
	Vec2 m_textAlignment = Vec2(0.05f, 0.5f);
	bool m_isCursonInBox = false;
	CallbackFunction m_onClickFunction;
	float m_overideFontSize = 0.f;

};