//#include<pthread.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define LINUX
//#define WINDOWS

#ifdef LINUX
	#undef WINDOWS
	#include<ncurses.h>
	#include<termios.h>  
	#include<unistd.h>  
	#include<fcntl.h>
	int kbhit(void)  
	{  
		struct termios oldt, newt;  
    	int ch;  
		int oldf;  
    	tcgetattr(STDIN_FILENO, &oldt);  
		newt = oldt;  
    	newt.c_lflag &= ~(ICANON | ECHO);  
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);  
    	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);  
 		fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);  
 		ch = getchar();  
 		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  
  		fcntl(STDIN_FILENO, F_SETFL, oldf);  
  		if(ch != EOF)  
    	{  
	    	ungetc(ch, stdin);  
	    	return 1;  
  		}  
	    return 0;  
	}  
#endif

#ifdef WINDOWS
	#include<windows.h>
	#include<conio.h>
#endif


typedef struct xy
{
	int x;
	int y;
}TEA;//可以用来存储坐标大小一类数据的数对 TEA=Two-Element Array

typedef struct map
{
	int layerA,layerB;//层数
	TEA size;//大小
	char a[4][100][100];//显示信息
	char b[4][100][100];//特殊信息(例如碰撞)
	int roleLayerNum;//角色所处层编号
	TEA roleBirthPlace;//角色出生点
}MAP;//存储地图的数据类型

typedef struct camera
{
	int movable;
	TEA size;
	TEA location;
	int moveRule;//0不动 1自动 3随人物动
	TEA roleXRange;//人物所处X轴范围
	TEA roleYRange;//人物所处Y轴范围
}CAM;

typedef struct screen
{
	TEA size;
	int left;
	int top;
	//游戏输出屏幕距实际输出屏幕左上角距离
	char a[40][100];//屏幕实时数据
}SCR;

typedef struct character
{
	TEA size;
	char a[20][20][20];//最大20×20字符 最多存储20帧
	int fpsN;//常态人物动画帧数
	int sSN;//殊态总数
	int fpsS[5];//殊态人物动画帧数 最多5种殊态
	TEA location;
}CHA;

void render(MAP ma,CAM ca,CHA ch,SCR *sc);
void clrs(int a)
{
#ifdef LINUX
	if(a==0)system("clear");
	else printf("\033[0;0H\033[?25l");
#endif
#ifdef WINDOWS
	system("cls");
#endif
}

