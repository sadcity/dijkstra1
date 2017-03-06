#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <malloc.h>
#include "resource.h"
#define UM_INVALIDATERECT WM_USER + 10
#define MAX_NODE 100		//������������,�ɵ�����������ʾ�����������
#define MAX_DIST 10001		//���徰��������

LRESULT __stdcall WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

/******************************

	������ں��� WinMain

*******************************/
int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int cxScreen = 0, cyScreen = 0;
	cxScreen = GetSystemMetrics(SM_CXSCREEN);	//��Ļ���
	cyScreen = GetSystemMetrics(SM_CYSCREEN);	//��Ļ�߶�
	if (cxScreen < 1300 || cyScreen < 730)
	{
		MessageBox(NULL, _T("��Ļ�ֱ��ʽϵͣ��޷����иó���"), _T("����ʯ�ʹ�ѧ��У��������Ϣ��ѯϵͳ"), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	TCHAR *szWindowClass = _T("daoyou");
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON_LOGO));	//����ͼ��
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1); 
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_TOP);					//���ز˵�
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindowEx(WS_EX_APPWINDOW, szWindowClass, _T("����ʯ�ʹ�ѧ��У��������Ϣ��ѯϵͳ"),
		WS_TILEDWINDOW&~WS_SIZEBOX&~WS_MAXIMIZEBOX, (cxScreen - 1300) / 2, (cyScreen - 700) / 2 - 15, 1300, 700, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

/*************************************

	����ȫ�ֱ���

*************************************/
typedef struct _SightNode
{
	int NodeNum;				//������
	int NodeXPos, NodeYPos;		//����� X��Y ����
	BOOL Checked;				//�Ƿ�ѡ��
} SightNode;
int CurOperation = 0;			//������ʶ��ǰ������1������Ӿ��㣬2��������·��
SightNode Sights[MAX_NODE] = { 0 };	//���嶥����Ϣ�ṹ������
int NodeNum = 0;				//��������

typedef struct _PathNode		//����·����Ϣ�ṹ��
{
	int StartNode;				//��ʼ������
	int EndNode;				//����������
	int PathLength;				//·������
} PathNode;

PathNode Paths[MAX_NODE * 2] = { 0 };//����·����Ϣ�ṹ������
int PathNum = 0;

typedef struct _TwoPathNode
{
	int Node1, Node2;
	int mouse_x, mouse_y;
} TwoPathNode;

int StartSight = -1, EndSight = -1;

int CheckedNum = 0;
static int StartNode = -1;
static int EndNode = -1;

/***********************

    ��Ӿ�����Ϣ�Ի�����̺���

************************/
LRESULT __stdcall SetPathProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
	case WM_INITDIALOG:
	{	
		TwoPathNode *PNode = NULL;
		PNode = (TwoPathNode*)lParam;
		TCHAR PathStr[50] = { 0 };
		RECT rect;
		GetWindowRect(hWnd, &rect);
		wsprintf(PathStr, _T("������  ����%d - ����%d  ֮��ľ���"), PNode->Node1,PNode->Node2);
		SetWindowText(hWnd, PathStr);
		SetDlgItemText(hWnd, IDC_REMIND, _T(" "));
		SetFocus(GetDlgItem(hWnd, IDC_EDIT_PathLength));
		SetWindowPos(hWnd, NULL, PNode->mouse_x + 70, PNode->mouse_y - 10, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
		break;
	}
	case WM_CLOSE:
	{
		EndDialog(hWnd, 1);
		break;
	}
	case WM_COMMAND:
	{
		switch (wParam)
			{
			case IDC_OK:
			{
				int PathLength = 0;
				PathLength = GetDlgItemInt(hWnd, IDC_EDIT_PathLength, NULL, FALSE);
				if (0 == PathLength)
				{
					SetDlgItemText(hWnd, IDC_REMIND, _T("�� �� �� �� Ϊ �� �� �� ��"));
				}
				else if (PathLength >= MAX_DIST)
				{
					SetDlgItemText(hWnd, IDC_REMIND, _T("�� �� �� ���� �� �� �� �� ��"));
				}
				else
				{
					Paths[PathNum - 1].PathLength = PathLength;
					EndDialog(hWnd, 1);
				}
				break;
			}			
			case IDC_CANCEL:
				PathNum -= 1;
				EndDialog(hWnd, 1);
				break;
			default:
				break;
			}
		break;
	}	
	}
	return 0;
}

