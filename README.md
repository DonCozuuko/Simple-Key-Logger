# Key Logger
My own simple remote key logger written in C. Uses winsock2 socket library for socket communication and windows.h for fetching current focused foreground window, so it only builds on Windows :( 

## Features
- Catches all keystrokes, including those on the shift layer.
- Displays keystrokes in real-time.
- Displays the current foreground window as a header to the victim's keystrokes. The thought was that a person's keystrokes are only as valuable as the context of the current window they are typing in.
- Color coding.

## Demo

https://github.com/user-attachments/assets/aaf67876-dd61-4cf1-8482-60f5d314d0d3

## Usage
1. Run server.exe on your local machine.
2. Somehow, whether it be through explicit means, get ggs.exe on the machine you want to listen on, and run it.
3. *OPTIONAL*, but if you are a windows machine, which that chances are pretty high, then you can add ggs.exe to run on startup by copying it to the startup directory:
```
C:\Users\J1R\AppData\Roaming\Microsoft\Windows\Start Menu\Programs
```
4. Sit back and embrace being ~~a script kiddie~~ an FBI-wanted Hacker Man 😎 .

## Build from Source
```
$ make
```
