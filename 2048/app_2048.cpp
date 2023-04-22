#include "app_2048.h"
#include <stdexcept>
#include "board.h"
#include <Windows.h>
std::wstring const app_2048::s_class_name{ L"2048 Window" };

//constructor initialising the screen centre option
app_2048::app_2048(HINSTANCE instance)
    : m_instance{ instance }
{
    register_class();

    DWORD main_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
    DWORD popup_style = WS_OVERLAPPED | WS_CAPTION;
    m_main = create_window(main_style);
    m_popup = create_window(popup_style, m_main);
    SetLayeredWindowAttributes(m_popup, 0, 255, LWA_ALPHA);
    ball_x = 10;
    ball_y = 10;
    ball_r = 7;
}


//we then change the colour of the bacvkground here 
bool app_2048::register_class()
{             
    WNDCLASSEXW desc{};
    if (GetClassInfoExW(m_instance, s_class_name.c_str(),
        &desc) != 0)
        return true;

    desc = {
    .cbSize = sizeof(WNDCLASSEXW),
    .lpfnWndProc = window_proc_static,
    .hInstance = m_instance,
    .hCursor = LoadCursorW(nullptr, L"IDC_ARROW"),
    .hbrBackground = CreateSolidBrush(RGB(92,243,0)),//new addition changes background
    .lpszClassName = s_class_name.c_str()
    };
    return RegisterClassExW(&desc) != 0;
}

HWND app_2048::create_window(DWORD style, HWND parent, DWORD ex_style)
{

    RECT size;
    GetWindowRect(parent, &size); //getting dimensions of the parent window and putting it into size
    //https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowrect
    //
    int x = (GetSystemMetrics(SM_CXSCREEN) - (size.right - size.left)) / 2; //calculating centre 

    int y = (GetSystemMetrics(SM_CYSCREEN) - (size.bottom - size.top)) / 2; //calculating centre
    SetWindowPos(parent, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    update_transparency();
    HWND window = CreateWindowExW(
        ex_style,
        s_class_name.c_str(),
        L"Main Window",
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        500,
        350,
        parent,
        nullptr,
        m_instance,
        this);

    RECT child;
    int cwidth = 15;
    int cheight = 60;
    GetClientRect(window, &child); //to get dimensions of the client area of the window
    int a = (child.right - cwidth);
    int b = (child.bottom - cheight) / 2;

    CreateWindowExW(
        0,
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE,
        a,
        b,
        15,
        60,
        window,
        nullptr,
        m_instance,
        nullptr);

    return window;
}


LRESULT app_2048::window_proc_static(
    HWND window,
    UINT message,
    WPARAM wparam,
    LPARAM lparam)
{
    app_2048* app = nullptr;
    if (message == WM_NCCREATE)
    {
        app = static_cast<app_2048*>(
            reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(app));

    }
    else
        app = reinterpret_cast<app_2048*>(
            GetWindowLongPtrW(window, GWLP_USERDATA));
    LRESULT res = app ?
        app->window_proc(window, message, wparam, lparam) :
        DefWindowProcW(window, message, wparam, lparam);
    if (message == WM_NCDESTROY)
        SetWindowLongPtrW(window, GWLP_USERDATA, 0);
    return res;
}

LRESULT app_2048::window_proc(
    HWND window,
    UINT message,
    WPARAM wparam,
    LPARAM lparam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(window, &ps);

        // draw ball
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));  // red brush
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        Ellipse(hdc, ball_x - ball_r, ball_y - ball_r, ball_x + ball_r, ball_y + ball_r);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);

        EndPaint(window, &ps);
    }
    break;

    case WM_TIMER:
    {
        // update ball position
        ball_x += 3;
        ball_y += 3;
        RECT client;
        GetClientRect(window, &client);
        if (ball_x - ball_r < 0 || ball_x + ball_r > client.right)
        {
            ball_x += -50;  // reverse the direction in x-axis
        }

        if (ball_y - ball_r < 0 || ball_y + ball_r > client.bottom)
        {
            ball_y += -50;  // reverse the direction in y-axis
        }
        // redraw window
        InvalidateRect(window, NULL, TRUE);
    }
    break;
    case WM_CLOSE:
        DestroyWindow(m_main);
        return 0;
    case WM_DESTROY:
        if (window == m_main)
            PostQuitMessage(EXIT_SUCCESS);
        return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

