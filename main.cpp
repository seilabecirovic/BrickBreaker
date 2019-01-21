#include "mbed.h"
#define dp23 P0_0
#include "main.h"
#include "N5110.h"

// Display
N5110 display(dp4, dp24, dp23, dp25, dp2, dp6, dp18);

// Kretanje po x-osi
AnalogIn VRx(dp11);

// Kretanje po y-osi
AnalogIn VRy(dp10);

// Taster joysticka
DigitalIn   SW(dp9);

// Rezultati
int highscore [3]= {0, 0, 0};

// Praćenje novih rezultata
int scoreFlag = 1;

// Pozicija lopte x-osa
int ballX = 42;

// Pozicija lopte y-osa
int ballY = 38;

// Širina lopte
int ballWidth = 1;

// Visina lopte
int ballHeight = 1;

// Potrebno fill stanje za drawRect
int ballFill = 1;

// Putanja lopte
int direction = 0;

// Reket x-osa
int paddleX = 38;

// Reket y-osa
int paddleY = 40;

// Širina reketa
int paddleWidth = 12;

// Visina reketa
int paddleHeight = 2;

// Okruženje lopte - potrebno za predviđanje udara
int surrounding[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Naznaka dodira lopte
int touchFlag = 0;

// Cigle x-osa
int bricksX[8] = {3, 13, 23, 33, 43, 53, 63, 73};

// Cigle y-osa
int bricksY[4] = {11, 16, 21, 26};

// Stanje cigli
int bricks[4][8];

// Prikaz menija
int displayMenu = 1;

// Prikaz igre
int displayGame = 0;

// Kraj
int end = 0;

// Kraj igre
int displayGameOver = 0;

// Nivo
int level = 0;

// Start=1, Stop=0;
int startStop = 1;

// Broj života
int lives = 6;

// Rezultat
int score = 0;

// Nacrtan okvir
int borderFlag = 0;

// Cigle koje se trebaju obrisati
int clearFlag[4][8];

// Cigle koje se trebaju nacrtati
int brickDrawFlag[4][8];

bool prelaz = false;

// Pomjeranje joysticka u igri
Ticker movePaddle;

//PlayPause
Ticker playPause;

void newLevel()
{
    display.clear();
    display.printString("Press button", 10, 2);
    display.printString("to start!", 10, 3);
    display.refresh();
    level++;
    prelaz = true;
        
    // počinjanje igre sa pauzom
    startStop = 0; 
    
    // kreiranje cigli novog levela
    initBricks(level);

    // resetiranje loptice i reketa
    ballY = 38;
    ballX = 42;
    paddleX = 38;
    direction = 8;
    display.refresh();
}

void pause()
{
    if (!SW)
    {
        while(!SW);
        startStop =! startStop;
        
        if (prelaz)
            display.clear(), prelaz = false;
    }
}

int getNumBricks()
{
    int sum = 0;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 4; j++)
            sum += bricks[j][i];
    return sum;
}

int lifeLost()
{
    if (ballY >= 48 && ballY <= 50) // lopta pala
    {
        lives -= 1; // oduzmi život
        ballY = 30; // startna pozicija
        ballX = 42;
        paddleX = 38;
        direction = 8;

        if (lives >= 0)
            startStop = 0; // pauza
        return 1;
    }
    return 0;
}

void game (int g)
{
    if (g == 1) // if the game is running
    {
        lifeLost(); // do all these crucial game functions
        borderInit();
        dispScore();
        dispLives();
        ball();
    }
}