LRESULT __stdcall SetNodeInfoProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
	case WM_INITDIALOG:
	{
		/*TwoPathNode *PNode = NULL;
		PNode = (TwoPathNode*)lParam;
		TCHAR PathStr[50] = { 0 };
		RECT rect;
		GetWindowRect(hWnd, &rect);
		wsprintf(PathStr, _T("������  ����%d - ����%d  ֮��ľ���"), PNode->Node1, PNode->Node2);
		SetWindowText(hWnd, PathStr);
		SetDlgItemText(hWnd, IDC_REMIND, _T(" "));
		SetFocus(GetDlgItem(hWnd, IDC_EDIT_PathLength));
		SetWindowPos(hWnd, NULL, PNode->mouse_x + 70, PNode->mouse_y - 10, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);*/
		break;
	}
	case WM_CLOSE:
	{
		EndDialog(hWnd, 1);
		break;
	}
	case WM_COMMAND:
	{
		switch (wParam)
		{
		case IDC_OK:
		{
			
			break;
		}
		case IDC_CANCEL:
			EndDialog(hWnd, 1);
			break;
		default:
			break;
		}
		break;
	}
	}
	return 0;
}

/**************************

	���ƴ���������

***************************/
void DrawInitFrame(HDC hdc)
{
	SIZE sz;
	HFONT NewFont, OldFont;
	HPEN hPen, hOldPen;
	HBRUSH hBrush;
	hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	hOldPen = (HPEN)SelectObject(hdc, hPen);
	Rectangle(hdc, 30, 60, 800, 550);
	hBrush = CreateSolidBrush(RGB(250, 251, 251));
	SelectObject(hdc, hBrush);
	hPen = CreatePen(PS_SOLID, 1, RGB(253, 251, 249));
	hOldPen = (HPEN)SelectObject(hdc, hPen);
	Rectangle(hdc, 800, 60, 1250, 550);
	hBrush = CreateSolidBrush(RGB(246, 246, 246));
	SelectObject(hdc, hBrush);
	hPen = CreatePen(PS_SOLID, 1, RGB(246, 246, 246));
	hOldPen = (HPEN)SelectObject(hdc, hPen);
	Rectangle(hdc, 30, 550, 1250, 600);

	hPen = CreatePen(PS_SOLID, 1, RGB(230, 230, 230));
	hOldPen = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, 30, 550, NULL);
	LineTo(hdc, 1250, 550);
	MoveToEx(hdc, 800, 60, NULL);
	LineTo(hdc, 800, 600);
	MoveToEx(hdc, 30, 60, NULL);
	LineTo(hdc, 1250, 60);
	LineTo(hdc, 1250, 600);
	LineTo(hdc, 30, 600);
	LineTo(hdc, 30, 60);
	MoveToEx(hdc, 800, 110, NULL);
	LineTo(hdc, 1250, 110);
	MoveToEx(hdc, 800, 240, NULL);
	LineTo(hdc, 1250, 240);
	
	NewFont = CreateFont(26, 13, 0, 0, FW_THIN, FALSE, FALSE, FALSE, CHINESEBIG5_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_MODERN, _T("����"));
	OldFont = SelectObject(hdc, NewFont);
	SetTextColor(hdc, RGB(176, 34, 77));
	SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
	TCHAR *TopTitle = _T("����ʯ�ʹ�ѧ��У��������Ϣ��ѯϵͳ");
	GetTextExtentPoint(hdc, TopTitle, _tcslen(TopTitle), &sz);
	TextOut(hdc, 650 - sz.cx / 2, 20, TopTitle, _tcslen(TopTitle));
	SelectObject(hdc, OldFont);
}