int main()
{
	MAP Map[2];
	CAM Cam[2];
	SCR Scr[2];
	CHA Cha[2];
	int k;//游戏数据组号
	int i,j,l,camI,camJ,scrI,scrJ,mapI;
	int indx;//地图层编号
	char key;//输入按键
	int keyLock;
	FILE *map,*cam,*cha;
	map=fopen("map.in","r");
	cam=fopen("cam.in","r");
	cha=fopen("cha.in","r");
	if(map==NULL||cam==NULL||cha==NULL)
	{
		printf("Failed openning .in file!\n");
		exit(0);
	}
	
	//文件读取开始
	//map.in读取开始
	k=0;
	fscanf(map,"%d%d%d%d",&Map[k].layerA,&Map[k].layerB,&Map[k].size.x,&Map[k].size.y);
	for(i=0;i<Map[k].layerA;i++)
	{
		fscanf(map,"%d",&indx);
		for(j=0;j<Map[k].size.y;j++)
			for(l=0;l<Map[k].size.x;l++)
				do
				{
					fscanf(map,"%c",&Map[k].a[indx][j][l]);
				}while(Map[k].a[indx][j][l]=='\n');
	}
	for(i=0;i<Map[k].layerB;i++)
	{
		fscanf(map,"%d",&indx);
		for(j=0;j<Map[k].size.y;j++)
			for(l=0;l<Map[k].size.x;l++)
				do
				{
					fscanf(map,"%c",&Map[k].b[indx][j][l]);
				}while(Map[k].b[indx][j][l]=='\n');
	}
	fscanf(map,"%d",&Map[k].roleLayerNum);
	fscanf(map,"%d%d",&Map[k].roleBirthPlace.x,&Map[k].roleBirthPlace.y);
	fclose(map);
	//map.in读取结束

	//cam.in读取开始
	fscanf(cam,"%d",&Cam[k].movable);
	fscanf(cam,"%d%d",&Cam[k].size.x,&Cam[k].size.y);
	fscanf(cam,"%d%d",&Cam[k].location.x,&Cam[k].location.y);
	fscanf(cam,"%d",&Cam[k].moveRule);
	fscanf(cam,"%d%d",&Cam[k].roleXRange.x,&Cam[k].roleXRange.y);
	fscanf(cam,"%d%d",&Cam[k].roleYRange.x,&Cam[k].roleYRange.y);
	fclose(cam);
	//cam.in读取结束
	
	//cha.in读取开始
	fscanf(cha,"%d%d",&Cha[k].size.x,&Cha[k].size.y);
	fscanf(cha,"%d%d",&Cha[k].fpsN,&Cha[k].sSN);
	for(i=0;i<Cha[k].fpsN;i++)//不完整：未输入殊态帧
		for(j=0;j<Cha[k].size.y;j++)
			for(l=0;l<Cha[k].size.x;l++)
				do
				{
					fscanf(cha,"%c",&Cha[k].a[i][j][l]);
				}while(Cha[k].a[i][j][l]=='\n');
	fclose(cha);
	//cha.in读取结束
	//文件读取结束
	
	//数据初始化开始
	for(i=0;i<Cam[k].size.y;i++)
		for(j=0;j<Cam[k].size.x;j++)
			Scr[k].a[i][j]=' ';
	Cha[k].location=Map[k].roleBirthPlace;
	Scr[k].size=Cam[k].size;
	Scr[k].left=0;
	Scr[k].top=0;
	//数据初始化结束

	clrs(0);
	//调试代码开始
	while(1)
	{//while开始
		clrs(1);
	//调试代码结束
	
	//数据处理开始
	for(i=0;i<Cam[k].size.y;i++)
		for(j=0;j<Cam[k].size.x;j++)
			Scr[k].a[i][j]=' ';
	for(camI=Cam[k].location.y;camI<Cam[k].location.y+Cam[k].size.y;camI++)
	{
		for(camJ=Cam[k].location.x;camJ<Cam[k].location.x+Cam[k].size.x;camJ++)
		{
			scrI=camI-Cam[k].location.y;
			scrJ=camJ-Cam[k].location.x;
			for(mapI=0;mapI<Map[k].layerA;mapI++)	
			{
				if(mapI+1==Map[k].roleLayerNum)
				{
					//TODO:更新人物状态将人物写入屏幕
				}
				if(Map[k].a[mapI][camI][camJ]!=' ')Scr[k].a[scrI][scrJ]=Map[k].a[mapI][camI][camJ];
			}
		}
	}
	//数据处理结束

	//数据呈现开始
	for(i=0;i<Scr[k].top;i++)printf("\n");
	for(scrI=0;scrI<Scr[k].size.y;scrI++)
	{
		for(i=0;i<Scr[k].left;i++)printf(" ");
		for(scrJ=0;scrJ<Scr[k].size.x;scrJ++)
		{
			printf("%c",Scr[k].a[scrI][scrJ]);
		}
		printf("\n");
	}
	//数据呈现结束
	keyLock=-1;

	//调试代码开始
	//nodelay();
		if(kbhit())
		{
			key=getchar();	
			if(keyLock==-1)
				switch(key)
				{
					case 'd':
						if(Cam[k].location.x<30)Cam[k].location.x++;
						keyLock=1;
						break;
					case 'a':
						if(Cam[k].location.x>0)Cam[k].location.x--;
						keyLock=1;
						break;
				}
			key='\0';
		}
		while(kbhit())
		key=getchar();
		key='\0';
	}//while结束
	//调试代码结束

	return 0;
}