void gameOver()
{
    while (SW == 1)
    {
        display.clear();
        display.printString("GAME OVER", 10, 0);

        display.printChar(((score / 100) % 10) + '0', 20, 1); // ispisivanje trocifrenog rezultata
        display.printChar(((score / 10) % 10) + '0', 27, 1);
        display.printChar((score % 10) + '0', 34, 1);

        // odredjivanje da li je dostignut novi highscore (u top 3)
        if ((score >= highscore[0]) && scoreFlag)
        {
            highscore[2] = highscore[1];
            highscore[1] = highscore[0];
            highscore[0] = score;
            scoreFlag = 0;
        }
        else if ((score >= highscore[1]) && scoreFlag)
        {
            highscore[2] = highscore[1];
            highscore[1] = score;
            scoreFlag = 0;
        }
        else if ((score >= highscore[2]) && scoreFlag)
        {
            highscore[2] = score;
            scoreFlag = 0;
        }
        if (score >= highscore[2])
            display.printString("NEW HIGH SCORE",0,3);

        display.refresh();
        wait_ms(32);
        displayGameOver = 0;
        displayMenu = 1;
        displayGame = 0;
    }

    while (!SW);
}


void dispLives()
{
    display.printChar(lives + '0', 2, 0); // ispisivanje broja zivota
    display.printChar(0x80, 7, 0); // crtanje ikonice za zivote
}

void dispScore()
{
    if (score < 10)
        display.printChar(score + '0', 77, 0); // ispisivanje jednocifrenog rezultata

    else if (score < 100)
    {
        display.printChar(((score / 10) % 10) + '0', 70, 0); // ispisivanje dvocifrenog rezultata
        display.printChar((score % 10) + '0', 77, 0);
    }
    else if (score < 1000)
    {
        display.printChar(((score / 100) % 10) + '0', 63, 0); // ispisivanje trocifrenog rezultata
        display.printChar(((score / 10) % 10) + '0', 70, 0);
        display.printChar((score % 10) + '0', 77, 0);
    }
}

void doBricks()
{

    for (int x = 0; x < 8; x++)   // prolazak kroz sve cigle
        for (int y = 0; y < 4; y++)
            getBrickTouch(x, y); // provjera da li je došlo do kontakta između cigle i loptice

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 8; j++)
            if (bricks[i][j] && brickDrawFlag[i][j])   // da li su cigle postavljene
            {
                display.drawRect(bricksX[j], bricksY[i], 6, 2, 1); //crtanje cigli
                brickDrawFlag[i][j] = 0; // izmjena flag-a kako bi se izbjegla ponavljanja
            }
    clearBricks(); // clear-anje cigli
}

void initBricks (int l) // crtanje cigli
{
    if (!l) // ako je prvi nivo (sve cigle u svim redovima)
    {
        paddleWidth = 12;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 8; j++)
            {
                bricks[i][j] = 1;
                brickDrawFlag[i][j] = 1;
            }
    }
    else if (l == 1) // ako je drugi nivo (cigle u svakoj drugoj koloni)
    {
        paddleWidth--;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 8; j += 2)
            {
                bricks[i][j] = 1;
                brickDrawFlag[i][j] = 1;
            }
    }
    else if (l == 2) // ako je treći nivo (cigle u svakom drugom redu)
    {
        paddleWidth--;
        for (int i = 0; i < 4; i += 2)
            for (int j = 0; j < 8; j++)
            {
                bricks[i][j] = 1;
                brickDrawFlag[i][j] = 1;
            }
    }
}

void clearBricks()
{
    for (int i = 0; i < 8; i++)   //prolazak kroz sve cigle
        for (int j = 0; j < 4; j++)
            if (clearFlag[j][i])   // da li ciglu treba clear-ati
            {
                display.clearRect(bricksX[i], bricksY[j], 7, 3); //clear-anje cigle
                clearFlag[j][i] = 0;
                score += 10; //povećavanje score-a
            }
}


void getBrickTouch (int x, int y)
{
    if (bricks[y][x])   //da li je cigla postavljena
    {
        for (int a = -1; a < 8; a++)   // prolazak kroz sve cigle
            if ((display.getPixel(bricksX[x] + a, bricksY[y] - 1)) || (display.getPixel(bricksX[x] + a, bricksY[y] + 3)))   // da li je piksel dotaknut
            {
                clearFlag[y][x] = 1; // oznaka da se cigla clear-a
                bricks[y][x] = 0; // clear-anje cigle
            }
        for (int b = -1; b < 3; b++)
            if ((display.getPixel(bricksX[x] - 1, bricksY[y] + b)) || (display.getPixel(bricksX[x] + 7,bricksY[y] + b)))
            {
                clearFlag[y][x] = 1;
                bricks[y][x] = 0;
            }
    }
}


