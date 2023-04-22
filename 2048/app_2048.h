#pragma once
#include <windows.h>
#include "board.h"
#include <string>
class app_2048
{
private:
	bool register_class(); //This function is used to register the window class for the application.

	static std::wstring const s_class_name;  // static member function that serves as the window procedure for the application.
	static LRESULT CALLBACK window_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

	LRESULT window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam); //recieves messages and stores them

	//HWND create_window(DWORD style, HWND parent = nullptr); //This function creates the main window for the application and returns a handle to it.
	//create_window() was the older version when we had only 1 window.
	HWND create_window(DWORD style, HWND parent = nullptr, DWORD ex_style = 0); // extension for transparancy

	HINSTANCE m_instance;

	HWND m_main;
	HWND m_popup;

	//board m_board; // this is a board member field

	POINT m_screen_centre; //this finds the centre of the screen
	paddle pad;
	int ball_x;
	int ball_y;
	int ball_r;
	//void on_window_move(HWND window, LPWINDOWPOS params); // this allows the two windows to move together

	void update_transparency();
public:

	app_2048(HINSTANCE instance);
	int run(int show_command);
};















//OLD VERSION
//
//
//
//
//
//
//
//
//
//
//
//#pragma once
//#include "windows.h"
//#include "board.h"
//#include <string>
//#include <dwmapi.h>
//class app_2048
//{
//private:
//	board m_board;
//	HBRUSH m_field_brush;
//	POINT m_screen_centre;
//	void update_transparency();
//	//HWND create_window();
//	HWND create_window(DWORD style, HWND parent = nullptr, DWORD ex_style = 0);//creates main window without parameters then with oar for second window
//	void on_window_move(HWND window, LPWINDOWPOS params);
//	bool register_class(); //registers window class
//	static std::wstring const s_class_name; //stores name of the window class
//	static LRESULT CALLBACK window_proc_static( //static window proc, forwards msgs to non static var on class instance
//		HWND window,
//		UINT message,
//		WPARAM wparam,
//		LPARAM lparam);
//	LRESULT window_proc( //handles messages sent to the windows
//		HWND window,
//		UINT message,
//		WPARAM wparam,
//		LPARAM lparam);
//	HINSTANCE m_instance; //stores handle to app instance
//	HWND m_main, m_popup;//handle to main window //m_popup is to store another window handle (secnd window)
//public:
//
//	app_2048(HINSTANCE instance); //constructor initializes m_instance var
//	int run(int show_command); //runs msg loop of application & takes show_command which specifies how window is shown
//};