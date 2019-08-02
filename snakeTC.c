#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <bios.h>
#include <stdlib.h>

#define MAXWIDTH    (79)
#define MAXHEIGHT   (24)
#define INITLEN     (5)

int snakeArrIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)] = {0};
int zhuizhongIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)] = {0};
int zhuiIndex = 0;
int dangTailIndex = 0;
int obstacleLen = 0;
int snakeLen = 0;
int scores = 0;
int headerIndex = 0, tailIndex = 0;
char newHeadFangXiang[4] = { '^', 'v', '<', '>'};

struct{
    char ch;
    char type;
}
charBorder = {'#', 1},
charBg = {'+', 2},
charSnake = {04, 3};

struct {
    char ch;
    char type;
    int fen;
}arrFood[3] = { {05, 4, 1},{06, 5, 5},{03, 6, 10} };

struct{
    char type;
    int index;
}globalMap[MAXHEIGHT+1][MAXWIDTH];

struct{
    int x;
    int y;
}snakeMap[ (MAXWIDTH-2)*(MAXHEIGHT-2) ], scoresPostion;


int panDuanFood(int x, int y);
void die();
char nextDirection(char ch, char directionOld);
char pause();
void cursor (unsigned mode);
void sudu(char charInput, char *charP);
void createFood();
void xiugaisnakeArrIndex(int fen);
void move(char direction);
void init();

int panDuanFood(int x, int y) {
    int i;

    for (i = 0; i < 3; i++) {
        if ( globalMap[x][y].type == arrFood[i].type) {
            return arrFood[i].fen;
        }
    }
    return 0;
}

void die(){
    int xCenter = MAXHEIGHT%2==0 ? MAXHEIGHT/2 : MAXHEIGHT/2+1;
    int yCenter = MAXWIDTH%2==0 ? MAXWIDTH/2 : MAXWIDTH/2+1;

    gotoxy(yCenter-9, xCenter);
    textcolor(0X04);
    cprintf("You die! Game Over!");
    getch();
    exit(0);
}

char nextDirection(char ch, char directionOld){
    int sum = ch+directionOld;
    ch = tolower(ch);
    if( (ch=='w' || ch=='a' || ch=='s' || ch=='d') && sum!=197 && sum!=234 ){
        return ch;
    }else{
        return directionOld;
    }
}

char pause(){
    return getch();
}

void cursor (unsigned mode) {
    union REGS r;
    r.h.ah=0x01;
    if(mode%=3){r.h.cl=7;r.h.ch=mode==1?6:0;}
    else r.x.cx=0x2020;
    int86(0x10,&r,&r);
}

void sudu(char charInput, char *charP) {
    int static ceng = 1;

    if (charInput == '9') {
            ceng++;
            ceng = ceng>3 ? 3 : ceng;
            *charP = '0';
    }
    else if (charInput == '3') {
        ceng--;
        ceng = ceng<1 ? 1 : ceng;
        *charP = '0';
    }
    if (ceng == 1) {
        delay(500);
    }
    else if (ceng == 2) {
        delay(250);
    }
    else if (ceng == 3) {
        delay(100);
    }
}

void createFood(){
    char gailuFoodArr[10] = {0};
    int offset, index, center, gailuIndex;
    int  x, y, i, j;
    char ch;

    srand((unsigned)time(NULL));
    center = (MAXWIDTH-2)*(MAXHEIGHT-2)-obstacleLen-snakeLen;
    offset = rand()%center;
    index = snakeArrIndex[ (obstacleLen+offset) ];
    x = snakeMap[index].x;
    y = snakeMap[index].y;
    ch = globalMap[x][y].type;
    while(ch==arrFood[0].type || ch==arrFood[1].type || ch==arrFood[2].type) {
        offset = rand()%center;
        index = snakeArrIndex[ (obstacleLen+offset) ];
        x = snakeMap[index].x;
        y = snakeMap[index].y;
        ch = globalMap[x][y].type;
    }
    gailuIndex = rand()%10;

    for (i = 0; i < 10; i++) {
        if (i <= 4) {
            gailuFoodArr[i] = arrFood[0].ch;
        }
        else if (i <= 7) {
            gailuFoodArr[i] = arrFood[1].ch;
        }
        else
            gailuFoodArr[i] = arrFood[2].ch;
    }

    for (i = 0; i < 10; i++) {
        if (gailuFoodArr[gailuIndex] == arrFood[0].ch) {
            j = 0;
            break;
        }
        else if (gailuFoodArr[gailuIndex] == arrFood[1].ch) {
            j = 1;
            break;
        }
        else if (gailuFoodArr[gailuIndex] == arrFood[2].ch){
            j = 2;
            break;
        }
    }

    gotoxy(y, x);
    textcolor(0X0E);
    cprintf("%c", gailuFoodArr[gailuIndex]);
    globalMap[x][y].type=arrFood[j].type;
}

