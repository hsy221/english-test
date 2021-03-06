#include "stdafx.h"
#include "english_test.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名
char str_en[150][15];
char str_jp[150][15];
int q[150];
int max;

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


//main関数
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	//変数宣言（警告表示用変数）
	static PAINTSTRUCT ps;
	static HDC hdc;

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ENGLISHTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

	hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

	//メインウィンドウの作成
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 340, 420, nullptr, nullptr, hInstance, nullptr);
	ShowWindow(hWnd, nCmdShow);

	//標準入出力をコマンドプロンプトに設定（デバッグ用）
	FILE *debug;
	AllocConsole();               // コマンドプロンプトが表示される
	freopen_s(&debug, "CONIN$", "r", stdin);     // 標準入力の割り当て
	freopen_s(&debug, "CONOUT$", "w", stdout);    // 標準出力の割り当て

	//winMain関数のローカル変数
    MSG msg;
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ENGLISHTEST));
	
	//databaseファイル読み込み
	FILE *fp;
	if (fopen_s(&fp, "database141_210.csv", "r") != 0) {
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 15, 100, "単語データベースファイルがありません", 36);
		EndPaint(hWnd, &ps);
	}
	int m = 0;
	while ((fscanf_s(fp, "%[^,], %[^\n]\n", str_en[m], 
			_countof(str_en[m]), str_jp[m], _countof(str_jp[m]))) != EOF) {
		printf("%s, %s\n", str_en[m], str_jp[m]);
		m++;
	}
	fclose(fp);
	max = m;
	if (max > 150) {
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 15, 100, "単語数は150語までにして下さい", 29);
		EndPaint(hWnd, &ps);
	}

	//問題順番設定(ランダム並べ替え)
	srand(time(NULL));	//乱数の初期化
	for (int i = 0; i < max; i++) {
		q[i] = i;
	}
	for (int i = 0; i < max; i++) {
		int j = rand() % max;
		int t = q[i];
		q[i] = q[j];
		q[j] = t;
	}

	UpdateWindow(hWnd);
    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	fclose(debug);
	fclose(fp);

    return (int) msg.wParam;
}


//ウィンドウクラス
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ENGLISHTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ENGLISHTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


//ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//変数宣言
	static HBRUSH hBrushW	= CreateSolidBrush(RGB(255, 255, 255));
	static HBRUSH hBrushM	= CreateSolidBrush(RGB(150, 200, 255));
	static HPEN hPenR = CreatePen(PS_SOLID, 10, RGB(255, 0, 0));
	static HPEN hPenB = CreatePen(PS_SOLID, 10, RGB(0, 0, 255));
	static HPEN hPenBK = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	static PAINTSTRUCT ps;
	static HDC hdc;

	static POINT point;
	static int countQ = 0;		//経過した問題数
	static int select_num[5];	//選択肢の単語番号を格納
	static int choiceNo = 0;	//クリックされた選択肢
	static int ansNo = 0;		//答えの選択肢
	static int correct = 0;	//正当数
	static int state = 3;
	static int dummyq[149];
	static char countQ_str[4];
	static char max_str[4];

	//問題作成
	srand(time(NULL));
	if (state == 0) {
		//正答選択肢を作成
		select_num[0] = q[countQ];
		//ダミー選択肢を作成
		////正解単語番号を除いた数字列の作成
		for (int i = 0; i < q[countQ]; i++) {
			dummyq[i] = i;
		}
		for (int i = q[countQ]; i < max - 1; i++) {
			dummyq[i] = i + 1;
		}
		////数字列の並び替え
		for (int i = 0; i < max-1; i++) {
			int j = rand() % (max-1);
			int t = dummyq[i];
			dummyq[i] = dummyq[j];
			dummyq[j] = t;
		}
		////先頭1-4を抜き出し
		for (int i = 1; i < 5; i++) {
			select_num[i] = dummyq[i-1];
		}
		//順番をミックス
		for (int i = 0; i < 5; i++) {
			int jj = rand() % 5;
			int tt = select_num[i];
			select_num[i] = select_num[jj];
			select_num[jj] = tt;
		}
		//正解の番号を取り出し
		for (int i = 0; i < 5; i++) {
			if (select_num[i] == q[countQ]) {
				ansNo = i;
			}
		}
		InvalidateRect(hWnd, NULL, TRUE);
		state = 1;
	}

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
			//バージョン情報
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
			//閉じる
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_LBUTTONDOWN:
		point.x = MAKEPOINTS(lParam).x;
		point.y = MAKEPOINTS(lParam).y;
		if (point.x > 50 && point.x < 270 && state == 1) {
			for (int i = 0; i < 5; i++) {
				if (point.y > 100 + 50 * i && point.y < 130 + 50 * i) {
					choiceNo = i;
					state = 2;
					InvalidateRect(hWnd, NULL, TRUE);
				}
			}
		}
		break;
    case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
			//四角描画
			SelectObject(hdc, hBrushW);
			Rectangle(hdc, 20, 20, 300, 60);
			Rectangle(hdc, 50, 100, 270, 130);
			Rectangle(hdc, 50, 150, 270, 180);
			Rectangle(hdc, 50, 200, 270, 230);
			Rectangle(hdc, 50, 250, 270, 280);
			Rectangle(hdc, 50, 300, 270, 330);
			//文字描画
			TextOut(hdc, 40, 30, _TEXT(str_en[select_num[ansNo]]), strlen(str_en[select_num[ansNo]]));
			_itoa_s(countQ, countQ_str, 5, 10);
			_itoa_s(max, max_str, 5, 10);
			TextOut(hdc, 200, 30, _TEXT(countQ_str), strlen(countQ_str));
			TextOut(hdc, 220, 30, _TEXT("/"), strlen("/"));
			TextOut(hdc, 240, 30, _TEXT(max_str), strlen(max_str));
			for (int i = 0; i < 5; i++) {
				TextOut(hdc, 70, 110 + i * 50, _TEXT(str_jp[select_num[i]]), strlen(str_jp[select_num[i]]));
			}
			//〇×表示と選択番号を再描画
			if(state == 2) {
				SelectObject(hdc, hBrushM);
				Rectangle(hdc, 50, 100+50*(choiceNo), 270, 130+50*(choiceNo));
				SetBkColor(hdc, RGB(150, 200, 255));
				TextOut(hdc, 70, 110 + (choiceNo) * 50, _TEXT(str_jp[select_num[choiceNo]]), strlen(str_jp[select_num[choiceNo]]));
				if (choiceNo == ansNo) {
					correct++;
					SelectObject(hdc, hPenR);
					MoveToEx(hdc, 220, 210, NULL);
					AngleArc(hdc, 170, 210, 50, 0, 359);
				}
				else {
					SelectObject(hdc, hPenB);
					MoveToEx(hdc, 130, 170, NULL);
					LineTo(hdc, 210, 250);
					MoveToEx(hdc, 130, 250, NULL);
					LineTo(hdc, 210, 170);
				}
				Sleep(500);
				countQ++;
				if (countQ == max) {
					InvalidateRect(hWnd, NULL, TRUE);
					state = 4;
				}
				else {
					state = 0;
				}
			}
			else if (state == 3) {
				state = 0;
			}
			else if (state == 4) {
				SelectObject(hdc, hBrushW);
				SelectObject(hdc, hPenBK);
				Rectangle(hdc, 20, 20, 300, 60);
				Rectangle(hdc, 50, 100, 270, 130);
				Rectangle(hdc, 50, 150, 270, 180);
				Rectangle(hdc, 50, 200, 270, 230);
				Rectangle(hdc, 50, 250, 270, 280);
				Rectangle(hdc, 50, 300, 270, 330);
				SetBkColor(hdc, RGB(255, 255, 255));
				TextOut(hdc, 40, 30, _TEXT("正答率="), strlen("正答率="));
				char correct_s[5];
				_itoa_s(correct, correct_s, 5, 10);
				TextOut(hdc, 120, 30, _TEXT(correct_s), strlen(correct_s));
				TextOut(hdc, 140, 30, _TEXT("/"), 1);
				char max_s[5];
				_itoa_s(max, max_s, 5, 10);
				TextOut(hdc, 160, 30, _TEXT(max_s), strlen(max_s));
			}
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

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
