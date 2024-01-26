#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include "Engine/Net/NetSystem.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/App.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Player.hpp"
#include "Unit.hpp"
#include "HexTile.hpp"

Renderer* g_theRenderer = nullptr;			
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
DevConsole* g_theDevConsole = nullptr;
NetSystem* g_theNet = nullptr;

constexpr float MAX_FRAME_SEC = 1.f / 10.f;

Camera* App::m_screenCamera = nullptr;

bool QuitAppCallbackFunction(EventArgs& args)
{
	UNUSED(args);
	s_theApp->SendRemoteCommond("RemoteCommand Command=\"PlayerQuit\"");
	m_isQuitting = true;
	return true;
}

bool PlayerQuit(EventArgs& args)
{
	UNUSED(args);
	if (s_theApp->m_theGame->m_isFindingPlayer)
	{
		if (s_theApp->m_theGame->m_currenMenuState == MenuState::Mainmenu)
		{
			FireEvent("quit");
		}
		else if (s_theApp->m_theGame->m_currenMenuState == MenuState::InGame)
		{
			s_theApp->m_theGame->m_currenMenuState = MenuState::Paused;
		}
	}
	return true;
}

bool GetReady(EventArgs& args)
{
	UNUSED(args);
	if (s_theApp->m_theGame->m_isFindingPlayer)
	{
		s_theApp->m_theGame->m_isOtherPlayerConnecting = true;
		s_theApp->SendRemoteCommond("RemoteCommand Command=\"Ready\"");
	}
	return true;
}
bool StartTurn(EventArgs& args)
{
	UNUSED(args);
	s_theApp->m_theGame->m_showTurnSwitchWarning = false;
	return true;
}

bool SetTarget(EventArgs& args)
{
	int i = -1;
	i = args.GetValue("TileIndex",i);
	s_theApp->m_theGame->m_curentCursorOverlap = s_theApp->m_theGame->GetCurrentMap()->m_tiles[i];
	return true;
}

bool Select(EventArgs& args)
{
	UNUSED(args);
	if (s_theApp->m_theGame->GetCurrentMap()->m_currentSelectUnit)
	{
		s_theApp->m_theGame->GetCurrentMap()->m_currentSelectUnit->m_currentState = UnitState::Unmoved;
	}
	s_theApp->m_theGame->m_curentCursorOverlap->m_unit->m_currentState = UnitState::move;
	return true;
}

bool SelectPrevious(EventArgs& args)
{
	UNUSED(args);
	if (s_theApp->m_theGame->m_currentSelect->m_currentState != UnitState::moved)
	{
		s_theApp->m_theGame->m_currentSelect->m_currentState = UnitState::Unmoved;
	}
	if (s_theApp->m_theGame->m_currentTurn == GameTurn::P1 && g_theNet->m_mode == NetSystem::Mode::SERVER)
	{

		for (int i = 0; i < s_theApp->m_theGame->GetCurrentMap()->m_P1Units.size(); i++)
		{
			if (s_theApp->m_theGame->GetCurrentMap()->m_P1Units[i] == s_theApp->m_theGame->m_currentSelect)
			{
				if (i == 0)
				{
					s_theApp->m_theGame->m_currentSelect = s_theApp->m_theGame->GetCurrentMap()->m_P1Units[s_theApp->m_theGame->GetCurrentMap()->m_P1Units.size() - 1];
				}
				else
				{
					s_theApp->m_theGame->m_currentSelect = s_theApp->m_theGame->GetCurrentMap()->m_P1Units[i - 1];
				}
				s_theApp->m_theGame->m_currentSelect->m_currentState = UnitState::move;
				return true;
			}
		}
	}
	else if (s_theApp->m_theGame->m_currentTurn == GameTurn::P2 && g_theNet->m_mode == NetSystem::Mode::CLIENT)
	{
		for (int i = 0; i < s_theApp->m_theGame->GetCurrentMap()->m_P2Units.size(); i++)
		{
			if (s_theApp->m_theGame->GetCurrentMap()->m_P2Units[i] == s_theApp->m_theGame->m_currentSelect)
			{
				if (i == 0)
				{
					s_theApp->m_theGame->m_currentSelect = s_theApp->m_theGame->GetCurrentMap()->m_P2Units[s_theApp->m_theGame->GetCurrentMap()->m_P1Units.size() - 1];
				}
				else
				{
					s_theApp->m_theGame->m_currentSelect = s_theApp->m_theGame->GetCurrentMap()->m_P2Units[i - 1];
				}
				s_theApp->m_theGame->m_currentSelect->m_currentState = UnitState::move;
				return true;
			}
		}
	}
	return true;
}

