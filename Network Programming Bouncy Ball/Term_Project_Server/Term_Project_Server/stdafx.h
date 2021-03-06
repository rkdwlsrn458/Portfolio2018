// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#pragma comment(lib, "ws2_32")

#include "targetver.h"
#include <winsock2.h>
#include <stdlib.h>
#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <vector>
#include <memory>
#include <chrono>

#define nPlayer 3
#define BoardMax 20

typedef struct BOARD {
	double x[2], y, z[2];
	double range;
	double m_range;
	bool mark;
	bool move;
	bool clicked;
	int item;
	int texture;
};

typedef struct OBJECT {
	double x, y, z;
	bool state;
};

typedef struct CLIENT_STRUCT
{
	OBJECT position; // 공의 위치 값
	bool change; // 공이 죽었을 시 화면전환
};

typedef struct SERVER_STRUCT
{
	OBJECT A[3]; // 새로운 위치 값
	bool effect[11]; // 5가지 효과에 대한 상태
	bool sound[12]; // 4가지 사운드의 On/ Off
	BOARD B[20]; // 현재 진행되고 있는 보드들 
	int pIndex;
	int pcount;
};

enum SOUND
{
	SOUND_FIRST = 0,
	FIRST_PONG = 0,
	FIRST_GET_ITEM = 1,
	FIRST_EXPLOSION = 2,
	FIRST_BLANK = 3,
	SECOND_PONG = 4,
	SECOND_GET_ITEM = 5,
	SECOND_EXPLOSION = 6,
	SECOND_BLANK = 7,
	THIRD_PONG = 8,
	THIRD_GET_ITEM = 9,
	THIRD_EXPLOSION = 10,
	THIRD_BLANK = 11,
	SOUND_LAST = 11
};
enum EFFECT
{
	EFFECT_FIRST = 0,
	FOG = 0, 
	STOP = 1,
	FIRST_NO_ALPHA =2,
	FIRST_SLOW = 3, 
	FIRST_NO_DIE =4,
	SECOND_NO_ALPHA =5,
	SECOND_SLOW = 6, 
	SECOND_NO_DIE = 7,
	THIRD_NO_ALPHA = 8,
	THIRD_SLOW = 9, 
	THIRD_NO_DIE = 10,
	EFFECT_LAST = 10
};

#include "GameSystem.h"

#define SERVERPORT	9000
#define BUFSIZE		512
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