/**************************

	���ƾ�����

***************************/
void DrawNode(HDC hdc)
{
	int i = 0;
	TCHAR CurNode[20] = { 0 }, NodeNumStr[20] = { 0 }, PathNumStr[20] = { 0 };
	HBRUSH hBrush, hOldBrush;
	HPEN hPen, hOldPen;
	HFONT NewFont, OldFont;
	SIZE sz;
	hBrush = hOldBrush = CreateSolidBrush(RGB(255, 0, 128));
	SelectObject(hdc, hBrush);
	SetBkColor(hdc, RGB(255, 0, 128));
	SetTextColor(hdc, RGB(255, 255, 255));
	hPen = hOldPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 128));
	SelectObject(hdc, hPen);
	for (i = 0; i < NodeNum; i++)
	{
		if (2 == CurOperation && Sights[i].Checked)
		{
			hBrush = CreateSolidBrush(RGB(69, 69, 69));
			hOldBrush = SelectObject(hdc, hBrush);
			hPen = CreatePen(PS_SOLID, 1, RGB(69, 69, 69));
			hOldPen = SelectObject(hdc, hPen);
			SetBkColor(hdc, RGB(69, 69, 69));
		}
		else if (3 == CurOperation && Sights[i].Checked)
		{
			hBrush = CreateSolidBrush(RGB(128, 0, 255));
			hOldBrush = SelectObject(hdc, hBrush);
			hPen = CreatePen(PS_SOLID, 1, RGB(128, 0, 255));
			hOldPen = SelectObject(hdc, hPen);
			SetBkColor(hdc, RGB(128, 0, 255));
		}
		_itot(i + 1, CurNode, 10);
		GetTextExtentPoint(hdc, CurNode, _tcslen(CurNode), &sz);
		Ellipse(hdc, Sights[i].NodeXPos - 16, Sights[i].NodeYPos - 16, Sights[i].NodeXPos + 16, Sights[i].NodeYPos + 16);
		TextOut(hdc, Sights[i].NodeXPos - sz.cx / 2, Sights[i].NodeYPos - sz.cy / 2, CurNode, _tcslen(CurNode));
		SelectObject(hdc, hOldBrush);
		SelectObject(hdc, hOldPen);
		SetBkColor(hdc, RGB(255, 0, 128));
	}

	NewFont = CreateFont(18, 9, 0, 0, FW_THIN, FALSE, FALSE, FALSE, CHINESEBIG5_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_MODERN, _T("����"));
	OldFont = SelectObject(hdc, NewFont);
	SetTextColor(hdc, RGB(3, 4, 255));
	SetBkColor(hdc, RGB(250, 251, 251));
	wsprintf(NodeNumStr, _T("����������%d"), NodeNum);
	TextOut(hdc, 810, 77,NodeNumStr, _tcslen(NodeNumStr));
	wsprintf(PathNumStr, _T("��������%d"), PathNum);
	TextOut(hdc, 970, 77, PathNumStr, _tcslen(NodeNumStr));
	SelectObject(hdc, OldFont);
}

/*****************************

	���ƾ����·��

*****************************/
void DrawPaths(HDC hdc)
{
	if (PathNum > 0)
	{
		int i, PathNumXPos = 0, PathNumYPos = 0;
		TCHAR PathLengthStr[10] = { 0 };
		HPEN hPen, hOldPen;
		hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
		hOldPen = SelectObject(hdc, hPen);
		for (i = 0; i < PathNum; i++)
		{
			MoveToEx(hdc, Sights[Paths[i].StartNode - 1].NodeXPos, Sights[Paths[i].StartNode - 1].NodeYPos, NULL);
			LineTo(hdc, Sights[Paths[i].EndNode - 1].NodeXPos, Sights[Paths[i].EndNode - 1].NodeYPos);
			if (Paths[i].PathLength)
			{
				SetTextColor(hdc, RGB(0, 0, 0));
				SetBkColor(hdc, RGB(255, 255, 255));
				PathNumXPos = (Sights[Paths[i].StartNode - 1].NodeXPos + Sights[Paths[i].EndNode - 1].NodeXPos) / 2;
				PathNumYPos = (Sights[Paths[i].StartNode - 1].NodeYPos + Sights[Paths[i].EndNode - 1].NodeYPos) / 2;
				_itot(Paths[i].PathLength, PathLengthStr, 10);
				TextOut(hdc, PathNumXPos, PathNumYPos, PathLengthStr, _tcslen(PathLengthStr));
			}		
		}
		SelectObject(hdc, hOldPen);
	}
}

/***************************

	�жϾ���֮������Ƿ�̫��

****************************/
BOOL IsNodePosAvailable(int newX, int newY)
{
	int i = 0;
	double range = 0;
	for (i = 0; i < NodeNum; i++)
	{
		range = pow((newX - Sights[i].NodeXPos), 2) + pow((newY - Sights[i].NodeYPos), 2);
		if (range < 5000)
			return FALSE;
	}
	return TRUE;
}

