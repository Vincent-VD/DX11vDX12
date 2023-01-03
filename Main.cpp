//
// Main.cpp
//

#include "pch.h"
#include <iostream>
#include <cstdio>

#include "BaseGame.h"
#include "GameDX11.h"
#include "GameDX12.h"
#include "resource.h"

using namespace DirectX;

namespace Game
{
	std::unique_ptr<BaseGame> g_game;
	static RenderType g_RenderType{ RenderType::DirectX11 };

}

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

#pragma warning(disable : 4061)

LPCWSTR g_szAppName = L"DirectXProj_Win32";

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ExitGame() noexcept;
//void AddMenus(HWND);
void SwitchRenderMode();


#define IDM_FILE_NEW 1
#define IDM_FILE_OPEN 2
#define IDM_FILE_QUIT 3

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	//using namespace Game;
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	std::cout << "Hello World\n";

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (!XMVerifyCPUSupport())
		return 1;

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))
		return 1;

	//LoadResource(hInstance, MAKEINTRESOURCE(IDR_MENU1));

	Game::g_game = std::make_unique<GameDX11>();

	// Register class and create window
	{
		// Register class
		WNDCLASSEXW wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIconW(hInstance, L"IDI_ICON");
		wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wcex.lpszClassName = L"DirectXProj_Win32WindowClass";
		wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
		wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");
		if (!RegisterClassExW(&wcex))
			return 1;

		// Create window
		int w, h;
		Game::g_game->GetDefaultSize(w, h);

		RECT rc = { 0, 0, static_cast<LONG>(w), static_cast<LONG>(h) };

		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		HWND hwnd = CreateWindowExW(0, L"DirectXProj_Win32WindowClass", g_szAppName, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
			nullptr);
		// TODO: Change to CreateWindowExW(WS_EX_TOPMOST, L"DirectXProj_Win32WindowClass", g_szAppName, WS_POPUP,
		// to default to fullscreen.

		if (!hwnd)
			return 1;

		ShowWindow(hwnd, nCmdShow);
		// TODO: Change nCmdShow to SW_SHOWMAXIMIZED to default to fullscreen.

		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Game::g_game.get()));

		GetClientRect(hwnd, &rc);

		Game::g_game->Initialize(hwnd, rc.right - rc.left, rc.bottom - rc.top);

#define DX11
	}

	// Main message loop
	MSG msg = {};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Game::g_game->Tick();
		}
	}

	Game::g_game.reset();

	CoUninitialize();


	return static_cast<int>(msg.wParam);
}

