// ramie_robota.cpp : Defines the entry point for the application.
//


#include "stdafx.h"
#include "ramie_robota.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <queue>
using namespace std;

#define MAX_LOADSTRING 100

#define IDM_BZASTOSUJ 1
#define IDM_BMANUALNY 2
#define IDM_BAUTOMATYCZNY 3
#define IDM_ESZYBKOSC 4

#define IDM_SNAGRYWANIE 5
#define IDM_BSTART 6
#define IDM_BPAUSE 7
#define IDM_BRESET 8
#define IDM_BODTWORZ 9

#define IDM_BODTWARZANIE 11

#define IDM_EUDZWIG 12
#define IDM_EBLOK1 13
#define IDM_EBLOK2 14
#define IDM_EBLOK3 15
#define IDM_BWAGA 16

//definicje kolorów
#define CZERWONY 255,0,0
#define ZIELONY 0,255,0
#define NIEBIESKI 0,0,255


// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING]; 					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
ULONG_PTR m_gdiplusToken;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

struct dane{
	int a;
};
dane WYSOKOSC_OKNA, SZEROKOSC_OKNA;

class rysuj_ramie{
public:

	LONG start_x = 0;
	LONG start_y = 0;
	LONG koniec_x = 0;
	LONG koniec_y = 0;
	LONG dlugosc = 250; //dlugosc ramienia
	LONG udzwig = 100;

	bool poczatkowe_ramie = FALSE;
	double nachylenie = 0;
	int predkosc_zmiany = 0;
	int domyslna_predkosc = 0;

	void start(HDC hdc){
		odswiez(poczatkowe_ramie, hdc);
	}

private:
	PAINTSTRUCT ps;
	RECT naroznik;
	LONG polozenie_x;
	LONG polozenie_y;
	void odswiez(bool poczatkowe_ramie, HDC hdc)
	{
		Graphics graphics(hdc);
		Pen obrys(Color(255, NIEBIESKI), 20);


		do{
			//obliczanie dlugosci bokow z wlasnosci trygonometrycznych
			koniec_x = LONG(dlugosc* cos(nachylenie));
			koniec_y = LONG(dlugosc* sin(nachylenie));

			if (poczatkowe_ramie)
			{
				start_x = LONG(SZEROKOSC_OKNA.a / 2);
				start_y = WYSOKOSC_OKNA.a;
			}

			koniec_x += start_x;
			koniec_y = start_y - koniec_y;

			if (nachylenie < 0) nachylenie += 0.01;
			else if (nachylenie > M_PI) nachylenie -= 0.01;

		} while (koniec_y > WYSOKOSC_OKNA.a);//petla odpowiada za korygowanie nachylenia w przypadku wyjscia poza zakres okna

		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		graphics.DrawLine(&obrys, start_x, start_y, koniec_x, koniec_y);
	}


};
class rysuj_kolo{
public:
	void start(HDC hdc, int polozenie_x, int polozenie_y, int srednica = 30){
		rysuj(hdc, polozenie_x, polozenie_y, srednica);
	}
private:
	PAINTSTRUCT ps;
	Rect rect;
	void rysuj(HDC hdc, int polozenie_x, int polozenie_y, int srednica)
	{
		rect.X = polozenie_x - srednica / 2;
		rect.Y = polozenie_y - srednica / 2;
		rect.Height = srednica;
		rect.Width = srednica;

		Graphics graphics(hdc);
		Pen obrys(Color(255, CZERWONY), 1);
		SolidBrush wypelnienie(Color(CZERWONY));
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		graphics.DrawEllipse(&obrys, rect);
		graphics.FillEllipse(&wypelnienie, rect);
	}


};
class rysuj_blok{
public:
	int polozenie_x = 0, polozenie_y = 0;
	int wielkosc = 100;
	bool poczatek = TRUE, poruszaj = FALSE;
	int temp_x = 0, temp_y = 0;
	int nr = 0;//numer bloku konieczny do poczatkowego pozycjonowania
	int waga = 100;

