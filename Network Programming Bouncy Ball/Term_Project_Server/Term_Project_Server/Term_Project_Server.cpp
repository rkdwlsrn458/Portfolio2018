// Term_Project_Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"


void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);
DWORD WINAPI ProcessClient(LPVOID arg);
DWORD UpdateThread(LPVOID arg);

SERVER_STRUCT sbuffer;
GameSystem gameSystem{ sbuffer };
SOCKADDR_IN blank_socket[3];
HANDLE    event[4];
bool change[3];
bool timebool;
int changeView[3];
int num = 0;

std::chrono::high_resolution_clock::time_point mill1;
std::chrono::high_resolution_clock::time_point mill2;

std::vector<SOCKET> PlayerSock;

int main()
{
	timebool = true;
	changeView[0] = changeView[1] = changeView[2] = -1;
	change[0] = change[1] = change[2] = false;
	event[0] = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (event[0] == NULL) return 1;
	event[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (event[1] == NULL) return 1;
	event[2] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (event[2] == NULL) return 1;
	event[3] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (event[3] == NULL) return 1;

	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	//socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	//bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		err_quit("listen()");

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread;

	CreateThread(NULL, 0, UpdateThread, NULL, 0, NULL);

	while (1)
	{
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		blank_socket[num] = clientaddr;
		num++;

		std::cout << std::endl;
		//std::cout << "[TCP 서버] 클라이언트 접속: IP주소 = " << (char*)inet_ntoa(clientaddr.sin_addr) << ", 포트번호 = " << ntohs(clientaddr.sin_port) <<std::endl;
		//스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);

		if (hThread == NULL)
		{
			closesocket(client_sock);
		}
		else
		{
			CloseHandle(hThread);
		}
	}

	closesocket(listen_sock);
	CloseHandle(event[0]);
	CloseHandle(event[1]);
	CloseHandle(event[2]);
	CloseHandle(event[3]);

	WSACleanup();
	return 0;
}

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << "[" << msg << "]" << " " << (char*)lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << "[" << msg << "]" << " " << (char*)lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
}


DWORD WINAPI ProcessClient(LPVOID arg) {

	SOCKET client_sock = (SOCKET)arg;
	int retval;
	SOCKADDR_IN clientaddr;
	int addrlen;
	CLIENT_STRUCT cbuffer;

	//클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);
	PlayerSock.push_back(client_sock);


	int n;

	for (int i = 0; i < num; i++)
	{
		if (blank_socket[i].sin_port == clientaddr.sin_port)
		{
			n = i;
		}
	}
	sbuffer.pIndex = n;
	retval = send(client_sock, (char*)&sbuffer, sizeof(SERVER_STRUCT), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("send()");
		return 0;
	}

	while (1)
	{
		WaitForSingleObject(event[n], INFINITE);
		sbuffer.pcount = num;
		sbuffer.pIndex = n;
		
		//데이터 받기, 고정길이
		retval = recvn(client_sock, (char*)&cbuffer, sizeof(CLIENT_STRUCT), 0);

		if (timebool)
		{
			mill1 = std::chrono::high_resolution_clock::now();
			timebool = false;
		}
		gameSystem.SetBalls(cbuffer, n);
		change[n] = cbuffer.change;
		if (retval == SOCKET_ERROR)
		{
			err_display("recvn()");
			break;
		}
		else if (retval == 0)
		{
			break;
		}
		int k = n + 1;
		if (k > nPlayer)
			k = 0;
		SetEvent(event[k]);
	}

	closesocket(client_sock);
	//std::cout << "[TCP서버] 클라이언트 종료 : IP주소=" << (char*)inet_ntoa(clientaddr.sin_addr) << "포트번호 = " << ntohs(clientaddr.sin_port) << std::endl;
	return 0;
}

int recvn(SOCKET s, char *buf, int len, int flags) {
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
		else if (received == 0) {
			break;
		}
		left -= received;
		ptr += received;
	}

	return (len - left);
}

DWORD UpdateThread(LPVOID arg)
{
	int retval;

	while (true)
	{
		WaitForSingleObject(event[3], INFINITE);
		gameSystem.MoveBoard();

		for (int i = 0; i < num; ++i)
		{
			if(sbuffer.A[i].state)
				gameSystem.BallUpdate(i);
		}

		while (1)
		{
			mill2 = std::chrono::high_resolution_clock::now();
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds> (mill2 - mill1);

			if (ms >(std::chrono::milliseconds)33)
			{
				timebool = true;
				break;
			}
		}

		for (int i = 0; i < num; i++)
		{
			sbuffer.pIndex = i;

			if (!sbuffer.A[i].state && change[i])
			{
				int k = -1;
				for (int q = 0; q < nPlayer; q++)
				{
					if (q != i && sbuffer.A[q].state && changeView[i] != q)
						k = q;
				}
				changeView[i] = k;
				sbuffer.A[i].x = sbuffer.A[k].x;
				sbuffer.A[i].y = sbuffer.A[k].y;
				sbuffer.A[i].z = sbuffer.A[k].z;
				change[i] = false;
			}
			else if (!sbuffer.A[i].state && !change[i] && changeView[i] != -1)
			{
				sbuffer.A[i].x = sbuffer.A[changeView[i]].x;
				sbuffer.A[i].y = sbuffer.A[changeView[i]].y;
				sbuffer.A[i].z = sbuffer.A[changeView[i]].z;
			}

			retval = send(PlayerSock[i], (char*)&sbuffer, sizeof(SERVER_STRUCT), 0);
			if (retval == SOCKET_ERROR) { err_display("send()"); return 0; }
		}
		SetEvent(event[0]);
	}
}