/*********************************

	���Ʋ˵���ť

**********************************/
void DrawMenuBotton(HDC hdc)
{
	HBRUSH hBrush1;
	HPEN hPen, hOldPen;
	HFONT NewFont, OldFont;
	hPen = CreatePen(PS_SOLID, 0, RGB(243, 156, 18));
	hOldPen = (HPEN)SelectObject(hdc, hPen);
	hBrush1 = CreateSolidBrush(RGB(243, 156, 18));
	SetBkColor(hdc, RGB(243, 156, 18));
	SelectObject(hdc, hBrush1);
	NewFont = CreateFont(17, 8, 0, 0, FW_THIN, FALSE, FALSE, FALSE, CHINESEBIG5_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_MODERN, _T("����"));
	OldFont = SelectObject(hdc, NewFont);
	SetTextColor(hdc, RGB(255, 255, 255));
	RoundRect(hdc, 50, 560, 140, 590, 10, 10);
	TextOut(hdc, 63, 566, _T("��Ӿ���"), 4);
	RoundRect(hdc, 150, 560, 240, 590, 10, 10);
	TextOut(hdc, 163, 566, _T("���þ���"), 4);
	RoundRect(hdc, 250, 560, 340, 590, 10, 10);
	TextOut(hdc, 263, 566, _T("����·��"), 4);
	RoundRect(hdc, 350, 560, 440, 590, 10, 10);
	TextOut(hdc, 363, 566, _T("����·��"), 4);
	RoundRect(hdc, 450, 560, 540, 590, 10, 10);
	TextOut(hdc, 463, 566, _T("ѡ�񾰵�"), 4);
	RoundRect(hdc, 550, 560, 670, 590, 10, 10);
	TextOut(hdc, 563, 566, _T("������Ϣ��ѯ"), 6);

	RoundRect(hdc, 820, 560, 941, 590, 10, 10);
	TextOut(hdc, 833, 566, _T("���·����ѯ"), 6);	
}

/***************************

	�������·��

***************************/
DrawSelectedNode(HDC hdc)
{
	TCHAR StartNumStr[10] = { 0 };
	TCHAR EndNumStr[10] = { 0 };
	if (StartSight != -1)
	{
		SetTextColor(hdc, RGB(69,69,69));
		SetBkColor(hdc, RGB(250, 251, 251));
		wsprintf(StartNumStr, _T("��ʼ���㣺%d"), StartSight);
		TextOut(hdc, 810, 165, StartNumStr, _tcslen(StartNumStr));
	}
	if (EndSight != -1)
	{
		wsprintf(EndNumStr, _T("��ֹ���㣺%d"), EndSight);
		TextOut(hdc, 810, 200, EndNumStr, _tcslen(EndNumStr));
	}
}

/********************************

	��̬�����ڽӾ���

********************************/

/**
*��̬�����ڽӾ����ά����
*@param rows   ��ά��������
*@param cols   ��ά��������
*@return int** ָ���ά����ͷ�Ķ���ָ��
*/
int** CreateMatrix(int rows, int cols)
{
	int i;
	int **p = NULL;
	p = (int**)malloc(sizeof(int*) * rows);
	if (!p)
	{
		return NULL;
	}
	for (i = 0; i < rows; i++)
	{
		p[i] = (int*)malloc(sizeof(int) * cols);
		if (!p[i])
		{
			return NULL;
		}
	}
	
	return p;
}

/********************************

	��ʼ���ڽӾ���

*******************************/
void InitMatrix(int **p)
{
	int i, j;
	for (i = 0; i < NodeNum; i++)
	{
		for (j = 0; j < NodeNum; j++)
		{
			p[i][j] = MAX_DIST;
		}
		p[i][i] = 0;
	}
	for (i = 0; i < PathNum; i++)
	{
		p[Paths[i].StartNode - 1][Paths[i].EndNode - 1] = Paths[i].PathLength;
		p[Paths[i].EndNode - 1][Paths[i].StartNode - 1] = Paths[i].PathLength;
	}
}

/*******************************

	Dijkstra�㷨����̾���

********************************/
/**
*����Dijkstra�㷨�����·��
*@param n	 �������
*@param v	 ��ʼ����
*@param prev ���澰��ǰһ�����������
*@param G	 �ڽӾ���
*/
void Dijkstra(int n, int v, int *dist, int *prev, int** G)
{
	BOOL *s = (BOOL*)malloc(sizeof(BOOL) * n);   
	for (int i = 0; i < n; i++)
	{
		dist[i] = G[v][i];
		s[i] = FALSE;     
		if (dist[i] == MAX_DIST)
		{
			prev[i] = 0;
		}	
		else
		{
			prev[i] = v;
		}
	}
	dist[v] = 0;
	s[v] = TRUE;

	for (int i = 1; i < n; i++)
	{
		int tmp = MAX_DIST;
		int u = v;
		for (int j = 0; j < n; j++)
		{
			if ((!s[j]) && dist[j] < tmp)
			{
				u = j;              
				tmp = dist[j];
			}
		}		
		s[u] = TRUE;   					
		for (int j = 0; j < n; j++)
		{
			if ((!s[j]) && G[u][j] < MAX_DIST)
			{
				int newdist = dist[u] + G[u][j];
				if (newdist < dist[j])
				{
					dist[j] = newdist;
					prev[j] = u;
				}
			}
		}			
	}
}

