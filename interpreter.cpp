#include<windows.h>
#include<fstream>
#include<stack>
#include<vector>
#include<string>
#include"Bitmap.h"
using namespace std;

#define GRAY(color) RGB(color, color, color)

const LPSTR TITLE = TEXT("Brainblast");

const int DOT_SIZE = 2;
const int FIELD_SIZE = 256;

const int WND_WIDTH = DOT_SIZE * FIELD_SIZE;
const int WND_HEIGHT = DOT_SIZE * FIELD_SIZE;

Bitmap bmpData(FIELD_SIZE, FIELD_SIZE);
Bitmap image("image.bmp");

bool readData(const char* filename, vector<char> *program)
{
	ifstream ifs(filename);
	if(!ifs) return false;
	
	char buf;
	while(ifs.get(buf))
	{
		program->push_back(buf);
	}
	
	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd , UINT msg , WPARAM wp , LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd , &ps);
		
		bmpData.Draw(hdc, 0, 0, DOT_SIZE * FIELD_SIZE, DOT_SIZE * FIELD_SIZE);
		
		EndPaint(hwnd , &ps);

		return 0;
	}
	}
	return DefWindowProc(hwnd , msg , wp , lp);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;
	MSG Msg;
	WNDCLASS winc = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc, 0, 0, hInstance,
		(HICON)LoadImage(NULL, MAKEINTRESOURCE(IDI_APPLICATION), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED),
		(HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(IDC_ARROW), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED),
		(HBRUSH)GetStockObject(WHITE_BRUSH),
		NULL, TITLE
	};
	if(!RegisterClass(&winc)) return -1;
	
	RECT wndRect = {0, 0, WND_WIDTH, WND_HEIGHT};
	AdjustWindowRect(&wndRect, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
	hWnd = CreateWindow(
			TITLE , TITLE ,
			WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
			CW_USEDEFAULT , CW_USEDEFAULT ,
			wndRect.right - wndRect.left, wndRect.bottom - wndRect.top,
			NULL , NULL , hInstance , NULL
	);
	
	if(hWnd == NULL) return -1;
	
	ShowWindow(hWnd, nCmdShow);
	
	const int DATA_MAX = 30000;
	vector<char> program;
	unsigned char data[DATA_MAX] = {0};
	int pc = 0;
	int ptr = 0;
	stack<int> loopStack;
	
	auto acs = [=](int idx) -> int { return ((idx >= 0)? idx % DATA_MAX : DATA_MAX - (-idx % DATA_MAX)); };
	
	if(!readData(lpCmdLine, &program))
	{
		MessageBox(hWnd, "ファイルのオープンに失敗しました", "error", MB_OK | MB_ICONERROR);
		return -1;
	}
	
	while(1)
	{
		if(PeekMessage(&Msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if(GetMessage(&Msg, NULL, 0, 0) > 0)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
			else break;
		}
		else
		{
			for(int i = 0;i < program.size();i++)
			{
				if(pc < program.size())
				{
					switch(program[pc])
					{
					case '>':
						ptr = acs(ptr + 1);
						break;
					case '<':
						ptr = acs(ptr - 1);
						break;
					case '+':
						data[ptr]++;
						break;
					case '-':
						data[ptr]--;
						break;
					case '.':
					case ',':
						break;
					case '[':
						if(!data[ptr])
						{
							while(program[pc] != ']') pc++;
						}
						else
						{
							loopStack.push(pc);
						}
						break;
					case ']':
						if(data[ptr])
						{
							pc = loopStack.top() - 1;
						}
						loopStack.pop();
						break;
					
					case '#':
						/*
							(x, y)にcolorで点を描く
							x : ptr
							y : ptr >
							color : ptr >>
						*/
						bmpData(data[ptr], data[acs(ptr + 1)]) = GRAY(data[acs(ptr + 2)]);
						break;
					
					case '$':
						/*
							(x, y)にマウス座標を格納する.
							ウィンドウから出ている場合はx = 0, y = 0となる
							x : ptr
							y : ptr >
						*/
						POINT pos;
						GetCursorPos(&pos);
						ScreenToClient(hWnd, &pos);
						if(pos.x < 0 || WND_WIDTH <= pos.x || pos.y < 0 || WND_HEIGHT <= pos.y)
						{
							pos.x = 0;
							pos.y = 0;
						}
						
						data[ptr] = pos.x / DOT_SIZE;
						data[acs(ptr + 1)] = pos.y / DOT_SIZE;
						break;
					case '@':
						/*
							fl, frにマウスが押されているかを格納する
							(押されていたら1、そうでなければ0)
							fl : ptr
							fr : ptr >
						*/
						
						data[ptr] = (GetAsyncKeyState(VK_LBUTTON) < 0);
						data[acs(ptr + 1)] = (GetAsyncKeyState(VK_RBUTTON) < 0);
						break;
					case '!':
					{
						/*
							messageをメッセージボックスで出す
							message : ptr>(ptr == 0 or ptr >= ProgramSizeまで)
						*/
						string str("");
						int tempPtr = ptr;
						while(tempPtr < program.size() || data[tempPtr] != 0)
						{
							str += (char)data[tempPtr];
							tempPtr++;
						}
						MessageBox(hWnd, str.c_str(), TITLE, MB_OK);
						
						break;
					}
					case '&':
						/*
							画像(./image.bmp)を(x, y)に出す
							src(0, 0)が透過色として扱われる
							x : ptr
							y : ptr >
						*/
						bmpData.Composition(image, data[ptr], data[acs(ptr + 1)], image(0, 0));
						break;
					case '%':
						/*
							キャンバス(インタプリタのクライアント領域)を
							ビットマップ(./out.bmp)で保存する
						*/
						bmpData.WriteToBitmapFile("out.bmp");
						break;
					}
					
					pc++;
					
				}
			}
			Sleep(1);
			InvalidateRect(hWnd, NULL, TRUE);
		}
	}
	
	return Msg.wParam;
}
