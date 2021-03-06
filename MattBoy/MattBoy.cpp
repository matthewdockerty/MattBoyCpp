#ifdef DONOTUSE

#include <Windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <strstream>

#include <thread>

#include "Gameboy.h"
#include "Util.h"

#define IDM_FILE_LOAD_ROM 1
#define IDM_FILE_SAVE_STATE 2
#define IDM_FILE_LOAD_STATE 3
#define IDM_FILE_OPTIONS 4
#define IDM_FILE_QUIT 5

#define IDM_EMULATION_PAUSE 6
#define IDM_EMULATION_RESET 7
#define IDM_EMULATION_SPEED_1 8
#define IDM_EMULATION_SPEED_2 9
#define IDM_EMULATION_SPEED_5 10
#define IDM_EMULATION_SPEED_10 11
#define IDM_EMULATION_TYPE_DMG 12
#define IDM_EMULATION_TYPE_COLOR 13
#define IDM_EMULATION_TYPE_ADVANCE 14

#define IDM_VIEW_WINDOW_SIZE_100 15
#define IDM_VIEW_WINDOW_SIZE_200 16
#define IDM_VIEW_WINDOW_SIZE_300 17
#define IDM_VIEW_WINDOW_SIZE_400 18
#define IDM_VIEW_WINDOW_SIZE_500 19
#define IDM_VIEW_DEBUG 20


const LPCWSTR WINDOW_TITLE = L"MattBoy";
const LPCWSTR g_szClassName = L"mattboyWindowClass";
HMENU menu_bar;
HMENU file_menu;
HMENU emulation_menu, emulation_type_menu, emulation_speed_menu;
HMENU view_menu, view_window_size_menu;

bool quit;
int scale_factor = 2;

mattboy::Gameboy gameboy;

void AddMenus(HWND hwnd)
{
	menu_bar = CreateMenu();

	// File menu
	file_menu = CreateMenu();
	AppendMenu(file_menu, MF_STRING, IDM_FILE_LOAD_ROM, L"Load ROM");
	AppendMenu(file_menu, MF_SEPARATOR, 0, NULL);
	AppendMenu(file_menu, MF_STRING, IDM_FILE_SAVE_STATE, L"Save State");
	AppendMenu(file_menu, MF_STRING, IDM_FILE_LOAD_STATE, L"Load State");
	AppendMenu(file_menu, MF_SEPARATOR, 0, NULL);
	AppendMenu(file_menu, MF_STRING, IDM_FILE_OPTIONS, L"Options");
	AppendMenu(file_menu, MF_STRING, IDM_FILE_QUIT, L"Quit");
	AppendMenu(menu_bar, MF_POPUP, (UINT_PTR)file_menu, L"File");

	// Emulation menu
	emulation_menu = CreateMenu();
	emulation_type_menu = CreateMenu();
	emulation_speed_menu = CreateMenu();

	AppendMenu(emulation_menu, MF_STRING, IDM_EMULATION_PAUSE, L"Pause");
	CheckMenuItem(emulation_menu, IDM_EMULATION_PAUSE, MF_UNCHECKED);
	AppendMenu(emulation_menu, MF_STRING, IDM_EMULATION_RESET, L"Reset");

	AppendMenu(emulation_speed_menu, MF_STRING, IDM_EMULATION_SPEED_1, L"x1");
	AppendMenu(emulation_speed_menu, MF_STRING, IDM_EMULATION_SPEED_2, L"x2");
	AppendMenu(emulation_speed_menu, MF_STRING, IDM_EMULATION_SPEED_5, L"x5");
	AppendMenu(emulation_speed_menu, MF_STRING, IDM_EMULATION_SPEED_10, L"x10");
	CheckMenuRadioItem(emulation_speed_menu, IDM_EMULATION_SPEED_1, IDM_EMULATION_SPEED_10, IDM_EMULATION_SPEED_1, MF_BYCOMMAND);
	AppendMenu(emulation_menu, MF_POPUP | MF_STRING, (UINT_PTR)emulation_speed_menu, L"Speed");

	AppendMenu(emulation_menu, MF_SEPARATOR, 0, NULL);

	AppendMenu(emulation_type_menu, MF_STRING, IDM_EMULATION_TYPE_DMG, L"GameBoy (DMG)");
	AppendMenu(emulation_type_menu, MF_STRING, IDM_EMULATION_TYPE_COLOR, L"GameBoy Color");
	AppendMenu(emulation_type_menu, MF_STRING, IDM_EMULATION_TYPE_ADVANCE, L"GameBoy Advance");
	CheckMenuRadioItem(emulation_type_menu, IDM_EMULATION_TYPE_DMG, IDM_EMULATION_TYPE_ADVANCE, IDM_EMULATION_TYPE_DMG, MF_BYCOMMAND);
	AppendMenu(emulation_menu, MF_POPUP | MF_STRING, (UINT_PTR)emulation_type_menu, L"Console Type");
	AppendMenu(menu_bar, MF_POPUP, (UINT_PTR)emulation_menu, L"Emulation");

	// View menu
	view_menu = CreateMenu();
	view_window_size_menu = CreateMenu();
	AppendMenu(view_window_size_menu, MF_STRING, IDM_VIEW_WINDOW_SIZE_100, L"100%");
	AppendMenu(view_window_size_menu, MF_STRING, IDM_VIEW_WINDOW_SIZE_200, L"200%");
	AppendMenu(view_window_size_menu, MF_STRING, IDM_VIEW_WINDOW_SIZE_300, L"300%");
	AppendMenu(view_window_size_menu, MF_STRING, IDM_VIEW_WINDOW_SIZE_400, L"400%");
	AppendMenu(view_window_size_menu, MF_STRING, IDM_VIEW_WINDOW_SIZE_500, L"500%");
	CheckMenuRadioItem(view_window_size_menu, IDM_VIEW_WINDOW_SIZE_100, IDM_VIEW_WINDOW_SIZE_500, IDM_VIEW_WINDOW_SIZE_200, MF_BYCOMMAND);
	AppendMenu(view_menu, MF_POPUP | MF_STRING, (UINT_PTR)view_window_size_menu, L"Window Size");
	AppendMenu(view_menu, MF_STRING, IDM_VIEW_DEBUG, L"Debugger");
	CheckMenuItem(view_menu, IDM_VIEW_DEBUG, MF_UNCHECKED);
	AppendMenu(menu_bar, MF_POPUP, (UINT_PTR)view_menu, L"View");

	SetMenu(hwnd, menu_bar);
}

