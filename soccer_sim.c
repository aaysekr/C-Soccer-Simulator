#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdarg.h>
#include <conio.h>


#define W 120
#define H 28 
#define PITCH_START_Y 2 
#define COMMENTARY_OFFSET 2 
#define CONSOLE_HEIGHT 34 

#define TEAM_SIZE 8
#define PITCH_MARGIN 2
#define CAPTURE_DISTANCE 2 
#define DRIBBLE_CHANCE 40  
#define SHOT_RANGE 40      


#define COLOR_PITCH 0x2F 
#define COLOR_GOAL_LINE 0x27
#define COLOR_BLACK_TEAM 0x0F
#define COLOR_RED_TEAM 0x4F
#define COLOR_BALL 0xF7 
#define COLOR_DEFAULT 0x0F
#define COLOR_GOAL_FLASH 0x4F 
#define COLOR_BALL_CHAR 0x07 


CHAR_INFO screenBuffer[W * H]; 
char map[H][W];


#define DUZ_YATAY (char)196
#define DUZ_DIKEY (char)179
#define KOSE_SOL_UST (char)218
#define KOSE_SAG_UST (char)191
#define KOSE_SOL_ALT (char)192
#define KOSE_SAG_ALT (char)217
#define T_UST (char)194
#define T_ALT (char)193
#define T_SOL (char)195
#define T_SAG (char)180
#define ARTI (char)197


typedef struct {
    int x;
    int y;
    int team;
    int id;
    bool hasBall;
} Player;

typedef struct {
    int x;
    int y;
    int dx;
    int dy;
    int ownerTeam;
    int ownerId;
    bool moving;
} Ball;


Player blackTeam[TEAM_SIZE];
Player redTeam[TEAM_SIZE];
Ball ball;
HANDLE hConsole;
int scoreBlack = 0;
int scoreRed = 0;
int matchTime = 0;
bool matchActive = false;
bool gamePaused = true; 
int halfTime = 1;
char commentary[200] = "Mac basliyor!";
int scoringTeamIndicator = -1; 


void clearMap();
void putChar(int x, int y, char c);
void putCharColored(int x, int y, char c, WORD color);
void drawH(int x1, int x2, int y, char ch);
void drawV(int x, int y1, int y2, char ch);
void drawRect(int x1, int y1, int x2, int y2);
void drawPitch();
void printMap();
void hideCursor();
void drawPlayer(int x, int y, int team, int id, bool isOwner);
void drawBall(int x, int y);
void drawAllPlayers();
void initTeamPositions(Player team[], int teamID, int startX);
void initTeams();
void updateCommentary(const char* format, ...);
void resetAfterGoal(int scoringTeam);
Player* findBestPassTarget(Player *p, Player team[], Player opposingTeam[]);
bool shouldShoot(Player *p, int goalX, int goalY);
void movePlayerAI(Player *p, int teamID, Player opposingTeam[]);
void playerActions();
void updateMatchTime();
int checkGoal();
void updateBallMovement();
void handleInput();