// kretanje joysticka
void paddle()
{
    if (!displayGame) return; // provjera da li je igra pokrenuta

    display.clearRect(1, paddleY, 82, 3); //clear-anje paddlea

    if (VRx < 1.0 / 3.0)
    {
        paddleX--; // pomjeranje paddle-a
        if(paddleX < 1)
            paddleX = 1;
    }
    else if (VRx > 2.0 / 3.0)
    {
        paddleX++; // pomjeranje paddle-a
        if(paddleX > 72)
            paddleX=72;
    }
    display.drawRect(paddleX, paddleY, paddleWidth, paddleHeight, 1);
}

// postavka touchFlag-a ako je lopta dotakla bilo koji piksel cigli
// bilježenje koji je piksel dotaknut
void getTouchFlag()
{
    if (display.getPixel(ballX - 1, ballY))         //11
        touchFlag = surrounding[11] = 1;
    if (display.getPixel(ballX - 1, ballY - 1))     //0
        touchFlag = surrounding[0] = 1;
    if (display.getPixel(ballX, ballY - 1))         //1
        touchFlag = surrounding[1] = 1;
    if (display.getPixel(ballX + 1, ballY - 1))     //2
        touchFlag = surrounding[2] = 1;
    if (display.getPixel(ballX + 2, ballY - 1))     //3
        touchFlag = surrounding[3] = 1;
    if (display.getPixel(ballX + 2, ballY))         //4
        touchFlag = surrounding[4] = 1;
    if (display.getPixel(ballX + 2, ballY + 1))     //5
        touchFlag = surrounding[5] = 1;
    if (display.getPixel(ballX + 2,ballY + 2))      //6
        touchFlag = surrounding[6] = 1;
    if (display.getPixel(ballX + 1, ballY + 2))     //7
        touchFlag = surrounding[7] = 1;
    if (display.getPixel(ballX, ballY + 2))         //8
        touchFlag = surrounding[8] = 1;
    if (display.getPixel(ballX - 1, ballY + 2))     //9
        touchFlag = surrounding[9] = 1;
    if (display.getPixel(ballX-  1, ballY + 1))     //10
        touchFlag = surrounding[10] = 1;
}