	void start(HDC hdc, int r, int g, int b){
		rysuj(hdc, r, g, b);
	}
private:
	PAINTSTRUCT ps;
	Rect rect;
	void rysuj(HDC hdc, int r, int g, int b)
	{
		if (poczatek)
		{
			polozenie_x = SZEROKOSC_OKNA.a / 2 - wielkosc - 100 - nr*(wielkosc + 10);//nie zachodzenie bloków na siebie przy pierwszym uruchomieniu
			polozenie_y = WYSOKOSC_OKNA.a - wielkosc;
			poczatek = FALSE;
		}
		rect.X = polozenie_x;
		rect.Y = polozenie_y;
		rect.Height = wielkosc;
		rect.Width = wielkosc;

		if (rect.Y + rect.Height > WYSOKOSC_OKNA.a)//jesli blok wyszedl poza okno
		{
			rect.Y = WYSOKOSC_OKNA.a - rect.Height;//ustaw maksymalnie przy koncowej krawedzi
		}

		Graphics graphics(hdc);
		Pen obrys(Color(255, r, g, b), 1);
		SolidBrush wypelnienie(Color(r, g, b));
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		graphics.DrawRectangle(&obrys, rect);
		graphics.FillRectangle(&wypelnienie, rect);
	}
};

rysuj_ramie ramie1;
rysuj_ramie ramie2;

rysuj_kolo kolo1;
rysuj_kolo kolo2;

rysuj_blok blok1;
rysuj_blok blok2;
rysuj_blok blok3;

//bloczki do identyfikowania wagi
rysuj_blok bloczek1;
rysuj_blok bloczek2;
rysuj_blok bloczek3;

class start_zapis
{
public:
	HWND hWnd;
	queue <double> nachylenie[2]; // 0 - ramie 1, 1 - ramie 2
	queue <int> blok[6];//przechowuje info na temat blokow : [0]-x,[1]-y (1 bloku) itd.
	char tryb = IDM_BRESET;
	int predkosc_odtwarzania = 25;

	void reset(void)
	{
		while (!nachylenie[0].empty())
		{
			nachylenie[0].pop();
			nachylenie[1].pop();
			blok[0].pop();
			blok[1].pop();
			blok[2].pop();
			blok[3].pop();
			blok[4].pop();
			blok[5].pop();
		}
	}
	void odtworz(void)
	{
		while (!nachylenie[0].empty())
		{
			ramie1.nachylenie = nachylenie[0].front();
			ramie2.nachylenie = nachylenie[1].front();


			blok1.polozenie_x = blok[0].front();
			blok1.polozenie_y = blok[1].front();

			blok2.polozenie_x = blok[2].front();
			blok2.polozenie_y = blok[3].front();

			blok3.polozenie_x = blok[4].front();
			blok3.polozenie_y = blok[5].front();

			InvalidateRect(hWnd, 0, 1);
			SendMessage(hWnd, WM_PAINT, -1, 0);// -1 aby mozna bylo schwac male bloki
			Sleep(1000 / predkosc_odtwarzania);
			nachylenie[0].pop();
			nachylenie[1].pop();
			blok[0].pop();
			blok[1].pop();
			blok[2].pop();
			blok[3].pop();
			blok[4].pop();
			blok[5].pop();
		}
		//aby bloczki byly ponownie widoczne
		SendMessage(hWnd, WM_PAINT, 0, 0);
		InvalidateRect(hWnd, 0, 1);
	}
	void wczytaj_dane(void)
	{
		nachylenie[0].push(ramie1.nachylenie);
		nachylenie[1].push(ramie2.nachylenie);
		blok[0].push(blok1.polozenie_x);
		blok[1].push(blok1.polozenie_y);
		blok[2].push(blok2.polozenie_x);
		blok[3].push(blok2.polozenie_y);
		blok[4].push(blok3.polozenie_x);
		blok[5].push(blok3.polozenie_y);
	}

};