void putCharColored(int x, int y, char c, WORD color) {
    if (x >= 0 && x < W && y >= 0 && y < H) {
        int index = y * W + x;
        screenBuffer[index].Char.AsciiChar = c;
        screenBuffer[index].Attributes = color;
    }
}
void clearMap() {
    int y, x;
    for (y = 0; y < H; y++) {
        for (x = 0; x < W; x++) {
            putCharColored(x, y, ' ', COLOR_PITCH);
            map[y][x] = ' ';
        }
    }
}
void putChar(int x, int y, char c) {
    if (x >= 0 && x < W && y >= 0 && y < H) {
        map[y][x] = c;
    }
}
void drawH(int x1, int x2, int y, char ch) {
    int x;
    for (x = x1; x <= x2; x++) putChar(x, y, ch);
}
void drawV(int x, int y1, int y2, char ch) {
    int y;
    for (y = y1; y <= y2; y++) putChar(x, y, ch);
}
void drawRect(int x1, int y1, int x2, int y2) {
    drawH(x1 + 1, x2 - 1, y1, DUZ_YATAY);
    drawH(x1 + 1, x2 - 1, y2, DUZ_YATAY); 
    
    drawV(x1, y1 + 1, y2 - 1, DUZ_DIKEY);
    drawV(x2, y1 + 1, y2 - 1, DUZ_DIKEY);
    
    putChar(x1, y1, KOSE_SOL_UST);
    putChar(x2, y1, KOSE_SAG_UST);
    putChar(x1, y2, KOSE_SOL_ALT);
    putChar(x2, y2, KOSE_SAG_ALT);
}
void drawPitch() {
    int marginX = PITCH_MARGIN;
    int marginY = PITCH_MARGIN;
    int boxH = 6;
    int boxW = 12;
    int y, x;

    drawRect(marginX, marginY, W - marginX - 1, H - marginY - 1); 
    
    drawV(W / 2, marginY + 1, H - marginY - 2, DUZ_DIKEY);
    putChar(W / 2, marginY, T_UST);
    putChar(W / 2, H - marginY - 1, T_ALT); 
    drawRect(W / 2 - 5 * 2, H / 2 - 5, W / 2 + 5 * 2, H / 2 + 5);
    putChar(W / 2, H / 2, ARTI);
    

    drawRect(marginX, H / 2 - boxH, marginX + boxW, H / 2 + boxH);
    putChar(marginX, H / 2 - boxH, T_SOL);
    putChar(marginX, H / 2 + boxH, T_SOL);
    

    drawRect(W - marginX - 1 - boxW, H / 2 - boxH, W - marginX - 1, H / 2 + boxH);
    putChar(W - marginX - 1, H / 2 - boxH, T_SAG);
    putChar(W - marginX - 1, H / 2 + boxH, T_SAG);

    for(y = 0; y < H; y++) {
        for(x = 0; x < W; x++) {
            if (map[y][x] != ' ') {
                 putCharColored(x, y, map[y][x], COLOR_GOAL_LINE);
            }
        }
    }
}
void printMap() {
    COORD bufferSize = {W, H};
    COORD characterPos = {0, 0};

    COORD timeCoord = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), timeCoord);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x2F); 

    printf("------------------------------------------------------------------------------------------------------------------------\n");
    printf(" %d. YARI | ZAMAN: %02d:00 DK | SKOR: Siyah %d - %d Kirmizi |", 
           halfTime, matchTime, scoreBlack, scoreRed);

    SMALL_RECT sahaRegion = {0, PITCH_START_Y, W - 1, PITCH_START_Y + H - 1}; 
    WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), screenBuffer, bufferSize, characterPos, &sahaRegion);

    if (gamePaused) {
        
        COORD centerCoord = {W / 2 - 25, PITCH_START_Y + H / 2 - 1}; 
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), centerCoord);
        
        if (scoringTeamIndicator != -1) { 
            WORD flashColor = (scoringTeamIndicator == 0) ? 0xF0 : 0xF4; 
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), flashColor);
            
            printf(" GOOOOL! * DEVAM ETMEK ICIN SPACE BASIN * "); 
            
        } else {

            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xF0);
            
            if (matchTime == 0 && halfTime == 1) {

                printf(" ** BASLAMAK ICIN SPACE TUSUNA BASIN ** ");
            } else if (matchTime == 45 && halfTime == 2) {
                printf("    * DEVRE ARASI DEVAM ETMEK ICIN SPACE BASIN * ");
            } else if (matchTime == 90 && halfTime == 2)
                 printf(" * MAC SONU DEVAM ETMEK ICIN SPACE BASIN * ");
            else {
                 printf(" * OYUN DURAKLATILDI. DEVAM ETMEK ICIN SPACE BASIN * ");
            }
        }
    }


    COORD commentaryCoord = {0, PITCH_START_Y + H + COMMENTARY_OFFSET - 4}; 
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), commentaryCoord);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x2F); 

    printf(" SPOR SERVISI: %s", commentary); 


    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){0, CONSOLE_HEIGHT - 1});
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_PITCH);
}
void hideCursor() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(h, &info);
}
void drawPlayer(int x, int y, int team, int id, bool isOwner) {
    WORD color = (team == 0) ? COLOR_BLACK_TEAM : COLOR_RED_TEAM;
    char playerChar = (char)(id + '0');

    putCharColored(x, y, playerChar, color);
}
void drawBall(int x, int y) {
    putCharColored(x, y, 'o', COLOR_PITCH | COLOR_BALL_CHAR); 
}
void drawAllPlayers() {
    int i;

    drawBall(ball.x, ball.y);

    for (i = 0; i < TEAM_SIZE; i++) {
        drawPlayer(blackTeam[i].x, blackTeam[i].y, 0, blackTeam[i].id, blackTeam[i].hasBall);
        drawPlayer(redTeam[i].x, redTeam[i].y, 1, redTeam[i].id, redTeam[i].hasBall);
    }
}


