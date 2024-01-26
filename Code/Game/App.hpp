#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

constexpr int MAX_NUM_KEYCODE = 256;
bool static m_isQuitting = false;
static std::string s_NetMode;
static bool s_windowFullScreen = true;
static IntVec2 s_windowSize;
static IntVec2 s_windowPos;
static std::string s_windowTitle;
static App* s_theApp;

class App
{
	friend class Game;
public :
	App();
	~App();
	void Startup();
	void Run();
	void Shutdown();
	void Runframe();
		
	bool IsQuitting() const { return m_isQuitting;  }
	bool HandleQuitRequested();
	bool OpenSlowMo();
	bool CloseSlowMo();
	bool SwitchPaused();
	void MoveOneStepThenPaused();
	bool const IsKeyDown(unsigned char keyCode);
	bool const WasKeyJustPressed(unsigned char keyCode);
	void ShowAttractMode();
	bool GetIsAttractMo();
	SoundPlaybackID GetBGMPlaybackID() const;
	void GetCmdLine(LPSTR cmd);
	void SendRemoteCommond(std::string cmd);

	static Camera* m_screenCamera;

private:
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();

	void UpdateShip(float deltaSeconds);
	void CopyIsDownToWasDown();
	void LoadAssets();

public:
	Game* m_theGame;
	SoundPlaybackID BGM;
	std::string m_cmdLine;
	
};