void app_2048::update_transparency()   //https://www.codeproject.com/Questions/61633/creating-a-transparent-window
{
    SetWindowLong(m_main, GWL_EXSTYLE, GetWindowLong(m_main, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(m_main, 0, 255 * 80 / 100, LWA_ALPHA);
}

int app_2048::run(int show_command)
{
    ShowWindow(m_main, show_command);
    //ShowWindow(m_popup, SW_SHOWNA); //3.3 number 4
    MSG msg{};
    BOOL result = TRUE;
    SetTimer(m_main, 1, 50, NULL);
    while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
    {
        if (result == -1)
            return EXIT_FAILURE;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return EXIT_SUCCESS;
}



//void app_2048::on_window_move(HWND window,
//LPWINDOWPOS params)
//{
//    // RECT window_rc;
//    HWND other = (window == m_main) ? m_popup : m_main;
//    RECT other_rc;
//    GetWindowRect(other, &other_rc);
//
//    SIZE other_size{
//    .cx = other_rc.right - other_rc.left,
//    .cy = other_rc.bottom - other_rc.top };
//
//    POINT window_centre{ params->x + other_size.cx / 2, params->y + other_size.cy / 2 };
//    POINT other_centre{ 2 * m_screen_centre.x - window_centre.x,2 * m_screen_centre.y - window_centre.y };
//
//    POINT new_pos{
//        /*calculate the new position of the other window*/
//        .x = other_centre.x - other_size.cx / 2,
//        .y = other_centre.y - other_size.cy / 2
//    };
//
//    if (new_pos.x == other_rc.left &&
//        new_pos.y == other_rc.top)
//        return;
//    SetWindowPos(
//        other,
//        nullptr,
//        new_pos.x,
//        new_pos.y,
//        0,
//        0,
//        SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
//}

//this includes icon addition
//bool app_2048::register_class()
//{
//    WNDCLASSEXW desc{};
//    if (GetClassInfoExW(m_instance, s_class_name.c_str(),
//        &desc) != 0)
//        return true;
//    desc = {
//    .cbSize = sizeof(WNDCLASSEXW),
//    .lpfnWndProc = window_proc_static,
//    .hInstance = m_instance,
//    .hIcon = static_cast<HICON>(LoadImageW(
//    m_instance,
//    MAKEINTRESOURCEW(ID_APPICON),
//    IMAGE_ICON,
//    0, 0,
//    LR_SHARED | LR_DEFAULTSIZE)),
//        /*you can also use:
//        LoadIconW(m_instance,
//        MAKEINTRESOURCEW(ID_APPICON)) */
//        .hCursor = LoadCursorW(nullptr, L"IDC_ARROW"),
//        .hbrBackground =
//        CreateSolidBrush(RGB(250, 247, 238)),
//        .lpszClassName = s_class_name.c_str()
//    };
//    return RegisterClassExW(&desc) != 0;
//}



//HWND app_2048::create_window()
//{
//    return CreateWindowExW(
//        0,
//        s_class_name.c_str(),
//        L"Main Window",
//        WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
//        WS_BORDER | WS_MINIMIZEBOX,
//        CW_USEDEFAULT,
//        0,
//        CW_USEDEFAULT,
//        0,
//        nullptr,
//        nullptr,
//        m_instance,
//        this);
//}


//HWND app_2048::create_window(DWORD style, HWND parent)
//{
//    return CreateWindowExW(
//        0,
//        s_class_name.c_str(),
//        L"2048",
//        style, //change here
//        CW_USEDEFAULT,
//        0,
//        CW_USEDEFAULT,
//        0,
//        parent, //and here
//        nullptr,
//        m_instance,
//        this);
//}

//this is the new createwindow that takes into account the neww board size as well as the window size

//HWND app_2048::create_window(DWORD style, HWND parent)
//{
//    RECT size{ 0, 0, board::width, board::height };
//    AdjustWindowRectEx(&size, style, false, 0); //this adjusts the window size
//    return CreateWindowExW(
//        0,
//        s_class_name.c_str(),
//        L"2048", style,
//        CW_USEDEFAULT,
//        0,
//        size.right - size.left,
//        size.bottom - size.top,
//        parent,
//        nullptr,
//        m_instance,
//        this);
//}

//we extend the createwindow class to add the static control child window for each of the board tiles
//HWND app_2048::create_window(DWORD style, HWND parent)
//{
//    RECT size{ 0, 0, board::width, board::height };
//    AdjustWindowRectEx(&size, style, false, 0); //this adjusts the window size
//        HWND window = CreateWindowExW(0,
//            s_class_name.c_str(),
//            L"2048", style,
//            CW_USEDEFAULT,
//            0,
//            size.right - size.left,
//            size.bottom - size.top,
//            parent,
//            nullptr,
//            m_instance,
//            this);
//
//    for (auto& f : m_board.fields())
//        CreateWindowExW(
//            0,
//            L"STATIC",
//            nullptr,
//            WS_CHILD | WS_VISIBLE | SS_CENTER,
//            f.position.left,
//            f.position.top,
//            f.position.right - f.position.left,
//            f.position.bottom - f.position.top,
//            window,
//            nullptr,
//            m_instance,
//            nullptr);
//    return window;
//}

//added for transparency 

//this new procedure returns the brush in response to the message
//LRESULT app_2048::window_proc(
//HWND window,
//UINT message,
//WPARAM wparam,
//LPARAM lparam)
//{
//    switch (message)
//    {
//    case WM_CLOSE:
//        DestroyWindow(m_main);
//        return 0;
//    case WM_DESTROY:
//        if (window == m_main)
//            PostQuitMessage(EXIT_SUCCESS);
//        return 0;
//    case WM_CTLCOLORSTATIC:
//        return reinterpret_cast<INT_PTR>(m_field_brush);
//    }
//    return DefWindowProcW(window, message, wparam, lparam);
//}

//LRESULT app_2048::window_proc(
//    HWND window,
//    UINT message,
//    WPARAM wparam,
//    LPARAM lparam)
//{
//    switch (message)
//    {
//    case WM_CLOSE:
//        DestroyWindow(m_main);
//        return 0;
//    case WM_DESTROY:
//        if (window == m_main)
//            PostQuitMessage(EXIT_SUCCESS);
//        return 0;
//    case WM_CTLCOLORSTATIC:
//        return reinterpret_cast<INT_PTR>(m_field_brush);
//    case WM_WINDOWPOSCHANGED:
//        on_window_move(window,
//            reinterpret_cast<LPWINDOWPOS>(lparam));
//        return 0;
//    }
//    return DefWindowProcW(window, message, wparam, lparam);
//}


//void app_2048::on_window_move(
//    HWND window,
//    LPWINDOWPOS params)
//{
//    // RECT window_rc;
//    HWND other = (window == m_main) ? m_popup : m_main;
//    RECT other_rc;
//    GetWindowRect(other, &other_rc);
//
//    SIZE other_size{
//    .cx = other_rc.right - other_rc.left,
//    .cy = other_rc.bottom - other_rc.top };
//
//    POINT window_centre{ params->x + other_size.cx / 2, params->y + other_size.cy / 2 };
//    POINT other_centre{ 2 * m_screen_centre.x - window_centre.x,2 * m_screen_centre.y - window_centre.y };
//
//    POINT new_pos{
//        /*calculate the new position of the other window*/
//        .x = other_centre.x - other_size.cx / 2,
//        .y = other_centre.y - other_size.cy / 2
//    };
//
//    if (new_pos.x == other_rc.left &&
//        new_pos.y == other_rc.top)
//        return;
//    SetWindowPos(
//        other,
//        nullptr,
//        new_pos.x,
//        new_pos.y,
//        0,
//        0,
//        SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
//}
//this selects the transparency level based on the intersection of the two windows
//
//void app_2048::update_transparency()
//{
//    RECT main_rc, popup_rc, intersection;
//    GetWindowRect(m_main, &main_rc);
//    GetWindowRect(m_popup, &popup_rc);
//    IntersectRect(&intersection, &main_rc, &popup_rc);
//    BYTE alpha = IsRectEmpty(&intersection) ?
//        255 :
//        255 * 50 / 100;
//    SetLayeredWindowAttributes(m_popup, 0, alpha, LWA_ALPHA);
//}
//
//void app_2048::update_transparency()
//{
//    RECT main_rc, popup_rc, intersection;
//    DwmGetWindowAttribute(
//        m_main,
//        DWMWA_EXTENDED_FRAME_BOUNDS,
//        &main_rc,
//        sizeof(RECT));
//    DwmGetWindowAttribute(
//        m_popup,
//        DWMWA_EXTENDED_FRAME_BOUNDS,
//        &popup_rc,
//        sizeof(RECT));
//    IntersectRect(&intersection, &main_rc, &popup_rc);
//    BYTE a =
//        IsRectEmpty(&intersection) ? 255 : 255 * 50 / 100;
//    SetLayeredWindowAttributes(m_popup, 0, a, LWA_ALPHA);
//}

/*
https://stackoverflow.com/questions/3463471/how-to-set-background-color-of-window-after-i
https://stackoverflow.com/questions/3970066/creating-a-transparent-window-in-c-win32
https://stackoverflow.com/questions/4631706/is-moving-a-window-with-setwindowpos-the-normal-way-to-do
https://stackoverflow.com/questions/33048092/calling-movewindow-with-brepaint-set-to-true
https://stackoverflow.com/questions/72505177/how-to-display-the-console-application-window-in-the-center-of-the-screen
*/







//OLD VERSION
// 
// 
//#include "app_2048.h"
//#include <stdexcept>
////#include "resource.h"
//
////std::wstring const app_2048::s_class_name{ L"2048 Window" };
////
////app_2048::app_2048(HINSTANCE instance)
////    : m_instance{ instance }, m_main{}
////{
////    register_class();
////        m_main = create_window();
////      // m_main = create_window(main_style);
////
////}
// //first window
//
//app_2048::app_2048(HINSTANCE instance)
//    : m_instance{ instance }, m_main{}, m_popup{},
//    m_field_brush{ CreateSolidBrush(RGB(204, 192, 174)) }, m_screen_centre{ GetSystemMetrics(SM_CXSCREEN) / 2,
//    GetSystemMetrics(SM_CYSCREEN) / 2 }
//{
//    register_class();
//
//    DWORD main_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
//    DWORD popup_style = WS_OVERLAPPED | WS_CAPTION;
//    m_main = create_window(main_style);
//    m_popup = create_window(popup_style, m_main, WS_EX_LAYERED);
//    SetLayeredWindowAttributes(m_popup, 0, 255, LWA_ALPHA);
//}//second window
//
////
////bool app_2048::register_class()
////{
////    WNDCLASSEXW desc{};
////    if (GetClassInfoExW(m_instance, s_class_name.c_str(),
////        &desc) != 0)
////        return true;
////    desc = {
////    .cbSize = sizeof(WNDCLASSEXW),
////    .lpfnWndProc = window_proc_static,
////    .hInstance = m_instance,
////    .hCursor = LoadCursorW(nullptr, L"IDC_ARROW"),
////    .hbrBackground =
////CreateSolidBrush(RGB(250, 247, 238)), //adds background color
////    .lpszClassName = s_class_name.c_str()
////    };
////    return RegisterClassExW(&desc) != 0;
////}
//
//bool app_2048::register_class()
//{
//    WNDCLASSEXW desc{};
//    if (GetClassInfoExW(m_instance, s_class_name.c_str(),
//        &desc) != 0)
//        return true;
//    desc = {
//    .cbSize = sizeof(WNDCLASSEXW),
//    .lpfnWndProc = window_proc_static,
//    .hInstance = m_instance,
//    .hIcon = static_cast<HICON>(LoadImageW(m_instance,MAKEINTRESOURCEW(ID_APPICON),IMAGE_ICON,0, 0,LR_SHARED | LR_DEFAULTSIZE)),
//    /*you can also use:
//    LoadIconW(m_instance,
//    MAKEINTRESOURCEW(ID_APPICON))*/
//    .hCursor = LoadCursorW(nullptr, L"IDC_ARROW"),
//    .hbrBackground =
//    CreateSolidBrush(RGB(250, 247, 238)),
//    .lpszClassName = s_class_name.c_str()
//    };
//    return RegisterClassExW(&desc) != 0;
//}
//
//
////HWND app_2048::create_window()
////{
////    return CreateWindowExW(
////        0,
////        s_class_name.c_str(),
////        L"2048",
////        WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
////        WS_BORDER | WS_MINIMIZEBOX,
////        CW_USEDEFAULT,
////        0,
////        CW_USEDEFAULT,
////        0,
////        nullptr,
////        nullptr,
////        m_instance,
////        this);
////}
//
//
///*
//HWND app_2048::create_window(DWORD style, HWND parent)
//{
//    return CreateWindowExW(
//        0,
//        s_class_name.c_str(),
//        L"2048",
//        style, //change here
//        CW_USEDEFAULT,
//        0,
//        CW_USEDEFAULT,
//        0,
//        parent, //and here
//        nullptr,
//        m_instance,
//        this);
//}
//*/ //part2
//
////HWND app_2048::create_window(DWORD style, HWND parent)
////{
////    RECT size{ 0, 0, board::width, board::height }; 
////    AdjustWindowRectEx(&size, style, false, 0);
////    HWND window = CreateWindowExW(
////        0,
////        s_class_name.c_str(),
////        L"2048", style,
////        CW_USEDEFAULT,
////        0,
////        size.right - size.left,
////        size.bottom - size.top,
////        parent,
////        nullptr,
////        m_instance,
////        this);
////    for (auto& f : m_board.fields()) 
////        CreateWindowExW(
////            0,
////            L"STATIC",
////            nullptr,
////            WS_CHILD | WS_VISIBLE | SS_CENTER,
////            f.position.left,
////            f.position.top,
////            f.position.right - f.position.left,
////            f.position.bottom - f.position.top,
////            window,
////            nullptr,
////            m_instance,
////            nullptr);
////    return window;
////}
//
//HWND app_2048::create_window(DWORD style, HWND parent, DWORD ex_style)
//{
//    RECT size{ 0, 0, board::width, board::height };
//    AdjustWindowRectEx(&size, style, false, 0);
//    HWND window = CreateWindowExW(
//        ex_style,
//        s_class_name.c_str(),
//        L"2048", style,
//        CW_USEDEFAULT,
//        0,
//        size.right - size.left,
//        size.bottom - size.top,
//        parent,
//        nullptr,
//        m_instance,
//        this);
//    for (auto& f : m_board.fields())
//        CreateWindowExW(
//            0,
//            L"STATIC",
//            nullptr,
//            WS_CHILD | WS_VISIBLE | SS_CENTER,
//            f.position.left,
//            f.position.top,
//            f.position.right - f.position.left,
//            f.position.bottom - f.position.top,
//            window,
//            nullptr,
//            m_instance,
//            nullptr);
//    return window;
//}
//
//
//
//LRESULT app_2048::window_proc_static(
//    HWND window,
//    UINT message,
//    WPARAM wparam,
//    LPARAM lparam)
//{
//    app_2048* app = nullptr;
//    if (message == WM_NCCREATE)
//    {
//        app = static_cast<app_2048*>(
//            reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams);
//        SetWindowLongPtrW(window, GWLP_USERDATA,
//            reinterpret_cast<LONG_PTR>(app));
//    }
//    else
//        app = reinterpret_cast<app_2048*>(
//            GetWindowLongPtrW(window, GWLP_USERDATA));
//    LRESULT res = app ?
//        app->window_proc(window, message, wparam, lparam) :
//        DefWindowProcW(window, message, wparam, lparam);
//    if (message == WM_NCDESTROY)
//        SetWindowLongPtrW(window, GWLP_USERDATA, 0);
//    return res;
//}
//
//LRESULT app_2048::window_proc(
//    HWND window,
//    UINT message,
//    WPARAM wparam,
//    LPARAM lparam)
//{
//    switch (message)
//    {
//    case WM_CLOSE:
//        DestroyWindow(m_main);
//        return 0;
//    case WM_DESTROY:
//        if (window == m_main)
//            PostQuitMessage(EXIT_SUCCESS);
//        return 0;
//    case WM_CTLCOLORSTATIC: //sets the background of a control by returning the brush
//        return reinterpret_cast<INT_PTR>(m_field_brush);
//    case WM_WINDOWPOSCHANGED:
//        on_window_move(window, reinterpret_cast<LPWINDOWPOS>(lparam));
//        return 0;
//    }
//    return DefWindowProcW(window, message, wparam, lparam);
//}
//
//int app_2048::run(int show_command)
//{
//    ShowWindow(m_main, show_command);
//    ShowWindow(m_popup, SW_SHOWNA); //3.3 number 4
//    MSG msg{};
//    BOOL result = TRUE;
//    while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
//    {
//        if (result == -1)
//            return EXIT_FAILURE;
//        TranslateMessage(&msg);
//        DispatchMessageW(&msg);
//    }
//    return EXIT_SUCCESS;
//}
//
//void app_2048::on_window_move(HWND window, LPWINDOWPOS params)
//{
//    HWND other = (window == m_main) ? m_popup : m_main;
//    RECT other_rc;
//    GetWindowRect(other, &other_rc);
//    SIZE other_size{
//    .cx = other_rc.right - other_rc.left,
//    .cy = other_rc.bottom - other_rc.top };
//    // Calculate the centre of the moved window
//    POINT moved_centre{ params->x + other_size.cx / 2, params->y + other_size.cy / 2 };
//    // Calculate the centre of the screen
//    POINT screen_centre = m_screen_centre;
//    // Calculate the centre of the other window
//    POINT other_centre{ 2 * screen_centre.x - moved_centre.x, 2 * screen_centre.y - moved_centre.y };
//    POINT new_pos{
//        .x = other_centre.x - other_size.cx / 2,
//        .y = other_centre.y - other_size.cy / 2
//    };
//    if (new_pos.x == other_rc.left && new_pos.y == other_rc.top)
//        return;
//    SetWindowPos(
//        other,
//        nullptr,
//        new_pos.x,
//        new_pos.y,
//        0,
//        0,
//        SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
//}
//
////void app_2048::update_transparency()
////{
////    RECT main_rc, popup_rc, intersection;
////    GetWindowRect(m_main, &main_rc);
////    GetWindowRect(m_popup, &popup_rc);
////    IntersectRect(&intersection, &main_rc, &popup_rc);
////    BYTE alpha = IsRectEmpty(&intersection) ?
////        255 :
////        255 * 50 / 100;
////    SetLayeredWindowAttributes(m_popup, 0, alpha, LWA_ALPHA);
////}
//
//
//void app_2048::update_transparency()
//{
//    RECT main_rc, popup_rc, intersection;
//    DwmGetWindowAttribute(
//        m_main,
//        DWMWA_EXTENDED_FRAME_BOUNDS,
//        &main_rc,
//        sizeof(RECT));
//    DwmGetWindowAttribute(
//        m_popup,
//        DWMWA_EXTENDED_FRAME_BOUNDS,
//        &popup_rc,
//        sizeof(RECT));
//    IntersectRect(&intersection, &main_rc, &popup_rc);
//    BYTE a =
//        IsRectEmpty(&intersection) ? 255 : 255 * 50 / 100;
//    SetLayeredWindowAttributes(m_popup, 0, a, LWA_ALPHA);
//}
