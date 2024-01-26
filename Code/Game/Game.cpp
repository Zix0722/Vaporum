#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Player.hpp"
#include "Prop.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Renderer/Model.hpp"
#include "MapDefinition.hpp"
#include "UnitDefinition.hpp"
#include "TileDefinition.hpp"
#include "Map.hpp"
#include "GameCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/HexTile.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/Button.hpp"
#include "Unit.hpp"

Map* Game::m_curentMap = nullptr;
BitmapFont* g_gameFont = nullptr;
OBJLoader* Game::m_OBJLoader = nullptr;

Map* Game::GetCurrentMap()
{
	return m_curentMap;
}


void Game::StartLocalGame()
{
	m_currenMenuState = MenuState::InGame;
	m_isFindingPlayer = true;
	m_curentMap->InitUnits();
	ChangeTurnInto(GameTurn::P1);
	
}


bool LoadModelFile(EventArgs& args)
{
	std::string path;
	path = args.GetValue("File", path);
	m_currentRenderingModel = Game::m_OBJLoader->LoadOrGetModelFromPath(path.c_str());
	return true;
}

bool LoadMap(EventArgs& args)
{
	std::string name;
	name = args.GetValue("Name", name);
	Map* newMap = Map::LoadMapByName(name);
	if (newMap)
	{
		Game::m_curentMap = newMap;
		return true;
	}
	else
	{
		g_theDevConsole->AddLine(Rgba8::RED, "Load Map Failed");
	}
	return false;
}

RandomNumberGenerator* g_theRNG = nullptr;
extern Renderer* g_theRenderer;
extern DevConsole* g_theDevConsole;

Game::Game(App* owner)
	:m_app(owner)
{
	g_theRNG = new RandomNumberGenerator();
	m_gameClock = new Clock();
}

Game::~Game()
{
	delete m_gameClock;
}

void Game::Startup()
{
	SubscribeEventCallbackFunction("LoadMap", LoadMap);
	SubscribeEventCallbackFunction("LoadModel", LoadModelFile);

	
	m_app->m_screenCamera->SetOrthoView(Vec2(0.f, 0.f), g_theWindow->m_fullScreenDimensions);
	m_app->m_screenCamera->SetOrthographicView(Vec2(0.f, 0.f), g_theWindow->m_fullScreenDimensions);

	m_player = new Player(this);

	g_gameFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");



	UpdateWorldCamera(0.f);


	m_entities.push_back(m_player);

	//m_entities.push_back(m_rotatingCube);
	//m_entities.push_back(m_blinkingCube);
	//m_entities.push_back(m_sphere);
	m_OBJLoader = new OBJLoader();
	//m_currentRenderingModel = m_OBJLoader->LoadOrGetModelFromPath(m_defaultModelPath.c_str());

	LoadingDefinitions();
	CreatingMap();
	LoadingModels();

	Vec2 dimensions = g_theWindow->m_fullScreenDimensions;
	Vec2 midPos = 0.5f * dimensions;
	m_startButt = new Button(dimensions.x * 0.4f, dimensions.y * 0.03f, true);
	m_quitButt = new Button(dimensions.x * 0.4f, dimensions.y * 0.03f, true);
	m_startButt->SetPosition(midPos + Vec2(-dimensions.y * 0.172f, 0.f));
	m_quitButt->SetPosition(midPos + Vec2(-dimensions.y * 0.172f, -dimensions.y * 0.03f));
	m_startButt->m_text = "New Game";
	m_quitButt->m_text = "Quit";
	m_startButt->m_isCusorOn = true;
	m_resumeButton = new Button(dimensions.x * 0.4f, dimensions.y * 0.03f, true);
	m_menuButton = new Button(dimensions.x * 0.4f, dimensions.y * 0.03f, true);
	m_resumeButton->SetPosition(midPos + Vec2(-dimensions.y * 0.172f, 0.f));
	m_menuButton->SetPosition(midPos + Vec2(-dimensions.y * 0.172f, -dimensions.y * 0.03f));
	m_resumeButton->m_text = "Resume Game";
	m_menuButton->m_text = "Main Menu";
	m_resumeButton->m_isCusorOn = true;

	InitInGameButtons();
}