void updateCommentary(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(commentary, sizeof(commentary) - 1, format, args);
    va_end(args);
}

void initTeamPositions(Player team[], int teamID, int startX) {
    int i;

    team[0].id = 1;
    team[0].x = (teamID == 0) ? PITCH_MARGIN + 2 : W - PITCH_MARGIN - 3;
    team[0].y = H / 2 + (rand() % 3 - 1);

    int defX = (teamID == 0) ? PITCH_MARGIN + 20 : W - PITCH_MARGIN - 21;
    int defY_offsets[] = {4, 8, 17, 24}; 
    for (i = 1; i < 5; i++) {
        team[i].id = i + 1;
        team[i].x = defX + (rand() % 7 - 3);
        team[i].y = defY_offsets[i-1];
    }

    int midX = (teamID == 0) ? W / 2 -3: W / 2 + 3;
    int midY_offsets[] = {6, 13, 21}; 
    for (i = 5; i < TEAM_SIZE; i++) {
        team[i].id = i + 1;
        team[i].x = midX;
        team[i].y = midY_offsets[i-5];
    }

    for (i = 0; i < TEAM_SIZE; i++) {
        team[i].team = teamID;
        team[i].hasBall = false;
        if (team[i].y < PITCH_MARGIN + 1 || team[i].y > H - PITCH_MARGIN - 2) {
             team[i].y = H / 2;
        }
    }
}

void initTeams() {
    initTeamPositions(blackTeam, 0, 0);
    initTeamPositions(redTeam, 1, 0);

    ball.x = W / 2;
    ball.y = H / 2;
    ball.ownerTeam = 0;
    ball.ownerId = 8;
    ball.moving = false;
    blackTeam[TEAM_SIZE - 1].hasBall = true;

    updateCommentary("Hazirsaniz basliyoruz!");
}

void resetAfterGoal(int scoringTeam) {
    int i;

    scoringTeamIndicator = scoringTeam;

    if (scoringTeam == 0) {
        scoreBlack++; 
        updateCommentary("Siyah Takim atti! (Space ile devam edin)"); 
    } else if (scoringTeam == 1) {
        scoreRed++; 
        updateCommentary("Kirmizi Takim atti! (Space ile devam edin)"); 
    }

    gamePaused = true; 

    initTeamPositions(blackTeam, 0, 0);
    initTeamPositions(redTeam, 1, 0);

    for(i = 0; i < TEAM_SIZE; i++) {
        blackTeam[i].hasBall = false;
        redTeam[i].hasBall = false;
    }

    ball.x = W / 2;
    ball.y = H / 2;
    ball.moving = false;

    if (scoringTeam == 0) {
        ball.ownerTeam = 1;
        redTeam[TEAM_SIZE - 1].hasBall = true;
    } else {
        ball.ownerTeam = 0;
        blackTeam[TEAM_SIZE - 1].hasBall = true;
    }
}

int checkGoal() {
    int goalX_Left = PITCH_MARGIN;
    int goalX_Right = W - PITCH_MARGIN - 1;
    int goalY_Min = H / 2 - 6;
    int goalY_Max = H / 2 + 6;

    if (ball.y > goalY_Min && ball.y < goalY_Max) {
        if (ball.x <= goalX_Left + 1) { 
            return 1; 
        }
        if (ball.x >= goalX_Right - 1) { 
            return 0; 
        }
    }
    return -1;
}