bool SelectNext(EventArgs& args)
{
	UNUSED(args);
	s_theApp->m_theGame->OnNextUnit();
	return true;
}

bool Move(EventArgs& args)
{
	UNUSED(args);
	for (int i = 0; i < s_theApp->m_theGame->GetCurrentMap()->m_P1Units.size(); ++i)
	{
		Unit*& unit = s_theApp->m_theGame->GetCurrentMap()->m_P1Units[i];
		if (unit)
		{
			if (unit->m_currentState == UnitState::move)
			{
				unit->MoveBehavior();
				return true;
			}
		}
	}

	for (int i = 0; i < s_theApp->m_theGame->GetCurrentMap()->m_P2Units.size(); ++i)
	{
		Unit*& unit = s_theApp->m_theGame->GetCurrentMap()->m_P2Units[i];
		if (unit)
		{
			if (unit->m_currentState == UnitState::move)
			{
				unit->MoveBehavior();
				return true;
			}
		}
	}
	return true;
}

bool ConfirmMove(EventArgs& args)
{
	UNUSED(args);
	for (int i = 0; i < s_theApp->m_theGame->GetCurrentMap()->m_P1Units.size(); ++i)
	{
		Unit*& unit = s_theApp->m_theGame->GetCurrentMap()->m_P1Units[i];
		if (unit)
		{
			if (unit->m_currentState == UnitState::confirmMove)
			{
				unit->ConfirmMove();
				return true;
			}
		}
	}

	for (int i = 0; i < s_theApp->m_theGame->GetCurrentMap()->m_P2Units.size(); ++i)
	{
		Unit*& unit = s_theApp->m_theGame->GetCurrentMap()->m_P2Units[i];
		if (unit)
		{
			if (unit->m_currentState == UnitState::confirmMove)
			{
				unit->ConfirmMove();
				return true;
			}
		}
	}
	return true;
}

bool NoAttack(EventArgs& args)
{
	UNUSED(args);

	return true;
}

bool Attack(EventArgs& args)
{
	UNUSED(args);
	for (int i = 0; i < s_theApp->m_theGame->GetCurrentMap()->m_P1Units.size(); ++i)
	{
		Unit*& unit = s_theApp->m_theGame->GetCurrentMap()->m_P1Units[i];
		if (unit)
		{
			if (unit->m_currentState == UnitState::attack)
			{
				unit->Attack();
				return true;
			}
		}
	}

	for (int i = 0; i < s_theApp->m_theGame->GetCurrentMap()->m_P2Units.size(); ++i)
	{
		Unit*& unit = s_theApp->m_theGame->GetCurrentMap()->m_P2Units[i];
		if (unit)
		{
			if (unit->m_currentState == UnitState::attack)
			{
				unit->Attack();
				return true;
			}
		}
	}
	return true;
}

