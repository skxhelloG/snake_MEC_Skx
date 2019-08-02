#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <bios.h>
#include <stdlib.h>

#define MAXWIDTH    (79)//最大列
#define MAXHEIGHT   (24)//最大行
#define INITLEN     (5) //蛇的初始长度
//行列的大小是需要适应屏幕的，太大太小都不行
//列的话对对设置的障碍可能有影响(具体看初始化函数里)，
//列台大对初始化大地图外的信息输出有影响(最大54，最小18)
//行最大24

//大地图：所有需要管理的屏幕点，       大小：（MAXWIDTH*MAXHEIGHT）
//小地图：大地图中除了边框点的其他点，  大小：(MAXWIDTH-2)*(MAXHEIGHT-2)

//全局变量都初始化(不初始化通常不会有问题，但这是通常)
int snakeArrIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)] = {0}; 
    //记录小地图中所有点的下标，初始化后把障碍下标与数组前面存的下标交换，把蛇身的下标与数组后面存的下标交换
    //结果：数组分三部分信息：障碍的下标、背景的下标、蛇的下标
    //移动过程：移动时若吃到食物，遍历数组第二段找到新蛇头点的下标，与第二段最后一个元素的值交换，，蛇长snakeLen加1用蛇长来识别第三代的长度
    //          若没有吃到食物，遍历数组第二段找到新蛇头点的下标，遍历数组第三段找到旧蛇尾点的下标，两者的值交换，蛇长不变
    //作用：随机创建食物时食物只能创建到背景中，让得到的随机数与第二段长度取余，
    //      加上障碍数obstacleLen得到第二段对应的！！！数组下标，然后取出对应的值，该值是随机的背景下标
int zhuizhongIndex[(MAXWIDTH-2)*(MAXHEIGHT-2)] = {0};
    //作用：蛇头每移动一次，用这个数组依次记录下新蛇头的下标值，蛇尾根据每次移动是否需要移动，若需要，依次从数组中取数据(蛇头移动过的位置下标)
    //最终目的：定位蛇尾的移动
    //1：该数组可循环使用：数组的最后一个元素记录了新蛇头下标后，下一次移动的新蛇头下标存到数组第一个元素，
    //2：1牵扯的第二个注意点是，该数组的长度为蛇能达到的最长
int zhuiIndex = 0;      //跟踪蛇头
int dangTailIndex = 0;  // 跟踪蛇尾
int obstacleLen = 0;    //障碍数
int snakeLen = 0;       //蛇长
int scores = 0;         //得分
int headerIndex = 0；    //蛇头下标
int tailIndex = 0;      //蛇尾下标
char newHeadFangXiang[4] = { '^', 'v', '<', '>'};   //蛇头图标

struct{
    char ch;
    char type;
}
charBorder = {'#', 1},  //边框
charBg = {'+', 2},      //背景
charSnake = {04, 3};   //蛇身

struct {
    char ch;
    char type;
    int fen;
}arrFood[3] = { {05, 4, 1},{06, 5, 5},{03, 6, 10} };     //食物；05,06,03都是十六进制的，有对应的字符上同04
//大地图中每个点都有信息
//      ：形状 ――可更改
//      ：类型 边框(障碍类型同边框)、蛇身、背景、食物(三种)  注意：每种类型都是不同的(蛇只是形状一样)


struct{
    char type;
    int index;
}globalMap[MAXHEIGHT+1][MAXWIDTH];  
    //大地图 ：由坐标得到对应的下标，type初始化后，记录每个点的类型
    //注：大小本来应该是[MAXHEIGHT][MAXWIDTH]，但是TurboC2.0 中，坐标从(1,1)开始――我不清楚有没有影响
    //[MAXHEIGHT][MAXWIDTH]的运行下，存在bug，最后一行存在四五个点，蛇可以穿过去，从小地图里某个点出来(咦，隧道诶)
    //我[MAXHEIGHT+1][MAXWIDTH]，消除了最后一行的影响，注意，不要对MAXHEIGHT修改达到此目的

struct{
    int x;
    int y;
}snakeMap[ (MAXWIDTH-2)*(MAXHEIGHT-2) ], scoresPostion;     //小地图：由下标得到对应的坐标
//scoresPostion:记录分数的点坐标(不在大地图内)

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

//判断是否吃到食物，否：返回0，是：返回食物类型对应的分数
int panDuanFood(int x, int y) {      //参数是新蛇头的坐标
    int i;

    for (i = 0; i < 3; i++) {
        if ( globalMap[x][y].type == arrFood[i].type) {
            return arrFood[i].fen;
        }
    }
    return 0;
}

