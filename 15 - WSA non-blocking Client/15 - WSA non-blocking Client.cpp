// 15 - WSA non-blocking Client.cpp : Define el punto de entrada de la aplicación.
//

#include "framework.h"
#ifndef WM_SOCKET
#define WM_SOCKET WM_USER + 1
#endif // !WM_SOCKET
#ifndef IDT_TIMER1
#define IDT_TIMER1 1
#endif // !IDT_TIMER1
#ifndef IDT_TIMER1_MILIS
#define IDT_TIMER1_MILIS 500
#endif
#include "15 - WSA non-blocking Client.h"

#define MAX_LOADSTRING 100
#define IDC_MAINFRM_BTN_1               40501
// Variables globales:
HINSTANCE hInst;                                // instancia actual
WCHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal
//Manejadores de controles comunes
HWND hwndButton, hwndEdit, hwndStatic, hwndStatic2, hwndMesageRec;
RECT StaticRect1 = { 10,90,80,20 };
RECT StaticRect2 = { 100,90,100,20 };
RECT StaticRect3 = { 10,120,80,20 };
RECT EditRect = { 100,120,0,0 };
RECT ButtonRect = { 100,150,100,20 };
int ID_SendButton = IDC_MAINFRM_BTN_1;
//WSA_NON_BLOKING inicializacion y declaracion
WSA_non_blocking_Client WSAnb_Client;

wchar_t IPString[] = L"127.0.0.1";
int portNumber = 27015;
// Declaraciones de funciones
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
int Ini_WSA_non_blocking_client(HWND);
void XTrace(LPCTSTR , ...);
void XTrace0(LPCTSTR );
void ShowIPandPORT(HDC , RECT* );
int Ini_UI(HWND);
void ShowMessageControls(BOOL );

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Colocar código aquí.

    // Inicializar cadenas globales
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY15WSANONBLOCKINGCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realizar la inicialización de la aplicación:
    HWND hwnd = InitInstance (hInstance, nCmdShow);
    if (!hwnd) return FALSE;

    if (!Ini_WSA_non_blocking_client(hwnd))return FALSE;
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY15WSANONBLOCKINGCLIENT));

    MSG msg;

    // Bucle principal de mensajes:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

int Ini_WSA_non_blocking_client(HWND hwnd) {
    if(WSAnb_Client.CreateClientSocket() == FALSE)return FALSE;
    SetTimer(hwnd,             // handle to main window 
        IDT_TIMER1,            // timer identifier 
        IDT_TIMER1_MILIS,     // interval 
        (TIMERPROC)NULL);     // no timer callback 

    if (!WSAnb_Client.bConnected)
    {

        //wchar_t IPString[] = L"192.168.137.1";
        WSAnb_Client.Attemp_connect(IPString, portNumber);
    }
    return TRUE;
}
int Ini_UI(HWND hwnd) {
    hwndStatic = CreateWindowEx(
        0, L"STATIC",   // predefined class 
        L"Recieve:",         // no window title 
        WS_CHILD  |WS_VISIBLE| ES_LEFT,
        StaticRect1.left,         // x position 
        StaticRect1.top,         // y position 
        StaticRect1.right,        // Button width
        StaticRect1.bottom,        // Button height
        hwnd,         // parent window 
        NULL,           // No menu. 
        hInst,
        NULL);        // pointer not needed 

    hwndMesageRec = CreateWindowEx(
        0, L"STATIC",   // predefined class 
        L"No Message",         // no window title 
        WS_CHILD  | WS_BORDER | ES_LEFT,
        StaticRect2.left,         // x position 
        StaticRect2.top,         // y position 
        StaticRect2.right,        // Button width
        StaticRect2.bottom,        // Button height
        hwnd,         // parent window 
        NULL,           // No menu. 
        hInst,
        NULL);        // pointer not needed 

    hwndStatic2 = CreateWindowEx(
        0, L"STATIC",   // predefined class 
        L"Send:",         // no window title 
        WS_CHILD  | ES_LEFT,
        StaticRect3.left,         // x position 
        StaticRect3.top,         // y position 
        StaticRect3.right,        // Button width
        StaticRect3.bottom,        // Button height
        hwnd,         // parent window 
        NULL,           // No menu. 
        hInst,
        NULL);        // pointer not needed 

    hwndButton = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"SEND",      // Button text 
        WS_TABSTOP  | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        ButtonRect.left,         // x position 
        ButtonRect.top,         // y position 
        ButtonRect.right,        // Button width
        ButtonRect.bottom,        // Button height
        hwnd,     // Parent window
        (HMENU)IDC_MAINFRM_BTN_1,       // For buttons, hMenu is used to send the WM_BUTTON identifier
        hInst,
        NULL);      // Pointer to identify the button in WndProc()

    hwndEdit = CreateWindowEx(
        0, L"EDIT",   // predefined class 
        NULL,         // no window title 
        WS_CHILD |WS_BORDER| ES_LEFT ,
        0, 0, 0, 0,   // set size in WM_SIZE message 
        hwnd,         // parent window 
        NULL,           // No menu. 
        hInst,
        NULL);        // pointer not needed 

    return TRUE;
}
//
//  FUNCIÓN: MyRegisterClass()
//
//  PROPÓSITO: Registra la clase de ventana.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY15WSANONBLOCKINGCLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY15WSANONBLOCKINGCLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCIÓN: InitInstance(HINSTANCE, int)
//
//   PROPÓSITO: Guarda el identificador de instancia y crea la ventana principal
//
//   COMENTARIOS:
//
//        En esta función, se guarda el identificador de instancia en una variable común y
//        se crea y muestra la ventana principal del programa.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Almacenar identificador de instancia en una variable global

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 300, 300, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return 0;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

