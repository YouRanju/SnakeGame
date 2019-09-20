// DirectXTest1.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "DirectXTest1.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <time.h>
#include <stdlib.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

LPDIRECT3D9 g_pD3D;
LPDIRECT3DDEVICE9 g_pD3DDevice;
D3DCOLOR g_ClearColor = D3DCOLOR_XRGB(0, 0, 255);

LPD3DXSPRITE g_Sprite;
LPDIRECT3DTEXTURE9 g_Texture[2];

D3DXVECTOR3 g_Snake[512];
D3DXVECTOR3 g_HeadPos = D3DXVECTOR3(100,100,0);
int g_length = 1;
D3DXVECTOR3 g_Dir = D3DXVECTOR3(1, 0, 0);

D3DXVECTOR3 g_BlockPos[10];
int g_BlockCnt = 0;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND&);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void CheckWorldBound();
void CheckBlocks();
void CheckSnakeBody();
bool InitDirect3D(HWND hwnd);
void ReleaseDirect3D();
void DrawSprite(D3DXVECTOR3& center, D3DXVECTOR3& position, int index);
void Render();

LPD3DXSPRITE CreateSprite(LPDIRECT3DDEVICE9 pD3DDevice);
void ReleaseSprite(LPD3DXSPRITE sprite);

bool isGameOver = false;
int whatDir = 1;

LPDIRECT3DTEXTURE9 CreateTexture(LPDIRECT3DDEVICE9 pD3DDevice, int width, int height, D3DCOLOR color);
void ReleaseTexture(LPDIRECT3DTEXTURE9 texture);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DIRECTXTEST1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

	HWND hwnd = NULL;

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow, hwnd))
    {
        return FALSE;
    }

	InitDirect3D(hwnd);
	g_Sprite = CreateSprite(g_pD3DDevice);
	g_Texture[0] = CreateTexture(g_pD3DDevice, 20, 20, D3DCOLOR_XRGB(30, 255, 0));
	g_Texture[1] = CreateTexture(g_pD3DDevice, 20, 20, D3DCOLOR_XRGB(200, 200, 0));

	srand((unsigned int)time(NULL));
	for (int i = 0; i < sizeof(g_BlockPos)/sizeof(D3DXVECTOR3); i++) {
		g_BlockPos[i] = D3DXVECTOR3(rand() % 32 * 20, rand() % 24 * 20, 0);
	}
	g_HeadPos = D3DXVECTOR3(100, 100, 0);

    MSG msg;
	ZeroMemory(&msg, sizeof(msg));

    // 기본 메시지 루프입니다:
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		else
		{
			if (!isGameOver) {
				static int count = 0;
				count++;

				if (count >= 4) {
					g_HeadPos = g_HeadPos + g_Dir * 20.f;

					CheckWorldBound();
					CheckBlocks();
					CheckSnakeBody();

					for (int i = g_length - 1; i > 0; i--) {
						g_Snake[i] = g_Snake[i - 1];
					}

					g_Snake[0] = g_HeadPos;

					count = 0;
				}

				Render();
			}
		}
    }

	ReleaseTexture(g_Texture[1]);
	ReleaseTexture(g_Texture[0]);
	ReleaseSprite(g_Sprite);
	ReleaseDirect3D();

    return (int) msg.wParam;
}

void CheckWorldBound() {
	if (g_HeadPos.x > 640 - 10.f)
		g_HeadPos.x = 640 - 10.f;
	else if (g_HeadPos.x < 10.f)
		g_HeadPos.x = 10.f;

	if (g_HeadPos.y > 480 - 10.f)
		g_HeadPos.y = 480 - 10.f;
	else if (g_HeadPos.y < 0 + 10.f)
		g_HeadPos.y = 10.f;
}

void CheckBlocks()
{
	if (g_HeadPos.x >= g_BlockPos[g_BlockCnt].x - 20.f && g_HeadPos.x <= g_BlockPos[g_BlockCnt].x + 20.f
		&& g_HeadPos.y >= g_BlockPos[g_BlockCnt].y - 20.f && g_HeadPos.y <= g_BlockPos[g_BlockCnt].y + 20.f) {

		g_length++; 

		g_BlockPos[g_BlockCnt] = D3DXVECTOR3(rand() % 32 * 20, rand() % 24 * 20, 0);
		g_BlockCnt++;

		if (g_BlockCnt >= 10) {
			g_BlockCnt = 0;
		}
	}
}

void CheckSnakeBody()
{
	for (int i = 1; i < g_length; i++) {
		if (g_HeadPos.x > g_Snake[i].x - 20.f && g_HeadPos.x < g_Snake[i].x + 20.f
			&& g_HeadPos.y > g_Snake[i].y - 20.f && g_HeadPos.y < g_Snake[i].y + 20.f) {
			g_length = i;
			//g_ClearColor = D3DCOLOR_XRGB(255, 0, 0);
			g_Dir = D3DXVECTOR3(0, 0, 0);
			//isGameOver = true;
		}
	}
}