bool ConfirmAttack(EventArgs& args)
{
	UNUSED(args);
	for (int i = 0; i < s_theApp->m_theGame->GetCurrentMap()->m_P1Units.size(); ++i)
	{
		Unit*& unit = s_theApp->m_theGame->GetCurrentMap()->m_P1Units[i];
		if (unit)
		{
			if (unit->m_currentState == UnitState::confirmAttack)
			{
				unit->ConfirmAttack();
				return true;
			}
		}
	}

	for (int i = 0; i < s_theApp->m_theGame->GetCurrentMap()->m_P2Units.size(); ++i)
	{
		Unit*& unit = s_theApp->m_theGame->GetCurrentMap()->m_P2Units[i];
		if (unit)
		{
			if (unit->m_currentState == UnitState::confirmAttack)
			{
				unit->ConfirmAttack();
				return true;
			}
		}
	}
	return true;
}

bool Cancel(EventArgs& args)
{
	UNUSED(args);
	s_theApp->m_theGame->OnCancel();
	return true;
}

bool EndTurn(EventArgs& args)
{
	UNUSED(args);
	s_theApp->m_theGame->OnEndTurn();
	return true;
}

bool ConfirmEndTurn(EventArgs& args)
{
	UNUSED(args);
	return true;
}

bool LoadAdditionalGameConfig(EventArgs& args)
{
	std::string path;
	path = args.GetValue("File", path);
	XmlDocument document;
	document.LoadFile(path.c_str());
	XmlElement* rootElement = document.RootElement();
	s_NetMode = ParseXMLAttribute(*rootElement, "netMode", s_NetMode);
	s_windowFullScreen = ParseXMLAttribute(*rootElement, "windowFullscreen", s_windowFullScreen);
	s_windowSize = ParseXMLAttribute(*rootElement, "windowSize", s_windowSize);
	s_windowPos = ParseXMLAttribute(*rootElement, "windowPosition", s_windowPos);
	s_windowTitle = ParseXMLAttribute(*rootElement, "windowTitle", s_windowTitle);
	return true;
}




App::App()
{
	BGM = 0;
	m_theGame = nullptr;
	s_theApp = this;
	m_screenCamera = new Camera();
}

App::~App()
{
	delete g_theDevConsole;
	delete m_theGame;
	delete g_theAudio;
	delete g_theRenderer;
	delete g_theWindow;
	delete g_theInput;
	delete m_screenCamera;

	g_theDevConsole = nullptr;
	m_theGame = nullptr;
	g_theAudio = nullptr;
	g_theWindow = nullptr;
	g_theRenderer = nullptr;
	g_theInput = nullptr;
	m_screenCamera = nullptr;
}