/***********************************

	�������·��

**********************************/
/**
*����·����Ϣ
*@param Prev  ���澰��ǰһ�����������
*@param v	  ��ʼ����
*@param u     ��������
*@return int* ����·������
*/
int* SearchPaths(int *Prev, int v, int u)
{
	int *que = (int*)malloc(sizeof(int) * (NodeNum));
	memset(que, -1, sizeof(int) * (NodeNum));
	int tot = 0;
	que[tot] = u;
	tot++;
	int tmp = Prev[u];
	while (tmp != v)
	{
		que[tot] = tmp;
		tot++;
		tmp = Prev[tmp];
	}
	que[tot] = v;
	
	return que;
}

/********************************

	���ò�����ʾ��Ϣ

********************************/
void SetOperateInfo(HWND hWnd, int oper, int node1, int node2)
{
	HFONT hFont, hOldFont;
	HDC hdc = GetDC(hWnd);
	hFont = CreateFont(17, 8, 0, 0, FW_THIN, FALSE, FALSE, FALSE, CHINESEBIG5_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_MODERN, _T("����"));
	hOldFont = SelectObject(hdc, hFont);
	SetBkColor(hdc, RGB(250, 251, 251));
	TextOut(hdc, 810, 130, _T("��ǰ������"), 5);
	SetTextColor(hdc, RGB(255, 0, 0));
	if (1 == oper)
	{
		TextOut(hdc, 890, 130, _T("��Ӿ���"), 6);
	}
	else if (2 == oper)
	{
		TextOut(hdc, 890, 130, _T("����·��"), 4);
	}
	else if (3 == oper)
	{
		TextOut(hdc, 890, 130, _T("ѡ�񾰵�"), 4);
	}
	else if (4 == oper)
	{
		TextOut(hdc, 890, 130, _T("���·����ѯ"), 6);
	}
	SetTextColor(hdc, RGB(0, 0, 0));
	ReleaseDC(hWnd, hdc);
}

/********************************

	������·�����

********************************/
void DrawShortestPaths(HWND hWnd, int Dist, int *PathsNode)
{
	HDC hdc;
	HFONT NewFont, OldFont;
	HPEN hOldPen, hPen;
	TCHAR DistStr[10] = { 0 }, PathNodeStr[10] = { 0 };
	hdc = GetDC(hWnd);
	SetBkColor(hdc, RGB(250, 251, 251));
	NewFont = CreateFont(17, 8, 0, 0, FW_THIN, FALSE, FALSE, FALSE, CHINESEBIG5_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_MODERN, _T("����"));
	OldFont = SelectObject(hdc, NewFont);
	TextOut(hdc, 810, 260, _T("��̾��룺"), 5);
	if (Dist == MAX_DIST)
	{
		TextOut(hdc, 890, 260, _T("������֮��û��·��"), 9);
	}
	else
	{
		hPen = CreatePen(PS_SOLID, 3, RGB(0, 255, 0));
		hOldPen = SelectObject(hdc, hPen);
		_itot(Dist, DistStr, 10);
		SetTextColor(hdc, RGB(0, 0, 255));
		TextOut(hdc, 890, 260, DistStr, _tcslen(DistStr));
		SetTextColor(hdc, RGB(0, 0, 0));
		TextOut(hdc, 810, 310, _T("���·����"), 5);
		SetTextColor(hdc, RGB(0, 0, 255));
		for (int j = 0, i = NodeNum - 1; i >= 0; i--)
		{
			if (-1 == PathsNode[i])
			{
				continue;
			}
			if (i > 0)
			{
				MoveToEx(hdc, Sights[PathsNode[i]].NodeXPos, Sights[PathsNode[i]].NodeYPos, NULL);
				LineTo(hdc, Sights[PathsNode[i - 1]].NodeXPos, Sights[PathsNode[i - 1]].NodeYPos);
			}
			_itot(PathsNode[i] + 1, PathNodeStr, 10);
			TextOut(hdc, 893 + 25 * j, 310, PathNodeStr, _tcslen(PathNodeStr));
			j++;
		}
		SetTextColor(hdc, RGB(0, 0, 0));
		SelectObject(hdc, hOldPen);
	}
	ReleaseDC(hWnd, hdc);
}


/********************************

	��Ӿ��㰴ť����¼�

********************************/
void AddNodeClickEvent(HWND hWnd)
{
	CurOperation = 1;
	SetOperateInfo(hWnd, 1, -1, -1);
}