Player* findBestPassTarget(Player *p, Player team[], Player opposingTeam[]) {
    int i, j;
    Player *bestTarget = NULL;
    double bestScore = -9999.0;

    int goalX = (p->team == 0) ? W - PITCH_MARGIN - 1 : PITCH_MARGIN;

    for (i = 0; i < TEAM_SIZE; i++) {
        if (&team[i] == p) continue;

        double distToGoal = (double)abs(goalX - team[i].x);
        double distToPasser = sqrt(pow(team[i].x - p->x, 2) + pow(team[i].y - p->y, 2));
        
        if (distToPasser < 5) continue; 
        
        double opennessScore = 0.0;

        for(j = 0; j < TEAM_SIZE; j++) {
            double distToOpponent = sqrt(pow(team[i].x - opposingTeam[j].x, 2) + pow(team[i].y - opposingTeam[j].y, 2));
            if (distToOpponent < 10) {
                 opennessScore -= (10 - distToOpponent) * 3;
            }
        }

        double currentScore = (double)(W - distToGoal) + opennessScore * 1.5 - distToPasser * 0.5;

        if (distToPasser > 30) { 
            currentScore += (distToPasser / 10.0); 
        }

        if (currentScore > bestScore) {
            bestScore = currentScore;
            bestTarget = &team[i];
        }
    }

    if (bestScore > 50 && rand() % 5 == 0) {
        return bestTarget;
    }

    return NULL;
}

bool shouldShoot(Player *p, int goalX, int goalY) {
    int goalCenterY = H / 2;

    double distToGoal = sqrt(pow(goalX - p->x, 2) + pow(goalCenterY - p->y, 2));

    if (distToGoal > SHOT_RANGE) {
        return false;
    }

    double shotScore = (SHOT_RANGE - distToGoal) * 5.0;

    if (distToGoal < 10) {
        if (rand() % 2 == 0) return true;
    }

    if (shotScore > 50 && rand() % 3 == 0) {
        return true;
    }

    return false;
}