//
//  FUNCIÓN: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PROPÓSITO: Procesa mensajes de la ventana principal.
//
//  WM_COMMAND  - procesar el menú de aplicaciones
//  WM_PAINT    - Pintar la ventana principal
//  WM_DESTROY  - publicar un mensaje de salida y volver
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
    {
        switch (wParam)
        {
        case IDT_TIMER1:
            XTrace(L"WM_TIMER\n");
            switch (WSAnb_Client.state)
            {
            case WSA_non_blocking_Client::STATE::NONE:
                //wchar_t IPString[] = L"192.168.137.1";
                WSAnb_Client.Attemp_connect(IPString, portNumber);
                break;
            case WSA_non_blocking_Client::STATE::CONNECTED:
                SetWindowTextA(hwndMesageRec, (LPCSTR)WSAnb_Client.BufferRecieved);
                break;
            default:
                break;
            }
             WSAnb_Client.testForEvents(); 

            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    case WM_CREATE :
    {
        Ini_UI(hWnd);
        // Add text to the window. 
        wchar_t text[] = L"Text for EditBox";
        SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)text);
        return 0;
    }
    case WM_COMMAND:
        {
        if (lParam== (LPARAM)hwndButton)
        {
            const int textSize = 1000;
            char text[textSize];
            if(GetWindowTextA(hwndEdit, text, textSize)!=0)
                WSAnb_Client.SendText(text,strlen(text));
            return 0;
        }
            int wmId = LOWORD(wParam);
            // Analizar las selecciones de menú:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                return 0;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                return 0;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_SETFOCUS:
        SetFocus(hwndEdit);
        return 0;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Agregar cualquier código de dibujo que use hDC aquí...
            RECT TextRect = { 10,10,300,100 };
            switch (WSAnb_Client.state)
            {
            case WSA_non_blocking_Client::STATE::NONE:
                DrawText(hdc, L"Estado:None", -1, &TextRect, DT_VCENTER | DT_LEFT);
                ShowMessageControls(FALSE);
                break;
            case WSA_non_blocking_Client::STATE::CONNECTED:
                DrawText(hdc, L"Estado:Conectado", -1, &TextRect, DT_VCENTER | DT_LEFT);
                ShowIPandPORT(hdc, &TextRect);
                ShowMessageControls(TRUE);
                break;
            case WSA_non_blocking_Client::STATE::REQUESTING:
                DrawText(hdc, L"Estado:Solicitando conexion", -1, &TextRect, DT_VCENTER | DT_LEFT);
                ShowIPandPORT(hdc, &TextRect);
                ShowMessageControls(FALSE);
                break;
            default:
                DrawText(hdc, L"Estado:Desconocido", -1, &TextRect, DT_VCENTER | DT_LEFT);
                ShowMessageControls(FALSE);
                break;
            }
            EndPaint(hWnd, &ps);
        }
        break; 
    case WM_SIZE:
    {
        // Make the edit control the size of the window's client area. 
        MoveWindow(hwndMesageRec,
            StaticRect2.left, StaticRect2.top,                    // starting x- and y-coordinates 
            LOWORD(lParam)- StaticRect2.left-20,                // width of client area 
            20,                // height of client area 
            TRUE);                 // repaint window 

        MoveWindow(hwndEdit,
            EditRect.left, EditRect.top,                    // starting x- and y-coordinates 
            LOWORD(lParam)-EditRect.left-20,                // width of client area 
            20,                // height of client area 
            TRUE);                 // repaint window 
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
void ShowIPandPORT(HDC hdc, RECT* TextRect) {
    TextRect->top += 20;
    TextRect->bottom += 20;
    DrawText(hdc, WSAnb_Client.IPString, -1, TextRect, DT_VCENTER | DT_LEFT);
    TextRect->top += 20;
    TextRect->bottom += 20;
    DrawText(hdc, WSAnb_Client.PortString, -1, TextRect, DT_VCENTER | DT_LEFT);
}
void ShowMessageControls(BOOL show) {
    ShowWindow(hwndStatic, show? SW_SHOW:SW_HIDE);
    ShowWindow(hwndStatic2, show ? SW_SHOW : SW_HIDE);
    ShowWindow(hwndMesageRec, show ? SW_SHOW : SW_HIDE);
    ShowWindow(hwndEdit, show ? SW_SHOW : SW_HIDE);
    ShowWindow(hwndButton, show ? SW_SHOW : SW_HIDE);
}
// Controlador de mensajes del cuadro Acerca de.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

/// <summary>
/// printf() style debugging
/// https://stackoverflow.com/questions/15240/
/// </summary>
/// <param name="lpszFormat">Debugging text</param>
void XTrace0(LPCTSTR lpszText)
{
    ::OutputDebugString(lpszText);
}

/// <summary>
/// printf() style debugging
/// https://stackoverflow.com/questions/15240/
/// </summary>
/// <param name="lpszFormat">-Debugging text</param>
/// <param name="">.... parameters in _vstprintf_s() style</param>
void XTrace(LPCTSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);
    int nBuf;
    TCHAR szBuffer[512]; // get rid of this hard-coded buffer
    nBuf = _vstprintf_s(szBuffer, 511, lpszFormat, args);
    ::OutputDebugString(szBuffer);
    va_end(args);
}