void xiugaisnakeArrIndex(int fen) {
    int center = (MAXWIDTH-2)*(MAXHEIGHT-2)-snakeLen;
    int i, j, tem;

    for (i = obstacleLen; i < center; i++) {
        if (snakeArrIndex[i] == headerIndex) {
            break;
        }
    }
    if (fen != 0) {
        tem = snakeArrIndex[i];
        snakeArrIndex[i] = snakeArrIndex[ (center-1) ];
        snakeArrIndex[ (center-1) ] = tem;
        ++snakeLen;
    } else {
        for (j = center; j < (MAXWIDTH-2)*(MAXHEIGHT-2); j++) {
            if (snakeArrIndex[j] == tailIndex) {
                break;
            }
        }
        tem = snakeArrIndex[i];
        snakeArrIndex[i] = snakeArrIndex[j];
        snakeArrIndex[j] = tem;
    }

}


void move(char direction){
    int newHeaderX, newHeaderY;
    int oldTailX, oldTailY;
    int fen;
    char fangXiangFu;

    switch(direction){
        case 'w':
            newHeaderX = snakeMap[headerIndex].x-1;
            newHeaderY = snakeMap[headerIndex].y;
            fangXiangFu = newHeadFangXiang[0];
            break;
        case 's':
            newHeaderX = snakeMap[headerIndex].x+1;
            newHeaderY = snakeMap[headerIndex].y;
            fangXiangFu = newHeadFangXiang[1];
            break;
        case 'a':
            newHeaderX = snakeMap[headerIndex].x;
            newHeaderY = snakeMap[headerIndex].y-1;
            fangXiangFu = newHeadFangXiang[2];
            break;
        case 'd':
            newHeaderX = snakeMap[headerIndex].x;
            newHeaderY = snakeMap[headerIndex].y+1;
            fangXiangFu = newHeadFangXiang[3];
            break;
    }

    gotoxy(snakeMap[headerIndex].y, snakeMap[headerIndex].x);
    textcolor(0X02);
    cprintf("%c", charSnake.ch);

    if (globalMap[newHeaderX][newHeaderY].index == tailIndex) {
        oldTailX = snakeMap[tailIndex].x;
        oldTailY = snakeMap[tailIndex].y;
        globalMap[oldTailX][oldTailY].type = charBg.type;
    }

    if ( globalMap[newHeaderX][newHeaderY].type == charBorder.type || 
         globalMap[newHeaderX][newHeaderY].type == charSnake.type  ) {
         gotoxy(snakeMap[headerIndex].y, snakeMap[headerIndex].x);
         textcolor(0X04);
         cprintf("%c", fangXiangFu);
         die();
    } else if ( (fen = panDuanFood(newHeaderX, newHeaderY)) != 0) {
        createFood();
        gotoxy(scoresPostion.y, scoresPostion.x);
        textcolor(0X02);
        scores += fen;
        cprintf("%d", scores);
    } else  {
        oldTailX = snakeMap[tailIndex].x;
        oldTailY = snakeMap[tailIndex].y;

        gotoxy(oldTailY, oldTailX);
        textcolor(0X01);
        cprintf("%c", charBg.ch);
        globalMap[oldTailX][oldTailY].type = charBg.type;

        if (dangTailIndex == (MAXWIDTH-2)*(MAXHEIGHT-2) -1) {
            dangTailIndex = -1;
        }
        dangTailIndex++;
        tailIndex = zhuizhongIndex[dangTailIndex];
    }

    headerIndex = globalMap[newHeaderX][newHeaderY].index;
    xiugaisnakeArrIndex(fen);
    if (zhuiIndex == (MAXWIDTH-2)*(MAXHEIGHT-2) ){
        zhuiIndex = 0;
    } 
    zhuizhongIndex[zhuiIndex] = headerIndex;
    zhuiIndex++;

    gotoxy(newHeaderY, newHeaderX);
    textcolor(0X04);
    cprintf("%c", fangXiangFu);
    globalMap[newHeaderX][newHeaderY].type = charSnake.type;
}

