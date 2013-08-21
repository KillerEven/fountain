#include<stdlib.h>

typedef struct xy
{
	int x;
	int y;
}TEA;//可以用来存储坐标大小一类数据的数对 TEA=Two-Element Array

typedef struct colorpair
{
	int frontColor;
	int backColor;
}COLORP;

typedef struct map
{
	int layerA,layerB;//层数
	TEA size;//大小
	char a[4][500][500];//显示信息
	char b[4][500][500];//特殊信息(例如碰撞)
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
	char a[500][500];//屏幕实时数据
	COLORP c[500][500];
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
