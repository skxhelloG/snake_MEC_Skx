#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <bios.h>
#include <stdlib.h>

#define MAXWIDTH    (79)
#define MAXHEIGHT   (24)
#define INITLEN     (5)

char newHeadFangXiang[4] = { '^', 'v', '<', '>'};
int snakeArrIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)] = {0};
int obstacleLen = 0;
int snakeLen = 0;
int scoresOne = 0;
int scoresTwo = 0;

int headerIndexOne = 0, tailIndexOne = 0;
int zhuizhongIndexOne[(MAXWIDTH-2)*(MAXHEIGHT-2)] = {0};
int zhuiIndexOne = 0;
int dangTailIndexOne = 0;

int headerIndexTwo = 0, tailIndexTwo = 0;
int zhuizhongIndexTwo[(MAXWIDTH-2)*(MAXHEIGHT-2)] = {0};
int zhuiIndexTwo = 0;
int dangTailIndexTwo = 0;

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
}arrFood[3] = { {05, 4, 1},{06, 5, 2},{03, 6, 5} };

struct{
    char type;
    int index;
}globalMap[MAXHEIGHT+1][MAXWIDTH];

struct{
    int x;
    int y;
}snakeMap[ (MAXWIDTH-2)*(MAXHEIGHT-2) ], scoresPostionOne, scoresPostionTwo;

int panDuanFood(int x, int y);
void dieOne();
void dieTwo();
char nextDirectionTwo(char ch, char directionOld);
char nextDirectionOne(char ch, char directionOld);
void cursor (unsigned mode);
void createFood();
void xiugaisnakeArrIndex(int fen, int site);
void moveTwo(char directionTwo);
void moveOne(char directionOne);
void move(char directionOne, char directionTwo);
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

void dieOne(){
    int xCenter = MAXHEIGHT%2==0 ? MAXHEIGHT/2 : MAXHEIGHT/2+1;
    int yCenter = MAXWIDTH%2==0 ? MAXWIDTH/2 : MAXWIDTH/2+1;

    gotoxy(yCenter-13, xCenter);
    textcolor(0X04);
    if (scoresOne-50 > scoresTwo) {
    	cprintf("Green Snake Win! Game Over!");
    } else if (scoresOne-50 < scoresTwo) {
    	cprintf("Purple Snake Win! Game Over!");
    } else {
    	gotoxy(yCenter-8, xCenter);
    	cprintf("DEUCE! Game Over!");
    }
    getch();
    exit(0);
}

void dieTwo(){
    int xCenter = MAXHEIGHT%2==0 ? MAXHEIGHT/2 : MAXHEIGHT/2+1;
    int yCenter = MAXWIDTH%2==0 ? MAXWIDTH/2 : MAXWIDTH/2+1;

    gotoxy(yCenter-13, xCenter);
    textcolor(0X04);
    if (scoresOne > scoresTwo-50) {
    	cprintf("Green Snake Win! Game Over!");
    } else if (scoresOne < scoresTwo-50) {
    	cprintf("Purple Snake Win! Game Over!");
    } else {
    	gotoxy(yCenter-8, xCenter);
    	cprintf("DEUCE! Game Over!");
    }
    getch();
    exit(0);
}

char nextDirectionTwo(char ch, char directionOld){
    int sum = ch+directionOld;
    ch = tolower(ch);
    if( (ch=='8' || ch=='4' || ch=='5' || ch=='6') && sum!=109 && sum!=106 ){
        return ch;
    }else{
        return directionOld;
    }
}

char nextDirectionOne(char ch, char directionOld){
    int sum = ch+directionOld;
    ch = tolower(ch);
    if( (ch=='w' || ch=='a' || ch=='s' || ch=='d') && sum!=197 && sum!=234 ){
        return ch;
    }else{
        return directionOld;
    }
}