start_zapis ruchy_ramienia;

struct uchwyt{
	HWND hwnd = 0;
};
uchwyt nagrywanie, start, pause, reset, odtworz, S_odtwarzanie, E_odtwarzanie, B_odtwarzanie;

char spr_pol(int poczatek_x, int poczatek_y, int wielkosc, int x_spr, int y_spr, int udzwig, int waga)//funkcja sprawdza czy x_spr i y_spr znajduja siê w zasiêgu podanego obszaru i czy ramie udzwignie blok
{
	if ((poczatek_x <= x_spr && poczatek_x + wielkosc >= x_spr)
		&& (poczatek_y <= y_spr && poczatek_y + wielkosc >= y_spr))
	{
		if (udzwig >= waga)
			return 1;
		else //jesli ramie znajduje sie w zasiegu bloku ale nie ma wystarczajacego udzwigu
			return -1;
	}
	else
		return 0;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	ramie1.poczatkowe_ramie = TRUE;
	blok1.nr = 0;
	blok2.nr = 1;
	blok3.nr = 2;

	bloczek1.nr = 1;
	bloczek1.wielkosc = 10;
	bloczek1.polozenie_x = 420;
	bloczek1.polozenie_y = 3;
	bloczek1.poczatek = 0;

	bloczek2.nr = 2;
	bloczek2.wielkosc = 10;
	bloczek2.polozenie_x = 490;
	bloczek2.polozenie_y = 3;
	bloczek2.poczatek = 0;

	bloczek3.nr = 3;
	bloczek3.wielkosc = 10;
	bloczek3.polozenie_x = 560;
	bloczek3.polozenie_y = 3;
	bloczek3.poczatek = 0;


	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_RAMIE_ROBOTA, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);


	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RAMIE_ROBOTA));


	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RAMIE_ROBOTA));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_RAMIE_ROBOTA);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		0, 0, 700, 700, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, SW_MAXIMIZE); //nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ruchy_ramienia.hWnd = hWnd;

	int wmId, wmEvent;

	HMODULE hModule = GetModuleHandleW(0);
	WCHAR path[100];


	bool wymazywanie_tla = FALSE;//przy odswiezaniu ekranu

	switch (message)
	{
	case WM_COMMAND:
	{
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_BZASTOSUJ:
		{
			TCHAR tekst[5];
			HWND hwnd_szybkosc = GetDlgItem(hWnd, IDM_ESZYBKOSC);
			GetWindowText(hwnd_szybkosc, tekst, 5);
			int liczba = _ttoi(tekst);
			ramie1.predkosc_zmiany = liczba;
			ramie2.predkosc_zmiany = liczba;
			ramie1.domyslna_predkosc = liczba;
			ramie2.domyslna_predkosc = liczba;
			SetFocus(hWnd);
			break;
		}
		case IDM_BMANUALNY:
		{

			ShowWindow(nagrywanie.hwnd, SW_HIDE);
			ShowWindow(start.hwnd, SW_HIDE);
			ShowWindow(reset.hwnd, SW_HIDE);
			ShowWindow(pause.hwnd, SW_HIDE);
			ShowWindow(odtworz.hwnd, SW_HIDE);
			ShowWindow(S_odtwarzanie.hwnd, SW_HIDE);
			ShowWindow(E_odtwarzanie.hwnd, SW_HIDE);
			ShowWindow(B_odtwarzanie.hwnd, SW_HIDE);
			SetFocus(hWnd);
			break;
		}
		case IDM_BAUTOMATYCZNY:
		{
			ShowWindow(nagrywanie.hwnd, SW_SHOW);
			ShowWindow(start.hwnd, SW_SHOW);
			ShowWindow(reset.hwnd, SW_SHOW);
			ShowWindow(pause.hwnd, SW_SHOW);
			ShowWindow(odtworz.hwnd, SW_SHOW);
			ShowWindow(S_odtwarzanie.hwnd, SW_SHOW);
			ShowWindow(E_odtwarzanie.hwnd, SW_SHOW);
			ShowWindow(B_odtwarzanie.hwnd, SW_SHOW);
			SetFocus(hWnd);
			break;
		}
		case IDM_BSTART:
		{
			ruchy_ramienia.tryb = IDM_BSTART;
			//zapamietuje poczakowe ulozenie ramion i blokow
			ruchy_ramienia.wczytaj_dane();

			SetFocus(hWnd);
			break;
		}
		case IDM_BPAUSE:
		{
			ruchy_ramienia.tryb = IDM_BPAUSE;
			SetFocus(hWnd);
			break;
		}
		case IDM_BRESET:
		{
			ruchy_ramienia.tryb = IDM_BRESET;
			ruchy_ramienia.reset();
			SetFocus(hWnd);
			break;
		}
		case IDM_BODTWORZ:
		{
			ruchy_ramienia.tryb = IDM_BODTWORZ;
			ruchy_ramienia.odtworz();
			SetFocus(hWnd);
			break;
		}
		case IDM_BODTWARZANIE:
		{
			TCHAR tekst[5];
			GetWindowText(E_odtwarzanie.hwnd, tekst, 5);
			int liczba = _ttoi(tekst);
			ruchy_ramienia.predkosc_odtwarzania = liczba;
			SetFocus(hWnd);
			break;
		}
		case IDM_BWAGA://ustawianie  wagi z kontrolek
		{
			TCHAR tekst[6];
			int liczba = 0;
			HWND hwnd_blok1 = GetDlgItem(hWnd, IDM_EBLOK1);
			HWND hwnd_blok2 = GetDlgItem(hWnd, IDM_EBLOK2);
			HWND hwnd_blok3 = GetDlgItem(hWnd, IDM_EBLOK3);
			HWND hwnd_udzwig = GetDlgItem(hWnd, IDM_EUDZWIG);

			GetWindowText(hwnd_blok1, tekst, 6);
			liczba = _ttoi(tekst);
			blok1.waga = liczba;
			blok1.poruszaj = 0;//zapobiega poruszaniu blokiem kiedy waga zostanie zmieniona w czasie trzymania go
			GetWindowText(hwnd_blok2, tekst, 6);
			liczba = _ttoi(tekst);
			blok2.waga = liczba;
			blok2.poruszaj = 0;
			GetWindowText(hwnd_blok3, tekst, 6);
			liczba = _ttoi(tekst);
			blok3.waga = liczba;
			blok3.poruszaj = 0;
			GetWindowText(hwnd_udzwig, tekst, 6);
			liczba = _ttoi(tekst);
			ramie1.udzwig = liczba;
			ramie2.udzwig = liczba;

			SetFocus(hWnd);
			break;
		}
		case ID_WYJSCIE:
			DestroyWindow(hWnd);
			break;
		case ID_ROZDZIELCZO32772://pelen ekran
			SZEROKOSC_OKNA.a = GetSystemMetrics(SM_CXSCREEN);//pobierz szerokosc ekranu
			WYSOKOSC_OKNA.a = GetSystemMetrics(SM_CYSCREEN);//wysokosc ekranu
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, SZEROKOSC_OKNA.a, WYSOKOSC_OKNA.a, 0);
			break;
		case ID_ROZDZIELCZO32773:
			SZEROKOSC_OKNA.a = 640;
			WYSOKOSC_OKNA.a = 480;
			SetWindowPos(hWnd, HWND_TOP, 0, 0, SZEROKOSC_OKNA.a, WYSOKOSC_OKNA.a, 0);
			break;
		case ID_ROZDZIELCZO32774:
			SZEROKOSC_OKNA.a = 800;
			WYSOKOSC_OKNA.a = 600;
			SetWindowPos(hWnd, HWND_TOP, 0, 0, SZEROKOSC_OKNA.a, WYSOKOSC_OKNA.a, 0);
			break;
		case ID_ROZDZIELCZO32775:
			SZEROKOSC_OKNA.a = 1024;
			WYSOKOSC_OKNA.a = 768;
			SetWindowPos(hWnd, HWND_TOP, 0, 0, SZEROKOSC_OKNA.a, WYSOKOSC_OKNA.a, 0);
			break;
		case ID_ROZDZIELCZO32776:
			SZEROKOSC_OKNA.a = 700;
			WYSOKOSC_OKNA.a = 700;
			SetWindowPos(hWnd, HWND_TOP, 0, 0, SZEROKOSC_OKNA.a, WYSOKOSC_OKNA.a, 0);
			break;
		case ID_RESTART:

			GetModuleFileName(hModule, path, 100);
			ShellExecute(0, 0, path, 0, 0, SW_SHOW);
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
	}
	case WM_PAINT:
	{

		PAINTSTRUCT ps;
		HDC hdc;
		HDC pamiec_hdc;
		HBITMAP pamiec_bitmapa;

		int bez_ods = 100; //wielkosc w px obszaru od góry okna która nie jest aktualizowana

		//pobiera wymiary okna, w razie jakichkolwiek ich modyfikacji
		RECT *wymiary = new RECT;

		GetClientRect(hWnd, wymiary);
		WYSOKOSC_OKNA.a = wymiary->bottom - wymiary->top - bez_ods;
		SZEROKOSC_OKNA.a = wymiary->right - wymiary->left;

		delete wymiary;

		hdc = BeginPaint(hWnd, &ps);
		pamiec_hdc = CreateCompatibleDC(hdc);
		pamiec_bitmapa = CreateCompatibleBitmap(hdc, SZEROKOSC_OKNA.a, WYSOKOSC_OKNA.a);
		//pobieranie koloru menu - taki sam jak kolor tla okna
		DWORD kolor_tla = GetSysColor(COLOR_MENU);
		BYTE skladowa_r = GetRValue(kolor_tla);
		BYTE skladowa_g = GetGValue(kolor_tla);
		BYTE skladowa_b = GetBValue(kolor_tla);

		//ustawianie identycznego tla na bitmapie
		BYTE *tlo = new BYTE[4 * SZEROKOSC_OKNA.a*(WYSOKOSC_OKNA.a)];
		LPBITMAPINFO naglowek = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];

		naglowek->bmiHeader.biBitCount = 32;
		naglowek->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		naglowek->bmiHeader.biWidth = SZEROKOSC_OKNA.a;
		naglowek->bmiHeader.biHeight = WYSOKOSC_OKNA.a;
		naglowek->bmiHeader.biPlanes = 1;
		naglowek->bmiHeader.biCompression = BI_RGB;
		naglowek->bmiHeader.biSizeImage = 0;
		naglowek->bmiHeader.biXPelsPerMeter = 0;
		naglowek->bmiHeader.biYPelsPerMeter = 0;
		naglowek->bmiHeader.biClrUsed = 0;
		naglowek->bmiHeader.biClrImportant = 0;

		for (int i = 0; i < (SZEROKOSC_OKNA.a*WYSOKOSC_OKNA.a); i++)
		{
			tlo[4 * i] = skladowa_r;//czerwony
			tlo[(4 * i) + 1] = skladowa_g; //zielony
			tlo[(4 * i) + 2] = skladowa_b; //niebieski
			tlo[(4 * i) + 3] = 0; //nieznaczacy
		}
		SetDIBits(pamiec_hdc, pamiec_bitmapa, 0, WYSOKOSC_OKNA.a, tlo, naglowek, DIB_RGB_COLORS);
		delete[] naglowek;
		delete[] tlo;
		//koniec bialego tla

		SelectObject(pamiec_hdc, pamiec_bitmapa);

		ramie1.start(pamiec_hdc);
		ramie2.start_x = ramie1.koniec_x;
		ramie2.start_y = ramie1.koniec_y;
		ramie2.start(pamiec_hdc);
		kolo1.start(pamiec_hdc, ramie1.start_x, ramie1.start_y);
		kolo2.start(pamiec_hdc, ramie2.start_x, ramie2.start_y);

		//jesli odblokowano tzn wcisnieto space przy ktoryms z blokow to zmieniaj jego polozenie zgodnie z koncowka ramienia 2
		if (blok1.poruszaj)
		{
			//przypisanie koorydnat konca ramienia z poprawka na uchwyt
			blok1.polozenie_x = ramie2.koniec_x - blok1.temp_x;
			blok1.polozenie_y = ramie2.koniec_y - blok1.temp_y;
		}
		if (blok2.poruszaj)
		{
			blok2.polozenie_x = ramie2.koniec_x - blok2.temp_x;
			blok2.polozenie_y = ramie2.koniec_y - blok2.temp_y;
		}
		if (blok3.poruszaj)
		{
			blok3.polozenie_x = ramie2.koniec_x - blok3.temp_x;
			blok3.polozenie_y = ramie2.koniec_y - blok3.temp_y;
		}

		blok1.start(pamiec_hdc, CZERWONY);
		blok2.start(pamiec_hdc, ZIELONY);
		blok3.start(pamiec_hdc, NIEBIESKI);

		BitBlt(hdc, 0, bez_ods, SZEROKOSC_OKNA.a, WYSOKOSC_OKNA.a, pamiec_hdc, 0, 0, SRCCOPY);


		if (wParam != -1)
		{
			bloczek1.start(hdc, CZERWONY);
			bloczek2.start(hdc, ZIELONY);
			bloczek3.start(hdc, NIEBIESKI);
		}

		DeleteObject(pamiec_bitmapa);
		DeleteDC(pamiec_hdc);
		DeleteDC(hdc);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_KEYDOWN:

		switch ((int)wParam)
		{
		case VK_LEFT:
			ramie1.predkosc_zmiany += 1;
			ramie1.nachylenie += 0.01 + (ramie1.predkosc_zmiany*0.005);
			InvalidateRect(hWnd, NULL, wymazywanie_tla);
			if (ruchy_ramienia.tryb == IDM_BSTART)
			{
				ruchy_ramienia.wczytaj_dane();
			}
			break;
		case VK_RIGHT:
			ramie1.predkosc_zmiany += 1;
			ramie1.nachylenie -= 0.01 + (ramie1.predkosc_zmiany*0.005);
			InvalidateRect(hWnd, NULL, wymazywanie_tla);
			if (ruchy_ramienia.tryb == IDM_BSTART)
			{
				ruchy_ramienia.wczytaj_dane();
			}
			break;
		case VK_UP:
			ramie2.predkosc_zmiany += 1;
			ramie2.nachylenie += 0.01 + (ramie2.predkosc_zmiany*0.005);
			InvalidateRect(hWnd, NULL, wymazywanie_tla);
			if (ruchy_ramienia.tryb == IDM_BSTART)
			{
				ruchy_ramienia.wczytaj_dane();
			}
			break;
		case VK_DOWN:
			ramie2.predkosc_zmiany += 1;
			ramie2.nachylenie -= 0.01 + (ramie2.predkosc_zmiany*0.005);
			InvalidateRect(hWnd, NULL, wymazywanie_tla);
			if (ruchy_ramienia.tryb == IDM_BSTART)
			{
				ruchy_ramienia.wczytaj_dane();
			}
			break;
		case VK_SPACE:
			char blok_ok[3];//przechowuje info na temat zwroconych danych z funkcji spr_pol

			blok_ok[0] = spr_pol(blok1.polozenie_x, blok1.polozenie_y, blok1.wielkosc, ramie2.koniec_x, ramie2.koniec_y, ramie2.udzwig, blok1.waga);
			blok_ok[1] = spr_pol(blok2.polozenie_x, blok2.polozenie_y, blok2.wielkosc, ramie2.koniec_x, ramie2.koniec_y, ramie2.udzwig, blok2.waga);
			blok_ok[2] = spr_pol(blok3.polozenie_x, blok3.polozenie_y, blok3.wielkosc, ramie2.koniec_x, ramie2.koniec_y, ramie2.udzwig, blok3.waga);

			if (blok_ok[0] == 1)//blok 1
			{
				if (blok1.poruszaj == FALSE)
				{
					blok1.poruszaj = TRUE;

					//zapisywanie odleglosci konca ramienia od poczatku bloku
					blok1.temp_x = ramie2.koniec_x - blok1.polozenie_x;
					blok1.temp_y = ramie2.koniec_y - blok1.polozenie_y;

					blok2.poruszaj = FALSE;
					blok3.poruszaj = FALSE;
				}
				else blok1.poruszaj = FALSE;//mozliwosc zmiany stanu chwytania przy nacisnieciu space
			}
			else if (blok_ok[1] == 1)//blok 2
			{
				if (blok2.poruszaj == FALSE)
				{
					blok2.poruszaj = TRUE;

					//zapisywanie odleglosci konca ramienia od poczatku bloku
					blok2.temp_x = ramie2.koniec_x - blok2.polozenie_x;
					blok2.temp_y = ramie2.koniec_y - blok2.polozenie_y;

					blok1.poruszaj = FALSE;
					blok3.poruszaj = FALSE;
				}
				else blok2.poruszaj = FALSE;
			}
			else if (blok_ok[2] == 1)//blok 3
			{
				if (blok3.poruszaj == FALSE)
				{
					blok3.poruszaj = TRUE;

					//zapisywanie odleglosci konca ramienia od poczatku bloku
					blok3.temp_x = ramie2.koniec_x - blok3.polozenie_x;
					blok3.temp_y = ramie2.koniec_y - blok3.polozenie_y;

					blok1.poruszaj = FALSE;
					blok2.poruszaj = FALSE;
				}
				else blok3.poruszaj = FALSE;
			}
			else
			{
				blok1.poruszaj = FALSE;
				blok2.poruszaj = FALSE;
				blok3.poruszaj = FALSE;
			}

			if (blok_ok[0] == -1 || blok_ok[1] == -1 || blok_ok[2] == -1)
			{
				MessageBox(0, TEXT("Przekroczono dopuszczalny udŸwig !"), TEXT("Brak mo¿liwoœci podniesienia."), MB_OK);
			}
			break;
		}
		break;
	case WM_KEYUP:
		switch ((int)wParam)
		{
		case VK_RIGHT:
			ramie1.predkosc_zmiany = ramie1.domyslna_predkosc;
			break;
		case VK_UP:
			ramie2.predkosc_zmiany = ramie1.domyslna_predkosc;
			break;
		case  VK_LEFT:
			ramie1.predkosc_zmiany = ramie2.domyslna_predkosc;
			break;
		case VK_DOWN:
			ramie2.predkosc_zmiany = ramie2.domyslna_predkosc;
			break;

		}
		break;
	case WM_CREATE:
	{

		CreateWindowEx(NULL, TEXT("STATIC"), TEXT("Szybkoœæ poruszania:"), WS_VISIBLE | WS_CHILD, 3, 0, 150, 20, hWnd, NULL, NULL, NULL);
		CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("0"), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 150, 0, 30, 20, hWnd, (HMENU)IDM_ESZYBKOSC, hInst, NULL);
		CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("Zastosuj"), WS_VISIBLE | WS_CHILD, 180, 0, 100, 20, hWnd, (HMENU)IDM_BZASTOSUJ, NULL, NULL);

		CreateWindowEx(NULL, TEXT("STATIC"), TEXT("Ustawienia wagi:"), WS_VISIBLE | WS_CHILD, 300, 0, 110, 20, hWnd, NULL, NULL, NULL);
		CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("100"), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 440, 0, 40, 20, hWnd, (HMENU)IDM_EBLOK1, hInst, NULL);
		CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("100"), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 510, 0, 40, 20, hWnd, (HMENU)IDM_EBLOK2, hInst, NULL);
		CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("100"), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 580, 0, 40, 20, hWnd, (HMENU)IDM_EBLOK3, hInst, NULL);
		CreateWindowEx(NULL, TEXT("STATIC"), TEXT("UdŸwig ramienia:"), WS_VISIBLE | WS_CHILD, 630, 0, 150, 20, hWnd, NULL, NULL, NULL);
		CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("100"), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 750, 0, 40, 20, hWnd, (HMENU)IDM_EUDZWIG, hInst, NULL);
		CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("Zastosuj"), WS_VISIBLE | WS_CHILD, 800, 0, 100, 20, hWnd, (HMENU)IDM_BWAGA, NULL, NULL);


		CreateWindowEx(NULL, TEXT("STATIC"), TEXT("Tryb:"), WS_VISIBLE | WS_CHILD, 3, 20, 50, 20, hWnd, NULL, NULL, NULL);
		CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("MANUALNY"), WS_VISIBLE | WS_CHILD | WS_GROUP | BS_AUTORADIOBUTTON, 50, 20, 130, 20, hWnd, (HMENU)IDM_BMANUALNY, NULL, NULL);
		CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("AUTOMATYCZNY"), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 180, 20, 130, 20, hWnd, (HMENU)IDM_BAUTOMATYCZNY, NULL, NULL);
		CheckRadioButton(hWnd, IDM_BMANUALNY, IDM_BAUTOMATYCZNY, IDM_BMANUALNY);//domyslnie ustaw zaznaczenie jako manualny

		S_odtwarzanie.hwnd = CreateWindowEx(NULL, TEXT("STATIC"), TEXT("Szybkoœæ odtwarzania:"), WS_CHILD, 320, 20, 150, 20, hWnd, NULL, NULL, NULL);
		E_odtwarzanie.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("25"), WS_CHILD | WS_BORDER | ES_NUMBER, 470, 20, 40, 20, hWnd, NULL, hInst, NULL);
		B_odtwarzanie.hwnd = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("Zastosuj"), WS_CHILD, 520, 20, 100, 20, hWnd, (HMENU)IDM_BODTWARZANIE, NULL, NULL);

		nagrywanie.hwnd = CreateWindowEx(NULL, TEXT("STATIC"), TEXT("Nagrywanie ruchów:"), WS_CHILD, 3, 40, 150, 20, hWnd, (HMENU)IDM_SNAGRYWANIE, NULL, NULL);
		start.hwnd = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("START"), WS_CHILD | WS_GROUP | BS_AUTORADIOBUTTON, 150, 40, 80, 20, hWnd, (HMENU)IDM_BSTART, NULL, NULL);
		pause.hwnd = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("PAUSE"), WS_CHILD | BS_AUTORADIOBUTTON, 230, 40, 80, 20, hWnd, (HMENU)IDM_BPAUSE, NULL, NULL);
		reset.hwnd = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("RESET"), WS_CHILD | BS_AUTORADIOBUTTON, 310, 40, 80, 20, hWnd, (HMENU)IDM_BRESET, NULL, NULL);
		odtworz.hwnd = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("ODTWÓRZ"), WS_CHILD | BS_AUTORADIOBUTTON, 390, 40, 100, 20, hWnd, (HMENU)IDM_BODTWORZ, NULL, NULL);
		CheckRadioButton(hWnd, IDM_BSTART, IDM_BODTWORZ, IDM_BRESET);
		break;

	}

	case WM_DESTROY:
		GdiplusShutdown(m_gdiplusToken);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
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
