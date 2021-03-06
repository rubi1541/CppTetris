/******************************************************************************
* Project Name      : CppTetris
* File Name         : tetris.c
* Version           : 1.0
* Compiler          : Visual Studio 2015
*
********************************************************************************
* Copyright (2016), tkihira.
********************************************************************************
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR
* A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
* IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/

#include "GameMaster.h"
#include "Basic.h"
#include "Tetris_AI.h"

HINSTANCE hInstance;
#pragma warning(disable: 4996)

extern INFO info;
extern STATUS current;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void printInfo(HDC* hdc, RECT r, char* string, int val, POINT p1, POINT p2);


/**
*  print player info at UI
*
* @param hdc
* @param Rectangle SIze
* @param Ui show name string
* @param Ui show number val
* @param name pos
* @param number pos
*
* @return DropResult block
*/
void printInfo(HDC* hdc, RECT r, char* string, int val, POINT p1,POINT p2){
	Rectangle(*hdc, r.left, r.top, r.right, r.bottom);
	
	char imsistr[128];
	wsprintf(imsistr, string);
	TextOut(*hdc, p1.x, p1.y, imsistr, strlen(imsistr));
	wsprintf(imsistr, "%d", val);
	TextOut(*hdc, p2.x, p2.y, imsistr, strlen(imsistr));
}


/* *
 * callback
 * */
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static int FrameCount = 0;
	static int keyboardUpFalg = TRUE;
	static int isAImode = FALSE;
	static int AIflag = FALSE;

	/*Map*/
	static int board[MAP_WIDTH][MAP_HEIGHT];

	switch (msg) {
	case WM_CREATE: {
		HDC hdc = GetDC(hWnd);
		initialize(board);
		MakeDCformBitmaps(hdc, hInstance);
		ReleaseDC(hWnd, hdc);

		if (DEBUG_MODE == TRUE){
			AllocConsole();
			freopen("CONOUT$", "wt", stdout);
		}
		break;
	}
	case WM_KEYUP:{
		switch (wParam){
		case VK_UP:
		case VK_SPACE:
			//continues input prevent
			keyboardUpFalg = TRUE;
		}
		break;
	}

	case WM_KEYDOWN:{
		switch (wParam) {
		case VK_UP:
		case VK_SPACE:
			if (keyboardUpFalg == FALSE)
				break;
			keyboardUpFalg = FALSE;
		case VK_LEFT:
		case VK_RIGHT:
		case VK_DOWN:{
			int keyDownResult;
			keyDownResult = processInput(wParam,board);
			if (keyDownResult) {
				Update(board);
				FrameCount = 0;
			}
			break;
		}
		}
		break;
	}

	case WM_TIMER:{
		if (isAImode && !AIflag && !info.bUpdateStop) { 
			deleteBlock(current, board);
			current = getBlock(current, board);
			AIflag = TRUE;
		}

		FrameCount++;
		//game difficulty
		if (info.Framecounter < 5000)
			info.Framecounter++;
		//0 ~ 7 
		int level = 0;
		level = level > 7 ? 7 : info.Framecounter / 150;
		//10 ~ 3
		level = 10 - level;


		if (FrameCount % level == level-1){
			Update(board);
			FrameCount = 0;
			AIflag = FALSE;
		}

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	case WM_PAINT:{
		HDC MemDC;
		MemDC = showBoard(board);
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		//print player info
		printInfo(&hdc, MakeRECT(251, 200, 335, 260), "SCORE", info.score, MakePOINT(270, 210), MakePOINT(275, 235));
		printInfo(&hdc, MakeRECT(251, 270, 335, 330), "COMBO", info.combo, MakePOINT(270, 280), MakePOINT(275, 305));
		printInfo(&hdc, MakeRECT(251, 340, 335, 400), "LINE", info.line, MakePOINT(270, 350), MakePOINT(275, 375));

	
		BitBlt(hdc, 0, 0, 24 * 10, 24 * 20, MemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_COMMAND:
		if (((HWND)lParam) && (HIWORD(wParam) == BN_CLICKED))
		{
			switch (LOWORD(wParam))
			{
				//pause button
			case 10000:
				KillTimer(hWnd, 100);
				if (MessageBox(hWnd, "게임을 계속 하시겠습니까?", "일시 정지", MB_YESNO) == IDYES)
					SetTimer(hWnd, 100, 1000 / 30, NULL);
				else{
					gameOver(board);
					InvalidateRect(hWnd, NULL, TRUE);
				}
				break;

				//restart button
			case 10001:
				KillTimer(hWnd, 100);
				if (MessageBox(hWnd, "게임을 재시작 하시겠습니까?", "재시작", MB_YESNO) == IDYES){
					initialize(board);
				}
				SetTimer(hWnd, 100, 1000 / 30, NULL);
				break;
			case 1:
				SetFocus(hWnd);
				isAImode = TRUE; //AI mode on
				break;
			case 0:
				SetFocus(hWnd);
				isAImode = FALSE; //AI mode off
				break;
			}
		}
		break;

	case WM_DESTROY:
		Destory();

		if (DEBUG_MODE == TRUE)
			FreeConsole();

		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow) {
	srand((unsigned int)time(NULL));
	hInstance = hInst;
	WNDCLASSEX wc;

	static LPCTSTR pClassName = "NicoNicoProgramming2";

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = pClassName;
	wc.hIconSm = NULL;

	if (!RegisterClassEx(&wc))
		return FALSE;

	RECT r;
	r.left = r.top = 0;
	r.right = 24 * 10;
	r.bottom = 24 * 20;
	AdjustWindowRectEx(&r, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION, FALSE, 0);

	//Create Windows 
	HWND hMainWindow = CreateWindow(pClassName, "Nico Nico Programming2", WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT, 360, r.bottom - r.top, NULL, NULL, hInst, NULL);

	
	CreateWindow(TEXT("button"), TEXT("▶|"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		24 * 10 + 30, 10, 50, 50, hMainWindow, (HMENU)10000, hInstance, NULL);
	CreateWindow(TEXT("button"), TEXT("Restart"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		24 * 10 + 20, 70, 70, 50, hMainWindow, (HMENU)10001, hInstance, NULL);

	//AI check_box
	CreateWindow(TEXT("button"), TEXT("AI"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		24 * 10 + 15, 130, 80, 60, hMainWindow, (HMENU)0, hInstance, NULL);
	CreateWindow(TEXT("button"), TEXT("ON"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
		24 * 10 + 25, 145, 50, 20, hMainWindow, (HMENU)1, hInstance, NULL);
	CreateWindow(TEXT("button"), TEXT("OFF"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		24 * 10 + 25, 165, 50, 20, hMainWindow, (HMENU)0, hInstance, NULL);

	ShowWindow(hMainWindow, SW_SHOW);
	SetTimer(hMainWindow, 100, 1000 / 30, NULL);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	KillTimer(hMainWindow, 100);

	return 0;
}
