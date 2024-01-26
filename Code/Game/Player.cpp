#include "Player.hpp"
#include "GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Map.hpp"
#include <corecrt_math_defines.h>


constexpr float MOUSE_DELTA_SPEED = 2.f;

Player::Player(Game* owner)
	:Entity(owner)
{
	LoadCameraSettingsFromXML();
	
	m_camera = new Camera();
	m_camera->SetOrthographicView(Vec2(-1.f, -1.f), Vec2(1.f, 1.f));
	m_angularVelocity.m_pitchDegrees = m_turnRatePerSec;
	m_angularVelocity.m_yawDegrees = m_turnRatePerSec;
	m_angularVelocity.m_rollDegrees = m_turnRatePerSec;
	IntVec2 dims = g_theWindow->GetClientDimensions();
	m_camera->SetPerspectiveView(((float)dims.x / (float)dims.y), m_cameraSettings.cameraFOVDegrees, m_cameraSettings.cameraNearClip, m_cameraSettings.cameraFarClip);
	m_position = m_cameraSettings.cameraStartPosition;
	m_orientation.m_yawDegrees = m_cameraSettings.cameraFixedAngle.x;
	m_orientation.m_pitchDegrees = m_cameraSettings.cameraFixedAngle.y;
	m_orientation.m_rollDegrees = m_cameraSettings.cameraFixedAngle.z;
}

Player::~Player()
{
	delete m_camera;
	m_camera = nullptr;
}