void movePlayerAI(Player *p, int teamID, Player opposingTeam[]) {
    int targetX, targetY;
    int dx = 0, dy = 0;
    int nextX, nextY;
    Player *ownTeam = (teamID == 0) ? blackTeam : redTeam;
    int i;

    int goalX = (teamID == 0) ? W - PITCH_MARGIN - 1 : PITCH_MARGIN;

    if (p->hasBall) {
        if (shouldShoot(p, goalX, H / 2)) {
             updateCommentary("Oyuncu #%d SUT CEKIYOR! Gole gidiyor!", p->id);
             p->hasBall = false;
             ball.ownerTeam = -1;
             ball.moving = true;

             ball.dx = (goalX - p->x) / 4; 
             ball.dy = (H / 2 - p->y) / 4;

             if (ball.dx == 0 && ball.dy == 0) ball.dx = (teamID == 0) ? 2 : -2;
             return;
        }

        Player* passTarget = (teamID == 0) ? findBestPassTarget(p, blackTeam, redTeam) : findBestPassTarget(p, redTeam, blackTeam);

        if (passTarget != NULL) {
             updateCommentary("Oyuncu #%d pas atiyor -> #%d! Akisi aciyor.", p->id, passTarget->id);
             p->hasBall = false;
             ball.ownerTeam = -1;
             ball.moving = true;

             double dist = sqrt(pow(passTarget->x - p->x, 2) + pow(passTarget->y - p->y, 2));
             int speed_divisor = (dist > 30) ? 4 : 3; 
             
             ball.dx = (passTarget->x - p->x) / speed_divisor;
             ball.dy = (passTarget->y - p->y) / speed_divisor;

        } else if (rand() % DRIBBLE_CHANCE > 1) {
             targetX = goalX;
             targetY = H / 2;
             dx = (targetX > p->x) ? 1 : (targetX < p->x ? -1 : 0);
             dy = (targetY > p->y) ? 1 : (targetY < p->y ? -1 : 0);
        }

    } else {
        int distToBall = (int)sqrt(pow(ball.x - p->x, 2) + pow(ball.y - p->y, 2));
        bool teammateHasBall = false;
        for(i = 0; i < TEAM_SIZE; i++) {
            if (ownTeam[i].hasBall && ownTeam[i].id != p->id) {
                teammateHasBall = true;
                break;
            }
        }

        if (teammateHasBall) {
             int base_x;
             if (p->id == 1) {
                 base_x = (teamID == 0) ? PITCH_MARGIN + 2 : W - PITCH_MARGIN - 3;
                 targetY = H / 2 + (rand() % 3 - 1);
             } else if (p->id <= 5) { 
                 base_x = (teamID == 0) ? W / 3 : W * 2 / 3;
                 targetY = p->y + (rand() % 5 - 2);
             } else { 
                 base_x = (teamID == 0) ? W * 2 / 3 : W / 3;
                 targetY = p->y + (rand() % 5 - 2);
             }

             targetX = base_x + (rand() % 9 - 4);

        } else {
            if (distToBall > CAPTURE_DISTANCE * 2) {
                 targetX = ball.x;
                 targetY = ball.y;
            } else {
                 int desired_x = ball.x + ((teamID == 0) ? -5 : 5); 
                 targetX = desired_x;
                 targetY = ball.y + (rand() % 5 - 2);
            }
        }

        if (targetX != p->x || targetY != p->y) {
            dx = (targetX > p->x) ? 1 : (targetX < p->x ? -1 : 0);
            dy = (targetY > p->y) ? 1 : (targetY < p->y ? -1 : 0);
        } else if (rand() % 10 == 0) {
             dx = rand() % 3 - 1;
             dy = rand() % 3 - 1;
        }

        for(i = 0; i < TEAM_SIZE; i++) {
            Player *other = &ownTeam[i];
            if (other->id == p->id) continue;

            int distToOther = (int)sqrt(pow(other->x - p->x, 2) + pow(other->y - p->y, 2));

            if (distToOther < 8) { 
                 int force = (8 - distToOther) / 2; 
                 
                 dx += (p->x - other->x > 0) ? force : ((p->x - other->x < 0) ? -force : 0);
                 dy += (p->y - other->y > 0) ? force : ((p->y - other->y < 0) ? -force : 0);
                 
                 if (dx > 1) dx = 1; else if (dx < -1) dx = -1;
                 if (dy > 1) dy = 1; else if (dy < -1) dy = -1;
            }
        }

        if (distToBall <= CAPTURE_DISTANCE && !ball.moving && ball.ownerTeam != teamID) {

            if (ball.ownerTeam != -1) {
                 Player *oldOwner = (ball.ownerTeam == 0) ? &blackTeam[ball.ownerId - 1] : &redTeam[ball.ownerId - 1];
                 if(oldOwner->hasBall) oldOwner->hasBall = false;
            }

            p->hasBall = true;
            ball.ownerTeam = teamID;
            ball.ownerId = p->id;
            updateCommentary("Top kapildi! #%d topa sahip!", p->id);
        }
    }

    nextX = p->x + dx;
    nextY = p->y + dy;

    if (nextX > PITCH_MARGIN + 1 && nextX < W - PITCH_MARGIN - 2) p->x = nextX;
    if (nextY > PITCH_MARGIN + 1 && nextY < H - PITCH_MARGIN - 2) p->y = nextY; 

    if (p->hasBall) {
        int ball_offset_x = (teamID == 0) ? CAPTURE_DISTANCE : -CAPTURE_DISTANCE;

        ball.x = p->x + ball_offset_x;
        ball.y = p->y;
        ball.moving = false;
    }
}

void playerActions() {
    int i;
    for (i = 0; i < TEAM_SIZE; i++) {
        movePlayerAI(&blackTeam[i], 0, redTeam);
    }
    for (i = 0; i < TEAM_SIZE; i++) {
        movePlayerAI(&redTeam[i], 1, blackTeam);
    }
}