void App::Startup()
{
	XmlDocument document;
	document.LoadFile("Data/GameConfig.xml");
	XmlElement* rootElement = document.RootElement();
	float windowAspect = ParseXMLAttribute(*rootElement, "windowAspect", 0.0f);
	int netSendBufferSize = ParseXMLAttribute(*rootElement, "netSendBufferSize", 0);
	int netRecvBufferSize = ParseXMLAttribute(*rootElement, "netRecvBufferSize", 0);
	std::string defaultMap;
	defaultMap = ParseXMLAttribute(*rootElement, "defaultMap", defaultMap);
	std::string netHostAddress;
	netHostAddress = ParseXMLAttribute(*rootElement, "netHostAddress", netHostAddress);
	s_NetMode = ParseXMLAttribute(*rootElement, "netMode", s_NetMode);

	std::string windowsTitle = "Vaporum ";
	if (!m_cmdLine.empty())
	{
		windowsTitle.append(m_cmdLine);
	}



	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	
	windowConfig.m_windowTitle = windowsTitle;
	windowConfig.m_clientAspect = windowAspect;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_isFullscreen = true;
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_camera = m_screenCamera;
	devConsoleConfig.m_renderer = g_theRenderer;

	g_theDevConsole = new DevConsole(devConsoleConfig);
	SubscribeEventCallbackFunction("LoadGameConfig", LoadAdditionalGameConfig);
	g_theDevConsole->ExecuteWithoutRender(m_cmdLine);

	if (s_windowTitle != "")
	{
		windowsTitle = s_windowTitle;
	}
	g_theWindow->GetConfig().m_isFullscreen = s_windowFullScreen;
	g_theWindow->GetConfig().m_windowTitle = s_windowTitle;
	g_theWindow->GetConfig().m_Pos = s_windowPos;
	g_theWindow->GetConfig().m_Size = s_windowSize;
	g_theWindow->GetConfig().m_clientAspect = (float)s_windowSize.x / s_windowSize.y;

	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);

	m_theGame = new Game(this);
	m_theGame->m_defualtMapName = defaultMap;
	m_theGame->m_defaultModelPath = ParseXMLAttribute(*rootElement, "model", m_theGame->m_defaultModelPath);

	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theAudio->Startup();
	m_theGame->Startup();
	SubscribeEventCallbackFunction("quit", QuitAppCallbackFunction);
	SubscribeEventCallbackFunction("Ready", GetReady); 
	SubscribeEventCallbackFunction("StartTurn", StartTurn);
	SubscribeEventCallbackFunction("SetTarget", SetTarget);
	SubscribeEventCallbackFunction("PlayerQuit", PlayerQuit);
	SubscribeEventCallbackFunction("Select", Select);
	SubscribeEventCallbackFunction("SelectPrevious", SelectPrevious);
	SubscribeEventCallbackFunction("SelectNext", SelectNext);
	SubscribeEventCallbackFunction("Move", Move);
	SubscribeEventCallbackFunction("ConfirmMove", ConfirmMove);
	SubscribeEventCallbackFunction("NoAttack", NoAttack);
	SubscribeEventCallbackFunction("Attack", Attack);
	SubscribeEventCallbackFunction("ConfirmAttack", ConfirmAttack);
	SubscribeEventCallbackFunction("Cancel", Cancel);
	SubscribeEventCallbackFunction("EndTurn", EndTurn);
	SubscribeEventCallbackFunction("ConfirmEndTurn", ConfirmEndTurn);

	
	NetSystemConfig netConfig;
	Strings netInfo;
	netInfo = SplitStringOnDelimiter(netHostAddress, ':');
	netConfig.m_hostAddressString = netInfo[0];
	netConfig.m_hostPort = netInfo[1];
	netConfig.m_recvBufferSize = netRecvBufferSize;
	netConfig.m_sendBufferSize = netSendBufferSize;
	netConfig.m_modeString = s_NetMode;


	g_theNet = new NetSystem(netConfig);
	g_theNet->Startup();

	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Keys");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "WASD    - Moving Horizontally");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Up      - Camera Up");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Down    - Camera Down");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Left    - Camera Left");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Right   - Camera Right");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Q/E     - Rolling Left/Right");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Z/C     - Flying Vertically");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "H       - Set back to the origin");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Shift   - Speed Up(Hold)");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "L       - Open A Model");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "O       - Reset Player");


	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	debugRenderConfig.m_startHidden = false;
	DebugRenderSystemStartup(debugRenderConfig);
	FireEvent("debugrenderclear");
	LoadAssets();
}


void App::Run()
{
	while (!m_isQuitting)			
	{
		Sleep(0); 
        Runframe();
	}
}

void App::Shutdown()
{	
	DebugRenderSystemShutdown();
	g_theInput->Shutdown();
	g_theWindow->Shutdown();
	g_theRenderer->Shutdown();
	g_theNet->Shutdown();
	g_theAudio->Shutdown();
	m_theGame->Shutdown();
}

void App::Runframe()
{	
	float deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	BeginFrame();
	Update(deltaSeconds);
	Render();
	EndFrame();
}

bool App::HandleQuitRequested()
{
	return false;
}

bool App::OpenSlowMo()
{
	g_theAudio->SetSoundPlaybackSpeed(BGM, 0.1f);

	return false;
}

bool App::CloseSlowMo()
{
	//if (!m_isPaused)
	{
		g_theAudio->SetSoundPlaybackSpeed(BGM, 1.f);
	}

	return false;
}