void Game::Update(float deltaSeconds)
{
	Vec2 cursorUV = g_theWindow->GetCursorNormalizeUVs();
	Vec2 cursorPos = App::m_screenCamera->GetPosAtUV(cursorUV);
	switch (m_currenMenuState)
	{
	case MenuState::Splash:
		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB) || g_theInput->WasKeyJustPressed(ENTER_KEY))
		{
			m_currenMenuState = MenuState::Mainmenu;
		}
		break;
	case MenuState::Mainmenu:
		m_startButt->Update();
		m_quitButt->Update();
		
		if (m_quitButt->m_isCursonInBox)
		{
			m_startButt->m_isCusorOn = false;
		}
		else if (m_startButt->m_isCursonInBox)
		{
			m_quitButt->m_isCusorOn = false;
		}


		if (m_startButt->m_bounds.IsPointInside(cursorPos) || m_quitButt->m_bounds.IsPointInside(cursorPos))
		{
		}
		else
		{
			if (g_theInput->WasKeyJustPressed(KEYCODE_UPARROW) || g_theInput->WasKeyJustPressed(KEYCODE_DOWNARROW))
			{
				if (m_startButt->m_isCusorOn)
				{
					m_quitButt->m_isCusorOn = true;
					m_startButt->m_isCusorOn = false;
				}
				else
				{
					m_quitButt->m_isCusorOn = false;
					m_startButt->m_isCusorOn = true;
				}
			}
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB) || g_theInput->WasKeyJustPressed(ENTER_KEY))
		{
			if (m_startButt->m_isCusorOn)
			{
				StartLocalGame();
			}
			else if (m_quitButt->m_isCusorOn)
			{
				FireEvent("quit");
			}
		}

		break;
	case MenuState::Paused:
		m_resumeButton->Update();
		m_menuButton->Update();

		if (m_menuButton->m_isCursonInBox)
		{
			m_resumeButton->m_isCusorOn = false;
		}
		else if (m_resumeButton->m_isCursonInBox)
		{
			m_menuButton->m_isCusorOn = false;
		}


		if (m_resumeButton->m_bounds.IsPointInside(cursorPos) || m_menuButton->m_bounds.IsPointInside(cursorPos))
		{
		}
		else
		{
			if (g_theInput->WasKeyJustPressed(KEYCODE_UPARROW) || g_theInput->WasKeyJustPressed(KEYCODE_DOWNARROW))
			{
				if (m_resumeButton->m_isCusorOn)
				{
					m_menuButton->m_isCusorOn = true;
					m_resumeButton->m_isCusorOn = false;
				}
				else
				{
					m_menuButton->m_isCusorOn = false;
					m_resumeButton->m_isCusorOn = true;
				}
			}
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB) || g_theInput->WasKeyJustPressed(ENTER_KEY))
		{
			if (m_resumeButton->m_isCusorOn)
			{
				ResumeGame();
			}
			else if (m_menuButton->m_isCusorOn)
			{
				m_resumeButton->m_isCusorOn = true;
				m_menuButton->m_isCusorOn = false;
				m_currenMenuState = MenuState::Mainmenu;
			}
		}
		break;
	case MenuState::InGame:
		for (int entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex)
		{
			if (m_entities[entityIndex] != nullptr)
			{
				m_entities[entityIndex]->Update(deltaSeconds);
			}
		}
		
		if (!m_isOtherPlayerConnecting)
		{
			s_theApp->SendRemoteCommond("RemoteCommand Command=\"Ready\"");
			return;
		}


		if (m_showTurnSwitchWarning)
		{
			if (g_theInput->WasKeyJustPressed(KEYCODE_ENTER) || g_theInput->WasKeyJustPressed(KEYCODE_LMB))
			{
				if (m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER)
				{
					s_theApp->SendRemoteCommond("RemoteCommand Command=\"StartTurn\"");
					m_showTurnSwitchWarning = false;
				}
				else if (m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
				{
					m_showTurnSwitchWarning = false;
					s_theApp->SendRemoteCommond("RemoteCommand Command=\"StartTurn\"");
				}
			}
			return;
		}
		CheckWinner();
		if (m_winner != GameTurn::Nall)
		{
			if (g_theInput->WasKeyJustPressed(KEYCODE_ENTER) || g_theInput->WasKeyJustPressed(KEYCODE_LMB))
			{
				m_currenMenuState = MenuState::Mainmenu;
			}
			return;
		}
		if (g_theInput->WasKeyJustPressed('P'))
		{
			m_currenMenuState = MenuState::Paused;
		}


		if (g_theInput->WasKeyJustPressed('L'))
		{
			std::string str;
			g_theWindow->GetXMLPathByOpenFileNameA(str);
			m_currentRenderingModel = m_OBJLoader->LoadOrGetModelFromPath(str.c_str());
		}


		

		m_curentMap->Update();
		GetCurrentSelectedTile();
		UpdateButtonClicks();
		UpdateMouseClickOnUnit();
		UpdateInfo();
		break;
	default:
		break;
	}

}

void Game::Render() const
{
	switch (m_currenMenuState)
	{
		case MenuState::Splash:
			g_theRenderer->BeginCamera(*m_app->m_screenCamera);
			ShowAttractMode();
			g_theRenderer->EndCamera(*m_app->m_screenCamera);
			DebugRenderScreen(*m_app->m_screenCamera);
			return;
		case MenuState::Mainmenu:
			g_theRenderer->BeginCamera(*m_app->m_screenCamera);
			RenderMainMenu();
			g_theRenderer->EndCamera(*m_app->m_screenCamera);
			DebugRenderScreen(*m_app->m_screenCamera);
			return;
		case MenuState::InGame:
			g_theRenderer->ClearScreen(Rgba8(100, 100, 100));
			g_theRenderer->BeginCamera(*m_player->m_camera);
			g_theRenderer->UpdateLightConstantsBuffer(m_sunDirection, m_sunIntensity, m_ambientIntensity);

			if (!m_isOtherPlayerConnecting)
			{
				g_theRenderer->ClearScreen(Rgba8::BLACK);
				g_theRenderer->BeginCamera(*m_app->m_screenCamera);
				RenderWaitingScreen();
				return;
			}

			RenderGroundTexture();
			m_curentMap->Render();
			

			//if (m_curentMap->m_def)
			//{
			//	std::vector<Vertex_PCU> lineBounds;
			//	Vec3 min = m_curentMap->m_def->m_worldBoundsMin;
			//	Vec3 max = m_curentMap->m_def->m_worldBoundsMax;
			//	AddVertsForLineSegment3D(lineBounds, Vec3(min.x, min.y, 0.f), Vec3(min.x, max.y, 0.f), 0.02f, Rgba8::MAGENTA);
			//	AddVertsForLineSegment3D(lineBounds, Vec3(min.x, min.y, 0.f), Vec3(max.x, min.y, 0.f), 0.02f, Rgba8::MAGENTA);
			//	AddVertsForLineSegment3D(lineBounds, Vec3(max.x, max.y, 0.f), Vec3(min.x, max.y, 0.f), 0.02f, Rgba8::MAGENTA);
			//	AddVertsForLineSegment3D(lineBounds, Vec3(max.x, max.y, 0.f), Vec3(max.x, min.y, 0.f), 0.02f, Rgba8::MAGENTA);
			//	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
			//	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
			//	g_theRenderer->BindTexture(nullptr);
			//	g_theRenderer->BindShader(nullptr);
			//	g_theRenderer->SetModelConstants();
			//	g_theRenderer->DrawVertexArray((int)lineBounds.size(), lineBounds.data());
			//}

			if (m_curentCursorOverlap)
			{
				if (m_currentTurn == GameTurn::P1)
				{
					if (!m_curentCursorOverlap->m_unit)
					{
						m_curentCursorOverlap->RenderInnerBorder(Rgba8::GREEN);
					}
					else if (m_curentCursorOverlap->m_unit->m_Tint == Rgba8::RED)
					{
						m_curentCursorOverlap->RenderInnerBorder(Rgba8::RED);
					}
					else
					{
						m_curentCursorOverlap->RenderInnerBorder(Rgba8::GREEN);
					}
				}
				else
				{
					if (!m_curentCursorOverlap->m_unit)
					{
						m_curentCursorOverlap->RenderInnerBorder(Rgba8::GREEN);
					}
					else if (m_curentCursorOverlap->m_unit->m_Tint == Rgba8::BLUE)
					{
						m_curentCursorOverlap->RenderInnerBorder(Rgba8::RED);
					}
					else
					{
						m_curentCursorOverlap->RenderInnerBorder(Rgba8::GREEN);
					}
				}
				
			}


			for (int entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex)
			{
				if (m_entities[entityIndex] != nullptr)
				{
					m_entities[entityIndex]->Render();
				}
			}

			if (m_currentRenderingModel)
			{
				m_currentRenderingModel->Render();
			}

			g_theRenderer->EndCamera(*m_player->m_camera);

			DebugRenderWorld(*m_player->m_camera);
			DebugRenderScreen(*m_app->m_screenCamera);

			g_theRenderer->BeginCamera(*m_app->m_screenCamera);
			if (m_showTurnSwitchWarning)
			{
				RenderSwitchTurnWarning();
			}
			else
			{
				RenderInGameUI();
			}
			if (m_winner != GameTurn::Nall)
			{
				RenderWinLog(m_winner);
			}

			g_theRenderer->EndCamera(*m_app->m_screenCamera);

			g_theRenderer->BeginCamera(*m_app->m_screenCamera);
			g_theDevConsole->Render(AABB2(m_app->m_screenCamera->GetOrthoBottomLeft(), m_app->m_screenCamera->GetOrthoTopRight()));
			g_theRenderer->EndCamera(*m_app->m_screenCamera);
			return;
		case MenuState::Paused:
			g_theRenderer->BeginCamera(*m_app->m_screenCamera);
			RenderPausedMenu();
			g_theRenderer->EndCamera(*m_app->m_screenCamera);
			DebugRenderScreen(*m_app->m_screenCamera);
			return;

	}
		
}


