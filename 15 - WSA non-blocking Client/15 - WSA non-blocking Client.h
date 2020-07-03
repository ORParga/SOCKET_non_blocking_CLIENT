#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include "resource.h"

#pragma comment (lib,"Ws2_32.lib")

#define TIME_OUT_FOR_EVENTS 50 //50 miliseconds
#define DATA_BUFSIZE 512
class WSA_non_blocking_Client {
    //*************** STATUS **********************************************************
public: enum class STATE { NONE, CONNECTED, LISTENING, REQUESTING };
public:STATE state = STATE::NONE;
public:BOOL bConnected = FALSE;
    //*************** SOCKET *********************************************************
protected: SOCKET ClientSocket = INVALID_SOCKET;
public:static const int IPString_Lenght = 50;
protected: WSADATA wsaData = { 0 };
protected: sockaddr_in  ClientAdress = { 0 };
protected: int ClientAdressLen;
public:      wchar_t IPString[IPString_Lenght] = { 0 };
public:      wchar_t PortString[IPString_Lenght] = { 0 };
      //****************** EVENTS *****************************************************
protected: DWORD EventTotal = 1;
protected: WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
//************************** SEND RECIEVE ********************************************

public: CHAR BufferRecieved[DATA_BUFSIZE + 1] = { 0 };
public:int ReceivedBytes = { 0 };
public: BOOL OverflowAlert = { FALSE };
//***************************** VARIOS *****************************************
public:int TimeOutForEvents = TIME_OUT_FOR_EVENTS;
public:int lastWSAError = 0;
protected: int iResult = 0;
protected: static const int lpBufferWindowsErrorLen = 1000;
protected: wchar_t lpBufferWindowsError[lpBufferWindowsErrorLen];

/// <summary>
/// Muestra un mensaje en la ventana del depurador
/// </summary>
/// <param name="lpszText">Texto a mostrar</param>
void XTrace0(LPCTSTR lpszText)
{
    ::OutputDebugString(lpszText);
}

/// <summary>
/// Muestra un mensaje en la ventana del depurador compatible con la sintaxis printf()
/// </summary>
/// <param name="lpszFormat">Texto a mostrar</param>
/// <param name="">Datos a insertar en el texto</param>
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

/// <summary>
/// Inicializa la maquinaria WinSock2 de windows.
/// Inicializa el socket cliente.
/// Inicializa el evento FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE.
/// No altera el STATUS del objeto WSA_non_blocking_Client
/// </summary>
/// <returns>TRUE si el socket está inicializado y enlazado con el Evento.
/// FALSE si algo ha fallado. lastWSAError guarda el ultimo error generado</returns>
public: int CreateClientSocket() {

    lastWSAError = 0;
    // socket() data *******************************************************************
    int iFamily = AF_INET;
    int iType = SOCK_STREAM;
    int iProtocol = IPPROTO_TCP;
 // Initialize Winsock*****************************************************************
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        lastWSAError = WSAGetLastError();
        XTrace(L"WSAStartup failed, returned= %d. lastWSAError=%u\n", iResult, lastWSAError);
        return FALSE;
    }
    // Create socket****************************************************************
    ClientSocket = socket(iFamily, iType, iProtocol);
    if (ClientSocket == INVALID_SOCKET)
    {
        lastWSAError = WSAGetLastError();
        XTrace(L"socket function failed with error = %d\n",lastWSAError );
        return FALSE;
    }
    // Associate event types  FD_READ|FD_WRITE| FD_CONNECT |FD_CLOSE*****************************************
        // with the listening socket and NewEvent
        // Create new event
    EventArray[0] = WSACreateEvent();
    iResult = WSAEventSelect(ClientSocket, EventArray[0], FD_READ|FD_WRITE | FD_CONNECT | FD_CLOSE);
    if (iResult == SOCKET_ERROR) {
        lastWSAError = WSAGetLastError();
        XTrace(L"WSAEventSelect failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        closesocket(ClientSocket);
        WSACleanup();
        return FALSE;
    }
    return TRUE;
}
/// <summary>
/// Inicia una conexion con el SERVIDOR en la IP y puertos especificados.
/// Si no consigue conectar, pero la orden de conectar se ejecuta satisfactoriamente,
/// coloca el STATUS en "REQUESTING" y WSA_non_blocking_Client entra en espera 
/// del disparo de un evento "FD_CONNECT". El metodo regresa inmediatamente ,
/// NO bloquea la aplicacion, pero la aplicacion debe
/// llamar a WSAnb_Client.Attemp_connect() para volver a intentarlo y a 
/// WSAnb_Client.testForEvents() regularmente ( con un WM_TIMER, por ejemplo)
/// para reaccionar al evento FD_CONNECT que se producirá cuando el Servidor acepte la 
/// conexion solicitada.
/// Attemp_connect() modifica STATUS- CONNECTED si se ha conectado a la primera o NONE
/// si se ha rechazado por tiempo para no bloquear la aplicacion
/// </summary>
/// <param name="IPString">IP v4 del servidor al que se quiere conectar</param>
/// <param name="port">Puerto de la IP al que se quiere conectar</param>
/// <returns>TRUE si se ha conectado o si se ha rechazado para no bloquear. FALSE si algo a fallado. 
/// lastWSAError guarda el último valor de WSAGetLastError. entre los posibles valores de lastWSAError,
/// WSAEWOULDBLOCK indica que se ha cancelado el intento de conexion para no bloquear </returns>
public: BOOL Attemp_connect(wchar_t* IPString, int port) {
    lastWSAError = 0;
    if (bConnected)return TRUE;
    IN_ADDR in_addr = { 0 };
    //Guarda el string IP u el Puerto recibidos
    //actualiza el estado de la clase
    wcscpy_s(this->IPString, this->IPString_Lenght, IPString);
    _itow_s(port, this->PortString, 10);
    if (InetPton(AF_INET, IPString, &in_addr) != 1) {
        lastWSAError = WSAGetLastError();
        XTrace(L"InetPton error %u\n", lastWSAError);
        closesocket(ClientSocket);
        WSACloseEvent(EventArray[0]);
        WSACleanup();
        return FALSE;

    }
    ClientAdressLen = sizeof(ClientAdress);
    ClientAdress.sin_family = AF_INET;
    ClientAdress.sin_addr = in_addr;
    ClientAdress.sin_port = htons(port);
    iResult = connect(ClientSocket, (SOCKADDR*)&ClientAdress, ClientAdressLen);
    if (iResult == SOCKET_ERROR) {
        lastWSAError = WSAGetLastError();
        XTrace(L"connect failed with error %u : %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        switch (lastWSAError)
        {
        case WSAEWOULDBLOCK:
            bConnected = FALSE;
            state = STATE::REQUESTING;
            return TRUE;
        default:
            break;
        }
        return FALSE;
    }
    bConnected = TRUE;
    state = STATE::CONNECTED;
    XTrace(L"Connected to server.\n");
    return TRUE;

}
/// <summary>
/// Evalua los Eventos registrados y reacciona apropiadamente.
/// FD_CONNECT (conexion rechazada para no bloquear). coloca STATUS en REQUESTING
/// FD_CONNECT (conexion aceptada por el servidor). coloca STATUS en CONNECTED y bConected=true,
///permitiendo la transmision de datos.
/// FD_CLOSE Si la conexion se cierra. Destruye el socket cliente y crea uno nuevo para ponerlo
/// nuevamente en modo "escucha". Actualiza el STATUS a NONE y bConnected a false;
/// </summary>
/// <returns>Devuelve cero si ha funconado, SOCKET_ERROR si ha fallado</returns>
public:int testForEvents() {

    lastWSAError = 0;
    WSANETWORKEVENTS NetworkEvents = { 0 };
    // Wait for one of the sockets to receive I/O notification and
    DWORD Event = WSAWaitForMultipleEvents(
        EventTotal,             //The number of event object handles in the array pointed to by lphEvents. 
        EventArray,             //A pointer to an array of event object handles.              
        FALSE,                  // If FALSE, the function returns when any of the event objects is signaled.
        TimeOutForEvents,       //The time-out interval, in milliseconds.
        FALSE                   //If FALSE, the thread is not placed in an alertable wait state and I/O completion routines are not executed.
    );
    switch (Event)
    {
    case WSA_WAIT_FAILED:
    {
        lastWSAError = WSAGetLastError();
        XTrace(L"WSAWaitForMultipleEvents() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        return SOCKET_ERROR;
    }
    case WSA_WAIT_IO_COMPLETION:
        XTrace(L"WSAWaitForMultipleEvents() WSA_WAIT_IO_COMPLETION\n");
        return 0;
    case WSA_WAIT_TIMEOUT:
        XTrace(L"WSAWaitForMultipleEvents() WSA_WAIT_TIMEOUT\n");
        return 0;
    default:
        break;
    }
    XTrace(L"WSAWaitForMultipleEvents() is pretty damn OK!\n");
    iResult = WSAEnumNetworkEvents(
        ClientSocket,          //A descriptor identifying the socket.
        EventArray[Event - WSA_WAIT_EVENT_0],           //An optional handle identifying an associated event object to be reset.
        &NetworkEvents);                                //A structure that is filled with a record of network events that occurred and any associated error codes.
    if (iResult == SOCKET_ERROR)
    {
        lastWSAError = WSAGetLastError();
        XTrace(L"WSAEnumNetworkEvents() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        return SOCKET_ERROR;
    }
    if (NetworkEvents.lNetworkEvents == 0) {
        XTrace(L"WSAEnumNetworkEvents() se ha disparado. Pero el evento no está definido.\n");
           return 0;
    }
    if (NetworkEvents.lNetworkEvents & FD_ACCEPT) {
        XTrace(L"FD_ACCEPT\n");
        return 0;
    }
    if (NetworkEvents.lNetworkEvents & FD_CLOSE) {
        XTrace(L"FD_CLOSE\n");
        closesocket(ClientSocket);
        WSACloseEvent(EventArray[0]);
        CreateClientSocket();
        bConnected = FALSE;
        state = STATE::NONE;
        return 0;
    }
    if (NetworkEvents.lNetworkEvents & FD_CONNECT) {
        if (NetworkEvents.iErrorCode[FD_CONNECT_BIT] != 0) {
            XTrace(L"FD_CONNECT conexion rechazada\n");
            bConnected = FALSE;
            state = STATE::NONE;
            return 0;
        }
        else
        {
            XTrace(L"FD_CONNECT\n");
            bConnected = TRUE;
            state = STATE::CONNECTED;
            return 0;
        }
    }
    if (NetworkEvents.lNetworkEvents & FD_READ) {
        if (NetworkEvents.iErrorCode[FD_READ_BIT] != 0) {
            XTrace(L"FD_READ ha devuelto error %u:\n", NetworkEvents.iErrorCode[FD_READ_BIT]);
            return 0;
        }
        else {
            XTrace(L"FD_READ\n");
            FD_READ_response();
            return 0;
        }
    }
    if (NetworkEvents.lNetworkEvents & FD_WRITE) {
        if (NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0) {
            XTrace(L"FD_WRITE ha devuelto error %u:\n", NetworkEvents.iErrorCode[FD_WRITE_BIT]);
            return 0;
        }
        else {
            XTrace(L"FD_WRITE\n");
            return 0;
        }
    }
    return 0;
}

protected:int FD_READ_response() {
    ReceivedBytes = recv(ClientSocket, BufferRecieved, DATA_BUFSIZE, 0);
    if (ReceivedBytes == SOCKET_ERROR) {

        lastWSAError = WSAGetLastError();
        XTrace(L"recv() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        return SOCKET_ERROR;
    }
    if (ReceivedBytes >= DATA_BUFSIZE)
    {
        OverflowAlert= TRUE;
    }
    else {
        
        OverflowAlert = FALSE;
    }
    //El tamaño del buffer es DATA_BUFSIZE+1 para poder colocar un cero al final
    BufferRecieved[ReceivedBytes] = 0;
    return 0;
}
/// <summary>
/// Devuelve un puntero a una cadena con el Error de Windos traducido para ser leido por un humano
/// </summary>
/// <param name="ErrorCode">Codigo de error de windows</param>
/// <returns>Cadena con el texto en humano</returns>
public: wchar_t* WindowsErrorToString(int ErrorCode)
{

    if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        ErrorCode,
        0,
        lpBufferWindowsError,
        lpBufferWindowsErrorLen,
        NULL) == 0)
    {
        XTrace(L"Error with FormatMessage\n");
    }
    return lpBufferWindowsError;
}

public: BOOL SendText(char* text,int len) {
    int bytesSend = 0;
    if (bConnected) {
        bytesSend=send(ClientSocket, text, len, 0);
        if (bytesSend == SOCKET_ERROR) {
            lastWSAError = WSAGetLastError();
            XTrace(L"Error al enviar texto. Codigo: %u = %s", lastWSAError, WindowsErrorToString(lastWSAError));
            return FALSE;
        }
        //XTrace(L"Texto enviado:\"%s\". %u",text,bytesSend);
    }
}
};