/********************************

	���þ��㰴ť����¼�

********************************/
void ResetNodeClickEvent(HWND hWnd)
{
	CurOperation = 0;
	if (0 == NodeNum)
	{
		MessageBox(hWnd, _T("������Ӿ��㣡"), _T("���þ���"), MB_OK | MB_ICONEXCLAMATION);
	}
	else
	{
		if (IDYES == MessageBox(hWnd, _T("ȷ�����þ�����Ϣ��"), _T("��ʾ"), MB_YESNO | MB_ICONEXCLAMATION))
		{
			NodeNum = 0;
			PathNum = 0;
			memset(Sights, 0, sizeof(Sights));
			memset(Paths, 0, sizeof(Paths));
			SendMessage(hWnd, UM_INVALIDATERECT, 0, 0);
		}
	}	
}

/********************************

	����·�߰�ť����¼�

********************************/
void SetLinesClickEvent(HWND hWnd)
{
	if (0 == NodeNum)
	{
		MessageBox(hWnd, _T("������Ӿ��㣡"), _T("����·��"), MB_OK | MB_ICONEXCLAMATION);
	}
	else
	{
		CurOperation = 2;
		for (int i = 0; i < NodeNum; i++)
		{
			Sights[i].Checked = FALSE;
		}
		StartSight = -1;
		EndSight = -1;
		SendMessage(hWnd, UM_INVALIDATERECT, 0, 0);
	}	
}

/******************************

	����·�߰�ť����¼�

*******************************/
void ResetLinesClickEvent(HWND hWnd)
{
	CurOperation = 0;
	if (0 == NodeNum)
	{
		MessageBox(hWnd, _T("������Ӿ��㣡"), _T("����·��"), MB_OK | MB_ICONEXCLAMATION);
	}
	else if (0 == PathNum)
	{
		MessageBox(hWnd, _T("��������·�ߣ�"), _T("����·��"), MB_OK | MB_ICONEXCLAMATION);
	}
	else
	{
		if (IDYES == MessageBox(hWnd, _T("ȷ������·����"), _T("��ʾ"), MB_YESNO | MB_ICONEXCLAMATION))
		{
			PathNum = 0;
			memset(Paths, 0, sizeof(Paths));
			SendMessage(hWnd, UM_INVALIDATERECT, 0, 0);
		}	
	}
}

/******************************

	ѡ�񾰵㰴ť����¼�

*******************************/
void SelectNodeClickEvent(HWND hWnd)
{
	CurOperation = 3;
	if(StartSight != -1)
		Sights[StartSight - 1].Checked = FALSE;
	if(EndSight != -1)
		Sights[EndSight - 1].Checked = FALSE;
	StartSight = -1;
	EndSight = -1;
	CheckedNum = 0;
	SendMessage(hWnd, UM_INVALIDATERECT, -1, -1);
}

/******************************

	��ѯ·�߰�ť����¼�

*******************************/
void QueryLinesClickEvent(HWND hWnd)
{
	if (-1 == StartSight)
	{
		MessageBox(hWnd, _T("��ѡ����ʼ���㣡"), _T("·����ѯ"), MB_OK);
	}
	else if(-1 == EndSight)
	{
		MessageBox(hWnd, _T("��ѡ���յ㾰�㣡"), _T("·����ѯ"), MB_OK);
	}
	else
	{
		int *Distance = (int*)malloc(sizeof(int) * NodeNum);
		int *PrevNode = (int*)malloc(sizeof(int) * NodeNum);
		memset(Distance, MAX_DIST, sizeof(int) * NodeNum);
		memset(PrevNode, -1, sizeof(int) * NodeNum);
		CurOperation = 4;
		int **G = CreateMatrix(NodeNum, NodeNum);
		InitMatrix(G);
		Dijkstra(NodeNum, StartSight - 1, Distance, PrevNode, G);					//Dijkstra�㷨����̾���
		int *ShortestPaths = SearchPaths(PrevNode, StartSight - 1, EndSight - 1);	//����̾���·��
		
		DrawShortestPaths(hWnd, Distance[EndSight - 1], ShortestPaths);
	}
}

/*****************************

	���ò˵���ť����¼�

*****************************/
void SetBtnClickEvents(HWND hWnd, int cx, int cy)
{
	if ((cx > 30) && (cx < 1250) && (cy >= 560) & (cy <= 590))
	{
		if (cx >= 50 && cx <= 140)
		{
			AddNodeClickEvent(hWnd);		//��Ӿ���
		}
		else if (cx >= 150 && cx <= 240)
		{
			ResetNodeClickEvent(hWnd);		//���þ���
		}
		else if (cx >= 250 && cx <= 340)
		{
			SetLinesClickEvent(hWnd);		//����·��
		}
		else if (cx >= 350 && cx <= 440)
		{
			ResetLinesClickEvent(hWnd);		//����·��
		}
		else if (cx >= 450 && cx <= 540)	
		{
			SelectNodeClickEvent(hWnd);		//ѡ�񾰵�
		}
		else if (cx >= 820 && cx <= 941)
		{
			QueryLinesClickEvent(hWnd);		//·����ѯ
		}
	}
}