void SetWindowScale(HWND hwnd, int scale)
{
	scale_factor = scale;
	unsigned int width = scale_factor * SCREEN_WIDTH;
	unsigned int height = scale_factor * SCREEN_HEIGHT;

	RECT client, window;
	POINT diff;
	GetClientRect(hwnd, &client);
	GetWindowRect(hwnd, &window);
	diff.x = (window.right - window.left) - client.right;
	diff.y = (window.bottom - window.top) - client.bottom;
	MoveWindow(hwnd, window.left, window.top, width + diff.x, height + diff.y, TRUE);
}

void LoadROM(HWND hwnd)
{
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[100] = { 0 };       // if using TCHAR macros
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"GameBoy ROM\0*.gb;*.rom\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		std::vector<char> rom;
		const std::wstring file(ofn.lpstrFile);
		if (mattboy::ReadFile(file, rom))
		{
			gameboy.LoadCartridge(rom, file);
			if (gameboy.GetCartridge()->IsValid())
			{
				std::cout << "TODO: Update window title" << std::endl;
				gameboy.SetRunning(true);
			}
			else
			{
				MessageBox(NULL, L"Unable to load cartridge. It is either invalid or unsupported at this time.", L"Invalid Cartridge!", MB_OK);
				gameboy.SetRunning(false);
			}
		}
		else
		{
			MessageBox(NULL, L"Unable to load ROM!", L"Error", MB_OK);
		}
	}
}