void Game::EndFrame()
{
	
}

void Game::Shutdown()
{

}


void Game::UpdateWorldCamera(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_player->m_camera->SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, 0.1f, 100.0f);
	m_player->m_camera->SetRenderBasis(Vec3(0.f,0.f,1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
}

void Game::UpdateScreenCamera(float deltaSeconds)
{
	UNUSED (deltaSeconds);

}

void Game::RendererGameTitle() const
{
	std::vector<Vertex_PCU> titleTextVerts;

	AddVertsForTextTriangles2D(titleTextVerts, "StarShip", Vec2(-5.f, -0.5f), 1.f, Rgba8(32, 142, 178));
	TransformVertexArrayXY3D(static_cast<int>(titleTextVerts.size()), titleTextVerts.data(), 100.f, 0.f, Vec2(1000.f, 640.f));
	g_theRenderer->DrawVertexArray(static_cast<int>(titleTextVerts.size()), &titleTextVerts[0]);

	unsigned char changeValue = static_cast<unsigned char>(50.f * sin(2.f * GetCurrentTimeSeconds()));
	Rgba8 goldLogoColor(200 + changeValue, 200 + changeValue, 0, 150 + changeValue);
	float changeValuef = static_cast <float> (50.f * sin(2.f * GetCurrentTimeSeconds()));

	std::vector<Vertex_PCU> goldTextVerts;
	AddVertsForTextTriangles2D(goldTextVerts, "Gold", Vec2(-5.f, -0.5f), 1.f, goldLogoColor);
	TransformVertexArrayXY3D(static_cast<int>(goldTextVerts.size()), goldTextVerts.data(), 65.f + changeValuef * 0.1f, 25.f + -changeValuef * 0.02f, Vec2(1260.f, 690.f));
	g_theRenderer->DrawVertexArray(static_cast<int>(goldTextVerts.size()), &goldTextVerts[0]);
}


void Game::DrawSingleDotAt(Vec2 position, float scale, Rgba8 color) const
{
	Vertex_PCU dot[6] = {};

	dot[0] = Vertex_PCU(position + Vec2(-1.f * scale, -1.f * scale), color, Vec2(0.f, 0.f));
	dot[1] = Vertex_PCU(position + Vec2(-1.f * scale, 1.f * scale), color, Vec2(0.f, 0.f));
	dot[2] = Vertex_PCU(position + Vec2(1.f * scale, -1.f * scale), color, Vec2(0.f, 0.f));

	dot[3] = Vertex_PCU(position + Vec2(1.f * scale, 1.f * scale), color, Vec2(0.f, 0.f));
	dot[4] = Vertex_PCU(position + Vec2(-1.f * scale, 1.f * scale), color, Vec2(0.f, 0.f));
	dot[5] = Vertex_PCU(position + Vec2(1.f * scale, -1.f * scale), color, Vec2(0.f, 0.f));

	g_theRenderer->DrawVertexArray(6, dot);
}


void Game::LoadingDefinitions()
{
	//unit 
	XmlDocument unitDocument;
	unitDocument.LoadFile("Data/Definitions/UnitDefinitions.xml");
	XmlElement* unitRootElement = unitDocument.RootElement();
	XmlElement* unitDefElement = unitRootElement->FirstChildElement();
	while (unitDefElement != nullptr)
	{
		UnitDefinition* unitDef = new UnitDefinition();
		unitDef->LoadFromXmlElement(*unitDefElement);
		UnitDefinition::s_UnitDefinitions.push_back(unitDef);
		unitDefElement = unitDefElement->NextSiblingElement();
	}
	//tiles
	XmlDocument tileDocument;
	tileDocument.LoadFile("Data/Definitions/TileDefinitions.xml");
	XmlElement* tileRootElement = tileDocument.RootElement();
	XmlElement* tileDefElement = tileRootElement->FirstChildElement();
	while (tileDefElement != nullptr)
	{
		TileDefinition* tileDef = new TileDefinition();
		tileDef->LoadFromXmlElement(*tileDefElement);
		TileDefinition::s_TileDefinitions.push_back(tileDef);
		tileDefElement = tileDefElement->NextSiblingElement();
	}
	// maps
	XmlDocument mapDocument;
	mapDocument.LoadFile("Data/Definitions/MapDefinitions.xml");
	XmlElement* mapRootElement = mapDocument.RootElement();
	XmlElement* mapDefElement = mapRootElement->FirstChildElement("MapDefinition");
	while (mapDefElement != nullptr)
	{
		MapDefinition* mapDef = new MapDefinition();
		mapDef->LoadFromXmlElement(*mapDefElement);
		MapDefinition::s_mapDefinitions.push_back(mapDef);
		mapDefElement = mapDefElement->NextSiblingElement("MapDefinition");
	}

	return;
}



void Game::CreatingMap()
{
	m_curentMap = new Map(this, m_defualtMapName);
}

void Game::GetCurrentSelectedTile()
{
	if (m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER || m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
	{
		Vec3 cursorPosWorld = m_player->m_cursorPositionInWorld;
		if (CursorOutOfWorldBounds(cursorPosWorld))
		{
			m_curentCursorOverlap = nullptr;
			return;
		}
		for (int i = 0; i < m_curentMap->m_tiles.size(); i++)
		{
			if (m_curentMap->m_tiles[i]->IsPointInHexagon(cursorPosWorld))
			{
				if (CursorOutOfWorldBounds(m_curentMap->m_tiles[i]->GetCenterPos()))
				{
					m_curentCursorOverlap = nullptr;
					return;
				}
				m_curentCursorOverlap = m_curentMap->m_tiles[i];
				std::string cmd = Stringf("RemoteCommand Command=\"SetTarget TileIndex=%d\"", i);
				s_theApp->SendRemoteCommond(cmd);
				return;
			}
		}
		m_curentCursorOverlap = nullptr;
	}
	
	return;
}

bool Game::CursorOutOfWorldBounds(Vec3 const& cursorWorldPos)
{
	if (cursorWorldPos.x < m_player->m_cameraSettings.worldBoundsMin.x 
		|| cursorWorldPos.x > m_player->m_cameraSettings.worldBoundsMax.x
		|| cursorWorldPos.y < m_player->m_cameraSettings.worldBoundsMin.y
		|| cursorWorldPos.y > m_player->m_cameraSettings.worldBoundsMax.y
		|| cursorWorldPos.z < m_player->m_cameraSettings.worldBoundsMin.z
		|| cursorWorldPos.z > m_player->m_cameraSettings.worldBoundsMax.z
		)
	{
		return true;
	}
	return false;
}

void Game::RenderMainMenu() const
{
	Vec2 screenCenter = g_theWindow->m_fullScreenDimensions * 0.5f;
	Texture* LogoTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Logo.png");
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	g_theRenderer->SetModelConstants();

	std::vector<Vertex_PCU> logoVerts;
	g_theRenderer->BindTexture(LogoTexture);
	AABB2 logoBox = AABB2(Vec2::ZERO, Vec2(g_theWindow->m_fullScreenDimensions.y, g_theWindow->m_fullScreenDimensions.y) * 0.6f);
	logoBox.SetCenter(screenCenter - Vec2(0.f, g_theWindow->m_fullScreenDimensions.y * 0.1f));
	AddVertsForAABB2(logoVerts, logoBox, Rgba8::WHITE);
	g_theRenderer->DrawVertexArray((int)logoVerts.size(), logoVerts.data());

	//----------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> whiteLineVerts;
	AABB2 whiteLine = AABB2(Vec2::ZERO, Vec2(g_theWindow->m_fullScreenDimensions.y * 0.01f, g_theWindow->m_fullScreenDimensions.y));
	whiteLine.SetCenter(screenCenter - Vec2(g_theWindow->m_fullScreenDimensions.x * 0.3f, 0.f));
	AddVertsForAABB2(whiteLineVerts, whiteLine, Rgba8::WHITE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)whiteLineVerts.size(), whiteLineVerts.data());

	std::vector<Vertex_PCU> pageName;
	AABB2 pagebox = AABB2(Vec2::ZERO, Vec2(g_theWindow->m_fullScreenDimensions.y * 0.4f, g_theWindow->m_fullScreenDimensions.y * 0.4f));
	g_gameFont->AddVertsForTextInBox2D(pageName, pagebox, 530.f, "Main Menu", Rgba8::WHITE, 1.f);
	TransformVertexArrayXY3D((int)pageName.size(), pageName.data(), 1.f, 90.f, Vec2(g_theWindow->m_fullScreenDimensions.x * 0.3f, g_theWindow->m_fullScreenDimensions.y * 0.3f));
	g_theRenderer->BindTexture(&g_gameFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)pageName.size(), pageName.data());

	m_startButt->Render();
	m_quitButt->Render();
}