// Windows procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool s_in_sizemove = false;
	static bool s_in_suspend = false;
	static bool s_minimized = false;
	static bool s_fullscreen = false;
	POINT initMousePos{};

	// TODO: Set s_fullscreen to true if defaulting to fullscreen.

	BaseGame* game{nullptr};

	switch(Game::g_RenderType)
	{
	case RenderType::DirectX11:
		game = reinterpret_cast<GameDX11*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		break;
	case RenderType::DirectX12:
		game = reinterpret_cast<GameDX12*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		break;
	case RenderType::Direct3d11on12:
		break;
	}

	WORD vkCode = LOWORD(wParam);

	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(hWnd, &mousePos);
	int x = mousePos.x;
	int y = mousePos.y;
	//ModelManager::GetInstance()->SetDrag(x, y);

	switch (message)
	{

	case WM_PAINT:
		if (s_in_sizemove && game)
		{
			game->Tick();
		}
		else
		{
			PAINTSTRUCT ps;
			std::ignore = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case ID_RENDERMODE_DIRECTX11:
			#undef DX12
			#define DX11
			std::cout << "TEST\n";
			CheckMenuItem(GetMenu(hWnd), ID_RENDERMODE_DIRECTX11, MF_CHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_RENDERMODE_DIRECTX12, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_RENDERMODE_DIRECT3D11ON12, MF_UNCHECKED);
			//g_game.get()->~BaseGame();
			Game::g_game = std::make_unique<GameDX11>();
			Game::g_game->Initialize(hWnd, 800, 600);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Game::g_game.get()));
			break;
		case ID_RENDERMODE_DIRECTX12:
			#undef DX11
			#define DX12
			std::cout << "TEST1\n";
			CheckMenuItem(GetMenu(hWnd), ID_RENDERMODE_DIRECTX11, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_RENDERMODE_DIRECTX12, MF_CHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_RENDERMODE_DIRECT3D11ON12, MF_UNCHECKED);
			Game::g_game = std::make_unique<GameDX12>();
			Game::g_game->Initialize(hWnd, 800, 600);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Game::g_game.get()));
			//  MessageBeep(MB_ICONINFORMATION);
			break;
		case ID_RENDERMODE_DIRECT3D11ON12:
			std::cout << "TEST2\n";
			CheckMenuItem(GetMenu(hWnd), ID_RENDERMODE_DIRECTX11, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_RENDERMODE_DIRECTX12, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_RENDERMODE_DIRECT3D11ON12, MF_CHECKED);
			//SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;

	/*case WM_LBUTTONDOWN:
		if(DragDetect(hWnd, initMousePos))
		{
			const int xDrag{ GetSystemMetrics(SM_CXDRAG) };
			const int yDrag{ GetSystemMetrics(SM_CYDRAG) };
			ModelManager::GetInstance()->SetDrag(xDrag, yDrag);
			std::cout << "AFGHDFSUIJ\n";
		}

	case WM_RBUTTONDOWN:
		if (DragDetect(hWnd, initMousePos))
		{
			const int xDrag{ GetSystemMetrics(SM_CXDRAG) };
			const int yDrag{ GetSystemMetrics(SM_CYDRAG) };
			ModelManager::GetInstance()->SetDrag(xDrag, yDrag);
		}*/

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
		{
			if (!s_minimized)
			{
				s_minimized = true;
				if (!s_in_suspend && game)
					game->OnSuspending();
				s_in_suspend = true;
			}
		}
		else if (s_minimized)
		{
			s_minimized = false;
			if (s_in_suspend && game)
				game->OnResuming();
			s_in_suspend = false;
		}
		else if (!s_in_sizemove && game)
		{
			game->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
		}
		break;

	case WM_ENTERSIZEMOVE:
		s_in_sizemove = true;
		break;

	case WM_EXITSIZEMOVE:
		s_in_sizemove = false;
		if (game)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);

			game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;

	case WM_GETMINMAXINFO:
		if (lParam)
		{
			auto info = reinterpret_cast<MINMAXINFO*>(lParam);
			info->ptMinTrackSize.x = 320;
			info->ptMinTrackSize.y = 200;
		}
		break;

	case WM_ACTIVATEAPP:
		if (game)
		{
			if (wParam)
			{
				game->OnActivated();
			}
			else
			{
				game->OnDeactivated();
			}
		}
		break;

	case WM_POWERBROADCAST:
		switch (wParam)
		{
		case PBT_APMQUERYSUSPEND:
			if (!s_in_suspend && game)
				game->OnSuspending();
			s_in_suspend = true;
			return TRUE;

		case PBT_APMRESUMESUSPEND:
			if (!s_minimized)
			{
				if (s_in_suspend && game)
					game->OnResuming();
				s_in_suspend = false;
			}
			return TRUE;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYUP:

		switch(vkCode)
		{
		case VK_F5:
			std::cout << "F5 pressed\n";
			//game->SwitchRenderMode();
			break;
		case VK_F6:
			std::cout << "F6 pressed\n";
			//game->SwitchRenderMode();
			break;
		}
		
		break;

	case WM_SYSKEYDOWN:
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
			// Implements the classic ALT+ENTER fullscreen toggle
			if (s_fullscreen)
			{
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

				int width = 800;
				int height = 600;
				if (game)
					game->GetDefaultSize(width, height);

				ShowWindow(hWnd, SW_SHOWNORMAL);

				SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			else
			{
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

				ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			}

			s_fullscreen = !s_fullscreen;
		}
		break;

	case WM_MENUCHAR:
		// A menu is active and the user presses a key that does not correspond
		// to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
		return MAKELRESULT(0, MNC_CLOSE);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// Exit helper
void ExitGame() noexcept
{
	PostQuitMessage(0);
}

//void AddMenus(HWND hWnd)
//{
//    HMENU hMenubar;
//    HMENU hMenu;
//
//    hMenubar = CreateMenu();
//    hMenu = CreateMenu();
//
//    AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(RenderType::DirectX11), L"&DirectX11");
//    AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(RenderType::DirectX12), L"&DirectX12");
//    AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(RenderType::Direct3d11on12), L"&Direct3D11On12");
//
//    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&Render Mode");
//    SetMenu(hWnd, hMenubar);
//}
//

void SwitchRenderMode()
{
	using namespace Game;
	switch (g_RenderType)
	{
	case RenderType::DirectX11:
		g_RenderType = RenderType::DirectX12;
		std::cout << "DirectX 12\n";
		break;
	case RenderType::DirectX12:
		g_RenderType = RenderType::Direct3d11on12;
		std::cout << "Direct3D 11 on 12\n";
		break;
	case RenderType::Direct3d11on12:
		g_RenderType = RenderType::DirectX11;
		std::cout << "DirectX 11\n";
		break;
	}

}