//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIRECTXTEST1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND& rhWnd)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      0, 0, 640, 480, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   rhWnd = hWnd;

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_KEYDOWN:
	{
		if (wParam == VK_SPACE && isGameOver) {
			isGameOver = false;
			g_HeadPos = D3DXVECTOR3(100, 100, 0);
			g_Snake[0] = g_HeadPos;
			g_length = 1;
		}
		D3DXVECTOR3 curDir;

		if (wParam == VK_LEFT && g_Snake[0].x > 0 + 10.f) {
			curDir = D3DXVECTOR3(-1, 0, 0);
		}
		else if (wParam == VK_RIGHT && g_Snake[0].x < 640 - 10.f) {
			curDir = D3DXVECTOR3(1, 0, 0);
		}
		else if (wParam == VK_UP && g_Snake[0].y > 0 + 10.f) {
			curDir = D3DXVECTOR3(0, -1, 0);
		}
		else if (wParam == VK_DOWN && g_Snake[0].y < 480 - 10.f) {
			curDir = D3DXVECTOR3(0, 1, 0);
		}

		D3DXVECTOR3 diff = g_Dir + curDir;
		if (fabs(diff.x) > 0.f || fabs(diff.y) > 0.f) {
			g_Dir = curDir;
		}
	}
		break;
	case WM_KEYUP:
	{
		if (wParam == VK_SPACE) { //같은 키로 업!!
			g_ClearColor = D3DCOLOR_XRGB(0, 0, 255);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
		TCHAR text[30];
		wsprintf(text, _T("점수: %d"), g_length);
		TextOut(hdc, 20, 20, text, lstrlen(text));

		EndPaint(hWnd, &ps);
	}
	break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool InitDirect3D(HWND hwnd)
{
	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (g_pD3D == NULL) {
		return false;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.Windowed = TRUE;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = 640;
	d3dpp.BackBufferHeight = 480;
	
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
		hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, 
		&d3dpp, &g_pD3DDevice) == E_FAIL) 
	{
		return false;
	}

	return true;
}

void ReleaseDirect3D()
{
	if (g_pD3DDevice != NULL) {
		g_pD3DDevice->Release();
	}

	if (g_pD3D != NULL) {
		g_pD3D->Release();
	}

	g_pD3DDevice = NULL;
	g_pD3D = NULL;
}

void DrawSprite(D3DXVECTOR3& center, D3DXVECTOR3& position, int index)
{
	
	/*D3DXMATRIX mat;
	D3DXVECTOR2 pos(position.x, position.y);
	D3DXVECTOR2 scale(1.f, 1.f);
	D3DXMatrixIdentity(&mat);
	D3DXMatrixTransformation2D(&mat, NULL, 0.f, &scale, NULL, 0.7f, &pos);*/
	

	g_Sprite->Begin(D3DXSPRITE_ALPHABLEND);
	//g_Sprite->SetTransform(&mat);
	g_Sprite->Draw(g_Texture[index], NULL, &center, &position, D3DCOLOR_XRGB(255, 255, 255));
	g_Sprite->End();
}

void Render()
{
	if (g_pD3DDevice == NULL) {
		return;
	}

	g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, g_ClearColor, 1.0f, 0);

	if (SUCCEEDED(g_pD3DDevice->BeginScene())) {
		D3DXVECTOR3 center(10, 10, 0);

		DrawSprite(center, g_BlockPos[g_BlockCnt], 1);

		for (int i = g_length-1; i >= 0 ; i--) {
			DrawSprite(center, g_Snake[i], 0);
		}

		g_pD3DDevice->EndScene();
	}

	g_pD3DDevice->Present(NULL, NULL, NULL, NULL);
}

LPD3DXSPRITE CreateSprite(LPDIRECT3DDEVICE9 pD3DDevice)
{
	LPD3DXSPRITE pSpr;
	auto hr = D3DXCreateSprite(pD3DDevice, &pSpr);

	if (FAILED(hr)) {
		return NULL;
	}

	return pSpr;
}

void ReleaseSprite(LPD3DXSPRITE sprite)
{
	if (sprite != NULL) {
		sprite->Release();
	}

	sprite = NULL;
}

LPDIRECT3DTEXTURE9 CreateTexture(LPDIRECT3DDEVICE9 pD3DDevice, int width, int height, D3DCOLOR color)
{
	LPDIRECT3DTEXTURE9 tex;
	pD3DDevice->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex, 0);
	D3DLOCKED_RECT rect;
	tex->LockRect(0, &rect, 0, D3DLOCK_DISCARD);
	DWORD* pColor = (DWORD*)rect.pBits;
	
	D3DCOLOR lineColor = D3DCOLOR_ARGB(255, 255, 255, 0);

	for (int i = 0; i < width * height; ++i) {
		pColor[i] = color; // D3DCOLOR_ARGB(128, 255, 0, 0);
	}
	
	for (int x = 0; x < width; x++) {
		pColor[x] = lineColor;
		pColor[x + 1 * width] = lineColor;
		pColor[x + 2 * width] = lineColor;
		pColor[x + (height - 3) * width] = lineColor;
		pColor[x + (height - 2) * width] = lineColor;
		pColor[x + (height - 1) * width] = lineColor;
	}

	for (int y = 0; y < height; y++) {
		pColor[y * width] = lineColor;
		pColor[1 + y * width] = lineColor;
		pColor[2 + y * width] = lineColor;
		
		pColor[width - 1 + y * width] = lineColor;
		pColor[width - 2 + y * width] = lineColor;
		pColor[width - 3 + y * width] = lineColor;
	}
	tex->UnlockRect(0);

	return tex;
}

void ReleaseTexture(LPDIRECT3DTEXTURE9 texture)
{
	if (texture != NULL) {
		texture->Release();
	}

	texture = NULL;
}