void Game::RenderPausedMenu() const
{
	Vec2 screenCenter = g_theWindow->m_fullScreenDimensions * 0.5f;
	Texture* LogoTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Logo.png");
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	g_theRenderer->SetModelConstants();

	std::vector<Vertex_PCU> logoVerts;
	g_theRenderer->BindTexture(LogoTexture);
	AABB2 logoBox = AABB2(Vec2::ZERO, Vec2(g_theWindow->m_fullScreenDimensions.y, g_theWindow->m_fullScreenDimensions.y) * 0.6f);
	logoBox.SetCenter(screenCenter - Vec2(0.f, g_theWindow->m_fullScreenDimensions.y * 0.1f));
	AddVertsForAABB2(logoVerts, logoBox, Rgba8::WHITE);
	g_theRenderer->DrawVertexArray((int)logoVerts.size(), logoVerts.data());
	//----------------------------------------------------------------------------------------

	std::vector<Vertex_PCU> whiteLineVerts;
	AABB2 whiteLine = AABB2(Vec2::ZERO, Vec2(g_theWindow->m_fullScreenDimensions.y * 0.01f, g_theWindow->m_fullScreenDimensions.y));
	whiteLine.SetCenter(screenCenter - Vec2(g_theWindow->m_fullScreenDimensions.x * 0.3f, 0.f));
	AddVertsForAABB2(whiteLineVerts, whiteLine, Rgba8::WHITE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)whiteLineVerts.size(), whiteLineVerts.data());

	std::vector<Vertex_PCU> pageName;
	AABB2 pagebox = AABB2(Vec2::ZERO, Vec2(g_theWindow->m_fullScreenDimensions.y * 0.4f, g_theWindow->m_fullScreenDimensions.y * 0.4f));
	g_gameFont->AddVertsForTextInBox2D(pageName, pagebox, 530.f, "Pause Menu", Rgba8::WHITE, 1.f);
	TransformVertexArrayXY3D((int)pageName.size(), pageName.data(), 1.f, 90.f, Vec2(g_theWindow->m_fullScreenDimensions.x * 0.3f, g_theWindow->m_fullScreenDimensions.y * 0.3f));
	g_theRenderer->BindTexture(&g_gameFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)pageName.size(), pageName.data());

	m_resumeButton->Render();
	m_menuButton->Render();
}

