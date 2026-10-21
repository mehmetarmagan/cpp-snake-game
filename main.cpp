#include <windows.h>
#include <vector>
#include <fstream>
#include <string>

#define GRID_SIZE 20
#define CELL_SIZE 20

struct Point {
    int x, y;
};

std::vector<Point> snake;
Point food;
Point dir = {1, 0};
int score = 0;
int highScore = 0;
bool gameOver = false;

void LoadHighScore() {
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
}

void SaveHighScore() {
    if (score > highScore) {
        highScore = score;
        std::ofstream file("highscore.txt");
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }
}

void SpawnFood() {
    food.x = rand() % GRID_SIZE;
    food.y = rand() % GRID_SIZE;
}

void InitGame() {
    snake.clear();
    snake.push_back({GRID_SIZE / 2, GRID_SIZE / 2});
    dir = {1, 0};
    score = 0;
    gameOver = false;
    LoadHighScore();
    SpawnFood();
}

void UpdateGame() {
    if (gameOver) return;

    Point newHead = {snake[0].x + dir.x, snake[0].y + dir.y};

    // Duvara çarpma kontrolü
    if (newHead.x < 0 || newHead.x >= GRID_SIZE || newHead.y < 0 || newHead.y >= GRID_SIZE) {
        gameOver = true;
        SaveHighScore();
        return;
    }

    // Kendine çarpma kontrolü
    for (const auto& segment : snake) {
        if (segment.x == newHead.x && segment.y == newHead.y) {
            gameOver = true;
            SaveHighScore();
            return;
        }
    }

    snake.insert(snake.begin(), newHead);

    // Yem yeme
    if (newHead.x == food.x && newHead.y == food.y) {
        score += 10;
        SpawnFood();
    } else {
        snake.pop_back();
    }
}

void Render(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // Double Buffering
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE + 40);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Arka planı temizle
    HBRUSH bgBrush = CreateSolidBrush(RGB(30, 30, 30));
    RECT rect = {0, 0, GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE + 40};
    FillRect(memDC, &rect, bgBrush);
    DeleteObject(bgBrush);

    // Yılanı çiz
    HBRUSH snakeBrush = CreateSolidBrush(RGB(0, 255, 100));
    for (const auto& segment : snake) {
        RECT r = {segment.x * CELL_SIZE, segment.y * CELL_SIZE, (segment.x + 1) * CELL_SIZE, (segment.y + 1) * CELL_SIZE};
        FillRect(memDC, &r, snakeBrush);
    }
    DeleteObject(snakeBrush);

    // Yemi çiz
    HBRUSH foodBrush = CreateSolidBrush(RGB(255, 50, 50));
    RECT fRect = {food.x * CELL_SIZE, food.y * CELL_SIZE, (food.x + 1) * CELL_SIZE, (food.y + 1) * CELL_SIZE};
    FillRect(memDC, &fRect, foodBrush);
    DeleteObject(foodBrush);

    // Skor Tablosu
    SetTextColor(memDC, RGB(255, 255, 255));
    SetBkMode(memDC, TRANSPARENT);
    std::string scoreText = "Skor: " + std::to_string(score) + " | En Yuksek: " + std::to_string(highScore);
    if (gameOver) scoreText += " - OYUN BITTI! (Yeniden Baslatmak Icin R'ye Basin)";
    TextOutA(memDC, 10, GRID_SIZE * CELL_SIZE + 10, scoreText.c_str(), scoreText.length());

    // Ekranı güncelle
    BitBlt(hdc, 0, 0, GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE + 40, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            InitGame();
            SetTimer(hwnd, 1, 120, NULL);
            break;
        case WM_TIMER:
            UpdateGame();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_KEYDOWN:
            if (wParam == VK_UP && dir.y == 0) dir = {0, -1};
            else if (wParam == VK_DOWN && dir.y == 0) dir = {0, 1};
            else if (wParam == VK_LEFT && dir.x == 0) dir = {-1, 0};
            else if (wParam == VK_RIGHT && dir.x == 0) dir = {1, 0};
            else if (wParam == 'R' && gameOver) InitGame();
            break;
        case WM_PAINT:
            Render(hwnd);
            break;
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "SnakeGameClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow("SnakeGameClass", "C++ Win32 Snake Game", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT, GRID_SIZE * CELL_SIZE + 16, GRID_SIZE * CELL_SIZE + 79,
                             NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
