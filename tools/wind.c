#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <unistd.h>

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

void handleFocusedWindow() {
    char titleBuff[256];
    HWND hwnd = GetForegroundWindow();
    int length = GetWindowText(hwnd, titleBuff, sizeof(titleBuff));
    printf("Current window - %s\n", titleBuff);
}

int main() {
    while (1) {
        handleFocusedWindow();
        
        ExeArr exeArr = {0};  // zero-initialize count and the array
        // Make copy exeList to compare with next iteration
        char lastList[MAX_EXES][MAX_PATH];
        // for (int i = 0; i < exeArr.exeCount; i++) {
        //     printf("ACTIVATED");
        // }
        EnumWindows(EnumWindowsProc, (LPARAM)&exeArr);  // have to cast exeArr type into LPARAM type
        checkIfWindowsChanged(&exeArr, lastList);
        // for (int i = 0; i < exeArr.exeCount; i++) {
        //     printf("EXE: %s\n", exeArr.exeList[i]);
        // }
        printf("\n\n");
        sleep(2);
    }
    return 0;
}