void init(){
    int xCenter = MAXHEIGHT%2==0 ? MAXHEIGHT/2 : MAXHEIGHT/2+1;
    int yCenter = MAXWIDTH%2==0 ? MAXWIDTH/2 : MAXWIDTH/2+1;
    int tem, x, y, index, count;

    cursor(0);
    clrscr();
    if(MAXWIDTH < 16){
        gotoxy(15, 10);
        textcolor(0X04);
        cprintf("'MAXWIDTH' is too small!\r\n");
        getch();
        exit(0);
    }

    for(x = 1; x <= MAXHEIGHT; x++){
        for(y = 1; y <= MAXWIDTH; y++){
            if(y==1 || y==MAXWIDTH || x==1 || x==MAXHEIGHT){
                globalMap[x][y].type = charBorder.type;
                textcolor(0X04);
                cprintf("%c", charBorder.ch);
            }else{
                index = (x-2)*(MAXWIDTH-2)+(y-2);
                snakeMap[index].x= x;
                snakeMap[index].y= y;
                globalMap[x][y].type = charBg.type;
                globalMap[x][y].index = index;
                snakeArrIndex[index] = index;
                textcolor(0X01);
                cprintf("%c", charBg.ch);
            }
        }
        cprintf("\r\n");
    }

    textcolor(0X04);
    for(x = 7; x <= MAXHEIGHT-2; x++){
        for(y = 7; y <= MAXWIDTH-2; y++){
            if( (x==7 || x==8 || x==MAXHEIGHT-6 || x==MAXHEIGHT-7) && 
                (y==7 || y==8 || y==MAXWIDTH -6 || y==MAXWIDTH -7) ){
                if (x == 9) {
                    x = MAXHEIGHT-5;
                    break;
                }
                if (y == 9) {
                    y = MAXWIDTH-5;
                }

                globalMap[x][y].type = charBorder.type;
                gotoxy(y, x);
                cprintf("%c", charBorder.ch);
                index = (x-2)*(MAXWIDTH-2)+(y-2);
                tem = snakeArrIndex[index];
                snakeArrIndex[index] = snakeArrIndex[obstacleLen];
                snakeArrIndex[obstacleLen] = tem;
                obstacleLen++;
            }
        }
    }

    textcolor(0X02);
    for (count = (-INITLEN/2); count <= (INITLEN%2==0 ? INITLEN/2-1 : INITLEN/2); count++) {
        globalMap[xCenter][yCenter+count].type = charSnake.type;
        gotoxy(yCenter+count, xCenter);        
        cprintf("%c", charSnake.ch);

        index = (xCenter-2)*(MAXWIDTH-2)+(yCenter+count-2);
        tem = snakeArrIndex[index];
        snakeArrIndex[index] = snakeArrIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)-1-snakeLen];
        snakeArrIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)-1-snakeLen] = tem;
        snakeLen++;

        zhuizhongIndex[zhuiIndex] = index;
        zhuiIndex++;
    }
    headerIndex = zhuizhongIndex[zhuiIndex-1];
    tailIndex = zhuizhongIndex[0];

    gotoxy(snakeMap[headerIndex].y, snakeMap[headerIndex].x);
    textcolor(0X04);
    cprintf("%c", newHeadFangXiang[3]);


    for (x = 0; x < 5; x++) {
        createFood();
    }
    gotoxy(MAXWIDTH/2-7, MAXHEIGHT+1);
    textcolor(0X04);
    cprintf("Fen : ");
    scoresPostion.x = MAXHEIGHT+1;
    scoresPostion.y = MAXWIDTH/2;
    gotoxy(scoresPostion.y, scoresPostion.x);
    textcolor(0X02);
    cprintf("%d", scores);
}

int main(){
    char charInput, direction = 'd';
    char *charInputP = &charInput;
    
    init();
    charInput = tolower(getch());
    direction = nextDirection(charInput, direction);

    while(1){
        if(bioskey(1)){
            charInput = bioskey(0);
            if(charInput == ' '){
                charInput = pause();
            }
            direction = nextDirection(charInput, direction);
        }   
        move(direction);       
        sudu(charInput, charInputP);
    }

    getch();
    return 0;
}