/***************************

	��ȡ��ǰ����ľ�����

**************************/
/**
*��ȡ��ǰ����ľ�����
*@param hWnd ���ھ��
*@param cx	 �������X����
*@param cy	 �������Y����
*@return int ��ǰ���������
*/
int GetCurClickedNode(HWND hWnd, int cx, int cy)
{
	int i;
	for (i = 0; i < NodeNum; i++)
	{
		double range = pow(Sights[i].NodeXPos - cx, 2) + pow(Sights[i].NodeYPos - cy, 2);
		if (range < 16 * 16)
		{
			return Sights[i].NodeNum;
		}
	}
	return -1;
}

/***************************

	�жϽ��·���Ƿ��Ѵ���

***************************/
/**
*�ж��������·���Ƿ�������
*@param Node1 ����1���
*@param Node2 ����2���
*@return BOOL �Ƿ�����
*/
BOOL IsPathExist(int Node1, int Node2)
{
	int i;
	for (i = 0; i < PathNum; i++)
	{
		if ((Node1 == Paths[i].StartNode && Node2 == Paths[i].EndNode) || (Node2 == Paths[i].StartNode && Node1 == Paths[i].EndNode))
		{
			return TRUE;
		}
	}
	return FALSE;
}


/***************************

	�����Ի�����̺���

****************************/
LRESULT __stdcall DlgHelpProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 1);
		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			EndDialog(hWnd, 1);
			break;
		default:
			break;
		}
	default:
		break;
	}
	return 0;
}

/***************************

	���ڶԻ�����̺���

****************************/
LRESULT __stdcall DlgAboutProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 1);
		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			EndDialog(hWnd, 1);
			break;
		default:
			break;
		}
	default:
		break;
	}
	return 0;
}

