#ifndef MAIN_H
#define MAIN_H
 
 #include "mbed.h"
#define dp23 P0_0
#include "N5110.h"

void newLevel();
void pause();
int getNumBricks();
int lifeLost();
void game(int g);
void gameOver();
void dispLives();
void dispScore();
void doBricks();
void initBricks(int l);
void clearBricks();
void getBrickTouch(int x, int y);
void paddle();
void getTouchFlag();
int setAngle();
void ball();
void moveBall1(int d);
void borderInit();
void showMenu();

#endif