void updateBallMovement() {
    bool out = false;
    int t, i;

    if (ball.moving) {
        ball.x += ball.dx;
        ball.y += ball.dy;

        if (rand() % 4 == 0) {
            if (ball.dx > 0) ball.dx--;
            else if (ball.dx < 0) ball.dx++;

            if (ball.dy > 0) ball.dy--;
            else if (ball.dy < 0) ball.dy++;
        }

        Player *teams[] = {blackTeam, redTeam};
        for (t = 0; t < 2; t++) {
            for (i = 0; i < TEAM_SIZE; i++) {
                Player *p = &teams[t][i];
                double distToBall = sqrt(pow(p->x - ball.x, 2) + pow(p->y - ball.y, 2));

                if (distToBall <= CAPTURE_DISTANCE && distToBall > 0) {
                    
                    if (ball.ownerTeam != -1) {
                         Player *oldOwner = (ball.ownerTeam == 0) ? &blackTeam[ball.ownerId - 1] : &redTeam[ball.ownerId - 1];
                         if(oldOwner->hasBall) oldOwner->hasBall = false;
                    }

                    p->hasBall = true;
                    ball.ownerTeam = p->team;
                    ball.ownerId = p->id;
                    ball.moving = false;

                    updateCommentary("Yakalandi! %s takimindan #%d topu kontrol ediyor.",
                                      (p->team == 0) ? "Siyah" : "Kirmizi", p->id);
                    return;
                }
            }
        }

        if (ball.dx == 0 && ball.dy == 0) {
            ball.moving = false;
            ball.ownerTeam = -1;
            updateCommentary("Top sahipsiz kaldi.");
        }
    }

    if (ball.x <= PITCH_MARGIN + 1) {
        ball.x = PITCH_MARGIN + 2;
        if (ball.moving) ball.dx *= -1;
        out = true;
    }
    if (ball.x >= W - PITCH_MARGIN - 2) {
        ball.x = W - PITCH_MARGIN - 3;
        if (ball.moving) ball.dx *= -1;
        out = true;
    }
    if (ball.y <= PITCH_MARGIN + 1) {
        ball.y = PITCH_MARGIN + 2;
        if (ball.moving) ball.dy *= -1;
        out = true;
    }
    if (ball.y >= H - PITCH_MARGIN - 2) { 
        ball.y = H - PITCH_MARGIN - 3;
        if (ball.moving) ball.dy *= -1;
        out = true;
    }
    
    if (out && ball.moving) {
        updateCommentary("Top saha kenarina carpti ve oyun devam ediyor.");
    }
}

void updateMatchTime() {
    static int cycleCounter = 0;
    cycleCounter++;

    if (cycleCounter % 5 == 0) {
        matchTime++;
        cycleCounter = 0;

        if (matchTime == 45 && halfTime == 1) {
            matchActive = false;
            updateCommentary("ILK YARI SONA ERDI! Skor: Siyah %d - %d Kirmizi. (Space ile devam edin)", scoreBlack, scoreRed);
            gamePaused = true;
            halfTime = 2;
        } else if (matchTime == 90 && halfTime == 2) {
            matchActive = false;
            updateCommentary("MAC SONA ERDI! FINAL SKOR: Siyah %d - %d Kirmizi.", scoreBlack, scoreRed);
            gamePaused = true;
        }
    }
}

void handleInput() {
    if (kbhit()) {
        int key = getch();
        if (key == ' ' || key == 32) {
            if (gamePaused) {
                gamePaused = false;
                matchActive = true;
                scoringTeamIndicator = -1;
                
                if (halfTime == 2 && matchTime == 45) {
                    updateCommentary("IKINCI YARI BASLIYOR!");
                }
            }
        }
    }
}


int main(void) {
    int goalResult;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    srand(time(NULL));

    char command[100];
    sprintf(command, "mode con: cols=%d lines=%d", W, CONSOLE_HEIGHT);
    system(command); 
    
    system("color 2F"); 
    system("chcp 437 > nul");

    hideCursor();

    initTeams();

    while (1) {

        handleInput();

        if (matchActive && !gamePaused) {
            
            updateBallMovement();
            
            goalResult = checkGoal(); 

            if (goalResult != -1) {
                resetAfterGoal(goalResult);
            }
            
            playerActions();
            updateMatchTime();
        }

        clearMap();
        drawPitch();
        drawAllPlayers();
        printMap();

        if (!matchActive && matchTime >= 90 && halfTime == 2) {
             Sleep(5000);
             break;
        }

        Sleep(150);
    }

    COORD finalCoord = {0, PITCH_START_Y + H + COMMENTARY_OFFSET + 1}; 
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), finalCoord);
    
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_DEFAULT);
    
    printf("\nMAC BITTI. Final Skoru: Siyah %d - %d Kirmizi.", scoreBlack, scoreRed); 
    printf("\nDevam etmek icin herhangi bir tusa basin...");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
    getchar();

    return 0;

}
