#include <iostream>
#include <windows.h>
#include <conio.h>

using namespace std;

const int WIDTH = 40;
const int HEIGHT = 20;
const int PADDLE_SIZE = 4;

struct GameState {
    int ball_x;
    int ball_y;
    int ball_dx;
    int ball_dy;
    int paddle1_y;
    int paddle2_y;
    int score1;
    int score2;
} game;

void setupConsole() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND consoleWindow = GetConsoleWindow();

    COORD bufferSize = { 80, 25 };
    SMALL_RECT windowSize = { 0, 0, 79, 24 };

    SetConsoleScreenBufferSize(consoleHandle, bufferSize);
    SetConsoleWindowInfo(consoleHandle, TRUE, &windowSize);

    RECT rectWindow;
    GetWindowRect(consoleWindow, &rectWindow);
    int width = rectWindow.right - rectWindow.left;
    int height = rectWindow.bottom - rectWindow.top;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    SetWindowPos(
        consoleWindow,
        0,
        (screenWidth - width) / 2,
        (screenHeight - height) / 2,
        width,
        height,
        SWP_NOZORDER | SWP_NOSIZE
    );

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

void gotoxy(int x, int y) {
    COORD pos = { (SHORT)(x), (SHORT)(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void initGame() {
    __asm {
        mov eax, WIDTH
        shr eax, 1
        mov game.ball_x, eax

        mov eax, HEIGHT
        shr eax, 1
        mov game.ball_y, eax

        mov game.ball_dx, 1
        mov game.ball_dy, 1

        mov eax, HEIGHT
        sub eax, PADDLE_SIZE
        shr eax, 1
        mov game.paddle1_y, eax
        mov game.paddle2_y, eax

        mov game.score1, 0
        mov game.score2, 0
    }
}

void moveBall() {
    __asm {
        mov eax, game.ball_x
        add eax, game.ball_dx
        mov game.ball_x, eax

        mov eax, game.ball_y
        add eax, game.ball_dy
        mov game.ball_y, eax

        cmp eax, 0
        jg check_bottom
        neg game.ball_dy
        jmp check_paddles

        check_bottom :
        cmp eax, HEIGHT
            jl check_paddles
            neg game.ball_dy

            check_paddles :

        mov eax, game.ball_x
            cmp eax, 1
            jne check_right_paddle

            mov ebx, game.ball_y
            mov ecx, game.paddle1_y
            cmp ebx, ecx
            jl miss_paddle

            add ecx, PADDLE_SIZE
            cmp ebx, ecx
            jg miss_paddle

            neg game.ball_dx
            jmp done_collision

            check_right_paddle :
        mov eax, game.ball_x
            cmp eax, WIDTH - 2
            jne check_scoring

            mov ebx, game.ball_y
            mov ecx, game.paddle2_y
            cmp ebx, ecx
            jl miss_paddle

            add ecx, PADDLE_SIZE
            cmp ebx, ecx
            jg miss_paddle

            neg game.ball_dx
            jmp done_collision

            miss_paddle :

    check_scoring:
        mov eax, game.ball_x
            cmp eax, 0
            jg check_right_score
            inc game.score2
            jmp do_reset_ball

            check_right_score :
        cmp eax, WIDTH
            jl done_collision
            inc game.score1

            do_reset_ball :
        mov eax, WIDTH
            shr eax, 1
            mov game.ball_x, eax

            mov eax, HEIGHT
            shr eax, 1
            mov game.ball_y, eax

            mov eax, game.ball_dx
            neg eax
            mov game.ball_dx, eax

            done_collision :
    }
}

void movePaddles() {
    if (_kbhit()) {
        char key = _getch();
        __asm {

            cmp key, 75  
            jne check_right
            mov eax, game.paddle1_y
            cmp eax, 1
            jle skip_move
            dec game.paddle1_y
            jmp skip_move

            check_right :
            cmp key, 77  
                jne check_up
                mov eax, game.paddle1_y
                add eax, PADDLE_SIZE
                cmp eax, HEIGHT
                jge skip_move
                inc game.paddle1_y
                jmp skip_move

                check_up :
            cmp key, 72
                jne check_down
                mov eax, game.paddle2_y
                cmp eax, 1
                jle skip_move
                dec game.paddle2_y
                jmp skip_move

                check_down :
            cmp key, 80
                jne skip_move
                mov eax, game.paddle2_y
                add eax, PADDLE_SIZE
                cmp eax, HEIGHT
                jge skip_move
                inc game.paddle2_y

                skip_move :
        }
    }
}

void drawGame() {

    for (int y = 0; y < HEIGHT + 2; y++) {
        gotoxy((80 - WIDTH) / 2 - 1, (25 - HEIGHT) / 2 + y - 1);
        if (y == 0 || y == HEIGHT + 1) {
            cout << string(WIDTH + 2, '=');
        }
        else {
            cout << "|" << string(WIDTH, ' ') << "|";
        }
    }


    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            bool should_print_char = false;
            char char_to_print = ' ';

            __asm {
                mov eax, x
                cmp eax, game.ball_x
                jne check_paddles
                mov eax, y
                cmp eax, game.ball_y
                jne check_paddles
                mov should_print_char, 1
                mov char_to_print, 'O'
                jmp print_decision

                check_paddles :
                cmp x, 0
                    jne check_right_paddle
                    mov eax, y
                    sub eax, game.paddle1_y
                    cmp eax, 0
                    jl print_decision
                    cmp eax, PADDLE_SIZE
                    jge print_decision
                    mov should_print_char, 1
                    mov char_to_print, '|'
                    jmp print_decision

                    check_right_paddle :
                mov eax, WIDTH
                    dec eax
                    cmp x, eax
                    jne print_decision
                    mov eax, y
                    sub eax, game.paddle2_y
                    cmp eax, 0
                    jl print_decision
                    cmp eax, PADDLE_SIZE
                    jge print_decision
                    mov should_print_char, 1
                    mov char_to_print, '|'

                    print_decision:
            }

            if (should_print_char) {
                gotoxy((80 - WIDTH) / 2 + x, (25 - HEIGHT) / 2 + y);
                cout << char_to_print;
            }
        }
    }


    gotoxy((80 - WIDTH) / 2, (25 - HEIGHT) / 2 + HEIGHT + 1);
    cout << "Wynik: " << game.score1 << " - " << game.score2;
}

int main() {
    setupConsole();
    initGame();

    bool gameOver = false;
    while (!gameOver) {
        moveBall();
        movePaddles();
        drawGame();
        Sleep(100);

        if (game.score1 >= 5 || game.score2 >= 5) {
            gotoxy((80 - WIDTH) / 2, (25 - HEIGHT) / 2 + HEIGHT + 2);
            if (game.score1 >= 5) {
                cout << "Gracz LEWY wygral!";
            }
            else {
                cout << "Gracz PRAWY wygral!";
            }
            gameOver = true;
            Sleep(2000); 
        }

        if (_kbhit() && _getch() == 'q') break;
    }

    return 0;
}