bool App::SwitchPaused()
{
	//if (m_isPaused)
	{
		g_theAudio->SetSoundPlaybackSpeed(BGM, 1.f);
	}
	//else
	{
		g_theAudio->SetSoundPlaybackSpeed(BGM, 0.f);
	}


	return false;
}

void App::MoveOneStepThenPaused()
{
	//if (m_isPaused) 
	{
		m_theGame->Update(0.f);
		m_theGame->Render();

	}
	//else
	{
		//if (m_isPaused)
		{
			g_theAudio->SetSoundPlaybackSpeed(BGM, 1.f);
		}
		//else
		{
			g_theAudio->SetSoundPlaybackSpeed(BGM, 0.f);
		}
	}
}

bool const App::IsKeyDown(unsigned char keyCode)
{
	return g_theInput->IsKeyDown(keyCode);
}

bool const App::WasKeyJustPressed(unsigned char keyCode)
{
	return g_theInput->WasKeyJustPressed(keyCode);
}

void App::ShowAttractMode()
{
	m_theGame->ShowAttractMode();
}

bool App::GetIsAttractMo()
{
	return m_theGame->m_isAttractMode;
}


SoundPlaybackID App::GetBGMPlaybackID() const
{
	return BGM;
}


void App::GetCmdLine(LPSTR cmd)
{
	m_cmdLine = cmd;
}

void App::SendRemoteCommond(std::string cmd)
{
	g_theDevConsole->ExecuteWithoutRender(cmd);
}

void App::BeginFrame()
{
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theDevConsole->BeginFrame();
	DebugRenderBeginFrame();
	g_theNet->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();

	if (g_theInput->WasKeyJustPressed(ESC_KEY))
	{
		g_theAudio->StopSound(BGM);

		if (m_theGame->m_currenMenuState == MenuState::Splash || m_theGame->m_currenMenuState == MenuState::Mainmenu)
		{
			m_isQuitting = true;
		}
		else
		{
			
		}
	}

	if (g_theInput->IsKeyDown('T'))
	{
		this->OpenSlowMo();
	}
	else
	{
		this->CloseSlowMo();
	}


// 	if (g_theInput->WasKeyJustPressed('O'))
// 	{
// 		this->MoveOneStepThenPaused();
// 	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		this->SwitchPaused();
	}

	if (g_theInput->WasKeyJustPressed(F8_KEY))
	{
		delete m_theGame;
		m_theGame = nullptr;

		m_theGame = new Game(this);
		m_theGame->Startup();
	}

	if (g_theInput->WasKeyJustPressed(F1_KEY)) 
	{
		m_theGame->g_DebugMo = !m_theGame->g_DebugMo;
	}

	if (g_theInput->WasKeyJustPressed(187)) //+
	{
		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		g_theAudio->StartSound(testSound);
		
	}

	XboxController const& controller = g_theInput->GetController(0);
	if (m_theGame->m_isAttractMode && (g_theInput->IsKeyDown(SPACE_KEY) || g_theInput->IsKeyDown('N') || controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START)))
	{
		
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theDevConsole->ToggleOpen();
	}
}

void App::Update(float deltaSeconds)
{
	Clock::TickSystemClock();
	if (m_theGame->m_isAttractMode || g_theDevConsole->IsOpen() || !g_theWindow->IsFocusingWindow())
	{
		g_theInput->SetCursorMode(true, false);
	}
	else
	{
		g_theInput->SetCursorMode(true, false);
	}

	m_theGame->Update(deltaSeconds);
	
}

void App::Render() const
{
	m_theGame->Render();
}

void App::EndFrame()
{
	CopyIsDownToWasDown();
	g_theNet->EndFrame();
	m_theGame->EndFrame();
	g_theAudio->EndFrame();
	DebugRenderEndFrame();
	g_theRenderer->EndFrame();

}

void App::UpdateShip(float deltaSeconds)
{
	deltaSeconds;
}



void App::CopyIsDownToWasDown()
{
	g_theInput->EndFrame();
}

void App::LoadAssets()
{
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Logo.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MoonSurface.png");
}