void Player::Update(float deltaSeconds)
{
	UpdateWorldBounds();
	FindCursorPosWorldOnXY();
	FindMidPointScreenWorldOnXY();

	XboxController const& controller = g_theInput->GetController(0);
	float movementPan = deltaSeconds * m_cameraSettings.cameraPanSpeed;
	float movementZoom = deltaSeconds * m_cameraSettings.cameraElevateSpeed;
	Mat44 mat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	mat.Orthonormalize_XFwd_YLeft_ZUp();
	Vec3 iBasis = mat.GetIBasis3D();
	Vec3 jBasis = mat.GetJBasis3D();
	Vec3 kBasis = mat.GetKBasis3D();

	Vec3 panFwd = Vec3(0.f, 1.f, 0.f);
	Vec3 panLeft = Vec3(-1.f, 0.f, 0.f);
	Vec3 upDir = Vec3(0.f, 0.f, 1.f);

	Vec3 moveIntention;
	float speedFactor = 1.f;

	if (g_theInput->IsKeyDown(KEYCODE_LEFTSHIFT) || controller.IsButtonDown(XBOX_BUTTON_A))
	{
		speedFactor = 10.f;
	}
	if (g_theInput->IsKeyDown('W'))
	{
		moveIntention += panFwd * movementPan * speedFactor;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		moveIntention -= panFwd * movementPan * speedFactor;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		moveIntention += panLeft * movementPan * speedFactor;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		moveIntention -= panLeft * movementPan * speedFactor;
	}
	
	if (g_theInput->IsKeyDown('Q') || controller.GetLeftTrigger() > 0.2f)
	{
		//m_orientation.m_rollDegrees += m_angularVelocity.m_rollDegrees * deltaSeconds * speedFactor;
		moveIntention += upDir * movementZoom * speedFactor;
	}
	if (g_theInput->IsKeyDown('E') || controller.GetRightTrigger() > 0.2f)
	{
		//m_orientation.m_rollDegrees -= m_angularVelocity.m_rollDegrees * deltaSeconds * speedFactor;
		moveIntention -= upDir * movementZoom * speedFactor;
	}
	if (g_theInput->IsKeyDown('Z') || controller.IsButtonDown(XBOX_BUTTON_L))
	{
		m_position.z += movementZoom * speedFactor;
	}
	if (g_theInput->IsKeyDown('C') || controller.IsButtonDown(XBOX_BUTTON_R))
	{
		m_position.z -= movementZoom * speedFactor;
	}
	if (g_theInput->WasKeyJustPressed('H') || controller.WasButtonJustPressed(XBOX_BUTTON_START))
	{
		m_position = Vec3(0.f, 0.f, 0.f);
		m_orientation.m_yawDegrees = 0.f;
		m_orientation.m_pitchDegrees = 0.f;
		m_orientation.m_rollDegrees = 0.f;
	}
	if (g_theInput->WasKeyJustPressed('1'))
	{
		Vec3 respwanPos = m_position + iBasis * 2.f;
		DebugAddWorldWireSphere(respwanPos, 1.f, 5.f, Rgba8::GREEN, Rgba8::RED);
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		Vec3 lineDisp = Vec3::MakeFromPolarDegrees(m_orientation.m_pitchDegrees, m_orientation.m_yawDegrees, 20.f);
		DebugAddWorldLine(m_position, m_position + lineDisp, 0.06f, 10.f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::X_RAY);
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		DebugAddWorldArrow(m_position, m_position + 1.5f * iBasis, 0.1f, 20.f, Rgba8::RED, Rgba8::RED);
		DebugAddWorldArrow(m_position, m_position + 1.5f * jBasis, 0.1f, 20.f, Rgba8::GREEN, Rgba8::GREEN);
		DebugAddWorldArrow(m_position, m_position + 1.5f * kBasis, 0.1f, 20.f, Rgba8::BLUE, Rgba8::BLUE);
	}
	if (g_theInput->WasKeyJustPressed('4'))
	{
		std::string billboardStr;
		billboardStr = Stringf("Position: %.2f, %.2f, %.2f Orientation: %.2f, %.2f, %.2f", m_position.x, m_position.y, m_position.z, m_orientation.m_yawDegrees, m_orientation.m_pitchDegrees, m_orientation.m_rollDegrees);
		DebugAddWorldBillBoardText(billboardStr, m_position + iBasis, 0.1f, Vec2(0.5f, 0.5f), 10.f, Rgba8::WHITE, Rgba8::RED);
	}
	if (g_theInput->WasKeyJustPressed('5'))
	{
		Vec3 endPoint = Vec3::MakeFromPolarDegrees(-90.f, 0.f, 0.7f);
		DebugAddWorldWireCylinder(m_position, m_position + endPoint, 0.3f, 10.f, Rgba8::WHITE, Rgba8::RED);
	}
	if (g_theInput->IsKeyDown('6'))
	{
		Vec3 smallDotPos = Vec3(m_position.x, m_position.y, 0.f);
		DebugAddWorldPoint(smallDotPos, 0.1f, 60.f, Rgba8(150,75,0,255), Rgba8(150, 75, 0, 255));
	}
	if (g_theInput->WasKeyJustPressed('7'))
	{
		std::string message = "Camera Orientation: ";
		message = Stringf("Camera Orientation: %.2f, %.2f, %.2f", m_orientation.m_yawDegrees, m_orientation.m_pitchDegrees, m_orientation.m_rollDegrees);
		DebugAddMessage(message, 5.f);
	}
	float leftStickMag = controller.GetLeftStick().GetMagnitude();
	float rightStickMag = controller.GetRightStick().GetMagnitude();
	if (leftStickMag > 0.3f)
	{
		float speedRate = RangeMap(leftStickMag, 0.3f, 1.f, 0.f, 1.f);
		Vec2  leftStickPos = Vec2::MakeFromPolarDegrees(controller.GetLeftStick().GetOrientationDegrees()); 
		moveIntention += iBasis * movementPan * speedFactor * leftStickPos.y * speedRate;
		moveIntention -= jBasis * movementPan * speedFactor * leftStickPos.x * speedRate;
	}

	if (rightStickMag > 0.3f)
	{
		float speedRate = RangeMap(leftStickMag, 0.3f, 1.f, 0.f, 1.f);
		Vec2  RightStickPos = Vec2::MakeFromPolarDegrees(controller.GetRightStick().GetOrientationDegrees());
		m_orientation.m_pitchDegrees += m_angularVelocity.m_pitchDegrees * deltaSeconds * speedFactor * RightStickPos.y * speedRate;
		m_orientation.m_yawDegrees += m_angularVelocity.m_yawDegrees * deltaSeconds * speedFactor * RightStickPos.x * speedRate;
	}
	// ----------------------------------------Adding Text on Screen Camera---------------------------------------------------------------------------
	//std::string topLeftStr;
	//topLeftStr = Stringf("Player Position: %.2f, %.2f, %.2f", m_position.x, m_position.y, m_position.z);
	//DebugAddScreenText(topLeftStr, Vec2(0.f, 300.f), 5.f, Vec2(0.f, 0.99f), 0.f);
	//std::string topRightStr = "Time:  ";
	//float totalSec = Clock::GetSystemClock().GetTotalSeconds();
	//topRightStr.append(std::to_string(totalSec).substr(0, 4) + " ");
	//topRightStr.append("FPS: ");
	//float FPS = 1.f / Clock::GetSystemClock().GetDeltaSeconds();
	//topRightStr.append(std::to_string(FPS).substr(0, 4) + " ");
	//topRightStr.append("Scale: ");
	//float timeScale = Clock::GetSystemClock().GetTimeScale();
	//topRightStr.append(std::to_string(timeScale).substr(0, 4));
	//DebugAddScreenText(topRightStr, Vec2(1100.f, 300.f), 5.f, Vec2(0.99f, 0.99f), 0.f);
	//-------------------------------------------------------------------------------------------------------------------------------------------------
	//m_orientation.m_yawDegrees -= g_theInput->GetCursorClientDelta().x * MOUSE_DELTA_SPEED * deltaSeconds * speedFactor;
	//m_orientation.m_pitchDegrees += g_theInput->GetCursorClientDelta().y * MOUSE_DELTA_SPEED * deltaSeconds * speedFactor;
	//
	//m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
	//m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);

	m_position += moveIntention;
	m_position.z = GetClamped(m_position.z, m_cameraSettings.cameraZmin, m_cameraSettings.worldBoundsMax.z);
	m_position.x = GetClamped(m_position.x, m_cameraSettings.worldBoundsMin.x - m_dispMidPointPos.x, m_cameraSettings.worldBoundsMax.x - m_dispMidPointPos.x);
	m_position.y = GetClamped(m_position.y, m_cameraSettings.worldBoundsMin.y - m_dispMidPointPos.y, m_cameraSettings.worldBoundsMax.y - m_dispMidPointPos.y);
	m_camera->SetTransform(m_position, m_orientation);
}