void Game::ResumeGame()
{
	m_currenMenuState = MenuState::InGame;
}

void Game::LoadingModels()
{
	for (int i = 0; i < UnitDefinition::s_UnitDefinitions.size(); ++i)
	{
		std::string filePath = UnitDefinition::s_UnitDefinitions[i]->m_modelFileName;
		m_OBJLoader->LoadOrGetModelFromPath(filePath.c_str());
	}
}

void Game::RenderInGameUI() const
{
	m_selectButton->Render();
	m_previousButton->Render();
	m_nextButton->Render();
	m_cancelButton->Render();
	m_endButton->Render();

	m_InfoHealth->m_text = m_healthInfo;
	m_InfoName->m_text = m_nameInfo;
	m_InfoAttack->m_text = m_attackInfo;
	m_InfoDefense->m_text = m_DefenseInfo;
	m_InfoRange->m_text = m_rangeInfo;
	m_InfoMove->m_text = m_moveInfo;

	m_InfoHealth->Render();
	m_InfoName->Render();
	m_InfoAttack->Render();
	m_InfoDefense->Render();
	m_InfoRange->Render();
	m_InfoMove->Render();
}

void Game::InitInGameButtons()
{
	Vec2 dimensions = g_theWindow->m_fullScreenDimensions;
	float halfWidth = (dimensions.x / 6.f) * 0.5f;
	float halfHeight = (dimensions.y * 0.03f) * 0.5f;
	float height = (dimensions.y * 0.03f);
	m_selectButton = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_selectButton->SetPosition(Vec2(halfWidth, halfHeight));
	m_selectButton->m_showOutline = true;
	m_selectButton->m_text = "[Mouse] Select";
	m_selectButton->m_overideFontSize = 12.f;

	m_previousButton = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_previousButton->SetPosition(Vec2(halfWidth + halfWidth * 2.f, halfHeight));
	m_previousButton->m_showOutline = true;
	m_previousButton->m_text = "[Left] Previous Unit";
	m_previousButton->m_overideFontSize = 12.f;

	m_nextButton = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_nextButton->SetPosition(Vec2(halfWidth + halfWidth * 4.f, halfHeight));
	m_nextButton->m_showOutline = true;
	m_nextButton->m_text = "[Right] Next Unit";
	m_nextButton->m_overideFontSize = 12.f;

	m_cancelButton = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_cancelButton->SetPosition(Vec2(halfWidth + halfWidth * 6.f, halfHeight));
	m_cancelButton->m_showOutline = true;
	m_cancelButton->m_text = "[Escape] Cancel";
	m_cancelButton->m_overideFontSize = 12.f;

	m_endButton = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_endButton->SetPosition(Vec2(halfWidth + halfWidth * 8.f, halfHeight));
	m_endButton->m_showOutline = true;
	m_endButton->m_text = "[Enter] End Turn";
	m_endButton->m_overideFontSize = 12.f;

//--------------------------------------------------------------------------------

	m_InfoHealth = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_InfoHealth->SetPosition(Vec2(halfWidth + halfWidth * 10.f, halfHeight));
	m_InfoHealth->m_showOutline = true;
	m_InfoHealth->m_overideFontSize = 12.f;


	m_InfoMove = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_InfoMove->SetPosition(Vec2(halfWidth + halfWidth * 10.f, halfHeight + 1.f * height));
	m_InfoMove->m_showOutline = true;
	m_InfoMove->m_overideFontSize = 12.f;

	m_InfoRange = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_InfoRange->SetPosition(Vec2(halfWidth + halfWidth * 10.f, halfHeight + 2.f * height));
	m_InfoRange->m_showOutline = true;
	m_InfoRange->m_overideFontSize = 12.f;

	m_InfoDefense = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_InfoDefense->SetPosition(Vec2(halfWidth + halfWidth * 10.f, halfHeight + 3.f * height));
	m_InfoDefense->m_showOutline = true;
	m_InfoDefense->m_overideFontSize = 12.f;

	m_InfoAttack = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_InfoAttack->SetPosition(Vec2(halfWidth + halfWidth * 10.f, halfHeight + 4.f * height));
	m_InfoAttack->m_showOutline = true;
	m_InfoAttack->m_overideFontSize = 12.f;

	m_InfoName = new Button(dimensions.x / 6.f, dimensions.y * 0.03f, true);
	m_InfoName->SetPosition(Vec2(halfWidth + halfWidth * 10.f, halfHeight + 5.f * height));
	m_InfoName->m_showOutline = true;
	m_InfoName->m_textAlignment = Vec2(0.5f, 0.5f);
	m_InfoName->m_overideFontSize = 12.f;
}