void MenuItemClick(HWND hwnd, WPARAM wParam)
{
	UINT state;

	switch (wParam)
	{
	case IDM_FILE_LOAD_ROM:
		LoadROM(hwnd);
		break;
	case IDM_FILE_SAVE_STATE:
		std::cout << "TODO: Save State" << std::endl;
		break;
	case IDM_FILE_LOAD_STATE:
		std::cout << "TODO: Load State" << std::endl;
		break;
	case IDM_FILE_OPTIONS:
		std::cout << "TODO: Options" << std::endl;
		break;
	case IDM_FILE_QUIT:
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		break;
	case IDM_EMULATION_PAUSE:
		state = GetMenuState(emulation_menu, IDM_EMULATION_PAUSE, MF_BYCOMMAND);
		if (state == MF_CHECKED)
		{
			gameboy.SetRunning(true);
			CheckMenuItem(emulation_menu, IDM_EMULATION_PAUSE, MF_UNCHECKED);
		}
		else
		{
			gameboy.SetRunning(false);
			CheckMenuItem(emulation_menu, IDM_EMULATION_PAUSE, MF_CHECKED);
		}
		break;
	case IDM_EMULATION_RESET:
		// TODO: Do we need to reload ROM before reset too?
		gameboy.Reset();
		break;
	case IDM_EMULATION_SPEED_1:
		CheckMenuRadioItem(emulation_speed_menu, IDM_EMULATION_SPEED_1, IDM_EMULATION_SPEED_10, IDM_EMULATION_SPEED_1, MF_BYCOMMAND);
		std::cout << "TODO: Emulation speed x1" << std::endl;
		break;
	case IDM_EMULATION_SPEED_2:
		CheckMenuRadioItem(emulation_speed_menu, IDM_EMULATION_SPEED_1, IDM_EMULATION_SPEED_10, IDM_EMULATION_SPEED_2, MF_BYCOMMAND);
		std::cout << "TODO: Emulation speed x2" << std::endl;
		break;
	case IDM_EMULATION_SPEED_5:
		CheckMenuRadioItem(emulation_speed_menu, IDM_EMULATION_SPEED_1, IDM_EMULATION_SPEED_10, IDM_EMULATION_SPEED_5, MF_BYCOMMAND);
		std::cout << "TODO: Emulation speed x5" << std::endl;
		break;
	case IDM_EMULATION_SPEED_10:
		CheckMenuRadioItem(emulation_speed_menu, IDM_EMULATION_SPEED_1, IDM_EMULATION_SPEED_10, IDM_EMULATION_SPEED_10, MF_BYCOMMAND);
		std::cout << "TODO: Emulation speed x10" << std::endl;
		break;
	case IDM_EMULATION_TYPE_DMG:
		std::cout << "TODO: Set DMG" << std::endl;
		CheckMenuRadioItem(emulation_type_menu, IDM_EMULATION_TYPE_DMG, IDM_EMULATION_TYPE_ADVANCE, IDM_EMULATION_TYPE_DMG, MF_BYCOMMAND);
		break;
	case IDM_EMULATION_TYPE_COLOR:
		std::cout << "TODO: Set GB Color" << std::endl;
		CheckMenuRadioItem(emulation_type_menu, IDM_EMULATION_TYPE_DMG, IDM_EMULATION_TYPE_ADVANCE, IDM_EMULATION_TYPE_COLOR, MF_BYCOMMAND);
		break;
	case IDM_EMULATION_TYPE_ADVANCE:
		std::cout << "TODO: Set GB Advance" << std::endl;
		CheckMenuRadioItem(emulation_type_menu, IDM_EMULATION_TYPE_DMG, IDM_EMULATION_TYPE_ADVANCE, IDM_EMULATION_TYPE_ADVANCE, MF_BYCOMMAND);
		break;
	case IDM_VIEW_WINDOW_SIZE_100:
		SetWindowScale(hwnd, 1);
		CheckMenuRadioItem(view_window_size_menu, IDM_VIEW_WINDOW_SIZE_100, IDM_VIEW_WINDOW_SIZE_500, IDM_VIEW_WINDOW_SIZE_100, MF_BYCOMMAND);
		break;
	case IDM_VIEW_WINDOW_SIZE_200:
		SetWindowScale(hwnd, 2);
		CheckMenuRadioItem(view_window_size_menu, IDM_VIEW_WINDOW_SIZE_100, IDM_VIEW_WINDOW_SIZE_500, IDM_VIEW_WINDOW_SIZE_200, MF_BYCOMMAND);
		break;
	case IDM_VIEW_WINDOW_SIZE_300:
		SetWindowScale(hwnd, 3);
		CheckMenuRadioItem(view_window_size_menu, IDM_VIEW_WINDOW_SIZE_100, IDM_VIEW_WINDOW_SIZE_500, IDM_VIEW_WINDOW_SIZE_300, MF_BYCOMMAND);
		break;
	case IDM_VIEW_WINDOW_SIZE_400:
		SetWindowScale(hwnd, 4);
		CheckMenuRadioItem(view_window_size_menu, IDM_VIEW_WINDOW_SIZE_100, IDM_VIEW_WINDOW_SIZE_500, IDM_VIEW_WINDOW_SIZE_400, MF_BYCOMMAND);
		break;
	case IDM_VIEW_WINDOW_SIZE_500:
		SetWindowScale(hwnd, 5);
		CheckMenuRadioItem(view_window_size_menu, IDM_VIEW_WINDOW_SIZE_100, IDM_VIEW_WINDOW_SIZE_500, IDM_VIEW_WINDOW_SIZE_500, MF_BYCOMMAND);
		break;
	case IDM_VIEW_DEBUG:
		state = GetMenuState(view_menu, IDM_VIEW_DEBUG, MF_BYCOMMAND);
		if (state == MF_CHECKED)
		{
			std::cout << "TODO: Hide debugger" << std::endl;
			CheckMenuItem(view_menu, IDM_VIEW_DEBUG, MF_UNCHECKED);
		}
		else
		{
			std::cout << "TODO: Show debugger" << std::endl;
			CheckMenuItem(view_menu, IDM_VIEW_DEBUG, MF_CHECKED);
		}
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		AddMenus(hwnd);
		break;
	case WM_COMMAND:
		MenuItemClick(hwnd, wParam);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void GameboyLoop(mattboy::Gameboy* gameboy)
{
	auto last_time = std::chrono::high_resolution_clock::now();
	int cycles = 0;

	while (!quit)
	{
		auto current_time = std::chrono::high_resolution_clock::now();
		auto time = current_time - last_time;
		auto ns = time / std::chrono::nanoseconds(1);
		if (ns >= mattboy::CPU::NANOSECONDS_PER_CLOCK * cycles)
		{
			last_time = current_time;
			cycles = gameboy->Cycle();
		}
	}

	gameboy->SetRunning(false);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	AllocConsole();
	FILE* new_stdout;
	freopen_s(&new_stdout, "CONOUT$", "w", stdout);

	WNDCLASSEX wc_;
	MSG msg;
	HWND hwnd;

	wc_.cbSize = sizeof(WNDCLASSEX);
	wc_.style = 0;
	wc_.lpfnWndProc = WndProc;
	wc_.cbClsExtra = 0;
	wc_.cbWndExtra = 0;
	wc_.hInstance = hInstance;
	wc_.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc_.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc_.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc_.lpszMenuName = NULL;
	wc_.lpszClassName = g_szClassName;
	wc_.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc_))
	{
		MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Step 2: Creating the Window
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		WINDOW_TITLE,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	SetWindowScale(hwnd, scale_factor);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	HDC hdc = GetDC(hwnd);

	BITMAPINFO bmi = {
		sizeof(BITMAPINFOHEADER),
		SCREEN_WIDTH,
		-SCREEN_HEIGHT,
		1,
		32
	};

	/*std::vector<char> rom;
	const std::wstring file(L"C:/Users/Matthew/Desktop/Programming Projects/MattBoy VS/MattBoy/Debug/roms/Tetris (JUE) (V1.1) [!].gb");
	if (mattboy::ReadFile(file, rom))
	{
	gameboy.LoadCartridge(rom, file);
	gameboy.SetRunning(true);
	}*/

	std::thread gameboyThread(GameboyLoop, &gameboy);


	quit = false;
	while (!quit)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				quit = true;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		StretchDIBits(hdc, 0, 0, SCREEN_WIDTH * scale_factor, SCREEN_HEIGHT * scale_factor, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, gameboy.GetScreen(), &bmi, 0, SRCCOPY);
	}

	gameboyThread.join();

	ReleaseDC(hwnd, hdc);
	FreeConsole();

	return msg.wParam;
}

#endif