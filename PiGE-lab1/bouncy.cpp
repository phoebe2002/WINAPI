#include "bouncy.h"
#include "paddle.h"
#include <stdexcept>
#include <Windows.h>
#include "resource.h"

int wins = 0;
int losses = 0;;
int x_ball = 10;
int y_ball = 10;
int r_ball = 5;
int ball_speed_x = 5;
int ball_speed_y = 5;
int child_w = 15;
int child_h = 50;
int radius_of_trail = r_ball / 2;
int trail_transparent = 100;
int number_of_trails = 10;
WCHAR szFile[256];

int p_y;
//added for bitmap
HBITMAP hBmp = NULL;
HBITMAP hOldBmp = NULL;
bool isBitmap = false;
bool isStretched = false;

std::wstring const bouncy::s_class_name{ L"Bouncy Game" };

bouncy::bouncy(HINSTANCE instance)
    : m_instance{ instance } //m_main{},m_child{}
{
    register_class();
    DWORD main_style = WS_OVERLAPPED | WS_SYSMENU |
        WS_CAPTION | WS_MINIMIZEBOX;
    m_main = create_window(main_style);
    DWORD m_child_style = WS_OVERLAPPED | WS_CAPTION;
    m_child = create_window(m_child_style, m_main); 
    SetLayeredWindowAttributes(m_child, 0, 255, LWA_ALPHA);
}


bool bouncy::register_class()
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
    .hbrBackground = CreateSolidBrush(RGB(140, 238, 140)),
    .lpszMenuName = MAKEINTRESOURCEW(IDR_MENU1),
    .lpszClassName =s_class_name.c_str() 
    };
    return RegisterClassExW(&desc) != 0;
}


//new bouncy window for two windows
HWND bouncy::create_window(DWORD style, HWND parent, DWORD ex_style)
{
    //code to centre window from https://forums.codeguru.com/showthread.php?500841-Set-my-window-position-at-screen-center
    RECT rc;
    GetWindowRect(parent, &rc);

    int x = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) /2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.bottom)) / 2;
    SetWindowPos(parent, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    transparency();
   
    window = CreateWindowExW(
        ex_style,
        s_class_name.c_str(),
        L"Bouncy Game",
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
        HWND paddle_window = CreateWindowExW(
            0,
            L"STATIC",
            nullptr,
            WS_CHILD | WS_VISIBLE , /*WS_CLIPSIBLINGS*/
            child.right - cwidth,
            (child.bottom - cheight) / 2,
            15,
            60,
            window,
            nullptr,
            m_instance, nullptr);

        HWND trail = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
            s_class_name.c_str(),
            nullptr,
            WS_POPUP | WS_VISIBLE,
            0,
            0,
            0,
            0,
            window,
            nullptr,
            m_instance,
            nullptr);

        SetLayeredWindowAttributes(trail, RGB(255, 255, 255), 128, LWA_COLORKEY | LWA_ALPHA);

        SetMenu(window, menu_creation());
    return window;
    }


LRESULT bouncy::window_proc_static(
    HWND window,
    UINT message,
    WPARAM wparam,
    LPARAM lparam)
{
    bouncy* bounce = nullptr;
    if (message == WM_NCCREATE)
    {
        bounce = static_cast<bouncy*>(
            reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(bounce));

    }
    else
        bounce = reinterpret_cast<bouncy*>(
            GetWindowLongPtrW(window, GWLP_USERDATA));
    LRESULT res = bounce ?
        bounce->window_procedure(window, message, wparam, lparam) :
        DefWindowProcW(window, message, wparam, lparam);
    if (message == WM_NCDESTROY)
        SetWindowLongPtrW(window, GWLP_USERDATA, 0);
    return res;
}

void bouncy::move_window(int p_y) {

    RECT change_pos;
    RECT rect;
    GetWindowRect(GetParent(FindWindowExW(m_main, nullptr, L"STATIC", nullptr)), &change_pos);
    GetWindowRect(FindWindowExW(m_main, nullptr, L"STATIC", nullptr), &rect);
    MapWindowPoints(nullptr, m_main, reinterpret_cast<POINT*>(&rect), 2);
    int paddleHeight = rect.bottom - rect.top;
    int newY = max(0, min(p_y, change_pos.bottom - change_pos.top - paddleHeight));
    MoveWindow(FindWindowExW(m_main, nullptr, L"STATIC", nullptr), rect.left, newY, rect.right - rect.left, rect.bottom - rect.top, true);
}