//izračunavanje dodira lopte i određivanje novog ugla i kretanja
int setAngle()
{
    getTouchFlag();

    if (touchFlag)
    {
        //Tri kvadrata na čošku
        if (surrounding[11] && surrounding[0] && surrounding[1])   //gornji desni
            direction = 6;
        else if (surrounding[2] && surrounding[3] && surrounding[4])     //donji desni
            direction = 10;
        else if (surrounding[5] && surrounding[6] && surrounding[7])     //donji lijevi
            direction = 14;
        else if (surrounding[8] && surrounding[9] && surrounding[10])     //gornji lijevi
            direction = 2;

        //3 na krajevima 
        else if (surrounding[0] && surrounding[1] && surrounding[2] && !surrounding[3])   //gornja lijeva
            direction = 5;
        else if (surrounding[3] && surrounding[1] && surrounding[2] && !surrounding[0])     //gornja desna
            direction = 11;
        else if (surrounding[6] && surrounding[7] && surrounding[8] && !surrounding[9])     // donjadesna
            direction = 13;
        else if (surrounding[7] && surrounding[8] && surrounding[9] && !surrounding[6])     //donja lijeva
            direction = 3;
        else if (surrounding[0] && surrounding[11] && surrounding[10] && !surrounding[9])     //lijeva - gornja
            direction = 5;
        else if (surrounding[3] && surrounding[4] && surrounding[5] && !surrounding[6])     //desna - gornja
            direction = 11;
        else if (surrounding[11] && surrounding[10] && surrounding[9] && !surrounding[0])     //lijeva - donja
            direction = 3;
        else if (surrounding[4] && surrounding[5] && surrounding[6] && !surrounding[3])     //desna - donja
            direction = 13;

        //Kvadrati u centru
        else if (surrounding[1] && surrounding[2])   //gornji rub
        {
            switch (direction)
            {
                case 3:
                    direction = 5;
                    break;
                case 2:
                    direction = 6;
                    break;
                case 1:
                    direction = 7;
                    break;
                case 0:
                    direction = 8;
                    break;
                case 15:
                    direction = 9;
                    break;
                case 14:
                    direction = 10;
                    break;
                case 13:
                    direction = 11;
                    break;
            }

        }
        else if (surrounding[4] && surrounding[5])     //desni rub
        {
            switch (direction)
            {
                case 1:
                    direction = 15;
                    break;
                case 2:
                    direction = 14;
                    break;
                case 3:
                    direction = 13;
                    break;
                case 4:
                    direction = 12;
                    break;
                case 5:
                    direction = 11;
                    break;
                case 6:
                    direction = 10;
                    break;
                case 7:
                    direction = 9;
                    break;
            }
        }
        else if (surrounding[10] && surrounding[11])     //Lijevi rub
        {
            switch (direction)
            {
                case 9:
                    direction = 7;
                    break;
                case 10:
                    direction = 6;
                    break;
                case 11:
                    direction = 5;
                    break;
                case 12:
                    direction = 4;
                    break;
                case 13:
                    direction = 3;
                    break;
                case 14:
                    direction = 2;
                    break;
                case 15:
                    direction = 1;
                    break;
            }
        }
        else if (surrounding[7] && surrounding[8])     //donji rub
        {
            switch (direction)
            {
                case 5:
                    direction = 3;
                    break;
                case 6:
                    direction = 2;
                    break;
                case 7:
                    direction = 1;
                    break;
                case 8:
                    direction = 0;
                    break;
                case 9:
                    direction = 15;
                    break;
                case 10:
                    direction = 14;
                    break;
                case 11:
                    direction = 13;
                    break;
            }
        }

        //2 Krajnja kvadrata
        else if (surrounding[0] && surrounding[1])   //gronja lijeva
        {
            switch (direction)
            {
                case 0:
                    direction = 7;
                    break;
                case 1:
                    direction = 6;
                    break;
                case 2:
                    direction = 5;
                    break;
                case 3:
                    direction = 5;
                    break;
                case 15:
                    direction = 8;
                    break;
                case 14:
                    direction = 9;
                    break;
                case 13:
                    direction = 10;
                    break;
            }
        }
        else if (surrounding[2] && surrounding[3])     //gornja desna
        {
            switch (direction)
            {
                case 0:
                    direction = 9;
                    break;
                case 1:
                    direction = 8;
                    break;
                case 2:
                    direction = 7;
                    break;
                case 3:
                    direction = 6;
                    break;
                case 15:
                    direction = 10;
                    break;
                case 14:
                    direction = 11;
                    break;
                case 13:
                    direction = 11;
                    break;
            }
        }
        else if (surrounding[9] && surrounding[8])     // donja desna
        {
            switch (direction)
            {
                case 11:
                    direction = 14;
                    break;
                case 10:
                    direction = 15;
                    break;
                case 9:
                    direction = 0;
                    break;
                case 8:
                    direction = 1;
                    break;
                case 7:
                    direction = 2;
                    break;
                case 6:
                    direction = 3;
                    break;
                case 5:
                    direction = 3;
                    break;
            }
        }
        else if (surrounding[7] && surrounding[6])     //donja lijeva
        {
            switch (direction)
            {
                case 11:
                    direction = 13;
                    break;
                case 10:
                    direction = 13;
                    break;
                case 9:
                    direction = 14;
                    break;
                case 8:
                    direction = 15;
                    break;
                case 7:
                    direction = 0;
                    break;
                case 6:
                    direction = 1;
                    break;
                case 5:
                    direction = 2;
                    break;
            }
        }
        else if (surrounding[0] && surrounding[11])     //lijeva - gornja
        {
            switch (direction)
            {
                case 9:
                    direction = 8;
                    break;
                case 10:
                    direction = 7;
                    break;
                case 11:
                    direction = 6;
                    break;
                case 13:
                    direction = 5;
                    break;
                case 14:
                    direction = 3;
                    break;
                case 15:
                    direction = 2;
                    break;
                case 0:
                    direction = 1;
                    break;
            }
        }
        else if (surrounding[3] && surrounding[4])     //desna gornja
        {
            switch (direction)
            {
                case 0:
                    direction = 15;
                    break;
                case 1:
                    direction = 14;
                    break;
                case 2:
                    direction = 13;
                    break;
                case 3:
                    direction = 11;
                    break;
                case 5:
                    direction = 10;
                    break;
                case 6:
                    direction = 9;
                    break;
                case 7:
                    direction = 8;
                    break;
            }
        }
        else if (surrounding[10] && surrounding[9])     //lijeva donja
        {
            switch (direction)
            {
                case 8:
                    direction = 7;
                    break;
                case 9:
                    direction = 6;
                    break;
                case 10:
                    direction = 5;
                    break;
                case 11:
                    direction = 3;
                    break;
                case 13:
                    direction = 2;
                    break;
                case 14:
                    direction = 1;
                    break;
                case 15:
                    direction = 0;
                    break;
            }
        }
        else if (surrounding[5] && surrounding[6])     //desna donja
        {
            switch (direction)
            {
                case 8:
                    direction = 9;
                    break;
                case 1:
                    direction = 0;
                    break;
                case 2:
                    direction = 15;
                    break;
                case 3:
                    direction = 14;
                    break;
                case 5:
                    direction = 13;
                    break;
                case 6:
                    direction = 11;
                    break;
                case 7:
                    direction = 10;
                    break;
            }
        }

        // dodirivanje uglova
        else if (surrounding[3])   // gornji desni
            direction = 10;
        else if (surrounding[6])     // donji desni
            direction = 14;
        else if (surrounding[9])     // donji lijevi
            direction = 2;
        else if (surrounding[0])     // gornji lijevi
            direction = 6;

        touchFlag = 0; // restartovanje touchFlag-a

        for (int i = 0; i < 12; i++)   // čišćenje tačaka dodira
            surrounding[i] = 0;
    }
    return direction;
}