void Game::UpdateInfo()
{
	if (m_curentCursorOverlap && m_curentCursorOverlap->m_unit)
	{
		for (int i = 0; i < m_curentMap->m_P1Units.size(); i++)
		{
			Unit*& unit = m_curentMap->m_P1Units[i];
			if (unit)
			{
				if (unit->m_currentTile == m_curentCursorOverlap)
				{
					m_nameInfo = unit->m_def->m_name;
					m_attackInfo = Stringf("Attack %d", unit->m_def->m_groundAttackDamage);
					m_DefenseInfo = Stringf("Defense %d", unit->m_def->m_defense);
					m_rangeInfo = Stringf("Range %d - %d", unit->m_def->m_groundAttackRangeMin, unit->m_def->m_groundAttackRangeMax);
					m_moveInfo = Stringf("Move %d", unit->m_def->m_movementRange);
					m_healthInfo = Stringf("Health %d", unit->m_currentHP);
					return;
				}
			}
		}
		for (int i = 0; i < m_curentMap->m_P2Units.size(); i++)
		{
			Unit*& unit = m_curentMap->m_P2Units[i];
			if (unit)
			{
				if (unit->m_currentTile == m_curentCursorOverlap)
				{
					m_nameInfo = unit->m_def->m_name;
					m_attackInfo = Stringf("Attack %d", unit->m_def->m_groundAttackDamage);
					m_DefenseInfo = Stringf("Defense %d", unit->m_def->m_defense);
					m_rangeInfo = Stringf("Range %d - %d", unit->m_def->m_groundAttackRangeMin, unit->m_def->m_groundAttackRangeMax);
					m_moveInfo = Stringf("Move %d", unit->m_def->m_movementRange);
					m_healthInfo = Stringf("Health %d", unit->m_currentHP);
					return;
				}
			}
		}
	}
	else
	{
		m_nameInfo = "";
		m_attackInfo = Stringf("Attack");
		m_DefenseInfo = Stringf("Defense ");
		m_rangeInfo = Stringf("Range ");
		m_moveInfo = Stringf("Move ");
		m_healthInfo = Stringf("Health");
	}

	if (m_curentMap->m_currentSelectUnit)
	{
		switch (m_curentMap->m_currentSelectUnit->m_currentState)
		{
		case UnitState::move:
			m_selectButton->m_text = "[Mouse] Move";
			break;
		case UnitState::confirmMove:
			m_selectButton->m_text = "[Mouse] Confirm Move";
			break;
		case UnitState::attack:
			m_selectButton->m_text = "[Mouse] Attack";
			break;
		case UnitState::confirmAttack:
			m_selectButton->m_text = "[Mouse] Confirm Attack";
			break;
		default:
			break;
		}
	}
	else
	{
		m_selectButton->m_text = "[Mouse] Select";
	}
}

