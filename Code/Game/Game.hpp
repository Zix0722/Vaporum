#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>
#include "Map.hpp"

class App;
class Clock;
class Player;
class Entity;
class Stopwatch;
class Prop;
class Model;
class OBJLoader;
class HexTile;
class Button;
class Unit;

static Model* m_currentRenderingModel = nullptr;



enum class MenuState
{
	Splash,
	Mainmenu,
	Paused,
	InGame,
};

enum class GameTurn
{
	P1,
	P2,
	Nall,
};

class Game
{
public:
	Game(App* owner);
	~Game();
	void Startup();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	void Shutdown();
public :
	bool m_isAttractMode = true;
	bool g_DebugMo = false;
	std::string m_defualtMapName = "Grid12x12";
	Player* m_player = nullptr;
	std::string m_defaultModelPath;
	void ShowAttractMode() const;
	HexTile* m_curentCursorOverlap = nullptr;
	MenuState m_currenMenuState = MenuState::Splash;
	static Map* m_curentMap;
	static Map* GetCurrentMap();
    void StartLocalGame();
	static OBJLoader* m_OBJLoader;
	GameTurn m_currentTurn = GameTurn::P1;

public:
	
	void UpdateWorldCamera(float deltaSeconds);
	void UpdateScreenCamera(float deltaSeconds);
	void RendererGameTitle() const;

	void DrawSingleDotAt(Vec2 position, float scale = 1.0f, Rgba8 color = Rgba8(255,255,255)) const;
	void LoadingDefinitions();

	void CreatingMap();
	void GetCurrentSelectedTile();

	bool CursorOutOfWorldBounds(Vec3 const& cursorWorldPos);
	void RenderMainMenu() const;
	void RenderPausedMenu() const;
	void ResumeGame();
	void LoadingModels();
	void RenderInGameUI() const;
	void InitInGameButtons();
	void UpdateInfo();
	Unit* m_currentSelect = nullptr;
	void OnPreviousUnit();
	void OnNextUnit();
	void OnEndTurn();
	void OnCancel();
	void ChangeTurnInto(GameTurn next);
	void RenderSwitchTurnWarning() const;
	void UpdateButtonClicks();
	void UpdateMouseClickOnUnit();
	void CheckWinner();
	void RenderWinLog(GameTurn m_winner) const;
	void RenderGroundTexture() const;
	void RenderWaitingScreen() const;
public:
	App* m_app = nullptr;
	float m_backAttractCounter = 0.f;
	float m_CameraShakeLevel = 0.f;
	Clock* m_gameClock = nullptr;
	std::vector<Entity*> m_entities;

	Vec3 m_sunDirection = Vec3(0.5f, 0.5f, -1.0f);
	float m_sunIntensity = 0.9f;
	float m_ambientIntensity = 0.1f;

	Button* m_startButt = nullptr;
	Button* m_quitButt = nullptr;
	Button* m_resumeButton = nullptr;
	Button* m_menuButton = nullptr;
	bool m_showTurnSwitchWarning = false;
	GameTurn m_winner = GameTurn::Nall;
	
	//------------------------In game UI--------------------------------------------
	Button* m_selectButton = nullptr;
	Button* m_previousButton = nullptr;
	Button* m_nextButton = nullptr;
	Button* m_cancelButton = nullptr;
	Button* m_endButton = nullptr;

	//-------------------Info------------------------------------------------------
	Button* m_InfoName = nullptr;
	Button* m_InfoAttack = nullptr;
	Button* m_InfoDefense = nullptr;
	Button* m_InfoRange = nullptr;
	Button* m_InfoMove = nullptr;
	Button* m_InfoHealth = nullptr;

	std::string m_healthInfo;
	std::string m_rangeInfo;
	std::string m_DefenseInfo;
	std::string m_attackInfo;
	std::string m_nameInfo;
	std::string m_moveInfo;

	//------------------------------------------------------------------------
	bool m_isOtherPlayerConnecting = false;
	bool m_isFindingPlayer = false;
};