void ball()
{
    int e = direction;
    setAngle(); // provjera sta je loptica dotakla i postavka novog ugla odbijanja
    doBricks(); // provjera dotaknutih cigli i brisanje
    moveBall1(direction); // pomijeranje loptice pod novim uglom
    wait_ms(32);

    if (direction == e)   // da li se putanja promijenila
    {
        setAngle();
        doBricks();
        moveBall1(direction);
        wait_ms(32);
    }
}

void moveBall1 (int d)   //pomijeranje loptice pod jednim od 16 mogućih uglova
{
    display.clearRect(ballX, ballY, ballWidth + 1, ballHeight + 1);

    if (d == 0)   //0 stepeni
        ballY--;
    else if (d == 1)     //22.5 stepeni
    {
        ballY--;
        ballX++;
    }
    else if (d == 2)     //45 stepeni
    {
        ballY--;
        ballX++;
    }
    else if (d == 3)     //62.5 stepeni
    {
        ballY--;
        ballX++;
    }
    else if (d == 4)     //90 stepeni
        ballX++;
    else if (d == 5)     //112.5 stepeni
    {
        ballX++;
        ballY++;
    }
    else if (d == 6)     //135 stepeni
    {
        ballX++;
        ballY++;
    }
    else if (d == 7)     //157.5 stepeni
    {
        ballX++;
        ballY++;
    }
    else if (d == 8)     //180 stepeni
        ballY++;
    else if (d == 9)     //202.5 stepeni
    {
        ballY++;
        ballX--;
    }
    else if (d == 10)     //225 stepeni
    {
        ballY++;
        ballX--;
    }
    else if (d == 11)     //247.5 stepeni
    {
        ballY++;
        ballX--;
    }
    else if (d == 12)     //270 stepeni
        ballX-=1;
    else if (d == 13)     //292.5 stepeni
    {
        ballX--;
        ballY--;
    }
    else if (d == 14)     //315 stepeni
    {
        ballX--;
        ballY--;
    }
    else if (d == 15)     //337.5 stepeni
    {
        ballX--;
        ballY--;
    }    
    display.drawRect(ballX, ballY, ballWidth, ballHeight, ballFill);
    display.refresh();    
}