void Game::OnPreviousUnit()
{
	if (m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER || m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
	{
		if (m_currentSelect)
		{
			//s_theApp->SendRemoteCommond("RemoteCommand Command=\"SelectPrevious\"");
			if (m_currentSelect->m_currentState != UnitState::moved)
			{
				m_currentSelect->m_currentState = UnitState::Unmoved;
			}
			if (m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER)
			{
				//Previous Select

				for (int i = 0; i < m_curentMap->m_P1Units.size(); i++)
				{
					if (m_curentMap->m_P1Units[i] == m_currentSelect)
					{
						if (i == 0)
						{
							m_currentSelect = m_curentMap->m_P1Units[m_curentMap->m_P1Units.size() - 1];
						}
						else
						{
							m_currentSelect = m_curentMap->m_P1Units[i - 1];
						}
						m_currentSelect->m_currentState = UnitState::move;
						return;
					}
				}
			}
			else if (m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
			{
				//Previous Select
				for (int i = 0; i < m_curentMap->m_P2Units.size(); i++)
				{
					if (m_curentMap->m_P2Units[i] == m_currentSelect)
					{
						if (i == 0)
						{
							m_currentSelect = m_curentMap->m_P2Units[m_curentMap->m_P1Units.size() - 1];
						}
						else
						{
							m_currentSelect = m_curentMap->m_P2Units[i - 1];
						}
						m_currentSelect->m_currentState = UnitState::move;
						return;
					}
				}
			}

		}
	}
	
}

void Game::OnNextUnit()
{
	if (m_currentSelect)
	{
		if (m_currentSelect->m_currentState != UnitState::moved)
		{
			m_currentSelect->m_currentState = UnitState::Unmoved;
		}
		if (m_currentTurn == GameTurn::P1)
		{
			for (int i = 0; i < m_curentMap->m_P1Units.size(); i++)
			{
				if (m_curentMap->m_P1Units[i] == m_currentSelect)
				{
					if (i == m_curentMap->m_P1Units.size() - 1)
					{
						m_currentSelect = m_curentMap->m_P1Units[0];
					}
					else
					{
						m_currentSelect = m_curentMap->m_P1Units[i + 1];
					}
					m_currentSelect->m_currentState = UnitState::move;
					return;
				}
			}
		}
		else
		{
			for (int i = 0; i < m_curentMap->m_P2Units.size(); i++)
			{
				if (m_curentMap->m_P2Units[i] == m_currentSelect)
				{
					if (i == m_curentMap->m_P1Units.size() - 1)
					{
						m_currentSelect = m_curentMap->m_P2Units[0];
					}
					else
					{
						m_currentSelect = m_curentMap->m_P2Units[i + 1];
					}
					m_currentSelect->m_currentState = UnitState::move;
					return;
				}
			}
		}

	}
}

void Game::OnEndTurn()
{
	if (m_endButton->m_text == "[Enter] End Turn")
	{
		m_endButton->m_text = "[Enter] Confirm End Turn";
	}
	else if (m_endButton->m_text == "[Enter] Confirm End Turn")
	{
		if (m_currentTurn == GameTurn::P1)
		{
			ChangeTurnInto(GameTurn::P2);
		}
		else
		{
			ChangeTurnInto(GameTurn::P1);
		}
		for (int i = 0; i < m_curentMap->m_P1Units.size(); i++)
		{
			Unit*& unit = m_curentMap->m_P1Units[i];
			if (unit)
			{
				unit->m_currentState = UnitState::Unmoved;
				unit->m_targets.clear();
			}
		}

		for (int i = 0; i < m_curentMap->m_P2Units.size(); i++)
		{
			Unit*& unit = m_curentMap->m_P2Units[i];
			if (unit)
			{
				unit->m_currentState = UnitState::Unmoved;
				unit->m_targets.clear();
			}
		}
	}
}

void Game::OnCancel()
{
	if (m_currentSelect)
	{
		if (m_currentSelect->m_currentState == UnitState::confirmMove)
		{
			m_currentSelect->m_currentState = UnitState::moved;
		}
		else if (m_currentSelect->m_currentState == UnitState::attack || m_currentSelect->m_currentState == UnitState::confirmAttack)
		{
			m_currentSelect->m_currentState = UnitState::moved;
		}
		else
		{
			m_currentSelect->m_currentState = UnitState::Unmoved;
		}
	}
	m_curentMap->m_startMovingTile = nullptr;
	m_curentMap->m_currentSelectUnit = nullptr;
	m_currentSelect = nullptr;
}

void Game::ChangeTurnInto(GameTurn next)
{
	m_endButton->m_text = "[Enter] End Turn";
	m_showTurnSwitchWarning = true;
	m_currentTurn = next;
}

void Game::RenderSwitchTurnWarning() const
{
	
	std::vector<Vertex_PCU> boxVerts;
	std::vector<Vertex_PCU> textVerts;
	Vec2 fullDimension = g_theWindow->m_fullScreenDimensions;
	Vec2 midScreen = fullDimension * 0.5f;
	AABB2 boxOut = AABB2(Vec2::ZERO, fullDimension * 0.3f);
	boxOut.SetCenter(midScreen);
	AddVertsForAABB2(boxVerts, boxOut, Rgba8::WHITE);
	AABB2 box;
	box.SetDimensions(Vec2(boxOut.GetDimensions().x - 2.f, boxOut.GetDimensions().y - 2.f));
	box.SetCenter(boxOut.GetCenter());
	AddVertsForAABB2(boxVerts, box, Rgba8::BLACK);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)boxVerts.size(), boxVerts.data());

	AABB2 bottTxt = AABB2(Vec2(boxOut.m_mins.x, boxOut.m_mins.y + boxOut.GetDimensions().y * 0.3f ), Vec2(boxOut.m_maxs.x, boxOut.m_mins.y + boxOut.GetDimensions().y * 0.4f));
	AABB2 TopTxt = AABB2(Vec2(boxOut.m_mins.x, boxOut.m_mins.y + boxOut.GetDimensions().y * 0.7f), Vec2(boxOut.m_maxs.x, boxOut.m_mins.y + boxOut.GetDimensions().y * 0.9f));
	std::string text;
	if (m_currentTurn == GameTurn::P1)
	{
		text = "Player 1's Turn";
	}
	else
	{
		text = "Player 2's Turn";
	}
	g_gameFont->AddVertsForTextInBox2D(textVerts, TopTxt, 50.f, text, Rgba8::WHITE);
	g_gameFont->AddVertsForTextInBox2D(textVerts, bottTxt, 20.f, "Press Enter or Click to continue", Rgba8::WHITE, 1.2f);

	g_theRenderer->BindTexture(&g_gameFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
}

void Game::UpdateButtonClicks()
{	
	m_currentSelect = m_curentMap->m_currentSelectUnit;
	//by keyBoard;
	if (g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
	{
		OnEndTurn();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW))
	{
		OnPreviousUnit();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		if (m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER || m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
		{
			OnNextUnit();
			s_theApp->SendRemoteCommond("RemoteCommand Command=\"SelectNext\"");
		}
	}
	if (g_theInput->WasKeyJustPressed(ESC_KEY))
	{
		if (m_curentMap->m_currentSelectUnit)
		{
			if (m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER || m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
			{
				OnCancel();
				s_theApp->SendRemoteCommond("RemoteCommand Command=\"Cancel\"");
			}
		}
		else
		{
			m_currenMenuState = MenuState::Paused;
		}
	}
	//By mouse
	m_endButton->Update();
	m_previousButton->Update();
	m_nextButton->Update();
	m_cancelButton->Update();
	if (m_endButton->m_isCursonInBox)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
		{
			if (m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER || m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
			{
				OnEndTurn();
				s_theApp->SendRemoteCommond("RemoteCommand Command=\"EndTurn\"");
			}
		}
	}
	if (m_previousButton->m_isCursonInBox)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
		{
			OnPreviousUnit();
		}
	}
	if (m_nextButton->m_isCursonInBox)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
		{
			if (m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER || m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
			{
				OnNextUnit();
				s_theApp->SendRemoteCommond("RemoteCommand Command=\"SelectNext\"");
			}
		}
	}

	if (m_cancelButton->m_isCursonInBox)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
		{
			if (m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER || m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
			{
				OnCancel();
				s_theApp->SendRemoteCommond("RemoteCommand Command=\"Cancel\"");
			}
		}
	}
}

void Game::UpdateMouseClickOnUnit()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
	{
		if (m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER)
		{
			if (m_curentCursorOverlap)
			{
				if (m_curentCursorOverlap->m_unit)
				{
					if (m_curentCursorOverlap->m_unit->m_Tint == Rgba8::BLUE)
					{
						if (m_curentCursorOverlap->m_unit->m_currentState == UnitState::Unmoved)
						{
							//Select
							if (m_curentMap->m_currentSelectUnit)
							{
								m_curentMap->m_currentSelectUnit->m_currentState = UnitState::Unmoved;
							}
							m_curentCursorOverlap->m_unit->m_currentState = UnitState::move;
							s_theApp->SendRemoteCommond("RemoteCommand Command=\"Select\"");
						}

					}
				}
			}
		}
		else if (m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
		{
			if (m_curentCursorOverlap)
			{
				if (m_curentCursorOverlap->m_unit)
				{
					if (m_curentCursorOverlap->m_unit->m_Tint == Rgba8::RED)
					{
						if (m_curentCursorOverlap->m_unit->m_currentState == UnitState::Unmoved)
						{
							//Select
							if (m_curentMap->m_currentSelectUnit)
							{
								m_curentMap->m_currentSelectUnit->m_currentState = UnitState::Unmoved;
							}
							m_curentCursorOverlap->m_unit->m_currentState = UnitState::move;
							s_theApp->SendRemoteCommond("RemoteCommand Command=\"Select\"");
						}
					}
				}
			}
		}
	}
}

void Game::CheckWinner()
{
	bool p1Won = true;
	bool p2Won = true;
	for (int i = 0; i < m_curentMap->m_P1Units.size(); i++)
	{
		Unit*& unit = m_curentMap->m_P1Units[i];
		if (unit)
		{
			p2Won = false;
		}
	}

	for (int i = 0; i < m_curentMap->m_P2Units.size(); i++)
	{
		Unit*& unit = m_curentMap->m_P2Units[i];
		if (unit)
		{
			p1Won = false;
		}
	}
	if (p1Won)
	{
		m_winner = GameTurn::P1;
	}
	else if (p2Won)
	{
		m_winner = GameTurn::P2;
	}
}

void Game::RenderWinLog(GameTurn win) const
{
	UNUSED(win);
	std::vector<Vertex_PCU> boxVerts;
	std::vector<Vertex_PCU> textVerts;
	Vec2 fullDimension = g_theWindow->m_fullScreenDimensions;
	Vec2 midScreen = fullDimension * 0.5f;
	AABB2 boxOut = AABB2(Vec2::ZERO, fullDimension * 0.3f);
	boxOut.SetCenter(midScreen);
	AddVertsForAABB2(boxVerts, boxOut, Rgba8::WHITE);
	AABB2 box;
	box.SetDimensions(Vec2(boxOut.GetDimensions().x - 2.f, boxOut.GetDimensions().y - 2.f));
	box.SetCenter(boxOut.GetCenter());
	AddVertsForAABB2(boxVerts, box, Rgba8::BLACK);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)boxVerts.size(), boxVerts.data());

	AABB2 bottTxt = AABB2(Vec2(boxOut.m_mins.x, boxOut.m_mins.y + boxOut.GetDimensions().y * 0.3f), Vec2(boxOut.m_maxs.x, boxOut.m_mins.y + boxOut.GetDimensions().y * 0.4f));
	AABB2 TopTxt = AABB2(Vec2(boxOut.m_mins.x, boxOut.m_mins.y + boxOut.GetDimensions().y * 0.7f), Vec2(boxOut.m_maxs.x, boxOut.m_mins.y + boxOut.GetDimensions().y * 0.9f));
	std::string text;
	if (m_winner == GameTurn::P1)
	{
		text = "Player 1 wins";
	}
	else
	{
		text = "Player 2 wins";
	}
	g_gameFont->AddVertsForTextInBox2D(textVerts, TopTxt, 50.f, text, Rgba8::WHITE);
	g_gameFont->AddVertsForTextInBox2D(textVerts, bottTxt, 20.f, "Press Enter or Click to continue", Rgba8::WHITE, 1.2f);

	g_theRenderer->BindTexture(&g_gameFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
}

void Game::RenderGroundTexture() const
{
	std::vector<Vertex_PCU> quadVerts;
	AddVertsForQuad3D(quadVerts,Vec3(-40.f,0.f,-0.5f), Vec3(40.f, 0.f, -0.5f), Vec3(40.f, 40.f, -0.5f), Vec3(-40.f, 40.f, -0.5f));
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MoonSurface.png"));
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->DrawVertexArray((int)quadVerts.size(), quadVerts.data());
}

void Game::RenderWaitingScreen() const
{
	std::vector<Vertex_PCU> txt;
	g_gameFont->AddVertsForTextInBox2D(txt, AABB2(Vec2::ZERO, g_theWindow->m_fullScreenDimensions), 100.f, "Waiting for players...", Rgba8::WHITE, 1.0f);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&g_gameFont->GetTexture());
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->DrawVertexArray((int)txt.size(), txt.data());
}

