#include "textic.h"
#include "kbhit.h"
#include "data.h"
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define _COLORPAIR(x) COLOR_PAIR((((x.frontColor)*8)+(x.backColor))+1)

//void render(MAP ma,CAM ca,CHA ch,SCR *sc);

void clrs(int a)
{
	if (a == 0) system("clear");
	else clear();//ncurses下的清屏
	//printw("\033[0;0H\033[?25l");
}

MAP Map[2];
CAM Cam[2];
SCR Scr[2];
CHA Cha[2];

//CHA testEnemy={5,1,2,0,{{" [#] "},{"[-#-]"}}};
//TODO:写一个数据初始函数

int main()
{
	int k;//游戏数据组号
	int i, j, l, camI, camJ, scrI, scrJ, mapI, chaI, chaJ;
	int chaFPS = 0;//人物当前状态帧编号
	int indx;//地图层编号
	int inputCon = 0;//输入频率控制
	int inputLock = 0;//输入锁定
	int tmpC = 1;
	COLORP defaultColor = {2, 0};
	wchar_t key;//输入按键
	FILE *map, *cam, *cha;
	map = fopen("map.in", "r");
	cam = fopen("cam.in", "r");
	cha = fopen("cha.in", "r");
	if (map == NULL || cam == NULL || cha == NULL) {
		printf("Failed openning .in file!\n");
		exit(0);
	}
	//文件读取开始
	//map.in读取开始
	k = 0;
	fscanf(map, "%d%d%d%d", &Map[k].layerA, &Map[k].layerB, &Map[k].size.x, &Map[k].size.y);
	for(i = 0; i < Map[k].layerA; i++) {
		fscanf(map,"%d", &indx);
		for(j = 0; j < Map[k].size.y; j++)
			for(l = 0; l < Map[k].size.x; l++)
				do {
					fscanf(map,"%c", &Map[k].a[indx][j][l]);
				}while(Map[k].a[indx][j][l] == '\n');
	}
	for(i = 0; i < Map[k].layerB; i++) {
		fscanf(map, "%d", &indx);
		for(j = 0; j < Map[k].size.y; j++)
			for(l = 0; l < Map[k].size.x; l++)
				do {
					fscanf(map, "%c", &Map[k].b[indx][j][l]);
				}while(Map[k].b[indx][j][l] == '\n');
	}
	fscanf(map, "%d", &Map[k].roleLayerNum);
	fscanf(map, "%d%d", &Map[k].roleBirthPlace.x,&Map[k].roleBirthPlace.y);
	fclose(map);
	//map.in读取结束

	//cam.in读取开始
	fscanf(cam, "%d", &Cam[k].movable);
	fscanf(cam, "%d%d", &Cam[k].size.x,&Cam[k].size.y);
	fscanf(cam, "%d%d", &Cam[k].location.x,&Cam[k].location.y);
	fscanf(cam, "%d", &Cam[k].moveRule);
	fscanf(cam, "%d%d", &Cam[k].roleXRange.x,&Cam[k].roleXRange.y);
	fscanf(cam, "%d%d", &Cam[k].roleYRange.x,&Cam[k].roleYRange.y);
	fclose(cam);
	//cam.in读取结束
	
	//cha.in读取开始
	fscanf(cha, "%d%d", &Cha[k].size.x,&Cha[k].size.y);
	fscanf(cha, "%d%d", &Cha[k].fpsN,&Cha[k].sSN);
	for(i = 0; i < Cha[k].fpsN; i++)//不完整：未输入殊态帧
		for(j = 0;j < Cha[k].size.y; j++)
			for(l = 0;l < Cha[k].size.x; l++)
				do {
					fscanf(cha, "%c", &Cha[k].a[i][j][l]);
				}while(Cha[k].a[i][j][l] == '\n');
	fclose(cha);
	//cha.in读取结束
	//文件读取结束
	
	//数据初始化开始
	for(i = 0; i < Cam[k].size.y; i++)
		for(j = 0; j < Cam[k].size.x; j++) {
			Scr[k].c[i][j] = defaultColor;
			Scr[k].a[i][j] = ' ';
		}
	Cha[k].location = Map[k].roleBirthPlace;
	Scr[k].size = Cam[k].size;
	Scr[k].left = 1;
	Scr[k].top = 1;
	//数据初始化结束

	//窗口数据初始化开始
	clrs(0);
	initscr();//ncurses初始化
	start_color();//开启颜色模式

	for(i = 0; i < 8; i++)
		for(j = 0; j < 8; j++)
			init_pair((i * 8 + j + 1), i, j);

	noecho();//关闭输入回显
	cbreak();//关闭行缓冲
	curs_set(0);//隐藏光标
	nodelay(stdscr,TRUE);//将stdstr设置为无延迟模式
	keypad(stdscr,TRUE);
	//窗口数据初始化结束

	for(;;) {//总循环开始
		clrs(1);
	
	//数据处理开始
		for(i = 0; i < Cam[k].size.y; i++)
			for(j = 0;j < Cam[k].size.x; j++)
				Scr[k].a[i][j] = ' ';
		for(camI = Cam[k].location.y; camI < Cam[k].location.y + Cam[k].size.y; camI++) {
			for(camJ = Cam[k].location.x; camJ < Cam[k].location.x + Cam[k].size.x; camJ++) {
				scrI = camI-Cam[k].location.y;
				scrJ = camJ-Cam[k].location.x;
				for(mapI = 0; mapI < Map[k].layerA; mapI++) {
					if (mapI == Map[k].roleLayerNum) {
						//TODO:更新人物状态将人物写入屏幕
						chaI = camI-Cha[k].location.y;
						chaJ = camJ-Cha[k].location.x;
						if ((chaI >= 0) && (chaI < Cha[k].size.y) && (chaJ >= 0) && (chaJ < Cha[k].size.x)) {
							if (Cha[k].a[chaFPS][chaI][chaJ] != ' ') {
								Scr[k].a[scrI][scrJ] = Cha[k].a[chaFPS][chaI][chaJ];
								Scr[k].c[scrI][scrJ].frontColor = 7;
							}
						}
					}
					if (Map[k].a[mapI][camI][camJ] != ' ') {
						Scr[k].a[scrI][scrJ] = Map[k].a[mapI][camI][camJ];
						Scr[k].c[scrI][scrJ] = defaultColor;
					}
				}
			}
		}
		//数据处理结束

		//数据呈现开始
		for(i = 0; i < Scr[k].top; i++) printw("\n");
		for(scrI = 0; scrI < Scr[k].size.y; scrI++) {
			for(i = 0; i < Scr[k].left; i++) printw(" ");
			for(scrJ = 0;scrJ < Scr[k].size.x; scrJ++) {
				attron(_COLORPAIR(Scr[k].c[scrI][scrJ]));
				//attron(COLOR_PAIR(tmpC));
				printw("%c", Scr[k].a[scrI][scrJ]);
				//attroff(COLOR_PAIR(tmpC));
				tmpC++;
				if (tmpC == 65) tmpC = 1;
				attroff(_COLORPAIR(Scr[k].c[scrI][scrJ]));
			}
			printw("\n");
		}
		//数据呈现结束

		if(inputLock) {
			inputCon++;
			if (inputCon == 3) {
				inputCon = 0;
				inputLock = 0;
			}
		}
		if (kbhit()) {
			key = getch();	
			if (inputLock == 0) {
				switch(key)
				{
					case 'd':
						if ((Cha[k].location.x + Cha[k].size.x) < Map[k].size.x) {
							Cha[k].location.x++;
							chaFPS++;
							if (Cam[k].moveRule == 3)
								if(Cha[k].location.x + Cha[k].size.x - 1 - Cam[k].location.x > Cam[k].roleXRange.y) Cam[k].location.x++;
							inputLock = 1;
						}
						break;
					case 'a':
						if (Cha[k].location.x > 1) {
							Cha[k].location.x--;
							chaFPS++;
							if (Cam[k].moveRule == 3)
								if((Cha[k].location.x - Cam[k].location.x) < Cam[k].roleXRange.x) Cam[k].location.x--;
							inputLock = 1;
						}
						break;
					case 'z':
						if (Cam[k].location.x > 0) Cam[k].location.x--;
						break;
					case 'c':
						if (Cam[k].location.x < Map[k].size.x - Scr[k].size.x) Cam[k].location.x++;
						break;
					case 'j':
						if (Scr[k].left > 1) Scr[k].left--;
						break;
					case 'l':
						if ((Scr[k].left + Scr[k].size.x + 1) < COLS) Scr[k].left++;
						break;
					case 'i':
						if (Scr[k].top > 1) Scr[k].top--;
						break;
					case 'k':
						if ((Scr[k].top + Scr[k].size.y + 1) < LINES) Scr[k].top++;
						break;
				}
			}
			if (key == 'q') break;//按q键退出
			key = '\0';
		}
		refresh();//刷新显示屏幕
		if (chaFPS == Cha[k].fpsN) chaFPS %= Cha[k].fpsN;
		napms(30);//小睡30ms
	}//总循环结束
	endwin();//结束ncurses模式

	return 0;
}
