#pragma once
#include "Entity.hpp"

struct CameraSettings
{
	Vec3 cameraStartPosition;
	Vec3 cameraFixedAngle;
	float cameraPanSpeed;
	float cameraElevateSpeed;
	float cameraFOVDegrees;
	float cameraNearClip;
	float cameraFarClip;
	float cameraZmin;
	Vec3 worldBoundsMax;
	Vec3 worldBoundsMin;
};

class Player : public Entity
{
public:
	Player(Game* owner);
	virtual ~Player();

	void Update(float deltaSeconds);
	void Render() const;

	void FindCursorPosWorldOnXY();
	void FindMidPointScreenWorldOnXY();
	void LoadCameraSettingsFromXML();
	void UpdateWorldBounds();
	Vec3 m_cameraCenterPositionInXYPlane;
	Vec3 m_cursorPositionInWorld;
	Vec3 m_midPointPositionInWorld;
	Vec3 m_dispMidPointPos;
public:
	Camera* m_camera = nullptr;
	float m_movementSpeed = 2.f;
	float m_turnRatePerSec = 90.f;
	CameraSettings m_cameraSettings;
private:
	float TanDegrees(float degrees);
};