// postavka okvira display-a
void borderInit()
{

    if (!displayGame) // provjera da li je igra pokrenuta
        return;

    for (int i = 0; i <= 83;i++)   //horizontalni okvir
    {
        display.setPixel(i, 0);
        display.setPixel(i, 8);
    }
    for (int i = 0; i <= 48; i++)   //vertikalne linije
    {
        display.setPixel(0, i);
        display.setPixel(83, i);
    }
}

void showMenu()
{
    int i = 2;
    while(1)
    {
        display.clear(); // ispis menija
        display.printString("*", 11, i);
        display.printString(" BrickBreaker", 0, 0);
        display.printString("Start", 16, 2);
        display.printString("Highscore", 16, 3);
        display.printString("About", 16, 4);
        display.printString("Exit", 16, 5);
        display.refresh();

        if (VRy > 2.0 / 3.0)    // pozicija joystick-a
        {
            i++;
            if (i > 5)
                i = 2;
            while(VRy > 2.0 / 3.0);
        }

        if (VRy < 1.0 / 3.0)
        {
            i--;
            if(i < 2)
                i = 5;
            while(VRy < 1.0 / 3.0);
        }

        if(!SW) 
        {
            while(!SW);

            if(i == 2)      // koja opcija menija je odabrana? - startGame
            {
                displayMenu = 0;
                displayGame = 1;
                movePaddle.attach(&paddle, 0.025);
                playPause.attach(&pause, 0.025);
                lives = 6;
                score = 0;
                prelaz = true;
                initBricks(0);
                startStop = 0;
                scoreFlag = 1;
                display.clear();
                display.printString("Press button", 10, 2);
                display.printString("to start!", 10, 3);
                display.refresh();
                return;
            }
            if (i == 3)     // highScore
            {
                while(SW == 1)
                {
                    display.clear();
                    display.printString(" HighScores", 0, 0);
                    for (int i = 0; i < 3; i++)
                    {
                        display.printChar(i + 1 + '0', 25, i + 2);
                        display.printChar('.', 32, i + 2);
                        display.printChar(((highscore[i] / 100) % 10) + '0', 43, i + 2); //ispisivanje trocifrenog score-a
                        display.printChar(((highscore[i] / 10) % 10) + '0', 50, i + 2);
                        display.printChar((highscore[i] % 10) + '0', 57, i + 2);
                    }
                    display.printString("Press button", 5, 5);
                    display.refresh();
                    wait_ms(50);
                }
                while (!SW);
            }
            else if (i==4) // about
            {
                while(SW == 1)
                {
                    display.clear();
                    display.printString(" BrickBreaker", 0, 0);
                    display.printString("Created by:", 2, 2);
                    display.printString("Seila B.", 2, 3);
                    display.printString("Berina C.", 2, 4);
                    display.printString("Press button", 2, 5);
                    display.refresh();
                    wait_ms(20);
                }
                while (!SW);
            }
            else if (i == 5)    // exit
            {
                display.turnOff();
                end = 1;
                return;
            }
        }
        wait_ms(40);
    }
}

int main()
{
    display.init();

    SW.mode(PullUp);
    while (1)
    {
        if (displayMenu)
            showMenu();

        while(displayGame == 1)
        {
            game(startStop);

            if (lives < 0) //da li su izgubljeni svi zivoti?
            {
                displayGame = 0;
                displayGameOver = 1;
            }

            if (!getNumBricks()) // da li je predjen drugi nivo?
            {
                if (level > 2)
                {
                    displayGameOver = 1;
                    break;
                }                
                newLevel();
            }
        }

        if(displayGameOver)
        {
            movePaddle.detach();
            playPause.detach();
            gameOver();           
        }
        display.refresh();
        
        if (end)
            break;
    }
    return 0;
}