void Player::Render() const
{

}

void Player::FindCursorPosWorldOnXY()
{
	float distToXYPlane = (float)fabs(m_position.z - 0.f);

	Mat44 mat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	mat.Orthonormalize_XFwd_YLeft_ZUp();
	Vec3 iBasis = mat.GetIBasis3D();
	Vec3 jBasis = mat.GetJBasis3D();
	Vec3 kBasis = mat.GetKBasis3D();

	float stepToXYPlane = (float)fabs(distToXYPlane / iBasis.z);
	m_cameraCenterPositionInXYPlane = m_position + (iBasis * stepToXYPlane);
	Vec2 const& cursorNormalizedPos = Window::GetWindowContext()->GetMouseOffset();

	float worldSpaceScreenHeight = stepToXYPlane * TanDegrees(m_camera->GetFOV() * 0.5f) * 2.f;
	float aspectRatio = Window::GetWindowContext()->GetConfig().m_clientAspect;
	//IntVec2 dims = g_theWindow->GetClientDimensions();
	//float aspectRatio = ((float)dims.x / (float)dims.y);
	float worldSpaceScreenWidth = worldSpaceScreenHeight * aspectRatio;
	float worldSpaceCursorPosX = cursorNormalizedPos.x * worldSpaceScreenWidth - (worldSpaceScreenWidth * 0.5f);
	float worldSpaceCursorPosY = cursorNormalizedPos.y * worldSpaceScreenHeight - (worldSpaceScreenHeight * 0.5f);

	Vec3 const& cursorPosInWorldSpace = m_cameraCenterPositionInXYPlane + (-jBasis * worldSpaceCursorPosX) + (kBasis * worldSpaceCursorPosY);
	Vec3 const& positionToCursorWorldPos = cursorPosInWorldSpace - m_position;
	float stepToXYPlaneInCursorDir = (float)fabs(distToXYPlane / positionToCursorWorldPos.z);
	m_cursorPositionInWorld = m_position + (positionToCursorWorldPos * stepToXYPlaneInCursorDir);

}

