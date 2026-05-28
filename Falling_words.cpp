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

struct Fragment {
    float x, y;
    float vx, vy;
    char symbol;
    int lifetime;
    int maxLifetime;
    bool active;
};

struct Particle {
    char symbol;
    float x, y;
    float vx, vy;
    int startDelay;
    bool active;
    bool exploding;
    int explodeFrame;
    int color;
};

int main() {
    srand(time(0));
    setlocale(LC_ALL, "");
    
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
    
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);
    keypad(stdscr, TRUE);
    
    
    start_color();
    
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    
    int screenHeight, screenWidth;
    getmaxyx(stdscr, screenHeight, screenWidth);
    screenHeight -= 1;
    
    float baseSpeedY = 2.5f;
    float speedVariation = 2.0f;
    int startDelayMax = 50;
    
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
        p.exploding = false;
        p.explodeFrame = 0;
        p.color = 1 + rand() % 7;
        particles.push_back(p);
    }
    
    vector<Fragment> fragments;
    
    auto createExplosion = [&](float x, float y, char symbol) {
        int numFragments = 12 + rand() % 8;
        
        for (int i = 0; i < numFragments; i++) {
            Fragment f;
            f.x = x;
            f.y = y;
            
            float angle = (rand() % 360) * M_PI / 180.0f;
            float speed = 1.5f + (rand() % 100) / 50.0f;
            f.vx = cos(angle) * speed;
            f.vy = sin(angle) * speed;
            
            char fragSymbols[] = {'.', '*', '+', '#', '~', '`', '\'', '^'};
            f.symbol = fragSymbols[rand() % 8];
            
            f.maxLifetime = 30 + rand() % 40;
            f.lifetime = 0;
            f.active = true;
            
            fragments.push_back(f);
        }
        
        for (int i = 0; i < 5; i++) {
            Fragment f;
            f.x = x;
            f.y = y;
            float angle = (rand() % 360) * M_PI / 180.0f;
            float speed = 2.0f + (rand() % 100) / 30.0f;
            f.vx = cos(angle) * speed;
            f.vy = sin(angle) * speed;
            f.symbol = (rand() % 2) ? '*' : '#';
            f.maxLifetime = 20 + rand() % 30;
            f.lifetime = 0;
            f.active = true;
            fragments.push_back(f);
        }
    };
    
    int frame = 0;
    bool running = true;
    
    while (running) {
        int ch = getch();
        if (ch == 27 || ch == 'q' || ch == 'Q') break;
        
        erase();
        
        for (auto& p : particles) {
            if (!p.active && !p.exploding && frame >= p.startDelay) {
                p.active = true;
            }
            
            if (p.active && !p.exploding) {
                p.x += p.vx;
                p.y += p.vy;
                
                if (p.y >= screenHeight - 1) {
                    p.y = screenHeight - 1;
                    p.vy = -p.vy * 0.55f;
                    p.vx *= 0.95f;
                    
                    if (fabs(p.vy) < 0.3f) {
                        p.exploding = true;
                        p.explodeFrame = frame;
                        createExplosion(p.x, p.y, p.symbol);
                        p.active = false;
                        continue;
                    }
                }
                
                if (p.y <= 0 && p.vy < 0) {
                    p.y = 0;
                    p.vy = -p.vy * 0.5f;
                }
                
                if (p.x <= 0) {
                    p.x = 0;
                    p.vx = -p.vx * 0.7f;
                }
                if (p.x >= screenWidth - 1) {
                    p.x = screenWidth - 1;
                    p.vx = -p.vx * 0.7f;
                }
                
                if (p.x < 0) p.x = 0;
                if (p.x >= screenWidth) p.x = screenWidth - 1;
                if (p.y < 0) p.y = 0;
                if (p.y >= screenHeight) p.y = screenHeight - 1;
                
                attron(COLOR_PAIR(p.color) | A_BOLD);
                mvaddch((int)p.y, (int)p.x, p.symbol);
                attroff(COLOR_PAIR(p.color) | A_BOLD);
            }
        }
        
        for (int i = 0; i < fragments.size(); ) {
            Fragment& f = fragments[i];
            
            if (!f.active) {
                fragments.erase(fragments.begin() + i);
                continue;
            }
            
            f.x += f.vx;
            f.y += f.vy;
            f.vx *= 0.98f;
            f.vy *= 0.98f;
            f.vy += 0.1f;
            f.lifetime++;
            
            if (f.lifetime >= f.maxLifetime || 
                f.x < 0 || f.x >= screenWidth || 
                f.y < 0 || f.y >= screenHeight) {
                fragments.erase(fragments.begin() + i);
                continue;
            }
            
            mvaddch((int)f.y, (int)f.x, f.symbol);
            
            i++;
        }
        
        for (auto& f : fragments) {
            if (f.lifetime < 5) {
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        int nx = (int)f.x + dx;
                        int ny = (int)f.y + dy;
                        if (nx >= 0 && nx < screenWidth && ny >= 0 && ny < screenHeight) {
                            if (rand() % 3 == 0) {
                                mvaddch(ny, nx, '~');
                            }
                        }
                    }
                }
            }
        }
        
        bool allExploded = true;
        for (auto& p : particles) {
            if (!p.exploding && frame >= p.startDelay) {
                allExploded = false;
                break;
            }
        }
        
        if (allExploded && fragments.size() < 3) {
            refresh();
            usleep(800000);
            
            for (auto& p : particles) {
                p.x = screenWidth - 1;
                p.y = 0;
                p.vx = -((rand() % 15) / 10.0f + 0.3f);
                p.vy = baseSpeedY + (rand() % (int)(speedVariation * 10)) / 10.0f;
                p.startDelay = rand() % startDelayMax;
                p.active = false;
                p.exploding = false;
                p.color = 1 + rand() % 7;
            }
            fragments.clear();
            frame = 0;
        }
        
        if (rand() % 100 < 2) {
            for (auto& p : particles) {
                if (p.active && !p.exploding) {
                    Fragment spark;
                    spark.x = p.x + (rand() % 3 - 1);
                    spark.y = p.y + (rand() % 3 - 1);
                    spark.vx = (rand() % 100 - 50) / 20.0f;
                    spark.vy = (rand() % 100) / 30.0f;
                    spark.symbol = '.';
                    spark.maxLifetime = 5 + rand() % 10;
                    spark.lifetime = 0;
                    spark.active = true;
                    fragments.push_back(spark);
                    break;
                }
            }
        }
        
        refresh();
        usleep(40000);
        frame++;
    }
    
    curs_set(1);
    endwin();
    return 0;
}
