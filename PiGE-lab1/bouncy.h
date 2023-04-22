#pragma once
#include <windows.h>
#include <string>
#include "paddle.h"
#include <tchar.h>
#include <windowsx.h>
#include "resource.h"


class bouncy
{
private:
	bool register_class();
	
	static std::wstring const s_class_name;  // static member function that serves as the window procedure for the application.
//	 std::wstring const ball_class_name;
//	 std::wstring const paddle_class_name;
	 static LRESULT CALLBACK window_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
	void Paddle_position(HWND window, LPARAM lparam);
    LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam); //recieves messages and stores them
	HWND create_window(DWORD style, HWND parent = nullptr , DWORD ex_style = 0);
    void ball_paddle_collision();
	void Draw_trail(HDC hdc);
	HINSTANCE m_instance;
	HWND m_main, m_child, window;
	
	POINT centre_screen;
	HWND paddle;
	HWND ball;
	HDC hdc;
	COLORREF background_colour = RGB(140, 238, 140);
	void resetgame();
	void transparency();
    void move_window(int value_y);
	void start_new_game(HWND window, HWND ball);
	void UpdateBitmap(HWND window);
	HMENU menu_creation();
	void LoadAndDisplayBitmap(HWND hwnd, LPCWSTR filename);

	INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:

	//paddle_dimensions paddle;
	bouncy(HINSTANCE instance);
	int run(int show_command);
};