/*************************

	�����ڹ��̺���

*************************/
LRESULT __stdcall WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
	case WM_CREATE:
	{

		return 0;
	}
	case WM_PAINT:
	{
		HDC hdc;
		PAINTSTRUCT ps;
		hdc = BeginPaint(hWnd, &ps);;

		DrawInitFrame(hdc);							//���ƴ��ڻ�������
		DrawMenuBotton(hdc);						//���Ʋ˵���ť
		DrawPaths(hdc);								//���ƾ����·��
		DrawSelectedNode(hdc);						//�������·��
		DrawNode(hdc);								//���ƾ�����
		SetOperateInfo(hWnd, CurOperation, 0, 0);

		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		HDC hdc;
		int mouse_x, mouse_y;
		hdc = GetDC(hWnd);
		mouse_x = LOWORD(lParam);		//���X����
		mouse_y = HIWORD(lParam);		//���Y����
		if (mouse_x > 50 && mouse_x < 780 && mouse_y > 62 && mouse_y < 548)			//��������
		{
			//��ȡϵͳ������ɫ����굱ǰλ����ɫ�������һ������Ϊ����
			if (RGB(255, 255, 255) != GetPixel(hdc, mouse_x, mouse_y))
			{
				SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));				//���������״Ϊ����
			}
		}
		if (mouse_x > 50 && mouse_x < 1240 && mouse_y > 560 && mouse_y < 590)		//�˵�����
		{
			//��ȡϵͳ������ɫ����굱ǰλ����ɫ�������һ������Ϊ����
			if (RGB(246, 246, 246) != GetPixel(hdc, mouse_x, mouse_y))
			{
				SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));				//���������״Ϊ����
			}
		}
		ReleaseDC(hWnd, hdc);
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		int mouse_x, mouse_y;
		mouse_x = LOWORD(lParam);
		mouse_y = HIWORD(lParam);
		SetBtnClickEvents(hWnd, mouse_x, mouse_y);
		if (mouse_x < 50 || mouse_x > 780 || mouse_y < 80 || mouse_y > 530) //ѡ�񾰵������ڻ�ͼ
		{
			return 0;
		}
		switch (CurOperation)
		{
		case 0:
		{
			break;
		}
		case 1:		//��Ӿ���
			if (NodeNum >= MAX_NODE)
			{
				MessageBox(hWnd, _T("����������"), _T("��ʾ"), MB_OK | MB_ICONEXCLAMATION);
				return 0;
			}
			if (!IsNodePosAvailable(mouse_x, mouse_y))
			{
				MessageBox(hWnd, _T("����֮����������������ѡ��"), _T("��ʾ"), MB_OK | MB_ICONEXCLAMATION);
				return 0;
			}
			Sights[NodeNum].NodeNum = NodeNum + 1;
			Sights[NodeNum].NodeXPos = mouse_x;
			Sights[NodeNum].NodeYPos = mouse_y;
			Sights[NodeNum].Checked = FALSE;
			NodeNum++;
			SendMessage(hWnd, UM_INVALIDATERECT, 0, 0);
			//DialogBox(NULL, MAKEINTRESOURCE(IDD_SetNodeInfo), hWnd, (DLGPROC)SetNodeInfoProc);
			break;
		case 2:		//����·��
		{
			static int Node1 = -1; 
			static int Node2 = -1;
			static int CheckedNum = 0;
			TwoPathNode PNode;
			if (0 == CheckedNum)
			{
				Node1 = GetCurClickedNode(hWnd, mouse_x, mouse_y);
				if (Node1 != -1)
				{
					CheckedNum = 1;
					Sights[Node1 - 1].Checked = TRUE;
				}
			}
			else if (1 == CheckedNum)
			{
				Node2 = GetCurClickedNode(hWnd, mouse_x, mouse_y);
				if (IsPathExist(Node1, Node2))		//�ж�������·���Ƿ�������
				{
					MessageBox(hWnd, _T("��·�������ã�"), _T("��ʾ"), MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				if (Node2 != -1)
				{
					if (Node1 == Node2)
					{
						CheckedNum = 0;
						Sights[Node1 - 1].Checked = FALSE;
					}
					else
					{
						CheckedNum = 2;
						Sights[Node2 - 1].Checked = TRUE;
						Paths[PathNum].StartNode = Node1;
						Paths[PathNum].EndNode = Node2;
						Paths[PathNum].PathLength = 0;
						PathNum++;		
						Sights[Node1 - 1].Checked = FALSE;
						Sights[Node2 - 1].Checked = FALSE;
						PNode.Node1 = Node1;
						PNode.Node2 = Node2;
						PNode.mouse_x = LOWORD(lParam);
						PNode.mouse_y = HIWORD(lParam);
						Node1 = -1;
						Node2 = -1;
						CheckedNum = 0;					
						SendMessage(hWnd, UM_INVALIDATERECT, 0, 0);
						DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_SetPathLength), hWnd, (DLGPROC)SetPathProc,(LPARAM)&PNode);
					}
				}
			}
			SendMessage(hWnd, UM_INVALIDATERECT, 0, 0);
			break;
		}
		case 3:					//���ѡ�񾰵㰴ť
		{			
			int CurNode = GetCurClickedNode(hWnd, mouse_x, mouse_y);
			if (0 == CheckedNum)
			{
				if (CurNode != -1)
				{
					CheckedNum = 1;
					Sights[CurNode - 1].Checked = TRUE;
					StartSight = CurNode;
				}
			}
			else if (1 == CheckedNum)
			{
				if (CurNode != -1)
				{
					if (StartSight == CurNode)
					{
						CheckedNum = 0;
						Sights[StartSight - 1].Checked = FALSE;
						StartSight = -1;
					}
					else
					{
						CheckedNum = 2;
						Sights[CurNode - 1].Checked = TRUE;
						EndSight = CurNode;
					}
				}
			}
			SendMessage(hWnd, UM_INVALIDATERECT, 0, 0);
			break;
		}
		default:
			break;
		}
		return 0;
	}
	case UM_INVALIDATERECT:		//�Զ�����Ϣ
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		InvalidateRect(hWnd, &rect, FALSE);
		return 0;
	}
	case WM_CLOSE:
	{
		if (IDYES == MessageBox(hWnd, _T("ȷ���ر�ϵͳ��"), _T("�ر���ʾ"), MB_YESNO | MB_ICONEXCLAMATION))
		{
			DestroyWindow(hWnd);
		}	
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_COMMAND:
	{
		switch (wParam)
		{
		case IDC_MENU_HELP:
			DialogBox(NULL, MAKEINTRESOURCE(IDD_DLG_HELP), hWnd, DlgHelpProc);
			break;
		case IDC_MENU_ABOUT:
			DialogBox(NULL, MAKEINTRESOURCE(IDD_DLG_ABOUT), hWnd, DlgHelpProc);
			break;
		case IDC_MENU_QUIT:
			if (IDYES == MessageBox(hWnd, _T("ȷ���ر�ϵͳ��"), _T("�ر���ʾ"), MB_YESNO | MB_ICONEXCLAMATION))
			{
				DestroyWindow(hWnd);
			}
			break;
		default:
			break;
		}
	}
	}

	return DefWindowProc(hWnd, nMsg, wParam, lParam);
}