LRESULT bouncy::window_procedure(
    HWND window,
    UINT message,
    WPARAM wparam,
    LPARAM lparam)
{
    static int pad_y = 0;
    static HBITMAP hBitmap = NULL;
    static BOOL bTile = FALSE;
    static BOOL bStretch = FALSE;
    static RECT rcClient;

    switch (message)
    {
    
    case WM_COMMAND:
    {
        int wmId = LOWORD(wparam);
        switch (wmId)
        {
        case ID_ABOUT:
             // DialogBoxW(m_instance, MAKEINTRESOURCE(ID_ABOUT_DIALOG_BOX), window, About);
            break;

        case ID_FILE_EXIT:
            DestroyWindow(window);
            break;
        case ID_FILE_NEWGAME:
            resetgame();
            break; 
        case ID_BACKGROUND_COLOUR:
            CHOOSECOLOR cc;
            static COLORREF acrCustClr[16];//we create an array of colors
            static DWORD rgbCurrent;// initial color selection


            cc.lStructSize = sizeof(cc);

            cc.hwndOwner = nullptr;
            cc.lpCustColors = (LPDWORD)acrCustClr;
            cc.rgbResult = rgbCurrent;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColor(&cc))
            {
                // Set the new background color
                HBRUSH hBrush = CreateSolidBrush(cc.rgbResult);
                background_colour = cc.rgbResult;
                SetClassLongPtr(window, (-10), (LONG_PTR)hBrush);
                InvalidateRect(window, nullptr, TRUE);
            }
            break;
       
        case ID_BITMAP:
        {

            //SOURCE https://stackoverflow.com/questions/36250254/problems-with-getopenfilename
            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = window;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Bitmap (*.bmp)\0*.bmp\0";
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (GetOpenFileName(&ofn))
            {

                HMENU menu = GetMenu(window);
                EnableMenuItem(menu, ID_BACKGROUND_TILE, MF_ENABLED);
                EnableMenuItem(menu, ID_BACKGROUND_STRETCH, MF_ENABLED);
                if (hBitmap != NULL)
                    DeleteObject(hBitmap);
                hBitmap = (HBITMAP)LoadImage(NULL, szFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
                if (hBitmap == NULL)
                {
                    MessageBox(window, L"Failed to load bitmap!", L"Error", MB_OK | MB_ICONERROR);
                    return 0;
                }
                isBitmap = true;
               // SelectItem(window, ID_BACKGROUND_TILE);
            }
        }
        break;
        }
    }

    case WM_TIMER:
    {

        x_ball += ball_speed_x;
        y_ball += ball_speed_y;

        RECT client;
        GetClientRect(window, &client);
        ball_paddle_collision();
        if (x_ball - r_ball < client.left)
        {
            ball_speed_x *= -1;  
        }

        if (y_ball - r_ball < client.top || y_ball + r_ball > client.bottom)
        {
            ball_speed_y *= -1;  
        }

        if (x_ball + r_ball > client.right) {
            ball_speed_x *= -1;
            losses++;
        }

        InvalidateRect(window, NULL, TRUE);
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(window, &ps);

        
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));  // red brush
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        Ellipse(hdc, x_ball - r_ball, y_ball - r_ball, x_ball + r_ball, y_ball + r_ball);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
        Draw_trail(hdc);

       
        COLORREF inverse = RGB(255, 255, 255) - COLOR_BACKGROUND;  //http://www.vb-helper.com/howto_invert_color.html
        HFONT hFont = CreateFont(70, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, inverse);
        SetBkColor(hdc, RGB(0, 0, 0));


        TCHAR s[20];
        _stprintf_s(s, _T("%d"), losses);
        RECT rc;
        GetClientRect(window, &rc);
        rc.left = 3 * (rc.right - rc.left) / 4;
        rc.top = (rc.top - rc.bottom) / 1.7;


        DrawText(hdc, s, -1, &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER);


        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        HFONT hFont1 = CreateFont(70, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
        HFONT hOldFont1 = (HFONT)SelectObject(hdc, hFont1);

        SetTextColor(hdc, inverse);
        SetBkMode(hdc, TRANSPARENT);
        SetBkColor(hdc, RGB(0, 0, 0));

        TCHAR s2[20];
        _stprintf_s(s2, _T("%d"), wins);
        RECT rc2;
        GetClientRect(window, &rc2);
        rc2.left = (rc2.right - rc2.left) / 5;
        rc2.top = (rc2.top - rc2.bottom) / 1.6;

        DrawText(hdc, s2, -1, &rc2, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
        LoadAndDisplayBitmap(window, szFile);
       //UpdateBitmap(window);


        EndPaint(window, &ps);
        break;

    }
    break;

    //case WM_LBUTTONDOWN:
    //    //FROM THE TUTORIAL
    //{

    //    HDC hdc = GetDC(window);
    //    HBRUSH brush = CreateSolidBrush(RGB(128, 128, 0));
    //    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    //    short x = (short)LOWORD(lparam);
    //    short y = (short)HIWORD(lparam);
    //    const int rad = 5;
    //    Ellipse(hdc, x - rad, y - rad, x + rad, y + rad);
    //    SelectObject(hdc, oldBrush);
    //    DeleteObject(brush);
    //    ReleaseDC(window, hdc);
    //}
    //break;
    case WM_MOUSEMOVE:
        pad_y = HIWORD(lparam) - (child_h / 2); //determines y coordinate of mouse cursor
        move_window(pad_y);
        break;
    
    case WM_CLOSE:
        DestroyWindow(m_main);
        return 0;
    case WM_DESTROY:
        if (window == m_main)
            PostQuitMessage(EXIT_SUCCESS);
        return 0;

    default:
        return DefWindowProc(window, message, wparam, lparam);

    }
    return DefWindowProcW(window, message, wparam, lparam);
}