void Game::ShowAttractMode() const
{
	Vec2 screenCenter = g_theWindow->m_fullScreenDimensions * 0.5f;
	Texture* LogoTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Logo.png");
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	g_theRenderer->SetModelConstants();

	std::vector<Vertex_PCU> logoVerts;
	g_theRenderer->BindTexture(LogoTexture);
	AABB2 logoBox = AABB2(Vec2::ZERO, Vec2(g_theWindow->m_fullScreenDimensions.y, g_theWindow->m_fullScreenDimensions.y) * 0.6f);
	logoBox.SetCenter(screenCenter - Vec2(0.f, g_theWindow->m_fullScreenDimensions.y * 0.1f));
	AddVertsForAABB2(logoVerts, logoBox, Rgba8::WHITE);
	g_theRenderer->DrawVertexArray((int)logoVerts.size(), logoVerts.data());
	//----------------------------------------------------------------------------------------

	std::vector<Vertex_PCU> txtVerts;
	AABB2 titleBox = AABB2(Vec2::ZERO, Vec2(g_theWindow->m_fullScreenDimensions.y * 0.9f, g_theWindow->m_fullScreenDimensions.y * 0.5f));
	titleBox.SetCenter(screenCenter + Vec2(0.f, g_theWindow->m_fullScreenDimensions.y * 0.3f));
	g_gameFont->AddVertsForTextInBox2D(txtVerts, titleBox, 530.f, "Vaporum", Rgba8::WHITE, 1.7f);
	g_theRenderer->BindTexture(&g_gameFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)txtVerts.size(), txtVerts.data());

	std::vector<Vertex_PCU> hintVerts;
	AABB2 hintBox = AABB2(Vec2::ZERO, Vec2(g_theWindow->m_fullScreenDimensions.y * 0.6f, g_theWindow->m_fullScreenDimensions.y * 0.5f));
	hintBox.SetCenter(screenCenter - Vec2(0.f, g_theWindow->m_fullScreenDimensions.y * 0.4f));
	g_gameFont->AddVertsForTextInBox2D(hintVerts, hintBox, 530.f, "Press ENTER or click anywhere to start", Rgba8::WHITE, 1.2f);
	g_theRenderer->BindTexture(&g_gameFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)hintVerts.size(), hintVerts.data());
}
