void cursor (unsigned mode) {
    union REGS r;
    r.h.ah=0x01;
    if(mode%=3){r.h.cl=7;r.h.ch=mode==1?6:0;}
    else r.x.cx=0x2020;
    int86(0x10,&r,&r);
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

void xiugaisnakeArrIndex(int fen, int site) {
    int center = (MAXWIDTH-2)*(MAXHEIGHT-2)-snakeLen;
    int i, j, tem, headIndex, tailIndex;

    if (site == 1) {
        headIndex = headerIndexOne;
        tailIndex = tailIndexOne;
    } else if (site == 2) {
        headIndex = headerIndexTwo;
        tailIndex = tailIndexTwo;
    }
    for (i = obstacleLen; i < center; i++) {
        if (snakeArrIndex[i] == headIndex) {
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

void moveTwo(char directionTwo) {
    int newHeaderTwoX, newHeaderTwoY;
    int oldTailTwoX, oldTailTwoY;
    int fen, siteTwo = 2;
    char fangXiangFu;

    switch(directionTwo){
        case '8':
            newHeaderTwoX = snakeMap[headerIndexTwo].x-1;
            newHeaderTwoY = snakeMap[headerIndexTwo].y;
            fangXiangFu = newHeadFangXiang[0];
            break;
        case '5':
            newHeaderTwoX = snakeMap[headerIndexTwo].x+1;
            newHeaderTwoY = snakeMap[headerIndexTwo].y;
            fangXiangFu = newHeadFangXiang[1];
            break;
        case '4':
            newHeaderTwoX = snakeMap[headerIndexTwo].x;
            newHeaderTwoY = snakeMap[headerIndexTwo].y-1;
            fangXiangFu = newHeadFangXiang[2];
            break;
        case '6':
            newHeaderTwoX = snakeMap[headerIndexTwo].x;
            newHeaderTwoY = snakeMap[headerIndexTwo].y+1;
            fangXiangFu = newHeadFangXiang[3];
            break;
    }

    gotoxy(snakeMap[headerIndexTwo].y, snakeMap[headerIndexTwo].x);
    textcolor(0X0D);
    cprintf("%c", charSnake.ch);

    if (globalMap[newHeaderTwoX][newHeaderTwoY].index == tailIndexTwo) {
        oldTailTwoX = snakeMap[tailIndexTwo].x;
        oldTailTwoY = snakeMap[tailIndexTwo].y;
        globalMap[oldTailTwoX][oldTailTwoY].type = charBg.type;
    }

    if ( globalMap[newHeaderTwoX][newHeaderTwoY].type == charBorder.type || 
         globalMap[newHeaderTwoX][newHeaderTwoY].type == charSnake.type  ) {
         gotoxy(snakeMap[headerIndexTwo].y, snakeMap[headerIndexTwo].x);
         textcolor(0X04);
         cprintf("%c", fangXiangFu);
         dieTwo();
    } else if ( (fen = panDuanFood(newHeaderTwoX, newHeaderTwoY)) != 0) {    
        createFood();        
        gotoxy(scoresPostionTwo.y, scoresPostionTwo.x);
        textcolor(0X02);
        scoresTwo += fen;
        cprintf("%d", scoresTwo);
    } else  {
        oldTailTwoX = snakeMap[tailIndexTwo].x;
        oldTailTwoY = snakeMap[tailIndexTwo].y;

        gotoxy(oldTailTwoY, oldTailTwoX);
        textcolor(0X01);
        cprintf("%c", charBg.ch);
        globalMap[oldTailTwoX][oldTailTwoY].type = charBg.type;

        if (dangTailIndexTwo == (MAXWIDTH-2)*(MAXHEIGHT-2) -1) {
            dangTailIndexTwo = -1;
        }
        dangTailIndexTwo++;
        tailIndexTwo = zhuizhongIndexTwo[dangTailIndexTwo];
    }

    headerIndexTwo = globalMap[newHeaderTwoX][newHeaderTwoY].index;
    xiugaisnakeArrIndex(fen, siteTwo);
    if (zhuiIndexTwo == (MAXWIDTH-2)*(MAXHEIGHT-2) ){
        zhuiIndexTwo = 0;
    } 
    zhuizhongIndexTwo[zhuiIndexTwo] = headerIndexTwo;
    zhuiIndexTwo++;

    gotoxy(newHeaderTwoY, newHeaderTwoX);
    textcolor(0X04);
    cprintf("%c", fangXiangFu);
    globalMap[newHeaderTwoX][newHeaderTwoY].type = charSnake.type;
}

void moveOne(char directionOne) {
    int newHeaderOneX, newHeaderOneY;
    int oldTailOneX, oldTailOneY;
    int fen;
    int siteOne = 1;
    char fangXiangFu;

    switch(directionOne){
        case 'w':
            newHeaderOneX = snakeMap[headerIndexOne].x-1;
            newHeaderOneY = snakeMap[headerIndexOne].y;
            fangXiangFu = newHeadFangXiang[0];
            break;
        case 's':
            newHeaderOneX = snakeMap[headerIndexOne].x+1;
            newHeaderOneY = snakeMap[headerIndexOne].y;
            fangXiangFu = newHeadFangXiang[1];
            break;
        case 'a':
            newHeaderOneX = snakeMap[headerIndexOne].x;
            newHeaderOneY = snakeMap[headerIndexOne].y-1;
            fangXiangFu = newHeadFangXiang[2];
            break;
        case 'd':
            newHeaderOneX = snakeMap[headerIndexOne].x;
            newHeaderOneY = snakeMap[headerIndexOne].y+1;
            fangXiangFu = newHeadFangXiang[3];
            break;
    }

    gotoxy(snakeMap[headerIndexOne].y, snakeMap[headerIndexOne].x);
    textcolor(0X02);
    cprintf("%c", charSnake.ch);

    if (globalMap[newHeaderOneX][newHeaderOneY].index == tailIndexOne) {
        oldTailOneX = snakeMap[tailIndexOne].x;
        oldTailOneY = snakeMap[tailIndexOne].y;
        globalMap[oldTailOneX][oldTailOneY].type = charBg.type;
    }

    if ( globalMap[newHeaderOneX][newHeaderOneY].type == charBorder.type || 
         globalMap[newHeaderOneX][newHeaderOneY].type == charSnake.type  ) {
         gotoxy(snakeMap[headerIndexOne].y, snakeMap[headerIndexOne].x);
         textcolor(0X04);
         cprintf("%c", fangXiangFu);
         dieOne();
    } else if ( (fen = panDuanFood(newHeaderOneX, newHeaderOneY)) != 0) {
        createFood();
        gotoxy(scoresPostionOne.y, scoresPostionOne.x);
        textcolor(0X02);
        scoresOne += fen;
        cprintf("%d", scoresOne);
    } else  {
        oldTailOneX = snakeMap[tailIndexOne].x;
        oldTailOneY = snakeMap[tailIndexOne].y;

        gotoxy(oldTailOneY, oldTailOneX);
        textcolor(0X01);
        cprintf("%c", charBg.ch);
        globalMap[oldTailOneX][oldTailOneY].type = charBg.type;

        if (dangTailIndexOne == (MAXWIDTH-2)*(MAXHEIGHT-2) -1) {
            dangTailIndexOne = -1;
        }
        dangTailIndexOne++;
        tailIndexOne = zhuizhongIndexOne[dangTailIndexOne];
    }

    headerIndexOne = globalMap[newHeaderOneX][newHeaderOneY].index;
    xiugaisnakeArrIndex(fen, siteOne);
    if (zhuiIndexOne == (MAXWIDTH-2)*(MAXHEIGHT-2) ){
        zhuiIndexOne = 0;
    } 
    zhuizhongIndexOne[zhuiIndexOne] = headerIndexOne;
    zhuiIndexOne++;

    gotoxy(newHeaderOneY, newHeaderOneX);
    textcolor(0X04);
    cprintf("%c", fangXiangFu);
    globalMap[newHeaderOneX][newHeaderOneY].type = charSnake.type;
}

void move(char directionOne, char directionTwo) {
    moveOne(directionOne);
    moveTwo(directionTwo);
}

void init(){
    int xCenter = MAXHEIGHT%2==0 ? MAXHEIGHT/2 : MAXHEIGHT/2+1;
    int yCenterOne = 3;
    int yCenterTwo = MAXWIDTH-2;
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
    for(x = (MAXHEIGHT/2)/2; x <= MAXHEIGHT-(MAXHEIGHT/2)/2; x++){
        for(y = (MAXWIDTH/2)/2; y <= MAXWIDTH-(MAXWIDTH/2)/2; y++){
            if( (x==(MAXHEIGHT/2)/2 || x==(MAXHEIGHT/2)/2+1 || x==(MAXHEIGHT/2)/2+2 ||
                 x==MAXHEIGHT-(MAXHEIGHT/2)/2-2 || x==MAXHEIGHT-(MAXHEIGHT/2)/2-1 || x==MAXHEIGHT-(MAXHEIGHT/2)/2 ) && 
                (y==(MAXWIDTH/2)/2 || y==(MAXWIDTH/2)/2+1 || y==(MAXWIDTH/2)/2+2 ||
                 y==MAXWIDTH-(MAXWIDTH/2)/2-2 || y==MAXWIDTH-(MAXWIDTH/2)/2-1 || y==MAXWIDTH-(MAXWIDTH/2)/2) ){

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
        globalMap[xCenter+count][yCenterOne].type = charSnake.type;
        gotoxy(yCenterOne, xCenter+count);        
        cprintf("%c", charSnake.ch);

        index = (xCenter+count-2)*(MAXWIDTH-2)+(yCenterOne-2);
        tem = snakeArrIndex[index];
        snakeArrIndex[index] = snakeArrIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)-1-snakeLen];
        snakeArrIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)-1-snakeLen] = tem;
        snakeLen++;

        zhuizhongIndexOne[zhuiIndexOne] = index;
        zhuiIndexOne++;
    }
    headerIndexOne = zhuizhongIndexOne[zhuiIndexOne-1];
    tailIndexOne = zhuizhongIndexOne[0];

    gotoxy(snakeMap[headerIndexOne].y, snakeMap[headerIndexOne].x);
    textcolor(0X04);
    cprintf("%c", newHeadFangXiang[1]);


    textcolor(0X0D);
    for (count = (-INITLEN/2); count <= (INITLEN%2==0 ? INITLEN/2-1 : INITLEN/2); count++) {
        globalMap[xCenter+count][yCenterTwo].type = charSnake.type;
        gotoxy(yCenterTwo, xCenter+count);        
        cprintf("%c", charSnake.ch);

        index = (xCenter+count-2)*(MAXWIDTH-2)+(yCenterTwo-2);
        tem = snakeArrIndex[index];
        snakeArrIndex[index] = snakeArrIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)-1-snakeLen];
        snakeArrIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)-1-snakeLen] = tem;
        snakeLen++;

        zhuizhongIndexTwo[zhuiIndexTwo] = index;
        zhuiIndexTwo++;
    }
    headerIndexTwo = zhuizhongIndexTwo[zhuiIndexTwo-1];
    tailIndexTwo = zhuizhongIndexTwo[0];

    gotoxy(snakeMap[headerIndexTwo].y, snakeMap[headerIndexTwo].x);
    textcolor(0X04);
    cprintf("%c", newHeadFangXiang[1]);

    for (count = 0; count < 10; count++) {
    	createFood();
    }

    textcolor(0X04);
    gotoxy(1, MAXHEIGHT+1);
    cprintf("Direction-Key:wasd");
    gotoxy(MAXWIDTH-18, MAXHEIGHT+1);
    cprintf("Direction-Key:8456");
    gotoxy(MAXWIDTH/2, MAXHEIGHT+1);
    cprintf(":");
    scoresPostionOne.x = MAXHEIGHT+1;
    scoresPostionOne.y = MAXWIDTH/2-4;
    gotoxy(scoresPostionOne.y, scoresPostionOne.x);
    textcolor(0X02);
    cprintf("%d", scoresOne);
    scoresPostionTwo.x = MAXHEIGHT+1;
    scoresPostionTwo.y = MAXWIDTH/2+4;
    gotoxy(scoresPostionTwo.y, scoresPostionTwo.x);
    cprintf("%d", scoresTwo);
}

int main(){
    char charInput;
    char directionOne = 's';
    char directionTwo = '5';
    
    init();
    charInput = tolower(getch());    
    directionOne = nextDirectionOne(charInput, directionOne);
    directionTwo = nextDirectionTwo(charInput, directionTwo);

    while(1){
        if(bioskey(1)){
            charInput = bioskey(0);
            directionTwo = nextDirectionTwo(charInput, directionTwo);
            directionOne = nextDirectionOne(charInput, directionOne);
        } 
        move(directionOne, directionTwo);
        delay(400);          
    }

    getch();
    return 0;
}