void bouncy::ball_paddle_collision()
{
    //handling when ball and paddle collide
    RECT ball_rect{ x_ball - r_ball, y_ball - r_ball, x_ball + r_ball, y_ball + r_ball };
    paddle = FindWindowExW(m_main, nullptr, L"STATIC", nullptr);
    RECT paddle_rect;
    GetWindowRect(paddle, &paddle_rect);
    MapWindowPoints(nullptr, m_main, reinterpret_cast<POINT*>(&paddle_rect), 2);

    if (IntersectRect(&ball_rect, &ball_rect, &paddle_rect))
    {//if they collide i increase wins by 1
        wins++;
        ball_speed_x = -ball_speed_x;
        if (ball_speed_x < 0)
            x_ball = paddle_rect.left - r_ball;
        else
            x_ball = paddle_rect.right + r_ball;
    }
}
HMENU bouncy::menu_creation()
{
    HMENU Menu = CreateMenu();
    HMENU File = CreateMenu();
    HMENU Background = CreateMenu();
    HMENU Help = CreateMenu();

    AppendMenuW(File, MF_STRING, ID_FILE_NEWGAME, L"&New Game\t F2");
    AppendMenuW(File, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(File, MF_STRING, ID_FILE_EXIT, L"&Exit\t Alt+F4");

    AppendMenuW(Background, MF_STRING, ID_BACKGROUND_COLOUR, L"&Color\t Ctrl+C");
    AppendMenuW(Background, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(Background, MF_STRING, ID_BITMAP, L"&Bitmap \t Ctrl+B");
    AppendMenuW(Background, MF_STRING | MF_GRAYED, ID_BACKGROUND_TILE, L"&Tile \t Ctrl+T");
    AppendMenuW(Background, MF_STRING | MF_GRAYED, ID_BACKGROUND_STRETCH, L"&Stretch \t Ctrl+S");

    AppendMenuW(Help, MF_STRING, ID_ABOUT, L"About...");

    AppendMenuW(Menu, MF_POPUP, (UINT_PTR)File, L"File");
    AppendMenuW(Menu, MF_POPUP, (UINT_PTR)Background, L"Background");
    AppendMenuW(Menu, MF_POPUP, (UINT_PTR)Help, L"Help");


    EnableMenuItem(Background, ID_BACKGROUND_COLOUR, MF_ENABLED);

    EnableMenuItem(Background, ID_BITMAP, MF_ENABLED);

    return Menu;
}




void bouncy::resetgame()
{
    losses = 0;
    wins = 0;

    RECT g;
    GetClientRect(window, &g);
    int width = g.right - g.left;
    int height = g.bottom - g.top;
    x_ball = width / 4;
    y_ball = height / 4;
    ball_speed_x = abs(ball_speed_x);
    ball_speed_y = abs(ball_speed_y);
    InvalidateRect(window, NULL, TRUE);
}


int bouncy::run(int show_command)
{
    ShowWindow(m_main, show_command);
   // ShowWindow(m_child, SW_SHOWNA);
    MSG msg{};
    BOOL result = TRUE;
    SetTimer(m_main, 1, 50, NULL);
    HACCEL h_haccel = LoadAccelerators(m_instance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
    while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
    {
        if (result == -1)
            return EXIT_FAILURE;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);

        if (!TranslateAccelerator(window, h_haccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return EXIT_SUCCESS;
}

//source needed https://stackoverflow.com/questions/8787753/win32-c-setlayeredwindowattributes-is-either-fully-opaque-or-fully-transparent
void bouncy::transparency()
{
    SetWindowLong(m_main, GWL_EXSTYLE, GetWindowLong(m_main, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(m_main, 0, 255 * 80 / 100, LWA_ALPHA);
}

void bouncy::Draw_trail(HDC hdc)
{
  //guidelines from https://learn.microsoft.com/en-us/windows/win32/gdi/using-brushes
    COLORREF colour_ofball = RGB(255, 0, 0); // red 
    HBRUSH colour_of_brush = CreateSolidBrush(colour_ofball);
    SelectObject(hdc, colour_of_brush);
    HPEN pen_ball = CreatePen(PS_SOLID, 1, colour_ofball);
    HPEN pen_bal_old_version = (HPEN)SelectObject(hdc, pen_ball);
    Ellipse(hdc, x_ball - r_ball, y_ball - r_ball, x_ball + r_ball, y_ball + r_ball);

    int x = x_ball - (ball_speed_x * 3); 
    int y = y_ball - (ball_speed_y * 3);
    for (int i = number_of_trails; i >= 1; i--)
    {
       
        int radius = radius_of_trail * i / number_of_trails;
        int opacity = trail_transparent * i / number_of_trails;
        COLORREF trailColor = RGB(255, 0, 0);
        trailColor = trailColor & 0x00FFFFFF; 
        HBRUSH trailBrush = CreateSolidBrush(trailColor);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, trailBrush);
        Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
        SelectObject(hdc, oldBrush);
        DeleteObject(trailBrush);
        x -= ball_speed_x * 2;
        y -= ball_speed_y * 2;
    }
    SelectObject(hdc, pen_bal_old_version);
    DeleteObject(colour_of_brush);
    DeleteObject(pen_ball);

}

void bouncy::LoadAndDisplayBitmap(HWND hwnd, LPCWSTR filename)
{
// SOURCE: https://learn.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-loading-and-displaying-bitmaps-use
    HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hBitmap == NULL)
    {
        return;
    }
    RECT rc;
    GetClientRect(window, &rc);
    HDC hdc = GetDC(hwnd);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBitmap);
   // SetStretchBltMode(hdc, HALFTONE);
   // StretchBlt(hdc, 0, 0, rc.right, rc.bottom, hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
    DeleteDC(memDC);
    ReleaseDC(hwnd, hdc);
    DeleteObject(hBitmap);
}

//void bouncy::UpdateBitmap(HWND window)
//{
//    RECT rc;
//    HDC hdc = GetDC(window);
//    GetClientRect(window, &rc);
//    if (isBitmap)
//    {
//        if (isStretched)
//        {
//            BITMAP bmp;
//            HDC hTmpDC = CreateCompatibleDC(hdc);
//          //  BITMAP bmpInfo;
//            HBITMAP hOldTmpBmp = (HBITMAP)SelectObject(hTmpDC, hBmp);
//            //From tutorials
//            BitBlt(hdc, rc.right, rc.bottom, child_w, child_h, hTmpDC, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
// 
//            GetObject(hBmp, sizeof(BITMAP), &bmp);
//            SetStretchBltMode(hdc, HALFTONE);
//            StretchBlt(hdc, 0, 0, rc.right, rc.bottom, hTmpDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
//
//            SelectObject(hTmpDC, hOldTmpBmp);
//            DeleteDC(hTmpDC);
//            ReleaseDC(window, hTmpDC);
//        }
//        else
//        {
//            HBRUSH newBrush = CreatePatternBrush(hBmp);
//            FillRect(hdc, &rc, newBrush);
//            DeleteBrush(newBrush);
//        }
//    }
//    else
//    {
//        HBRUSH newBrush = CreateSolidBrush(COLOR_BACKGROUND);
//        FillRect(hdc, &rc, newBrush);
//        DeleteBrush(newBrush);
//    }
//
//    SetBkMode(hdc, TRANSPARENT);
//    HFONT hFont = CreateFont(child_h, 0, 0, 0, 0, false, FALSE, 0, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Verdana");
//    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
//   // wchar_t wWinCount[POINT_DIGITS + 1];
//   // _itow_s(winCount, wWinCount, 10);
//    SetTextColor(hdc, COLOR_BACKGROUND ^ 0x00FFFFFF);
//   // DrawText(hMemDC, wWinCount, -1, &winRc, DT_CENTER);
//
//    SelectObject(hdc, hOldFont);
//    DeleteFont(hFont);
//
//    InvalidateRect(window, NULL, true);
//}

//CODE FROM TUTORIAL
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return static_cast <INT_PTR>(TRUE);

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL)
        {
           
            EndDialog(hDlg, LOWORD(wParam));
            return static_cast <INT_PTR>(TRUE);
        }
        break;

    }
    return static_cast <INT_PTR>(FALSE);
  
}








