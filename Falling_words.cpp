#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <ncurses.h>
#include <cmath>

using namespace std;

struct Particle {
    char symbol;
    float x, y;
    float vx, vy;
    int startDelay;
    bool active;
    bool stopped;  // флаг, что частица полностью остановилась
};

int main() {
    srand(time(0));
    setlocale(LC_ALL, "");
    
    // Чтение файла
    ifstream file("input.txt");
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл input.txt" << endl;
        return 1;
    }
    
    string content;
    getline(file, content, '\0');
    file.close();
    
    if (content.empty()) {
        cerr << "Файл пуст" << endl;
        return 1;
    }
    
    // Инициализация ncurses
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);
    keypad(stdscr, TRUE);
    
    int screenHeight, screenWidth;
    getmaxyx(stdscr, screenHeight, screenWidth);
    screenHeight -= 1;
    
    // Параметры
    float baseSpeedY = 2.5f;
    float speedVariation = 2.0f;
    int startDelayMax = 50;
    
    // Создание частиц
    vector<Particle> particles;
    for (char ch : content) {
        if (ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') continue;
        
        Particle p;
        p.symbol = ch;
        p.x = screenWidth - 1;
        p.y = 0;
        p.vx = -((rand() % 15) / 10.0f + 0.3f);
        p.vy = baseSpeedY + (rand() % (int)(speedVariation * 10)) / 10.0f;
        p.startDelay = rand() % startDelayMax;
        p.active = false;
        p.stopped = false;
        particles.push_back(p);
    }
    
    if (particles.empty()) {
        endwin();
        cerr << "Нет символов для отображения" << endl;
        return 1;
    }
    
    // Основной цикл анимации
    int frame = 0;
    bool running = true;
    
    while (running) {
        int ch = getch();
        if (ch == 27 || ch == 'q' || ch == 'Q') break;
        
        erase();
        
        // Обновляем частицы
        for (auto& p : particles) {
            // Активация после задержки
            if (!p.active && !p.stopped && frame >= p.startDelay) {
                p.active = true;
            }
            
            if (p.active && !p.stopped) {
                // Обновление координат
                p.x += p.vx;
                p.y += p.vy;
                
                // Неупругое соударение с нижним краем
                if (p.y >= screenHeight - 1) {
                    p.y = screenHeight - 1;
                    p.vy = -p.vy * 0.55f;
                    p.vx *= 0.95f;
                    
                    // Если скорость стала очень маленькой - частица остановилась
                    if (fabs(p.vy) < 0.2f) {
                        p.vy = 0;
                        p.stopped = true;  // Помечаем как остановленную
                        p.active = false;
                        continue;  // Пропускаем отрисовку остановленной частицы
                    }
                }
                
                // Отскок от верхнего края
                if (p.y <= 0 && p.vy < 0) {
                    p.y = 0;
                    p.vy = -p.vy * 0.5f;
                }
                
                // Горизонтальные отскоки
                if (p.x <= 0) {
                    p.x = 0;
                    p.vx = -p.vx * 0.7f;
                }
                if (p.x >= screenWidth - 1) {
                    p.x = screenWidth - 1;
                    p.vx = -p.vx * 0.7f;
                }
                
                // Ограничение координат
                if (p.x < 0) p.x = 0;
                if (p.x >= screenWidth) p.x = screenWidth - 1;
                if (p.y < 0) p.y = 0;
                if (p.y >= screenHeight) p.y = screenHeight - 1;
                
                // Отрисовка активной частицы
                mvaddch((int)p.y, (int)p.x, p.symbol);
            }
        }
        
        // Проверяем, все ли частицы остановились
        bool allStopped = true;
        for (auto& p : particles) {
            if (!p.stopped) {
                allStopped = false;
                break;
            }
        }
        
        // Если все остановились - перезапускаем анимацию
        if (allStopped) {
            refresh();
            usleep(500000);  // Пауза 0.5 секунды
            
            // Сбрасываем все частицы
            for (auto& p : particles) {
                p.x = screenWidth - 1;
                p.y = 0;
                p.vx = -((rand() % 15) / 10.0f + 0.3f);
                p.vy = baseSpeedY + (rand() % (int)(speedVariation * 10)) / 10.0f;
                p.startDelay = rand() % startDelayMax;
                p.active = false;
                p.stopped = false;
            }
            frame = 0;
        }
        
        refresh();
        usleep(50000);
        frame++;
    }
    
    curs_set(1);
    endwin();
    return 0;
}