void Player::FindMidPointScreenWorldOnXY()
{
	float distToXYPlane = (float)fabs(m_position.z - 0.f);

	Mat44 mat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	mat.Orthonormalize_XFwd_YLeft_ZUp();
	Vec3 iBasis = mat.GetIBasis3D();
	Vec3 jBasis = mat.GetJBasis3D();
	Vec3 kBasis = mat.GetKBasis3D();

	float stepToXYPlane = (float)fabs(distToXYPlane / iBasis.z);
	m_cameraCenterPositionInXYPlane = m_position + (iBasis * stepToXYPlane);
	Vec2 const& cursorNormalizedPos = Vec2(0.5f, 0.5f);
	float worldSpaceScreenHeight = stepToXYPlane * TanDegrees(m_camera->GetFOV() * 0.5f) * 2.f;
	float aspectRatio = Window::GetWindowContext()->GetConfig().m_clientAspect;
	float worldSpaceScreenWidth = worldSpaceScreenHeight * aspectRatio;
	float worldSpaceCursorPosX = cursorNormalizedPos.x * worldSpaceScreenWidth - (worldSpaceScreenWidth * 0.5f);
	float worldSpaceCursorPosY = cursorNormalizedPos.y * worldSpaceScreenHeight - (worldSpaceScreenHeight * 0.5f);

	Vec3 const& cursorPosInWorldSpace = m_cameraCenterPositionInXYPlane + (-jBasis * worldSpaceCursorPosX) + (kBasis * worldSpaceCursorPosY);
	Vec3 const& positionToCursorWorldPos = cursorPosInWorldSpace - m_position;
	float stepToXYPlaneInCursorDir = (float)fabs(distToXYPlane / positionToCursorWorldPos.z);
	m_dispMidPointPos = (positionToCursorWorldPos * stepToXYPlaneInCursorDir);
	m_midPointPositionInWorld = m_position + (positionToCursorWorldPos * stepToXYPlaneInCursorDir);
}

void Player::LoadCameraSettingsFromXML()
{
	XmlDocument document;
	document.LoadFile("Data/GameConfig.xml");
	XmlElement* rootElement = document.RootElement();
	m_cameraSettings.cameraStartPosition = ParseXMLAttribute(*rootElement, "cameraStartPosition", m_cameraSettings.cameraStartPosition);
	m_cameraSettings.cameraFixedAngle = ParseXMLAttribute(*rootElement, "cameraFixedAngle", m_cameraSettings.cameraFixedAngle);
	m_cameraSettings.cameraPanSpeed = ParseXMLAttribute(*rootElement, "cameraPanSpeed", m_cameraSettings.cameraPanSpeed);
	m_cameraSettings.cameraElevateSpeed = ParseXMLAttribute(*rootElement, "cameraElevateSpeed", m_cameraSettings.cameraElevateSpeed);
	m_cameraSettings.cameraFOVDegrees = ParseXMLAttribute(*rootElement, "cameraFOVDegrees", m_cameraSettings.cameraFOVDegrees);
	m_cameraSettings.cameraNearClip = ParseXMLAttribute(*rootElement, "cameraNearClip", m_cameraSettings.cameraNearClip);
	m_cameraSettings.cameraFarClip = ParseXMLAttribute(*rootElement, "cameraFarClip", m_cameraSettings.cameraFarClip);
	m_cameraSettings.cameraZmin = ParseXMLAttribute(*rootElement, "cameraMinHeight", m_cameraSettings.cameraZmin);
}

void Player::UpdateWorldBounds()
{
	m_cameraSettings.worldBoundsMax = Game::GetCurrentMap()->m_def->m_worldBoundsMax;
	m_cameraSettings.worldBoundsMin = Game::GetCurrentMap()->m_def->m_worldBoundsMin;
}

float Player::TanDegrees(float degrees)
{
	float radians = degrees * (float)(M_PI / 180.f);
	return tanf(radians);
}