//结束程序
void die(){
    int xCenter = MAXHEIGHT%2==0 ? MAXHEIGHT/2 : MAXHEIGHT/2+1;
    int yCenter = MAXWIDTH%2==0 ? MAXWIDTH/2 : MAXWIDTH/2+1;

    gotoxy(yCenter-9, xCenter);
    textcolor(0X04);
    cprintf("You die! Game Over!");
    getch();
    exit(0);
}

//判断移动的方向，返回值：方向('w','a'.'s','d')
//参数：ch是键盘新输入的信息；directionOld是上一次输入的信息
char nextDirection(char ch, char directionOld){
    int sum = ch+directionOld;
    ch = tolower(ch);//必须将其小写化，因为主函数里传的参数可能是大写的，它可以写到主函数去
    if( (ch=='w' || ch=='a' || ch=='s' || ch=='d') && sum!=197 && sum!=234 ){
        return ch;
    }else{
        return directionOld;
    }
}

//暂停
char pause(){
    return getch();
}

//消除光标(网上找的)――在初始化函数里调用了一次cursor(0)
void cursor (unsigned mode) {
    union REGS r;
    r.h.ah=0x01;
    if(mode%=3){r.h.cl=7;r.h.ch=mode==1?6:0;}
    else r.x.cx=0x2020;
    int86(0x10,&r,&r);
}

//速度
//参数：charInput是最后一次的输入值；charP对主函数的charInput修改的权限
/void sudu(char charInput, char *charP) {
    int static ceng = 1;

    if (charInput == '9') {
            ceng++;
            ceng = ceng>3 ? 3 : ceng;
            *charP = '0';//这里若不修改那第二次调用的时候charInput的值任然是'9'(下同)，结果，直观上输入一次'9'就从最慢到最快
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


//创建食物
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
    while(ch==arrFood[0].type || ch==arrFood[1].type || ch==arrFood[2].type) {  //判断得到的随机点的类型：三种食物，条件为真重新取随机数
                                                                                //背景类型，为假，下标合理
        offset = rand()%center;
        index = snakeArrIndex[ (obstacleLen+offset) ];
        x = snakeMap[index].x;
        y = snakeMap[index].y;
        ch = globalMap[x][y].type;
    }
    gailuIndex = rand()%10;

    for (i = 0; i < 10; i++) {//初始化存储三种食物的数组：比例不同
        if (i <= 4) {
            gailuFoodArr[i] = arrFood[0].ch;
        }
        else if (i <= 7) {
            gailuFoodArr[i] = arrFood[1].ch;
        }
        else
            gailuFoodArr[i] = arrFood[2].ch;
    }

    for (i = 0; i < 10; i++) {  //判断下标对应的食物类型，作用：该点的类型和后期吃到该点得到的分数
        if (gailuFoodArr[gailuIndex] == arrFood[0].ch) {
            j = 0;
            break;
        }
        else if (gailuFoodArr[gailuIndex] == arrFood[1].ch) {
            j = 1;
            break;
        }
        else {
            j = 2;
            break;
        }
    }

    gotoxy(y, x);
    textcolor(0X0E);
    cprintf("%c", gailuFoodArr[gailuIndex]);
    globalMap[x][y].type=arrFood[j].type;
}

//对snakeArrIndex数组的修改
void xiugaisnakeArrIndex(int fen) {
    int center = (MAXWIDTH-2)*(MAXHEIGHT-2)-snakeLen;
    int i, j, tem;

    for (i = obstacleLen; i < center; i++) {
        if (snakeArrIndex[i] == headerIndex) {
            break;
        }
    }
    if (fen != 0) { //新点是食物
        tem = snakeArrIndex[i];
        snakeArrIndex[i] = snakeArrIndex[ (center-1) ];
        snakeArrIndex[ (center-1) ] = tem;
        ++snakeLen;
    } else {    //新点是背景
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
        tailIndex = zhuizhongIndex[dangTailIndex];//蛇尾的改变；每次移动完dangTailIndex对应的数组中的值为tailIndex
    }

    headerIndex = globalMap[newHeaderX][newHeaderY].index;
    xiugaisnakeArrIndex(fen);
    if (zhuiIndex == (MAXWIDTH-2)*(MAXHEIGHT-2) ){
        zhuiIndex = 0;
    } 
    zhuizhongIndex[zhuiIndex] = headerIndex;
    zhuiIndex++;//zhuizhongIndex[zhuiIndex]记录蛇头的改变；每次移动完zhuiIndex对应的数组中的位置为存headerIndex的位置的后一个

    gotoxy(newHeaderY, newHeaderX);
    textcolor(0X04);
    cprintf("%c", fangXiangFu);
    globalMap[newHeaderX][newHeaderY].type = charSnake.type;
}







//初始化
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

    textcolor(0X04);//对障碍的必要处理部分
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

    textcolor(0X02);//对初始化蛇身的处理部分
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
    
    //对大地图外的信息处理
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