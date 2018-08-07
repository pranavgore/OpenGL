#include <Windows.h>
#include <windowsx.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <stdio.h>
#include <math.h>
#include <string>
//#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "Common\MyMath3D.h"
#include "Structure.h"
#include "Hall.h"
#include "Door_Design.h"
#include "Interior.h"
#include "Stairs.h"
#include "jitter.h"

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLu32.lib")
#pragma comment(lib, "Winmm.lib")

#define WIN_WIDTH	800
#define WIN_HEIGHT	600

#define FOV_Y	45.0

#define FRUSTUM_LEFT	-1.0
#define FRUSTUM_RIGHT	1.0
#define FRUSTUM_BOTTOM	-1.0
#define FRUSTUM_TOP		1.0
#define FRUSTUM_NEAR	0.1
#define FRUSTUM_FAR		1000.0

#define FLOOR_EXTENT	500.0
#define STEP_SIZE		10.0
#define ZOOM_FACTOR		0.05

// Camera ID
#define ID_CAM_MOVEFORWARD		101
#define ID_CAM_YAW_LEFT			102
#define ID_CAM_MOVE_ON_STAIRS	103
#define ID_RESET_ATTENUATION	104
#define ID_CAM_MOVE_MAINDOOR	105
#define ID_CAM_MOVE_UP			106
#define ID_OPEN_DOORS			107
#define ID_MOVEIN_CLASSROOM		108
#define ID_MOVEIN_FINAL			109

#define TIMER_GLOBAL	201

#define TIMER_ID_1				1

/*Variables used*/
float angle_end_Ga1, angle_end_Ga2 = 0.25, angle_end_Ku1, angle_end_Ku2, angle_end_Ku3, angle_end_Ra1, angle_end_Ra2;
float angle_end_La1, angle_end_La2, angle_end_La3;
float vertex_end_Ga, vertex_end_Ka, vertex_end_Ra = -0.1f, vertex_end_La, vertex_end_HL;
int en_Ga1, en_Ga2, en_Ga3, en_Ku1, en_Ku2, en_Ku3, en_Ku4, en_Ra1, en_Ra2, en_Ra3, en_La1, en_La2, en_La3, en_La4;
int En_Ga1, En_Ga2, En_Ku1, En_Ku2, En_Ra1, En_Ra2, En_Ra3, En_La;
int en_HL, en_Gurukul;

using namespace std;

////////////////////////////////////////////////////
// Struct and class for model loading
struct vertex
{
	vector<float> v;
	void normalize()
	{
		float magnitude = 0.0f;
		for (int i = 0; i < v.size(); i++)
			magnitude += pow(v[i], 2.0f);

		magnitude = sqrt(magnitude);
		for (int i = 0; i < v.size(); i++)
			v[i] /= magnitude;
	}

	vertex operator-(vertex v2)
	{
		vertex v3;
		if (v.size() != v2.v.size())
		{
			v3.v.push_back(0.0f);
			v3.v.push_back(0.0f);
			v3.v.push_back(0.0f);
		}
		else
		{
			for (int i = 0; i < v.size(); i++)
				v3.v.push_back(v[i] - v2.v[i]);
		}
		return v3;
	}

	vertex cross(vertex v2)
	{
		vertex v3;
		if (v.size() != 3 || v2.v.size() != 3)
		{
			v3.v.push_back(0.0f);
			v3.v.push_back(0.0f);
			v3.v.push_back(0.0f);
		}
		else
		{
			v3.v.push_back(v[1] * v2.v[2] - v[2] * v2.v[1]);
			v3.v.push_back(v[2] * v2.v[0] - v[0] * v2.v[2]);
			v3.v.push_back(v[0] * v2.v[1] - v[1] * v2.v[0]);
		}
		return v3;
	}
};

struct face
{
	vector<int> vertex;
	vector<int> texture;
	vector<int> normal;
};

class OBJ
{
private:
	vector<vertex> vertices;
	vector<vertex> texcoords;
	vector<vertex> normals;
	vector<vertex> parameters;
	vector<face> faces;

	GLuint list;

public:
	//OBJ();
	~OBJ();

	void ReadOBJ(string filename);
	void DrawObject(void);
};
/////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//  For Bezier
struct Point
{
	GLfloat X;
	GLfloat Y;
	GLfloat Z;
};

struct ThreePointStruct
{
	struct Point P1;
	struct Point P2;
	struct Point P3;
};

struct ControlPoints
{
	struct ThreePointStruct TP1;
	struct ThreePointStruct TP2;
	struct ThreePointStruct TP3;
};
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// Struct for lighting
struct Light
{
	GLfloat R;
	GLfloat G;
	GLfloat B;
	GLfloat A;
};

struct Position
{
	GLfloat X;
	GLfloat Y;
	GLfloat Z;
	GLfloat W;	// Light type : Directional Light or Point Light
};

struct Scale
{
	GLfloat X;
	GLfloat Y;
	GLfloat Z;
};

struct Rotation
{
	GLfloat X;
	GLfloat Y;
	GLfloat Z;
};

struct Attenuation
{
	GLfloat Constant;
	GLfloat Linear;
	GLfloat Quadratic;
};

struct Color
{
	GLfloat R;
	GLfloat G;
	GLfloat B;
	GLfloat A;
};
//////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// for camera
struct M3DVector3d
{
	double X;
	double Y;
	double Z;
};

struct M3DVector2d
{
	double X;
	double Y;
};

M3DVector3d M3DVector(double, double, double);

class CCamera
{
public:

	// Related to camera
	M3DVector3d ViewDirection;
	M3DVector3d RightVector;
	M3DVector3d UpVector;
	M3DVector3d CameraPosition;
	M3DVector3d TargetPoint;

	double RotateX = 0.0;
	double RotateY = 0.0;
	double RotateZ = 0.0;

	void InitializeCamera(void);
	void SetCameraPosition(void);
	void Move(M3DVector3d Direction);
	void Pitch(double Angle);
	void Yaw(double Angle);
	void Roll(double Angle);

	void MoveForward(double Distance);
	void MoveUpward(double Distance);
	void StrafeRight(double Distance);
};
////////////////////////////////////////////////////////

// Display list ds

//-----Sujay Variables----//
GLuint dsGround;
GLuint dsSphere;
GLuint dsCube;
GLuint dsCubeRegular;
GLuint dsEntrance;
GLuint dsDoorDesign;
GLuint dsGateBaluster;
GLuint dsBalconyBaluster;
GLuint dsStairSectionCylinder;
GLuint dsClassDoor;
GLuint dsWhiteDesign;
GLuint dsStairs;
GLuint dsBalconyDesign;
GLuint dsNewelCap;
GLuint dsPillarBase;

GLuint dsRoofShingle;
GLuint dsRoof;
GLuint dsFrontBalconyRoof;
GLuint dsUpperFrontRoof;
GLuint dsUpperBackRoof;
GLuint dsLeftBuildingLeftRoof;
GLuint dsLeftBuildingRightRoof;
GLuint dsRightBuildingLeftRoof;
GLuint dsRightBuildingRightRoof;
GLuint dsRightBuildingFrontRoof;
GLuint dsPyramidalRoof;

GLuint dsBlackboard;
GLuint dsClassFloor;

// texture
GLuint biLeafTexture, biBranchTexture;

bool gbChairShow = false;
GLfloat gfChairRed = 0.0f;
GLfloat gfChairGreen = 0.0f;
GLfloat gfChairBlue = 0.0f;
GLfloat gfChairAlpha = 0.1f;

// rotation angles
GLfloat gfFanAngle = 0.0f;
GLfloat gfUpperFlapAngle = 0.0f;
GLfloat gfDoorAngle = 0.0f;
GLfloat gfWindowAngle = 0.0f;
GLfloat gfMainDoorAngle = 0.0f;

//-----Aashish Variables--//
GLfloat gScaleX = 0.0f;
GLfloat gScaleY = 0.0f;
GLfloat gScaleZ = 0.0f;
GLfloat gTranslateX = 0.0f;
GLfloat gTranslateY = 0.0f;
GLfloat gTranslateZ = 0.0f;
GLuint WhiteDesignTranslate = 0;

//----khare
GLfloat class_height = 40.0f;
GLfloat class_width = 40.0f;
GLfloat angleCube = 0.0f; //Angle of Pyramid

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HWND ghWnd = NULL;
HDC ghDC = NULL;
HGLRC ghGLRC = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
POINT ClickPoint, MovePoint, OldMovePoint;
bool gbFullSceneAtiAliasing = false;

//Gaurav 
//Gaurav's Global variables
//GLUquadric *quadric = NULL;
GLUquadric *quadric1 = NULL;
GLUquadric *quadric2 = NULL;
GLfloat i = -200.0f;
GLfloat angle = -90.0f;
GLfloat rAngle = 90.0f;
GLfloat fLeftTranslate = 30.0f;
GLfloat fRightTranslate = -30.0f;

void myCube();
void BrownCube();

// Timer related
void update(void);
int giCamMoveForward;
float giCamMoveForwardCount = 0;
bool gbInitialCameraMoveComplete = false;

int giResetAttenuation;
bool gbAttenuationResetComplete = false;
bool gbResetAttenuation = false;
bool gbSetAttenuation = false;

int giCamYawLeft;
int giCamYawLeftCount = 0;
bool gbCamYawLeftComplete = false;

int giCamMoveOnStairs;
int giCamStairClimbCount = 0;
bool gbCamStairsClimbed = false;

int giCamYawCorridor;
int giCamYawCorridorCount = 0;
bool gbCamYawCorridorComplete = false;

int giCamMoveMaindoor;
int giCamMoveMaindoorCount = 0;
bool giCamMoveMaindoorComplete = false;

int giCamMoveUp;
int giCamMoveUpCount = 0;
bool gbCamMoveUpComplete = false;

int giOpenDoors;
int giOpenDoorsCount = 0;
bool gbOpenDoorsComplete = false;

int giMoveInClass;
int giMoveInClassCount = 0;
bool gbMoveInClassComplete = false;

int giMoveInFinal;
int giMoveInFinalCount = 0;
bool gbMoveInFinalComplete = false;
bool gbAttenuationDecrease = false;

int giTimerGlobal;
int giTImerGlobalCount = 0;
bool giKillTimerGlobal = false;

// stipple
GLubyte stipple[128];

// Background clearing color
GLfloat gfNoLight[] = { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat gfLowLight[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat gfBrightLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };

// Lighting data
Position PositionLight0, PositionLight1, PositionLight2, CubeTranslate;
Rotation CubeRotate;
Scale CubeScale;
Light AmbientLight0, DiffuseLight0, SpecularLight0, AmbientLight1, DiffuseLight1, SpecularLight1, AmbientLight2, DiffuseLight2, SpecularLight2;
Color BackgroundColor;
Attenuation AttenuationLight0, AttenuationLight1, AttenuationLight2;
GLint giLightSelect = 1;
GLfloat gfHUDScreenAlpha = 0.0f;
bool gbFadeout = false;
bool gbFadeReset = false;

GLfloat MaterialSpecular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat MaterialShininess[] = { 100.0 };
GLfloat gfLightSphereAlpha = 1.0f;
bool gbLightOnCamera = false;

GLfloat gfLightModelLocalViewer[] = { 10.0f };
GLfloat gbAmbientLightModel[] = { 0.2f, 0.2f, 0.2f, 0.0f };
GLfloat gbNoAmbientLightModel[] = { 0.0f, 0.0f, 0.0f, 0.0f };

GLdouble gdZoomFactor = 1.0;
GLdouble gzDelta = 0;
GLint giWidth = 0;
GLint giHeight = 0;
GLdouble fovy = 60.0;

GLdouble iPositiveXThreshold = (giWidth / 2.0) + ((giWidth * 40.0) / 100.0);
GLdouble iPositiveYThreshold = (giHeight / 2.0) + ((giHeight * 40.0) / 100.0);

GLdouble iNegativeXThreshold = (giWidth / 2.0) - ((giWidth * 40.0) / 100.0);
GLdouble iNegativeYThreshold = (giHeight / 2.0) - ((giHeight * 40.0) / 100.0);

bool gbActiveWindow = false;
bool gbEscapeKeyPress = false;
bool gbFullScreen = false;
bool gbDone = false;
bool gbIsPointLight = false;
unsigned int guiPolygonMode = 0;
bool gbIsEditModeOn = false;

bool gbEnableLight1 = true;
bool gbEnableLight2 = true;

bool gbLight0FinalAttenuation = false;
bool gbLight1FinalAttenuation = false;

FILE *stream;

OBJ obj;
CCamera cam;
TCHAR WindowTitle[255];

GLUquadric *quadric = NULL;

GLint nNumPoints = 3;
GLfloat ctrlPoints[3][3][3] = { { { -6.0f, 0.0f, 6.0f },
								  { 0.0f, -4.0f, 6.0f },
								  { 6.0f, 0.0f, 6.0f } },

								{ {-6.0f, 0.0f, 0.0f },
								  { 0.0f, -4.0f, 0.0f },
								{ 6.0f, 0.0f, 0.0f } },

								{ { -6.0f, 0.0f, -6.0f },
								{ 0.0f, -4.0f, -6.0f },
								{ 6.0f, 0.0f, -6.0f } } };

GLfloat ctrlPoints1[3][3][3] = { { { -1.0f, 0.0f, 0.0f },
{ 0.0f, 1.0f, -1.0f },
{ 1.0f, 0.0f, 0.0f } },

{ { -1.0f, 1.0f, 0.0f },
{ 0.0f, 2.0f, -1.0f },
{ 1.0f, 1.0f, 0.0f } },

{ { -1.0f, 1.0f, 0.0f },
{ 0.0f, 2.0f, -1.0f },
{ 1.0f, 1.0f, 0.0f } } };

GLuint dsArc;		// Global variable

					// Bezier Curve Pranav
					// Curve for entrance arc
GLint giArcPoints = 3;
GLfloat gfControlPointsArc[3][3][3] = { { { -3.0f, 0.0f, 4.0f },
{ 0.0f, -4.0f, 4.0f },
{ 3.0f, 0.0f, 4.0f } },

{ { -3.0f, 0.0f, 0.0f },
{ 0.0f, -4.0f, 0.0f },
{ 3.0f, 0.0f, 0.0f } },

{ { -3.0f, 0.0f, -4.0f },
{ 0.0f, -4.0f, -4.0f },
{ 3.0f, 0.0f, -4.0f } } };

// Surface above arc
GLint giSurfacePoints = 3;
GLfloat gfControlPointsSurface[3][3][3] = { { { -3.0f, 0.0f, 0.0f },
{ 0.0f, 4.0f, 0.0f },
{ 3.0f, 0.0f, 0.0f } },

{ { -3.0f, 2.0f, 0.0f },
{ 0.0f, 2.0f, 0.0f },
{ 3.0f, 2.0f, 0.0f } },

{ { -3.0f, 2.5f, 0.0f },
{ 0.0f, 2.5f, 0.0f },
{ 3.0f, 2.5f, 0.0f } } };

// for roof shingle bezier curve
// Bezier Surface for roof
GLint RoofPoints = 3;
GLfloat RoofControlPoints[3][3][3] = { { { -4.0f, 0.0f, 4.0f },
{ 0.0f, 4.0f, 4.0f },
{ 4.0f, 0.0f, 4.0f } },

{ { -4.0f, 0.0f, 0.0f },
{ 0.0f, 4.0f, 0.0f },
{ 4.0f, 0.0f, 0.0f } },

{ { -4.0f, 0.0f, -4.0f },
{ 0.0f, 4.0f, -4.0f },
{ 4.0f, 0.0f, -4.0f } } };

GLfloat RoofControlPoints2[3][3][3] = { { { -4.0f, 0.0f, 0.0f },
{ 0.0f, 4.0f, 0.0f },
{ 4.0f, 0.0f, 0.0f } },

{ { -4.0f, 1.0f, 0.0f },
{ 0.0f, 5.0f, 0.0f },
{ 4.0f, 1.0f, 0.0f } },

{ { -4.0f, 2.0f, 0.0f },
{ 0.0f, 6.0f, 0.0f },
{ 4.0f, 2.0f, 0.0f } } };



////////////////////////////////////////////
// Shapes
void DrawPyramid(GLfloat Side);
void DrawColoredPyramid(GLfloat Side);
void DrawColoredCube(GLfloat Side);
void DrawCube(GLfloat Side);
void DrawSphere(GLdouble Radius, GLuint Slices, GLuint Stacks);
void DrawCone(GLdouble BaseRadius, GLdouble Height, GLuint Slices, GLuint Stacks);
void DrawCylinder(GLdouble BaseRadius, GLdouble TopRadius, GLdouble Height, GLuint Slices, GLuint Stacks);
void DrawTorus(GLfloat MajorRadius, GLfloat MinorRadius, GLuint NumMajor, GLuint NumMinor);
void DrawGrid(GLdouble FloorExtent, GLdouble StepSize);
void DrawGround(GLdouble FloorExtent, GLdouble StepSize);
void DrawOriginAxes(GLfloat Length, GLfloat LineWidth);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	void Initialize(void);
	void Display(void);
	void Uninitialize(void);
	void ToggleFullScreen(void);
	void update(void);

	WNDCLASSEX WndClass;
	HWND hWnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("RTRGroupDemo");

	WndClass.cbSize = sizeof(WNDCLASSEX);
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = szClassName;
	WndClass.lpszMenuName = NULL;
	//WndClass.lpszMenuName = MAKEINTRESOURCE(ID_GURUKUL);

	RegisterClassEx(&WndClass);

	unsigned int uiScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	unsigned int uiSceenHeight = GetSystemMetrics(SM_CYSCREEN);

	unsigned int uiXFinal = (uiScreenWidth / 2) - (WIN_WIDTH / 2);
	unsigned int uiYFinal = (uiSceenHeight / 2) - (WIN_HEIGHT / 2);

	hWnd = CreateWindowEx(WS_EX_APPWINDOW, szClassName, TEXT("Gurukul"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, uiXFinal, uiYFinal, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);

	ghWnd = hWnd;

	Initialize();

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	if (gbFullScreen == false)
	{
		ToggleFullScreen();
		gbFullScreen = true;
	}
	// Game Loop
	while (gbDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				gbDone = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbIsEditModeOn == false)
				update();
			Display();
			if (gbActiveWindow == true)
			{
				if (gbEscapeKeyPress == true)
					gbDone = true;
			}
		}
	}

	Uninitialize();
	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void Resize(int, int);
	void Zoom(short);
	void ToggleFullScreen(void);
	void Uninitialize(void);
	void InitializeObjects(void);

	void OnLButtonDown(int, int, DWORD);
	void OnLButtonUp(int, int, DWORD);
	void OnRButtonDown(int, int, DWORD);
	void OnRButtonUp(int, int, DWORD);
	void OnMouseMove(int, int, DWORD);
	void OnMButtonDown(int, int, DWORD);
	void OnMButtonUp(int, int, DWORD);

	void OnNumpad1(void);
	void OnNumpad2(void);
	void OnNumpad3(void);
	void OnNumpad4(void);
	void OnNumpad5(void);
	void OnNumpad6(void);
	void OnNumpad7(void);
	void OnNumpad8(void);

	void OnZeroKeyPress(void);
	void OnJKeyPress(void);
	void OnShiftJKeyPress(void);
	void OnKKeyPress(void);
	void OnShiftKKeyPress(void);
	void OnLKeyPress(void);
	void OnShiftLKeyPress(void);
	void OnShiftRKeyPress(void);
	void OnPKeyPress(void);

	int iShiftState = 0;

	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;
		break;

		//case WM_ERASEBKGND:
		//return(0);
		//break;

	/*case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_WIREFRAME:
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			guiPolygonMode = 2;
			break;

		case IDM_SOLID:
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			guiPolygonMode = 1;
			break;

		case IDM_POINT:
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			guiPolygonMode = 0;
			break;

		case IDM_EDITMODE:
			break;

		case IDM_DEMOMODE:
			break;

		default:
			break;
		}
		break;*/

	case WM_SIZE:
		giWidth = LOWORD(lParam);
		giHeight = HIWORD(lParam);
		Resize(giWidth, giHeight);
		break;

	case WM_LBUTTONDOWN:
		if (gbIsEditModeOn == true)
			OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		break;

	case WM_LBUTTONUP:
		if (gbIsEditModeOn == true)
			OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		break;

	case WM_RBUTTONDOWN:
		if (gbIsEditModeOn == true)
			OnRButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		break;

	case WM_RBUTTONUP:
		if (gbIsEditModeOn == true)
			OnRButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		break;

	case WM_MOUSEWHEEL:
		if (gbIsEditModeOn == true)
			Zoom(GET_WHEEL_DELTA_WPARAM(wParam));
		break;

	case WM_MOUSEMOVE:
		if (gbIsEditModeOn == true)
			OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		break;

	case WM_MBUTTONDOWN:
		if (gbIsEditModeOn == true)
			OnMButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		break;

	case WM_MBUTTONUP:
		if (gbIsEditModeOn == true)
			OnMButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		break;

	case WM_CHAR:
		switch (wParam)
		{

		case '@':
			if (gbIsEditModeOn == true)
			{

			}
			break;

		case '#':
			if (gbIsEditModeOn == true)
			{

			}
			break;

		case 'p':
			OnPKeyPress();
			break;

		case 'e':
			if (gbIsEditModeOn == true)
				cam.Roll(-1.0);
			break;

		case 'q':
			if (gbIsEditModeOn == true)
				cam.Roll(1.0);
			break;

		case 'r':
			gdZoomFactor = 1.0;
			fovy = 45.0;
			Resize(giWidth, giHeight);
			break;

		case 'w':
			if (gbIsEditModeOn == true)
			{
				//cam.Pitch(1.0);
				cam.MoveForward(-1.5);
			}
			break;

		case 's':
			if (gbIsEditModeOn == true)
			{
				//cam.Pitch(-1.0);
				cam.MoveForward(1.5);
			}
			break;

		case 'a':
			if (gbIsEditModeOn == true)
			{
				//cam.Yaw(1.0);
				cam.StrafeRight(-1.5);
			}
			break;

		case 'd':
			if (gbIsEditModeOn == true)
			{
				//cam.Yaw(-1.0);
				cam.StrafeRight(1.5);
			}
			break;

			
		case 'j':
			if (gbIsEditModeOn == true)
				OnJKeyPress();
			break;

		case 'J':
			if (gbIsEditModeOn == true)
				OnShiftJKeyPress();
			break;

		case 'k':
			if (gbIsEditModeOn == true)
				OnKKeyPress();
			break;

		case 'K':
			if (gbIsEditModeOn == true)
				OnShiftKKeyPress();
			break;

		case 'l':
			if (gbIsEditModeOn == true)
				OnLKeyPress();
			break;

		case 'L':
			if (gbIsEditModeOn == true)
				OnShiftLKeyPress();
			break;

		case '0':
			if (gbIsEditModeOn == true)
				OnZeroKeyPress();
			break;
			
		case 'R':
			if (gbIsEditModeOn == true)
				OnShiftRKeyPress();
			break;

			/*
		case 'A':
			AmbientLight0.R = 0.0f;
			AmbientLight0.G = 0.0f;
			AmbientLight0.B = 0.0f;
			AmbientLight0.A = 1.0f;
			break;

		case 'D':
			DiffuseLight0.R = 1.0f;
			DiffuseLight0.G = 1.0f;
			DiffuseLight0.B = 1.0f;
			DiffuseLight0.A = 1.0f;
			break;

		case 'S':
			SpecularLight0.R = 1.0f;
			SpecularLight0.G = 1.0f;
			SpecularLight0.B = 1.0f;
			SpecularLight0.A = 1.0f;
			break;
			*/
		case 'O':
			gfWindowAngle = gfWindowAngle + 0.5f;

			if (gfWindowAngle == 80.0f)
				gfWindowAngle = 0.0f;
			break;
		case 'o':
			gfWindowAngle = gfWindowAngle - 0.5f;

			if (gfWindowAngle == -80.0f)
				gfWindowAngle = 0.0f;
			break;

			// for Anti aliasing
		case 'A':
			if (gbFullSceneAtiAliasing == false)
				gbFullSceneAtiAliasing = true;
			else
				gbFullSceneAtiAliasing = false;
			break;

		case 'c':
		case 'C':
			if (gbChairShow == false)
			{
				gbChairShow = true;
			}
			else
			{
				gbChairShow = false;
			}
			break;


		// cube X Y Z translation
		case 'X':
			if (gbIsEditModeOn == true)
				CubeTranslate.X += 0.5;
			break;

		case 'x':
			if (gbIsEditModeOn == true)
				CubeTranslate.X -= 0.5;
			break;

		case 'Y':
			if (gbIsEditModeOn == true)
				CubeTranslate.Y += 0.5;
			break;

		case 'y':
			if (gbIsEditModeOn == true)
				CubeTranslate.Y -= 0.5;
			break;

		case 'Z':
			if (gbIsEditModeOn == true)
				CubeTranslate.Z += 0.5f;
			break;

		case 'z':
			if (gbIsEditModeOn == true)
				CubeTranslate.Z -= 0.5f;
			break;

		// Cube scaling B -> X scaling N -> Y scaling M -> z scaling
		case 'B':
			if (gbIsEditModeOn == true)
				CubeScale.X += 0.1f;
			break;

		case 'b':
			if (gbIsEditModeOn == true)
			{
				CubeScale.X -= 0.1f;
				if (CubeScale.X <= 0.0f)
					CubeScale.X = 0.1f;
			}
			break;

		case 'N':
			if (gbIsEditModeOn == true)
				CubeScale.Y += 0.1f;
			break;

		case 'n':
			if (gbIsEditModeOn == true)
			{
				CubeScale.Y -= 0.1f;
				if (CubeScale.Y <= 0.0f)
					CubeScale.Y = 0.1f;
			}
			break;

		case 'M':
			if (gbIsEditModeOn == true)
				CubeScale.Z += 0.1f;
			break;

		case 'm':
			if (gbIsEditModeOn == true)
			{
				CubeScale.Z -= 0.1f;
				if (CubeScale.Z <= 0.0f)
					CubeScale.Z = 0.1f;
			}
			break;

		// Cube rotation F -> X rotation G -> Y rotation H -> Z rotation
		case 'F':
			if (gbIsEditModeOn == true)
			{
				CubeRotate.X += 0.1f;
				if (CubeRotate.X >= 360.0f)
					CubeRotate.X = 0.0f;
			}
			break;

		case 'f':
			if (gbIsEditModeOn == true)
			{
				CubeRotate.X -= 0.1f;
				if (CubeRotate.X <= 0.0f)
					CubeRotate.X = 360.0f;
			}
			break;

		case 'G':
			if (gbIsEditModeOn == true)
			{
				CubeRotate.Y += 0.1f;
				if (CubeRotate.Y >= 360.0f)
					CubeRotate.Y = 0.0f;
			}
			break;

		case 'g':
			if (gbIsEditModeOn == true)
			{
				CubeRotate.Y -= 0.1f;
				if (CubeRotate.Y <= 0.0f)
					CubeRotate.Y = 360.0f;
			}
			break;

		case 'H':
			if (gbIsEditModeOn == true)
			{
				CubeRotate.Z += 0.1f;
				if (CubeRotate.Z >= 360.0f)
					CubeRotate.Z = 0.0f;
			}
			break;

		case 'h':
			if (gbIsEditModeOn == true)
			{
				CubeRotate.Z -= 0.1f;
				if (CubeRotate.Z <= 0.0f)
					CubeRotate.Z = 360.0f;
			}
			break;

		case 'I':
		case 'i':
			en_Gurukul = 1;
			En_Ga1 = 1;
			en_Ga1 = 1;

			SetTimer(hWnd, TIMER_ID_1, 1, (TIMERPROC)NULL);
			break;

		case 'E':
			if (gbIsEditModeOn == false)
			{
				gbIsEditModeOn = true;
				InitializeObjects();

				// Kill all timers
				KillTimer(ghWnd, TIMER_ID_1);
				KillTimer(ghWnd, TIMER_GLOBAL);


				/*KillTimer(ghWnd, ID_CAM_MOVEFORWARD);
				KillTimer(ghWnd, ID_CAM_YAW_LEFT);
				KillTimer(ghWnd, ID_CAM_MOVE_ON_STAIRS);
				KillTimer(ghWnd, ID_RESET_ATTENUATION);
				KillTimer(ghWnd, ID_CAM_MOVE_MAINDOOR);
				KillTimer(ghWnd, ID_CAM_MOVE_UP);
				KillTimer(ghWnd, ID_OPEN_DOORS);
				KillTimer(ghWnd, ID_MOVEIN_CLASSROOM);
				KillTimer(ghWnd, ID_MOVEIN_FINAL);*/


				giTImerGlobalCount = 0;
				giKillTimerGlobal = false;

				//// reset all counters
				//giCamMoveForwardCount = 0;
				//gbInitialCameraMoveComplete = false;

				//gbAttenuationResetComplete = false;
				//gbResetAttenuation = false;

				//giCamYawLeftCount = 0;
				//gbCamYawLeftComplete = false;

				//giCamStairClimbCount = 0;
				//gbCamStairsClimbed = false;
				//
				//giCamYawCorridorCount = 0;
				//gbCamYawCorridorComplete = false;
				//
				//giCamMoveMaindoorCount = 0;
				//giCamMoveMaindoorComplete = false;
				//
				//giCamMoveUpCount = 0;
				//gbCamMoveUpComplete = false;
				//
				//giOpenDoorsCount = 0;
				//gbOpenDoorsComplete = false;
				//
				//giMoveInClassCount = 0;
				//gbMoveInClassComplete = false;
				//
				//giMoveInFinalCount = 0;
				//gbMoveInFinalComplete = false;
				//gbAttenuationDecrease = false;

				//giTImerGlobalCount = 0;
				//giKillTimerGlobal = false;

				gfLightSphereAlpha = 1.0f;
				gbLightOnCamera = false;
				gfHUDScreenAlpha = 0.0f;

				// reset all rotation angles
				gfFanAngle = 0.0f;
				gfUpperFlapAngle = 0.0f;
				gfDoorAngle = 0.0f;
				gfWindowAngle = 0.0f;
				gfMainDoorAngle = 0.0f;

				PlaySound(NULL, NULL, NULL);
			}
			else
			{
				gbIsEditModeOn = false;
				InitializeObjects();
				ShowCursor(false);
			}
			break;

		default:
			break;
		}
		break;

	case WM_TIMER:
		switch (wParam)
		{

		case TIMER_GLOBAL:
			giTImerGlobalCount++;
			break;

		case TIMER_ID_1:
			Draw_Gurukul();
			if (!en_Gurukul)
				KillTimer(hWnd, TIMER_ID_1);
			break;

		case ID_CAM_MOVEFORWARD:
			giCamMoveForwardCount++;
			cam.MoveForward(-0.8);
			PositionLight0.Z -= 0.9f;
			PositionLight0.X += 0.05f;

			if (giCamMoveForwardCount >= 180)
			{
				//cam.Yaw(0.08);
			}
			break;

		case ID_RESET_ATTENUATION:
			gbResetAttenuation = true;
			break;

		case ID_CAM_YAW_LEFT:
			giCamYawLeftCount++;
			cam.Yaw(0.2);
			cam.Pitch(0.08);
			break;

		case ID_CAM_MOVE_ON_STAIRS:
			giCamStairClimbCount++;

			if (giCamStairClimbCount <= 880)
			{
				cam.MoveForward(-0.1);
				cam.MoveUpward(0.03);
			}
			else
			{
				cam.MoveForward(-0.1);
				cam.Pitch(-0.05);
			}
			break;

		case ID_CAM_MOVE_MAINDOOR:
			giCamMoveMaindoorCount++;
			cam.MoveForward(-1.0);

			PositionLight0.Z -= 0.9f;
			PositionLight0.Y += 0.05f;
			PositionLight0.X -= 0.05f;

			if (giCamMoveMaindoorCount >= 150.0f)
			{
				gfMainDoorAngle = gfMainDoorAngle + 1.0f;
				if (gfMainDoorAngle >= 90.0f)
					gfMainDoorAngle = 90.0f;
			}

			if (giCamMoveMaindoorCount > 330 && giCamMoveMaindoorCount < 350)
			{
				cam.Yaw(1.0);
			}

			if (giCamMoveMaindoorCount > 430 && giCamMoveMaindoorCount < 470)
			{
				cam.Yaw(-1.2);
			}
			break;

		case ID_CAM_MOVE_UP:
			giCamMoveUpCount++;

			cam.MoveUpward(0.35);
			cam.Yaw(-1.1);
			break;

		case ID_OPEN_DOORS:
			giOpenDoorsCount++;
			gfWindowAngle = gfWindowAngle + 1.0f;
			if (gfWindowAngle >= 120.0f)
				gfWindowAngle = 120.0f;

			gfDoorAngle = gfDoorAngle + 1.0f;
			if (gfDoorAngle >= 100.0f)
				gfDoorAngle = 100.0f;

			gfUpperFlapAngle = gfUpperFlapAngle + 1.0f;
			if (gfUpperFlapAngle >= 70.0f)
				gfUpperFlapAngle = 70.0f;
			break;

		case ID_MOVEIN_CLASSROOM:
			giMoveInClassCount++;
			cam.MoveForward(-0.5);

			if (giMoveInClassCount <= 50)
				cam.Yaw(-0.140);
			break;

		case ID_MOVEIN_FINAL:
			giMoveInFinalCount++;
			cam.MoveForward(-0.2);
			cam.Yaw(0.07);

			if(giMoveInFinalCount >= 150)
				cam.Pitch(-0.07);

			if (giMoveInFinalCount >= 250)
			{
				cam.Pitch(0.07);
				cam.Yaw(-0.5);
			}
			
			break;

		default:
			break;
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			if (gbEscapeKeyPress == false)
				gbEscapeKeyPress = true;
			break;

			//Enter key fo fullscreen mode
		case VK_RETURN:
			if (gbFullScreen == false)
			{
				ToggleFullScreen();
				gbFullScreen = true;
			}
			else
			{
				ToggleFullScreen();
				gbFullScreen = false;
			}
			break;

		case VK_SPACE:
			if (gbIsEditModeOn == false)
			{
				/*giCamMoveForward = (int)SetTimer(hWnd, ID_CAM_MOVEFORWARD, 10, (TIMERPROC)NULL);
				if (giCamMoveForward == 0)
				{
					MessageBox(hWnd, TEXT("No timer"), TEXT("Error"), MB_OK);
				}*/

				giTimerGlobal = (int)SetTimer(hWnd, TIMER_GLOBAL, 10, (TIMERPROC)NULL);
			}
			break;

		case VK_CONTROL:
			break;

		case VK_ADD:
			if (gbIsEditModeOn == true)
			{
				fovy += 1.0;
				if (fovy >= 179.0)
					fovy = 179.0;
				Resize(giWidth, giHeight);
			}
			break;

		case VK_SUBTRACT:
			if (gbIsEditModeOn == true)
			{
				fovy -= 1.0;
				if (fovy <= 1.0)
					fovy = 1.0;
				Resize(giWidth, giHeight);
			}
			break;

		case VK_UP:
			if (gbIsEditModeOn == true)
			{
				//cam.MoveForward(-1.5);
				cam.Pitch(1.5);
			}
			break;

		case VK_DOWN:
			if (gbIsEditModeOn == true)
			{
				//cam.MoveForward(1.5);
				cam.Pitch(-1.5);
			}
			break;

		case VK_LEFT:
			if (gbIsEditModeOn == true)
			{
				//cam.StrafeRight(-1.5);
				cam.Yaw(1.5);
			}
			break;

		case VK_RIGHT:
			if (gbIsEditModeOn == true)
			{
				//cam.StrafeRight(1.5);
				cam.Yaw(-1.5);
			}
			break;

			// PAGE UP key
		case VK_PRIOR:
			if (gbIsEditModeOn == true)
				cam.MoveUpward(0.5);
			break;

			// PAGE DOWN KEY
		case VK_NEXT:
			if (gbIsEditModeOn == true)
				cam.MoveUpward(-0.5);
			break;

		case VK_BACK:
			fprintf_s(stream, "PMG current cube's translation X : %f Y : %f Z : %f \nscale X : %f Y : %f Z : %f\nrotation X : %f Y : %f Z : %f\nGoodbye..\n", CubeTranslate.X, CubeTranslate.Y, CubeTranslate.Z, CubeScale.X, CubeScale.Y, CubeScale.Z, CubeRotate.X, CubeRotate.Y, CubeRotate.Z);
			fprintf_s(stream, "Lighting data for LIGHT0\n\nPosition X : %f\nPosition Y : %f\nPosition Z : %f\nAmbient R : %f\nAmbient G : %f\nAmbient B : %f\n\nDiffuse R : %f\nDiffuse G : %f\nDiffuse B : %f\n\nSpecular R : %f\nSpecular G : %f\nSpecular B : %f\nConstant Attenuation : %f\nLinear Attenuation : %f\nQuadratic Attenuation : %f\n", PositionLight0.X, PositionLight0.Y, PositionLight0.Z, AmbientLight0.R, AmbientLight0.G, AmbientLight0.B, DiffuseLight0.R, DiffuseLight0.G, DiffuseLight0.B, SpecularLight0.R, SpecularLight0.G, SpecularLight0.B, AttenuationLight0.Constant, AttenuationLight0.Linear, AttenuationLight0.Quadratic);
			fprintf_s(stream, "Lighting data for LIGHT1\n\nPosition X : %f\nPosition Y : %f\nPosition Z : %f\nAmbient R : %f\nAmbient G : %f\nAmbient B : %f\n\nDiffuse R : %f\nDiffuse G : %f\nDiffuse B : %f\n\nSpecular R : %f\nSpecular G : %f\nSpecular B : %f\nConstant Attenuation : %f\nLinear Attenuation : %f\nQuadratic Attenuation : %f\n", PositionLight1.X, PositionLight1.Y, PositionLight1.Z, AmbientLight1.R, AmbientLight1.G, AmbientLight1.B, DiffuseLight1.R, DiffuseLight1.G, DiffuseLight1.B, SpecularLight1.R, SpecularLight1.G, SpecularLight1.B, AttenuationLight1.Constant, AttenuationLight1.Linear, AttenuationLight1.Quadratic);
			fprintf_s(stream, "Lighting data for LIGHT2\n\nPosition X : %f\nPosition Y : %f\nPosition Z : %f\nAmbient R : %f\nAmbient G : %f\nAmbient B : %f\n\nDiffuse R : %f\nDiffuse G : %f\nDiffuse B : %f\n\nSpecular R : %f\nSpecular G : %f\nSpecular B : %f\nConstant Attenuation : %f\nLinear Attenuation : %f\nQuadratic Attenuation : %f\n", PositionLight2.X, PositionLight2.Y, PositionLight2.Z, AmbientLight2.R, AmbientLight2.G, AmbientLight2.B, DiffuseLight2.R, DiffuseLight2.G, DiffuseLight2.B, SpecularLight2.R, SpecularLight2.G, SpecularLight2.B, AttenuationLight2.Constant, AttenuationLight2.Linear, AttenuationLight2.Quadratic);
			fprintf_s(stream, "Camera X : %f Camera Y : %f Camera Z : %f\n", cam.CameraPosition.X, cam.CameraPosition.Y, cam.CameraPosition.Z);
			break;

		case VK_NUMPAD1:
			if (gbIsEditModeOn == true)
				OnNumpad1();
			break;

		case VK_NUMPAD2:
			if (gbIsEditModeOn == true)
				OnNumpad2();
			break;

		case VK_NUMPAD3:
			if (gbIsEditModeOn == true)
				OnNumpad3();
			break;

		case VK_NUMPAD4:
			if (gbIsEditModeOn == true)
				OnNumpad4();
			break;

		case VK_NUMPAD5:
			if (gbIsEditModeOn == true)
				OnNumpad5();
			break;

		case VK_NUMPAD6:
			if (gbIsEditModeOn == true)
				OnNumpad6();
			break;

		case VK_NUMPAD7:
			if (gbIsEditModeOn == true)
				OnNumpad7();
			break;

		case VK_NUMPAD8:
			if (gbIsEditModeOn == true)
				OnNumpad8();
			break;

		case VK_NUMPAD9:
			// code
			break;

			// 1
		case 0x31:
			if (gbIsEditModeOn == true)
			{
				giLightSelect = 1;
			}
			break;
			
			// 2
		case 0x32:
			if (gbIsEditModeOn == true)
			{
				//iShiftState = GetKeyState(VK_SHIFT);
				//if (iShiftState < 0)
				//{
				//	AmbientLight0.R -= 0.1f;	// key is pressed
				//	if (AmbientLight0.R <= 0.0f)
				//		AmbientLight0.R = 0.0f;
				//}
				//else
				//{
				//	AmbientLight0.R += 0.1f;
				//	if (AmbientLight0.R >= 1.0f)
				//		AmbientLight0.R = 1.0f;
				//}
				giLightSelect = 2;
			}
			break;

		// 3
		case 0x33:
			if (gbIsEditModeOn == true)
			{
				//iShiftState = GetKeyState(VK_SHIFT);
				//if (iShiftState < 0)
				//{
				//	AmbientLight0.G -= 0.1f;	// key is pressed
				//	if (AmbientLight0.G <= 0.0f)
				//		AmbientLight0.G = 0.0f;
				//}
				//else
				//{
				//	AmbientLight0.G += 0.1f;
				//	if (AmbientLight0.G >= 1.0f)
				//		AmbientLight0.G = 1.0f;
				//}
				giLightSelect = 3;
			}
			break;

		//	// 3
		//case 0x33:
		//	iShiftState = GetKeyState(VK_SHIFT);
		//	if (iShiftState < 0)
		//	{
		//		AmbientLight0.B -= 0.1f;	// key is pressed
		//		if (AmbientLight0.B <= 0.0f)
		//			AmbientLight0.B = 0.0f;
		//	}
		//	else
		//	{
		//		AmbientLight0.B += 0.1f;
		//		if (AmbientLight0.B >= 1.0f)
		//			AmbientLight0.B = 1.0f;
		//	}
		//	break;

		//	// 4
		//case 0x34:
		//	iShiftState = GetKeyState(VK_SHIFT);
		//	if (iShiftState < 0)
		//	{
		//		DiffuseLight0.R -= 0.1f;	// key is pressed
		//		if (DiffuseLight0.R <= 0.0f)
		//			DiffuseLight0.R = 0.0f;
		//	}
		//	else
		//	{
		//		DiffuseLight0.R += 0.1f;
		//		if (DiffuseLight0.R >= 1.0f)
		//			DiffuseLight0.R = 1.0f;
		//	}
		//	break;

		//	// 5
		//case 0x35:
		//	iShiftState = GetKeyState(VK_SHIFT);
		//	if (iShiftState < 0)
		//	{
		//		DiffuseLight0.G -= 0.1f;	// key is pressed
		//		if (DiffuseLight0.G <= 0.0f)
		//			DiffuseLight0.G = 0.0f;
		//	}
		//	else
		//	{
		//		DiffuseLight0.G += 0.1f;
		//		if (DiffuseLight0.G >= 1.0f)
		//			DiffuseLight0.G = 1.0f;
		//	}
		//	break;

		//	// 6
		//case 0x36:
		//	iShiftState = GetKeyState(VK_SHIFT);
		//	if (iShiftState < 0)
		//	{
		//		DiffuseLight0.B -= 0.1f;	// key is pressed
		//		if (DiffuseLight0.B <= 0.0f)
		//			DiffuseLight0.B = 0.0f;
		//	}
		//	else
		//	{
		//		DiffuseLight0.B += 0.1f;
		//		if (DiffuseLight0.B >= 1.0f)
		//			DiffuseLight0.B = 1.0f;
		//	}
		//	break;

		//	// 7
		//case 0x37:
		//	iShiftState = GetKeyState(VK_SHIFT);
		//	if (iShiftState < 0)
		//	{
		//		SpecularLight0.R -= 0.1f;	// key is pressed
		//		if (SpecularLight0.R <= 0.0f)
		//			SpecularLight0.R = 0.0f;
		//	}
		//	else
		//	{
		//		SpecularLight0.R += 0.1f;
		//		if (SpecularLight0.R >= 1.0f)
		//			SpecularLight0.R = 1.0f;
		//	}
		//	break;

		//	// 8
		//case 0x38:
		//	iShiftState = GetKeyState(VK_SHIFT);
		//	if (iShiftState < 0)
		//	{
		//		SpecularLight0.G -= 0.1f;	// key is pressed
		//		if (SpecularLight0.G <= 0.0f)
		//			SpecularLight0.G = 0.0f;
		//	}
		//	else
		//	{
		//		SpecularLight0.G += 0.1f;
		//		if (SpecularLight0.G >= 1.0f)
		//			SpecularLight0.G = 1.0f;
		//	}
		//	break;

		//	// 9
		//case 0x39:
		//	iShiftState = GetKeyState(VK_SHIFT);
		//	if (iShiftState < 0)
		//	{
		//		SpecularLight0.B -= 0.1f;	// key is pressed
		//		if (SpecularLight0.B <= 0.0f)
		//			SpecularLight0.B = 0.0f;
		//	}
		//	else
		//	{
		//		SpecularLight0.B += 0.1f;
		//		if (SpecularLight0.B >= 1.0f)
		//			SpecularLight0.B = 1.0f;
		//	}
		//	break;
		default:
			break;
		}
		break;

	case WM_CLOSE:
		Uninitialize();
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return(DefWindowProc(hWnd, iMsg, wParam, lParam));
}

void OnNumpad1(void)
{
	if (giLightSelect == 1)
		PositionLight0.Z += 1.0f;
	if (giLightSelect == 2)
		PositionLight1.Z += 1.0f;
	if (giLightSelect == 3)
		PositionLight2.Z += 1.0f;
}

void OnNumpad2(void)
{
	if (giLightSelect == 1)
		PositionLight0.Y -= 1.0f;
	if (giLightSelect == 2)
		PositionLight1.Y -= 1.0f;
	if (giLightSelect == 3)
		PositionLight2.Y -= 1.0f;
}

void OnNumpad3(void)
{
	// code
}

void OnNumpad4(void)
{
	if (giLightSelect == 1)
		PositionLight0.X -= 1.0f;
	if (giLightSelect == 2)
		PositionLight1.X -= 1.0f;
	if (giLightSelect == 3)
		PositionLight2.X -= 1.0f;
}

void OnNumpad5(void)
{
	// code
}

void OnNumpad6(void)
{
	if (giLightSelect == 1)
		PositionLight0.X += 1.0f;
	if (giLightSelect == 2)
		PositionLight1.X += 1.0f;
	if (giLightSelect == 3)
		PositionLight2.X += 1.0f;
}

void OnNumpad7(void)
{
	if (giLightSelect == 1)
		PositionLight0.Z -= 1.0f;
	if (giLightSelect == 2)
		PositionLight1.Z -= 1.0f;
	if (giLightSelect == 3)
		PositionLight2.Z -= 1.0f;
}

void OnNumpad8(void)
{
	if (giLightSelect == 1)
		PositionLight0.Y += 1.0f;
	if (giLightSelect == 2)
		PositionLight1.Y += 1.0f;
	if (giLightSelect == 3)
		PositionLight2.Y += 1.0f;
}

void OnZeroKeyPress(void)
{
	if (giLightSelect == 1)
	{
		if (gbIsPointLight == false)
		{
			PositionLight0.W = 1.0f;
			gbIsPointLight = true;
		}
		else
		{
			PositionLight0.W = 0.0f;
			gbIsPointLight = false;
		}
	}
	else if (giLightSelect == 2)
	{
		if (gbIsPointLight == false)
		{
			PositionLight1.W = 1.0f;
			gbIsPointLight = true;
		}
		else
		{
			PositionLight1.W = 0.0f;
			gbIsPointLight = false;
		}
	}
	else if (giLightSelect == 3)
	{
		if (gbIsPointLight == false)
		{
			PositionLight2.W = 1.0f;
			gbIsPointLight = true;
		}
		else
		{
			PositionLight2.W = 0.0f;
			gbIsPointLight = false;
		}
	}
}

void OnPKeyPress(void)
{
	switch (guiPolygonMode)
	{
	case 0:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		guiPolygonMode = 1;
		break;

	case 1:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		guiPolygonMode = 2;
		break;

	case 2:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		guiPolygonMode = 0;
		break;

	default:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		guiPolygonMode = 1;
		break;
	}
}

void OnJKeyPress(void)
{
	if (giLightSelect == 1)
		AttenuationLight0.Constant -= 0.1f;
	if (giLightSelect == 2)
		AttenuationLight1.Constant -= 0.1f;
	if (giLightSelect == 3)
		AttenuationLight2.Constant -= 0.1f;
}

void OnShiftJKeyPress(void)
{
	if (giLightSelect == 1)
		AttenuationLight0.Constant += 0.1f;
	if (giLightSelect == 2)
		AttenuationLight1.Constant += 0.1f;
	if (giLightSelect == 3)
		AttenuationLight2.Constant += 0.1f;
}

void OnKKeyPress(void)
{
	if (giLightSelect == 1)
		AttenuationLight0.Quadratic -= 0.01f;
	if (giLightSelect == 2)
		AttenuationLight1.Quadratic -= 0.01f;
	if (giLightSelect == 3)
		AttenuationLight2.Quadratic -= 0.01f;
}

void OnShiftKKeyPress(void)
{
	if (giLightSelect == 1)
		AttenuationLight0.Quadratic += 0.01f;
	if (giLightSelect == 2)
		AttenuationLight1.Quadratic += 0.01f;
	if (giLightSelect == 3)
		AttenuationLight2.Quadratic += 0.01f;
}

void OnLKeyPress(void)
{
	if (giLightSelect == 1)
		AttenuationLight0.Linear -= 0.01f;
	if (giLightSelect == 2)
		AttenuationLight1.Linear -= 0.01f;
	if (giLightSelect == 3)
		AttenuationLight2.Linear -= 0.01f;
}

void OnShiftLKeyPress(void)
{
	if (giLightSelect == 1)
		AttenuationLight0.Linear += 0.01f;
	if (giLightSelect == 2)
		AttenuationLight1.Linear += 0.01f;
	if (giLightSelect == 3)
		AttenuationLight2.Linear += 0.01f;
}

void OnShiftRKeyPress(void)
{
	if (giLightSelect == 1)
	{
		AttenuationLight0.Constant = 1.0f;
		AttenuationLight0.Linear = 0.0f;
		AttenuationLight0.Quadratic = 0.0f;
	}
	if (giLightSelect == 2)
	{
		AttenuationLight1.Constant = 1.0f;
		AttenuationLight1.Linear = 0.0f;
		AttenuationLight1.Quadratic = 0.0f;
	}
	if (giLightSelect == 3)
	{
		AttenuationLight2.Constant = 1.0f;
		AttenuationLight2.Linear = 0.0f;
		AttenuationLight2.Quadratic = 0.0f;
	}
}

void Initialize(void)
{
	void Resize(int, int);
	int LoadGLTextures(GLuint *, TCHAR[]);
	void drawDesign(void);
	void Draw_Door_Design(void);
	void InitializeObjects(void);

	PIXELFORMATDESCRIPTOR pfd;
	INT iPixelFormatIndex;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghDC = GetDC(ghWnd);

	iPixelFormatIndex = ChoosePixelFormat(ghDC, &pfd);

	if (iPixelFormatIndex == 0)
	{
		ReleaseDC(ghWnd, ghDC);
		ghDC = NULL;
	}

	if (SetPixelFormat(ghDC, iPixelFormatIndex, &pfd) == FALSE)
	{
		ReleaseDC(ghWnd, ghDC);
		ghDC = NULL;
	}

	ghGLRC = wglCreateContext(ghDC);
	if (ghGLRC == NULL)
	{
		ReleaseDC(ghWnd, ghDC);
		ghDC = NULL;
	}

	if (wglMakeCurrent(ghDC, ghGLRC) == FALSE)
	{
		wglDeleteContext(ghGLRC);
		ghGLRC = NULL;
		ReleaseDC(ghWnd, ghDC);
		ghDC = NULL;
	}
	
	// initialize lights
	InitializeObjects();

	errno_t error;
	error = fopen_s(&stream, "Log.txt", "w");

	if (error != 0)
	{
		MessageBox(ghWnd, TEXT("Failed to generate log file. Exiting."), TEXT("File error."), MB_OK | MB_ICONEXCLAMATION);
	}

	fprintf(stream, "Log file generated successfully\n");

	for (int i = 0; i < 128;)
	{
		// even row 85 = 01010101
		stipple[i++] = 85;
		stipple[i++] = 85;
		stipple[i++] = 85;
		stipple[i++] = 85;

		// odd row 170 = 10101010
		stipple[i++] = 170;
		stipple[i++] = 170;
		stipple[i++] = 170;
		stipple[i++] = 170;
	}

	// load all external objects as display lists
	dsBalconyBaluster = glGenLists(1);
	glNewList(dsBalconyBaluster, GL_COMPILE);
	obj.ReadOBJ("../../Resources/Objects/BalconyBaluster.obj");
	glEndList();

	dsGateBaluster = glGenLists(1);
	glNewList(dsGateBaluster, GL_COMPILE);
	obj.ReadOBJ("../../Resources/Objects/GateBaluster.obj");
	glEndList();

	dsRoofShingle = glGenLists(1);
	glNewList(dsRoofShingle, GL_COMPILE);
	obj.ReadOBJ("../../Resources/Objects/RoofShingle.obj");
	glEndList();

	dsBalconyDesign = glGenLists(1);
	glNewList(dsBalconyDesign, GL_COMPILE);
	obj.ReadOBJ("../../Resources/Objects/BalconyDesign.obj");
	glEndList();

	dsNewelCap = glGenLists(1);
	glNewList(dsNewelCap, GL_COMPILE);
	obj.ReadOBJ("../../Resources/Objects/NewelCap.obj");
	glEndList();

	dsPillarBase = glGenLists(1);
	glNewList(dsPillarBase, GL_COMPILE);
	obj.ReadOBJ("../../Resources/Objects/PillarBase.obj");
	glEndList();
	
	dsGround = glGenLists(1);
	glNewList(dsGround, GL_COMPILE);
	DrawGround(500.0, 5.0);
	glEndList();

	dsCube = glGenLists(1);
	glNewList(dsCube, GL_COMPILE);
	DrawCube(5.0f);
	glEndList();

	dsCubeRegular = glGenLists(1);
	glNewList(dsCubeRegular, GL_COMPILE);
	DrawCube(1.0f);
	glEndList();

	dsDoorDesign = glGenLists(1);
	glNewList(dsDoorDesign, GL_COMPILE);
	Draw_Door_Design();
	glEndList();

	dsClassDoor = glGenLists(1);
	glNewList(dsClassDoor, GL_COMPILE);
	drawDoor();
	glEndList();



	dsStairSectionCylinder = glGenLists(1);
	glNewList(dsStairSectionCylinder, GL_COMPILE);
	DrawCylinder(0.2, 0.2, 1.0, 50, 50);
	glEndList();


	dsWhiteDesign = glGenLists(1);
	glNewList(dsWhiteDesign, GL_COMPILE);
	drawDesign();
	glEndList();

	dsArc = glGenLists(1);
	glNewList(dsArc, GL_COMPILE);
	glPushMatrix();
	{
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
		DrawBezierSurface();
	}
	glPopMatrix();
	glPushMatrix();
	{
		glTranslatef(0.0f, 0.0f, 4.0f);
		DrawBezierSurfaceAboveArc();
	}
	glPopMatrix();
	glPushMatrix();
	{
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, 0.0f, 4.0f);
		DrawBezierSurfaceAboveArc();
	}
	glPopMatrix();
	glEndList();

	/*dsRoofShingle = glGenLists(1);
	glNewList(dsRoofShingle, GL_COMPILE);
	DrawRoofShingle();
	glEndList();*/



	dsRoof = glGenLists(1);
	glNewList(dsRoof, GL_COMPILE);
	DrawRoof();
	glEndList();

	dsUpperFrontRoof = glGenLists(1);
	glNewList(dsUpperFrontRoof, GL_COMPILE);
	DrawUpperFrontRoof();
	glEndList();

	dsUpperBackRoof = glGenLists(1);
	glNewList(dsUpperBackRoof, GL_COMPILE);
	DrawUpperBackRoof();
	glEndList();

	dsPyramidalRoof = glGenLists(1);
	glNewList(dsPyramidalRoof, GL_COMPILE);
	DrawPyramidalRoof();
	glEndList();

	dsFrontBalconyRoof = glGenLists(1);
	glNewList(dsFrontBalconyRoof, GL_COMPILE);
	DrawFrontBalconyRoof();
	glEndList();

	dsLeftBuildingRightRoof = glGenLists(1);
	glNewList(dsLeftBuildingRightRoof, GL_COMPILE);
	DrawLeftBuildingRightRoof();
	glEndList();

	dsRightBuildingFrontRoof = glGenLists(1);
	glNewList(dsRightBuildingFrontRoof, GL_COMPILE);
	DrawRightBuildingFrontRoof();
	glEndList();

	dsRightBuildingLeftRoof = glGenLists(1);
	glNewList(dsRightBuildingLeftRoof, GL_COMPILE);
	DrawRightBuildingLeftRoof();
	glEndList();

	dsRightBuildingRightRoof = glGenLists(1);
	glNewList(dsRightBuildingRightRoof, GL_COMPILE);
	DrawRightBuildingRightRoof();
	glEndList();

	dsBlackboard = glGenLists(1);
	glNewList(dsBlackboard, GL_COMPILE);
	DrawGround(20.0, 0.5);
	glEndList();

	dsStairs = glGenLists(1);
	glNewList(dsStairs, GL_COMPILE);
	/*Prabhakar-Start*/
	Init_Stairs_Drawing_Objects();
	/*Prabhakar-End*/
	Draw_Stairs();
	glEndList();

	dsClassFloor = glGenLists(1);
	glNewList(dsClassFloor, GL_COMPILE);
	DrawGround(40, 0.1);
	glEndList();

	/*LoadGLTextures(&biLeafTexture, MAKEINTRESOURCE(IDBITMAP_LEAFTEXTURE));
	LoadGLTextures(&biBranchTexture, MAKEINTRESOURCE(IDBITMAP_BRANCHEXTURE));
	glEnable(GL_TEXTURE_2D);*/

	//clear the depth buffer
	glClearDepth(1.0f);

	//cull backs of polygons
	glFrontFace(GL_CCW);	// Counter clock-wise polygon face out
	glCullFace(GL_BACK);	
	//glEnable(GL_CULL_FACE);

	// Clear the Accumulation buffer
	glClearAccum(0.0f, 0.0f, 0.0f, 0.0f);

	//Depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//smooth shade model
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);
	//////////////////////////////////////////////////////////////////////
	glEnable(GL_NORMALIZE);
	glEnable(GL_AUTO_NORMAL);

	// Enable material color tracking
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	//Resize(WIN_WIDTH, WIN_HEIGHT);
	Resize(giWidth, giHeight);
}

void InitializeObjects(void)
{
	cam.InitializeCamera();

	// Cube Initial position
	CubeTranslate.X = 0.0f;
	CubeTranslate.Y = 10.0f;
	CubeTranslate.Z = -150.0f;

	// Cube initial scale
	CubeScale.X = 1.0f;
	CubeScale.Y = 1.0f;
	CubeScale.Z = 1.0f;

	// Cube initial rotation
	CubeRotate.X = 0.0f;
	CubeRotate.Y = 0.0f;
	CubeRotate.Z = 0.0f;

	BackgroundColor.R = 0.0f;
	BackgroundColor.G = 0.0f;
	BackgroundColor.B = 0.0f;
	BackgroundColor.A = 1.0f;

	////////////////////////////////////////////////////// Lighting start
	// Set up LIGHT0
	// LIGHT0 initial position
	PositionLight0.X = 10.0f;
	PositionLight0.Y = 5.0f;
	if (gbIsEditModeOn == true)
		PositionLight0.Z = 0.0f;
	else
		PositionLight0.Z = 375.0f;
	PositionLight0.W = 1.0f;

	if (gbIsEditModeOn == true)
	{
		// LIGHT0 ambient component
		AmbientLight0.R = 0.2f;
		AmbientLight0.G = 0.2f;
		AmbientLight0.B = 0.2f;
		AmbientLight0.A = 1.0f;
	}
	else
	{
		// LIGHT0 ambient component
		AmbientLight0.R = 0.0f;
		AmbientLight0.G = 0.0f;
		AmbientLight0.B = 0.0f;
		AmbientLight0.A = 1.0f;
	}

	
	// LIGHT0 diffuse component
	DiffuseLight0.R = 1.0f;
	DiffuseLight0.G = 1.0f;
	DiffuseLight0.B = 1.0f;
	DiffuseLight0.A = 1.0f;

	// LIGHT0 specular component
	/*SpecularLight0.R = 1.0f;
	SpecularLight0.G = 1.0f;
	SpecularLight0.B = 1.0f;
	SpecularLight0.A = 1.0f;*/

	SpecularLight0.R = 0.2f;
	SpecularLight0.G = 0.2f;
	SpecularLight0.B = 0.2f;
	SpecularLight0.A = 1.0f;


	// LIGHT0 attenuation

	if (gbIsEditModeOn == true)
	{
		AttenuationLight0.Constant = 1.0f;
		AttenuationLight0.Linear = 0.0f;
		AttenuationLight0.Quadratic = 0.0f;
	}
	else
	{
		AttenuationLight0.Constant = 2.0f;
		AttenuationLight0.Linear = 0.5f;
		AttenuationLight0.Quadratic = 0.0f;
	}

	GLfloat AmbientLight_0[] = { AmbientLight0.R , AmbientLight0.G, AmbientLight0.B, AmbientLight0.A };
	GLfloat DiffuseLight_0[] = { DiffuseLight0.R, DiffuseLight0.G, DiffuseLight0.B, DiffuseLight0.A };
	GLfloat SpecularLight_0[] = { SpecularLight0.R, SpecularLight0.G, SpecularLight0.B, SpecularLight0.A };

	glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientLight_0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseLight_0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularLight_0);
	
	// set up the light model
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gbAmbientLightModel);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gbNoAmbientLightModel);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, gfLightModelLocalViewer);

	// Set up LIGHT1
	// LIGHT1 initial position
	PositionLight1.X = 0.0f;
	PositionLight1.Y = 60.0f;
	PositionLight1.Z = -24.0f;
	PositionLight1.W = 1.0f;

	// LIGHT1 ambient component
	AmbientLight1.R = 0.0f;
	AmbientLight1.G = 0.0f;
	AmbientLight1.B = 0.0f;
	AmbientLight1.A = 1.0f;

	// LIGHT1 diffuse component
	// goldenrod color
	//DiffuseLight1.R = 0.855f;
	//DiffuseLight1.G = 0.647f;
	//DiffuseLight1.B = 0.125f;
	//DiffuseLight1.A = 1.0f;

	DiffuseLight1.R = 0.0f;
	DiffuseLight1.G = 1.0f;
	DiffuseLight1.B = 1.0f;
	DiffuseLight1.A = 1.0f;


	// LIGHT1 specular component
	//SpecularLight1.R = 0.4275f;
	//SpecularLight1.G = 0.3235f;
	//SpecularLight1.B = 0.0625f;
	//SpecularLight1.A = 1.0f;

	SpecularLight1.R = 0.0f;
	SpecularLight1.G = 1.0f;
	SpecularLight1.B = 1.0f;
	SpecularLight1.A = 1.0f;

	AttenuationLight1.Constant = 5.0f;
	AttenuationLight1.Linear = 5.0f;
	AttenuationLight1.Quadratic = 0.0f;
	//set up the light1

	GLfloat AmbientLight_1[] = { AmbientLight1.R , AmbientLight1.G, AmbientLight1.B, AmbientLight1.A };
	GLfloat DiffuseLight_1[] = { DiffuseLight1.R, DiffuseLight1.G, DiffuseLight1.B, DiffuseLight1.A };
	GLfloat SpecularLight_1[] = { SpecularLight1.R, SpecularLight1.G, SpecularLight1.B, SpecularLight1.A };
	


	glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientLight_1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, DiffuseLight_1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, SpecularLight_1);

	///////////////////////////////////////////////////////
	// Set up LIGHT2
	// LIGHT2 initial position
	PositionLight2.X = 0.0f;
	PositionLight2.Y = 51.0f;
	PositionLight2.Z = 12.0f;
	PositionLight2.W = 1.0f;

	// LIGHT2 ambient component
	AmbientLight2.R = 0.0f;
	AmbientLight2.G = 0.0f;
	AmbientLight2.B = 0.0f;
	AmbientLight2.A = 1.0f;

	// LIGHT2 diffuse component
	DiffuseLight2.R = 0.71f;
	DiffuseLight2.G = 0.2588f;
	DiffuseLight2.B = 0.9568f;
	DiffuseLight2.A = 1.0f;

	// LIGHT2 specular component
	SpecularLight2.R = 0.71f;
	SpecularLight2.G = 0.2588f;
	SpecularLight2.B = 0.9568f;
	SpecularLight2.A = 1.0f;

	AttenuationLight2.Constant = 1.0f;
	AttenuationLight2.Linear = 0.0f;
	AttenuationLight2.Quadratic = 0.0f;
	//set up the LIGHT2

	GLfloat AmbientLight_2[] = { AmbientLight2.R , AmbientLight2.G, AmbientLight2.B, AmbientLight2.A };
	GLfloat DiffuseLight_2[] = { DiffuseLight2.R, DiffuseLight2.G, DiffuseLight2.B, DiffuseLight2.A };
	GLfloat SpecularLight_2[] = { SpecularLight2.R, SpecularLight2.G, SpecularLight2.B, SpecularLight2.A };

	glLightfv(GL_LIGHT2, GL_AMBIENT, AmbientLight_2);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, DiffuseLight_2);
	glLightfv(GL_LIGHT2, GL_SPECULAR, SpecularLight_2);



	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_LIGHT1);
	//gbEnableLight1 = true;

	if (gbIsEditModeOn == true)
	{
		//glEnable(GL_LIGHT1);
		//gbEnableLight1 = true;

		/*glEnable(GL_LIGHT2);
		gbEnableLight2 = true;*/
	}
	else
	{
		// original
		/*glDisable(GL_LIGHT1);
		gbEnableLight1 = false;
		glDisable(GL_LIGHT2);
		gbEnableLight2 = false;*/
	}
}


#define ACCSIZE 15

void Display(void)
{
	void RenderScene(void);
	void AccumulationPerspective(GLdouble fovy, GLdouble aspect, GLdouble znear, GLdouble zfar, GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble focus);
	glClearColor(BackgroundColor.R, BackgroundColor.G, BackgroundColor.B, BackgroundColor.A);

	GLint viewport[4];
	int jitter;

	if (gbFullSceneAtiAliasing == true)
	{
		glGetIntegerv(GL_VIEWPORT, viewport);
		glClear(GL_ACCUM_BUFFER_BIT);
		for (jitter = 0; jitter < ACCSIZE; jitter++)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			AccumulationPerspective(60.0, (GLdouble)viewport[2] / (GLdouble)viewport[3], 0.1, 1000.0, j15[jitter].x, j15[jitter].y, 0.0, 0.0, 1.0);
			{
				RenderScene();
			}
			glAccum(GL_ACCUM, 1.0f / ACCSIZE);
		}
		glAccum(GL_RETURN, 1.0);
	}
	else
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderScene();
	}

	
	SwapBuffers(ghDC);
}

void RenderScene(void)
{
	void GauravCode();
	void drawFloor(void);
	void DrawStairs(void);
	void drawPositionalTrapezoid(void);
	void drawHallUpperBeam(void);
	void drawInnerLeftSide(void);
	void drawInnerRightSide(void);

	//function prototypes
	void DrawCorridorFloor(void);
	void balcony(GLfloat width, GLfloat height, GLfloat depth, GLint noOfPillers, GLfloat distBetweenPillers, GLfloat PillerHeight, GLfloat middleHalfDistance, GLint i);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(0.0, 1.0, 5.0, 0.0, 1.0, -5.0, 0.0, 1.0, 0.0);
	cam.SetCameraPosition();
	gluLookAt(cam.CameraPosition.X, cam.CameraPosition.Y, cam.CameraPosition.Z, cam.TargetPoint.X, cam.TargetPoint.Y, cam.TargetPoint.Z, cam.UpVector.X, cam.UpVector.Y, cam.UpVector.Z);

	//GLfloat LightPosition[] = { PositionLight0.X, PositionLight0.Y, PositionLight0.Z, PositionLight0.W };
	GLfloat LightPosition[4];
	GLfloat LightPosition1[] = { PositionLight1.X, PositionLight1.Y, PositionLight1.Z, PositionLight1.W };
	GLfloat LightPosition2[] = { PositionLight2.X, PositionLight2.Y, PositionLight2.Z, PositionLight2.W };

	GLfloat SpotDirection_1[] = { 0.0f, -1.0f, 0.0f };
	GLfloat SpotExponent = 10.0f;

	if (gbLightOnCamera == false)
	{
	LightPosition[0] = PositionLight0.X;
	LightPosition[1] = PositionLight0.Y;
	LightPosition[2] = PositionLight0.Z;
	LightPosition[3] = PositionLight0.W;
	}
	else
	{
	LightPosition[0] = (GLfloat)cam.CameraPosition.X;
	LightPosition[1] = (GLfloat)cam.CameraPosition.Y;
	LightPosition[2] = (GLfloat)cam.CameraPosition.Z;
	LightPosition[3] = PositionLight0.W;
	}

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, AttenuationLight0.Constant);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, AttenuationLight0.Linear);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, AttenuationLight0.Quadratic);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, AttenuationLight1.Constant);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, AttenuationLight1.Linear);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, AttenuationLight1.Quadratic);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition1);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, SpotDirection_1);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
	//glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, SpotExponent);


	glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, AttenuationLight2.Constant);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, AttenuationLight2.Linear);
	glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, AttenuationLight2.Quadratic);
	glLightfv(GL_LIGHT2, GL_POSITION, LightPosition2);

	glPushMatrix();

	if (gbIsEditModeOn == true)
	{
		// Draw movable and scalable cube
		glPushMatrix();
		{
			glColor3f(0.941f, 0.902f, 0.549f);
			glTranslatef(CubeTranslate.X, CubeTranslate.Y, CubeTranslate.Z);
			glRotatef(CubeRotate.X, 1.0f, 0.0f, 0.0f);
			glRotatef(CubeRotate.Y, 0.0f, 1.0f, 0.0f);
			glRotatef(CubeRotate.Z, 0.0f, 0.0f, 1.0f);
			glScalef(CubeScale.X, CubeScale.Y, CubeScale.Z);
			//glCallList(dsCubeRegular);
			//DrawOriginAxes(2.0f, 2.0f);
		}
		glPopMatrix();
	}
	

	/*Gaurav Patil Start*/
	GauravCode();
	/*Gaurav patil end*/

	drawMainFrontStructure();

	glPushMatrix();					//Chair - chair must be drawn after class room
	{
		glTranslatef(0.0f, 41.0f, -25.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glScalef(0.5f, 0.5f, 0.5f);
		drawChair();
	}
	glPopMatrix();

	/*Atul Linganwar - Balcony and corridor floor*/
	DrawCorridorFloor();
	glPushMatrix();
	{
		glTranslatef(-1.0f, 46.0f, -112.0f);
		balcony(2.0f, 1.0f, 2.0f, 4, 24.0f, 23.0f, 21.0f, 1);
		balcony(2.0f, 1.0f, 2.0f, 4, 24.0f, 23.0f, 20.0f, -1);
	}
	glPopMatrix();

	/*Prabhakar Patil -> stairs*/
	DrawStairs();

	///////////////////////////////////////////////////////////////////////
	glPushMatrix();
	drawPositionalTrapezoid();
	glPopMatrix();
	///////////////////////////////////////////////////////////////////////

	//display
	glPushMatrix();			//Aashish
	//glScalef(0.8f, 1.0f, 1.0f);
	drawHallUpperBeam();
	glPopMatrix();

	glPushMatrix();
	drawInnerLeftSide();
	glPopMatrix();

	glPushMatrix();
	drawInnerRightSide();
	glPopMatrix();
	
	// essentials for editing
	glDisable(GL_LIGHTING);

	if (gbIsEditModeOn == true)
	{
		glPushMatrix();
		//DrawOriginAxes(10.0f, 3.0f);
		glPopMatrix();
	}
	
	glPushMatrix();
	glTranslatef(LightPosition[0], LightPosition[1], LightPosition[2]);
	if (gbIsEditModeOn == true)
		glColor4f(DiffuseLight0.R, DiffuseLight0.G, DiffuseLight0.B, DiffuseLight0.A);
	else
		glColor4f(1.0f, 1.0f, 1.0f, gfLightSphereAlpha);
	DrawSphere(0.5, 50, 50);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(LightPosition1[0], LightPosition1[1], LightPosition1[2]);
	glColor4f(DiffuseLight1.R, DiffuseLight1.G, DiffuseLight1.B, DiffuseLight1.A);
	DrawSphere(0.5, 50, 50);
	glPopMatrix();

	glEnable(GL_LIGHTING);

	glPopMatrix();	// origin

	glLoadIdentity();
	glBegin(GL_QUADS);
	glColor4f(0.0f, 0.0f, 0.0f, gfHUDScreenAlpha);
	glVertex3f(1.0f, 1.0f, -0.1f);
	glVertex3f(-1.0f, 1.0f, -0.1f);
	glVertex3f(-1.0f, -1.0f, -0.1f);
	glVertex3f(1.0f, -1.0f, -0.1f);
	glEnd();
}

void AccumulationFrustum(GLdouble Left, GLdouble Right, GLdouble Bottom, GLdouble Top, GLdouble ZNear, GLdouble ZFar, GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble focus)
{
	GLdouble xwsize, ywsize;
	GLdouble dx, dy;
	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);

	xwsize = Right - Left;
	ywsize = Top - Bottom;

	dx = -(pixdx * xwsize / (GLdouble)viewport[2] + eyedy * ZNear / focus);
	dy = -(pixdy * ywsize / (GLdouble)viewport[3] + eyedy * ZNear / focus);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(Left + dx, Right + dx, Bottom + dy, Top + dy, ZNear, ZFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glTranslatef(-eyedx, -eyedy, 0.0);
}

void AccumulationPerspective(GLdouble fovy, GLdouble aspect, GLdouble znear, GLdouble zfar, GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble focus)
{
	void AccumulationFrustum(GLdouble Left, GLdouble Right, GLdouble Bottom, GLdouble Top, GLdouble ZNear, GLdouble ZFar, GLdouble pixdx, GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble focus);

	GLdouble fov2, left, right, bottom, top;
	fov2 = ((fovy * M_PI) / 180.0) / 2.0;

	top = znear / (cos(fov2) / sin(fov2));
	bottom = -top;
	right = top * aspect;
	left = -right;

	AccumulationFrustum(left, right, bottom, top, znear, zfar, pixdx, pixdy, eyedx, eyedy, focus);
}


void Resize(int Width, int Height)
{
	//fprintf(stream, "Window Width : %d Window Height : %d", Width, Height);

	GLdouble Left = FRUSTUM_LEFT;
	GLdouble Right = FRUSTUM_RIGHT;
	GLdouble Bottom = FRUSTUM_BOTTOM;
	GLdouble Top = FRUSTUM_TOP;
	GLdouble Near = FRUSTUM_NEAR;
	GLdouble Far = FRUSTUM_FAR;
	//GLdouble FOVY = FOV_Y;
	GLdouble FOVY = fovy;

	if (Height == 0)
		Height = 1;

	if (Width == 0)
		Width = 1;

	if (Width <= Height)
	{
		Bottom = Bottom * ((GLdouble)Height / (GLdouble)Width);
		Top = Top * ((GLdouble)Height / (GLdouble)Width);
	}
	else
	{
		Left = Left * ((GLdouble)Width / (GLdouble)Height);
		Right = Right * ((GLdouble)Width / (GLdouble)Height);
	}

	glViewport(0, 0, (GLsizei)Width, (GLsizei)Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLdouble dAspectRatio = (GLdouble)Width / (GLdouble)Height;

	Top = Near * tan(FOVY / 360.0 * M_PI);
	Right = Top * dAspectRatio;

	Bottom = -Top;
	Left = -Top * dAspectRatio;

	glFrustum(Left * gdZoomFactor, Right * gdZoomFactor, Bottom * gdZoomFactor, Top * gdZoomFactor, Near, Far);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Zoom(short zDelta)
{
	if (zDelta > 0)
		gdZoomFactor -= ZOOM_FACTOR;
	else
		gdZoomFactor += ZOOM_FACTOR;

	if (gdZoomFactor <= 0.05)
		gdZoomFactor = 0.05;

	if (gdZoomFactor >= 2.0)
		gdZoomFactor = 1.99;

	Resize(giWidth, giHeight);
}

void OnLButtonDown(int LeftClickX, int LeftClickY, DWORD LeftClickFlags)
{
	//fprintf(stream, "\n\n*Left Mouse Click X : %d Y : %d\n\n", LeftClickX, LeftClickY);
	RECT Clip;
	//RECT PrevClip;

	SetCapture(ghWnd);
	//GetClipCursor(&PrevClip);
	GetWindowRect(ghWnd, &Clip);
	ClipCursor(&Clip);

	GetCursorPos(&ClickPoint);
	if (gbIsEditModeOn == true)
		ShowCursor(true);
	else
		ShowCursor(false);
}

void OnLButtonUp(int LeftUpX, int LeftUpY, DWORD LeftUpFlags)
{
	//fprintf(stream, "\n\n*Left Mouse Up X : %d Y : %d\n\n", LeftUpX, LeftUpY);

	ClipCursor(NULL);
	ReleaseCapture();
	SetCursorPos(ClickPoint.x, ClickPoint.y);
	if (gbIsEditModeOn == true)
		ShowCursor(true);
	else
		ShowCursor(false);
}

void OnRButtonDown(int RightClickX, int RightClickY, DWORD RightClickFlags)
{
	//fprintf(stream, "\n\n*Right Mouse Click X : %d Y : %d\n\n", RightClickX, RightClickY);

	RECT Clip;
	//RECT PrevClip;

	SetCapture(ghWnd);
	//GetClipCursor(&PrevClip);
	GetWindowRect(ghWnd, &Clip);
	ClipCursor(&Clip);

	GetCursorPos(&ClickPoint);
	if (gbIsEditModeOn == true)
		ShowCursor(true);
	else
		ShowCursor(false);
}

void OnRButtonUp(int RightUpX, int RightUpY, DWORD RightUpFlags)
{
	//fprintf(stream, "\n\n*Right Mouse Up X : %d Y : %d\n\n", RightUpX, RightUpY);

	ClipCursor(NULL);
	ReleaseCapture();
	SetCursorPos(ClickPoint.x, ClickPoint.y);
	if (gbIsEditModeOn == true)
		ShowCursor(true);
	else
		ShowCursor(false);
}

void OnMButtonDown(int MiddleDownX, int MiddleDownY, DWORD MiddleDownFlags)
{
	RECT Clip;
	//RECT PrevClip;

	SetCapture(ghWnd);
	//GetClipCursor(&PrevClip);
	GetWindowRect(ghWnd, &Clip);
	ClipCursor(&Clip);

	GetCursorPos(&ClickPoint);
	if (gbIsEditModeOn == true)
		ShowCursor(true);
	else
		ShowCursor(false);
}

void OnMButtonUp(int MiddleUpX, int MiddleUpY, DWORD MiddleUpFlags)
{
	ClipCursor(NULL);
	ReleaseCapture();
	SetCursorPos(ClickPoint.x, ClickPoint.y);
	if (gbIsEditModeOn == true)
		ShowCursor(true);
	else
		ShowCursor(false);
}

void OnMouseMove(int MouseMoveX, int MouseMoveY, DWORD Flags)
{
	//fprintf(stream, "+X Threshold : %f -X Threshold : %f +Y Threshold : %f -Y Threshold : %f\n", iPositiveXThreshold, iNegativeXThreshold, iPositiveYThreshold, iNegativeYThreshold);
	//fprintf(stream, "On Mouse Move X : %d Y : %d\n", MouseMoveX, MouseMoveY);
	if (gbIsEditModeOn == true)
		ShowCursor(true);
	else
		ShowCursor(false);


	if (Flags & MK_LBUTTON)
	{
		// Code
	}

	MovePoint.x = MouseMoveX;
	MovePoint.y = MouseMoveY;

	/*
	if (Flags & MK_RBUTTON)
	{
	if (MouseMoveX > iPositiveXThreshold)
	{
	fprintf(stream, "\n\nOn Right Click Mose move X: %d\n\n", MouseMoveX);
	cam.CameraRotateRight(0.5);
	}


	if (MouseMoveX < iNegativeXThreshold)
	{
	fprintf(stream, "\n\nOn Right Click Mose move X: %d\n\n", MouseMoveX);
	cam.CameraRotateLeft(0.5);
	}

	if (MouseMoveY > iPositiveYThreshold)
	{
	fprintf(stream, "\n\nOn Right Click Mose move Y: %d\n\n", MouseMoveY);
	cam.CameraRotateUp(0.5);
	}

	if (MouseMoveY < iNegativeYThreshold)
	{
	fprintf(stream, "\n\nOn Right Click Mose move Y: %d\n\n", MouseMoveY);
	cam.CameraRotateDown(0.5);
	}
	}
	*/
	if (Flags & MK_RBUTTON)
	{
		if (MovePoint.x > OldMovePoint.x)
		{
			cam.Yaw(-3.0);
		}

		if (MovePoint.x < OldMovePoint.x)
		{
			cam.Yaw(3.0);
		}

		if (MovePoint.y > OldMovePoint.y)
		{
			cam.Pitch(-3.0);
		}

		if (MovePoint.y < OldMovePoint.y)
		{
			cam.Pitch(3.0);
		}
	}

	if (Flags & MK_MBUTTON || Flags & MK_CONTROL)
	{
		if (MovePoint.y > OldMovePoint.y)
		{

		}

		if (MovePoint.y < OldMovePoint.y)
		{

		}
	}

	if (gbIsEditModeOn == true)
		ShowCursor(true);
	else
		ShowCursor(false);

	OldMovePoint = MovePoint;
}

void ToggleFullScreen(void)
{
	HMONITOR hMonitor;
	MONITORINFO mi;
	BOOL bWindowPlacement;
	BOOL bMonitorInfo;

	if (gbFullScreen == false)
	{
		dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			wpPrev.length = { sizeof(WINDOWPLACEMENT) };
			mi.cbSize = { sizeof(MONITORINFO) };

			bWindowPlacement = GetWindowPlacement(ghWnd, &wpPrev);
			hMonitor = MonitorFromWindow(ghWnd, MONITOR_DEFAULTTOPRIMARY);
			bMonitorInfo = GetMonitorInfo(hMonitor, &mi);

			if (bWindowPlacement == TRUE && bMonitorInfo == TRUE)
			{
				SetWindowLong(ghWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		if (gbIsEditModeOn == true)
			ShowCursor(true);
		else
			ShowCursor(false);
	}
	else
	{
		SetWindowLong(ghWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghWnd, &wpPrev);
		SetWindowPos(ghWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(true);
	}
}

void Uninitialize(void)
{

	fprintf_s(stream, " end cube's translation X : %f Y : %f Z : %f \nscale X : %f Y : %f Z : %f\nrotation X : %f Y : %f Z : %f\nGoodbye..\n", CubeTranslate.X, CubeTranslate.Y, CubeTranslate.Z, CubeScale.X, CubeScale.Y, CubeScale.Z, CubeRotate.X, CubeRotate.Y, CubeRotate.Z);
	fclose(stream);


	if (gbFullScreen == true)
	{
		dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
		SetWindowLong(ghWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghWnd, &wpPrev);
		SetWindowPos(ghWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(true);
	}

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(ghGLRC);
	ghGLRC = NULL;

	ReleaseDC(ghWnd, ghDC);
	ghDC = NULL;

	DestroyWindow(ghWnd);
}

////////////////////////////////////////////////////////////
// model loading code
void OBJ::ReadOBJ(string FileName)
{
	ifstream ifs(FileName.c_str(), ifstream::in);
	string line, key;
	while (ifs.good() && !ifs.eof() && getline(ifs, line))
	{
		//fprintf_s(stream, "in while\n");
		key = "";
		stringstream StringStream(line);
		StringStream >> key >> ws; //Extracts as many whitespace characters as possible from the current position in the input sequence. The extraction stops as soon as a non-whitespace character is found. These extracted whitespace characters are discarded.
		//fprintf_s(stream, "Key : %s\n", key.c_str());
		if (key == "v")	// Vertex
		{
			//fprintf_s(stream, "vertex found\n");
			vertex v;
			float x;
			while (!StringStream.eof())
			{
				StringStream >> x >> ws;
				v.v.push_back(x);
			}
			vertices.push_back(v);
		}
		else if (key == "vp")	// Parameter
		{
			//fprintf_s(stream, "parameter found\n");
			vertex v;
			float x;
			while (!StringStream.eof())
			{
				StringStream >> x >> ws;
				v.v.push_back(x);
			}
			parameters.push_back(v);
		}
		else if (key == "vt")	// Texture Co-ordinates
		{
			//fprintf_s(stream, "texcoord found\n");
			vertex v;
			float x;
			while (!StringStream.eof())
			{
				StringStream >> x >> ws;
				v.v.push_back(x);
			}
			texcoords.push_back(v);
		}
		else if (key == "vn")	// Normal
		{
			//fprintf_s(stream, "normal found\n");
			vertex v;
			float x;
			while (!StringStream.eof())
			{
				StringStream >> x >> ws;
				v.v.push_back(x);
			}
			v.normalize();
			normals.push_back(v);
		}
		else if (key == "f")
		{
			//fprintf_s(stream, "face found\n");
			face f;
			int v, t, n;
			while (!StringStream.eof())
			{
				StringStream >> v >> ws;
				f.vertex.push_back(v - 1);
				if (StringStream.peek() == '/')
				{
					StringStream.get();
					if (StringStream.peek() == '/')
					{
						StringStream.get();
						StringStream >> n >> ws;
						f.normal.push_back(n - 1);
					}
					else
					{
						StringStream >> t >> ws;
						f.texture.push_back(t - 1);
						if (StringStream.peek() == '/')
						{
							StringStream.get();
							StringStream >> n >> ws;
							f.normal.push_back(n - 1);
						}
					}
				}
			}
			faces.push_back(f);
		}
		else
		{
			fprintf_s(stream, "other line found\n");
			// Other Line
		}
	}
	ifs.close();
	fprintf_s(stream, "Name : %s\n", FileName.c_str());
	fprintf_s(stream, "Vertices : %zd\n", vertices.size());
	fprintf_s(stream, "Parameters : %zd\n", parameters.size());
	fprintf_s(stream, "Texture Coordinates : %zd\n", texcoords.size());
	fprintf_s(stream, "Normals : %zd\n", normals.size());
	fprintf_s(stream, "Faces : %zd\n", faces.size());

	DrawObject();
	vertices.clear();
	texcoords.clear();
	normals.clear();
	faces.clear();
}

OBJ::~OBJ()
{
	//glDeleteLists(list, 1);
}

void OBJ::DrawObject()
{
	for (int i = 0; i < faces.size(); i++)
	{
		if (faces[i].vertex.size() == 3)	// Triangle
		{
			if (faces[i].normal.size() == 3)	// With Normals
			{
				glBegin(GL_TRIANGLES);
				glNormal3f(normals[faces[i].normal[0]].v[0], normals[faces[i].normal[0]].v[1], normals[faces[i].normal[0]].v[2]);
				glVertex3f(vertices[faces[i].vertex[0]].v[0], vertices[faces[i].vertex[0]].v[1], vertices[faces[i].vertex[0]].v[2]);
				glNormal3f(normals[faces[i].normal[1]].v[0], normals[faces[i].normal[1]].v[1], normals[faces[i].normal[1]].v[2]);
				glVertex3f(vertices[faces[i].vertex[1]].v[0], vertices[faces[i].vertex[1]].v[1], vertices[faces[i].vertex[1]].v[2]);
				glNormal3f(normals[faces[i].normal[2]].v[0], normals[faces[i].normal[2]].v[1], normals[faces[i].normal[2]].v[2]);
				glVertex3f(vertices[faces[i].vertex[2]].v[0], vertices[faces[i].vertex[2]].v[1], vertices[faces[i].vertex[2]].v[2]);
				glEnd();
			}
			else  // Without normals
			{
				//fprintf_s(stream, "without normal not supported");
			}
		}
		else
		{
			//fprintf_s(stream, "Quadrilateral not supported");
		}
	}
	
}
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// Functions for camera
M3DVector3d M3DVector(double X, double Y, double Z)
{
	M3DVector3d vector;
	vector.X = X;
	vector.Y = Y;
	vector.Z = Z;
	return vector;
}

double Get3DVectorLength(M3DVector3d *vector)
{
	double VectorLength = sqrt(SQR(vector->X) + SQR(vector->Y) + SQR(vector->Z));
	return VectorLength;
}

M3DVector3d Normalize3DVector(M3DVector3d vector)
{
	M3DVector3d ResultantVector;
	double length = Get3DVectorLength(&vector);
	if (length == 0.0)
		return NULL_VECTOR;

	ResultantVector.X = vector.X / length;
	ResultantVector.Y = vector.Y / length;
	ResultantVector.Z = vector.Z / length;
	return ResultantVector;
}

M3DVector3d operator+(M3DVector3d V, M3DVector3d U)
{
	M3DVector3d ResultantVector;
	ResultantVector.X = V.X + U.X;
	ResultantVector.Y = V.Y + U.Y;
	ResultantVector.Z = V.Z + U.Z;
	return ResultantVector;
}

M3DVector3d operator-(M3DVector3d V, M3DVector3d U)
{
	M3DVector3d ResultantVector;
	ResultantVector.X = V.X - U.X;
	ResultantVector.Y = V.Y - U.Y;
	ResultantVector.Z = V.Z - U.Z;
	return ResultantVector;
}

M3DVector3d operator*(M3DVector3d V, double R)
{
	M3DVector3d ResultantVector;
	ResultantVector.X = V.X * R;
	ResultantVector.Y = V.Y * R;
	ResultantVector.Z = V.Z * R;
	return ResultantVector;
}

M3DVector3d CrossProduct(M3DVector3d *U, M3DVector3d *V)
{
	M3DVector3d ResultantVector;
	ResultantVector.X = U->Y * V->Z - U->Z * V->Y;
	ResultantVector.Y = U->Z * V->X - U->X * V->Z;
	ResultantVector.Z = U->X * V->Y - U->Y * V->X;

	return ResultantVector;
}

double operator*(M3DVector3d V, M3DVector3d U)	// dot product
{
	return V.X * U.X + V.Y * U.Y + V.Z * U.Z;
}

void CCamera::InitializeCamera(void)
{
	if (gbIsEditModeOn == true)
		CameraPosition = M3DVector(0.0, 5.0, 10.0);
	else
		CameraPosition = M3DVector(0.0, 5.0, 400.0);

	ViewDirection = M3DVector(0.0, 0.0, -1.0);
	RightVector = M3DVector(1.0, 0.0, 0.0);
	UpVector = M3DVector(0.0, 1.0, 0.0);

	RotateX = 0.0;
	RotateY = 0.0;
	RotateZ = 0.0;
}

void CCamera::Move(M3DVector3d Direction)
{
	CameraPosition = CameraPosition + Direction;
}

void CCamera::Pitch(double Angle)
{
	RotateX += Angle;

	// Rotate ViewDirection around the right vector
	ViewDirection = Normalize3DVector(ViewDirection * cos(Angle * PI_DIV_180) + UpVector * sin(Angle * PI_DIV_180));

	// Compute the new up vector (by cross product)
	//UpVector = CrossProduct(&ViewDirection, &RightVector) * -1;
}

void CCamera::Yaw(double Angle)
{
	RotateY += Angle;

	// Rotate ViewDirection around the up vector
	ViewDirection = Normalize3DVector(ViewDirection * cos(Angle * PI_DIV_180) - RightVector * sin(Angle * PI_DIV_180));

	// Compute the new right vector (by cross product)
	RightVector = CrossProduct(&ViewDirection, &UpVector);
}

void CCamera::Roll(double Angle)
{
	RotateZ += Angle;

	RightVector = Normalize3DVector(RightVector * cos(Angle * PI_DIV_180) + UpVector * sin(Angle * PI_DIV_180));

	UpVector = CrossProduct(&ViewDirection, &RightVector) * -1;
}

void CCamera::SetCameraPosition(void)
{
	// The target point
	TargetPoint = CameraPosition + ViewDirection;
}

void CCamera::MoveForward(double Distance)
{
	CameraPosition = CameraPosition + (ViewDirection * -Distance);
}

void CCamera::StrafeRight(double Distance)
{
	CameraPosition = CameraPosition + (RightVector * Distance);
}

void CCamera::MoveUpward(double Distance)
{
	CameraPosition = CameraPosition + (UpVector * Distance);
}

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Shapes
void DrawColoredCube(GLfloat Side)
{
	M3DVector3f vNormal;
	glBegin(GL_QUADS);

	{ // Front Face
		M3DVector3f vPoints[4] = { { Side, Side, Side },
		{ -Side, Side, Side },
		{ -Side, -Side, Side },
		{ Side, -Side, Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Right Face
		M3DVector3f vPoints[4] = { { Side, Side, -Side },
		{ Side, Side, Side },
		{ Side, -Side, Side },
		{ Side, -Side, -Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Back Face
		M3DVector3f vPoints[4] = { { -Side, Side, -Side },
		{ Side, Side, -Side },
		{ Side, -Side, -Side },
		{ -Side, -Side, -Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Left Face
		M3DVector3f vPoints[4] = { { -Side, Side, Side },
		{ -Side, Side, -Side },
		{ -Side, -Side, -Side },
		{ -Side, -Side, Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Top Face
		M3DVector3f vPoints[4] = { { Side, Side, -Side },
		{ -Side, Side, -Side },
		{ -Side, Side, Side },
		{ Side, Side, Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Bottom Face
		M3DVector3f vPoints[4] = { { Side, -Side, Side },
		{ -Side, -Side, Side },
		{ -Side, -Side, -Side },
		{ Side, -Side, -Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	glEnd();
}

void DrawCube(GLfloat Side)
{
	M3DVector3f vNormal;
	glBegin(GL_QUADS);

	{ // Front Face
		M3DVector3f vPoints[4] = { { Side, Side, Side },
		{ -Side, Side, Side },
		{ -Side, -Side, Side },
		{ Side, -Side, Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Right Face
		M3DVector3f vPoints[4] = { { Side, Side, -Side },
		{ Side, Side, Side },
		{ Side, -Side, Side },
		{ Side, -Side, -Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Back Face
		M3DVector3f vPoints[4] = { { -Side, Side, -Side },
		{ Side, Side, -Side },
		{ Side, -Side, -Side },
		{ -Side, -Side, -Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Left Face
		M3DVector3f vPoints[4] = { { -Side, Side, Side },
		{ -Side, Side, -Side },
		{ -Side, -Side, -Side },
		{ -Side, -Side, Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Top Face
		M3DVector3f vPoints[4] = { { Side, Side, -Side },
		{ -Side, Side, -Side },
		{ -Side, Side, Side },
		{ Side, Side, Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Bottom Face
		M3DVector3f vPoints[4] = { { Side, -Side, Side },
		{ -Side, -Side, Side },
		{ -Side, -Side, -Side },
		{ Side, -Side, -Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	glEnd();
}

void DrawColoredPyramid(GLfloat Side)
{
	M3DVector3f vNormal;
	glBegin(GL_TRIANGLES);

	{	// Front Face
		M3DVector3f vPoints[3] = { { 0.0f, Side, 0.0f },
		{ -Side, -Side, Side },
		{ Side, -Side, Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[0]);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[1]);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glVertex3fv(vPoints[2]);
	}

	{	// Right Face
		M3DVector3f vPoints[3] = { { 0.0f, Side, 0.0f },
		{ Side, -Side, Side },
		{ Side, -Side, -Side } };
		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[0]);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glVertex3fv(vPoints[1]);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[2]);
	}

	{	// Back Face
		M3DVector3f vPoints[3] = { { Side, Side, 0.0f },
		{ Side, -Side, -Side },
		{ -Side, -Side, -Side } };
		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[0]);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[1]);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glVertex3fv(vPoints[2]);
	}

	{	// Left face
		M3DVector3f vPoints[3] = { { 0.0f, Side, 0.0f },
		{ -Side, -Side, -Side },
		{ -Side, -Side, Side } };
		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[0]);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glVertex3fv(vPoints[1]);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3fv(vPoints[2]);
	}

	glEnd();
}

void DrawPyramid(GLfloat Side)
{
	M3DVector3f vNormal;
	glBegin(GL_TRIANGLES);

	{	// Front Face
		M3DVector3f vPoints[3] = { { 0.0f, Side, 0.0f },
		{ -Side, -Side, Side },
		{ Side, -Side, Side } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
	}

	{	// Right Face
		M3DVector3f vPoints[3] = { { 0.0f, Side, 0.0f },
		{ Side, -Side, Side },
		{ Side, -Side, -Side } };
		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
	}

	{	// Back Face
		M3DVector3f vPoints[3] = { { Side, Side, 0.0f },
		{ Side, -Side, -Side },
		{ -Side, -Side, -Side } };
		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
	}

	{	// Left face
		M3DVector3f vPoints[3] = { { 0.0f, Side, 0.0f },
		{ -Side, -Side, -Side },
		{ -Side, -Side, Side } };
		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
	}

	glEnd();
}

void DrawSphere(GLdouble Radius, GLuint Slices, GLuint Stacks)
{
	GLUquadric *quadric = NULL;
	quadric = gluNewQuadric();
	gluSphere(quadric, Radius, Slices, Stacks);
	gluDeleteQuadric(quadric);
}

void DrawCone(GLdouble BaseRadius, GLdouble Height, GLuint Slices, GLuint Stacks)
{
	GLUquadric *quadric = NULL;
	quadric = gluNewQuadric();
	glBegin(GL_LINE_LOOP);
	gluCylinder(quadric, BaseRadius, 0.0, Height, Slices, Stacks);
	glEnd();
	gluDeleteQuadric(quadric);
}

void DrawCylinder(GLdouble BaseRadius, GLdouble TopRadius, GLdouble Height, GLuint Slices, GLuint Stacks)
{
	GLUquadric *quadric = NULL;
	quadric = gluNewQuadric();
	gluCylinder(quadric, BaseRadius, TopRadius, Height, Slices, Stacks);
	gluDeleteQuadric(quadric);
}

void DrawTorus(GLfloat MajorRadius, GLfloat MinorRadius, GLuint NumMajor, GLuint NumMinor)
{
	M3DVector3f vNormal;
	GLdouble MajorStep = 2.0f * M3D_PI / NumMajor;
	GLdouble MinorStep = 2.0f * M3D_PI / NumMinor;
	unsigned int i, j;

	for (i = 0; i < NumMajor; ++i)
	{
		GLdouble a0 = i * MajorStep;
		GLdouble a1 = a0 + MajorStep;
		GLfloat x0 = (GLfloat)cos(a0);
		GLfloat y0 = (GLfloat)sin(a0);
		GLfloat x1 = (GLfloat)cos(a1);
		GLfloat y1 = (GLfloat)sin(a1);

		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j <= NumMinor; ++j)
		{
			GLdouble b = j * MinorStep;
			GLfloat c = (GLfloat)cos(b);
			GLfloat r = MinorRadius * c + MajorRadius;
			GLfloat z = MinorRadius * (GLfloat)sin(b);

			// First point
			glTexCoord2f((GLfloat)(i) / (GLfloat)(NumMajor), (GLfloat)(j) / (GLfloat)(NumMinor));
			vNormal[0] = x0 * c;
			vNormal[1] = y0 * c;
			vNormal[2] = z / MinorRadius;
			m3dNormalizeVector(vNormal);
			glNormal3fv(vNormal);
			glVertex3f(x0 * r, y0 * r, z);

			glTexCoord2f((GLfloat)(i + 1) / (GLfloat)(NumMajor), (GLfloat)(j) / (GLfloat)(NumMinor));
			vNormal[0] = x1 * c;
			vNormal[1] = y1 * c;
			vNormal[2] = z / MinorRadius;
			m3dNormalizeVector(vNormal);
			glNormal3fv(vNormal);
			glVertex3f(x1 * r, y1 * r, z);
		}
		glEnd();
	}
}

void DrawGrid(GLdouble FloorExtent, GLdouble StepSize)
{
	// ground grid
	GLdouble fExtent = FloorExtent;
	GLdouble fStep = StepSize;
	GLdouble y = 0.0f;
	GLdouble iLine;
	glBegin(GL_LINES);
	glNormal3f(0.0f, 1.0f, 0.0f); //all point up
	for (iLine = -fExtent; iLine <= fExtent; iLine += fStep)
	{
		glVertex3d(iLine, y, fExtent);    // Draw Z lines
		glVertex3d(iLine, y, -fExtent);

		glVertex3d(fExtent, y, iLine);
		glVertex3d(-fExtent, y, iLine);
	}
	glEnd();
}

void DrawGround(GLdouble FloorExtent, GLdouble StepSize)
{
	GLdouble fExtent = FloorExtent;
	GLdouble fStep = StepSize;
	GLdouble y = 0.0f;
	GLdouble fStrip, fRun;

	for (fStrip = -fExtent; fStrip <= fExtent; fStrip += fStep)
	{
		glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(0.0f, 1.0f, 0.0f); //all point up

		for (fRun = fExtent; fRun >= -fExtent; fRun -= fStep)
		{
			glVertex3d(fStrip, y, fRun);
			glVertex3d(fStrip + fStep, y, fRun);
		}
		glEnd();
	}
}

void DrawOriginAxes(GLfloat Length, GLfloat LineWidth)
{
	glLineWidth(LineWidth);

	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(Length, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, Length, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, Length);
	glEnd();

	glLineWidth(1.0);
}
///////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/// Drawing of Gurukul starts here
void drawDoor(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);
	void drawWindowGlassFrame(GLfloat, GLfloat, GLfloat);

	glPushMatrix();
		glColor3f(0.367059f, 0.254706f, 0.204706f);
		drawWindowBorder(10.0f, 5.0f, 0.55f, 0.5f);

		glPushMatrix();                        //Right Door

			glTranslatef(4.5f, 0.0f, 0.0f);
			glRotatef(-gfDoorAngle, 0.0f, 1.0f, 0.0f);
			glTranslatef(-2.25f, 0.0f, 0.0f);
			glColor3f(0.477059f, 0.355706f, 0.304706f);
			drawWindowBorder(9.5f, 2.5f, 0.4f, 0.5f);

			glPushMatrix();
				glTranslatef(0.0f, 4.0f, 0.0f);
				drawWindowGlassFrame(5.0f, 2.0f, 0.3f);          //Refer Frame Code
			glPopMatrix();

			glPushMatrix();

				glTranslatef(0.0f, -2.0f, 0.0f);                   ///////
				glColor3f(0.367059f, 0.254706f, 0.204706f);
				drawCube(2.0f, 4.0f, 0.7f);

				glPushMatrix();

					glColor3f(0.8f, 0.49f, 0.15f);
					glTranslatef(-0.1f, 0.15f, 0.6f);
					glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
					drawWindowBorder(0.25f, 0.25f, 0.05f, 0.1f);

					glPushMatrix();
						glTranslatef(0.0f, 0.0f, -2.0f);
						glColor3f(0.75f, 0.75f, 0.75f);
						quadric = gluNewQuadric();
						gluCylinder(quadric, 0.15f, 0.15f, 2.5f, 26, 13);
						glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
						glTranslatef(1.5f, -0.5f, 0.0f);
						drawWindowBorder(0.5f, 0.3f, 0.05f, 0.2f);
					glPopMatrix();

					glColor3f(0.8f, 0.49f, 0.15f);
					glTranslatef(0.0f, 0.0f, -1.5f);
					drawWindowBorder(0.25f, 0.25f, 0.05f, 0.1f);
					glTranslatef(0.0f, -0.5f, 0.3f);
					drawWindowBorder(0.25f, 0.25f, 0.05f, 0.1f);

					glPopMatrix();

				glPopMatrix();

				glPushMatrix();
					glTranslatef(0.0f, -6.0f, 0.0f);
					glColor3f(1.0f, 1.0f, 1.0f);
					drawWindowBorder(3.0f, 2.0f, 0.2f, 0.8f);
					glColor3f(0.367059f, 0.254706f, 0.204706f);
					drawCube(5.0f, 2.5f, 0.5f);
				glPopMatrix();
		
				glPopMatrix();

				glPushMatrix();                        //Left Door

					glTranslatef(-4.5f, 0.0f, 0.0f);
					glRotatef(gfDoorAngle, 0.0f, 1.0f, 0.0f);
					glTranslatef(2.25f, 0.0f, 0.0f);
					glColor3f(0.477059f, 0.355706f, 0.304706f);
					drawWindowBorder(9.5f, 2.5f, 0.4f, 0.5f);

					glPushMatrix();
						glTranslatef(0.0f, 4.0f, 0.0f);
						drawWindowGlassFrame(5.0f, 2.0f, 0.3f);          //Refer Frame Code
					glPopMatrix();

					glPushMatrix();

						glTranslatef(0.0f, -2.0f, 0.0f);                   ///////
						glColor3f(0.367059f, 0.254706f, 0.204706f);
						drawCube(2.0f, 4.0f, 0.7f);

					glPushMatrix();

						glColor3f(0.8f, 0.49f, 0.15f);
						glTranslatef(1.5f, 0.15f, 0.6f);
						glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
						drawWindowBorder(0.25f, 0.25f, 0.05f, 0.1f);
					glPopMatrix();
			glPopMatrix();

			glPushMatrix();
				glTranslatef(0.0f, -6.0f, 0.0f);
				glColor3f(1.0f, 1.0f, 1.0f);
				drawWindowBorder(3.0f, 2.0f, 0.2f, 0.8f);
				glColor3f(0.367059f, 0.254706f, 0.204706f);
				drawCube(5.0f, 2.5f, 0.5f);
			glPopMatrix();

		glPopMatrix();

	///---------UPPER FLAP------///


		glTranslatef(0.0f, 12.0f, 0.0f);
		glColor3f(0.367059f, 0.254706f, 0.204706f);
		drawWindowBorder(2.0f, 5.0f, 0.55f, 0.5f);

		glPushMatrix();
			glTranslatef(0.0f, 2.5f, 0.0f);
			glRotatef(gfUpperFlapAngle, 1.0f, 0.0f, 0.0f);
			glTranslatef(0.0f, -2.5f, 0.0f);
			glColor3f(0.477059f, 0.355706f, 0.304706f);
			drawWindowBorder(1.5f, 4.5f, 0.4f, 0.3f);

			glPushMatrix();
				glTranslatef(-2.0f, 0.0f, 0.0f);
				drawCube(2.5f, 0.25f, 0.5f);
				glTranslatef(2.0f, 0.0f, 0.0f);
				drawCube(2.5f, 0.25f, 0.5f);
				glTranslatef(2.0f, 0.0f, 0.0f);
				drawCube(2.5f, 0.25f, 0.5f);

			glPopMatrix();

			glPushMatrix();
				glEnable(GL_POLYGON_STIPPLE);
				glPolygonStipple(stipple);
				glTranslatef(3.2f, 0.0f, 0.0f);
				//glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
				glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
				drawCube(2.2f, 2.2f, 0.3f);
				glTranslatef(-2.2f, 0.0f, 0.0f);
				//glColor4f(0.0f, 0.5f, 0.0f, 0.5f);
				glColor4f(0.0f, 0.5f, 0.0f, 1.0f);
				drawCube(2.2f, 2.0f, 0.3f);
				glTranslatef(-2.0f, 0.0f, 0.0f);
				drawCube(2.2f, 2.0f, 0.3f);
				glTranslatef(-2.2f, 0.0f, 0.0f);
				//glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
				glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
				drawCube(2.2f, 2.2f, 0.3f);
				glDisable(GL_POLYGON_STIPPLE);
			glPopMatrix();

		glPopMatrix();
	glPopMatrix();

}

void drawWindowMedium(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(10.0f, 5.0f, 0.55f, 0.5f);

	glPushMatrix();
	glTranslatef(4.5f, 0.0f, 0.0f);
	glRotatef(-gfWindowAngle, 0.0f, 1.0f, 0.0f);
	glTranslatef(-2.3f, 4.5f, 0.0f);

	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(5.0f, 2.5f, 0.25f, 0.5f);

	glPushMatrix();
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawCube(0.5f, 4.0f, 0.5f);

	glPushMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);		/////////////////////////------BLENDING of glass is here-------/////////////

	glTranslatef(0.0f, 2.5f, 0.0f);
	drawCube(4.5f, 4.5f, 0.3f);

	glPopMatrix();

	glPushMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);		/////////////////////////------BLENDING of glass is here-------/////////////

	glTranslatef(0.0f, -2.5f, 0.0f);
	drawCube(4.5f, 4.5f, 0.3f);

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-4.5f, 0.0f, 0.0f);
	glRotatef(gfWindowAngle, 0.0f, 1.0f, 0.0f);
	glTranslatef(2.3f, 4.5f, 0.0f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(5.0f, 2.5f, 0.25f, 0.5f);

	glPushMatrix();
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawCube(0.5f, 4.0f, 0.5f);

	glPushMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);		/////////////////////////------BLENDING of glass is here-------/////////////

	glTranslatef(0.0f, 2.5f, 0.0f);
	drawCube(4.5f, 4.5f, 0.3f);

	glPopMatrix();

	glPushMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);		/////////////////////////------BLENDING of glass is here-------/////////////

	glTranslatef(0.0f, -2.5f, 0.0f);
	drawCube(4.5f, 4.5f, 0.3f);

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	glTranslatef(0.0f, -1.0f, 0.0f);
	drawCube(1.0f, 10.0f, 0.5f);

	glPushMatrix();
	glTranslatef(2.3f, -4.5f, 0.0f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(4.0f, 2.3f, 0.3f, 0.8f);

	glPushMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(3.2f, 1.5f, 0.1f, 0.7f);

	glPushMatrix();
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(5.0f, 1.6f, 0.3f);

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-2.3f, -4.5f, 0.0f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(4.0f, 2.3f, 0.3f, 0.8f);

	glPushMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(3.2f, 1.5f, 0.1f, 0.7f);

	glPushMatrix();
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(5.0f, 1.6f, 0.3f);

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

}

void drawWindowGlassFrame(GLfloat frameHeight, GLfloat frameWidth, GLfloat frameDepth)
{
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(frameHeight, frameWidth, frameDepth, 0.1f);

	glLineWidth(5.0f);
	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-frameWidth, frameHeight, 0.15f);
	glVertex3f(frameWidth, 0.0f, 0.15f);
	glVertex3f(-frameWidth, -frameHeight, 0.15f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(frameWidth, frameHeight, 0.15f);
	glVertex3f(-frameWidth, 0.0f, 0.15f);
	glVertex3f(frameWidth, -frameHeight, 0.15f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-frameWidth, frameHeight, 0.0f);
	glVertex3f(frameWidth, 0.0f, 0.0f);
	glVertex3f(-frameWidth, -frameHeight, 0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(frameWidth, frameHeight, 0.0f);
	glVertex3f(-frameWidth, 0.0f, 0.0f);
	glVertex3f(frameWidth, -frameHeight, 0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-frameWidth, frameHeight, -0.15f);
	glVertex3f(frameWidth, 0.0f, -0.15f);
	glVertex3f(-frameWidth, -frameHeight, -0.15f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(frameWidth, frameHeight, -0.15f);
	glVertex3f(-frameWidth, 0.0f, -0.15f);
	glVertex3f(frameWidth, -frameHeight, -0.15f);
	glEnd();


	glEnable(GL_POLYGON_STIPPLE);
	glPolygonStipple(stipple);

	glBegin(GL_TRIANGLES);
	//glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

	glNormal3f(0.0f, 0.0f, 1.0f);                   //UPPER blue glass
	glVertex3f(-frameWidth, frameHeight, 0.1f);
	glVertex3f(frameWidth, frameHeight, 0.1f);
	glVertex3f(0.0f, frameHeight / 2, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);                             //SAME GLASS but front and back side is done here with opposite Normal axis.
	glVertex3f(-frameWidth, frameHeight, -0.1f);
	glVertex3f(0.0f, frameHeight / 2, -0.1f);
	glVertex3f(frameWidth, frameHeight, -0.1f);

	glNormal3f(0.0f, 0.0f, 1.0f);                   //BOTTOM blue glass
	glVertex3f(frameWidth, -frameHeight, 0.1f);
	glVertex3f(-frameWidth, -frameHeight, 0.1f);
	glVertex3f(0.0f, -frameHeight / 2, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);                             //SAME GLASS but front and back side is done here with opposite Normal axis.
	glVertex3f(frameWidth, -frameHeight, -0.1f);
	glVertex3f(-frameWidth, -frameHeight, -0.1f);
	glVertex3f(0.0f, -frameHeight / 2, -0.1f);


	//glColor4f(0.0f, 0.5f, 0.0f, 0.5f);
	glColor4f(0.0f, 0.5f, 0.0f, 1.0f);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(frameWidth, frameHeight, 0.1f);
	glVertex3f(0.0f, frameHeight / 2, 0.1f);
	glVertex3f(frameWidth, 0.0f, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(frameWidth, frameHeight, -0.1f);
	glVertex3f(0.0f, frameHeight / 2, -0.1f);
	glVertex3f(frameWidth, 0.0f, -0.1f);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-frameWidth, frameHeight, 0.1f);
	glVertex3f(0.0f, frameHeight / 2, 0.1f);
	glVertex3f(-frameWidth, 0.0f, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-frameWidth, frameHeight, -0.1f);
	glVertex3f(0.0f, frameHeight / 2, -0.1f);
	glVertex3f(-frameWidth, 0.0f, -0.1f);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(frameWidth, 0.0f, 0.1f);
	glVertex3f(0.0f, -frameHeight / 2, 0.1f);
	glVertex3f(frameWidth, -frameHeight, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(frameWidth, 0.0f, -0.1f);
	glVertex3f(0.0f, -frameHeight / 2, -0.1f);
	glVertex3f(frameWidth, -frameHeight, -0.1f);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-frameWidth, 0.0f, 0.1f);
	glVertex3f(0.0f, -frameHeight / 2, 0.1f);
	glVertex3f(-frameWidth, -frameHeight, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-frameWidth, 0.0f, -0.1f);
	glVertex3f(0.0f, -frameHeight / 2, -0.1f);
	glVertex3f(-frameWidth, -frameHeight, -0.1f);

	glEnd();

	glBegin(GL_QUADS);

	//glColor4f(0.9f, 0.9f, 0.9f, 0.5f);         //Middle glassyd
	glColor4f(0.9f, 0.9f, 0.9f, 1.0f);         //Middle glassyd

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, frameHeight / 2, 0.1f);
	glVertex3f(-frameWidth, 0.0f, 0.1f);
	glVertex3f(0.0f, -frameHeight / 2, 0.1f);
	glVertex3f(frameWidth, 0.0f, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(0.0f, frameHeight / 2, -0.1f);
	glVertex3f(-frameWidth, 0.0f, -0.1f);
	glVertex3f(0.0f, -frameHeight / 2, -0.1f);
	glVertex3f(frameWidth, 0.0f, -0.1f);

	glEnd();
	glDisable(GL_POLYGON_STIPPLE);
}

void drawWindowSmall(void)
{
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);
	void drawCube(GLfloat, GLfloat, GLfloat);

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(10.0f, 5.0f, 0.55f, 0.5f);

	glPushMatrix();
	glTranslatef(2.3f, 4.8f, 0.0f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(5.0f, 2.5f, 0.25f, 0.5f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(2.3f, 4.7f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(4.5f, 2.0f, 0.1f, 0.8f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	glTranslatef(2.3f, 4.7f, 0.0f);
	drawCube(7.3f, 2.5f, 0.3f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-2.3f, 4.8f, 0.0f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(5.0f, 2.5f, 0.25f, 0.5f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-2.3f, 4.7f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(4.5f, 2.0f, 0.1f, 0.8f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	glTranslatef(-2.3f, 4.7f, 0.0f);
	drawCube(7.3f, 2.5f, 0.3f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	glTranslatef(0.0f, -0.7f, 0.0f);
	drawCube(1.0f, 10.0f, 0.5f);
	glPopMatrix();

	///////////////////////////////////////////
	glPushMatrix();
	glTranslatef(2.3f, -5.3f, -0.3f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(4.1f, 2.25f, 0.15f, 1.0f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(2.3f, -5.2f, -0.3f);
	drawCube(6.2f, 2.5f, 0.05f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	glTranslatef(2.3f, -5.0f, -0.3f);
	drawCube(1.0f, 2.7f, 0.1f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-2.3f, -5.3f, -0.3f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(4.1f, 2.25f, 0.15f, 1.0f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(-2.3f, -5.2f, -0.3f);
	drawCube(6.2f, 2.5f, 0.05f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	glTranslatef(-2.3f, -5.0f, -0.3f);
	drawCube(1.0f, 2.7f, 0.1f);
	glPopMatrix();

	//////////////////////////////////////
	glPushMatrix();
	glColor3f(0.75f, 0.75f, 0.75f);
	glTranslatef(-3.7f, -1.2f, 0.1f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

	quadric = gluNewQuadric();
	gluCylinder(quadric, 0.1f, 0.1f, 8.0f, 26, 13);

	glTranslatef(1.5f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.1f, 0.1f, 8.0f, 26, 13);

	glTranslatef(1.5f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.1f, 0.1f, 8.0f, 26, 13);

	glTranslatef(1.5f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.1f, 0.1f, 8.0f, 26, 13);

	glTranslatef(1.5f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.1f, 0.1f, 8.0f, 26, 13);

	glTranslatef(1.5f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.1f, 0.1f, 8.0f, 26, 13);

	glPopMatrix();

}

void drawCube(GLfloat fCubeHeight, GLfloat fCubeWidth, GLfloat fCubeDepth)
{
	fCubeDepth = fCubeDepth / 2;
	fCubeHeight = fCubeHeight / 2;
	fCubeWidth = fCubeWidth / 2;
	glBegin(GL_QUADS);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(fCubeWidth, fCubeHeight, fCubeDepth);
	glVertex3f(-fCubeWidth, fCubeHeight, fCubeDepth);
	glVertex3f(-fCubeWidth, -fCubeHeight, fCubeDepth);
	glVertex3f(fCubeWidth, -fCubeHeight, fCubeDepth);

	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(fCubeWidth, fCubeHeight, -fCubeDepth);
	glVertex3f(fCubeWidth, fCubeHeight, fCubeDepth);
	glVertex3f(fCubeWidth, -fCubeHeight, fCubeDepth);
	glVertex3f(fCubeWidth, -fCubeHeight, -fCubeDepth);
	
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-fCubeWidth, fCubeHeight, -fCubeDepth);
	glVertex3f(fCubeWidth, fCubeHeight, -fCubeDepth);
	glVertex3f(fCubeWidth, -fCubeHeight, -fCubeDepth);
	glVertex3f(-fCubeWidth, -fCubeHeight, -fCubeDepth);
	

	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-fCubeWidth, fCubeHeight, fCubeDepth);
	glVertex3f(-fCubeWidth, fCubeHeight, -fCubeDepth);
	glVertex3f(-fCubeWidth, -fCubeHeight, -fCubeDepth);
	glVertex3f(-fCubeWidth, -fCubeHeight, fCubeDepth);
	
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(fCubeWidth, fCubeHeight, -fCubeDepth);
	glVertex3f(-fCubeWidth, fCubeHeight, -fCubeDepth);
	glVertex3f(-fCubeWidth, fCubeHeight, fCubeDepth);
	glVertex3f(fCubeWidth, fCubeHeight, fCubeDepth);
	
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(fCubeWidth, -fCubeHeight, fCubeDepth);
	glVertex3f(-fCubeWidth, -fCubeHeight, fCubeDepth);
	glVertex3f(-fCubeWidth, -fCubeHeight, -fCubeDepth);
	glVertex3f(fCubeWidth, -fCubeHeight, -fCubeDepth);
	
	glEnd();
}

void drawWindowBorder(GLfloat fWindowHeight, GLfloat fWindowWidth, GLfloat fWindowDepth, GLfloat fWindowThickness)
{

	//fWindowDepth = fWindowDepth / 2;
	//fWindowHeight = fWindowHeight / 2;
	//fWindowThickness = fWindowHeight / 2;

	glBegin(GL_QUADS);

	////////////------Left Bar------////////////
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-fWindowWidth, fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth, -fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, -fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, fWindowHeight, fWindowDepth);

	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-fWindowWidth + fWindowThickness, fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, -fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, -fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, fWindowHeight, -fWindowDepth);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-fWindowWidth, fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth, -fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, -fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, fWindowHeight, -fWindowDepth);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-fWindowWidth, fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth, -fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth, -fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth, fWindowHeight, -fWindowDepth);

	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-fWindowWidth, fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth, fWindowHeight, fWindowDepth);

	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-fWindowWidth, -fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, -fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth + fWindowThickness, -fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth, -fWindowHeight, fWindowDepth);



	///////-------LEFT DOOR-----///////



	////////////////---------------Right Bar-------/////////////////
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(fWindowWidth, fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, -fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, fWindowHeight, fWindowDepth);

	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(fWindowWidth - fWindowThickness, fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, -fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, -fWindowHeight, -fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, fWindowHeight, -fWindowDepth);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(fWindowWidth, fWindowHeight, -fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight, -fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, -fWindowHeight, -fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, fWindowHeight, -fWindowDepth);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(fWindowWidth, fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight, -fWindowDepth);
	glVertex3f(fWindowWidth, fWindowHeight, -fWindowDepth);

	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(fWindowWidth, fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, fWindowHeight, -fWindowDepth);
	glVertex3f(fWindowWidth, fWindowHeight, fWindowDepth);

	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(fWindowWidth, -fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, -fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth - fWindowThickness, -fWindowHeight, -fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight, fWindowDepth);

	/////////////////--------------TOP BAR---------//////////////
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-fWindowWidth, fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth, fWindowHeight - fWindowThickness, fWindowDepth);
	glVertex3f(fWindowWidth, fWindowHeight - fWindowThickness, fWindowDepth);
	glVertex3f(fWindowWidth, fWindowHeight, fWindowDepth);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-fWindowWidth, fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth, fWindowHeight - fWindowThickness, -fWindowDepth);
	glVertex3f(fWindowWidth, fWindowHeight - fWindowThickness, -fWindowDepth);
	glVertex3f(fWindowWidth, fWindowHeight, -fWindowDepth);

	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-fWindowWidth, fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth, fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth, fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth, fWindowHeight, -fWindowDepth);

	/////////////////--------------BOTTOM BAR---------//////////////
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-fWindowWidth, -fWindowHeight, fWindowDepth);
	glVertex3f(-fWindowWidth, -fWindowHeight + fWindowThickness, fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight + fWindowThickness, fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight, fWindowDepth);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-fWindowWidth, -fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth, -fWindowHeight + fWindowThickness, -fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight + fWindowThickness, -fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight, -fWindowDepth);

	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-fWindowWidth, -fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight, fWindowDepth);
	glVertex3f(fWindowWidth, -fWindowHeight, -fWindowDepth);
	glVertex3f(-fWindowWidth, -fWindowHeight, -fWindowDepth);


	glEnd();

}

void drawShutter(void)
{
	GLfloat n = 9.3f;
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);
	void drawCube(GLfloat, GLfloat, GLfloat);


	glPushMatrix();

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(10.0f, 7.0f, 0.55f, 0.5f);

	glPushMatrix();
	glTranslatef(0.0f, 11.5f, 0.5f);
	drawCube(3.0f, 14.0f, 2.0f);

	glPopMatrix();

	glPushMatrix();
	quadric = gluNewQuadric();

	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

	glTranslatef(0.0f, 0.0f, -6.5f);
	for (int i = 0; i <= 43; i++)
	{
		glPushMatrix();
		glTranslatef(0.0f, n, 0.0f);
		gluCylinder(quadric, 0.25f, 0.25f, 13.5f, 26, 13);
		glPopMatrix();
		n = n - 0.45f;
	}
	

	glPopMatrix();
	glPopMatrix();
}

void drawWhiteDoor(void)
{
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);
	void drawCube(GLfloat, GLfloat, GLfloat);


	glPushMatrix();

	glColor3f(0.6f, 0.6f, 0.6f);
	drawWindowBorder(10.0f, 8.0f, 0.55f, 0.5f);

	glPushMatrix();   //right Door
	glTranslatef(3.8f, 0.0f, 0.0f);
	
	glColor3f(0.5f, 0.5f, 0.5f);
	drawWindowBorder(9.6f, 4.0f, 0.4f, 0.5f);

	glPushMatrix();

	glColor3f(0.8f, 0.8f, 0.8f);
	glTranslatef(3.0f, 0.0f, 0.20f);
	drawCube(19.0f, 1.0f, 0.3f);

	for (int i = 0; i < 3; i++)
	{
		glColor3f(0.7f, 0.7f, 0.7f);
		glTranslatef(-1.0f, 0.0f, -0.20f);
		drawCube(19.0f, 1.0f, 0.3f);

		glColor3f(0.9f, 0.9f, 0.9f);
		glTranslatef(-1.0f, 0.0f,0.20f);
		drawCube(19.0f, 1.0f, 0.3f);
	}

	glPopMatrix();

	glTranslatef(0.0f, 0.0f, 0.2f);
	drawCube(0.8f, 7.2f, 0.4f);

	glPushMatrix();

	glColor3f(0.25f, 0.25f, 0.25f);

	glTranslatef(1.5f, 6.0f, 0.2f);
	drawCube(0.5f, 1.2f, 0.1f);

	glTranslatef(-3.0f, 0.0f, 0.0f);
	drawCube(0.5f, 1.2f, 0.1f);

	glTranslatef(0.0f, -12.0f, 0.0f);
	drawCube(0.5f, 1.2f, 0.1f);

	glTranslatef(3.0f, 0.0f, 0.0f);
	drawCube(0.5f, 1.2f, 0.1f);

	glPopMatrix();

	glPopMatrix();

	glPushMatrix();   //left Door
	glTranslatef(-3.8f, 0.0f, 0.0f);

	glColor3f(0.5f, 0.5f, 0.5f);
	drawWindowBorder(9.6f, 4.0f, 0.4f, 0.5f);

	glPushMatrix();

	glColor3f(0.8f, 0.8f, 0.8f);
	glTranslatef(3.0f, 0.0f, 0.20f);
	drawCube(19.0f, 1.0f, 0.3f);

	for (int i = 0; i < 3; i++)
	{
		glColor3f(0.7f, 0.7f, 0.7f);
		glTranslatef(-1.0f, 0.0f, -0.20f);
		drawCube(19.0f, 1.0f, 0.3f);

		glColor3f(0.9f, 0.9f, 0.9f);
		glTranslatef(-1.0f, 0.0f, 0.20f);
		drawCube(19.0f, 1.0f, 0.3f);
	}

	glPopMatrix();

	glTranslatef(0.0f, 0.0f, 0.2f);
	drawCube(0.8f, 7.2f, 0.4f);

	glPushMatrix();

	glColor3f(0.25f, 0.25f, 0.25f);

	glTranslatef(1.5f, 6.0f, 0.2f);
	drawCube(0.5f, 1.2f, 0.1f);

	glTranslatef(-3.0f, 0.0f, 0.0f);
	drawCube(0.5f, 1.2f, 0.1f);

	glTranslatef(0.0f, -12.0f, 0.0f);
	drawCube(0.5f, 1.2f, 0.1f);

	glTranslatef(3.0f, 0.0f, 0.0f);
	drawCube(0.5f, 1.2f, 0.1f);

	glPopMatrix();

	glPopMatrix();


	glPopMatrix();
}

void drawBookShelf(void)
{
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);
	void drawCube(GLfloat, GLfloat, GLfloat);


	glPushMatrix();

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(10.0f, 7.0f, 0.55f, 0.5f);

	drawCube(0.5f, 13.0f, 0.5f);

	glPushMatrix();

	glTranslatef(3.3f, 4.9f, 0.0f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(4.7f, 3.3f, 0.3f, 0.5f);

	glPushMatrix();
	drawCube(8.5f, 0.3f, 0.3f);

	glTranslatef(0.0f, 1.5f, 0.0f);
	drawCube(0.3f, 6.0f, 0.3f);

	glTranslatef(0.0f, -3.0f, 0.0f);
	drawCube(0.3f, 6.0f, 0.3f);
	glPopMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
	drawCube(8.5f, 5.5f, 0.2f);

	glPopMatrix();

	glPushMatrix();

	glTranslatef(-3.3f, 4.9f, 0.0f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(4.7f, 3.3f, 0.3f, 0.5f);

	glPushMatrix();
	drawCube(8.5f, 0.3f, 0.3f);

	glTranslatef(0.0f, 1.5f, 0.0f);
	drawCube(0.3f, 6.0f, 0.3f);

	glTranslatef(0.0f, -3.0f, 0.0f);
	drawCube(0.3f, 6.0f, 0.3f);
	glPopMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
	drawCube(8.5f, 5.5f, 0.2f);

	glPopMatrix();

	glPushMatrix();

	glTranslatef(-3.3f, -4.9f, 0.0f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(4.7f, 3.3f, 0.3f, 0.5f);

	glPushMatrix();
	drawCube(8.5f, 0.3f, 0.3f);

	glTranslatef(0.0f, 1.5f, 0.0f);
	drawCube(0.3f, 6.0f, 0.3f);

	glTranslatef(0.0f, -3.0f, 0.0f);
	drawCube(0.3f, 6.0f, 0.3f);
	glPopMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
	drawCube(8.5f, 5.5f, 0.2f);

	glPopMatrix();

	glPushMatrix();

	glTranslatef(3.3f, -4.9f, 0.0f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(4.7f, 3.3f, 0.3f, 0.5f);

	glPushMatrix();
	drawCube(8.5f, 0.3f, 0.3f);

	glTranslatef(0.0f, 1.5f, 0.0f);
	drawCube(0.3f, 6.0f, 0.3f);

	glTranslatef(0.0f, -3.0f, 0.0f);
	drawCube(0.3f, 6.0f, 0.3f);
	glPopMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
	drawCube(8.5f, 5.5f, 0.2f);

	glPopMatrix();



	glPopMatrix();
}

void drawWindowHall(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);
	void drawWindowGlassFrame(GLfloat, GLfloat, GLfloat);

	glPushMatrix();
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(10.0f, 7.0f, 0.6f, 0.5f);

	///////////--------SIDE COLUMNS-------////////

	glPushMatrix();

	glTranslatef(9.2f, 2.0f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(12.0f, 2.5f, 0.6f, 0.5f);

	glTranslatef(0.0f, 8.2f, 0.0f);
	drawCube(0.7f, 4.0f, 0.5f);                               

	glPushMatrix();											 //Added Door Design
	glTranslatef(0.0f, 2.0f, 0.0f);
	glColor3f(0.9f, 0.756f, 0.56f);
	
//	glColor3f(1.0f, 1.0f, 1.0f);
	drawCube(3.5f, 3.9f, 0.01f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -0.1f, 0.35f);
	glLineWidth(0.000005f);
	glScalef(1.7f, 1.5f, 1.5f);
	//Draw_Door_Design();
	glCallList(dsDoorDesign);
	glTranslatef(0.0f, 0.0f, -0.35f);
	//Draw_Door_Design();
	glCallList(dsDoorDesign);

	glPopMatrix();
	
	
	glPushMatrix();                                            // Wooden Inclined flaps of Window

	glPushMatrix();

	glPushMatrix();
	glColor3f(0.9f, 0.756f, 0.56f);
	glTranslatef(0.0f, -0.5f, 0.15f);
	glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -0.5f, 0.15f);
	drawCube(0.2f, 4.0f, 0.4f);
	glPopMatrix();

	for (int i = 0; i < 25; i++)
	{
		glTranslatef(0.0f, -0.5f, 0.0f);
		glPushMatrix();

		glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
		drawCube(0.2f, 4.0f, 0.4f);
		glPopMatrix();
	}

	glPopMatrix();
	glColor3f(0.8f, 0.656f, 0.46f);
	glTranslatef(0.0f, -6.4f, -0.3f);

	drawCube(12.5, 0.2f, 0.2f);

	glPopMatrix();



	glTranslatef(0.0f, -12.8f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(0.7f, 4.0f, 0.5f);

	glPushMatrix();

	glTranslatef(0.0f, -3.7f, 0.0f);
	glColor3f(0.467059f, 0.354706f, 0.304706f);
	drawWindowBorder(3.3f, 2.0f, 0.3f, 0.5f);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(2.9f, 1.5f, 0.1f, 0.9f);

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(4.1f, 1.6f, 0.5f);
	glPopMatrix();

	glPopMatrix();

	glPushMatrix();                             // LEFT Column

	glTranslatef(-9.2f, 2.0f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(12.0f, 2.5f, 0.6f, 0.5f);

	glTranslatef(0.0f, 8.2f, 0.0f);
	drawCube(0.7f, 4.0f, 0.5f);


	glPushMatrix();											 //Added Door Design
	glTranslatef(0.0f, 2.0f, 0.0f);
	glColor3f(0.9f, 0.756f, 0.56f);

	//	glColor3f(1.0f, 1.0f, 1.0f);
	drawCube(3.5f, 3.9f, 0.01f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -0.1f, 0.35f);
	glLineWidth(0.000005f);
	glScalef(1.7f, 1.5f, 1.5f);
	//Draw_Door_Design();
	glCallList(dsDoorDesign);
	glTranslatef(0.0f, 0.0f, -0.35f);
//	Draw_Door_Design();
	glCallList(dsDoorDesign);
	glPopMatrix();

	glPushMatrix();

	glPushMatrix();

	glPushMatrix();
	glColor3f(0.9f, 0.756f, 0.56f);
	glTranslatef(0.0f, -0.5f, 0.15f);
	glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -0.5f, 0.15f);
	drawCube(0.2f, 4.0f, 0.4f);
	glPopMatrix();

	for (int i = 0; i < 25; i++)
	{
		glTranslatef(0.0f, -0.5f, 0.0f);
		glPushMatrix();

		glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
		drawCube(0.2f, 4.0f, 0.4f);
		glPopMatrix();
	}

	glPopMatrix();
	glColor3f(0.8f, 0.656f, 0.46f);
	glTranslatef(0.0f, -6.4f, -0.3f);

	drawCube(12.5, 0.2f, 0.2f);

	glPopMatrix();



	glTranslatef(0.0f, -12.8f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(0.7f, 4.0f, 0.5f);

	glPushMatrix();

	glTranslatef(0.0f, -3.7f, 0.0f);
	glColor3f(0.467059f, 0.354706f, 0.304706f);
	drawWindowBorder(3.3f, 2.0f, 0.3f, 0.5f);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(2.9f, 1.5f, 0.1f, 0.9f);

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(4.1f, 1.6f, 0.5f);
	glPopMatrix();

	glPopMatrix();

	//////////////////////////////////////////////////////////////////////////

	glTranslatef(0.0f, -2.5f, 0.0f);
	drawCube(0.7f, 13.0f, 0.5f);

	glPushMatrix();                                                               //Bars
	glColor3f(0.75f, 0.75f, 0.75f);
	glTranslatef(-5.5f, -0.3f, 0.15f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

	quadric = gluNewQuadric();
	gluCylinder(quadric, 0.1f, 0.1f, 7.0f, 26, 13);

	for (int i = 0; i <= 10; i++)
	{
		glTranslatef(1.0f, 0.0f, 0.0f);
		gluCylinder(quadric, 0.1f, 0.1f, 7.0f, 26, 13);

	}
	glPopMatrix();

	glPushMatrix();          //Right Pane

	glColor3f(0.467059f, 0.354706f, 0.304706f);

	glTranslatef(6.5f, 0.0f, 0.0f);
	glRotatef(-gfWindowAngle, 0.0f, 1.0f, 0.0f);
	glTranslatef(-3.2f, 6.2f, 0.0f);
	drawWindowBorder(5.9f, 3.3f, 0.3f, 0.6f);

	drawWindowGlassFrame(5.3f, 2.7f, 0.2f);

	glPopMatrix();

	glPushMatrix();         //Left Pane

	glColor3f(0.467059f, 0.354706f, 0.304706f);
	glTranslatef(-6.5f, 0.0f, 0.0f);
	glRotatef(gfWindowAngle, 0.0f, 1.0f, 0.0f);
	glTranslatef(3.2f, 6.2f, 0.0f);
	drawWindowBorder(5.9f, 3.3f, 0.3f, 0.6f);

	drawWindowGlassFrame(5.3f, 2.7f, 0.2f);

	glPopMatrix();

	glPushMatrix();
	glColor3f(0.467059f, 0.354706f, 0.304706f);

	glTranslatef(6.6f, 0.0f, 0.0f);
	glRotatef(-gfWindowAngle, 0.0f, 1.0f, 0.0f);
	glTranslatef(-3.3f, -3.6f, -0.2f);
	drawWindowBorder(3.4f, 3.3f, 0.3f, 0.5f);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(3.0f, 3.0f, 0.1f, 1.0f);

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(4.0f, 4.0f, 0.3f);

	glPopMatrix();

	glPushMatrix();
	glColor3f(0.467059f, 0.354706f, 0.304706f);

	glTranslatef(-6.6f, 0.0f, 0.0f);
	glRotatef(gfWindowAngle, 0.0f, 1.0f, 0.0f);
	glTranslatef(3.3f, -3.6f, -0.2f);	
	drawWindowBorder(3.4f, 3.3f, 0.3f, 0.5f);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(3.0f, 3.0f, 0.1f, 1.0f);

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(4.0f, 4.0f, 0.3f);

	glPopMatrix();

	glPopMatrix();

	//////////////

	glTranslatef(0.0f, 12.0f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(2.0f, 7.0f, 0.55f, 0.5f);

	glPushMatrix();
	glTranslatef(0.0f, 2.5f, 0.0f);
	glRotatef(gfUpperFlapAngle, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -2.5f, 0.0f);
	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(1.6f, 6.5f, 0.25f, 0.4f);

	glPushMatrix();
	glTranslatef(-4.0f, 0.0f, 0.0f);
	drawCube(3.0f, 0.25f, 0.5f);

	for (int i = 0; i < 4; i++)
	{
		glTranslatef(2.0f, 0.0f, 0.0f);
		drawCube(3.0f, 0.25f, 0.5f);
	}

	glPopMatrix();

	glPushMatrix();
	glEnable(GL_POLYGON_STIPPLE);
	glPolygonStipple(stipple);
	glTranslatef(5.3f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.0f, 8.0f, 0.5f);
	glColor4f(0.0f, 0.0f, 8.0f, 1.0f);
	drawCube(2.5f, 2.3f, 0.4f);


	glTranslatef(-2.2f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.7f, 0.0f, 0.5f);
	glColor4f(0.0f, 0.7f, 0.0f, 1.0f);
	drawCube(2.5f, 2.0f, 0.4f);

	glTranslatef(-2.0f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.0f, 8.0f, 0.5f);
	glColor4f(0.0f, 0.0f, 8.0f, 1.0f);
	drawCube(2.5f, 2.0f, 0.4f);

	glTranslatef(-2.2f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.7f, 0.0f, 0.5f);
	glColor4f(0.0f, 0.7f, 0.0f, 1.0f);
	drawCube(2.5f, 2.0f, 0.4f);

	glTranslatef(-2.0f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.0f, 8.0f, 0.5f);
	glColor4f(0.0f, 0.0f, 8.0f, 1.0f);
	drawCube(2.5f, 2.0f, 0.4f);

	glTranslatef(-2.2f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.7f, 0.0f, 0.5f);
	glColor4f(0.0f, 0.7f, 0.0f, 1.0f);
	drawCube(2.5f, 2.3f, 0.4f);
	glDisable(GL_POLYGON_STIPPLE);
	glPopMatrix();

	glPopMatrix();

}

void MainDoorDesign(void)
{
	void DesignTrapezoid(void);
	void CornerCubeAndPyramid(void);

	glPushMatrix();
	glColor3f(0.445f, 0.171f, 0.075f);
	glScalef(0.4f, 0.8f, 0.05f);
	glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
	glCallList(dsCube);
	glPopMatrix();

	glColor3f(0.545f, 0.271f, 0.075f);
	glPushMatrix();
	glScalef(1.5f, 1.0f, 0.14f);
	glTranslatef(0.0f, -6.0f, 0.0f);
	DesignTrapezoid();
	glPopMatrix();

	glPushMatrix();
	glScalef(1.5f, 1.0f, 0.14f);
	glTranslatef(0.0f, 6.0f, 0.0f);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	DesignTrapezoid();
	glPopMatrix();

	glPushMatrix();
	glScalef(3.0f, 5.0f, 0.14f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();	// draw four corner cube structure
	glTranslatef(2.0f, 5.8f, 0.0f);
	glRotatef(-50.0f, 0.0f, 0.0f, 1.0f);
	glScalef(0.5f, 0.5f, 0.14f);
	CornerCubeAndPyramid();
	glPopMatrix();

	glPushMatrix();	// draw four corner cube structure
	glTranslatef(-2.0f, 5.8f, 0.0f);
	glRotatef(50.0f, 0.0f, 0.0f, 1.0f);
	glScalef(0.5f, 0.5f, 0.14f);
	CornerCubeAndPyramid();
	glPopMatrix();

	glPushMatrix();	// draw four corner cube structure
	glTranslatef(2.0f, -5.8f, 0.0f);
	glRotatef(-130.0f, 0.0f, 0.0f, 1.0f);
	glScalef(0.5f, 0.5f, 0.14f);
	CornerCubeAndPyramid();
	glPopMatrix();

	glPushMatrix();	// draw four corner cube structure
	glTranslatef(-2.0f, -5.8f, 0.0f);
	glRotatef(130.0f, 0.0f, 0.0f, 1.0f);
	glScalef(0.5f, 0.5f, 0.14f);
	CornerCubeAndPyramid();
	glPopMatrix();
}

void CornerCubeAndPyramid(void)
{
	glPushMatrix();
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 2.0f, 0.0f);
	M3DVector3f vNormal;
	glBegin(GL_TRIANGLES);
	{
		// front triangle
		M3DVector3f vPoints[3] = { { 0.0f, 0.7f, 1.0f },{ -1.0f, -1.0f, 1.0f },{ 1.0f, -1.0f, 1.0f } };
		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
	}
	{
		// Back triangle
		M3DVector3f vPoints[3] = { { 0.0f, 0.7f, -1.0f },{ 1.0f, -1.0f, -1.0f } ,{ -1.0f, -1.0f, -1.0f } };
		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
	}
	glEnd();

	glBegin(GL_QUADS);
	{	// Right Face
		M3DVector3f vPoints[4] = { { 0.0f, 0.7f, -1.0f },
		{ 0.0f, 0.7f, 1.0f },
		{ 1.0f, -1.0f, 1.0f },
		{ 1.0f, -1.0f, -1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}
	{	// Left Face
		M3DVector3f vPoints[4] = { { 0.0f, 0.7f, 1.0f },
		{ 0.0f, 0.7f, -1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{ -1.0f, -1.0f, 1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}
	glEnd();
	glPopMatrix();
}

void DesignTrapezoid(void)
{
	M3DVector3f vNormal;
	glBegin(GL_QUADS);
	{ // Front Face
		M3DVector3f vPoints[4] = { { 2.0f, 1.0f, 1.0f },
		{ -2.0f, 1.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Right Face
		M3DVector3f vPoints[4] = { { 2.0f, 1.0f, -1.0f },
		{ 2.0f, 1.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f },
		{ 1.0f, -1.0f, -1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Back Face
		M3DVector3f vPoints[4] = { { -2.0f, 1.0f, -1.0f },
		{ 2.0f, 1.0f, -1.0f },
		{ 1.0f, -1.0f, -1.0f },
		{ -1.0f, -1.0f, -1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Left Face
		M3DVector3f vPoints[4] = { { -2.0f, 1.0f, 1.0f },
		{ -2.0f, 1.0f, -1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{ -1.0f, -1.0f, 1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Top Face
		M3DVector3f vPoints[4] = { { 2.0f, 1.0f, -1.0f },
		{ -2.0f, 1.0f, -1.0f },
		{ -2.0f, 1.0f, 1.0f },
		{ 2.0f, 1.0f, 1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Bottom Face
		M3DVector3f vPoints[4] = { { 1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{ 1.0f, -1.0f, -1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	glEnd();
}

void drawMainDoor(void)
{
	void MainDoorDesign(void);
	void drawCube(GLfloat, GLfloat, GLfloat);
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);
	void drawMainDoorDesign(void);

	glPushMatrix();													//Right Door
	
	glTranslatef(16.2f, 0.0f, 0.0f);
	glRotatef(-gfMainDoorAngle, 0.0f, 1.0f, 0.0f);
	
	glTranslatef(-8.1f, 0.0f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(10.0f, 8.0f, 0.6f, 0.5f);

	glPushMatrix();
	glTranslatef(0.0f, 1.0f, 0.0f);
	drawCube(0.6f, 15.0f, 0.6f);

	glPushMatrix();										//Lower Right Design
	glTranslatef(3.7f, -5.5f, 0.0f);
	glColor3f(0.427059f, 0.314706f, 0.264706f);
	drawWindowBorder(5.2f, 3.8f, 0.4f, 0.5f);
	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(4.7f, 3.3f, 0.1f, 1.5f);
	glScalef(0.7f, 0.6f, 1.0f);
	MainDoorDesign();
	glPopMatrix();

	glPushMatrix();										//Lower Left Design
	glTranslatef(-3.7f, -5.5f, 0.0f);
	glColor3f(0.427059f, 0.314706f, 0.264706f);
	drawWindowBorder(5.2f, 3.8f, 0.4f, 0.5f);
	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(4.7f, 3.3f, 0.1f, 1.5f);
	glScalef(0.7f, 0.6f, 1.0f);
	MainDoorDesign();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();                                                    //Place for Baluesters
	glColor3f(0.8f, 0.8f, 0.8f);
	glTranslatef(-7.5f, 2.0f, 0.0f);
	for (GLfloat i = 0.0f; i <= 15.0f; i = i + 1.5f)
	{
		glPushMatrix();
		glTranslatef(i, 0.0f, 0.0f);
		glScalef(0.6f, 0.8f, 0.6f);
		glCallList(dsGateBaluster);
		glPopMatrix();
	}
	glPopMatrix();

	glTranslatef(-7.6f, 13.2f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(6.8f, 0.85f, 0.9f);

	glPushMatrix();

	glTranslatef(5.0f, 0.0f, 0.25f);
	glRotatef(180.0, 0.0f, 1.0f, 0.0f);
	glScalef(5.5f, 4.5f, 0.0f);
	glNormal3f(0.0f, 0.0f, 1.0f);
	drawMainDoorDesign();


	

//	glPopMatrix();


	glPopMatrix();
	glPushMatrix();
	glTranslatef(5.0f, 0.0f, -0.25f);
	glRotatef(180.0, 0.0f, 1.0f, 0.0f);
	glScalef(5.5f, 4.5f, 0.0f);
	glNormal3f(0.0f, 0.0f, -1.0f);
	drawMainDoorDesign();

	glPopMatrix();


	glPopMatrix();

	glPushMatrix();													//Left Door
	glTranslatef(-16.2f, 0.0f, 0.0f);
	glRotatef(gfMainDoorAngle, 0.0f, 1.0f, 0.0f);
	glTranslatef(8.1f, 0.0f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(10.0f, 8.0f, 0.6f, 0.5f);

	glPushMatrix();
	glTranslatef(0.0f, 1.0f, 0.0f);
	drawCube(0.6f, 15.0f, 0.6f);

	glPushMatrix();										//Lower Right Design
	glTranslatef(3.7f, -5.5f, 0.0f);
	glColor3f(0.427059f, 0.314706f, 0.264706f);
	drawWindowBorder(5.2f, 3.8f, 0.4f, 0.5f);
	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(4.7f, 3.3f, 0.1f, 1.5f);
	glScalef(0.7f, 0.6f, 1.0f);
	MainDoorDesign();
	glPopMatrix();

	glPushMatrix();										//Lower Left Design
	glTranslatef(-3.7f, -5.5f, 0.0f);
	glColor3f(0.427059f, 0.314706f, 0.264706f);
	drawWindowBorder(5.2f, 3.8f, 0.4f, 0.5f);
	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(4.7f, 3.3f, 0.1f, 1.5f);
	glScalef(0.7f, 0.6f, 1.0f);
	MainDoorDesign();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();                                                    //Place for Baluesters
	glColor3f(0.8f, 0.8f, 0.8f);
	glTranslatef(-7.5f, 2.0f, 0.0f);
	for (GLfloat i = 0.0f; i <= 15.0f; i = i + 1.5f)
	{
		glPushMatrix();
		glTranslatef(i, 0.0f, 0.0f);
		glScalef(0.6f, 0.8f, 0.6f);
		glCallList(dsGateBaluster);
		glPopMatrix();
	}
	glPopMatrix();

	glTranslatef(7.6f, 13.2f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(6.8f, 0.85f, 0.9f);
	
	glPushMatrix();
		glTranslatef(-5.0f, 0.0f, 0.25f);
		glScalef(5.5f, 4.5f, 0.0f);
		glNormal3f(0.0f, 0.0f, 1.0f);
		drawMainDoorDesign();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-5.0f, 0.0f, -0.25f);
	glScalef(5.5f, 4.5f, 0.0f);
	glNormal3f(0.0f, 0.0f, -1.0f);
	drawMainDoorDesign();
	glPopMatrix();
	
	glPopMatrix();

}

void drawMainDoorDesign(void)
{
	glTranslatef(0, 0, -3);
	glTranslatef(0, -0.5, 0);
	glLineWidth(2);							//2
	glPushMatrix();

	/*Draw Parabola on Top*/
	glPopMatrix();
	glPushMatrix();

	glColor3f(192.0f / 255.0f, 192.0f / 255.0f, 192.0f / 255.0f);
	glTranslatef(0.8f, 1.5f, 0.0f);
	Draw_Parabola(0.22f);


	glLineWidth(4);					//4
	Draw_V_Lines();

	glPopMatrix();
	glPushMatrix();

	glTranslatef(-0.54f, 0.23f, 0.0f);
	glScalef(0.16f, 0.16f, 1.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	Draw_Spiral(0.15f, 0.2f, 4);

	glTranslatef(5.37f, 0.0f, 0.0f);//0.68
	Draw_Spiral(0.13f, 0.15f, 7);//0.13,0.2,5  0 -0.05 2

	glPopMatrix();
	glPushMatrix();
	glColor3f(139.0f / 255.0f, 69.0f / 255.0f, 19.0f / 255.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	glTranslatef(0.38f, -0.21f, 0.0f);//0.42
	Draw_Door_Arch(0.53f, -2.33f, 0.01f);//-1.84,-0.01

	glScalef(0.25f, 0.25f, 1.0f);
	glTranslatef(1.64f, 3.98f, 0.0f);
	glLineWidth(5.0f);											//9
	Draw_Door_Arch(2.22f, -1.52f, -0.09f);//-1.84,-0.01

	glPopMatrix();
	glPushMatrix();

	glLineWidth(5.0f);											//8
	/*Draw Horizontal Line*/
	Draw_Line(-1.95f, 0.8f, -0.16f, -0.16f);

	/*Draw Vertical Line*/
	Draw_Line(0.8f, 0.8f, 1.05f, -0.17f);


	glPopMatrix();

}

void Draw_Spiral(float radius_spiral, float b_spiral, int no_of_rot)
{
	glLineWidth(5);										//8
	glBegin(GL_LINES);
	for (float angle = 0.0f; angle <= no_of_rot * M_PI; angle = angle + 0.01f)
	{
		glVertex2f(radius_spiral * exp(b_spiral * angle) * cos(angle), radius_spiral * exp(b_spiral * angle) * sin(angle));
		glVertex2f(radius_spiral * exp(b_spiral * (angle + 0.01f)) * cos(angle + 0.01f), radius_spiral * exp(b_spiral * (angle + 0.01f)) * sin(angle + 0.01f));
	}

	glEnd();



}

void Draw_Door_Arch(float b, float x_1, float x_2)
{
	//float y_arch, y_arch_2, y_arch_3;
	float y_arch, y_arch_2;
	// y=1/(1+e^(-x)). 

	glBegin(GL_LINE_STRIP);
	for (float x_arch = x_1; x_arch < x_2; x_arch = x_arch + 0.1f)
	{
		y_arch = exp(-x_arch * x_arch * b);//0.87
		y_arch_2 = exp(-(x_arch + 0.1f) * (x_arch + 0.1f) * b);
		glVertex3f(x_arch, y_arch, 0.0f);
		glVertex3f(x_arch + 0.1f, y_arch_2, 0.0f);

	}
	glEnd();


}

void Draw_Lines_Parabola(void)
{
	glBegin(GL_LINES); //-2.73 -0.09
	glVertex3f(-2.70f, 1.53f, 0.0f);
	glVertex3f(-2.70f, -0.09f, 0.0f);
	glEnd();
}

void Draw_Line(float x1, float x2, float y1, float y2)
{
	glBegin(GL_LINES);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y2, 0);
	glEnd();
}

void Draw_Parabola(float radius_para)
{
	glBegin(GL_LINE_STRIP);
	for (float x = -2.73f; x < 0.01f; x = x + 0.01f)
	{
		glVertex3f(x, -radius_para * x * x, 0.0f);
	}
	glEnd();
}

void Draw_V_Lines(void)
{
	/*Draw Vertical Line*/
	/*
	-2.34	-1.57	-1.16
	-1.95	-1.41	-0.81
	-1.56	-1.21	-0.52
	-1.17	-0.97	-0.29
	-0.78	-0.77	-0.14
	-0.39	-0.71	-0.04
	0	-0.45	-0.01
	*/
	Draw_Line(-2.34f, -2.34f, -1.57f, -1.21f);
	Draw_Line(-1.95f, -1.95f, -1.41f, -0.84f);
	Draw_Line(-1.56f, -1.56f, -1.21f, -0.54f);
	Draw_Line(-1.17f, -1.17f, -0.97f, -0.29f);
	Draw_Line(-0.78f, -0.78f, -0.77f, -0.14f);
	Draw_Line(-0.39f, -0.39f, -0.71f, -0.04f);
	Draw_Line(0.0f, 0.0f, -0.45f, -0.01f);
}

void drawFloor(void)
{
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);
	void drawCube(GLfloat, GLfloat, GLfloat);

	glPushMatrix();

	glRotatef(-45.0f, 1.0f, 0.0f,0.0f);
	glColor3f(0.5f, 0.5f, 0.5f);
	drawWindowBorder(3.5f, 3.5f, 1.0f, 0.5f);

	glColor3f(0.9f, 0.8f, 0.8f);
	drawCube(6.0f, 6.0f, 2.0f);

	quadric = gluNewQuadric();

	glTranslatef(2.8f, 2.8f, 0.0f);

	for (int i = 0; i < 20; i++)
	{
		glTranslatef(-0.17f, -0.1f, 0.0f);
		gluDisk(quadric, 0.0f, 1.0f, 13, 26);

		glPushMatrix();
		for (int j = 0; j < 20; j++)
		{
			glTranslatef(0.2f, -0.17f, 0.0f);
			gluDisk(quadric, 0.0f, 1.0f, 13, 26);
		}
		glPopMatrix();

	}

	glPopMatrix();
}

void Draw_Four_Circles(void)
{
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	//glBegin(GL_LINE_LOOP);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 32; i++)
	{
		glVertex3fv(FourCircleFirst[i]);
		glVertex3fv(FourCircleSecond[i]);
		glVertex3fv(FourCircleThird[i]);
		glVertex3fv(FourCircleFourth[i]);
	}
	glEnd();
}

void Draw_Three_Circles(void)
{
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 32; i++)
	{
		glVertex3fv(ThreeCirclesFirst[i]);
	}
	for (int i = 0; i < 32; i++)
	{
		glVertex3fv(ThreeCirclesSecond[i]);
	}
	for (int i = 0; i < 32; i++)
	{
		glVertex3fv(ThreeCirclesThird[i]);
	}
	glEnd();
}

void Draw_Leaf(void)
{
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 206; i += 1)
	{
		glBegin(GL_LINES);
		glVertex3fv(Leaf[i]);
		glVertex3fv(Leaf[i + 1]);
		glEnd();
	}
}

void Draw_Astroid_In_Leaf(void)
{
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 40; i += 2)
	{
		glBegin(GL_LINES);
		glVertex3fv(Astroid[i]);
		glVertex3fv(Astroid[i + 1]);
		glEnd();
	}
}

void Draw_Triangle_Below_Astroid()
{
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-0.05f, 0.14f, 0.0f);
	glVertex3f(-0.05f, -0.14f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
}

void Draw_Door_Design(void)
{
	
	glPushMatrix();

	/*Draw Four Circles in Centre*/
	Draw_Four_Circles();

	/*Draw Three Circles in First Quadrant*/
	glPopMatrix();
	glPushMatrix();
	glRotatef(-45, 0, 0, 1);
	glTranslatef((GLfloat)(RADIUS_SMALL_CIRCLE * 4 - X_OFF_THREE_CIRCLES_1_Q), (GLfloat)(RADIUS_SMALL_CIRCLE * 4 + Y_OFF_THREE_CIRCLES_2_Q), 0.0f);
	Draw_Three_Circles();
	glTranslatef(0.0f, (GLfloat)Y_OFF_3C_Outer_1Q, 0.0f);
	Draw_Three_Circles();

	/*Second Quadrant*/
	glPopMatrix();
	glPushMatrix();
	glRotatef(45, 0, 0, 1);
	glTranslatef((GLfloat)(-RADIUS_SMALL_CIRCLE * 4 + X_OFF_THREE_CIRCLES_2_Q), (GLfloat)(RADIUS_SMALL_CIRCLE * 4 + Y_OFF_THREE_CIRCLES_2_Q), 0.0f);
	Draw_Three_Circles();
	glTranslatef(0.0f, (GLfloat)Y_OFF_3C_Outer_1Q, 0.0f);
	Draw_Three_Circles();

	/*Third Quadrant*/
	glPopMatrix();
	glPushMatrix();
	glRotatef(135, 0, 0, 1);
	glTranslatef((GLfloat)(-RADIUS_SMALL_CIRCLE * 4 + X_OFF_THREE_CIRCLES_2_Q), (GLfloat)(RADIUS_SMALL_CIRCLE * 4 + Y_OFF_THREE_CIRCLES_3_Q), 0.0f);
	Draw_Three_Circles();
	glTranslatef(0.0f, (GLfloat)Y_OFF_3C_Outer_1Q, 0.0f);
	Draw_Three_Circles();

	glPopMatrix();
	glPushMatrix();
	glRotatef(225, 0, 0, 1);
	glTranslatef((GLfloat)(RADIUS_SMALL_CIRCLE * 4 + X_OFF_THREE_CIRCLES_4_Q), (GLfloat)(-RADIUS_SMALL_CIRCLE * 4 + Y_OFF_THREE_CIRCLES_4_Q), 0.0f);
	Draw_Three_Circles();
	glTranslatef(0.0f, (GLfloat)Y_OFF_3C_Outer_1Q, 0.0f);
	Draw_Three_Circles();

	/*Bottom Four circles*/
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0f, -0.80f, 0.0f);
	Draw_Four_Circles();

	/*Top Four circles*/
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0f, 0.80f, 0.0f);
	Draw_Four_Circles();

	/*Right Four circles*/
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.80f, 0.0f, 0.0f);
	Draw_Four_Circles();

	/*Left Four circles*/
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-0.80f, 0.0f, 0.0f);
	Draw_Four_Circles();


	/*======Start: First Quadrant Leaf======= */
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0f, (GLfloat)LEAF_OFFSET, 0.0f);
	glTranslatef((GLfloat)LEAF_OFFSET, 0.0f, 0.0f);
	//rotate
	glRotatef(45, 0, 0, 1);
	// Circle with lines
	glScalef(1.0f, (GLfloat)LEAF_SCALING_FACTOR, 1.0f);

	glLineWidth(1);
	Draw_Leaf();
	glTranslatef((GLfloat)(-ASTROID_OFFSET), 0.0f, 0.0f);

	Draw_Astroid_In_Leaf();

	glRotatef(-180, 1, 0, 0);
	Draw_Astroid_In_Leaf();

	glRotatef(180, 0, 1, 0);
	Draw_Astroid_In_Leaf();

	glRotatef(-180, 1, 0, 0);
	Draw_Astroid_In_Leaf();

	glTranslatef((GLfloat)(-ASTROID_OFFSET / 2.0f), 0.0f, 0.0f);
	Draw_Triangle_Below_Astroid();
	/*======End: First Quadrant Leaf======= */

	/*======Start: Second Quadrant Leaf======= */
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0f, (GLfloat)LEAF_OFFSET, 0.0f);
	glTranslatef((GLfloat)(-LEAF_OFFSET), 0.0f, 0.0f);
	glRotatef(135, 0, 0, 1);
	glScalef(1.0f, (GLfloat)LEAF_SCALING_FACTOR, 1.0f);
	glLineWidth(1);
	Draw_Leaf();

	glTranslatef((GLfloat)(-ASTROID_OFFSET), 0.0f, 0.0f);

	Draw_Astroid_In_Leaf();

	glRotatef(-180, 1, 0, 0);
	Draw_Astroid_In_Leaf();

	glRotatef(180, 0, 1, 0);
	Draw_Astroid_In_Leaf();

	glRotatef(-180, 1, 0, 0);
	Draw_Astroid_In_Leaf();

	glTranslatef((GLfloat)(-ASTROID_OFFSET / 2.0f), 0.0f, 0.0f);
	Draw_Triangle_Below_Astroid();
	/*======End: Second Quadrant Leaf======= */

	glPopMatrix();
	glPushMatrix();

	/*======Start: Third Quadrant Leaf======= */
	glTranslatef((GLfloat)(-LEAF_OFFSET), (GLfloat)(-LEAF_OFFSET), 0.0f);
	glRotatef(225, 0, 0, 1);
	glScalef(1, LEAF_SCALING_FACTOR, 1);
	glLineWidth(1);
	Draw_Leaf();
	glTranslatef((GLfloat)(-ASTROID_OFFSET), 0.0f, 0.0f);

	Draw_Astroid_In_Leaf();

	glRotatef(-180, 1, 0, 0);
	Draw_Astroid_In_Leaf();

	glRotatef(180, 0, 1, 0);
	Draw_Astroid_In_Leaf();

	glRotatef(-180, 1, 0, 0);
	Draw_Astroid_In_Leaf();

	glTranslatef((GLfloat)(-ASTROID_OFFSET / 2.0f), 0.0f, 0.0f);
	Draw_Triangle_Below_Astroid();
	/*======End: Third Quadrant Leaf======= */

	glPopMatrix();
	glPushMatrix();

	/*======Start: Fourth Quadrant Leaf======= */
	glTranslatef((GLfloat)LEAF_OFFSET, (GLfloat)(-LEAF_OFFSET), 0.0f);
	glRotatef(315, 0, 0, 1);
	glScalef(1, LEAF_SCALING_FACTOR, 1);
	glLineWidth(1);
	Draw_Leaf();

	glTranslatef((GLfloat)(-ASTROID_OFFSET), 0.0f, 0.0f);

	Draw_Astroid_In_Leaf();

	glRotatef(-180, 1, 0, 0);
	Draw_Astroid_In_Leaf();

	glRotatef(180, 0, 1, 0);
	Draw_Astroid_In_Leaf();

	glRotatef(-180, 1, 0, 0);
	Draw_Astroid_In_Leaf();

	glTranslatef((GLfloat)(-ASTROID_OFFSET / 2.0f), 0.0f, 0.0f);
	Draw_Triangle_Below_Astroid();
	/*======End: Fourth Quadrant Leaf======= */
	//glFlush();
	glPopMatrix();
	
}

void drawPillarDesign(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);
	void drawWindowCube(GLfloat, GLfloat, GLfloat, GLfloat);

	glPushMatrix();

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(10.0f, 2.0f, 0.55f, 0.5f);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawCube(19.0f, 3.0f, 0.2f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 8.5f, 0.15f);
	glScalef(1.7f, 1.2f, 2.0f);
	
	for (int i = 0; i <= 7; i++)
	{
		//Draw_Door_Design();
		glCallList(dsDoorDesign);
		glTranslatef(0.0f, -2.0f, 0.0f);
	}

	glPopMatrix();
}

void drawChair(void)
{
	glEnable(GL_MAP2_VERTEX_3);
	glEnable(GL_AUTO_NORMAL);

	if (gbChairShow == true)
	{
		
		if (gfChairAlpha <= 1.0f)
		{			
			gfChairAlpha = gfChairAlpha + 0.005f;
		}
	}
	else
	{
		gfChairAlpha = 0.0f;
	}
	
	//glColor4f(0.15f, 0.25555f, 0.15f, gfChairAlpha);
	glColor4f(0.184f, 0.310f, 0.310f, gfChairAlpha);

	glPushMatrix();
	quadric = gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_OUTSIDE);

	glPushMatrix();                                        //Chair front part
	glTranslatef(-5.0f, 0.0f, 0.0f);
	glRotatef(65.0f, 1.0f, 0.0f, 0.0f);
	
	glPushMatrix();										//Back Rest
	
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(4.0f, -3.3f, 0.0f);
		glScalef(4.0f, 3.5f, 2.0f);
		glMap2f(GL_MAP2_VERTEX_3, 0.0f, 10.0f, 3, nNumPoints, 0.0f, 10.0f, 9, nNumPoints, &ctrlPoints1[0][0][0]);
		glMapGrid2f(10, 0.0f, 10.0f, 10, 0.0f, 10.0f);
		glEvalMesh2(GL_FILL, 0, 10, 0, 10);

	glPopMatrix();
	
	glPushMatrix();
	
	glTranslatef(0.0f, 0.0f, 13.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	gluCylinder(quadric, 0.2f, 0.2f, 8.0f, 13, 26);
	glPopMatrix();

	

	glTranslatef(0.0f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.2f, 0.2f, 15.0f, 13, 26);

	glTranslatef(8.0f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.2f, 0.2f, 15.0f, 13, 26);

	
	glPopMatrix();

	glPushMatrix();                                        //Chair back part
	glTranslatef(-5.0f, -13.0f, -2.0f);
	glRotatef(-60.0f, 1.0f, 0.0f, 0.0f);

	glPushMatrix();

	glTranslatef(0.0f, 0.0f, 2.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	gluCylinder(quadric, 0.2f, 0.2f, 8.0f, 13, 26);
	glPopMatrix();

	glTranslatef(0.0f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.2f, 0.2f, 9.0f, 13, 26);

	glTranslatef(8.0f, .0f, 0.0f);
	gluCylinder(quadric, 0.2f, 0.2f, 9.0f, 13, 26);


	glPopMatrix();
	glPopMatrix();
	
	glPushMatrix();

	glMap2f(GL_MAP2_VERTEX_3, 0.0f, 10.0f, 3, nNumPoints, 0.0f, 10.0f, 9, nNumPoints, &ctrlPoints[0][0][0]);
		glMapGrid2f(10, 0.0f, 10.0f, 10, 0.0f, 10.0f);

	glTranslatef(-1.0f, -7.0f, 4.0f);

	glPushMatrix();
	glScalef(0.6f, 0.25f, 0.7f);
	glEvalMesh2(GL_FILL, 0, 10, 0, 10);

	glPopMatrix();

	glTranslatef(0.0f, -0.25f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	drawWindowBorder(4.2f, 3.8f, 0.3f, 0.3f);


	glPopMatrix();
}

void drawFan(void)
{
	quadric = gluNewQuadric();
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	glEnable(GL_AUTO_NORMAL);

	glPushMatrix();
	{
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		gluCylinder(quadric, 2.5f, 1.0f, 2.5f, 13, 26);

	
		glTranslatef(0.0f, 0.0f, 2.5f);

		gluCylinder(quadric, 1.0f, 0.7f, 1.5f, 50, 50);

		glTranslatef(0.0f, 0.0f, 1.5f);
	
		gluCylinder(quadric, 0.5f, 0.5f, 15.0f, 50, 50);

		glTranslatef(0.0f, 0.0f, 15.0f);
		gluCylinder(quadric, 0.5f, 3.5f, 2.5f, 50, 50);

		glPushMatrix();
		{
			glRotatef(gfFanAngle, 0.0f, 0.0f, 1.0f);
			glTranslatef(0.0f, 0.0f, 2.5f);
			gluDisk(quadric, 3.5f, 5.0f, 50, 50);

			glPushMatrix();                                              //Same Disk for Normal Adjustment
			{	
				glTranslatef(0.0f, 0.0f, -0.01f);
				glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
				gluDisk(quadric, 3.5f, 5.0f, 50, 50);
			}
			glPopMatrix();

			gluCylinder(quadric, 5.0f, 5.5f, 2.0f, 50, 50);
			glPushMatrix();
			{


				glPushMatrix();												//Rotating Cubes
				{
					glPushMatrix();										// first Wing
					{
						glTranslatef(7.0f, 0.0f, 2.0f);
						drawCube(1.0f, 3.2f, 0.5f);

						glTranslatef(9.5f, 0.0f, 0.0f);
						drawCube(4.0f, 17.0f, 0.1f);
					}
					glPopMatrix();
					glPushMatrix();										// second Wing
					{
						glRotatef(120.0f, 0.0f, 0.0f, 1.0f);
						glTranslatef(7.0f, 0.0f, 2.0f);
						drawCube(1.0f, 3.2f, 0.5f);

						glTranslatef(9.5f, 0.0f, 0.0f);
						drawCube(4.0f, 17.0f, 0.1f);
					}
					glPopMatrix();
					glPushMatrix();										// third Wing
					{
						glRotatef(240.0f, 0.0f, 0.0f, 1.0f);
						glTranslatef(7.0f, 0.0f, 2.0f);
						drawCube(1.0f, 3.2f, 0.5f);

						glTranslatef(9.5f, 0.0f, 0.0f);
						drawCube(4.0f, 17.0f, 0.1f);
					}
					glPopMatrix();
				}
				glPopMatrix();

				glTranslatef(0.0f, 0.0f, 2.0f);
				gluCylinder(quadric, 5.5f, 5.0f, 1.0f, 50, 50);
				glTranslatef(0.0f, 0.0f, 1.0f);
				gluDisk(quadric, 0.0f, 5.0f, 50, 50);
				glPushMatrix();                                              //Same Disk for Normal Adjustment
				{
					glTranslatef(0.0f, 0.0f, -0.01f);
					glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
					gluDisk(quadric, 0.0f, 5.0f, 50, 50);
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
	glPopMatrix();

}

void drawWindowOutside(void)
{
	glPushMatrix();
	{
		glColor3f(0.367059f, 0.254706f, 0.204706f);
		drawWindowBorder(5.0f, 5.0f, 0.5f, 1.0f);

		drawCube(1.0f, 8.5f, 0.5f);
		drawCube(8.5f, 1.0f, 0.5f);

		glPushMatrix();
		{
			glTranslatef(2.25f, 2.25f, 0.0f);
			glColor3f(1.0f, 1.0f, 1.0f);
			drawCube(3.7f, 3.7f, 0.3f);
		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(-2.25f, 2.25f, 0.0f);
			glColor3f(1.0f, 1.0f, 1.0f);
			drawCube(3.7f, 3.7f, 0.3f);
		}
		glPopMatrix();
		glPushMatrix();
		{
			glTranslatef(2.25f, -2.25f, 0.0f);
			glColor3f(1.0f, 1.0f, 1.0f);
			drawCube(3.7f, 3.7f, 0.3f);
		}
		glPopMatrix();
		glPushMatrix();
		{
			glTranslatef(-2.25f, -2.25f, 0.0f);
			glColor3f(1.0f, 1.0f, 1.0f);
			drawCube(3.7f, 3.7f, 0.3f);
		}
		glPopMatrix();

	}
	glPopMatrix();
}

void drawWindowLibrary(void)
{
	void drawWindowPlaneGlassFrame(GLfloat, GLfloat, GLfloat);
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);

	glPushMatrix();
		glColor3f(0.367059f, 0.254706f, 0.204706f);
		drawWindowBorder(6.2f, 6.7f, 0.6f, 0.5f);
		
		glPushMatrix();          //Right Pane

			glColor3f(0.467059f, 0.354706f, 0.304706f);

			glTranslatef(-3.0f, 0.0f, 0.0f);
			drawWindowBorder(5.9f, 3.3f, 0.3f, 0.6f);

			drawWindowPlaneGlassFrame(5.3f, 2.7f, 0.2f);

		glPopMatrix();

		glPushMatrix();         //Left Pane

			glColor3f(0.467059f, 0.354706f, 0.304706f);
			glTranslatef(3.0f, 0.0f, 0.0f);

			drawWindowBorder(5.9f, 3.3f, 0.3f, 0.6f);

			drawWindowPlaneGlassFrame(5.3f, 2.7f, 0.2f);

		glPopMatrix();
	glPopMatrix();
}

void drawWindowHallBackside(void)
{
	glPushMatrix();
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(10.0f, 7.0f, 0.6f, 0.5f);

	glPushMatrix();										// BlackBoard
	{
		glPushMatrix();
		{
			glColor3f(0.8f, 0.8f, 0.8f);
			glTranslatef(5.0f, -3.0f, -2.0f);

			drawCube(0.3f, 0.3f, 3.0f);
		}
		glPopMatrix();

		glPushMatrix();
		{
			glColor3f(0.8f, 0.8f, 0.8f);
			glTranslatef(-5.0f, -3.0f, -2.0f);

			drawCube(0.3f, 0.3f, 3.0f);
		}
		glPopMatrix();

		glColor3f(0.2f, 0.2f, 0.2f);
		glTranslatef(0.0f, 3.55f, -3.0f);
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		glScalef(0.5f, 1.0f, 0.3f);
		//drawCube(12.5f, 20.0f, 0.2f);	// blackboard
		glCallList(dsBlackboard);
	}
	glPopMatrix();

	///////////--------SIDE COLUMNS-------////////

	glPushMatrix();

	glTranslatef(9.2f, 2.0f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(12.0f, 2.5f, 0.6f, 0.5f);

	glTranslatef(0.0f, 8.2f, 0.0f);
	drawCube(0.7f, 4.0f, 0.5f);

	glPushMatrix();											 //Added Door Design
	glTranslatef(0.0f, 2.0f, 0.0f);
	glColor3f(0.9f, 0.756f, 0.56f);

	//	glColor3f(1.0f, 1.0f, 1.0f);
	drawCube(3.5f, 3.9f, 0.01f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -0.1f, 0.35f);
	glLineWidth(0.000005f);
	glScalef(1.7f, 1.5f, 1.5f);
	//Draw_Door_Design();
	glCallList(dsDoorDesign);
	glTranslatef(0.0f, 0.0f, -0.35f);
	//Draw_Door_Design();
	glCallList(dsDoorDesign);

	glPopMatrix();


	glPushMatrix();                                            // Wooden Inclined flaps of Window

	glPushMatrix();

	glPushMatrix();
	glColor3f(0.9f, 0.756f, 0.56f);
	glTranslatef(0.0f, -0.5f, 0.15f);
	glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -0.5f, 0.15f);
	drawCube(0.2f, 4.0f, 0.4f);
	glPopMatrix();

	for (int i = 0; i < 25; i++)
	{
		glTranslatef(0.0f, -0.5f, 0.0f);
		glPushMatrix();

		glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
		drawCube(0.2f, 4.0f, 0.4f);
		glPopMatrix();
	}

	glPopMatrix();
	glColor3f(0.8f, 0.656f, 0.46f);
	glTranslatef(0.0f, -6.4f, -0.3f);

	drawCube(12.5, 0.2f, 0.2f);

	glPopMatrix();



	glTranslatef(0.0f, -12.8f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(0.7f, 4.0f, 0.5f);

	glPushMatrix();

	glTranslatef(0.0f, -3.6f, 0.0f);
	glColor3f(0.467059f, 0.354706f, 0.304706f);
	drawWindowBorder(3.3f, 2.0f, 0.3f, 0.5f);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(2.8f, 1.5f, 0.1f, 0.7f);

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(4.1f, 1.6f, 0.5f);
	glPopMatrix();

	glPopMatrix();

	glPushMatrix();                             // LEFT Column

	glTranslatef(-9.2f, 2.0f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(12.0f, 2.5f, 0.6f, 0.5f);

	glTranslatef(0.0f, 8.2f, 0.0f);
	drawCube(0.7f, 4.0f, 0.5f);


	glPushMatrix();											 //Added Door Design
	glTranslatef(0.0f, 2.0f, 0.0f);
	glColor3f(0.9f, 0.756f, 0.56f);

	//	glColor3f(1.0f, 1.0f, 1.0f);
	drawCube(3.5f, 3.9f, 0.01f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -0.1f, 0.35f);
	glLineWidth(0.000005f);
	glScalef(1.7f, 1.5f, 1.5f);
	//Draw_Door_Design();
	glCallList(dsDoorDesign);
	glTranslatef(0.0f, 0.0f, -0.35f);
	//	Draw_Door_Design();
	glCallList(dsDoorDesign);
	glPopMatrix();

	glPushMatrix();

	glPushMatrix();

	glPushMatrix();
	glColor3f(0.9f, 0.756f, 0.56f);
	glTranslatef(0.0f, -0.5f, 0.15f);
	glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -0.5f, 0.15f);
	drawCube(0.2f, 4.0f, 0.4f);
	glPopMatrix();

	for (int i = 0; i < 25; i++)
	{
		glTranslatef(0.0f, -0.5f, 0.0f);
		glPushMatrix();

		glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
		drawCube(0.2f, 4.0f, 0.4f);
		glPopMatrix();
	}

	glPopMatrix();
	glColor3f(0.8f, 0.656f, 0.46f);
	glTranslatef(0.0f, -6.4f, -0.3f);

	drawCube(12.5, 0.2f, 0.2f);

	glPopMatrix();



	glTranslatef(0.0f, -12.8f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(0.7f, 4.0f, 0.5f);

	glPushMatrix();

	glTranslatef(0.0f, -3.6f, 0.0f);
	glColor3f(0.467059f, 0.354706f, 0.304706f);
	drawWindowBorder(3.3f, 2.0f, 0.3f, 0.5f);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(2.8f, 1.5f, 0.1f, 0.7f);

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(4.1f, 1.6f, 0.5f);
	glPopMatrix();

	glPopMatrix();

	//////////////////////////////////////////////////////////////////////////

	glTranslatef(0.0f, -2.5f, 0.0f);
	drawCube(0.7f, 13.0f, 0.5f);

	glPushMatrix();                                                               //Bars
	glColor3f(0.75f, 0.75f, 0.75f);
	glTranslatef(-5.5f, -0.3f, 0.15f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

	quadric = gluNewQuadric();
	gluCylinder(quadric, 0.1f, 0.1f, 7.0f, 26, 13);

	for (int i = 0; i <= 10; i++)
	{
		glTranslatef(1.0f, 0.0f, 0.0f);
		gluCylinder(quadric, 0.1f, 0.1f, 7.0f, 26, 13);

	}
	glPopMatrix();

	glPushMatrix();          //Right Pane

	glColor3f(0.467059f, 0.354706f, 0.304706f);

	glTranslatef(3.2f, 6.2f, 0.0f);
	drawWindowBorder(5.9f, 3.3f, 0.3f, 0.6f);

	drawWindowGlassFrame(5.3f, 2.7f, 0.2f);

	glPopMatrix();

	glPushMatrix();         //Left Pane

	glColor3f(0.467059f, 0.354706f, 0.304706f);

	glTranslatef(-3.2f, 6.2f, 0.0f);
	drawWindowBorder(5.9f, 3.3f, 0.3f, 0.6f);

	drawWindowGlassFrame(5.3f, 2.7f, 0.2f);

	glPopMatrix();

	glPushMatrix();
	glColor3f(0.467059f, 0.354706f, 0.304706f);


	glTranslatef(3.3f, -3.6f, -0.2f);
	drawWindowBorder(3.4f, 3.3f, 0.3f, 0.5f);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(3.0f, 3.0f, 0.1f, 1.0f);

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(4.0f, 4.0f, 0.3f);

	glPopMatrix();

	glPushMatrix();
	glColor3f(0.467059f, 0.354706f, 0.304706f);


	glTranslatef(-3.3f, -3.6f, -0.2f);	
	drawWindowBorder(3.4f, 3.3f, 0.3f, 0.5f);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(3.0f, 3.0f, 0.1f, 1.0f);

	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawCube(4.0f, 4.0f, 0.3f);

	glPopMatrix();

	glPopMatrix();

	//////////////

	glTranslatef(0.0f, 12.0f, 0.0f);
	glColor3f(0.367059f, 0.254706f, 0.204706f);
	drawWindowBorder(2.0f, 7.0f, 0.55f, 0.5f);

	glPushMatrix();

	glColor3f(0.477059f, 0.355706f, 0.304706f);
	drawWindowBorder(1.6f, 6.5f, 0.25f, 0.4f);

	glPushMatrix();
	glTranslatef(-4.0f, 0.0f, 0.0f);
	drawCube(3.0f, 0.25f, 0.5f);

	for (int i = 0; i < 4; i++)
	{
		glTranslatef(2.0f, 0.0f, 0.0f);
		drawCube(3.0f, 0.25f, 0.5f);
	}

	glPopMatrix();

	glPushMatrix();
	glEnable(GL_POLYGON_STIPPLE);
	glPolygonStipple(stipple);
	glTranslatef(5.3f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.0f, 8.0f, 0.5f);
	glColor4f(0.0f, 0.0f, 8.0f, 1.0f);
	drawCube(2.5f, 2.3f, 0.4f);


	glTranslatef(-2.2f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.7f, 0.0f, 0.5f);
	glColor4f(0.0f, 0.7f, 0.0f, 1.0f);
	drawCube(2.5f, 2.0f, 0.4f);

	glTranslatef(-2.0f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.0f, 8.0f, 0.5f);
	glColor4f(0.0f, 0.0f, 8.0f, 1.0f);
	drawCube(2.5f, 2.0f, 0.4f);

	glTranslatef(-2.2f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.7f, 0.0f, 0.5f);
	glColor4f(0.0f, 0.7f, 0.0f, 1.0f);
	drawCube(2.5f, 2.0f, 0.4f);

	glTranslatef(-2.0f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.0f, 8.0f, 0.5f);
	glColor4f(0.0f, 0.0f, 8.0f, 1.0f);
	drawCube(2.5f, 2.0f, 0.4f);

	glTranslatef(-2.2f, 0.0f, 0.0f);
	//glColor4f(0.0f, 0.7f, 0.0f, 0.5f);
	glColor4f(0.0f, 0.7f, 0.0f, 1.0f);
	drawCube(2.5f, 2.3f, 0.4f);
	glDisable(GL_POLYGON_STIPPLE);
	glPopMatrix();

	glPopMatrix();

}

void drawDoorStairs(void)
{
	glPushMatrix();
	{
		glColor3f(0.367059f, 0.254706f, 0.204706f);
		drawWindowBorder(10.0f, 5.0f, 0.55f, 0.5f);

		glPushMatrix();                        //Right Door
		{
			glTranslatef(2.25f, 0.0f, 0.0f);
			glColor3f(0.477059f, 0.355706f, 0.304706f);
			drawWindowBorder(9.7f, 2.5f, 0.4f, 0.5f);

			drawCube(0.5f, 4.3f, 0.5f);

			glPushMatrix();
			{
				glTranslatef(0.0f, 4.75f, 0.0f);
				glColor3f(1.0f, 1.0, 1.0f);
				drawWindowBorder(4.55f, 2.0f, 0.3f, 0.4f);

				glColor3f(0.367059f, 0.254706f, 0.204706f);
				drawCube(8.5f, 3.5f, 0.5f);
			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(0.0f, -4.75f, 0.0f);
				glColor3f(1.0f, 1.0, 1.0f);
				drawWindowBorder(4.55f, 2.0f, 0.3f, 0.4f);

				glColor3f(0.367059f, 0.254706f, 0.204706f);
				drawCube(8.5f, 3.5f, 0.5f);
			}
			glPopMatrix();

		}
		glPopMatrix();

		glPushMatrix();                        //Left Door
		{
			glTranslatef(-2.25f, 0.0f, 0.0f);
			glColor3f(0.477059f, 0.355706f, 0.304706f);
			drawWindowBorder(9.7f, 2.5f, 0.4f, 0.5f);

			drawCube(0.5f, 4.3f, 0.5f);

			glPushMatrix();
			{
				glTranslatef(0.0f, 4.75f, 0.0f);
				glColor3f(1.0f, 1.0, 1.0f);
				drawWindowBorder(4.55f, 2.0f, 0.3f, 0.4f);

				glColor3f(0.367059f, 0.254706f, 0.204706f);
				drawCube(8.5f, 3.5f, 0.5f);
			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(0.0f, -4.75f, 0.0f);
				glColor3f(1.0f, 1.0, 1.0f);
				drawWindowBorder(4.55f, 2.0f, 0.3f, 0.4f);

				glColor3f(0.367059f, 0.254706f, 0.204706f);
				drawCube(8.5f, 3.5f, 0.5f);
			}
			glPopMatrix();

		}
		glPopMatrix();
	}
	glPopMatrix();

}

void drawDoorSide(void)
{
	glPushMatrix();
	{
		glColor3f(0.367059f, 0.254706f, 0.204706f);
		drawWindowBorder(10.0f, 5.0f, 0.55f, 0.5f);

		glPushMatrix();                        //Right Door
		{
			glTranslatef(2.25f, 0.0f, 0.0f);
			glColor3f(0.477059f, 0.355706f, 0.304706f);
			drawWindowBorder(9.7f, 2.5f, 0.4f, 0.5f);

			glTranslatef(0.0f, -3.0f, 0.0f);
			drawCube(0.5f, 4.3f, 0.5f);

			glPushMatrix();
			{
				glColor3f(0.9f, 0.756f, 0.56f);
				glTranslatef(0.0f, 12.5f, 0.15f);

				drawCube(0.2f, 4.0f, 0.4f);
				
				for (int i = 0; i < 24; i++)
				{
					glTranslatef(0.0f, -0.5f, 0.0f);
					glPushMatrix();

					glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
					drawCube(0.2f, 4.0f, 0.4f);
					glPopMatrix();
				}

			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(0.0f, -3.2f, 0.0f);
				glColor3f(1.0f, 1.0, 1.0f);
				drawWindowBorder(3.1f, 2.0f, 0.3f, 0.5f);

				glColor3f(0.367059f, 0.254706f, 0.204706f);
				drawCube(5.5f, 3.5f, 0.5f);
			}
			glPopMatrix();

		}
		glPopMatrix();

		glPushMatrix();                        //Left Door
		{
			glTranslatef(-2.25f, 0.0f, 0.0f);
			glColor3f(0.477059f, 0.355706f, 0.304706f);
			drawWindowBorder(9.7f, 2.5f, 0.4f, 0.5f);

			glTranslatef(0.0f, -3.0f, 0.0f);
			drawCube(0.5f, 4.3f, 0.5f);

			glPushMatrix();
			{
				glColor3f(0.9f, 0.756f, 0.56f);
				glTranslatef(0.0f, 12.5f, 0.15f);

				drawCube(0.2f, 4.0f, 0.4f);

				for (int i = 0; i < 24; i++)
				{
					glTranslatef(0.0f, -0.5f, 0.0f);
					glPushMatrix();

					glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
					drawCube(0.2f, 4.0f, 0.4f);
					glPopMatrix();
				}

			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(0.0f, -3.2f, 0.0f);
				glColor3f(1.0f, 1.0, 1.0f);
				drawWindowBorder(3.1f, 2.0f, 0.3f, 0.5f);

				glColor3f(0.367059f, 0.254706f, 0.204706f);
				drawCube(5.5f, 3.5f, 0.5f);
			}
			glPopMatrix();

		}
		glPopMatrix();
	}
	glPopMatrix();

}

void drawDoorLibrary(void)
{
	glPushMatrix();
	{
		glColor3f(0.367059f, 0.254706f, 0.204706f);
		drawWindowBorder(10.0f, 5.0f, 0.55f, 0.5f);

		glPushMatrix();          //front Part
		{
			glPushMatrix();
			{
			
				glColor3f(0.75f, 0.75f, 0.75f);
				glTranslatef(-4.0f, 9.5f, 0.3f);
				glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

				quadric = gluNewQuadric();
				gluCylinder(quadric, 0.08, 0.08, 19.0, 50, 50);

				for (int i = 0; i <= 9; i++)
				{
					glTranslatef(0.8f, 0.0f, 0.0f);
					gluCylinder(quadric, 0.08, 0.08, 19.0, 50, 50);

				}
			}
			glPopMatrix();

			glTranslatef(0.0f, 0.0f, 0.3f);
			glColor3f(0.367059f, 0.254706f, 0.204706f);
			drawCube(19.0f, 0.5f, 0.5f);

			glPushMatrix();
			{
				drawCube(0.5f, 9.0f, 0.5f);
			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(0.0f, 3.3f, 0.0f);
				drawCube(0.5f, 9.0f, 0.5f);
			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(0.0f, -3.3f, 0.0f);
				drawCube(0.5f, 9.0f, 0.5f);
			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(0.0f, 6.6f, 0.0f);
				drawCube(0.5f, 9.0f, 0.5f);
			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(0.0f, -6.6f, 0.0f);
				drawCube(0.5f, 9.0f, 0.5f);
			}
			glPopMatrix();
		}
		glPopMatrix();


		glPushMatrix();															//Back Part
		{
			glTranslatef(0.0f, 0.0f, 0.0f);

			glPushMatrix();												//White Part
			{
				glTranslatef(0.0f, 1.65f, 0.0f);
				glColor3f(1.0f, 1.0f, 1.0f);
				drawCube(3.0f, 9.0f, 0.2f);
			}
			glPopMatrix();

			glPushMatrix();												
			{
				glTranslatef(0.0f, -4.95f, 0.0f);
				glColor3f(1.0f, 1.0f, 1.0f);
				drawCube(3.0f, 9.0f, 0.2f);
			}
			glPopMatrix();

			glPushMatrix();												
			{
				glTranslatef(0.0f, 8.25f, 0.0f);
				glColor3f(1.0f, 1.0f, 1.0f);
				drawCube(3.0f, 9.0f, 0.2f);
			}
			glPopMatrix();

			glPushMatrix();												//Chocolaty Part
			{
				glTranslatef(0.0f, -1.65f, 0.0f);
				glColor3f(0.467059f, 0.354706f, 0.304706f);
				drawCube(3.0f, 9.0f, 0.2f);
			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(0.0f, 4.95f, 0.0f);
				glColor3f(0.467059f, 0.354706f, 0.304706f);
				drawCube(3.0f, 9.0f, 0.2f);
			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(0.0f, -8.25f, 0.0f);
				glColor3f(0.467059f, 0.354706f, 0.304706f);
				drawCube(3.0f, 9.0f, 0.2f);
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
	glPopMatrix();
}

void drawDoorOutside(void)
{
	glPushMatrix();
	{
		glColor3f(0.367059f, 0.254706f, 0.204706f);
		drawWindowBorder(10.0f, 5.0f, 0.55f, 0.7f);

		drawCube(19.0f, 0.5f, 0.5f);

		glColor3f(1.0f, 1.0f, 1.0f);
		drawCube(19.0f, 9.5f, 0.3f);
		
		glColor3f(0.467059f, 0.354706f, 0.304706f);
		glPushMatrix();
		{
			glTranslatef(0.0f, 9.2f, 0.0f);
			drawCube(1.0f, 9.5f, 0.5f);

		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.0f, -9.2f, 0.0f);
			drawCube(1.0f, 9.5f, 0.5f);

		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.0f, 3.5f, 0.0f);
			drawCube(0.5f, 9.5f, 0.5f);

		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.0f, -3.5f, 0.0f);
			drawCube(0.5f, 9.5f, 0.5f);

		}
		glPopMatrix();

	}
	glPopMatrix();
}

void drawWindowPlaneGlassFrame(GLfloat frameHeight, GLfloat frameWidth, GLfloat frameDepth)
{
	void drawWindowBorder(GLfloat, GLfloat, GLfloat, GLfloat);

	glColor3f(1.0f, 1.0f, 1.0f);
	drawWindowBorder(frameHeight, frameWidth, frameDepth, 0.1f);

	glLineWidth(5.0f);
	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-frameWidth, frameHeight, 0.15f);
	glVertex3f(frameWidth, 0.0f, 0.15f);
	glVertex3f(-frameWidth, -frameHeight, 0.15f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(frameWidth, frameHeight, 0.15f);
	glVertex3f(-frameWidth, 0.0f, 0.15f);
	glVertex3f(frameWidth, -frameHeight, 0.15f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-frameWidth, frameHeight, 0.0f);
	glVertex3f(frameWidth, 0.0f, 0.0f);
	glVertex3f(-frameWidth, -frameHeight, 0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(frameWidth, frameHeight, 0.0f);
	glVertex3f(-frameWidth, 0.0f, 0.0f);
	glVertex3f(frameWidth, -frameHeight, 0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-frameWidth, frameHeight, -0.15f);
	glVertex3f(frameWidth, 0.0f, -0.15f);
	glVertex3f(-frameWidth, -frameHeight, -0.15f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(frameWidth, frameHeight, -0.15f);
	glVertex3f(-frameWidth, 0.0f, -0.15f);
	glVertex3f(frameWidth, -frameHeight, -0.15f);
	glEnd();



	glEnable(GL_POLYGON_STIPPLE);
	glPolygonStipple(stipple);
	glBegin(GL_TRIANGLES);
	//glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
	glColor4f(0.8f, 0.8f, 0.8f, 1.0f);

	glNormal3f(0.0f, 0.0f, 1.0f);                   
	glVertex3f(frameWidth, frameHeight, 0.1f);
	glVertex3f(-frameWidth, frameHeight, 0.1f);
	glVertex3f(0.0f, frameHeight / 2, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);                             //SAME GLASS but front and back side is done here with opposite Normal axis.
	glVertex3f(frameWidth, frameHeight, -0.1f);
	glVertex3f(-frameWidth, frameHeight, -0.1f);
	glVertex3f(0.0f, frameHeight / 2, -0.1f);

	glNormal3f(0.0f, 0.0f, 1.0f);                   //BOTTOM blue glass
	glVertex3f(frameWidth, -frameHeight, 0.1f);
	glVertex3f(-frameWidth, -frameHeight, 0.1f);
	glVertex3f(0.0f, -frameHeight / 2, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);                             //SAME GLASS but front and back side is done here with opposite Normal axis.
	glVertex3f(frameWidth, -frameHeight, -0.1f);
	glVertex3f(-frameWidth, -frameHeight, -0.1f);
	glVertex3f(0.0f, -frameHeight / 2, -0.1f);


	//glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
	glColor4f(0.8f, 0.8f, 0.8f, 1.0f);

	glNormal3f(0.0f, 0.0f, 1.0f);

	glVertex3f(frameWidth, frameHeight, 0.1f);
	glVertex3f(0.0f, frameHeight / 2, 0.1f);
	glVertex3f(frameWidth, 0.0f, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);

	glVertex3f(frameWidth, frameHeight, -0.1f);
	glVertex3f(0.0f, frameHeight / 2, -0.1f);
	glVertex3f(frameWidth, 0.0f, -0.1f);

	glNormal3f(0.0f, 0.0f, 1.0f);

	glVertex3f(-frameWidth, frameHeight, 0.1f);
	glVertex3f(0.0f, frameHeight / 2, 0.1f);
	glVertex3f(-frameWidth, 0.0f, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);

	glVertex3f(-frameWidth, frameHeight, -0.1f);
	glVertex3f(0.0f, frameHeight / 2, -0.1f);
	glVertex3f(-frameWidth, 0.0f, -0.1f);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(frameWidth, 0.0f, 0.1f);
	glVertex3f(0.0f, -frameHeight / 2, 0.1f);
	glVertex3f(frameWidth, -frameHeight, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(frameWidth, 0.0f, -0.1f);
	glVertex3f(0.0f, -frameHeight / 2, -0.1f);
	glVertex3f(frameWidth, -frameHeight, -0.1f);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-frameWidth, 0.0f, 0.1f);
	glVertex3f(0.0f, -frameHeight / 2, 0.1f);
	glVertex3f(-frameWidth, -frameHeight, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-frameWidth, 0.0f, -0.1f);
	glVertex3f(0.0f, -frameHeight / 2, -0.1f);
	glVertex3f(-frameWidth, -frameHeight, -0.1f);

	glEnd();

	glBegin(GL_QUADS);

	//glColor4f(0.8f, 0.8f, 0.8f, 0.5f);         //Middle glassyd
	glColor4f(0.8f, 0.8f, 0.8f, 1.0f);         //Middle glassyd

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, frameHeight / 2, 0.1f);
	glVertex3f(-frameWidth, 0.0f, 0.1f);
	glVertex3f(0.0f, -frameHeight / 2, 0.1f);
	glVertex3f(frameWidth, 0.0f, 0.1f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(0.0f, frameHeight / 2, -0.1f);
	glVertex3f(-frameWidth, 0.0f, -0.1f);
	glVertex3f(0.0f, -frameHeight / 2, -0.1f);
	glVertex3f(frameWidth, 0.0f, -0.1f);

	glEnd();
	glDisable(GL_POLYGON_STIPPLE);
}

void drawMainFrontStructure(void)
{
	glPushMatrix();
	{
		//glTranslatef(0.0f, 2.0f, 0.0f);

		glPushMatrix();
		drawCentralPart();
		glPopMatrix();

		glPushMatrix();
		drawLeftBuilding();
		glPopMatrix();


		glPushMatrix();
		drawRightBuilding();
		glPopMatrix();

		glPushMatrix();
		{
			DrawLeftBuildingPyramid();
		}
		glPopMatrix();

		glPushMatrix();
		DrawMainArc();
		glPopMatrix();

		glPushMatrix();
		{
			drawIntegratedClassRoom();
		}
		glPopMatrix();
		
	}
	glPopMatrix();

	
}

void DrawBezierSurface(void)
{
	// Set up the Bezier's surface
	// glMap2f(Type of the data generated, Lowe U range, Upper U range, Distance between points in the data, Dimension in u direction (order), Lower V range, Upper V range, Distance between points in the data, Dimension in v direction (order), array of control points)
	glMap2f(GL_MAP2_VERTEX_3, 0.0f, 100.0f, 3, 3, 0.0f, 100.0f, 9, 3, &gfControlPointsArc[0][0][0]);

	// Enable the evaluator
	glEnable(GL_MAP2_VERTEX_3);

	// Use higher level functions to map to a grid, then evaluate the entire thing
	// Map a grid of 10 points from 0 to 10
	glMapGrid2f(100, 0.0f, 100.0f, 100, 0.0f, 100.0f);

	// Evaluate the grid, using lines
	//glEvalMesh2(GL_LINE, 0, 10, 0, 10);
	glEvalMesh2(GL_FILL, 0, 100, 0, 100);
}

void DrawBezierSurfaceAboveArc(void)
{
	// Set up the Bezier's surface
	// glMap2f(Type of the data generated, Lowe U range, Upper U range, Distance between points in the data, Dimension in u direction (order), Lower V range, Upper V range, Distance between points in the data, Dimension in v direction (order), array of control points)
	glMap2f(GL_MAP2_VERTEX_3, 0.0f, 100.0f, 3, 3, 0.0f, 100.0f, 9, 3, &gfControlPointsSurface[0][0][0]);

	// Enable the evaluator
	glEnable(GL_MAP2_VERTEX_3);

	// Use higher level functions to map to a grid, then evaluate the entire thing
	// Map a grid of 10 points from 0 to 10
	glMapGrid2f(100, 0.0f, 100.0f, 100, 0.0f, 100.0f);

	// Evaluate the grid, using lines
	//glEvalMesh2(GL_LINE, 0, 10, 0, 10);
	glEvalMesh2(GL_FILL, 0, 100, 0, 100);
}

void drawBehindWalls(void)
{
	void drawCube(GLfloat height, GLfloat width, GLfloat depth);
	drawCube(30.0f, 4.5f, 10.0f);

	glColor3f(1.0f, 0.0f, 0.0f);//first door
	glPushMatrix();
	glTranslatef(5.6f, 0.0f, -0.5f);
	drawCube(30.0f, 6.6f, 10.0f);
	glPopMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glTranslatef(12.69f, 0.0f, 0.0f);
	drawCube(30.0f, 7.6f, 10.0f);
	glPopMatrix();

	glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();//second door
	glTranslatef(19.9f, 0.0f, -0.5f);
	drawCube(30.0f, 6.6f, 10.0f);
	glPopMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glTranslatef(27.0f, 0.0f, 0.0f);
	drawCube(30.0f, 7.6f, 10.0f);
	glPopMatrix();

	glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();//third door
	glTranslatef(34.0f, 0.0f, -0.5f);
	drawCube(30.0f, 6.6f, 10.0f);
	glPopMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glTranslatef(41.1f, 0.0f, 0.0f);
	drawCube(30.0f, 7.6f, 10.0f);
	glPopMatrix();

	glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();//fourth door
	glTranslatef(48.2f, 0.0f, -0.5f);
	drawCube(30.0f, 6.6f, 10.0f);
	glPopMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glTranslatef(55.3f, 0.0f, 0.0f);
	drawCube(30.0f, 7.6f, 10.0f);
	glPopMatrix();

	glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();//fifth door
	glTranslatef(62.5f, 0.0f, -0.5f);
	drawCube(30.0f, 6.6f, 10.0f);
	glPopMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glTranslatef(68.1f, 0.0f, 0.0f);
	drawCube(30.0f, 4.5f, 10.0f);
	glPopMatrix();
}

void drawCentralPart(void)
{
	void drawCentralPillars(void);
	void drawChabutra(void);

	glPushMatrix();
	glTranslatef(0.0f, -15.0f, -12.0f);

	glPushMatrix();
	drawCentralPillars();
	glPopMatrix();

	glPushMatrix();
	drawChabutra();
	glPopMatrix();

	glPopMatrix();
}

void drawCentralPillars(void)
{
	void drawCube(GLfloat height, GLfloat width, GLfloat depth);
	void drawLowerLeftSideBlock(void);
	void drawLowerRightSideBlock(void);
	void drawStrips(void);
	void drawCubedPillars(void);

	glPushMatrix();

	glScalef(1.0f, 0.7f, 1.0f);
	glTranslatef(0.0f, 20.0f, 0.0f);
	//push 0
	glColor3f(0.81f, 0.71f, 0.23f);
	glPushMatrix();							//Left Side Block (lower wall)
	drawLowerLeftSideBlock();
	glPopMatrix();

	glPushMatrix();
	drawLowerRightSideBlock();				//Right Side Block (lower wall)
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 45.0f, 0.0f);		//Upper Block for arched entry gate (lower wall)
	drawCube(10.0f, 40.0f, 10.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 47.5f, 17.5f);		//gate shed block
	drawCube(5.0f, 40.0f, 15.0f);
	glPopMatrix();

	glPushMatrix();
	drawStrips();							//Strips along the wall
	glPopMatrix();

	glPushMatrix();
	drawCubedPillars();						//cubed pillars for the balcony
	glPopMatrix();

	glPopMatrix();
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void DrawMainArc(void)
{
	glPushMatrix();
	{
		glColor3f(0.81f, 0.71f, 0.23f);
		glTranslatef(0.0f, 17.0f, -13.05f);
		glScalef(7.0f, 5.0f, 1.5f);
		glCallList(dsArc);
	}
	glPopMatrix();

	glPushMatrix(); // Arc Right Pillar
	{
		glTranslatef(18.0f, 10.0f, -12.0f);
		glScalef(2.0f, 11.0f, 5.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix(); // Arc Left Pillar
	{
		glTranslatef(-18.0f, 10.0f, -12.0f);
		glScalef(2.0f, 11.0f, 5.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();
}

void drawChabutra(void)
{
	void drawCube(GLfloat height, GLfloat width, GLfloat depth);
	///////////////////////////////////////////////////////chabutra left
	glPushMatrix(); //push 0
	glTranslatef(-50.8f, 19.0f, 6.0f);
	glScalef(0.395f, 0.50f, 1.5f);

	glColor3f(0.752941f, 0.752941f, 0.752941f);
	/*glPushMatrix();//push for translate1
	glTranslatef(0.0f, -15.0f, 0.0f);
	glPushMatrix();
	drawCube(10.0f, 150.0f, 10.0f);
	glPopMatrix();//pop for translate1
	glPopMatrix();*/


	//glColor3f(0.0f, 1.0f, 0.0f);
	glPushMatrix();//push for translate2
	glTranslatef(-66.0f, -4.7f, 3.0f);
	glPushMatrix();
	drawCube(10.0f, 15.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();

	//glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();//push for translate2
	glTranslatef(-43.0f, -4.7f, -1.5f);
	glPushMatrix();
	drawCube(10.0f, 36.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();
	///////////////////////////////////////////trail
	//glColor3f(0.0f, 0.0f, 1.0f);
	glPushMatrix();//push for translate2
	glTranslatef(-38.0f, -7.3f, 2.4f);
	glPushMatrix();
	drawCube(5.0f, 41.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();
	///////////////////////////////////////////////

	//glColor3f(0.0f, 1.0f, 0.0f);
	glPushMatrix();//push for translate2
	glTranslatef(0.7f, -4.7f, 3.0f);
	glPushMatrix();
	drawCube(10.0f, 55.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();


	//glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();//push for translate2
	glTranslatef(38.0f, -4.7f, -1.5f);
	glPushMatrix();
	drawCube(10.0f, 20.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();
	////////////////////////////////////////////////////trial
	//glColor3f(0.0f, 0.0f, 1.0f);
	glPushMatrix();//push for translate2
	glTranslatef(38.0f, -7.3f, 2.4f);
	glPushMatrix();
	drawCube(5.0f, 30.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();
	//////////////////////////////////////////////////////
	//glColor3f(0.0f, 1.0f, 0.0f);
	glPushMatrix();//push for translate2
	glTranslatef(55.5f, -4.7f, 3.0f);
	glPushMatrix();
	drawCube(10.0f, 20.0f, 10.0f);
	glPushMatrix();                       //Curve
	{
		quadric = gluNewQuadric();
		gluQuadricOrientation(quadric, GLU_INSIDE);

		glScalef(2.0f, 1.0f, 1.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(5.0f, -5.0f, -5.0f);
		glNormal3f(0.0f, -1.0f, 0.0f);
		
		gluPartialDisk(quadric, 0.0f, 10.0f, 50, 50, 0.0f, 90.0f);
		gluQuadricOrientation(quadric, GLU_OUTSIDE);
		gluCylinder(quadric, 10.0f, 10.0f, 10.0f, 50, 50);
		

	}
	glPopMatrix();

	glPopMatrix();//pop for translate2
	glPopMatrix();

	glPopMatrix();//pop 0

				  ////////////////////////////////////////////////////////////////////////////////////////////////chabutra right


	glPushMatrix(); //push 0
					//	glTranslatef(50.0f, 20.0f, 6.0f);
					//	glScalef(0.4f, 0.5f, 1.5f);

	glTranslatef(50.8f, 19.0f, 6.0f);
	glScalef(0.44f, 0.50f, 1.5f);

//	glScalef(0.395f, 0.50f, 1.5f);
	/*glColor3f(0.0f, 0.0f, 1.0f);
	glPushMatrix();//push for translate1
	glTranslatef(0.0f, -15.0f, 0.0f);
	glPushMatrix();
	drawCube(10.0f, 150.0f, 10.0f);
	glPopMatrix();//pop for translate1
	glPopMatrix();*/


	//glColor3f(0.0f, 1.0f, 0.0f);
	glPushMatrix();//push for translate2
	glTranslatef(-51.0f, -4.7f, 3.0f);
	glPushMatrix();
	drawCube(10.0f, 15.0f, 10.0f);
	
	glPushMatrix();
	{
		quadric = gluNewQuadric();
		gluQuadricOrientation(quadric, GLU_INSIDE);

		glScalef(2.0f, 1.0f, 1.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(-3.75f, -5.0f, -5.0f);
		glNormal3f(0.0f, -1.0f, 0.0f);

		gluPartialDisk(quadric, 0.0f, 10.0f, 50, 50, 270.0f, 90.0f);
		gluQuadricOrientation(quadric, GLU_OUTSIDE);
		gluCylinder(quadric, 10.0f, 10.0f, 10.0f, 50, 50);
	}
	glPopMatrix();

	glPopMatrix();//pop for translate2
	glPopMatrix();

	//glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();//push for translate2
	glTranslatef(-37.0f, -4.7f, -1.5f);
	glPushMatrix();
	drawCube(10.0f, 35.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();

	///////////////////////////////////////////////////////trial
	//glColor3f(0.0f, 0.0f, 1.0f);
	glPushMatrix();//push for translate2
	glTranslatef(-37.0f, -7.3f, 2.4f);
	glPushMatrix();
	drawCube(5.0f, 35.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();

	//glColor3f(0.0f, 1.0f, 0.0f);
	glPushMatrix();//push for translate2
	glTranslatef(8.0f, -4.7f, 3.0f);
	glPushMatrix();
	drawCube(10.0f, 55.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();


	//glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();//push for translate2
	glTranslatef(46.0f, -5.0f, -1.5f);
	glPushMatrix();
	drawCube(10.0f, 20.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();

	///////////////////////////////////////////////////////trial
	//glColor3f(0.0f, 0.0f, 1.0f);
	glPushMatrix();//push for translate2
	glTranslatef(46.0f, -7.3f, 2.4f);
	glPushMatrix();
	drawCube(5.0f, 30.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();

	///////////////////////////////////////////////////////////
	//glColor3f(0.0f, 1.0f, 0.0f);
	glPushMatrix();//push for translate2
	glTranslatef(65.5f, -5.0f, 3.0f);
	glPushMatrix();
	drawCube(10.0f, 20.0f, 10.0f);
	glPopMatrix();//pop for translate2
	glPopMatrix();

	glPopMatrix();//pop 0

}

/////////////////////////////////////////////////////////////////////////////////////////////////

void DrawLeftBuildingPyramid(void)
{
	glColor3f(0.81f, 0.71f, 0.23f);
	glPushMatrix();
	{
		glTranslatef(-92.0f, 102.5f, -34.5f);
		//glTranslatef(-92.0f, 110.0f, -14.5f);
		glScalef(12.0f, 10.0f, 40.0f);
		DrawLeftTopTrapezoid();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glTranslatef(118.0f, 102.5f, 34.5f);
		//glTranslatef(118.0f, 110.0f, 14.5f);
		glScalef(12.0f, 10.0f, 40.0f);
		DrawLeftTopTrapezoid();
	}
	glPopMatrix();
}

void drawLeftBuilding(void)
{
	void drawLeftBuildingGroundFloor();
	void drawLeftBuildingMiddleFloor();
	void drawLeftBuildingTopFloor();
	glPushMatrix();
	glTranslatef(2.5f, 1.65f, -1.75f);
	drawLeftBuildingGroundFloor();
	drawLeftBuildingMiddleFloor();
	//drawLeftBuildingMiddleFloor();
	drawLeftBuildingTopFloor();
	glPopMatrix();
}

void drawRightBuilding(void)
{
	void drawRightBuildingGroundFloor();
	void drawRightBuildingMiddleFloor();
	void drawRightBuildingTopFloor();

	glPushMatrix();
	glTranslatef(-2.1f, 1.5f, -1.5f);

	drawRightBuildingGroundFloor();
	drawRightBuildingMiddleFloor();
	drawRightBuildingTopFloor();

	glPopMatrix();
}


/////////////////////////////////////////////////////////////////////////////////////////////////

void drawRightBuildingGroundFloor(void)
{
	void drawBlockLeftGroundFloor(void);
	void drawBlockMiddleGroundFloor(void);
	void drawBlockRightGroundFloor(void);
	void drawBlockTopGroundFloor(void);
	void drawHollowRightGround(void);

	glPushMatrix();
	glTranslatef(85.0f, 15.0f, 0.0f);

	glPushMatrix();
	drawBlockLeftGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockMiddleGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockRightGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockTopGroundFloor();
	glPopMatrix();

	glPushMatrix();
	//drawBlockBottomGroundFloor();
	glPopMatrix();

	glPushMatrix();
//	drawHollowRightGround();
	glTranslatef(11.5f, -1.2f, 6.0f);
	glScalef(1.1f, 1.2f, 1.2f);
	drawWhiteDoor();

	glTranslatef(20.0f, 0.0f, 0.0f);
	drawWhiteDoor();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, -23.2f, 2.0f);
	glScalef(1.0f, 0.55f, 1.0f);
	drawBlockTopGroundFloor();
	glPopMatrix();

	glPopMatrix();//pop for translate

}

void drawRightBuildingMiddleFloor()
{
	void drawBlockLeftGroundFloor(void);
	void drawBlockMiddleGroundFloor(void);
	void drawBlockRightGroundFloor(void);
	void drawBlockTopGroundFloor(void);
	void drawHollowRightGround(void);


	glPushMatrix();
	glTranslatef(85.0f, 52.0f, 0.0f);

	glPushMatrix();
	drawBlockLeftGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockMiddleGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockRightGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockTopGroundFloor();
	glPopMatrix();

	glPushMatrix();
	//drawBlockBottomGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawHollowRightGround();
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(12.0f, -7.0f, 7.0f);
		drawWindowMedium();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(34.0f, -7.0f, 7.0f);
		drawWindowMedium();
	}
	glPopMatrix();

	glPopMatrix();//pop for translate
}

void drawRightBuildingTopFloor()
{
	void drawBlockLeftTopFloorRight(void);
	void drawBlockMiddleTopFloorRight(void);
	void drawBlockRightTopFloorRight(void);
	void drawBlockTopTopFloorRight(void);
	void drawHollowTopFloorRight(void);

	glPushMatrix();
	glTranslatef(85.0f, 84.0f, 0.0f);


	glPushMatrix();
	drawBlockLeftTopFloorRight();
	glPopMatrix();

	glPushMatrix();
	drawBlockMiddleTopFloorRight();
	glPopMatrix();

	glPushMatrix();
	drawBlockRightTopFloorRight();
	glPopMatrix();

	glPushMatrix();
	drawBlockTopTopFloorRight();
	glPopMatrix();

	glPushMatrix();
	//drawBlockBottomGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawHollowTopFloorRight();
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(11.5f, -2.0f, 7.0f);
		drawWindowMedium();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(34.0f, -2.0f, 7.0f);
		drawWindowMedium();
	}
	glPopMatrix();

	glPopMatrix();//for translate
}

void drawLeftBuildingGroundFloor()
{
	void drawBlockLeftGroundFloor(void);
	void drawBlockMiddleGroundFloor(void);
	void drawBlockRightGroundFloor(void);
	void drawBlockTopGroundFloor(void);
	void drawHollow(void);

	glPushMatrix();
	glTranslatef(-130.0f, 15.0f, 0.0f);

	glPushMatrix();
	drawBlockLeftGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockMiddleGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockRightGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockTopGroundFloor();
	glPopMatrix();

	glPushMatrix();
//	drawBlockTopGroundFloor();
	glPopMatrix();

	glPushMatrix();
//	drawHollow();
	glTranslatef(11.5f, -1.0f, 5.0f);
	glScalef(1.1f, 1.2f, 1.2f);
	drawWhiteDoor();

	glTranslatef(20.0f, 0.0f, 0.0f);
	drawWhiteDoor();

	glPopMatrix();

	
	glPushMatrix();
	glTranslatef(0.0f, -23.2f, 2.0f);
	glScalef(1.0f, 0.55f, 1.0f);
	drawBlockTopGroundFloor();
	glPopMatrix();

	glPopMatrix();//pop for translate

}

void drawLeftBuildingMiddleFloor()
{
	void drawBlockLeftGroundFloor(void);
	void drawBlockMiddleGroundFloor(void);
	void drawBlockRightGroundFloor(void);
	void drawBlockTopGroundFloor(void);
	void drawHollow(void);

	glPushMatrix();
	glTranslatef(-130.0f, 52.0f, 0.0f);


	glPushMatrix();
	drawBlockLeftGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockMiddleGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockRightGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawBlockTopGroundFloor();
	glPopMatrix();

	glPushMatrix();
	//drawBlockBottomGroundFloor();
	glPopMatrix();

	glPushMatrix();
	drawHollow();
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(12.0f, -7.0f, 7.0f);
		drawWindowMedium();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(34.0f, -7.0f, 7.0f);
		drawWindowMedium();
	}
	glPopMatrix();

	glPopMatrix();//pop for translate
}

void drawLeftBuildingTopFloor()
{
	void drawBlockLeftTopFloorLeft();
	void drawBlockMiddleTopFloorLeft();
	void drawBlockRightTopFloorLeft();

	glPushMatrix();
	glTranslatef(-127.5f, 86.0f, 0.0f);

	glPushMatrix();
	drawBlockLeftTopFloorLeft();
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(11.5f, -4.8f, 7.0f);
		drawWindowSmall();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(29.0f, -4.8f, 7.0f);
		drawWindowSmall();
	}
	glPopMatrix();

	glPushMatrix();
	drawBlockMiddleTopFloorLeft();
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(11.5f, -4.8f, 7.0f);
		drawWindowSmall();
	}
	glPopMatrix();

	glPushMatrix();
	drawBlockRightTopFloorLeft();
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(0.0f, -6.0f, 0.0f);
		glScalef(0.9f, 0.7f, 1.0f);
		drawHollow();
	}
	glPopMatrix();


	glPopMatrix();//pop for translate
}

void drawBlockLeftGroundFloor(void)
{
	//void drawCube(void);

	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 0.0f;
	gTranslateY = 0.0f;
	gTranslateZ = -32.5f;

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);

	drawCube(45.0f, 5.0f, 80.0f);//change

}

void drawBlockMiddleGroundFloor(void)
{
	//void drawCube(void);

	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 22.5f;
	gTranslateY = 0.0f;
	gTranslateZ = 0.0f;

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);

	drawCube(35.0f, 5.0f, 15.0f);

}

void drawBlockRightGroundFloor(void)
{
	//void drawCube(void);

	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 45.0f;
	gTranslateY = 0.0f;
	gTranslateZ = -32.5f; //change

	gScaleX = 13.0f;
	gScaleY = 100.0f;
	gScaleZ = 10.0f;

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	//glScalef(gScaleX, gScaleY, gScaleZ);


	drawCube(45.0f, 5.0f, 80.0f);  //change

}

void drawBlockTopGroundFloor(void)
{
	//void drawCube(void);
	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 22.5f;
	gTranslateY = 15.0f;
	gTranslateZ = 0.0f;

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	//glScalef(gScaleX, gScaleY, gScaleZ);


	drawCube(9.0f, 50.f, 15.0f);

}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void drawHollow(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 20.0f;
	gTranslateY = 0.0f;
	gTranslateZ = 4.0f;

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	//glScalef(gScaleX, gScaleY, gScaleZ);


	drawCube(35.0f, 45.0f, 5.0f);

}

void drawHollowRightGround(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 25.0f;
	gTranslateY = 0.0f;
	gTranslateZ = 4.0f;

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	//glScalef(gScaleX, gScaleY, gScaleZ);


	drawCube(35.0f, 45.0f, 5.0f);

}

void drawBlockLeftTopFloorRight(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 0.0f;
	gTranslateY = 0.0f;
	gTranslateZ = -32.5f; //change

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);

	drawCube(37.0f, 5.0f, 80.0f); //change

}

void drawBlockMiddleTopFloorRight(void)
{
	//void drawCube(void);

	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 22.5f;
	gTranslateY = 0.0f;
	gTranslateZ = 0.0f;

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);

	drawCube(30.0f, 5.0f, 15.0f);

}

void drawBlockRightTopFloorRight(void)
{
	//void drawCube(void);

	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 45.0f;
	gTranslateY = 0.0f;
	gTranslateZ = -32.5f;//change


	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);

	drawCube(37.0f, 5.0f, 80.0f); //change

}

void drawHollowTopFloorRight(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 25.0f;
	gTranslateY = 0.0f;
	gTranslateZ = 3.5f;

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	//glScalef(gScaleX, gScaleY, gScaleZ);


	drawCube(30.0f, 45.0f, 5.0f);

}

void drawBlockTopTopFloorRight(void)
{
	//void drawCube(void);
	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 22.5f;
	gTranslateY = 15.0f;
	gTranslateZ = 0.0f;

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	//glScalef(gScaleX, gScaleY, gScaleZ);


	drawCube(7.0f, 50.f, 15.0f);

}

void drawRightRoof(void)
{

	glBegin(GL_TRIANGLES);

	//front face

	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-25.0f, -5.0f, 10.0f);
	glVertex3f(25.0f, -5.0f, 10.0f);

	//back face
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-25.0f, -5.0f, -10.0f);
	glVertex3f(25.0f, -5.0f, -10.0f);

	//right face
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(25.0f, -5.0f, 10.0f);
	glVertex3f(25.0f, -5.0f, -10.0f);

	//left face
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-25.0f, -5.0f, 10.0f);
	glVertex3f(-25.0f, -5.0f, -10.0f);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(-25.0f, -5.0f, 10.0f);
	glVertex3f(-25.0f, -5.0f, -10.0f);
	glVertex3f(25.0f, -5.0f, 10.0f);
	glVertex3f(25.0f, -5.0f, -10.0f);
	glEnd();
}

void drawBlockLeftTopFloorLeft(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 0.0f;
	gTranslateY = 0.0f;
	gTranslateZ = 0.0f;

	glColor3f(0.81f, 0.71f, 0.23f);
	//glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	//glScalef(gScaleX, gScaleY, gScaleZ);

	glPushMatrix();
	glTranslatef(0.0f, -5.0f, 0.0f);
	glPushMatrix();
	drawCube(20.0f, 10.f, 15.0f);
	glPopMatrix();
	glPopMatrix();

}

void drawBlockMiddleTopFloorLeft(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 20.5f;
	gTranslateY = 5.0f;
	gTranslateZ = 0.0f;

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	//glScalef(gScaleX, gScaleY, gScaleZ);


	drawCube(38.0f, 5.0f, 15.0f);
}

void drawBlockRightTopFloorLeft(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);


	gTranslateX = 40.0f;
	gTranslateY = 0.0f;
	gTranslateZ = -32.5f; //change

	glColor3f(0.81f, 0.71f, 0.23f);
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	//glScalef(gScaleX, gScaleY, gScaleZ);


	glPushMatrix();
	glTranslatef(0.0f, -5.0f, 0.0f);
	glPushMatrix();
	drawCube(20.0f, 10.f, 80.0f); //change
	glPopMatrix();
	glPopMatrix();

}

void drawLeftRoof(void)
{
	glBegin(GL_TRIANGLES);
	//front face
	glVertex3f(2.0f, 5.5f, 10.0f);
	glVertex3f(-25.0f, -10.0f, 10.0f);
	glVertex3f(25.0f, -10.0f, 10.0f);

	//back face
	glVertex3f(-25.0f, -10.0f, -10.0f);
	glVertex3f(25.0f, -10.0f, -10.0f);
	glVertex3f(2.0f, 5.5f, 10.0f);

	//right face
	glVertex3f(2.0, 5.5, 10.0f);
	glVertex3f(25.0f, -10.0f, 10.0f);
	glVertex3f(25.0f, -10.0f, -10.0f);

	//left face
	glVertex3f(2.0, 5.5, 10.0f);
	glVertex3f(-25.0f, -10.0f, 10.0f);
	glVertex3f(-25.0f, -10.0f, -10.0f);

	glEnd();
}

////////////////////////////////////////////////////////////////////////////////////////////////
void drawCubedPillars(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);
	//cubed pillar spaces - left side
	//1st - vertical
	
	 

	glPushMatrix();
	glTranslatef(-78.0f, 70.0f, 10.0f);

	glScalef(1.0f, 2.0f, 1.0f);
	drawPillarDesign();

	glColor3f(1.0f, 1.0f, 1.0f);	

	glPopMatrix();

	glPushMatrix();
	{

		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-74.5f, 47.0f, 10.0f);
		glScalef(1.2f, 1.5f, 1.0f);

		glCallList(dsWhiteDesign);

		for (int i = 0; i <= 6; i++)
		{
			glTranslatef(2.5f, 0.0f, 0.0f);
			glCallList(dsWhiteDesign);

		}
	}
	glPopMatrix();

	//2nd - vertical
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-48.875f, 70.0f, 10.0f);
	glScalef(1.0f, 2.0f, 1.0f);
	drawPillarDesign();
	glPopMatrix();

	//3rd - horizontal
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-63.5f, 65.5f, 10.0f);
	glScalef(1.0f, 1.5f, 1.0f);
	drawCube(0.625f, 25.375f, 1.25f);
	glPopMatrix();
///////////////////////////////////////////////////////////////

	glPushMatrix();

	glTranslatef(29.0f, 0.0f, 0.0f);

	glPushMatrix();
	{

		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-74.5f, 47.0f, 10.0f);
		glScalef(1.2f, 1.5f, 1.0f);

		glCallList(dsWhiteDesign);

		for (int i = 0; i <= 6; i++)
		{
			glTranslatef(2.5f, 0.0f, 0.0f);
			glCallList(dsWhiteDesign);

		}
	}
	glPopMatrix();

	//2nd - vertical
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-48.875f, 70.0f, 10.0f);
	glScalef(1.0f, 2.0f, 1.0f);
	drawPillarDesign();
	glPopMatrix();

	//3rd - horizontal
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-63.5f, 65.5f, 10.0f);
	glScalef(1.0f, 1.5f, 1.0f);
	drawCube(0.625f, 25.375f, 1.25f);
	glPopMatrix();


	glPopMatrix();

///////////////////////////////////////////////////////////////
	glPushMatrix();

	glTranslatef(98.0f, 0.0f, 0.0f);
	glPushMatrix();
	glTranslatef(-78.0f, 70.0f, 10.0f);

	glScalef(1.0f, 2.0f, 1.0f);
	drawPillarDesign();

	glColor3f(1.0f, 1.0f, 1.0f);

	glPopMatrix();

	glPushMatrix();
	{

		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-74.5f, 47.0f, 10.0f);
		glScalef(1.2f, 1.5f, 1.0f);

		glCallList(dsWhiteDesign);

		for (int i = 0; i <= 6; i++)
		{
			glTranslatef(2.5f, 0.0f, 0.0f);
			glCallList(dsWhiteDesign);

		}
	}
	glPopMatrix();

	//2nd - vertical
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-48.875f, 70.0f, 10.0f);
	glScalef(1.0f, 2.0f, 1.0f);
	drawPillarDesign();
	glPopMatrix();

	//3rd - horizontal
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-63.5f, 65.5f, 10.0f);
	glScalef(1.0f, 1.5f, 1.0f);
	drawCube(0.625f, 25.375f, 1.25f);
	glPopMatrix();
	///////////////////////////////////////////////////////////////

	glPushMatrix();

	glTranslatef(29.0f, 0.0f, 0.0f);

	glPushMatrix();
	{

		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-74.5f, 47.0f, 10.0f);
		glScalef(1.2f, 1.5f, 1.0f);

		glCallList(dsWhiteDesign);

		for (int i = 0; i <= 6; i++)
		{
			glTranslatef(2.5f, 0.0f, 0.0f);
			glCallList(dsWhiteDesign);

		}
	}
	glPopMatrix();

	//2nd - vertical
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-48.875f, 70.0f, 10.0f);
	glScalef(1.0f, 2.0f, 1.0f);
	drawPillarDesign();
	glPopMatrix();

	//3rd - horizontal
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-63.5f, 65.5f, 10.0f);
	glScalef(1.0f, 1.5f, 1.0f);
	drawCube(0.625f, 25.375f, 1.25f);
	glPopMatrix();


	glPopMatrix();
	glPopMatrix();



///////////////////////////////////////////////////////////////////////////
	//for central part
//3rd - horizontal
	//glPushMatrix();
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-21.0f, 65.5f, 25.0f);
	glScalef(1.0f, 1.5f, 1.0f);
	drawCube(0.625f,2.0f, 1.25f);
	glPopMatrix();

	glPushMatrix();
	{

		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-14.5f, 47.0f, 25.0f);
		glScalef(1.2f, 1.5f, 1.0f);

		glCallList(dsWhiteDesign);

		for (int i = 0; i < 1; i++)
		{
			glTranslatef(2.5f, 0.0f, 0.0f);
			glCallList(dsWhiteDesign);

		}
	}
	glPopMatrix();
	////////////////////////////////////////////////////////////////////////////////
	//glPushMatrix();
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-12.5f, 65.5f, 25.0f);
	glScalef(3.5f, 1.5f, 1.0f);
	drawCube(0.625f, 2.0f, 1.25f);
	glPopMatrix();

	glPushMatrix();
	{

		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-3.5f, 47.0f, 25.0f);
		glScalef(1.2f, 1.5f, 1.0f);

		glCallList(dsWhiteDesign);

		for (int i = 0; i < 2; i++)
		{
			glTranslatef(2.5f, 0.0f, 0.0f);
			glCallList(dsWhiteDesign);

		}
	}
	glPopMatrix();
	//////////////////////////////////////////////////////////////////////////////////////////////
	//glPushMatrix();
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(0.0f, 65.5f, 25.0f);
	glScalef(5.0f, 1.5f, 1.0f);
	drawCube(0.625f, 2.0f, 1.25f);
	glPopMatrix();
	//glPopMatrix();

	glPushMatrix();
	{

		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(10.5f, 47.0f, 25.0f);
		glScalef(1.2f, 1.5f, 1.0f);

		glCallList(dsWhiteDesign);

		for (int i = 0; i < 1; i++)
		{
			glTranslatef(2.5f, 0.0f, 0.0f);
			glCallList(dsWhiteDesign);

		}
	}
	glPopMatrix();
/////////////////////////////////////////////////////////////////////////////////////////////////////////

	//glPushMatrix();
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(12.5f, 65.5f, 25.0f);
	glScalef(3.5f, 1.5f, 1.0f);
	drawCube(0.625f, 2.0f, 1.25f);
	glPopMatrix();
	//glPopMatrix();

	//glPushMatrix();
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(21.0f, 65.5f, 25.0f);
	glScalef(1.0f, 1.5f, 1.0f);
	drawCube(0.625f, 2.0f, 1.25f);
	glPopMatrix();
///////////////////////////////////////////////////////////////////////////
	glPushMatrix(); //left side central front
	glTranslatef(-18.0f, 70.0f, 25.0f);
	glScalef(1.0f, 2.0f, 1.0f);
	drawPillarDesign();
	glPopMatrix();
////////////////////////////////////////////////////////////////////////////
	
	glPushMatrix(); //right side central 1 front
	glTranslatef(18.0f, 70.0f, 25.0f);
	glScalef(1.0f, 2.0f, 1.0f);
	drawPillarDesign();
	glPopMatrix();

////////////////////////////////////////////////////////////////////////////////
	glPushMatrix(); //left side central 1 front
	glTranslatef(7.0f, 70.0f, 25.0f);
	glScalef(1.0f, 2.0f, 1.0f);
	drawPillarDesign();
	glPopMatrix();
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////

//for left side rotated part
//3rd - horizontal
//glPushMatrix();
	glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-22.0f, 65.5f, 17.5f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(8.2f, 1.5f, 1.0f);
	drawCube(0.625f, 2.0f, 4.25f);
	glPopMatrix();

	glPushMatrix();
	{

		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-22.5f, 47.0f, 23.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.2f, 1.5f, 1.0f);

		glPushMatrix();
		glTranslatef(0.0f, 0.0f, 1.5f);
		glCallList(dsWhiteDesign);

		for (int i = 0; i < 3; i++)
		{
			glTranslatef(2.5f, 0.0f, 0.0f);
			glCallList(dsWhiteDesign);

		}
		glPopMatrix();
	}
	glPopMatrix();


//////////////////////////////////////////////////////////////////////////////////
	glColor3f(0.7f, 0.3f, 0.2f);// right side rotated part
	glPushMatrix();
	glTranslatef(22.5f, 65.5f, 17.5f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(8.2f, 1.5f, 1.0f);
	drawCube(0.625f, 2.0f, 4.25f);
	glPopMatrix();



	//glPushMatrix();
	//{

	//	glColor3f(1.0f, 1.0f, 1.0f);
	//	glTranslatef(-14.5f, 47.0f, 25.0f);
	//	glScalef(1.2f, 1.5f, 1.0f);

	//	glCallList(dsWhiteDesign);

	//	for (int i = 0; i < 1; i++)
	//	{
	//		glTranslatef(2.5f, 0.0f, 0.0f);
	//		glCallList(dsWhiteDesign);

	//	}
	//}
	//glPopMatrix();

	/*glColor3f(0.7f, 0.3f, 0.2f);
	glPushMatrix();
	glTranslatef(-22.0f, 65.5f, 17.5f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(8.2f, 1.5f, 1.0f);
	drawCube(0.625f, 2.0f, 4.25f);
	glPopMatrix();*/

	glPushMatrix();
	{

		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(19.0f, 47.0f, 23.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.2f, 1.5f, 1.0f);

		glPushMatrix();
		glTranslatef(0.0f, 0.0f, 2.0f);
		glCallList(dsWhiteDesign);

		for (int i = 0; i < 3; i++)
		{
			glTranslatef(2.5f, 0.0f, 0.0f);
			glCallList(dsWhiteDesign);

		}
		glPopMatrix();
	}
	glPopMatrix();


	//////////////////////////////////////////////////////////////////////////////////
	glPushMatrix(); //right side central 1 front
	glTranslatef(-7.0f, 70.0f, 25.0f);
	glScalef(1.0f, 2.0f, 1.0f);
	drawPillarDesign();
	glPopMatrix();

////////////////////////////////////////////////////////////////////////////
}

void drawStrips(void)
{
	void drawCube(GLfloat, GLfloat, GLfloat);

	glPushMatrix();
	glTranslatef(0.0f, 47.5f, 7.5f);
	drawCube(5.0f, 160.0f, 5.0f);			//lower wall front strip
	glPopMatrix();

	//Maroon and dark yellow strips on left side
	glPushMatrix();
	glColor3f(0.89f, 0.77f, 0.28f);			//dark yellow
	glTranslatef(-50.0f, 49.375f, 10.625f);
	drawCube(1.25f, 60.0f, 1.25f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.5f, 0.0f, 0.0f);			//maroon
	glTranslatef(-50.0f, 48.125f, 10.3125f);
	drawCube(1.25f, 60.0f, 0.625f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.89f, 0.77f, 0.28f);
	glTranslatef(-50.0f, 46.875f, 10.625f);
	drawCube(1.25f, 60.0f, 0.3125f);
	glPopMatrix();

	//maroon and dark yellow strips on right side
	glPushMatrix();
	glColor3f(0.89f, 0.77f, 0.28f);
	glTranslatef(50.0f, 49.375f, 10.625f);
	drawCube(1.25f, 60.0f, 1.25f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.5f, 0.0f, 0.0f);
	glTranslatef(50.0f, 48.125f, 10.3125f);
	drawCube(1.25f, 60.0f, 0.625f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.89f, 0.77f, 0.28f);
	glTranslatef(50.0f, 46.875f, 10.625f);
	drawCube(1.25f, 60.0f, 0.3125f);
	glPopMatrix();

	//maroon and dark yellow strips on the front
	glPushMatrix();
	glColor3f(0.89f, 0.77f, 0.28f);
	glTranslatef(0.0f, 49.375f, 25.625f);
	drawCube(1.25f, 42.5f, 1.25f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.5f, 0.0f, 0.0f);
	glTranslatef(0.0f, 48.125f, 25.3125f);
	drawCube(1.25f, 41.25f, 0.625f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.89f, 0.77f, 0.28f);
	glTranslatef(0.0f, 46.875f, 25.15625f);
	drawCube(1.25f, 40.f, 0.3125);
	glPopMatrix();


	//---maroon and yellow strips on the gate shed
	//on left side

	glPushMatrix();
	glColor3f(0.89f, 0.77f, 0.28f);
	glTranslatef(-20.0f, 46.825f, 17.5f);
	drawCube(1.25f, 0.3125f, 15.0f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.5f, 0.0f, 0.0f);
	glTranslatef(-20.125f, 48.125f, 17.5f);
	drawCube(1.25f, 0.625f, 15.0f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.89f, 0.77f, 0.28f);
	glTranslatef(-20.625f, 49.325f, 17.5f);
	drawCube(1.25f, 1.25f, 15.0f);
	glPopMatrix();

	//on right side

	glPushMatrix();
	glColor3f(0.89f, 0.77f, 0.28f);
	glTranslatef(20.0f, 46.825f, 17.5f);
	drawCube(1.25f, 0.3125f, 15.0f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.5f, 0.0f, 0.0f);
	glTranslatef(20.125f, 48.125f, 17.5f);
	drawCube(1.25f, 0.625f, 15.0f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.89f, 0.77f, 0.28f);
	glTranslatef(20.625f, 49.325f, 17.5f);
	drawCube(1.25f, 1.25f, 15.0f);
	glPopMatrix();

}

void drawLowerLeftSideBlock(void)
{

	void drawCube(GLfloat, GLfloat, GLfloat);

	//Lower Shop 1 - left side
	glPushMatrix();
	gTranslateX = -77.5f;
	gTranslateY = 25.0f;
	gTranslateZ = 0.0f;
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	glColor3f(0.81f, 0.71f, 0.23f);
	drawCube(50.0f, 5.0f, 10.0f);

	glPushMatrix();
	{
		glTranslatef(42.5f, -5.0f, 4.0f);
		glScalef(1.25f, 1.5f, 1.0f);
		drawWhiteDoor();
	}
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	gTranslateX = -65.0f;
	gTranslateY = 42.5f;
	gTranslateZ = 0.0f;
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	glColor3f(0.81f, 0.71f, 0.23f);
	drawCube(15.0f, 20.0f, 10.0f);
	glPopMatrix();

	//lower shop 2 - left side
	glPushMatrix();
	gTranslateX = -50.0f;
	gTranslateY = 25.0f;
	gTranslateZ = 0.0f;
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	glColor3f(0.81f, 0.71f, 0.23f);
	drawCube(50.0f, 10.0f, 10.0f);

	glPushMatrix();
	{
		glTranslatef(-15.0f, -5.0f, 4.0f);
		glScalef(1.25f, 1.5f, 1.0f);
		drawWhiteDoor();
	}
	glPopMatrix();

	glPopMatrix();

	glPushMatrix();
	gTranslateX = -35.0f;
	gTranslateY = 42.5f;
	gTranslateZ = 0.0f;
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	glColor3f(0.81f, 0.71f, 0.23f);
	drawCube(15.0f, 20.0f, 10.0f);
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(0.0f, 13.0f, -5.0f);
		glScalef(1.0f, 1.3f, 1.0f);
		drawMainDoor();
	}
	glPopMatrix();
	
	glPushMatrix();
	gTranslateX = -22.5f;
	gTranslateY = 25.0f;
	gTranslateZ = 0.0f;
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	glColor3f(0.81f, 0.71f, 0.23f);
	drawCube(50.0f, 5.0f, 10.0f);
	glPopMatrix();
}

void drawLowerRightSideBlock(void)
{

	void drawCube(GLfloat, GLfloat, GLfloat);

	//Lower Shop 1 - right side
	glPushMatrix();
	gTranslateX = 77.5f;
	gTranslateY = 25.0f;
	gTranslateZ = 0.0f;
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	glColor3f(0.81f, 0.71f, 0.23f);
	drawCube(50.0f, 5.0f, 10.0f);

	glPushMatrix();
	{
		glTranslatef(-42.5f, -5.0f, 4.5f);
		glScalef(1.5f, 1.5f, 1.0f);
		drawShutter();
	}
	glPopMatrix();

	glPopMatrix();



	glPushMatrix();
	gTranslateX = 65.0f;
	gTranslateY = 42.5f;
	gTranslateZ = 0.0f;
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	glColor3f(0.81f, 0.71f, 0.23f);
	drawCube(15.0f, 20.0f, 10.0f);
	glPopMatrix();

	//lower shop 2 - right side
	glPushMatrix();
	gTranslateX = 50.0f;
	gTranslateY = 25.0f;
	gTranslateZ = 0.0f;
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	glColor3f(0.81f, 0.71f, 0.23f);
	drawCube(50.0f, 10.0f, 10.0f);

	glPushMatrix();
	{
		glTranslatef(15.0f, -5.0f, 4.5f);
		glScalef(1.5f, 1.5f, 1.0f);
		drawShutter();
	}
	glPopMatrix();

	glPopMatrix();

	glPushMatrix();
	gTranslateX = 35.0f;
	gTranslateY = 42.5f;
	gTranslateZ = 0.0f;
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	glColor3f(0.81f, 0.71f, 0.23f);
	drawCube(15.0f, 20.0f, 10.0f);
	glPopMatrix();

	glPushMatrix();
	gTranslateX = 22.5f;
	gTranslateY = 25.0f;
	gTranslateZ = 0.0f;
	glTranslatef(gTranslateX, gTranslateY, gTranslateZ);
	glColor3f(0.81f, 0.71f, 0.23f);
	drawCube(50.0f, 5.0f, 10.0f);
	glPopMatrix();

}

void DrawLeftTopTrapezoid(void)
{
	M3DVector3f vNormal;
	glBegin(GL_QUADS);
	{	// Left Face
		M3DVector3f vPoints[4] = { { -1.0f, 1.0f, 1.0f },
		{ -1.0f, 1.0f, -1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{ -1.0f, -1.0f, 1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Top Face
		M3DVector3f vPoints[4] = { { 1.0f, -1.0f, -1.0f },
		{ -1.0f, 1.0f, -1.0f },
		{ -1.0f, 1.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{	// Bottom Face
		M3DVector3f vPoints[4] = { { 1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{ 1.0f, -1.0f, -1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}
	glEnd();

	glBegin(GL_TRIANGLES);
	{
		// Front face
		M3DVector3f vPoints[3] = { { -1.0f, 1.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
	}
	{
		//Back face
		M3DVector3f vPoints[3] = { { -1.0f, 1.0f, -1.0f },
		{ 1.0f, -1.0f, -1.0f },
		{ -1.0f, -1.0f, -1.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
	}
	glEnd();
}


///////////////////////////////////////////////////////////////////////////////////////
// Ashish Khare

void DrawClassRoom(void)
{
	//Function Declaration
	void DrawFrontWall(void);
	void DrawMiddleArch(void);
	void DrawLeftSideWall(void);
	void DrawRightSideWall(void);
	void DrawCarpate(void);
	void DrawClassroomRoof(void);

	glPushMatrix();
	{
		glTranslatef(100.0f, 0.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
		DrawLeftSideWall();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(0.0f, 0.0f, 85.0f);
		glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
		DrawFrontWall();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-100.0f, 0.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
		DrawRightSideWall();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(0.0f, 0.0f, -75.0f);                           //Entry Door
		glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
		DrawFrontWall();
	}
	glPopMatrix();

	///////////Sujay ///////////


	glPushMatrix();                               //Bookshelf
	{
		glTranslatef(-95.0f, -10.0f, -40.0f);
		glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.6f, 2.0f, 2.0f);
		drawBookShelf();
	}
	glPopMatrix();

	glPushMatrix();                               //Bookshelf
	{
		glTranslatef(95.0f, -10.0f, -40.0f);
		glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.6f, 2.0f, 2.0f);
		drawBookShelf();
	}
	glPopMatrix();

	glPushMatrix();                                  //Middle Windows
	{
		glTranslatef(0.0f, -16.0f, -75.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glScalef(2.2f, 2.4f, 3.0f);
		drawWindowHall();
		glPushMatrix();
		{
			glColor3f(0.86f, 0.85f, 0.50f);
			glTranslatef(0.0f, 4.5f, 0.0f);
			drawCube(5.0f, 25.0f, 2.0f);
		}
		glPopMatrix();
		
		glTranslatef(0.0f, -12.0f, -53.0f);
		drawWindowHallBackside();
		glPushMatrix();
		{
			glColor3f(0.86f, 0.85f, 0.50f);
			glTranslatef(0.0f, 4.5f, 0.0f);
			drawCube(5.0f, 25.0f, 2.0f);
		}
		glPopMatrix();

		
	}
	glPopMatrix();

	glPushMatrix();										//Balcony Doors
	{
		glPushMatrix();									//One side
		{
			glTranslatef(-127.0f, -15.7f, -80.0f);
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glScalef(2.2f, 2.4f, 3.0f);

			drawDoorSide();
		}
		glPopMatrix();

		glPushMatrix();									//One side
		{
			glTranslatef(-177.0f, -16.0f, -80.0f);
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glScalef(2.2f, 2.4f, 3.0f);

			drawDoorSide();
		}
		glPopMatrix();

		glPushMatrix();									//One side
		{
			glTranslatef(-230.0f, -20.0f, -80.0f);
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glScalef(1.9f, 1.9f, 2.0f);

			drawDoorSide();
		}
		glPopMatrix();

		glPushMatrix();									//One side
		{
			glTranslatef(127.0f, -15.7f, -80.0f);
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glScalef(2.2f, 2.4f, 3.0f);

			drawDoorSide();
		}
		glPopMatrix();

		glPushMatrix();									//One side
		{
			glTranslatef(177.0f, -16.0f, -80.0f);
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glScalef(2.2f, 2.4f, 3.0f);

			drawDoorSide();
		}
		glPopMatrix();

		glPushMatrix();									//One side
		{
			glTranslatef(230.0f, -20.0f, -80.0f);
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glScalef(1.9f, 1.9f, 2.0f);

			drawDoorSide();
		}
		glPopMatrix();

	}
	glPopMatrix();


	glPushMatrix();											//Fans new
	{
		glTranslatef(0.0f, 35.0f, 65.0f);
		glScalef(0.6f, 0.6f, 0.6f);
		drawFan();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(45.0f, 35.0f, 30.0f);
		glScalef(0.6f, 0.6f, 0.6f);
		drawFan();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-45.0f, 35.0f, 30.0f);
		glScalef(0.6f, 0.6f, 0.6f);
		drawFan();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(45.0f, 35.0f, -30.0f);
		glScalef(0.6f, 0.6f, 0.6f);
		drawFan();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-45.0f, 35.0f, -30.0f);
		glScalef(0.6f, 0.6f, 0.6f);
		drawFan();
	}
	glPopMatrix();

	glPushMatrix();												//Doors
	{
		glTranslatef(70.0f, -17.0f, 85.0f);
		glScalef(3.0f, 2.3f, 3.0f);
		drawDoor();
		//glCallList(dsClassDoor);
		glPushMatrix();
		{
			glColor3f(0.86f, 0.85f, 0.50f);
			glTranslatef(0.0f, 16.5f, 0.0f);
			drawCube(5.0f, 10.0f, 2.0f);
		}
		glPopMatrix();

		glTranslatef(0.0f, 0.0f, -53.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		drawDoor();
		//glCallList(dsClassDoor);
		glPushMatrix();
		{
			glColor3f(0.86f, 0.85f, 0.50f);
			glTranslatef(0.0f, 16.5f, 0.0f);
			drawCube(5.0f, 10.0f, 2.0f);
		}
		glPopMatrix();
	}
	glPopMatrix();

	glPushMatrix();												//Doors
	{
		glTranslatef(-70.0f, -17.0f, 85.0f);
		glScalef(3.0f, 2.3f, 3.0f);
		drawDoor();
		//glCallList(dsClassDoor);
		glPushMatrix();
		{
			glColor3f(0.86f, 0.85f, 0.50f);
			glTranslatef(0.0f, 16.5f, 0.0f);
			drawCube(5.0f, 10.0f, 2.0f);
		}
		glPopMatrix();

		glTranslatef(0.0f, 0.0f, -53.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		drawDoor();
		//glCallList(dsClassDoor);
		glPushMatrix();
		{
			glColor3f(0.86f, 0.85f, 0.50f);
			glTranslatef(0.0f, 16.5f, 0.0f);
			drawCube(5.0f, 10.0f, 2.0f);
		}
		glPopMatrix();
	}
	glPopMatrix();

	glPushMatrix();                               //Doors
	{
		glTranslatef(100.0f, -17.0f, 35.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		glScalef(3.0f, 2.3f, 3.0f);
		drawDoor();
		//glCallList(dsClassDoor);
		glPushMatrix();
		{
			glColor3f(0.86f, 0.85f, 0.50f);
			glTranslatef(0.0f, 16.5f, 0.0f);
			drawCube(5.0f, 10.0f, 2.0f);
		}
		glPopMatrix();
	}
	glPopMatrix();

	glPushMatrix();                               //Doors
	{
		glTranslatef(-100.0f, -17.0f, 35.0f);
		glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
		glScalef(3.0f, 2.3f, 3.0f);
		drawDoor();
		//glCallList(dsClassDoor);
		glPushMatrix();
		{
			glColor3f(0.86f, 0.85f, 0.50f);
			glTranslatef(0.0f, 16.5f, 0.0f);
			drawCube(5.0f, 10.0f, 2.0f);
		}
		glPopMatrix();
	}
	glPopMatrix();

	

	glPushMatrix();
	{
		glTranslatef(0.0f, -5.1f, 0.0f);
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		DrawClassroomRoof();
	}
	glPopMatrix();

}

void DrawFrontWall(void)
{
	void DrawMiddleArch(GLfloat, GLfloat, GLfloat, GLfloat, int, int);
	void DrawShape(GLfloat, GLfloat, GLint);

	GLfloat height = 40.0f;
	GLfloat width = 15.0f;
	GLfloat depth = 5.0f;

	//glRotatef(0, 0.0f, 0.0f, 0.0f);

	glPushMatrix();
	{
		glTranslatef(-40.0f, 0.0f, 0.0f);
		//glRotatef(180.0f,0.0f,1.0f,0.0f);

		glBegin(GL_QUADS);
		//Front Wall - middle part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();

	//glLoadIdentity();
	glPushMatrix();
	{
		glTranslatef(40.0f, 0.0f, 0.0f);
		//glRotatef(180.0f,0.0f,1.0f,0.0f);
		glBegin(GL_QUADS);
		//Front Wall - left part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();

	//glLoadIdentity();

	height = 40.0f;
	width = 5.0f;
	depth = 5.0f;
	glPushMatrix();
	{
		glTranslatef(-90.0f, 0.0f, 0.0f);
		//glRotatef(180.0f,0.0f,1.0f,0.0f);
		glBegin(GL_QUADS);
		//Front Wall - middle part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();


	//glLoadIdentity();

	height = 40.0f;
	width = 5.0f;
	depth = 5.0f;
	glPushMatrix();
	{
		glTranslatef(90.0f, 0.0f, 0.0f);
		//glRotatef(180.0f,0.0f,1.0f,0.0f);
		glBegin(GL_QUADS);
		//Front Wall - middle part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();

	//Middle Arch 

	//glLoadIdentity();

	glPushMatrix();
	{
		glTranslatef(0.0f, -10.0f, 0.0f);
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
		//DrawMiddleArch(-89.0f,5.0f,120.0f,2.0f,10,30);
		glColor3f(0.96f, 0.95f, 0.60f);
		DrawShape(55.0f, 25.0f, 250);
	}
	glPopMatrix();

	//left arch
	//glLoadIdentity();

	height = 40.0f;
	width = 5.0f;
	depth = 5.0f;

	glPushMatrix();
	{
		glTranslatef(-70.0f, -10.0f, 0.0f);
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
		//DrawMiddleArch(-20.0f,-2.0f,120.0f,2.0f,10,25);
		glColor3f(0.96f, 0.95f, 0.60f);
		DrawShape(35.0f, 10.0f, 50);
	}
	glPopMatrix();


	//right arch
	//glLoadIdentity();
	height = 40.0f;
	width = 5.0f;
	depth = 5.0f;
	glPushMatrix();
	{
		glTranslatef(70.0f, -10.0f, 0.0f);
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
		//DrawMiddleArch(-158.0f,-2.0f,120.0f,2.0f,10,25);
		glColor3f(0.96f, 0.95f, 0.60f);
		DrawShape(35.0f, 10.0f, 50);
	}
	glPopMatrix();

	//Middle Arc Filling
	height = 2.9f;
	width = 25.0f;
	depth = 5.0f;

	glPushMatrix();
	{
		//glLoadIdentity();
		glTranslatef(0.0f, -37.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);
		//Front Wall - middle part
		//top face

		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 1.0f, 0.0f); //normals for top face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, -1.0f, 0.0f); //normals for bottom face
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 0.0f, 1.0f); //normals for front face
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 0.0f, -1.0f); //normals for back face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(1.0f, 0.0f, 0.0f); //normals for right face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube

		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(-1.0f, 0.0f, 0.0f); //normals for right face
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();


	//left arc filling
	height = 10.0f;
	width = 15.0f;
	depth = 5.0f;

	glPushMatrix();
	{
		//glLoadIdentity();
		glTranslatef(70.0f, -30.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);
		//Front Wall - middle part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 1.0f, 0.0f); //normals for top face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f); //normals for bottom face
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 0.0f, 1.0f); //normals for top face
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 0.0f, -1.0f); //normals for back face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(1.0f, 0.0f, 0.0f); //normals for right face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(-1.0f, 0.0f, 0.0f); //normals for left face
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();


	//right arc filling
	height = 10.0f;
	width = 15.0f;
	depth = 5.0f;

	//glLoadIdentity();

	glPushMatrix();
	{
		glTranslatef(-70.0f, -30.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);

		//Front Wall - middle part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();

}

void DrawLeftSideWall(void)
{
	void DrawMiddleArch(GLfloat, GLfloat, GLfloat, GLfloat, int, int);
	void DrawShape(GLfloat, GLfloat, GLint);

	GLfloat height = 40.0f;
	GLfloat width = 45.0f;
	GLfloat depth = 5.0f;

	glRotatef(-90, 0.0f, 1.0f, 0.0f);

	glPushMatrix();
	{
		glTranslatef(-25.0f, 0.0f, 0.0f);
		//glRotatef(180.0f,0.0f,1.0f,0.0f);

		glBegin(GL_QUADS);
		//Front Wall - middle part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();

	//glLoadIdentity();
	glPushMatrix();
	{
		height = 40.0f;
		width = 15.0f;
		depth = 5.0f;

		glTranslatef(65.0f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
		//Front Wall - left part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();

	//right arch
	//glLoadIdentity();
	height = 40.0f;
	width = 5.0f;
	depth = 5.0f;
	glPushMatrix();
	{
		glTranslatef(35.0f, -10.0f, 0.0f);
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
		//DrawMiddleArch(-158.0f,-2.0f,120.0f,2.0f,10,25);
		glColor3f(0.96f, 0.95f, 0.60f);
		DrawShape(35.0f, 10.0f, 50);
	}
	glPopMatrix();

	//left arc filling
	height = 10.0f;
	width = 15.0f;
	depth = 5.0f;

	glPushMatrix();
	{
		//glLoadIdentity();
		glTranslatef(35.0f, -30.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);
		//Front Wall - middle part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 1.0f, 0.0f); //normals for top face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f); //normals for bottom face
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 0.0f, 1.0f); //normals for top face
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 0.0f, -1.0f); //normals for back face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(1.0f, 0.0f, 0.0f); //normals for right face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(-1.0f, 0.0f, 0.0f); //normals for left face
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();

	////Draw Cupboard

	//glPushMatrix();
	//{
	//	height = 25.0f;
	//	width = 12.0f;
	//	depth = 5.0f;
	//	//glLoadIdentity();
	//	glTranslatef(-40.0f, 8.0f, -10.0f);
	//	glLineWidth(10.0f);
	//	//glRotatef(180.0f,0.0f,1.0f,0.0f);
	//	glBegin(GL_LINES);
	//	//Front Wall - middle part
	//	//top face
	//	glColor3f(0.39f, 0.19f, 0.0f);
	//	glNormal3f(0.0f, 0.0f, -1.0f); //normals for top face
	//	glVertex3f(width, height, depth);
	//	glVertex3f(-width, height, depth);

	//	glVertex3f(-width, height, depth);
	//	glVertex3f(-width, -height, depth);

	//	glVertex3f(-width, -height, depth);
	//	glVertex3f(width, -height, depth);

	//	glVertex3f(width, -height, depth);
	//	glVertex3f(width, height, depth);
	//	glEnd();
	//}
	//glPopMatrix();


	////Draw Photo Frame

	//glPushMatrix();
	//{
	//	height = 20.0f;
	//	width = 10.0f;
	//	depth = 5.0f;
	//	//glLoadIdentity();
	//	glTranslatef(-4.0f, -10.0f, -10.0f);
	//	glLineWidth(10.0f);
	//	//glRotatef(180.0f,0.0f,1.0f,0.0f);
	//	glBegin(GL_QUADS);
	//	//Front Wall - middle part
	//	//top face
	//	glColor3f(0.39f, 0.19f, 0.0f);
	//	glNormal3f(0.0f, 0.0f, -1.0f); //normals for top face
	//	glVertex3f(width, height, depth);
	//	glVertex3f(-width, height, depth);
	//	glVertex3f(-width, -height, depth - 5);
	//	glVertex3f(width, -height, depth - 5);

	//	glEnd();
	//}
	//glPopMatrix();

	////Lines below photograph

	//glPushMatrix();
	//{
	//	height = 5.0f;
	//	width = 12.0f;
	//	depth = 5.0f;
	//	//glLoadIdentity();
	//	glTranslatef(-4.0f, 6.0f, -10.0f);
	//	glLineWidth(10.0f);
	//	//glRotatef(180.0f,0.0f,1.0f,0.0f);
	//	glBegin(GL_LINES);
	//	//Front Wall - middle part
	//	//top face
	//	glColor3f(0.39f, 0.19f, 0.0f);
	//	glNormal3f(0.0f,0.0f,-1.0f); //normals for top face
	//	glVertex3f(width, height, depth);
	//	glVertex3f(-width, height, depth);

	//	glEnd();
	//}
	//glPopMatrix();

	//glPushMatrix();
	//height = 5.0f;
	//width = 11.0f;
	//depth = 5.0f;
	////glLoadIdentity();
	//glTranslatef(-4.0f, 8.0f, -10.0f);
	//glLineWidth(10.0f);
	////glRotatef(180.0f,0.0f,1.0f,0.0f);
	//glBegin(GL_LINES);
	////Front Wall - middle part
	////top face
	//glColor3f(0.39f, 0.19f, 0.0f);
	////glNormal3f(0.0f,1.0f,0.0f); //normals for top face
	//glVertex3f(width, height, depth);
	//glVertex3f(-width, height, depth);

	//glEnd();
	//glPopMatrix();
}

void DrawRightSideWall(void)
{
	void DrawMiddleArch(GLfloat, GLfloat, GLfloat, GLfloat, int, int);
	void DrawShape(GLfloat, GLfloat, GLint);

	GLfloat height = 40.0f;
	GLfloat width = 45.0f;
	GLfloat depth = 5.0f;

	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

	glPushMatrix();
	{
		glTranslatef(25.0f, 0.0f, 0.0f);
		//glRotatef(90.0f,0.0f,1.0f,0.0f);

		glBegin(GL_QUADS);
		//Front Wall - middle part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();

	glPushMatrix();
	{
		height = 40.0f;
		width = 15.0f;
		depth = 5.0f;

		glTranslatef(-65.0f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
		//Front Wall - left part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();

	//right arch
	//glLoadIdentity();
	height = 40.0f;
	width = 5.0f;
	depth = 5.0f;
	glPushMatrix();
	{
		glTranslatef(-35.0f, -10.0f, 0.0f);
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
		//DrawMiddleArch(-158.0f,-2.0f,120.0f,2.0f,10,25);
		glColor3f(0.96f, 0.95f, 0.60f);
		DrawShape(35.0f, 10.0f, 50);
	}
	glPopMatrix();

	//left arc filling
	height = 10.0f;
	width = 15.0f;
	depth = 5.0f;
	glPushMatrix();
	{
		//glLoadIdentity();
		glTranslatef(-35.0f, -30.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);
		//Front Wall - middle part
		//top face
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 1.0f, 0.0f); //normals for top face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, height, depth); //left-bottom
		glVertex3f(width, height, depth); //right-bottom

										  //Bottom Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);
		glNormal3f(0.0f, -1.0f, 0.0f); //normals for bottom face
		glVertex3f(width, -height, -depth); //right-top
		glVertex3f(-width, -height, -depth); //left-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Front-Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 0.0f, 1.0f); //normals for top face
		glVertex3f(width, height, depth); //right-top
		glVertex3f(-width, height, depth); //lef-top
		glVertex3f(-width, -height, depth); //left-bottom
		glVertex3f(width, -height, depth); //right-bottom

										   //Back Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(0.0f, 0.0f, -1.0f); //normals for back face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Right Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(1.0f, 0.0f, 0.0f); //normals for right face
		glVertex3f(width, height, -depth); //right-top
		glVertex3f(width, height, depth); //left-top
		glVertex3f(width, -height, depth); //left-bottom
		glVertex3f(width, -height, -depth); //right-bottom

											//Left Face of Cube
		glColor3f(0.96f, 0.95f, 0.60f);

		glNormal3f(-1.0f, 0.0f, 0.0f); //normals for left face
		glVertex3f(-width, height, depth); //right-top
		glVertex3f(-width, height, -depth); //left-top
		glVertex3f(-width, -height, -depth); //left-bottom
		glVertex3f(-width, -height, depth); //right-bottom
		glEnd();
	}
	glPopMatrix();

	
}

void DrawCarpate(void)
{
	//glLoadIdentity();

	glPushMatrix();
	//glTranslatef(0.0f,0.0f,0.0f);
	glBegin(GL_QUADS);
	glColor3f(0.0f, 0.70f, 0.35f);
	glVertex3f(90.0f, 75.0f, -40.0f);
	glVertex3f(-90.0f, 75.0f, -40.0f);
	glVertex3f(-90.0f, -75.0f, -40.0f);
	glVertex3f(90.0f, -75.0f, -40.0f);
	glEnd();
	glPopMatrix();
}

void DrawClassroomRoof(void)
{
	void DrawConcentricRectangles(GLfloat, GLfloat);
	void DrawConcentricRectangles_left(GLfloat, GLfloat);

	GLint i = 0;
	GLfloat x = 0.0f, y = 0.0f;

	glPushMatrix();
	//glTranslatef(0.0f,0.0f,0.0f);
	glBegin(GL_QUADS);
	glColor3f(0.96f, 0.95f, 0.60f);
	//glColor3f(0.412f, 0.412f, 0.412f);	//grey
	glVertex3f(90.0f, 75.0f, 40.0f);
	glVertex3f(-90.0f, 75.0f, 40.0f);
	glVertex3f(-90.0f, -75.0f, 40.0f);
	glVertex3f(90.0f, -75.0f, 40.0f);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_LINES);
	//glColor3f(0.0f, 0.0f, 0.0f);
	glColor3f(0.412f, 0.412f, 0.412f);	//grey
	glVertex3f(90.0f, 30.0f, 40.0f);
	glVertex3f(-90.0f, 30.0f, 40.0f);

	glVertex3f(90.0f, -30.0f, 40.0f);
	glVertex3f(-90.0f, -30.0f, 40.0f);

	glVertex3f(40.0f, 75.0f, 40.0f);
	glVertex3f(40.0f, -75.0f, 40.0f);

	glVertex3f(-40.0f, 75.0f, 40.0f);
	glVertex3f(-40.0f, -75.0f, 40.0f);
	glEnd();
	glPopMatrix();

	glTranslatef(0.0f, 0.0f, 40.0f);
	glScalef(0.5f, 0.5f, 0.5f);

	glPushMatrix();
	glScalef(1.1f, 0.9f, 0.0f);
	glRotatef(45.0f, 0, 0, 1);

	//glScalef(0.0f,20.0f,0.0f);
	for (int i = 0; i < 8; i++) {
		x = x + 5.0f;
		y = y + 5.0f;
		DrawConcentricRectangles(x, y);
	}
	glPopMatrix();

	x = 0.0f, y = 0.0f;

	glPushMatrix();
	glScalef(1.1f, 0.9f, 0.0f);
	glTranslatef(-120.0f, 0.0f, 0.0f);
	glRotatef(45.0f, 0, 0, 1);

	//glScalef(0.0f,20.0f,0.0f);
	for (int i = 0; i < 8; i++) {
		x = x + 5.0f;
		y = y + 5.0f;
		DrawConcentricRectangles(x, y);
	}
	glPopMatrix();


	x = 0.0f, y = 0.0f;

	glPushMatrix();
	glScalef(1.1f, 0.9f, 0.0f);
	glTranslatef(120.0f, 0.0f, 0.0f);
	glRotatef(45.0f, 0, 0, 1);

	//glScalef(0.0f,20.0f,0.0f);
	for (int i = 0; i < 8; i++) {
		x = x + 5.0f;
		y = y + 5.0f;
		DrawConcentricRectangles(x, y);
	}
	glPopMatrix();


	x = 0.0f, y = 0.0f;

	glPushMatrix();
	glScalef(1.1f, 0.9f, 0.0f);
	glTranslatef(0.0f, -120.0f, 0.0f);
	glRotatef(45.0f, 0, 0, 1);

	//glScalef(0.0f,20.0f,0.0f);
	for (int i = 0; i < 8; i++) {
		x = x + 5.0f;
		y = y + 5.0f;
		DrawConcentricRectangles(x, y);
	}
	glPopMatrix();


	x = 0.0f, y = 0.0f;

	glPushMatrix();
	glScalef(1.1f, 0.9f, 0.0f);
	glTranslatef(60.0f, -60.0f, 0.0f);
	glRotatef(45.0f, 0, 0, 1);

	//glScalef(0.0f,20.0f,0.0f);
	for (int i = 0; i < 8; i++) {
		x = x + 5.0f;
		y = y + 5.0f;
		DrawConcentricRectangles(x, y);
	}
	glPopMatrix();

	x = 0.0f, y = 0.0f;

	glPushMatrix();
	glScalef(1.1f, 0.9f, 0.0f);
	glTranslatef(-60.0f, 60.0f, 0.0f);
	glRotatef(45.0f, 0, 0, 1);

	//glScalef(0.0f,20.0f,0.0f);
	for (int i = 0; i < 8; i++) {
		x = x + 5.0f;
		y = y + 5.0f;
		DrawConcentricRectangles(x, y);
	}
	glPopMatrix();

	x = 0.0f, y = 0.0f;

	glPushMatrix();
	glScalef(1.1f, 0.9f, 0.0f);
	glTranslatef(0.0f, 120.0f, 0.0f);
	glRotatef(45.0f, 0, 0, 1);

	//glScalef(0.0f,20.0f,0.0f);
	for (int i = 0; i < 8; i++) {
		x = x + 5.0f;
		y = y + 5.0f;
		DrawConcentricRectangles(x, y);
	}
	glPopMatrix();

	x = 0.0f, y = 0.0f;

	glPushMatrix();
	glScalef(1.1f, 0.9f, 0.0f);
	glTranslatef(-60.0f, -60.0f, 0.0f);
	glRotatef(45.0f, 0, 0, 1);

	//glScalef(0.0f,20.0f,0.0f);
	for (int i = 0; i < 8; i++) {
		x = x + 5.0f;
		y = y + 5.0f;
		DrawConcentricRectangles(x, y);
	}
	glPopMatrix();

	x = 0.0f, y = 0.0f;

	glPushMatrix();
	glScalef(1.1f, 0.9f, 0.0f);
	glTranslatef(60.0f, 60.0f, 0.0f);
	glRotatef(45.0f, 0, 0, 1);

	//glScalef(0.0f,20.0f,0.0f);
	for (int i = 0; i < 8; i++) {
		x = x + 5.0f;
		y = y + 5.0f;
		DrawConcentricRectangles(x, y);
	}
	glPopMatrix();
}

void DrawConcentricRectangles(GLfloat x, GLfloat y)
{


	glBegin(GL_LINES);
	//glColor3f(0.0f, 0.0f, 0.0f);
	glColor3f(0.412f, 0.412f, 0.412f);	//grey
	glVertex3f(-x, y, 40.0f);
	glVertex3f(-x, -y, 40.0f);

	glVertex3f(-x, -y, 40.0f);
	glVertex3f(x, -y, 40.0f);

	glVertex3f(x, -y, 40.0f);
	glVertex3f(x, y, 40.0f);

	glVertex3f(x, y, 40.0f);
	glVertex3f(-x, y, 40.0f);

	glEnd();
}


void DrawConcentricRectangles_left(GLfloat x, GLfloat y)
{
	glBegin(GL_LINES);
	//glColor3f(0.0f, 0.0f, 0.0f);
	glColor3f(0.412f, 0.412f, 0.412f);	//grey
	glVertex3f(-x, y, 40.0f);
	glVertex3f(-x, -y, 40.0f);

	glVertex3f(-x, -y, 40.0f);
	glVertex3f(x, -y, 40.0f);

	//glVertex3f(x,-y,40.0f);
	//glVertex3f(x,y,40.0f);

	//glVertex3f(x,y,40.0f);
	//glVertex3f(-x,y,40.0f);

	glEnd();
}

void DrawMiddleArch(GLfloat cx, GLfloat cy, GLfloat start_angle, GLfloat arc_angle, int num_segments, int arc_length)
{

	GLfloat theta = arc_angle / float(num_segments - 1);

	GLfloat tangetial_factor = tanf(theta);
	GLfloat radial_factor = cosf(theta);

	int r = arc_length;

	GLfloat x = r * cosf(start_angle);
	GLfloat y = r * sinf(start_angle);

	glLineWidth(20.0f);
	glBegin(GL_LINE_STRIP);
	for (int ii = 0; ii <= num_segments; ii++)
	{
		glColor3f(1.0f, 1.0f, 1.0f); //milk-yellow
		glVertex3f(x + cx, y + cy, 0.0f);

		GLfloat tx = -y;
		GLfloat ty = x;

		x += tx * tangetial_factor;
		y += ty * tangetial_factor;

		x *= radial_factor;
		y *= radial_factor;
	}

	glEnd();

	glBegin(GL_LINE);
	glVertex3f(x + cx, y + cy, 0.0f);
	glVertex3f(x + cx, y + cy, -20);
	glEnd();

}

void DrawShape(GLfloat width, GLfloat height, GLint slices)
{
	GLfloat depth = 5.0f;
	glPushMatrix();
	{
		GLfloat offset = 0.5f;
		glScalef(width / 2, height / (1 + offset), 0.99f);
		glBegin(GL_QUADS);
		for (int i = 0; i < slices; ++i) {
			float curAngle = ((i + 0.0f) / (float)slices) * 3.14159f;
			float nxtAngle = ((i + 1.0f) / (float)slices) * 3.14159f;
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(cos(curAngle), sin(curAngle), depth);
			glVertex3f(cos(curAngle), 1.0f + offset, depth);
			glVertex3f(cos(nxtAngle), 1.0f + offset, depth);
			glVertex3f(cos(nxtAngle), sin(nxtAngle), depth);

			glNormal3f(.0f, 0.0f, -1.0f);
			glVertex3f(cos(curAngle), sin(curAngle), -depth);
			glVertex3f(cos(curAngle), 1.0f + offset, -depth);
			glVertex3f(cos(nxtAngle), 1.0f + offset, -depth);
			glVertex3f(cos(nxtAngle), sin(nxtAngle), -depth);

			{
				M3DVector3f vNormal;
				M3DVector3f vPoints[4] = { { cos(curAngle), sin(curAngle), depth },{ cos(nxtAngle), sin(nxtAngle), depth },{ cos(nxtAngle), sin(nxtAngle), -depth },{ cos(curAngle), sin(curAngle), -depth } };

				m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
				glNormal3fv(vNormal);

				glVertex3fv(vPoints[0]);
				glVertex3fv(vPoints[1]);
				glVertex3fv(vPoints[2]);
				glVertex3fv(vPoints[3]);
			}
		}
		glEnd();
	}
	glPopMatrix();
}
///////////////////

void drawIntegratedClassRoom(void)
{
	void DrawRoof(void);
	void QuarterCircularStructure(void);
	/////////////////////////////////////////
	// Draw main class room
	glPushMatrix();
	{
		glTranslatef(0.0f, 50.0f, -50.0f);
		glScalef(0.4f, 0.4f, 0.4f);
		DrawClassRoom();
	}
	glPopMatrix();

	// classroom floor
	glPushMatrix();
		//glColor3f(1.0f, 1.0f, 1.0f);
		//glColor3f(0.184f, 0.310f, 0.310f);	// Dark slate gray
		glColor3f(0.333f, 0.420f, 0.184f);	// Dark Olive Green
		glTranslatef(0.0f, 34.05f, -50.0f);
		glCallList(dsClassFloor);
	glPopMatrix();

	// classroom floor border

	glPushMatrix();
		//glColor3f(0.941f, 0.973f, 1.0f);	// Alice blue
		glColor3f(0.098f, 0.098f, 0.439f);	// midnight blue
		glTranslatef(0.0f, 33.57f, -48.0f);
		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		drawWindowBorder(30.0f, 40.0f, 0.5f, 5.0f);
	glPopMatrix();

	// Draw the slab
	glPushMatrix();
	{
		glColor3f(0.8f, 0.8f, 0.8f);
		glTranslatef(0.0f, 30.5f, -50.0f);
		glScalef(100.0f, 3.5f, 35.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	// Draw the classroom roof (temporary)
	glPushMatrix();
	{
		glColor3f(0.96f, 0.95f, 0.60f);
		glTranslatef(0.0f, 72.0f, -50.0f);
		glScalef(105.0f, 8.0f, 32.1f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	// Draw front side balcony walls adjacent to classroom
	glPushMatrix();
	{
		glColor3f(0.96f, 0.95f, 0.60f);
		glTranslatef(60.0f, 50.0f, -16.0f);
		glScalef(22.0f, 16.0f, 2.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.96f, 0.95f, 0.60f);
		glTranslatef(-60.0f, 50.0f, -16.0f);
		glScalef(22.0f, 16.0f, 2.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.96f, 0.95f, 0.60f);
		glTranslatef(0.0f, 71.0f, -16.0f);
		glScalef(80.0f, 5.0f, 2.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	// Draw the inner side balcony walls adjacent to classroom
	glPushMatrix();
	{
		glColor3f(0.96f, 0.95f, 0.60f);
		glTranslatef(70.0f, 50.0f, -80.0f);
		glScalef(32.0f, 16.0f, 2.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.96f, 0.95f, 0.60f);
		glTranslatef(-70.0f, 50.0f, -80.0f);
		glScalef(32.0f, 16.0f, 2.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	////////////////////////////////////////
	// Curved surfaces over windows (experimantal)
	// Right side first floor
	glColor3f(0.91f, 0.78f, 0.25f);
	glPushMatrix();
	{
		glTranslatef(94.0f, 55.0, 0.5f);
		DrawShape(18, 10, 50);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(116.5f, 55.0, 0.5f);
		DrawShape(18, 10, 50);
	}
	glPopMatrix();

	// Right side second floor
	glPushMatrix();
	{
		glTranslatef(94.0f, 90.0, 0.5f);
		DrawShape(18, 10, 50);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(116.5f, 90.0, 0.5f);
		DrawShape(18, 10, 50);
	}
	glPopMatrix();

	// Left side first floor
	glPushMatrix();
	{
		glTranslatef(-94.0f, 55.0, 0.5f);
		DrawShape(18, 10, 50);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-116.5f, 55.0, 0.5f);
		DrawShape(18, 10, 50);
	}
	glPopMatrix();

	// Left side second floor
	glPushMatrix();
	{
		glTranslatef(-96.0f, 90.0, 0.57f);
		DrawShape(10, 8, 50);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-113.5f, 90.0, 0.57f);
		DrawShape(10, 8, 50);
	}
	glPopMatrix();

	// slab beneath the inner balcony
	glPushMatrix();
	{
		glColor3f(0.941f, 0.902f, 0.549f);	// khaki
		glTranslatef(0.0f, 33.5f, -93.0f);
		glScalef(103.20f, 0.30f, 8.09f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.941f, 0.902f, 0.549f);	// khaki
		glTranslatef(0.0f, 33.5f, -107.0f);
		glScalef(37.0f, 0.30f, 6.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-36.0f, 33.0f, -98.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		for (GLfloat i = 0.0f; i < 145.0f; i += 4.0f)
		{
			glPushMatrix();
			glScalef(0.5f, 1.0f, 13.0f);
			glTranslatef(i, 0.0f, 0.0f);
			glCallList(dsCubeRegular);
			glPopMatrix();
		}
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	// dark wood
		glTranslatef(0.0f, 33.0f, -112.0f);
		glScalef(102.0f, 2.0f, 1.10f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	// dark wood
		glTranslatef(0.0f, 35.0f, -114.0f);
		glScalef(100.50f, 0.40f, 1.40f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-90.0f, 0.0f, 0.0f);
		for (GLfloat i = 0.0f; i < 180.0f; i += 2.0f)
		{
			glPushMatrix();
			{
				glTranslatef(i, 34.0f, -114.0f);
				glScalef(3.0f, 3.0f, 3.0f);
				glCallList(dsBalconyDesign);
			}
			glPopMatrix();
		}
	}
	glPopMatrix();

	// Newel Caps
	// left stair area
	glColor3f(0.52f, 0.37f, 0.26f);	// dark wood
	glPushMatrix();
	{
		glTranslatef(-21.0f, 17.0f, -110.30f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-21.0f, 17.0f, -96.60f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-22.80f, 17.00f, -111.80f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-43.50f, 17.0f, -112.0f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-31.50f, 14.50f, -110.80f);
		glScalef(0.60f, 0.70f, 0.70f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-31.50f, 14.50f, -101.30f);
		glScalef(0.60f, 0.70f, 0.70f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-67.0f, 44.0f, -101.0f);
		glScalef(1.0f, 1.0f, 1.70f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-36.0f, 44.0f, -100.50f);
		glScalef(1.0f, 1.0f, 1.70f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-67.0f, 44.0f, -110.50f);
		glScalef(1.0f, 1.0f, 1.70f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	// right stair area
	glPushMatrix();
	{
		glTranslatef(19.70f, 17.0f, -110.30f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(19.70f, 17.0f, -96.60f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(21.60f, 17.00f, -111.80f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(42.30f, 17.0f, -112.0f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(31.50f, 14.50f, -110.80f);
		glScalef(0.60f, 0.70f, 0.70f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(31.50f, 14.50f, -101.30f);
		glScalef(0.60f, 0.70f, 0.70f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(67.0f, 44.0f, -101.0f);
		glScalef(1.0f, 1.0f, 1.70f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(36.0f, 44.0f, -100.50f);
		glScalef(1.0f, 1.0f, 1.70f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(67.0f, 44.0f, -110.50f);
		glScalef(1.0f, 1.0f, 1.70f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	////////////////////////////////////
	glPushMatrix();
	{
		glTranslatef(90.66f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(69.16f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(66.66f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(45.5f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(42.5f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(21.5f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(18.5f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-91.66f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();


	glPushMatrix();
	{
		glTranslatef(-70.5f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-67.5f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-46.5f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-43.5f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-22.5f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-19.7f, 46.5f, -112.0f);
		glScalef(1.5f, 1.5f, 1.5f);
		glCallList(dsNewelCap);
	}
	glPopMatrix();
	///////////////////////////////////

	// balcony left most and right most walls
	glPushMatrix();
	{
		glColor3f(0.941f, 0.902f, 0.549f);	// khaki
		glTranslatef(103.0f, 57.0f, -96.0f);
		glScalef(1.0f, 22.9f, 16.50f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.941f, 0.902f, 0.549f);	// khaki
		glTranslatef(-103.0f, 57.0f, -96.0f);
		glScalef(1.0f, 22.9f, 16.50f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	// left side quarter circular structure
	glPushMatrix();
	{
		glTranslatef(-102.0f, 35.0f, -113.0f);
		glScalef(0.4f, 0.37f, 0.4f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		QuarterCircularStructure();
	}
	glPopMatrix();

	// right side quarter circular structure
	glPushMatrix();
	{
		glTranslatef(101.0f, 35.0f, -113.0f);
		glScalef(0.45f, 0.37f, 0.4f);
		glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
		QuarterCircularStructure();
	}
	glPopMatrix();

	// balcony balusters
	glPushMatrix();
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-90.0f, 0.0f, 0.0f);
		for (GLfloat i = 0.0f; i < 180.0f; i += 2.0f)
		{
			glPushMatrix();
			{
				glTranslatef(i, 37.7f, -112.0f);
				glScalef(0.5f, 0.4f, 0.5f);
				glCallList(dsBalconyBaluster);
			}
			glPopMatrix();
		}
	}
	glPopMatrix();

	// walls adjacent to stairs
	glPushMatrix();
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(56.0f, 17.50f, -112.0f);
		glScalef(11.0f, 13.6f, 1.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(80.0f, 17.50f, -112.0f);
		glScalef(11.10f, 13.6f, 1.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-57.0f, 17.50f, -112.0f);
		glScalef(11.10f, 13.60f, 1.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(-81.0f, 17.50f, -112.0f);
		glScalef(11.10f, 13.60f, 1.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	drawBalconyDoors();
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-78.0f, 60.0f, 0.0f);
		glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
		//glScalef(2.0f, 1.0f, 0.4f);
		//DrawRoof();
		glCallList(dsRoof);
	}
	glPopMatrix();
	

	glPushMatrix();
	{
		glTranslatef(-78.0f, 75.0f, -10.0f);
		glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
		//glScalef(2.5f, 1.0f, 1.0f);
		glCallList(dsUpperFrontRoof);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-100.0f, 77.0f, -80.0f);
		glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);
		//glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glScalef(2.5f, 1.0f, 1.0f);
		glCallList(dsUpperBackRoof);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(3.0f, 88.0f, -65.0f);
		glRotatef(51.20f, 1.0f, 0.0f, 0.0f);
		//glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glScalef(84.0f, 18.0f, 1.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glPushMatrix();
		glTranslatef(-20.0f, 63.0f, 10.0f);
		glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
		glCallList(dsPyramidalRoof);
		glPopMatrix();

		glTranslatef(0.0f, 67.0f, 0.0f);
		glRotatef(-60.0f, 1.0f, 0.0f, 0.0f);
		glScalef(23.0f, 15.0f, 5.0f);
		glBegin(GL_TRIANGLES);
		glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glVertex3f(1.0f, -1.0f, 0.0f);
		glEnd();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-72.0f, 86.0f, 10.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(38.0f, 1.0f, 0.0f, 0.0f);
		glCallList(dsLeftBuildingRightRoof);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-105.0f, 113.0f, 10.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(-40.0f, 1.0f, 0.0f, 0.0f);
		glCallList(dsLeftBuildingRightRoof);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(80.0f, 100.0f, 12.0f);
		glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
		glCallList(dsRightBuildingFrontRoof);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.35f, 0.16f, 0.14f);	// very dark brown
		glTranslatef(105.50f, 106.50f, -38.50f);
		glScalef(31.60f, 2.50f, 37.40f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	// Right stairs upper railings
	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glTranslatef(67.0f, 39.0f, -100.5f);
		glScalef(0.50f, 4.5f, 0.50f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glTranslatef(36.0f, 39.0f, -100.5f);
		glScalef(0.50f, 4.5f, 0.50f);
		glCallList(dsCubeRegular);

		glColor3f(1.0f, 1.0f, 1.0f);
		for (GLfloat i = 0.0f; i < 65; i += 2.0f)
		{
			glPushMatrix();
			glTranslatef(i, -0.2f, 0.0f);
			glScalef(0.2f, 0.8f, 0.2f);
			glCallList(dsCubeRegular);
			glPopMatrix();
		}
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glTranslatef(36.0f, 40.0f, -111.0f);
		glScalef(0.50f, 5.6f, 0.50f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glTranslatef(36.0f, 42.0f, -106.0f);
		glScalef(0.30f, 0.30f, 5.50f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glTranslatef(51.0f, 42.0f, -100.5f);
		glScalef(15.20f, 0.30f, 0.40f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	// left stairs upper railings
	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glTranslatef(-67.0f, 39.0f, -100.5f);
		glScalef(0.50f, 4.5f, 0.50f);
		glCallList(dsCubeRegular);

		glColor3f(1.0f, 1.0f, 1.0f);
		for (GLfloat i = 65.0f; i > 0.0f; i -= 2.0f)
		{
			glPushMatrix();
			glTranslatef(i, -0.2f, 0.0f);
			glScalef(0.2f, 0.8f, 0.2f);
			glCallList(dsCubeRegular);
			glPopMatrix();
		}
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glTranslatef(-36.0f, 39.0f, -100.5f);
		glScalef(0.50f, 4.5f, 0.50f);
		glCallList(dsCubeRegular);

	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glTranslatef(-36.0f, 40.0f, -111.0f);
		glScalef(0.50f, 5.6f, 0.50f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glTranslatef(-36.0f, 42.0f, -106.0f);
		glScalef(0.30f, 0.30f, 5.50f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glTranslatef(-51.0f, 42.0f, -100.5f);
		glScalef(15.20f, 0.30f, 0.40f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();


	// Block behind the grid door adjacent to stairs
	glPushMatrix();
	{
		glColor3f(0.871f, 0.722f, 0.529f);	//burly wood
		glTranslatef(-61.0f, 18.0f, -92.50f);
		glScalef(20.10f, 15.20f, 8.50f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glColor3f(0.871f, 0.722f, 0.529f);	//burly wood
		glTranslatef(61.0f, 18.0f, -92.50f);
		glScalef(20.10f, 15.20f, 8.50f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	// fill in the blanks
	glPushMatrix();
	{
		glTranslatef(105.0f, 48.50f, -75.50f);
		glScalef(25.0f, 55.50f, 1.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-105.0f, 47.50f, -75.50f);
		glScalef(25.0f, 44.70f, 1.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-127.0f, 84.00f, -41.0f);
		glScalef(3.0f, 9.40f, 33.10f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	// ground in front of gurukul
	glPushMatrix();
	{
		glTranslatef(0.0f, -1.0f, 0.0f);
		glCallList(dsGround);
	}
	glPopMatrix();
}
/////////

///Ashihs Design//
void drawDesign(void)
{
	void drawCube(GLfloat height, GLfloat width, GLfloat depth);
	void drawX(void);
	void drawCentralCube(void);

	glPushMatrix();
	glTranslatef(0.0f, 7.0f, 0.0f);

	glPushMatrix();
	drawCube(10.0f, 0.25f, 0.5f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.0f, 0.0f, 0.0f);
	drawCube(10.0f, 0.25f, 0.5f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-4.5f, 3.0f, 0.0f);
	drawX();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-4.5f, -3.5f, 0.0f);
	drawX();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(5.295f, 0.0f, 0.0f);
	glScalef(0.87f, 1.0f, 1.0f);
	drawCentralCube();
	glPopMatrix();

	glPopMatrix();
}

void drawX(void)
{
	void drawCube(GLfloat height, GLfloat width, GLfloat depth);

	glColor3f(0.75f, 0.75f, 0.75f);
	glPushMatrix();
	glTranslatef(5.0f, 0.0f, 0.0f);
	glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
	drawCube(1.5f, 0.25f, 0.2f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(5.0f, 0.0f, 0.0f);
	glRotatef(-45.0f, 0.0f, 0.0f, 1.0f);
	drawCube(1.5f, 0.25f, 0.2f);
	glPopMatrix();
}

void drawCentralCube(void)
{
	void drawCube(GLfloat height, GLfloat width, GLfloat depth);

	glColor3f(0.75f, 0.75f, 0.75f);
	glPushMatrix();
	glTranslatef(-5.0f, 0.0f, 0.0f);
	//glScalef(1.0f, 0.79f, 1.0f);
	//glRotatef(45.0f, 0.0f, 0.0f, 1.0f);

	glPushMatrix();
	glRotatef(46.5f, 0.0f, 0.0f, 1.0f);
	drawCube(1.5f, 0.25f, 0.2f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.0f, 0.0f, 0.0f);
	glRotatef(-46.5f, 0.0f, 0.0f, 1.0f);
	drawCube(1.5f, 0.25f, 0.2f);
	glPopMatrix();
	////////////////////////////////////////////////////////////
	glPushMatrix();
	glTranslatef(0.0f, -0.7f, 0.0f);
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);

	glPushMatrix();
	glRotatef(46.5f, 0.0f, 0.0f, 1.0f);
	drawCube(1.5f, 0.25f, 0.2f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.0f, 0.0f, 0.0f);
	glRotatef(-46.5f, 0.0f, 0.0f, 1.0f);
	drawCube(1.5f, 0.25f, 0.2f);
	glPopMatrix();
	glPopMatrix();

	glPopMatrix();

}

void drawPointedPart(void)
{
	void drawCube(GLfloat height, GLfloat width, GLfloat depth);
	void DrawPyramid(GLfloat side);

	glColor3f(0.75f, 0.75f, 0.75f);
	glPushMatrix();
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(-4.0f, 0.0f, 0.0f);

	glPushMatrix();
	glTranslatef(4.5f, -0.5f, 0.0f);
	glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
	drawCube(1.0f, 0.25f, 0.2f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(5.0f, -0.5f, 0.0f);
	glRotatef(-45.0f, 0.0f, 0.0f, 1.0f);
	drawCube(1.0f, 0.25f, 0.2f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(4.75f, -1.25f, 0.0f);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glScalef(0.3f, 0.3f, 0.3f);
	DrawPyramid(1.0f);
	glPopMatrix();

	glPopMatrix();
}



/////////////////////////////////////////////////////////////////////
/// Gaurav Patil

void GauravCode()
{
	//Gauarv's function declaration
	void LeftWall();
	void RightWall();
	void Kundi();
	void Floor();
	void Roof();
	void LeftStairs();
	void RightStairs();
	void spin();
	void translate();
	void Shivling();
	void ShivMandir();
	void ShivMandirFloor();

	glPushMatrix();
	glTranslatef(-22.0f, 16.0f, -55.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.6f, 0.6f, 0.6f);
	glPushMatrix();

	LeftWall();

	RightWall();
	Kundi();
	//Floor();
	//ShivMandirFloor();
	Roof();


	LeftStairs();
	RightStairs();


	Shivling();

	ShivMandir();
	
	//spin();
	translate();
	glPopMatrix();
	glPopMatrix();

}

void LeftWall()
{
	glPushMatrix();

	glTranslatef(5.0f, -0.5f, 4.4f);

	//cube on left side of curve(left side wall of curve)
	glPushMatrix();
	glTranslatef(-37.0f, 1.0f, -5.5f);
	glScalef(-4.0f, 4.0f, 1.0f);

	glBegin(GL_QUADS);
	//front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor3f(1.0f, 1.0f, 0.5f);
	//glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	//glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);

	//right face
	glNormal3f(1.0f, 0.0f, 0.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	//glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);

	//back face
	glNormal3f(0.0f, 0.0f, -1.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	//glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);

	//left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	//glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	//top face
	glNormal3f(0.0f, 1.0f, 0.0f);
	//glColor3f(1.0f, 0.0f, 1.0f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	
	//bottom face
	glNormal3f(0.0f, -1.0f, 0.0f);
	//glColor3f(0.0f, 1.0f, 1.0f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	
	glEnd();
	glPopMatrix();

	// Arc on left wall
	glPushMatrix();
	{
		//glColor3f(1.0f, 1.0f, 0.5f);
		glColor3f(0.941f, 0.902f, 0.549f);
		glTranslatef(0.0f, -5.0f, -5.5f);
		DrawShape(40.0f, 30.0f, 100);
	}
	glPopMatrix();

	// room behind the arc
	glPushMatrix();
	{
		// left wall
		glColor3f(0.941f, 0.902f, 0.549f);
		glTranslatef(-25.0f, 0.0f, -23.0f);
		glScalef(2.0f, 25.0f, 20.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		// front wall
		glColor3f(0.941f, 0.902f, 0.549f);
		glTranslatef(10.0f, 0.0f, -40.0f);
		glScalef(35.0f, 25.0f, 2.0f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	//cube on right side of curve(right side wall of curve)
	glPushMatrix();
	glTranslatef(31.0f, 1.0f, -5.5f);
	glScalef(3.0f, 4.0f, 1.0f);
	glBegin(GL_QUADS);

	//front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	//glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);

	//right face
	glNormal3f(1.0f, 0.0f, 0.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	//glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);

	//back face
	glNormal3f(0.0f, 0.0f, -1.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	//glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);

	//left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	//top face
	//glNormal3f(0.0f, 1.0f, 0.0f);
	//glColor3f(1.0f, 0.0f, 1.0f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);

	//bottom face
	//glNormal3f(0.0f, -1.0f, 0.0f);
	//glColor3f(0.0f, 1.0f, 1.0f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);

	glEnd();
	glPopMatrix();

	//ground block
	glPushMatrix();
	glTranslatef(-5.5f, -24.0f, -5.5f);
	glScalef(10.35f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	//front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	//glColor3f(0.25f, 0.25f, 0.25f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	//glColor3f(0.40f, 0.40f, 0.40f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	//glColor3f(0.25f, 0.25f, 0.25f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	//glColor3f(0.40f, 0.40f, 0.40f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(5.0f, -5.0f, 5.0f);

	//right face
	glNormal3f(1.0f, 0.0f, 0.0f);
	//glColor3f(0.25f, 0.25f, 0.25f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);

	/*//back face
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/

	//left face
	//glColor3f(-1.0f, 0.0f, 0.0f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	//top face
	glNormal3f(0.0f, 1.0f, 0.0f);
	glColor3f(0.25f, 0.25f, 0.05f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);

	/*//bottom face
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/

	glEnd();
	glPopMatrix();

	//wall of window(leftmost wall)

	glPushMatrix();
	glTranslatef(-59.0f, 0.5f, -8.0f);
	glScalef(1.9f, 3.9f, 1.0f);
	glBegin(GL_QUADS);
	//front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	//glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);

	/*//right face
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);

	//back face
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/

	//left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	/*//top face
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);

	//bottom face
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/
	glEnd();
	glPopMatrix();

	//ground block below window
	glPushMatrix();
	glTranslatef(-62.0f, -24.0f, -7.95f);
	glScalef(1.5f, 1.1f, 1.0f);
	glBegin(GL_QUADS);
	//front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	//glColor3f(0.40f, 0.40f, 0.40f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);

	/*//right face
	glColor3f(0.25f, 0.25f, 0.20f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);

	//back face
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/

	//left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	//top face
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);

	//bottom face
	//glColor3f(0.75f, 0.75f, 0.75f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);

	glEnd();
	glPopMatrix();

	//window
	glPushMatrix();
	glTranslatef(-61.0f, 2.0f, -2.95f);

	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, -3.0f, 0.0f);
	glColor3f(0.40f, 0.40f, 0.40f);
	glVertex3f(3.0f, -3.0f, 0.0f);
	glVertex3f(3.0f, 3.0f, 0.0f);
	glEnd();

	glLineWidth(3.0);
	glBegin(GL_LINES);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-1.5f, 1.5f, 0.0f);
	glVertex3f(-1.5f, -1.5f, 0.0f);

	glVertex3f(1.5f, 1.5f, 0.0f);
	glVertex3f(1.5f, -1.5f, 0.0f);
	glEnd();
	glPopMatrix();
	glPopMatrix();
}

void RightWall()
{
	glPushMatrix();
	glTranslatef(5.0f, 0.5f, -2.0f);
	//glRotatef(rAngle, 0.0f, 1.0f, 0.0f);
	glPushMatrix();

	//wall block
	glPushMatrix();
	glTranslatef(-4.0f, 0.0f, 75.0f);
	glScalef(9.75f, 4.0f, 1.0f);
	glBegin(GL_QUADS);
	//front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	//glColor3f(0.25f, 0.25f, 0.25f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);

	//right face
	glNormal3f(1.0f, 0.0f, 0.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);

	//back face
	glNormal3f(0.0f, 0.0f, -1.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	//glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);

	//left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	//glColor3f(1.0f, 1.0f, 0.7f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	/*//top face
	glColor3f(0.25f, 0.25f, 0.05f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);

	//bottom face
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/

	glEnd();
	glPopMatrix();

	//ground block
	glPushMatrix();
	glTranslatef(-4.0f, -25.0f, 74.9f);
	glScalef(9.78f, 1.3f, 1.0f);
	glBegin(GL_QUADS);
	//front face
	/*glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, 5.0f);

	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f*/

	//right face
	glNormal3f(1.0f, 0.0f, 0.0f);
	//glColor3f(0.25f, 0.25f, 0.25f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);

	//back face
	glNormal3f(0.0f, 0.0f, -1.0f);
	//glColor3f(0.25f, 0.25f, 0.25f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	//glColor3f(0.40f, 0.40f, 0.40f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);

	//left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	//glColor3f(0.25f, 0.25f, 0.25f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	//top face
	glNormal3f(0.0f, 1.0f, 0.0f);
	//glColor3f(0.25f, 0.25f, 0.05f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);

	/*//bottom face
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/

	glEnd();
	glPopMatrix();

	//wall of window(rightmost wall)

	glPushMatrix();
	glTranslatef(-60.0f, -1.0f, 78.0f);
	glScalef(1.9f, 4.0f, 1.0f);
	glBegin(GL_QUADS);
	/*//front face
	glColor3f(1.0f, 1.0f, 0.5f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);*/

	//right face
	/*glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);*/

	//back face
	glNormal3f(0.0f, 0.0f, -1.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	//glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);

	//left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	//glColor3f(1.0f, 1.0f, 0.5f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	//top face
	/*glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);

	//bottom face
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/

	glEnd();
	glPopMatrix();

	//ground block below window
	{
		glPushMatrix();
		glTranslatef(-60.0f, -25.5f, 77.9f);
		glScalef(1.9f, 1.3f, 1.0f);
		glBegin(GL_QUADS);
		//front face
		/*glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);*/

		/*//right face
		glColor3f(0.25f, 0.25f, 0.20f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);*/

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		//glColor3f(0.25f, 0.25f, 0.25f);
		glColor3f(0.10f, 0.10f, 0.10f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		//glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		//glColor3f(1.0f, 1.0f, 0.5f);
		glColor3f(0.941f, 0.902f, 0.549f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		/*	//top face
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		glColor3f(0.75f, 0.75f, 0.75f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();
	}
	//sharma uncle's Room
	{    glPushMatrix();
	// far wall from stair view
	//glColor3f(0.8f, 0.8f, 0.8f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glTranslatef(-25.0f, 0.0f, 95.0f);
	glScalef(2.0f, 25.0f, 20.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//left wall from stair view
	glPushMatrix();
	//glColor3f(0.8f, 0.8f, 0.8f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glTranslatef(11.0f, 0.0f, 107.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(2.0f, 25.0f, 34.1f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//near wall from stair view
	glPushMatrix();
	// far wall from stair view
	//glColor3f(0.8f, 0.8f, 0.8f);
	glColor3f(0.941f, 0.902f, 0.549f);
	glTranslatef(43.0f, 0.0f, 100.0f);
	glScalef(2.1f, 25.0f, 5.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();
	}
	//window
	glPushMatrix();
	glTranslatef(-59.0f, 2.0f, 72.95f);


	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, -1.0f);
	//glColor3f(0.25f, 0.25f, 0.25f);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(-3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, -3.0f, 0.0f);
	glVertex3f(3.0f, -3.0f, 0.0f);
	glVertex3f(3.0f, 3.0f, 0.0f);
	glEnd();

	glLineWidth(3.0);
	glBegin(GL_LINES);
	glColor3f(0.10f, 0.10f, 0.10f);
	glVertex3f(-1.5f, 1.5f, 0.0f);
	glVertex3f(-1.5f, -1.5f, 0.0f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(1.5f, 1.5f, 0.0f);
	glVertex3f(1.5f, -1.5f, 0.0f);
	glEnd();
	glPopMatrix();

	//sharma uncle's window
	glPushMatrix();
	glTranslatef(8.5f, 2.0f, 69.95f);
	glScalef(2.0f, 3.0f, 0.0f);

	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, -3.0f, 0.0f);
	glColor3f(0.40f, 0.40f, 0.40f);
	glVertex3f(3.0f, -3.0f, 0.0f);
	glVertex3f(3.0f, 3.0f, 0.0f);
	glEnd();

	glLineWidth(3.0);
	glBegin(GL_LINES);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-1.5f, 1.5f, 0.0f);
	glVertex3f(-1.5f, -1.5f, 0.0f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(1.5f, 1.5f, 0.0f);
	glVertex3f(1.5f, -1.5f, 0.0f);
	glEnd();
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();

}

void Kundi()
{
	glPushMatrix();
	static GLfloat KundiColor = 0.50f;
	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//view transformation
	//gluLookAt(i, -5.0f, 40.0f, 35.0f, 0.0f, 25.0f, 0.0f, 1.0f, 0.0f);
	//gluLookAt(0.0f,0.0,80.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f);
	mygluLookAt();*/



	glTranslatef(220.0f, -30.0f, 35.0f);
	//kundi no.1
	glPushMatrix();
	glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(90.0f,0.0f,0.0f,1.0f);

	// glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	quadric = gluNewQuadric();
	glColor3f(KundiColor, 0.0f, 0.0f);
	//3rd param is slices (like longitude) and 4th param is stacks (like latitudes)
	//more the value of 3rd and 4th param ,more will be the divisions,moer circular

	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_OUTSIDE);
	gluCylinder(quadric, 5.0, 10.0, 10.0, 50, 50);

	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_INSIDE);
	gluCylinder(quadric, 4.9, 9.9, 10.0, 50, 50);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 10.0f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	quadric1 = gluNewQuadric();
	gluQuadricNormals(quadric1, GLU_SMOOTH);
	glColor3f(0.70f, 0.0f, 0.0f);
	gluDisk(quadric1, 8.5, 9.5, 50, 50);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.1f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	quadric1 = gluNewQuadric();
	gluQuadricNormals(quadric1, GLU_SMOOTH);
	glColor3f(0.70f, 0.0f, 0.0f);
	gluDisk(quadric1, 4.0, 5.0, 50, 50);
	glPopMatrix();


	//Kundi no.2
	glPushMatrix();
	glTranslatef(5.0f, 0.0f, 20.0f);
	glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
	quadric = gluNewQuadric();
	
	glColor3f(KundiColor, 0.0f, 0.0f);
	//3rd param is slices (like longitude) and 4th param is stacks (like latitudes)
	//more the value of 3rd and 4th param ,more will be the divisions,moer circular

	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_OUTSIDE);
	gluCylinder(quadric, 5.0, 10.0, 10.0, 50, 50);

	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_INSIDE);
	gluCylinder(quadric, 4.9, 9.9, 10.0, 50, 50);
	glPopMatrix();


	/*glPushMatrix();
	glTranslatef(0.0f, 10.0f, 15.0f);
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
	quadric1 = gluNewQuadric();
	glColor3f(0.70f, 0.0f, 0.0f);
	gluDisk(quadric1, 8.5, 9.5, 50.0, 50.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.1f, 0.0f);
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
	quadric1 = gluNewQuadric();
	glColor3f(0.70f, 0.0f, 0.0f);
	gluDisk(quadric1, 4.0, 5.0, 50.0, 50.0);
	glPopMatrix();
	glPopMatrix();*/

	//Kundi no.3
	glPushMatrix();
	glTranslatef(5.0f, 0.0f, -20.0f);
	glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
	quadric = gluNewQuadric();
	
	glColor3f(KundiColor, 0.0f, 0.0f);

	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_OUTSIDE);
	gluCylinder(quadric, 5.0, 10.0, 10.0, 50, 50);

	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_INSIDE);
	gluCylinder(quadric, 4.9, 9.9, 10.0, 50, 50);
	glPopMatrix();


	/*if (KundiColor <0.50f)
	KundiColor = KundiColor + 0.0001f;*/

	glPopMatrix();
}

void Floor()
{

	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//view transformation
	//gluLookAt(i, -5.0f, 40.0f, 35.0f, 0.0f, 25.0f, 0.0f, 1.0f, 0.0f);
	//gluLookAt(0.0f,0.0,80.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f);
	mygluLookAt();*/



	glPushMatrix();
	glTranslatef(0.0f, -28.0f, 30.0f);
	//Floor
	glScalef(25.0f, 0.0f, 12.0f);
	glColor3f(0.40f, 0.40f, 0.40f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-5.0f, 0.0f, -5.0f);
	glVertex3f(-5.0f, 0.0f, 5.0f);
	glColor3f(0.60f, 0.60f, 0.60f);
	glVertex3f(5.0f, 0.0f, 5.0f);
	glVertex3f(5.0f, 0.0f, -5.0f);
	glEnd();
	glPopMatrix();

	//Vertical FloorLines
	//for (GLint FloorLines = 30.0f;FloorLines < 36.0f;FloorLines++)

	/*glPushMatrix();
	glTranslatef(20.0f, -29.0f, 30.0f);
	glScalef(90.0f, 0.0f, 1.0f);
	glColor3f(0.20f, 0.20f, 0.20f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, 0.0f, -1.0f);
	glColor3f(0.50f, 0.50f, 0.50f);
	glVertex3f(-1.0f, 0.0f, -1.0f);
	glVertex3f(-1.0f, 0.0f, 1.0f);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-25.0f, -29.0f, 45.0f);
	glScalef(90.0f, 0.0f, 1.0f);
	glColor3f(0.20f, 0.20f, 0.20f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, 0.0f, -1.0f);
	glColor3f(0.50f, 0.50f, 0.50f);
	glVertex3f(-1.0f, 0.0f, -1.0f);
	glVertex3f(-1.0f, 0.0f, 1.0f);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-25.0f, -29.0f, 15.0f);
	glScalef(90.0f, 0.0f, 1.0f);
	glColor3f(0.20f, 0.20f, 0.20f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, 0.0f, -1.0f);
	glColor3f(0.50f, 0.50f, 0.50f);
	glVertex3f(-1.0f, 0.0f, -1.0f);
	glVertex3f(-1.0f, 0.0f, 1.0f);
	glEnd();
	glPopMatrix();*/

	////Horizontal Floor Lines
	//for (GLfloat shortLines = -75.0f;shortLines < 75.0f;shortLines = shortLines + 15.0f)
	//{
	//	glPushMatrix();
	//	glTranslatef(shortLines, -29.0f, 38.0f);
	//	glScalef(1.0f, 0.0f, 7.0f);
	//	glColor3f(0.20f, 0.20f, 0.20f);
	//	glBegin(GL_QUADS);
	//	glNormal3f(0.0f, 1.0f, 0.0f);
	//	glVertex3f(1.0f, 0.0f, 1.0f);
	//	glVertex3f(1.0f, 0.0f, -1.0f);
	//	glColor3f(0.50f, 0.50f, 0.50f);
	//	glVertex3f(-1.0f, 0.0f, -1.0f);
	//	glVertex3f(-1.0f, 0.0f, 1.0f);
	//	glEnd();
	//	glPopMatrix();
	//}

	//for (GLfloat shortLines = -65.0f;shortLines < 75.0f;shortLines = shortLines + 15.0f)
	//{
	//	glPushMatrix();
	//	glTranslatef(shortLines, -29.0f, 22.0f);
	//	glScalef(1.0f, 0.0f, 7.0f);
	//	glColor3f(0.20f, 0.20f, 0.20f);
	//	glBegin(GL_QUADS);
	//	glNormal3f(0.0f, 1.0f, 0.0f);
	//	glVertex3f(1.0f, 0.0f, 1.0f);
	//	glVertex3f(1.0f, 0.0f, -1.0f);
	//	//glColor3f(0.50f, 0.50f, 0.50f);
	//	glVertex3f(-1.0f, 0.0f, -1.0f);
	//	glVertex3f(-1.0f, 0.0f, 1.0f);
	//	glEnd();
	//	glPopMatrix();
	//}

	//for (GLfloat shortLines = -75.0f;shortLines < 75.0f;shortLines = shortLines + 15.0f)
	//{
	//	glPushMatrix();
	//	glTranslatef(shortLines, -29.0f, 10.0f);
	//	glScalef(1.0f, 0.0f, 8.0f);
	//	glColor3f(0.20f, 0.20f, 0.20f);
	//	glBegin(GL_QUADS);
	//	glNormal3f(0.0f, 1.0f, 0.0f);
	//	glVertex3f(1.0f, 0.0f, 1.0f);
	//	glVertex3f(1.0f, 0.0f, -1.0f);
	//	glColor3f(0.50f, 0.50f, 0.50f);
	//	glVertex3f(-1.0f, 0.0f, -1.0f);
	//	glVertex3f(-1.0f, 0.0f, 1.0f);
	//	glEnd();
	//	glPopMatrix();
	//}

	//for (GLfloat shortLines = -80.0f;shortLines < 75.0f;shortLines = shortLines + 15.0f)
	//{
	//	glPushMatrix();
	//	glTranslatef(shortLines, -29.0f, 55.0f);
	//	glScalef(1.0f, 0.0f, 12.0f);
	//	glColor3f(0.20f, 0.20f, 0.20f);
	//	glBegin(GL_QUADS);
	//	glNormal3f(0.0f, 1.0f, 0.0f);
	//	glVertex3f(1.0f, 0.0f, 1.0f);
	//	glVertex3f(1.0f, 0.0f, -1.0f);
	//	//glColor3f(0.50f, 0.50f, 0.50f);
	//	glVertex3f(-1.0f, 0.0f, -1.0f);
	//	glVertex3f(-1.0f, 0.0f, 1.0f);
	//	glEnd();
	//	glPopMatrix();
	//}
}

void Roof()
{
	glPushMatrix();
	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//view transformation
	//gluLookAt(i, -5.0f, 40.0f, 35.0f, 0.0f, 25.0f, 0.0f, 1.0f, 0.0f);
	//gluLookAt(0.0f,0.0,80.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f);
	mygluLookAt();*/

	//brown roof
	glTranslatef(-1.0f, 20.0f, 32.0f);
	//glRotatef(-0.5f, 1.0f, 0.0f, 0.0f);
	/*glPushMatrix();
	//glRotatef(1.5f, 1.0f, 0.0f, 0.0f);
	glScalef(14.0f, 0.0f, 10.0f);
	glColor3f(0.5f, 0.03f, 0.03f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(5.0f, 0.0f, 5.0f);
	glVertex3f(5.0f, 0.0f, -5.0f);
	glVertex3f(-5.0f, 0.0f, -5.0f);
	glVertex3f(-5.0f, 0.0f, 5.0f);
	glEnd();
	glPopMatrix();*/

	//beam bar
	glPushMatrix();
	glTranslatef(48.0f, -1.5f, 5.0f);
	//glRotatef(1.0f, 1.0f, 0.0f, 0.0f);
	glScalef(0.5f, 0.5f, 8.0f);
	glBegin(GL_QUADS);
	/*/
	//front face
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	*/
	//right face
	glNormal3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.941f, 0.902f, 0.549f);	// khaki
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);

	/*//back face
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/

	//left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glColor3f(0.941f, 0.902f, 0.549f);	// khaki
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glColor3f(1.0f, 0.7f, 0.4f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	/*//top face
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);*/

	//bottom face
	glNormal3f(0.0f, -1.0f, 0.0f);
	glColor3f(0.941f, 0.902f, 0.549f);	// khaki
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glColor3f(0.941f, 0.902f, 0.549f);	// khaki
	glVertex3f(5.0f, -5.0f, 5.0f);
	glColor3f(0.941f, 0.902f, 0.549f);	// khaki
	glVertex3f(5.0f, -5.0f, -5.0f);
	glColor3f(0.941f, 0.902f, 0.549f);	// khaki
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glEnd();
	glPopMatrix();


	//bars
	for (GLfloat fBars = -55.0f; fBars < 45.0f; fBars = fBars + 10.0f)
	{
		glPushMatrix();
		glTranslatef(fBars, -1.9f, 5.0f);
		//glRotatef(1.5f, 1.0f, 0.0f, 0.0f);
		glScalef(0.2f, 0.2f, 8.0f);
		glBegin(GL_QUADS);
		/*//front face
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		*/
		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.7f, 0.01f, 0.01f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		//glColor3f(1.0f, 0.7f, 0.4f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		/*//back face
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.7f, 0.01f, 0.01f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		//glColor3f(1.0f, 0.7f, 0.4f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		/*//top face
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);*/

		//bottom face
		glNormal3f(0.0f, -1.0f, 0.0f);
		glColor3f(1.0f, 0.2f, 0.2f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		//glColor3f(1.0f, 1.0f, 0.5f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		//glColor3f(1.0f, 1.0f, 0.5f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		//glColor3f(1.0f, 0.7f, 0.4f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glEnd();
		glPopMatrix();
	}
	glPopMatrix();
}

void LeftStairs()
{
	glPushMatrix();
	static GLfloat  fVerticalMovement = 0.0f;
	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	mygluLookAt();*/

	//left bar
	{
		glTranslatef(70.0f, fVerticalMovement, fLeftTranslate);
		if (fVerticalMovement < 0.0f)
			fVerticalMovement = fVerticalMovement + 0.011f;

		glPushMatrix();
		glTranslatef(-0.7f, -9.5f, -2.3f);
		glScalef(0.16f, 2.0f, 0.2f);
		glColor3f(0.36f, 0.25f, 0.20f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		/*glColor3f(0.0f, 1.0f, 1.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();
	}
	//right bar
	{
		glPushMatrix();
		glTranslatef(22.0f, -9.5f, -2.3f);
		glScalef(0.2f, 2.0f, 0.2f);
		glColor3f(0.36f, 0.25f, 0.20f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		//glNormal3f(0.0f, 0.0f, 0.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		//glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		/*glColor3f(0.0f, 1.0f, 1.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();
	}
	//top bar
	{
		glPushMatrix();
		glTranslatef(10.7f, -1.5f, -2.3f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(0.2f, 2.1f, 0.15f);


		glColor3f(0.36f, 0.25f, 0.20f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.20f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		glNormal3f(0.0f, -1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		glEnd();
		glPopMatrix();
	}
	//bottom bar
	{
		glPushMatrix();
		glTranslatef(10.7f, -18.8f, -2.3f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(0.1f, 2.1f, 0.2f);


		glColor3f(0.36f, 0.25f, 0.20f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		glNormal3f(0.0f, -1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		glEnd();
		glPopMatrix();
	}
	//bars
	for (GLfloat fBars = 1.0f; fBars < 20.0f; fBars = fBars + 2.0f)
	{
		glPushMatrix();
		glTranslatef(fBars, -9.5f, -2.3f);
		glScalef(0.01f, 1.8f, 0.07f);
		glColor3f(0.9f, 0.9f, 0.9f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		//glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		//glColor3f(0.36f, 0.25f, 0.20f);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		//glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		//glColor3f(0.36f, 0.25f, 0.20f);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		//glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		//glColor3f(0.36f, 0.25f, 0.20f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		//glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		//glColor3f(0.36f, 0.25f, 0.20f);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		//glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		/*glColor3f(0.0f, 1.0f, 1.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();
	}

	//left side temple facing bars
	{
		glPushMatrix();
		//left bar
		glTranslatef(27.0f, fVerticalMovement, -6.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		glPushMatrix();
		{
			glTranslatef(-0.7f, -9.5f, -2.3f);
			glScalef(0.16f, 2.0f, 0.2f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glBegin(GL_QUADS);
			//front face
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);

			//right face
			glNormal3f(1.0f, 0.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);

			//back face
			glNormal3f(0.0f, 0.0f, -1.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);

			//left face
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);

			//top face
			glNormal3f(0.0f, 1.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);

			//bottom face
			/*glColor3f(0.0f, 1.0f, 1.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);*/

			glEnd();
		}
		glPopMatrix();

		//right bar
		glPushMatrix();
		glTranslatef(34.0f, -9.5f, -2.3f);
		glScalef(0.2f, 2.0f, 0.2f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		//glNormal3f(0.0f, 0.0f, 0.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		/*glColor3f(0.0f, 1.0f, 1.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();


		//top bar
		glPushMatrix();
		glTranslatef(16.7f, -1.5f, -2.3f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(0.2f, 3.7f, 0.15f);


		glColor3f(0.36f, 0.25f, 0.20f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.20f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		glNormal3f(0.0f, -1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		glEnd();
		glPopMatrix();

		//bottom bar
		glPushMatrix();
		glTranslatef(16.7f, -18.8f, -2.3f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(0.1f, 3.7f, 0.2f);


		glColor3f(0.36f, 0.25f, 0.20f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		glNormal3f(0.0f, -1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		glEnd();
		glPopMatrix();

		//bars
		for (GLfloat fBars = 1.0f; fBars < 33.0f; fBars = fBars + 2.0f)
		{
			glPushMatrix();
			glTranslatef(fBars, -9.5f, -2.3f);
			glScalef(0.01f, 1.8f, 0.07f);
			glColor3f(0.9f, 0.9f, 0.9f);
			glBegin(GL_QUADS);
			//front face
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			//glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);

			//right face
			//glColor3f(0.36f, 0.25f, 0.20f);
			glNormal3f(1.0f, 0.0f, 0.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			//glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);

			//back face
			//glColor3f(0.36f, 0.25f, 0.20f);
			glNormal3f(0.0f, 0.0f, -1.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			//glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);

			//left face
			//glColor3f(0.36f, 0.25f, 0.20f);
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			//glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);

			//top face
			//glColor3f(0.36f, 0.25f, 0.20f);
			glNormal3f(0.0f, 1.0f, 0.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			//glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);

			//bottom face
			/*glColor3f(0.0f, 1.0f, 1.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);*/

			glEnd();
			glPopMatrix();
		}
		glPopMatrix();
	}

	//left side bottom cube
	{
		glPushMatrix();
		glTranslatef(12.0f, -24.5f, -55.2f);
		glScalef(2.9f, 1.0f, 10.9f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		/*//back face
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.05f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		/*//bottom face
		glColor3f(0.75f, 0.75f, 0.75f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();
	}

	//left side hidden bottom block
	{
		glPushMatrix();
		glTranslatef(-57.0f, -24.5f, -24.2f);
		glRotatef(90.0f, 0.0f, 1.0, 0.0f);
		glScalef(2.9f, 1.0f, 10.9f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		/*//back face
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.05f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		/*//bottom face
		glColor3f(0.75f, 0.75f, 0.75f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();
	}

	//left side stair
	{
		glPushMatrix();
		glTranslatef(-7.0f, -26.5f, -8.2f);
		glScalef(2.5f, 0.5f, 1.0f);
		myCube();
		glPopMatrix();
	}

	//left side grid door area
	{
		glPushMatrix();
		glTranslatef(9.0f, 0.0f, 0.0f);
		glScalef(1.4f, 1.0f, 1.0f);
		//RightBar
		glPushMatrix();
		glTranslatef(-2.0f, -6.5f, -34.2f);
		glScalef(0.2f, 8.5f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//left Bar
		glPushMatrix();
		glTranslatef(-20.0f, -6.5f, -34.2f);
		glScalef(0.2f, 8.5f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//Top Bar
		glPushMatrix();
		glTranslatef(-11.0f, 18.5f, -34.2f);
		//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(3.0f, 0.2f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//Bottom bar
		glPushMatrix();
		glTranslatef(-11.0f, -19.0f, -34.2f);
		//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(3.0f, 0.2f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//middle Bar (horizontal)
		glPushMatrix();
		glTranslatef(-11.0f, 12.0f, -34.2f);
		//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(3.0f, 0.2f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//middle bar(vertical)
		glPushMatrix();
		glTranslatef(-9.0f, -10.5f, -34.2f);
		glScalef(0.2f, 7.5f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//Grid
		for (GLfloat fBars = -19.0f; fBars < -1.0f; fBars = fBars + 1.0f)
		{
			glPushMatrix();
			glTranslatef(fBars, 15.0f, -34.2f);
			glScalef(0.1f, 1.0f, 0.1f);
			glBegin(GL_QUADS);
			BrownCube();
			glEnd();
			glPopMatrix();
		}
		for (GLfloat fHoriBars = 13.0f; fHoriBars < 19.0f; fHoriBars = fHoriBars + 1.0f)
		{
			glPushMatrix();
			glTranslatef(-11.0f, fHoriBars, -34.2f);
			//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
			glScalef(3.0f, 0.1f, 0.1f);
			glBegin(GL_QUADS);
			BrownCube();
			glEnd();
			glPopMatrix();
		}

		//brownBlock
		glPushMatrix();
		glTranslatef(-5.5f, -10.0f, -35.2f);
		glScalef(1.0f, 7.5f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();
		glPopMatrix();
	}
	//right side curve door
	{
		glColor3f(0.941f, 0.902f, 0.549f);	// khaki
		glPushMatrix();
		glTranslatef(-25.0f, 10.0f, -17.5f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		DrawShape(18.0f, 10.0f, 50);
		glTranslatef(18.0f, -5.0f, 0.0f);
		glScalef(10.0f, 25.0f, 5.1f);
		glCallList(dsCubeRegular);
		glPopMatrix();
	}
	glPopMatrix();

}

void RightStairs()
{
	glPushMatrix();
	static GLfloat fVerticalMovement = -0.0f;
	//void RightBar();
	//void LeftBar();
	//void TopBar();
	//void BottomBar();
	//void Bars();
	//void GroundBlock();
	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	mygluLookAt();*/
	glTranslatef(70.0f, fVerticalMovement, fRightTranslate);
	if (fVerticalMovement<0.0f)
		fVerticalMovement = fVerticalMovement + 0.011f;
	//glRotatef();
	glPushMatrix();



	glTranslatef(0.0f, 0.0f, -7.5f);
	//right bar

	{
		glPushMatrix();
		glTranslatef(-0.7f, -9.5f, 79.0f);
		glScalef(0.16f, 2.0f, 0.2f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.2f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.2f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 2.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		/*glColor3f(0.0f, 1.0f, 1.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();
	}
	//left bar

	{
		glPushMatrix();
		glTranslatef(22.0f, -9.5f, 79.0f);
		glScalef(0.2f, 2.0f, 0.2f);
		glColor3f(0.36f, 0.25f, 0.2f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		/*glColor3f(0.0f, 1.0f, 1.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();
	}
	//top bar

	{
		glPushMatrix();
		glTranslatef(10.7f, -1.5f, 79.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(0.2f, 2.1f, 0.15f);


		glColor3f(0.36f, 0.25f, 0.20f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		glNormal3f(0.0f, -1.0f, 0.0f);
		glColor3f(0.0f, 1.0f, 1.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		glEnd();
		glPopMatrix();
	}
	//bottom bar

	{
		glPushMatrix();
		glTranslatef(10.7f, -18.8f, 79.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(0.1f, 2.1f, 0.2f);


		glColor3f(0.36f, 0.25f, 0.20f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		//glColor3f(1.0f, 0.7f, 0.4f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.36f, 0.12f, 0.10f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.36f, 0.25f, 0.20f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		//bottom face
		glNormal3f(0.0f, -1.0f, 0.0f);
		glColor3f(0.0f, 1.0f, 1.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		glEnd();
		glPopMatrix();
	}
	//bars

	{
		for (GLfloat fBars = 1.0f; fBars < 20.0f; fBars = fBars + 2.0f)
		{
			glPushMatrix();
			glTranslatef(fBars, -9.5f, 79.0f);
			glScalef(0.01f, 1.8f, 0.07f);
			glColor3f(0.9f, 0.9f, 0.9f);
			glBegin(GL_QUADS);
			//front face
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			//glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);

			//right face
			//glColor3f(0.36f, 0.25f, 0.20f);
			glNormal3f(1.0f, 0.0f, 0.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			//glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);

			//back face
			//glColor3f(0.36f, 0.25f, 0.20f);
			glNormal3f(0.0f, 0.0f, -1.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			//glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);

			//left face
			//glColor3f(0.36f, 0.25f, 0.20f);
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			//glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);

			//top face
			//glColor3f(0.36f, 0.25f, 0.20f);
			glNormal3f(0.0f, 1.0f, 0.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			//glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);

			//bottom face
			/*glColor3f(0.0f, 1.0f, 1.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);*/

			glEnd();
			glPopMatrix();
		}
	}
	glPopMatrix();

	//right side temple facing bars

	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, 114.5f);
		//glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

		{
			glPushMatrix();
			//left bar
			glTranslatef(27.0f, fVerticalMovement, -6.0f);
			glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

			glPushMatrix();
			glTranslatef(-0.7f, -9.5f, -2.3f);
			glScalef(0.16f, 2.0f, 0.2f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glBegin(GL_QUADS);
			//front face
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);

			//right face
			glNormal3f(1.0f, 0.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);

			//back face
			glNormal3f(0.0f, 0.0f, -1.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);

			//left face
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);

			//top face
			glNormal3f(0.0f, 1.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);

			//bottom face
			/*glColor3f(0.0f, 1.0f, 1.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);*/

			glEnd();
			glPopMatrix();

			//right bar
			glPushMatrix();
			glTranslatef(34.0f, -9.5f, -2.3f);
			glScalef(0.2f, 2.0f, 0.2f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glBegin(GL_QUADS);
			//front face
			glNormal3f(0.0f, 0.0f, 1.0f);
			//glNormal3f(0.0f, 0.0f, 0.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);

			//right face
			glNormal3f(1.0f, 0.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);

			//back face
			glNormal3f(0.0f, 0.0f, -1.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);

			//left face
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);

			//top face
			glNormal3f(0.0f, 1.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);

			//bottom face
			/*glColor3f(0.0f, 1.0f, 1.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);*/

			glEnd();
			glPopMatrix();


			//top bar
			glPushMatrix();
			glTranslatef(16.7f, -1.5f, -2.3f);
			glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
			glScalef(0.2f, 3.7f, 0.15f);


			glColor3f(0.36f, 0.25f, 0.20f);
			glBegin(GL_QUADS);
			//front face
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.20f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);

			//right face
			glNormal3f(1.0f, 0.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);

			//back face
			glNormal3f(0.0f, 0.0f, -1.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);

			//left face
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);

			//top face
			glNormal3f(0.0f, 1.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);

			//bottom face
			glNormal3f(0.0f, -1.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);

			glEnd();
			glPopMatrix();

			//bottom bar
			glPushMatrix();
			glTranslatef(16.7f, -18.8f, -2.3f);
			glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
			glScalef(0.1f, 3.7f, 0.2f);


			glColor3f(0.36f, 0.25f, 0.20f);
			glBegin(GL_QUADS);
			//front face
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);

			//right face
			glNormal3f(1.0f, 0.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);

			//back face
			glNormal3f(0.0f, 0.0f, -1.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);

			//left face
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(-5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);

			//top face
			glNormal3f(0.0f, 1.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.36f, 0.12f, 0.10f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);

			//bottom face
			glNormal3f(0.0f, -1.0f, 0.0f);
			glColor3f(0.36f, 0.25f, 0.20f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);

			glEnd();
			glPopMatrix();

			//bars
			for (GLfloat fBars = 1.0f; fBars < 33.0f; fBars = fBars + 2.0f)
			{
				glPushMatrix();
				glTranslatef(fBars, -9.5f, -2.3f);
				glScalef(0.01f, 1.8f, 0.07f);
				glColor3f(0.9f, 0.9f, 0.9f);
				glBegin(GL_QUADS);
				//front face
				glNormal3f(0.0f, 0.0f, 1.0f);
				glVertex3f(5.0f, 5.0f, 5.0f);
				glVertex3f(-5.0f, 5.0f, 5.0f);
				//glColor3f(0.36f, 0.12f, 0.10f);
				glVertex3f(-5.0f, -5.0f, 5.0f);
				glVertex3f(5.0f, -5.0f, 5.0f);

				//right face
				//glColor3f(0.36f, 0.25f, 0.20f);
				glNormal3f(1.0f, 0.0f, 0.0f);
				glVertex3f(5.0f, 5.0f, -5.0f);
				glVertex3f(5.0f, 5.0f, 5.0f);
				//glColor3f(0.36f, 0.12f, 0.10f);
				glVertex3f(5.0f, -5.0f, 5.0f);
				glVertex3f(5.0f, -5.0f, -5.0f);

				//back face
				//glColor3f(0.36f, 0.25f, 0.20f);
				glNormal3f(0.0f, 0.0f, -1.0f);
				glVertex3f(-5.0f, 5.0f, -5.0f);
				glVertex3f(5.0f, 5.0f, -5.0f);
				//glColor3f(0.36f, 0.12f, 0.10f);
				glVertex3f(5.0f, -5.0f, -5.0f);
				glVertex3f(-5.0f, -5.0f, -5.0f);

				//left face
				//glColor3f(0.36f, 0.25f, 0.20f);
				glNormal3f(-1.0f, 0.0f, 0.0f);
				glVertex3f(-5.0f, 5.0f, 5.0f);
				glVertex3f(-5.0f, 5.0f, -5.0f);
				//glColor3f(0.36f, 0.12f, 0.10f);
				glVertex3f(-5.0f, -5.0f, -5.0f);
				glVertex3f(-5.0f, -5.0f, 5.0f);

				//top face
				//glColor3f(0.36f, 0.25f, 0.20f);
				glNormal3f(0.0f, 1.0f, 0.0f);
				glVertex3f(-5.0f, 5.0f, 5.0f);
				glVertex3f(5.0f, 5.0f, 5.0f);
				//glColor3f(0.36f, 0.12f, 0.10f);
				glVertex3f(5.0f, 5.0f, -5.0f);
				glVertex3f(-5.0f, 5.0f, -5.0f);

				//bottom face
				/*glColor3f(0.0f, 1.0f, 1.0f);
				glVertex3f(-5.0f, -5.0f, 5.0f);
				glVertex3f(5.0f, -5.0f, 5.0f);
				glVertex3f(5.0f, -5.0f, -5.0f);
				glVertex3f(-5.0f, -5.0f, -5.0f);*/

				glEnd();
				glPopMatrix();
			}
			glPopMatrix();
		}

		glPopMatrix();
	}
	//ground block

	{
		glPushMatrix();
		glTranslatef(12.0f, -24.5f, 124.5f);
		glScalef(2.9f, 1.0f, 10.9f);
		glBegin(GL_QUADS);
		//front face
		/*glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(5.0f, -5.0f, 5.0f);*/

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0, -1.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0, 0.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.05f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		/*//bottom face
		glColor3f(0.75f, 0.75f, 0.75f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();
	}
	//right side grid door
	{
		glPushMatrix();
		glTranslatef(-22.0f, 0.0f, 72.0f);
		glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.4f, 1.0f, 1.0f);
		//RightBar
		glPushMatrix();
		glTranslatef(-2.0f, -6.5f, -34.2f);
		glScalef(0.2f, 8.5f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//left Bar
		glPushMatrix();
		glTranslatef(-20.0f, -6.5f, -34.2f);
		glScalef(0.2f, 8.5f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//Top Bar
		glPushMatrix();
		glTranslatef(-11.0f, 18.5f, -34.2f);
		//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(3.0f, 0.2f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//Bottom bar
		glPushMatrix();
		glTranslatef(-11.0f, -19.0f, -34.2f);
		//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(3.0f, 0.2f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//middle Bar (horizontal)
		glPushMatrix();
		glTranslatef(-11.0f, 12.0f, -34.2f);
		//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glScalef(3.0f, 0.2f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//middle bar(vertical)
		glPushMatrix();
		glTranslatef(-13.0f, -10.5f, -34.2f);
		glScalef(0.2f, 7.5f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();

		//Grid
		for (GLfloat fBars = -19.0f; fBars < -1.0f; fBars = fBars + 1.0f)
		{
			glPushMatrix();
			glTranslatef(fBars, 15.0f, -34.2f);
			glScalef(0.1f, 1.0f, 0.1f);
			glBegin(GL_QUADS);
			BrownCube();
			glEnd();
			glPopMatrix();
		}
		for (GLfloat fHoriBars = 13.0f; fHoriBars < 19.0f; fHoriBars = fHoriBars + 1.0f)
		{
			glPushMatrix();
			glTranslatef(-11.0f, fHoriBars, -34.2f);
			//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
			glScalef(3.0f, 0.1f, 0.1f);
			glBegin(GL_QUADS);
			BrownCube();
			glEnd();
			glPopMatrix();
		}

		//brownBlock
		glPushMatrix();
		glTranslatef(-16.5f, -10.0f, -35.2f);
		glScalef(1.0f, 7.5f, 0.2f);
		glBegin(GL_QUADS);
		BrownCube();
		glEnd();
		glPopMatrix();
		glPopMatrix();
	}

	//right side hidden bottom block
	{
		glPushMatrix();
		glTranslatef(-57.0f, -24.5f, 93.2f);
		glRotatef(90.0f, 0.0f, 1.0, 0.0f);
		glScalef(2.9f, 1.0f, 10.9f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		/*//back face
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.05f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		/*//bottom face
		glColor3f(0.75f, 0.75f, 0.75f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();
	}

	//right side stair
	{
		glPushMatrix();
		glTranslatef(-7.0f, -27.5f, 77.2f);
		glScalef(2.7f, 0.7f, 1.0f);
		myCube();
		glPopMatrix();
		glPopMatrix();
	}

	//left side curve door
	{
		glColor3f(0.9f, 0.9f, 0.9f);
		glPushMatrix();
		glTranslatef(45.0f, 10.0f, 85.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		DrawShape(18.0f, 10.0f, 50);
		glPopMatrix();
	}
	glPopMatrix();

}

void translate()       //leftstairs and rightstaird translation
{
	/*if (i >-70.0f)
	{

	if (fLeftTranslate >4.0f)
	fLeftTranslate = fLeftTranslate - 0.0135f;
	if (fRightTranslate <-2.0f)
	fRightTranslate = fRightTranslate + 0.015f;
	}*/
	fLeftTranslate = 4.0f;
	fRightTranslate = -2.0f;

}

void Shivling()
{
	glPushMatrix();
	static GLfloat fShivlingColor = 0.0f;
	//static GLfloat fMovement = -60.0f;
	static GLfloat fMovement = -28.0f;
	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	mygluLookAt();*/
	glTranslatef(200.0f, fMovement, 35.0f);
	/*if (fMovement < -28.0f)
	fMovement = fMovement + 0.009;*/

	glPushMatrix();//(1) main push

				   //bottom cube
	{
		glTranslatef(0.0f, 0.0f, 0.0f);
		glScalef(3.0f, 0.5f, 3.0f);
		glPushMatrix();                  //bottom cube push
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(5.0f, -5.0f, 5.0f);

		//right face
		glNormal3f(1.0f, 0.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);
		glColor3f(0.35f, 0.35f, 0.35f);
		glVertex3f(-5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, 5.0f);

		//top face
		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(0.40f, 0.40f, 0.40f);
		glVertex3f(-5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, 5.0f);
		glVertex3f(5.0f, 5.0f, -5.0f);
		glVertex3f(-5.0f, 5.0f, -5.0f);

		/*//bottom face
		glColor3f(0.75f, 0.75f, 0.75f);
		glVertex3f(-5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, 5.0f);
		glVertex3f(5.0f, -5.0f, -5.0f);
		glVertex3f(-5.0f, -5.0f, -5.0f);*/

		glEnd();
		glPopMatrix();//bottom cube pop
	}

	//second cube from bottom
	{

		//if (fMovement == -30.0f)
		{
			glTranslatef(0.0f, 3.0f, 0.0f);
			glScalef(1.1f, 0.3f, 1.1f);
			glPushMatrix();                  //bottom cube push
			glBegin(GL_QUADS);
			//front face
			glNormal3f(0.0f, 0.0f, 1.0f);
			glColor3f(0.25f, 0.25f, 0.25f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glColor3f(0.40f, 0.40f, 0.40f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glColor3f(0.25f, 0.25f, 0.25f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glColor3f(0.40f, 0.40f, 0.40f);
			glVertex3f(5.0f, -5.0f, 5.0f);

			//right face
			glNormal3f(1.0f, 0.0f, 0.0f);
			glColor3f(0.25f, 0.25f, 0.25f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);

			//back face
			glNormal3f(0.0f, 0.0f, -1.0f);
			glColor3f(0.25f, 0.25f, 0.25f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);

			//left face
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glColor3f(0.25f, 0.25f, 0.25f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, 5.0f);

			//top face
			glNormal3f(0.0f, 1.0f, 0.0f);
			glColor3f(0.25f, 0.25f, 0.05f);
			glVertex3f(-5.0f, 5.0f, 5.0f);
			glVertex3f(5.0f, 5.0f, 5.0f);
			glVertex3f(5.0f, 5.0f, -5.0f);
			glVertex3f(-5.0f, 5.0f, -5.0f);

			/*//bottom face
			glColor3f(0.75f, 0.75f, 0.75f);
			glVertex3f(-5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, 5.0f);
			glVertex3f(5.0f, -5.0f, -5.0f);
			glVertex3f(-5.0f, -5.0f, -5.0f);*/

			glEnd();
			glPopMatrix();//bottom cube pop
		}
	}


	//bottom cylinder(ulti kundi)
	{
		glTranslatef(0.0f, 0.0f, 0.0f);
		glPushMatrix();

		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.5f, 0.0f, 0.0f);

		gluCylinder(quadric, 3.0, 2.0, 70.0, 50, 50);
		glPopMatrix();


	}

	//torus
	{
		glTranslatef(0.0f, 70.0f, 0.0f);
		glPushMatrix();

		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.5f, 0.0f, 0.0f);

		gluCylinder(quadric, 2.5, 2.5, 10.0, 50, 50);
		glPopMatrix();
	}
	//upper cylinder
	{
		glTranslatef(0.0f, 5.0f, 0.0f);
		glPushMatrix();
		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.5f, 0.0f, 0.0f);

		gluCylinder(quadric, 2.0, 3.0, 70.0, 50, 50);
		glPopMatrix();
	}
	//ling
	/*{
	glTranslatef(0.0f, 75.0f, 0.0f);

	glPushMatrix();
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	quadric2 = gluNewQuadric();
	glColor3f(0.5f, 0.0f, 0.0f);

	gluSphere(quadric2, 5.0, 100.0, 100.0);
	glPopMatrix();
	}*/

	glPopMatrix();
	glPopMatrix();

}

void ShivMandir()
{
	void StairCube(void);

	glPushMatrix();
	static GLfloat fVerticalMovement = -30.0f;
	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	mygluLookAt();*/

	glTranslatef(400.0f, fVerticalMovement, 35.0f);
	if (fVerticalMovement < -30.0f)
		fVerticalMovement = fVerticalMovement + 0.025f;

	glScalef(1.0f, 1.0f, 1.0f);
	glPushMatrix();//main push
	glPushMatrix();
	glTranslatef(0.0f, 95.0f, 0.0f);
	glScalef(1.0f, 0.6f, 1.0f);
	//Centre Block
	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, 0.0f);
		glScalef(5.0f, 45.0f, 20.0f);
		//glScalef(5.0f, 45.0f, 17.0f);

		myCube();//main cube
		glPopMatrix();
	}

	//Right Block
	{   glPushMatrix();
	glTranslatef(10.0f, 0.0, 120.0f);
	glScalef(5.0f, 40.0f, 6.0f);

	myCube();
	glPopMatrix();
	}
	//left block
	{
		{   glPushMatrix();
		glTranslatef(10.0f, 0.0, -120.0f);
		glScalef(5.0f, 40.0f, 6.0f);

		myCube();
		glPopMatrix();
		}
	}
	glPopMatrix();
	//front door on temple
	{
		glPushMatrix();
		glTranslatef(-35.0f, 40.0f, 0.0f);
		glScalef(0.1f, 8.0f, 4.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

		glColor3f(0.745f, 0.137f, 0.137f);
		glBegin(GL_QUADS);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);

		glEnd();
		glPopMatrix();
	}
	//left side door on temple
	{
		glPushMatrix();
		glTranslatef(-20.0f, 40.0f, -123.0f);
		glScalef(0.1f, 8.0f, 4.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

		glColor3f(0.745f, 0.137f, 0.137f);
		glBegin(GL_QUADS);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);

		glEnd();
		glPopMatrix();
	}
	//right side door on temple
	{
		glPushMatrix();
		glTranslatef(-20.0f, 40.0f, 123.0f);
		glScalef(0.1f, 8.0f, 4.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

		glColor3f(0.745f, 0.137f, 0.137f);
		glBegin(GL_QUADS);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);

		glEnd();
		glPopMatrix();
	}
	//white color patch(middle)
	{
		glPushMatrix();
		glTranslatef(-35.0f, 105.0f, 0.0f);
		glScalef(0.2f, 1.0f, 33.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);
		glEnd();
		glPopMatrix();

	}
	//left side white color patch
	{

		glPushMatrix();
		glTranslatef(-18.0f, 105.0f, -90.0f);
		glScalef(0.2f, 1.0f, 20.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);
		glEnd();
		glPopMatrix();


	}
	//right side white patch
	{


		glPushMatrix();
		glTranslatef(-18.0f, 105.0f, 90.0f);
		glScalef(0.2f, 1.0f, 20.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);
		glEnd();
		glPopMatrix();

	}
	//curve above centre door and cubes supprting that curve
	{
		glPushMatrix();
		glTranslatef(-35.0f, 55.0f, 0.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(1.0f, 1.0f, 1.0f);

		gluPartialDisk(quadric, 17.0, 20.0, 50, 20, 0.0, 180.0);
		glPopMatrix();

		//left cube
		glPushMatrix();
		glTranslatef(-35.0f, 55.0f, -17.0f);
		glScalef(0.1f, 0.5f, 1.5f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);
		glEnd();
		glPopMatrix();

		//right cube
		glPushMatrix();
		glTranslatef(-35.0f, 55.0f, 17.0f);
		glScalef(0.1f, 0.5f, 1.5f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);
		glEnd();
		glPopMatrix();
	}
	//curve above left door and cubes supprting that curve
	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -13.0f);
		glPushMatrix();
		glTranslatef(-20.0f, 57.0f, -110.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(1.0f, 1.0f, 1.0f);

		gluPartialDisk(quadric, 17.0, 20.0, 50, 20, 0.0, 180.0);
		glPopMatrix();

		//left cube
		glPushMatrix();
		glTranslatef(-23.0f, 56.0f, -128.0f);
		glScalef(0.1f, 0.5f, 1.5f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);
		glEnd();
		glPopMatrix();

		//right cube
		glPushMatrix();
		glTranslatef(-23.0f, 56.0f, -90.0f);
		glScalef(0.1f, 0.5f, 1.5f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);
		glEnd();
		glPopMatrix();

		glPopMatrix();
	}

	//curve above right door and cubes supprting that curve
	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, 13.0f);
		glPushMatrix();
		glTranslatef(-20.0f, 57.0f, 110.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(1.0f, 1.0f, 1.0f);

		gluPartialDisk(quadric, 17.0, 20.0, 50, 20, 0.0, 180.0);
		glPopMatrix();

		//left cube
		glPushMatrix();
		glTranslatef(-23.0f, 56.0f, 90.0f);
		glScalef(0.1f, 0.5f, 1.5f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);
		glEnd();
		glPopMatrix();

		//right cube
		glPushMatrix();
		glTranslatef(-23.0f, 56.0f, 128.0f);
		glScalef(0.1f, 0.5f, 1.5f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);
		glEnd();
		glPopMatrix();

		glPopMatrix();
	}

	//left side window
	{

		glPushMatrix();
		glTranslatef(-20.0f, 140.0f, -123.0f);
		glScalef(0.1f, 4.0f, 4.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

		glColor3f(0.745f, 0.137f, 0.137f);
		glBegin(GL_QUADS);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);

		glEnd();
		glPopMatrix();
	}
	//curve above left side window and cubes supprting that curve
	{   glPushMatrix();
	glTranslatef(0.0f, 0.0f, -13.0f);
	glPushMatrix();
	glTranslatef(-20.0f, 145.0f, -110.0f);
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
	quadric = gluNewQuadric();
	glColor3f(1.0f, 1.0f, 1.0f);

	gluPartialDisk(quadric, 17.0, 20.0, 50, 20, 0.0, 180.0);
	glPopMatrix();

	//left cube
	glPushMatrix();
	glTranslatef(-23.0f, 145.0f, -128.0f);
	glScalef(0.1f, 0.5f, 1.5f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, -3.0f, 0.0f);
	glVertex3f(3.0f, -3.0f, 0.0f);
	glEnd();
	glPopMatrix();

	//right cube
	glPushMatrix();
	glTranslatef(-23.0f, 145.0f, -90.0f);
	glScalef(0.1f, 0.5f, 1.5f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, -3.0f, 0.0f);
	glVertex3f(3.0f, -3.0f, 0.0f);
	glEnd();
	glPopMatrix();

	glPopMatrix();
	}


	//right side window

	{

		glPushMatrix();
		glTranslatef(-20.0f, 140.0f, 123.0f);
		glScalef(0.1f, 4.0f, 4.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

		glColor3f(0.745f, 0.137f, 0.137f);
		glBegin(GL_QUADS);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 0.0f);
		glVertex3f(-3.0f, -3.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, 0.0f);

		glEnd();
		glPopMatrix();
	}
	//curve above right side window and cubes supprting that curve
	{   glPushMatrix();
	glTranslatef(0.0f, 0.0f, 13.0f);
	glPushMatrix();
	glTranslatef(-20.0f, 145.0f, 110.0f);
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
	quadric = gluNewQuadric();
	glColor3f(1.0f, 1.0f, 1.0f);

	gluPartialDisk(quadric, 17.0, 20.0, 50, 20, 0.0, 180.0);
	glPopMatrix();

	//left cube
	glPushMatrix();
	glTranslatef(-23.0f, 145.0f, 128.0f);
	glScalef(0.1f, 0.5f, 1.5f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, -3.0f, 0.0f);
	glVertex3f(3.0f, -3.0f, 0.0f);
	glEnd();
	glPopMatrix();

	//right cube
	glPushMatrix();
	glTranslatef(-23.0f, 145.0f, 90.0f);
	glScalef(0.1f, 0.5f, 1.5f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, 3.0f, 0.0f);
	glVertex3f(-3.0f, -3.0f, 0.0f);
	glVertex3f(3.0f, -3.0f, 0.0f);
	glEnd();
	glPopMatrix();

	glPopMatrix();
	}

	//curve above middle windows 
	{

		{
			glPushMatrix();
			glTranslatef(-35.0f, 140.0f, 0.0f);
			glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
			glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
			quadric = gluNewQuadric();
			glColor3f(1.0f, 1.0f, 1.0f);

			gluPartialDisk(quadric, 67.0, 70.0, 50, 20, 0.0, 180.0);
			glPopMatrix();

			//left cube
			glPushMatrix();
			glTranslatef(-35.0f, 140.0f, -67.0f);
			glScalef(0.1f, 0.5f, 1.5f);
			glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_QUADS);
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(3.0f, 3.0f, 0.0f);
			glVertex3f(-3.0f, 3.0f, 0.0f);
			glVertex3f(-3.0f, -3.0f, 0.0f);
			glVertex3f(3.0f, -3.0f, 0.0f);
			glEnd();
			glPopMatrix();

			//right cube
			glPushMatrix();
			glTranslatef(-35.0f, 140.0f, 67.0f);
			glScalef(0.1f, 0.5f, 1.5f);
			glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_QUADS);
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(3.0f, 3.0f, 0.0f);
			glVertex3f(-3.0f, 3.0f, 0.0f);
			glVertex3f(-3.0f, -3.0f, 0.0f);
			glVertex3f(3.0f, -3.0f, 0.0f);
			glEnd();
			glPopMatrix();
		}
	}
	//middle three curves on centre block
	{

		glPushMatrix();
		glTranslatef(0.0f, 5.0f, 0.0f);
		//middle curve
		glPushMatrix();
		glTranslatef(-35.0f, 140.0f, 0.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(1.0f, 1.0f, 1.0f);

		gluPartialDisk(quadric, 17.0, 20.0, 50, 20, 0.0, 180.0);
		glPopMatrix();

		//left curve
		glPushMatrix();
		glTranslatef(-35.0f, 140.0f, -35.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(1.0f, 1.0f, 1.0f);

		gluPartialDisk(quadric, 12.0, 15.0, 50, 20, 0.0, 180.0);
		glPopMatrix();

		//right curve
		glPushMatrix();
		glTranslatef(-35.0f, 140.0f, 35.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(1.0f, 1.0f, 1.0f);

		gluPartialDisk(quadric, 12.0, 15.0, 50, 20, 0.0, 180.0);
		glPopMatrix();
		glPopMatrix();

	}
	//balcony above centre door
	{

		glPushMatrix();
		glTranslatef(-40.0f, 115.0f, 0.0f);
		glScalef(2.0f, 4.0f, 19.0f);
		glBegin(GL_QUADS);
		//front face
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(3.0f, 3.0f, 3.0f);
		glVertex3f(-3.0f, 3.0f, 3.0f);
		glVertex3f(-3.0f, -3.0f, 3.0f);
		glVertex3f(3.0f, -3.0f, 3.0f);

		//left face
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-3.0f, 3.0f, 3.0f);
		glVertex3f(-3.0f, 3.0f, -3.0f);
		glVertex3f(-3.0f, -3.0f, -3.0f);
		glVertex3f(-3.0f, -3.0f, 3.0f);

		//back face
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(3.0f, 3.0f, -3.0f);
		glVertex3f(-3.0f, 3.0f, -3.0f);
		glVertex3f(-3.0f, -3.0f, -3.0f);
		glVertex3f(3.0f, -3.0f, -3.0f);

		//bottom face
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(3.0f, -3.0f, -3.0f);
		glVertex3f(-3.0f, -3.0f, -3.0f);
		glVertex3f(-3.0f, -3.0f, 3.0f);
		glVertex3f(3.0f, -3.0f, 3.0f);
		glEnd();
		glPopMatrix();
	}
	//cube design above main door
	{
		for (GLfloat hcubes = -40.0f, vcubes = 95.0f; hcubes < -20.0f && vcubes > 80.0f; hcubes = hcubes + 3.0f, vcubes = vcubes - 3.0f)
		{
			glPushMatrix();
			glTranslatef(hcubes, vcubes, 0.0f);
			glScalef(2.0f, 2.0f, 18.0f);
			glBegin(GL_QUADS);
			//front face
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(3.0f, 3.0f, 3.0f);
			glVertex3f(-3.0f, 3.0f, 3.0f);
			glVertex3f(-3.0f, -3.0f, 3.0f);
			glVertex3f(3.0f, -3.0f, 3.0f);

			//left face
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glVertex3f(-3.0f, 3.0f, 3.0f);
			glVertex3f(-3.0f, 3.0f, -3.0f);
			glVertex3f(-3.0f, -3.0f, -3.0f);
			glVertex3f(-3.0f, -3.0f, 3.0f);

			//back face
			glNormal3f(0.0f, 0.0f, -1.0f);
			glVertex3f(3.0f, 3.0f, -3.0f);
			glVertex3f(-3.0f, 3.0f, -3.0f);
			glVertex3f(-3.0f, -3.0f, -3.0f);
			glVertex3f(3.0f, -3.0f, -3.0f);

			//bottom face
			glNormal3f(0.0f, -1.0f, 0.0f);
			glVertex3f(3.0f, -3.0f, -3.0f);
			glVertex3f(-3.0f, -3.0f, -3.0f);
			glVertex3f(-3.0f, -3.0f, 3.0f);
			glVertex3f(3.0f, -3.0f, 3.0f);
			glEnd();
			glPopMatrix();
		}
	}
	//left side disk design of main door
	{   //biggest curve
		glPushMatrix();
		glTranslatef(-30.0f, 80.0f, -35.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.50f, 0.50f, 0.50f);

		gluDisk(quadric, 0.0, 15.0, 50, 50);
		glPopMatrix();

		//bigger curve
		glPushMatrix();
		glTranslatef(-30.0f, 67.0f, -35.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.50f, 0.50f, 0.50f);

		gluDisk(quadric, 0.0, 8.0, 50, 50);
		glPopMatrix();

		//big curve
		glPushMatrix();
		glTranslatef(-30.0f, 60.0f, -35.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.50f, 0.50f, 0.50f);

		gluDisk(quadric, 0.0, 4.0, 50, 50);
		glPopMatrix();
	}
	//right side disk design of main door
	{   //biggest curve
		glPushMatrix();
		glTranslatef(-30.0f, 80.0f, 35.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.50f, 0.50f, 0.50f);

		gluDisk(quadric, 0.0, 15.0, 50, 50);
		glPopMatrix();

		//bigger curve
		glPushMatrix();
		glTranslatef(-30.0f, 67.0f, 35.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.50f, 0.50f, 0.50f);

		gluDisk(quadric, 0.0, 8.0, 50, 50);
		glPopMatrix();

		//big curve
		glPushMatrix();
		glTranslatef(-30.0f, 60.0f, 35.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.50f, 0.50f, 0.50f);

		gluDisk(quadric, 0.0, 4.0, 50, 50);
		glPopMatrix();
	}

	//Left side cynlinder near main door
	{  //cylinder
		glPushMatrix();
		glTranslatef(-35.0f, 0.0f, -60.0f);

		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.25f, 0.20f, 0.20f);

		gluCylinder(quadric, 35.0, 35.0, 20.0, 50, 50);
		glPopMatrix();

		//Disk
		glPushMatrix();
		glTranslatef(-35.0f, 20.0f, -60.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.50f, 0.50f, 0.50f);

		gluDisk(quadric, 0.0, 35.0, 50, 50);
		glPopMatrix();
	}

	//right side cylinder near main door
	{  //cylinder
		glPushMatrix();
		glTranslatef(-35.0f, 0.0f, 60.0f);

		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.25f, 0.20f, 0.20f);

		gluCylinder(quadric, 35.0, 35.0, 20.0, 50, 50);
		glPopMatrix();

		//Disk
		glPushMatrix();
		glTranslatef(-35.0f, 20.0f, 60.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		quadric = gluNewQuadric();
		glColor3f(0.50f, 0.50f, 0.50f);

		gluDisk(quadric, 0.0, 35.0, 50, 50);
		glPopMatrix();
	}

	//main door stairs
	{   //stair no.1
		glPushMatrix();
		glTranslatef(-35.0f, 11.0f, 0.0f);
		glScalef(1.0f, 0.5f, 2.8f);
		StairCube();
		glPopMatrix();

		//stair no.2
		glPushMatrix();
		glTranslatef(-39.0f, 6.0f, 0.0f);
		glScalef(1.0f, 0.5f, 2.8f);
		StairCube();
		glPopMatrix();

		//stair no.3
		glPushMatrix();
		glTranslatef(-43.0f, 2.0f, 0.0f);
		glScalef(1.0f, 0.5f, 2.8f);
		StairCube();
		glPopMatrix();
	}
	//left side door stairs
	{   //stair no.1
		glPushMatrix();
		glTranslatef(-25.0f, 11.0f, -123.0f);
		glScalef(1.0f, 0.5f, 2.8f);
		StairCube();
		glPopMatrix();

		//stair no.2
		glPushMatrix();
		glTranslatef(-29.0f, 6.0f, -123.0f);
		glScalef(1.0f, 0.5f, 2.8f);
		StairCube();
		glPopMatrix();

		//stair no.3
		glPushMatrix();
		glTranslatef(-33.0f, 2.0f, -123.0f);
		glScalef(1.0f, 0.5f, 2.8f);
		StairCube();
		glPopMatrix();
	}
	//right side door stairs
	{   //stair no.1
		glPushMatrix();
		glTranslatef(-25.0f, 11.0f, 123.0f);
		glScalef(1.0f, 0.5f, 2.8f);
		StairCube();
		glPopMatrix();

		//stair no.2
		glPushMatrix();
		glTranslatef(-29.0f, 6.0f, 123.0f);
		glScalef(1.0f, 0.5f, 2.8f);
		StairCube();
		glPopMatrix();

		//stair no.3
		glPushMatrix();
		glTranslatef(-33.0f, 2.0f, 123.0f);
		glScalef(1.0f, 0.5f, 2.8f);
		StairCube();
		glPopMatrix();
	}
	//astriods
	{
		for (GLfloat astroids = -50.0f; astroids<55.0f; astroids = astroids + 10.0f)
		{   //astroid no.1
			glPushMatrix();
			glTranslatef(-48.0f, 115.0f, astroids);
			glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
			//glScalef(1.0f, 1.0f, 1.0f);
			glColor3f(1.0f, 0.50f, 0.80f);
			glPointSize(2.0);
			glBegin(GL_POINTS);
			for (GLfloat angle = 0.0f; angle < 2.0f*3.14; angle = angle + 0.01f)
			{
				glVertex3f(5 * (pow(cos(angle), 3)), 5 * (pow(sin(angle), 3)), 0.0f);
			}
			glEnd();
			glPopMatrix();

			//astroid no 2
			glPushMatrix();
			glTranslatef(-48.0f, 115.0f, astroids);
			glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
			glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
			//glScalef(1.0f, 1.0f, 1.0f);
			glColor3f(1.0f, 0.50f, 0.80f);
			glPointSize(2.0);
			glBegin(GL_POINTS);
			for (GLfloat angle = 0.0f; angle < 2.0f*3.14; angle = angle + 0.01f)
			{
				glVertex3f(8 * (pow(cos(angle), 3)), 8 * (pow(sin(angle), 3)), 0.0f);
			}
			glEnd();
			glPopMatrix();
		}
	}
	//astroid above middle three curves
	{   //astroid no1.
		glPushMatrix();
		glTranslatef(-34.0f, 185.0f, 0.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		//glScalef(1.0f, 1.0f, 1.0f);
		glColor3f(1.0f, 0.50f, 0.80f);
		glPointSize(2.0);
		glBegin(GL_POINTS);
		for (GLfloat angle = 0.0f; angle < 2.0f*3.14; angle = angle + 0.01f)
		{
			glVertex3f(8 * (pow(cos(angle), 3)), 8 * (pow(sin(angle), 3)), 0.0f);
		}
		glEnd();
		glPopMatrix();

		//astroid no 2
		glPushMatrix();
		glTranslatef(-34.0f, 185.0f, 0.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
		//glScalef(1.0f, 1.0f, 1.0f);
		glColor3f(1.0f, 0.50f, 0.80f);
		glPointSize(2.0);
		glBegin(GL_POINTS);
		for (GLfloat angle = 0.0f; angle < 2.0f*3.14; angle = angle + 0.01f)
		{
			glVertex3f(8 * (pow(cos(angle), 3)), 8 * (pow(sin(angle), 3)), 0.0f);
		}
		glEnd();
		glPopMatrix();
	}
	//main door railing
	{  //left railing
		glPushMatrix();
		glTranslatef(-53.0f, 15.0f, -14.0f);

		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.0f, 1.5f, 1.5f);

		quadric = gluNewQuadric();
		glColor3f(0.75f, 0.75f, 0.75f);

		gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
		glPopMatrix();

		//left bars
		for (GLfloat hbars = -53.0f, vbars = -15.0f; hbars < -20.0f && vbars <5.0f; hbars = hbars + 3.0f, vbars = vbars + 3.0f)
		{
			glPushMatrix();
			glTranslatef(hbars, vbars, -14.0f);

			glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
			//glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
			glScalef(1.0f, 1.5f, 1.5f);

			quadric = gluNewQuadric();
			glColor3f(0.75f, 0.75f, 0.75f);

			gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
			glPopMatrix();
		}
		//right railing
		glPushMatrix();
		glTranslatef(-53.0f, 15.0f, 14.0f);

		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.0f, 1.5f, 1.5f);

		quadric = gluNewQuadric();
		glColor3f(0.75f, 0.75f, 0.75f);

		gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
		glPopMatrix();

		//right bars
		for (GLfloat hbars = -53.0f, vbars = -15.0f; hbars < -20.0f && vbars <5.0f; hbars = hbars + 3.0f, vbars = vbars + 3.0f)
		{
			glPushMatrix();
			glTranslatef(hbars, vbars, 14.0f);

			glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
			//glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
			glScalef(1.0f, 1.5f, 1.5f);

			quadric = gluNewQuadric();
			glColor3f(0.75f, 0.75f, 0.75f);

			gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
			glPopMatrix();
		}

	}

	//left door railing
	{

		glPushMatrix();
		glTranslatef(12.0f, 0.0, -123.0f);
		//left railing
		glPushMatrix();
		glTranslatef(-53.0f, 15.0f, -14.0f);

		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.0f, 1.5f, 1.5f);

		quadric = gluNewQuadric();
		glColor3f(0.75f, 0.75f, 0.75f);

		gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
		glPopMatrix();

		//left bars
		for (GLfloat hbars = -53.0f, vbars = -15.0f; hbars < -20.0f && vbars <5.0f; hbars = hbars + 3.0f, vbars = vbars + 3.0f)
		{
			glPushMatrix();
			glTranslatef(hbars, vbars, -14.0f);

			glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
			//glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
			glScalef(1.0f, 1.5f, 1.5f);

			quadric = gluNewQuadric();
			glColor3f(0.75f, 0.75f, 0.75f);

			gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
			glPopMatrix();
		}
		//right railing
		glPushMatrix();
		glTranslatef(-53.0f, 15.0f, 14.0f);

		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.0f, 1.5f, 1.5f);

		quadric = gluNewQuadric();
		glColor3f(0.75f, 0.75f, 0.75f);

		gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
		glPopMatrix();

		//right bars
		for (GLfloat hbars = -53.0f, vbars = -15.0f; hbars < -20.0f && vbars <5.0f; hbars = hbars + 3.0f, vbars = vbars + 3.0f)
		{
			glPushMatrix();
			glTranslatef(hbars, vbars, 14.0f);

			glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
			//glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
			glScalef(1.0f, 1.5f, 1.5f);

			quadric = gluNewQuadric();
			glColor3f(0.75f, 0.75f, 0.75f);

			gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
			glPopMatrix();
		}
		glPopMatrix();
	}
	//right door railing
	{

		glPushMatrix();
		glTranslatef(12.0f, 0.0, 123.0f);
		//left railing
		glPushMatrix();
		glTranslatef(-53.0f, 15.0f, -14.0f);

		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.0f, 1.5f, 1.5f);

		quadric = gluNewQuadric();
		glColor3f(0.75f, 0.75f, 0.75f);

		gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
		glPopMatrix();

		//left bars
		for (GLfloat hbars = -53.0f, vbars = -15.0f; hbars < -20.0f && vbars <5.0f; hbars = hbars + 3.0f, vbars = vbars + 3.0f)
		{
			glPushMatrix();
			glTranslatef(hbars, vbars, -14.0f);

			glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
			//glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
			glScalef(1.0f, 1.5f, 1.5f);

			quadric = gluNewQuadric();
			glColor3f(0.75f, 0.75f, 0.75f);

			gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
			glPopMatrix();
		}
		//right railing
		glPushMatrix();
		glTranslatef(-53.0f, 15.0f, 14.0f);

		glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.0f, 1.5f, 1.5f);

		quadric = gluNewQuadric();
		glColor3f(0.75f, 0.75f, 0.75f);

		gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
		glPopMatrix();

		//right bars
		for (GLfloat hbars = -53.0f, vbars = -15.0f; hbars < -20.0f && vbars <5.0f; hbars = hbars + 3.0f, vbars = vbars + 3.0f)
		{
			glPushMatrix();
			glTranslatef(hbars, vbars, 14.0f);

			glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
			//glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
			glScalef(1.0f, 1.5f, 1.5f);

			quadric = gluNewQuadric();
			glColor3f(0.75f, 0.75f, 0.75f);

			gluCylinder(quadric, 0.5, 0.5, 20.0, 50, 50);
			glPopMatrix();
		}
		glPopMatrix();
	}
	glPopMatrix();
	glPopMatrix();
}

void myCube()
{
	glBegin(GL_QUADS);
	//front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glColor3f(0.40f, 0.40f, 0.40f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glColor3f(0.40f, 0.40f, 0.40f);
	glVertex3f(5.0f, -5.0f, 5.0f);

	//right face

	glNormal3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);

	//back face
	glNormal3f(0.0f, 0.0f, -1.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);

	//left face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	//top face
	glNormal3f(0.0f, 1.0f, 0.0f);
	glColor3f(0.05f, 0.05f, 0.05f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glColor3f(0.15f, 0.15f, 0.15f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);

	/*//bottom face
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/

	glEnd();
}

void ShivMandirFloor()
{
	glPushMatrix();
	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	mygluLookAt();*/

	glTranslatef(290.0f, -30.0f, 35.0f);

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glScalef(35.0f, 0.1f, 30.0f);
	myCube();
	glPopMatrix();
	glPopMatrix();

}

void BrownCube()
{
	//front face
	glNormal3f(0.0f, 0.0f, 1.0f);

	glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
	glVertex3f(3.0f, 3.0f, 3.0f);
	glVertex3f(-3.0f, 3.0f, 3.0f);

	//glColor3f(0.36f, 0.12f, 0.10f);
	glVertex3f(-3.0f, -3.0f, 3.0f);
	glVertex3f(3.0f, -3.0f, 3.0f);

	//Right face
	glNormal3f(1.0f, 0.0f, 0.0f);

	//glColor3f(0.36f, 0.25f, 0.20f);
	glVertex3f(3.0f, 3.0f, -3.0f);
	glVertex3f(3.0f, 3.0f, 3.0f);

	//glColor3f(0.36f, 0.12f, 0.10f);
	glVertex3f(3.0f, -3.0f, 3.0f);
	glVertex3f(3.0f, -3.0f, -3.0f);

	//left face 
	glNormal3f(-1.0f, 0.0f, 0.0f);

	//glColor3f(0.36f, 0.25f, 0.20f);
	glVertex3f(-3.0f, 3.0f, 3.0f);
	glVertex3f(-3.0f, 3.0f, -3.0f);

	//glColor3f(0.36f, 0.12f, 0.10f);
	glVertex3f(-3.0f, -3.0f, -3.0f);
	glVertex3f(-3.0f, -3.0f, 3.0f);

	//Top face
	glNormal3f(0.0f, 1.0f, 0.0f);

	//glColor3f(0.36f, 0.25f, 0.20f);
	glVertex3f(3.0f, 3.0f, -3.0f);
	glVertex3f(-3.0f, 3.0f, -3.0f);

	//glColor3f(0.36f, 0.12f, 0.10f);
	glVertex3f(-3.0f, 3.0f, 3.0f);
	glVertex3f(3.0f, 3.0f, 3.0f);

	//Bottom face
	glNormal3f(0.0f, -1.0f, 0.0f);

	//glColor3f(0.36f, 0.25f, 0.20f);
	glVertex3f(3.0f, -3.0f, -3.0f);
	glVertex3f(-3.0f, -3.0f, -3.0f);

	//glColor3f(0.36f, 0.12f, 0.10f);
	glVertex3f(-3.0f, -3.0f, 3.0f);
	glVertex3f(3.0f, -3.0f, 3.0f);
}

void StairCube()
{
	glBegin(GL_QUADS);
	//front face
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glColor3f(0.40f, 0.40f, 0.40f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glColor3f(0.40f, 0.40f, 0.40f);
	glVertex3f(5.0f, -5.0f, 5.0f);

	//right face

	glNormal3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);

	//back face
	glNormal3f(0.0f, 0.0f, -1.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);

	//left face
	glNormal3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.50f, 0.50f, 0.50f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);
	//glColor3f(0.25f, 0.13f, 0.10f);
	glVertex3f(-5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, 5.0f);

	//top face
	glNormal3f(0.0f, 1.0f, 0.0f);
	glColor3f(0.05f, 0.05f, 0.05f);
	glVertex3f(-5.0f, 5.0f, 5.0f);
	glVertex3f(5.0f, 5.0f, 5.0f);
	glColor3f(0.15f, 0.15f, 0.15f);
	glVertex3f(5.0f, 5.0f, -5.0f);
	glVertex3f(-5.0f, 5.0f, -5.0f);

	/*//bottom face
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex3f(-5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, 5.0f);
	glVertex3f(5.0f, -5.0f, -5.0f);
	glVertex3f(-5.0f, -5.0f, -5.0f);*/

	glEnd();
}
////////////////////////////////////////////////
// Atul Linganwar Balcony
///////----------------Atul-----------///////////////

void DrawCorridorFloor(void)
{
	void balcony(GLfloat width, GLfloat height, GLfloat depth, GLint noOfPillers, GLfloat distBetweenPillers, GLfloat PillerHeight, GLfloat middleHalfDistance, GLint i);
	void BaseQuad(GLfloat, GLfloat, GLfloat);
	void LowerFloorQuad(GLfloat, GLfloat, GLfloat);
	void MiddleLeftFloorQuad(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
	void MiddleRightFloorQuad(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
	void TopFloorQuad(GLfloat, GLfloat, GLfloat);

	glPushMatrix();
	{
		

		glPushMatrix();
		glTranslatef(0.0f, 34.5f, -82.0f);
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		LowerFloorQuad(204.0f, 16.0f, 0.0f);
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(-52.0f, 34.5f, -98.0f);
			glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
			MiddleLeftFloorQuad(36.0f, 8.0f, 0.0f, 32.5f, 34.0f);
		}
		glPopMatrix();
		
		glPushMatrix();
		{
			glTranslatef(52.0f, 34.5f, -98.0f);
			glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
			MiddleRightFloorQuad(36.0f, 8.0f, 0.0f, 34.0f, 32.5f);
		}
		glPopMatrix();
	
		glPushMatrix();
		{
			glTranslatef(0.0f, 34.5f, -106.0f);
			glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
			TopFloorQuad(72.0f, 6.0f, 0.0f);
		}
		glPopMatrix();
		
		glPushMatrix();
		{
			glTranslatef(-85.0f, 34.5f, -106.0f);
			glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
			TopFloorQuad(34.0f, 6.0f, 0.0f);
		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(85.0f, 34.5f, -106.0f);
			glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
			TopFloorQuad(34.0f, 6.0f, 0.0f);
		}
		glPopMatrix();
	}
	glPopMatrix();
}

void TopFloorQuad(GLfloat width, GLfloat height, GLfloat depth)
{
	void BaseQuad(GLfloat, GLfloat, GLfloat);
	void BrownQuad(GLfloat, GLfloat, GLfloat);
	void HorizontalBlackBrownQuad(GLfloat);
	void VerticalBlackBrownQuad(GLfloat);

	glPushMatrix();
	glTranslatef(0.0f, height - 0.5f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(width, 0.5f, 0.0f);
	glPopMatrix();

	glPushMatrix();										//horizontal First yellow quad
	glTranslatef(0.0f, height - 2.5f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(width, 2.0f, depth);
	glPopMatrix();

	glPushMatrix();										//lower Second black strip
	glTranslatef(0.0f, height - 3.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(width - 3.0f - 2.0f, 0.5f, depth);
	glPopMatrix();

	glPushMatrix();										//horizontal Second yellow quad
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(width - 5.0f, height - 3.0f, depth);
	glPopMatrix();

	glPushMatrix();										//right Second black strip
	glTranslatef(width / 2.0f + (0.5f / 2.0f), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height, depth);
	glPopMatrix();

	glPushMatrix();										//right First black strip
	glTranslatef(width / 2.0f - 2.0f - (0.5f / 2.0f), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//left Second black strip
	glTranslatef(-(width / 2.0f + (0.5f / 2.0f)), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height, depth);
	glPopMatrix();

	glPushMatrix();										//left First black strip
	glTranslatef(-(width / 2.0f - 2.0f - (0.5f / 2.0f)), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//vertical right yellow quad
	glTranslatef(width / 2.0f - (2.0f / 2.0f), 0.0, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(2.0f, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//vertical left yellow quad
	glTranslatef(-(width / 2.0f - (2.0f / 2.0f)), 0.0f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(2.0f, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.03f);
	glColor3f(0.925f, 0.447f, 0.333f);
	BrownQuad(width / 2.0f - 3.0f, height - 3.0f, 0.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, height - 2.5f, 0.03f);
	HorizontalBlackBrownQuad(width / 2.0f - 3.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-width / 2.0f + 2.0f, 0.5f, 0.03f);
	VerticalBlackBrownQuad(height);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(width / 2.0f, 0.5f, 0.03f);
	VerticalBlackBrownQuad(height);
	glPopMatrix();
}

void LowerFloorQuad(GLfloat width, GLfloat height, GLfloat depth)
{
	void BaseQuad(GLfloat, GLfloat, GLfloat);
	void BrownQuad(GLfloat, GLfloat, GLfloat);
	void HorizontalBlackBrownQuad(GLfloat);
	void VerticalBlackBrownQuad(GLfloat);

	glColor3f(0.0f, 0.0f, 0.05f);						// lower First black strip
	BaseQuad(width, 0.5f, depth);

	glPushMatrix();										//horizontal First yellow quad
	glTranslatef(0.0f, 0.5f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(width, 2.0f, depth);
	glPopMatrix();

	glPushMatrix();										//lower Second black strip
	glTranslatef(0.0f, 2.5f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(width - 3.0f - 2.0f, 0.5f, depth);
	glPopMatrix();

	glPushMatrix();										//horizontal Second yellow quad
	glTranslatef(0.0f, 3.0f, 0.0);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(width - 2.5f, height - 3.0f, depth);
	glPopMatrix();

	glPushMatrix();										//right Second black strip
	glTranslatef(width / 2.0f + (0.5f / 2.0f), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height, depth);
	glPopMatrix();

	glPushMatrix();										//right First black strip
	glTranslatef(width / 2.0f - 2.0f - (0.5f / 2.0f), 2.5f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//left Second black strip
	glTranslatef(-(width / 2.0f + (0.5f / 2.0f)), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height, depth);
	glPopMatrix();

	glPushMatrix();										//left First black strip
	glTranslatef(-(width / 2.0f - 2.0f - (0.5f / 2.0f)), 2.5f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//vertical right yellow quad
	glTranslatef(width / 2.0f - (2.0f / 2.0f), 0.5f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(2.0f, height - 0.5f, depth);
	glPopMatrix();

	glPushMatrix();										//vertical left yellow quad
	glTranslatef(-(width / 2.0f - (2.0f / 2.0f)), 0.5f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(2.0f, height - 0.5f, depth);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 3.0f, 0.03f);
	glColor3f(0.925f, 0.447f, 0.333f);
	BrownQuad(width / 2.0f - 3.5f, height - 3.0f, 0.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.5f, 0.03f);
	HorizontalBlackBrownQuad(width / 2.0f - 3.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-width / 2.0f + 2.0f, 2.5f, 0.03f);
	VerticalBlackBrownQuad(height);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(width / 2.0f, 2.5f, 0.03f);
	VerticalBlackBrownQuad(height);
	glPopMatrix();
	glPopMatrix();
}

void MiddleRightFloorQuad(GLfloat width, GLfloat height, GLfloat depth, GLfloat LeftWidth, GLfloat RightWidth)
{
	void BaseQuad(GLfloat, GLfloat, GLfloat);
	void BrownQuad(GLfloat, GLfloat, GLfloat);
	void HorizontalBlackBrownQuad(GLfloat);
	void VerticalBlackBrownQuad(GLfloat);


	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);						// lower First black strip
	BaseQuad(width, 0.5f, depth);
	glPopMatrix();

	glPushMatrix();										//horizontal First yellow quad
	glTranslatef(0.0f, 0.5f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(width, 2.0f, depth);
	glPopMatrix();

	glPushMatrix();										//lower Second black strip
	glTranslatef(0.0f, 2.5f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(width - 3.0f - 2.0f, 0.5f, depth);
	glPopMatrix();

	glPushMatrix();										//right Second black strip
	glTranslatef(width / 2.0f + (0.5f / 2.0f), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height, depth);
	glPopMatrix();

	glPushMatrix();										//right First black strip
	glTranslatef((width / 2.0f - 2.0f - (0.5f / 2.0f)), 2.5f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//left Second black strip
	glTranslatef(-(width / 2.0f + (0.5f / 2.0f)), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height, depth);
	glPopMatrix();

	glPushMatrix();										//left First black strip
	glTranslatef(-(width / 2.0f - 2.0f - (0.5f / 2.0f)), 2.5f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//vertical right yellow quad
	glTranslatef((width / 2.0f - (2.0f / 2.0f)), 0.5f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(2.0f, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//vertical left yellow quad
	glTranslatef(-(width / 2.0f - (2.0f / 2.0f)), 2.5f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(2.0f, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.5f, 0.03f);
	HorizontalBlackBrownQuad(width / 2.0f - 3.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef((-width / 2.0f + 2.0f), 2.5f, 0.03f);
	VerticalBlackBrownQuad(height);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(width / 2.0f, 2.5f, 0.03f);
	VerticalBlackBrownQuad(height);
	glPopMatrix();

	glPushMatrix();						//////////----------------right side
	glTranslatef(-(-RightWidth - width / 2.0f + (0.5f / 2.0f)), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5f, height, depth);								//left black

	glTranslatef(-((0.5f / 2.0f) + 2.0f / 2.0f), 0.0f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(2.0f, height, 0.0f);								//left yellow


	glTranslatef(-(2.0f / 2.0f + 0.5f / 2.0f), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5f, height, depth);								//left second black

	glTranslatef(-(RightWidth / 2.0f - 1.5f), 0.0f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(RightWidth - 3.5f, height, 0.0f);					//left second yellow
	glColor3f(0.925f, 0.447f, 0.333f);
	glTranslatef(0.0f, 0.0f, 0.03f);
	BrownQuad(RightWidth / 2.0f - 3.12f, height, 0.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-(-RightWidth - width / 2.0f) - 0.5f, 2.0f, 0.03f);
	VerticalBlackBrownQuad(height);
	glPopMatrix();

	////////////////---------------------left side

	glPushMatrix();
	glTranslatef(-(LeftWidth / 2.0f + width / 2.0f) - 0.5f, 0.0f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(LeftWidth, height, depth);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-(LeftWidth / 2.0f + width / 2.0f) - 0.9f, 0.0f, 0.03f);
	glColor3f(0.925f, 0.447f, 0.333f);
	BrownQuad(LeftWidth / 2.0f - 0.5f, height, depth);
	glPopMatrix();

}

void MiddleLeftFloorQuad(GLfloat width, GLfloat height, GLfloat depth, GLfloat LeftWidth, GLfloat RightWidth)
{

	void BaseQuad(GLfloat, GLfloat, GLfloat);
	void BrownQuad(GLfloat, GLfloat, GLfloat);
	void HorizontalBlackBrownQuad(GLfloat);
	void VerticalBlackBrownQuad(GLfloat);

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);						// lower First black strip
	BaseQuad(width, 0.5f, depth);
	glPopMatrix();

	glPushMatrix();										//horizontal First yellow quad
	glTranslatef(0.0f, 0.5f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(width, 2.0f, depth);
	glPopMatrix();

	glPushMatrix();										//lower Second black strip
	glTranslatef(0.0f, 2.5f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(width - 2.5f - 2.0f, 0.5f, depth);
	glPopMatrix();

	glPushMatrix();										//right Second black strip
	glTranslatef(width / 2.0f + (0.5f / 2.0f), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height, depth);
	glPopMatrix();

	glPushMatrix();										//right First black strip
	glTranslatef(width / 2.0f - 2.0f - (0.5f / 2.0f), 2.5f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//left Second black strip
	glTranslatef(-(width / 2.0f + (0.5f / 2.0f)), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height, depth);
	glPopMatrix();

	glPushMatrix();										//left First black strip
	glTranslatef(-(width / 2.0f - 2.0f - (0.5f / 2.0f)), 2.5f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//vertical right yellow quad
	glTranslatef(width / 2.0f - (2.0f / 2.0f), 2.5f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(2.0f, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();										//vertical left yellow quad
	glTranslatef(-(width / 2.0f - (2.0f / 2.0f)), 2.5f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(2.0f, height - 2.5f, depth);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.5f, 0.03f);
	HorizontalBlackBrownQuad(width / 2.0f - 3.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-width / 2.0f + 2.0f, 2.5f, 0.03f);
	VerticalBlackBrownQuad(height);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(width / 2.0f, 2.5f, 0.03f);
	VerticalBlackBrownQuad(height);
	glPopMatrix();

	glPushMatrix();						//////////----------------left side
	glTranslatef(-LeftWidth - width / 2.0f + (0.5f / 2.0f), 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5f, height, depth);								//left black

	glTranslatef((0.5f / 2.0f) + 2.0f / 2.0f, 0.0f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(2.0f, height, 0.0f);								//left yellow


	glTranslatef(2.0f / 2.0f + 0.5f / 2.0f, 0.0f, 0.05f);
	glColor3f(0.0f, 0.0f, 0.0f);
	BaseQuad(0.5f, height, depth);								//left second black

	glTranslatef(LeftWidth / 2.0f - 1.5f, 0.0f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(LeftWidth - 3.5f, height, 0.0f);					//left second yellow
	glColor3f(0.925f, 0.447f, 0.333f);
	glTranslatef(-0.5f, 0.0f, 0.03f);
	BrownQuad(LeftWidth / 2.0f - 3.12f, height, 0.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-LeftWidth - width / 2.0f + 2.5f, 2.0f, 0.03f);
	VerticalBlackBrownQuad(height);
	glPopMatrix();

	////////////////---------------------right side

	glPushMatrix();
	glTranslatef(RightWidth / 2.0f + width / 2.0f + 0.5f, 0.0f, 0.0f);
	glColor3f(0.850f, 0.698f, 0.588f);
	BaseQuad(RightWidth, height, depth);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(RightWidth / 2.0f + width / 2.0f + 0.9f, 0.0f, 0.03f);
	glColor3f(0.925f, 0.447f, 0.333f);
	BrownQuad(RightWidth / 2.0f - 0.5f, height, depth);
	glPopMatrix();
}

void BrownQuad(GLfloat xDim, GLfloat yDim, GLfloat zDim)
{

	void BrownQuadDesign(GLfloat, GLfloat, GLfloat);

	for (GLfloat i = -xDim; i <= xDim; i += 2.0f)
	{
		for (GLfloat j = 0.0f; j < yDim; j += 2.0f)
		{
			glPushMatrix();
			glTranslatef(i, j, 0.0f);
			BrownQuadDesign(2.0f, 2.0f, zDim);
			glPopMatrix();
		}
	}
}

void HorizontalBlackBrownQuad(GLfloat xDim)
{
	void BlackBrownQuadDesign(GLfloat, GLfloat, GLfloat);

	for (GLfloat i = -xDim; i <= xDim; i += 2.0f * 2.0f)
	{
		glPushMatrix();
		glTranslatef(i, 0.0f, 0.0f);
		BlackBrownQuadDesign(2.0f, 2.0f, 0.0f);
		glPopMatrix();
	}
}

void VerticalBlackBrownQuad(GLfloat yDim)
{
	void BlackBrownQuadDesign(GLfloat, GLfloat, GLfloat);

	for (GLfloat i = 0.0f; i < yDim; i += 2.0f * 2.0f)
	{
		glPushMatrix();
		glTranslatef(0.0f, i, 0.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		BlackBrownQuadDesign(2.0f, 2.0f, 0.0f);
		glPopMatrix();
	}
}

void BrownQuadDesign(GLfloat fQuadwidth, GLfloat fQuadheight, GLfloat fQuaddepth)
{
	M3DVector3f vNormal;

	glBegin(GL_QUADS);
	{
		M3DVector3f vPoints[] = { { -fQuadwidth / 2.0f, fQuadheight / 2, 0.0f },
		{ 0.0f, 0.0f, fQuaddepth },
		{ fQuadwidth / 2.0f, fQuadheight / 2, fQuaddepth },
		{ 0.0f, fQuadheight, fQuaddepth } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}
	glEnd();
}

void BlackBrownQuadDesign(GLfloat fQuadwidth, GLfloat fQuadheight, GLfloat fQuaddepth)
{
	//code
	M3DVector3f vNormal;

	glPushMatrix();

	glTranslatef(-fQuadwidth, 0.0f, 0.1f);
	glBegin(GL_QUADS);
	glColor3f(0.517f, 0.223f, 0.164f);
	{
		M3DVector3f vPoints[] = { { 0.0f, fQuadheight / 2, 0.0f },
		{ fQuadwidth / 2, 0.0f, fQuaddepth },
		{ fQuadwidth, fQuadheight / 2, fQuaddepth },
		{ fQuadwidth / 2, fQuadheight, fQuaddepth } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}
	glEnd();

	glTranslatef(fQuadwidth, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	glColor3f(0.925f, 0.560f, 0.333f);
	{
		M3DVector3f vPoints[] = { { 0.0f, fQuadheight / 2, 0.0f },
		{ fQuadwidth / 2, 0.0f, fQuaddepth },
		{ fQuadwidth, fQuadheight / 2, fQuaddepth },
		{ fQuadwidth / 2, fQuadheight, fQuaddepth } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}
	glEnd();

	glPopMatrix();
}

void BaseQuad(GLfloat width, GLfloat height, GLfloat depth)
{
	M3DVector3f vNormal;

	glBegin(GL_QUADS);//front face
	{
		M3DVector3f vPoints[] = { { -width / 2.0f, 0.0f, depth / 2.0f },
		{ width / 2.0f, 0.0f, depth / 2.0f },
		{ width / 2.0f, height, depth / 2.0f },
		{ -width / 2.0f, height, depth / 2.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}
	glEnd();

}

////////-----------------------/////////////////////////////////

void balcony(GLfloat width, GLfloat height, GLfloat depth, GLint noOfPillers, GLfloat distBetweenPillers, GLfloat PillerHeight, GLfloat middleHalfDistance, GLint j)
{
	void DrawPiller(GLfloat, GLfloat, GLfloat);
	///---------------------------Vertical Pillers
	glPushMatrix();

	glTranslatef(j * middleHalfDistance, 0.0f, 0.0f);

	for (GLfloat i = 0.0f; i < (GLfloat)noOfPillers * distBetweenPillers; i += distBetweenPillers)
	{
		glPushMatrix();
		glTranslatef(j * i, 0.0, 0.0f);
		//glColor3f(0.423f, 0.058f, 0.058f);
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		DrawPiller(width, PillerHeight, depth);

		glTranslatef(0.0f, -25.0f, 0.0f);						//lower pillers
		DrawPiller(width, 34.0f, depth);
		glPopMatrix();
	}


	// ---------------------------------------------------
	//------------------------Horizontal Bars
	for (GLfloat i = 0.0f; i < (noOfPillers - 1) * distBetweenPillers; i += distBetweenPillers)
	{
		glPushMatrix();

		glTranslatef(j * i, 0.0f, 0.0f);

		glPushMatrix();
		////////////-------------------------------Horizontal rows
		glTranslatef(j * distBetweenPillers / 2.0f, -PillerHeight / 2.0f + (height / 2.0f), 0.0f);
		//glColor3f(0.423f, 0.058f, 0.058f);
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		DrawPiller(distBetweenPillers, height, depth);
		glTranslatef(0.0f, 2.0f * PillerHeight / 8.0f, 0.0f);
		//glColor3f(0.423f, 0.058f, 0.058f);
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		DrawPiller(distBetweenPillers, height, depth);
		glTranslatef(0.0f, 1.5f * PillerHeight / 8.0f, 0.0f);
		//glColor3f(0.423f, 0.058f, 0.058f);
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		DrawPiller(distBetweenPillers, height, depth);
		glTranslatef(0.0f, PillerHeight - (height)-(3.5f * PillerHeight / 8.0f), 0.0f);
		//glColor3f(0.423f, 0.058f, 0.058f);
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		DrawPiller(distBetweenPillers, height, depth);

		glPopMatrix();

		glPushMatrix();
		///////////////////-------------------------left inside column
		glTranslatef(j * ((width - 1.0f) + 0.3f), -(((4.5f * PillerHeight / 8.0f) / 2.0f) + height * 2) + 2.2f, 0.0f);
		//glColor3f(0.423f, 0.058f, 0.058f);
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		DrawPiller(width - 1.0f, (4.0f * PillerHeight / 8.0f) + height, depth);

		glPopMatrix();

		glPushMatrix();
		/////////////////--------------------------right inside column
		glTranslatef(j * ((distBetweenPillers - width + 1.0f) - 0.3f), -(((4.5f * PillerHeight / 8.0f) / 2.0f) + height * 2) + 2.2f, 0.0f);
		//glColor3f(0.423f, 0.058f, 0.058f);
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		DrawPiller(width - 1.0f, (4.0f * PillerHeight / 8.0f) + height, depth);

		glPopMatrix();

		glPopMatrix();
	}

	glPopMatrix();
	//////////////////////////////------------------Half Horizontal bars
	glPushMatrix();

	glPushMatrix();
	////////////-------------------------------Horizontal rows
	glTranslatef(j * middleHalfDistance / 2.0f, -PillerHeight / 2.0f + (height / 2.0f), 0.0f);
	//glColor3f(0.423f, 0.058f, 0.058f);
	glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
	DrawPiller(middleHalfDistance, height, depth);
	glTranslatef(0.0f, 2.0f * PillerHeight / 8.0f, 0.0f);
	//glColor3f(0.423f, 0.058f, 0.058f);
	glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
	DrawPiller(middleHalfDistance, height, depth);
	glTranslatef(0.0f, 1.5f * PillerHeight / 8.0f, 0.0f);
	//glColor3f(0.423f, 0.058f, 0.058f);
	glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
	DrawPiller(middleHalfDistance, height, depth);
	glTranslatef(0.0f, PillerHeight - (height)-(3.5f * PillerHeight / 8.0f), 0.0f);
	//glColor3f(0.423f, 0.058f, 0.058f);
	glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
	DrawPiller(middleHalfDistance, height, depth);

	glPopMatrix();

	glPushMatrix();

	if (j == 1.0f)
	{
		glTranslatef(j * (middleHalfDistance - width + 1.0f), -(((4.5f * PillerHeight / 8.0f) / 2.0f) + height * 2) + 2.2f, 0.0f);
		//glColor3f(0.423f, 0.058f, 0.058f);
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		DrawPiller(width - 1.0f, (4.0f * PillerHeight / 8.0f) + height, depth);
	}
	else if (j == -1.0f)
	{
		glTranslatef(j * (middleHalfDistance - width + 1.0f), -(((4.5f * PillerHeight / 8.0f) / 2.0f) + height * 2) + 2.2f, 0.0f);
		//glColor3f(0.423f, 0.058f, 0.058f);
		glColor3f(0.52f, 0.37f, 0.26f);	//dark wood
		DrawPiller(width - 1.0f, (4.0f * PillerHeight / 8.0f) + height, depth);
	}
	glPopMatrix();

	glPopMatrix();

	glPushMatrix();
	//////////////-----------------Top Pillers
	glTranslatef(0.0f, (PillerHeight / 2.0f) + 9.0f, 14.0f);
	glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);
	for (GLfloat j = -((noOfPillers - 1) * distBetweenPillers + middleHalfDistance); j <= ((noOfPillers - 1) * distBetweenPillers + middleHalfDistance); j += 4.0f)
	{
		glPushMatrix();
		glTranslatef(j, 0.0f, 0.0f);
		glColor3f(0.858f, 0.760f, 0.760f);
		DrawPiller(width, height, 40.0f);
		glPopMatrix();
	}

	glPopMatrix();
}



void DrawPiller(GLfloat width, GLfloat height, GLfloat depth)
{
	M3DVector3f vNormal;


	glBegin(GL_QUADS);//front face
	{
		M3DVector3f vPoints[] = { { -width / 2.0f,-height / 2.0f, depth / 2.0f },
		{ width / 2.0f, -height / 2.0f, depth / 2.0f },
		{ width / 2.0f, height / 2.0f, depth / 2.0f },
		{ -width / 2.0f, height / 2.0f, depth / 2.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}


	{
		M3DVector3f vPoints[] = { { width / 2.0f, -height / 2.0f, depth / 2.0f },
		{ width / 2.0f, -height / 2.0f, -depth / 2.0f },
		{ width / 2.0f, height / 2.0f, -depth / 2.0f },
		{ width / 2.0f, height / 2.0f, depth / 2.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{
		M3DVector3f vPoints[] = { { width / 2.0f, -height / 2.0f, -depth / 2.0f },
		{ -width / 2.0f, -height / 2.0f, -depth / 2.0f },
		{ -width / 2.0f, height / 2.0f, -depth / 2.0f },
		{ width / 2.0f, height / 2.0f, -depth / 2.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{
		M3DVector3f vPoints[] = { { -width / 2.0f, -height / 2.0f, -depth / 2.0f },
		{ -width / 2.0f, -height / 2.0f, depth / 2.0f },
		{ -width / 2.0f, height / 2.0f, depth / 2.0f },
		{ -width / 2.0f, height / 2.0f, -depth / 2.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{
		M3DVector3f vPoints[] = { { -width / 2.0f, height / 2.0f, depth / 2.0f },
		{ width / 2.0f, height / 2.0f, depth / 2.0f },
		{ width / 2.0f, height / 2.0f, -depth / 2.0f },
		{ -width / 2.0f, height / 2.0f, -depth / 2.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}

	{
		M3DVector3f vPoints[] = { { -width / 2.0f, -height / 2.0f, -depth / 2.0f },
		{ width / 2.0f, -height / 2.0f, -depth / 2.0f },
		{ width / 2.0f, -height / 2.0f, depth / 2.0f },
		{ -width / 2.0f, -height / 2.0f, depth / 2.0f } };

		m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
		glVertex3fv(vPoints[3]);
	}
	glEnd();
}


//////////////////----------------------////////////////////////////////


void DrawStairs(void)
{
	/*Prabhakar-Start*/
	glPushMatrix();
	glCallList(dsStairs);
	glPopMatrix();

	// left stairs side beams
	glPushMatrix();
	glTranslatef(-49.5f, 18.5f, -101.5f);
	glRotatef(1.30f, 1.0f, 0.0f, 0.0f);
	glRotatef(358.99f, 0.0f, 1.0f, 0.0f);
	glRotatef(319.69f, 0.0f, 0.0f, 1.0f);
	glScalef(22.90f, 1.0f, 0.80f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-49.5f, 18.5f, -110.0f);
	glRotatef(1.30f, 1.0f, 0.0f, 0.0f);
	glRotatef(358.99f, 0.0f, 1.0f, 0.0f);
	glRotatef(319.69f, 0.0f, 0.0f, 1.0f);
	glScalef(22.90f, 1.0f, 0.80f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	// right stairs side beams
	glPushMatrix();
	glTranslatef(49.5f, 18.5f, -101.5f);
	glRotatef(1.30f, 1.0f, 0.0f, 0.0f);
	glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(41.0f, 0.0f, 0.0f, 1.0f);
	glScalef(22.90f, 1.0f, 0.80f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(49.5f, 18.5f, -110.0f);
	glRotatef(1.30f, 1.0f, 0.0f, 0.0f);
	glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(41.0f, 0.0f, 0.0f, 1.0f);
	glScalef(22.90f, 1.0f, 0.80f);
	glCallList(dsCubeRegular);
	glPopMatrix();
	/*Prabhakar-End*/

	//right stair upper right vertical bar
	glPushMatrix();
	glTranslatef(67.1f, 39.0f, -101.25f);
	glScalef(0.30f, 4.89f, 0.40f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//right stair upper left vertical bar
	glPushMatrix();
	glTranslatef(67.1f, 39.0f, -110.75f);
	glScalef(0.30f, 4.89f, 0.40f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//left stair upper left vertical bar
	glPushMatrix();
	glTranslatef(-67.1f, 39.0f, -101.25f);
	glScalef(0.30f, 4.89f, 0.40f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//left stair upper left vertical bar
	glPushMatrix();
	glTranslatef(-67.1f, 39.0f, -110.75f);
	glScalef(0.30f, 4.89f, 0.40f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//right stair lower right vertical bar
	glPushMatrix();
	glTranslatef(31.5f, 9.0f, -101.25f);
	glScalef(0.30f, 5.0f, 0.40f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//right stair lower left vertical bar
	glPushMatrix();
	glTranslatef(31.5f, 9.0f, -110.75f);
	glScalef(0.30f, 5.0f, 0.40f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//left stair lower left vertical bar
	glPushMatrix();
	glTranslatef(-31.5f, 9.0f, -101.25f);
	glScalef(0.30f, 5.0f, 0.40f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//left stair lower left vertical bar
	glPushMatrix();
	glTranslatef(-31.5f, 9.0f, -110.75f);
	glScalef(0.30f, 5.0f, 0.40f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();										//StairDoor
	{
		glTranslatef(-40.5f, 14.0f, -90.25f);
		glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
		glScalef(0.865f, 0.90f, 1.0f);
		drawDoorStairs();
	}
	glPopMatrix();

	glPushMatrix();										//StairDoor
	{
		glTranslatef(40.5f, 14.0f, -90.25f);
		glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
		glScalef(0.865f, 0.90f, 1.0f);
		drawDoorStairs();
	}
	glPopMatrix();
}

void QuarterCircularStructure(void)
{
	GLfloat gfAngle = 0.0f;
	// Circular Structure
	glColor3f(0.9f, 0.9f, 0.9f);
	for (gfAngle = 0.0f; gfAngle < 0.5 * M_PI; gfAngle = gfAngle + 0.15f)
	{
		glPushMatrix();
		glTranslatef(20.0f * (GLfloat)cos(gfAngle), 8.0f, 20.0f * (GLfloat)sin(gfAngle));
		glScalef(1.0f, 1.0f, 1.0f);
		glCallList(dsBalconyBaluster);
		glPopMatrix();
	}

	glColor3f(0.52f, 0.37f, 0.26f);
	for (gfAngle = 0.0f; gfAngle < 0.5 * M_PI; gfAngle = gfAngle + 0.01f)
	{
		glPushMatrix();
		glTranslatef(20.0f * (GLfloat)cos(gfAngle), 1.5f, 20.0f * (GLfloat)sin(gfAngle));
		glScalef(0.2f, 0.2f, 0.2f);
		glCallList(dsCube);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(20.0f * (GLfloat)cos(gfAngle), 15.5f, 20.0f * (GLfloat)sin(gfAngle));
		glScalef(0.2f, 0.2f, 0.2f);
		glCallList(dsCube);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(20.0f * (GLfloat)cos(gfAngle), 25.5f, 20.0f * (GLfloat)sin(gfAngle));
		glScalef(0.2f, 0.2f, 0.2f);
		glCallList(dsCube);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(20.0f * (GLfloat)cos(gfAngle), 60.5f, 20.0f * (GLfloat)sin(gfAngle));
		glScalef(0.2f, 0.2f, 0.2f);
		glCallList(dsCube);
		glPopMatrix();
	}

	glColor3f(0.9f, 0.9f, 0.9f);
	for (gfAngle = 0.0f; gfAngle < 0.5 * M_PI; gfAngle = gfAngle + 0.1f)
	{
		glPushMatrix();
		glTranslatef(20.0f * (GLfloat)cos(gfAngle), 60.0f, 20.0f * (GLfloat)sin(gfAngle));
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glScalef(1.0f, 1.0f, 45.0f);
		glCallList(dsStairSectionCylinder);
		glPopMatrix();
	}

	glColor3f(0.52f, 0.37f, 0.26f);
	glPushMatrix();
	glTranslatef(-2.0f, 31.0f, 20.0f);
	glScalef(0.2f, 6.1f, 0.2f);
	glCallList(dsCube);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(20.0f, 31.0f, -2.0f);
	glScalef(0.2f, 6.1f, 0.2f);
	glCallList(dsCube);
	glPopMatrix();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////CHANGE THIS FOR TRAPEZOIDAL SHAPE.
void drawPositionalTrapezoid(void)
{
	void drawTrapezoid(void);
	glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();//right upper
	glTranslatef(54.5f, 65.0f, 10.0f);
	glScalef(11.0f, 1.0f, 1.0f);
	drawTrapezoid();
	glPopMatrix();

	glPushMatrix();//left upper
	glTranslatef(-152.0f, 65.0f, 9.5f);
	glScalef(10.8f, 1.0f, 1.0f);
	drawTrapezoid();
	glPopMatrix();

	glPushMatrix();//left lower
	glTranslatef(-152.0f, 28.0f, 9.5f);
	glScalef(10.6f, 1.0f, 1.0f);
	drawTrapezoid();
	glPopMatrix();

	glPushMatrix();//right lower
	glTranslatef(58.0f, 28.0f, 9.5f);
	glScalef(10.6f, 1.0f, 1.0f);
	drawTrapezoid();
	glPopMatrix();

	glPushMatrix();//right upper sideways
	glTranslatef(77.0f, 65.0f, -28.0f);
	glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
	glScalef(5.0f, 1.0f, 1.0f);
	drawTrapezoid();
	glPopMatrix();

	glPushMatrix();//left upper sideways
	glTranslatef(-77.0f, 65.0f, 18.0f);
	glRotatef(-270.0f, 0.0f, 1.0f, 0.0f);
	glScalef(5.0f, 1.0f, 1.0f);
	drawTrapezoid();
	glPopMatrix();


}

void drawTrapezoid(void)
{
	glColor3f(0.81f, 0.71f, 0.23f);

	glBegin(GL_QUADS);
	//upper face
	//	glColor3f(1.0f, 0.0f, 0.0f);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(2.0f, 5.0f, 0.5f);
	glVertex3f(7.0f, 5.0f, 0.5f);
	glVertex3f(7.0f, 5.0f, -4.0f);
	glVertex3f(2.0f, 5.0f, -4.0f);

	//front face
	//glColor3f(0.0f, 1.0f, 0.0f);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(2.0f, 5.0f, 0.5f);
	glVertex3f(7.0f, 5.0f, 0.5f);
	glVertex3f(6.8f, 1.0f, -2.0f);
	glVertex3f(2.2f, 1.0f, -2.0f);

	//back face
	//glColor3f(0.0f, 0.0f, 1.0f);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(2.0f, 5.0f, -4.0f);
	glVertex3f(2.2f, 1.0f, -4.0f);
	glVertex3f(6.8f, 1.0f, -4.0f);
	glVertex3f(7.0f, 5.0f, -4.0f);


	//right side face

	//glColor3f(1.0f, 0.0f, 0.0f);
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(7.0f, 5.0f, 0.5f);
	glVertex3f(6.8f, 1.0f, -2.0f);
	glVertex3f(6.8f, 1.0f, -4.0f);
	glVertex3f(7.0f, 5.0f, -4.0f);

	//left side face

	//glColor3f(1.0f, 0.0f, 0.0f);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(2.0f, 5.0f, 0.5f);
	glVertex3f(2.0f, 5.0f, -4.0f);
	glVertex3f(2.2f, 1.0f, -4.0f);
	glVertex3f(2.2f, 1.0f, -2.0f);


	//bottom face

	//glColor3f(0.0f, 0.0f, 1.0f);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(6.8f, 1.0f, -2.0f);
	glVertex3f(6.8f, 1.0f, -4.0f);
	glVertex3f(2.2f, 1.0f, -4.0f);
	glVertex3f(2.2f, 1.0f, -2.0f);

	glEnd();

}

//// Sujay Ingale
void drawBalconyDoors(void)
{
	void drawGURUKUL(void);

	glPushMatrix();															//One Side
	{
		glTranslatef(-102.0f, 44.5f, -87.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawDoorStairs();
	}
	glPopMatrix();

	glPushMatrix();															//One Side
	{
		glTranslatef(102.0f, 44.5f, -87.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawDoorStairs();
	}
	glPopMatrix();

	glPushMatrix();															//Library Doors
	{
		glTranslatef(-97.5f, 46.0f, -111.0f);
		glScalef(0.9f, 1.1f, 1.0f);

		drawDoorLibrary();
	}
	glPopMatrix();

	glPushMatrix();															//Library Doors
	{
		glTranslatef(97.0f, 46.0f, -111.0f);
		glScalef(0.9f, 1.1f, 1.0f);

		drawDoorLibrary();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-80.0f, 14.0f, -1.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawDoorOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(80.0f, 14.0f, -1.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawDoorOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-80.0f, 80.0f, -1.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawWindowOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(80.0f, 80.0f, -1.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawWindowOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(65.0f, 44.0f, -13.5f);

		drawDoor();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-65.0f, 44.0f, -13.5f);

		drawDoor();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-18.0f, 41.0f, -110.0f);
		glScalef(0.8f, 0.9f, 1.0f);

		drawBench();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-33.5f, 38.0f, -105.5f);
		glScalef(2.2f, 3.2f, 5.4f);
		glColor3f(0.267059f, 0.154706f, 0.104706f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(33.5f, 38.0f, -105.5f);
		glScalef(2.2f, 3.2f, 5.4f);
		glColor3f(0.267059f, 0.154706f, 0.104706f);
		glCallList(dsCubeRegular);
	}
	glPopMatrix();

	glPushMatrix();										//GURUKUL
	{
		glDisable(GL_LIGHTING);
		glTranslatef(0.5f, 49.0f, -20.0f);
		glScalef(2.0f, 2.0f, 1.0f);
		drawGURUKUL();
		glEnable(GL_LIGHTING);
	}
	glPopMatrix();

	//new
	glPushMatrix();
	{
		glTranslatef(-1.0f, 21.5f, -274.0f);
		glScalef(1.55f, 1.55f, 1.0f);

		drawDoorOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(72.5f, 21.5f, -283.0f);
		glScalef(1.55f, 1.55f, 1.0f);

		drawDoorOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-74.5f, 21.5f, -283.0f);
		glScalef(1.55f, 1.55f, 1.0f);

		drawDoorOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-75.0f, 82.5f, -283.0f);
		glScalef(1.55f, 1.55f, 1.0f);

		drawWindowOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(73.0f, 82.5f, -283.0f);
		glScalef(1.55f, 1.55f, 1.0f);

		drawWindowOutside();
	}
	glPopMatrix();

	//new new
	glPushMatrix();
	{
		glTranslatef(-98.0f, 26.0f, -138.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawWindowOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-98.0f, 26.0f, -156.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawWindowOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-98.0f, 26.0f, -174.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawWindowOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(98.0f, 26.0f, -138.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawWindowOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(98.0f, 26.0f, -156.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawWindowOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(98.0f, 26.0f, -174.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		drawWindowOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-98.0f, 9.0f, -122.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		glScalef(0.9f, 1.0f, 1.0f);

		drawLibraryMainDoor();

		glTranslatef(-0.6f, 0.0f, 0.0f);
		glColor3f(0.1f, 0.1f, 0.1f);
		drawCube(32.0f, 18.0f, 0.1f);

	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(98.0f, 9.0f, -122.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		glScalef(0.9f, 1.0f, 1.0f);

		drawLibraryMainDoor();

		glTranslatef(0.6f, 0.0f, 0.0f);
		glColor3f(0.1f, 0.1f, 0.1f);
		drawCube(32.0f, 18.0f, 0.1f);

	}
	glPopMatrix();

	//refine
	glPushMatrix();
	{
		glTranslatef(79.5f, 14.0f, -113.7f);
		drawDoorOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-81.0f, 14.0f, -113.7f);
		drawDoorOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-57.0f, 20.5f, -113.0f);
		glScalef(0.8f, 0.8f, 1.0f);
		drawWindowOutside();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(56.5f, 20.5f, -113.0f);
		glScalef(0.8f, 0.8f, 1.0f);
		drawWindowOutside();
	}
	glPopMatrix();

	glPopMatrix();
}

///////////////////////////////////////////////////////////////////////
// Roof Shingle by Bezier Curve method
void DrawBezierCurve(void)
{
	void ShingleUpperPart(void);
	void ShingleLeftPart(void);
	void ShingleFrontPart(void);

	glColor3f(1.000f, 0.271f, 0.000f);

	glPushMatrix();
	glTranslatef(0.0f, 2.0f, 0.0f);
	ShingleUpperPart();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 4.0f);
	ShingleFrontPart();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-4.0f, 0.0f, 0.0f);
	ShingleLeftPart();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(4.0f, 0.0f, 0.0f);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	ShingleLeftPart();
	glPopMatrix();

	glPushMatrix();
	ShingleUpperPart();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -4.0f);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	ShingleFrontPart();
	glPopMatrix();
}

void DrawRoofShingle(void)
{

	void ShingleUpperPart(void);
	void ShingleLeftPart(void);
	void ShingleFrontPart(void);

	glColor3f(1.000f, 0.271f, 0.000f);
	glPushMatrix();
	//glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
	//glRotatef(60.0f, 1.0f, 0.0f, 0.0f);

	//glTranslatef(0.0f, 5.0f, 0.0f);

	glPushMatrix();
	glTranslatef(0.0f, 2.0f, 0.0f);
	ShingleUpperPart();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 4.0f);
	ShingleFrontPart();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-4.0f, 0.0f, 0.0f);
	ShingleLeftPart();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(4.0f, 0.0f, 0.0f);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	ShingleLeftPart();
	glPopMatrix();

	/*glPushMatrix();
	ShingleUpperPart();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -4.0f);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	ShingleFrontPart();
	glPopMatrix();
	*/



	/*glPointSize(5.0f);
	glBegin(GL_POINTS);
	for (GLint i = 0; i < nNumPoints; i++)
	for(GLint j = 0; j < 3; j++)
	glVertex3fv(ctrlPoints1[i][j]);
	glEnd();*/

	glPopMatrix();
}

void ShingleUpperPart(void)
{
	glPushMatrix();
	//glColor3f(0.823, 0.317, 0.345);
	glColor3f(1.000f, 0.271f, 0.000f);
	glMap2f(GL_MAP2_VERTEX_3, 0.0f, 10.0f, 3, RoofPoints, 0.0f, 10.0f, 9, RoofPoints, &RoofControlPoints[0][0][0]);
	glEnable(GL_MAP2_VERTEX_3);

	//glMapGrid2f(10, 0.0f, 10.0f, 10, 0.0f, 10.0f);
	glMapGrid2f(2, 0.0f, 10.0f, 2, 0.0f, 10.0f);

	//glEvalMesh2(GL_FILL, 0, 10, 0, 10);
	glEvalMesh2(GL_FILL, 0, 2, 0, 2);
	glEnable(GL_AUTO_NORMAL);


	glPopMatrix();
}

void ShingleLeftPart(void)
{
	glPushMatrix();
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	//glColor3f(0.823, 0.317, 0.345);

	glVertex3f(0.0f, 0.0f, 4.0f);
	glVertex3f(0.0f, 2.0f, 4.0f);
	glVertex3f(0.0f, 2.0f, -4.0f);
	glVertex3f(0.0f, 0.0f, -4.0f);
	glEnd();
	glPopMatrix();
}

void ShingleFrontPart(void)
{
	glPushMatrix();
	//glColor3f(0.823, 0.317, 0.345);
	glColor3f(0.545f, 0.000f, 0.000f);
	glMap2f(GL_MAP2_VERTEX_3, 0.0f, 10.0f, 3, RoofPoints, 0.0f, 10.0f, 9, RoofPoints, &RoofControlPoints2[0][0][0]);
	glEnable(GL_MAP2_VERTEX_3);

	glMapGrid2f(10, 0.0f, 10.0f, 10, 0.0f, 10.0f);

	glEvalMesh2(GL_FILL, 0, 10, 0, 10);

	glPopMatrix();
}

//////////////////////////////////////////////////////////
// Actual roof
void DrawRoof(void)
{
	//glColor3f(0.52f, 0.37f, 0.26f);	// dark wood
	//glColor3f(0.36f, 0.25f, 0.20f);	// dark brown
	glColor3f(0.35f, 0.16f, 0.14f);	// very dark brown
	for (GLfloat j = 0.0f, k = 0.0f; j < 2.5f && k < 20.0f; j = j + 0.3f, k = k + 3.5f)
	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -k);
		glTranslatef(0.0f, j, 0.0f);
		for (GLfloat i = 0.0f; i < 160.0f; i = i + 7.5f)
		{
			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(1.5f, 2.0f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
}

void DrawUpperFrontRoof(void)
{
	for (GLfloat j = 0.0f, k = 0.0f; j < 2.5f && k < 45.0f; j = j + 0.2f, k = k + 4.5f)
	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -k);
		glTranslatef(0.0f, j, 0.0f);
		for (GLfloat i = 0.0f; i < 160.0f; i = i + 12.5f)
		{
			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(2.5f, 2.5f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
}

void DrawUpperBackRoof(void)
{
	for (GLfloat j = 0.0f, k = 0.0f; j < 2.5f && k < 45.0f; j = j + 0.2f, k = k + 4.5f)
	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -k);
		glTranslatef(0.0f, j, 0.0f);
		for (GLfloat i = 0.0f; i < 85.0f; i = i + 7.5f)
		{
			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(1.5f, 2.5f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
}

void DrawLeftBuildingLeftRoof(void)
{

}

void DrawLeftBuildingRightRoof(void)
{
	for (GLfloat j = 0.0f, k = 0.0f; j < 2.5f && k < 45.0f; j = j + 0.2f, k = k + 4.5f)
	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -k);
		glTranslatef(0.0f, j, 0.0f);
		for (GLfloat i = 0.0f; i < 90.0f; i = i + 12.5f)
		{
			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(2.5f, 2.5f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
}

void DrawRightBuildingLeftRoof(void)
{
	for (GLfloat j = 0.0f, k = 0.0f; j < 2.5f && k < 45.0f; j = j + 0.2f, k = k + 4.5f)
	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -k);
		glTranslatef(0.0f, j, 0.0f);
		for (GLfloat i = 0.0f; i < 90.0f; i = i + 12.5f)
		{
			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(2.5f, 2.5f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
}

void DrawRightBuildingRightRoof(void)
{

}

void DrawRightBuildingFrontRoof(void)
{
	for (GLfloat j = 0.0f, k = 0.0f; j < 2.5f && k < 20.0f; j = j + 0.2f, k = k + 4.5f)
	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -k);
		glTranslatef(0.0f, j, 0.0f);
		for (GLfloat i = 0.0f; i < 60.0f; i = i + 12.5f)
		{
			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(2.5f, 2.5f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
}

void DrawFrontBalconyRoof(void)
{

}

void DrawPyramidalRoof(void)
{
	//for (GLfloat j = 0.0f, k = 0.0f; j < 2.5f && k < 50.0f; j = j + 0.2f, k = k + 7.5f)
	//{
	//	glPushMatrix();
	//	glTranslatef(0.0f, 0.0f, -k);
	//	glTranslatef(0.0f, j, 0.0f);
	//	for (GLfloat i = 0.0f; i < k; i = i + 4.0f)
	//	{
	//		glPushMatrix();
	//		{
	//			glTranslatef(i, 0.0f, 0.0f);
	//			glScalef(0.5f, 0.3f, 1.0f);
	//			glCallList(dsRoofShingle);
	//		}
	//		glPopMatrix();
	//	}
	//	glPopMatrix();
	//}

		for (GLfloat i = 0.0f; i < 45; i = i + 4.0f)
		{
			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(2.5f, 2.5f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}

		
		glTranslatef(5.0f, 0.2f, -6.0f);
		for (GLfloat i = 0.0f; i < 30; i = i + 4.0f)
		{	
			
			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(2.5f, 2.5f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}

		glTranslatef(5.0f, 0.2f, -6.0f);
		for (GLfloat i = 0.0f; i < 20; i = i + 4.0f)
		{

			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(2.5f, 2.5f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}
		
		glTranslatef(5.0f, 0.2f, -6.0f);
		for (GLfloat i = 0.0f; i < 10; i = i + 4.0f)
		{

			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(2.5f, 2.5f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}

		glTranslatef(5.0f, 0.2f, -6.0f);
		for (GLfloat i = 0.0f; i < 3; i = i + 4.0f)
		{

			glPushMatrix();
			{
				glTranslatef(i, 0.0f, 0.0f);
				glScalef(2.5f, 2.5f, 2.5f);
				glCallList(dsRoofShingle);
			}
			glPopMatrix();
		}
}

////////----
void drawBench(void)
{
	glPushMatrix();
	{
		glColor3f(0.7f, 0.3f, 0.2f);
		glPushMatrix();										//legs
		{
			glTranslatef(10.0f, -5.0f, 5.0f);
			glRotatef(-7.0f, 1.0f, 0.0f, 0.0f);
			drawCube(7.0f, 0.75f, 0.75f);
		}
		glPopMatrix();

		glPushMatrix();										//legs
		{
			glTranslatef(-10.0f, -5.0f, 5.0f);
			glRotatef(-7.0f, 1.0f, 0.0f, 0.0f);
			drawCube(7.0f, 0.75f, 0.75f);
		}
		glPopMatrix();

		glPushMatrix();										//legs
		{
			glTranslatef(10.0f, -5.0f, 0.0f);

			drawCube(7.0f, 0.75f, 0.75f);
		}
		glPopMatrix();

		glPushMatrix();										//legs
		{
			glTranslatef(-10.0f, -5.0f, 0.0f);

			drawCube(7.0f, 0.75f, 0.75f);
		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.0f, -3.2f, 4.8f);
			glRotatef(-7.0f, 1.0f, 0.0f, 0.0f);
			drawWindowBorder(1.5f, 9.8f, 0.35f, 0.5f);

			drawCube(2.8f, 0.5f, 0.5f);

			glPushMatrix();
			{
				glTranslatef(3.0f, 0.0f, 0.0);
				drawCube(2.8f, 0.5f, 0.5f);

			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(-3.0f, 0.0f, 0.0);
				drawCube(2.8f, 0.5f, 0.5f);

			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(-6.0f, 0.0f, 0.0);
				drawCube(2.8f, 0.5f, 0.5f);

			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(6.0f, 0.0f, 0.0);
				drawCube(2.8f, 0.5f, 0.5f);

			}
			glPopMatrix();
		}
		glPopMatrix();

		glPushMatrix();						//seat
		{
			glTranslatef(0.0f, -1.8f, 2.5f);
			drawCube(0.5f, 23.0f, 7.0f);
		}
		glPopMatrix();

		glPushMatrix();							//hands
		{
			glTranslatef(10.0f, 0.0f, 4.8f);
			drawCube(3.0f, 0.6f, 0.6f);

			glTranslatef(0.0f, 1.7f, -2.0f);
			drawCube(0.7f, 0.7f, 7.0f);
		}
		glPopMatrix();

		glPushMatrix();							//hands
		{
			glTranslatef(-10.0f, 0.0f, 4.8f);
			drawCube(3.0f, 0.6f, 0.6f);

			glTranslatef(0.0f, 1.7f, -2.0f);
			drawCube(0.7f, 0.7f, 7.0f);
		}
		glPopMatrix();

		glPushMatrix();							//back rest
		{
			glTranslatef(0.0f, 2.0f, 0.0f);
			glRotatef(-5.0f, 1.0f, 0.0f, 0.0f);
			drawWindowBorder(4.0f, 10.0f, 0.5f, 0.5f);

			glPushMatrix();
			{
				glTranslatef(0.0f, 0.75f, 0.0f);
				drawWindowBorder(2.8f, 9.7f, 0.5f, 0.5f);

				glPushMatrix();
				{
					glTranslatef(8.5f, 0.0f, 0.0f);
					drawCube(5.0f, 0.5f, 0.5f);

					for (int i = 0; i < 15; i++)
					{
						glTranslatef(-1.2f, 0.0f, 0.0f);
						drawCube(5.0f, 0.5f, 0.5f);
					}
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
	glPopMatrix();
}

///////------GURUKUL letters-------//////

void Draw_Spiral(float radius_spiral, float b_spiral, float no_of_rot, float angle_begin, float angle_end)
{

	glBegin(GL_LINES);
	for (float angle = -angle_begin; angle <= no_of_rot; angle = angle + 0.01f)
	{
		glVertex2f(radius_spiral * exp(b_spiral * angle) * cos(angle), radius_spiral * exp(b_spiral * angle) * sin(angle));
		glVertex2f(radius_spiral * exp(b_spiral * (angle + 0.01f)) * cos(angle + 0.01f), radius_spiral * exp(b_spiral * (angle + 0.01f)) * sin(angle + 0.01f));
	}

	glEnd();



}

void Draw_Ka(float scale_fac)
{
	glBegin(GL_LINES);
	for (float angle = 0.0f; angle <= angle_end_Ku2; angle = angle + 0.01f)
	{
		glVertex3f(cos(angle) / scale_fac, sin(angle) / scale_fac, 0.0f);
		glVertex3f(cos(angle + 0.01f) / scale_fac, sin(angle + 0.01f) / scale_fac, 0.0f);
	}

	glEnd();

	glTranslatef(0.56f, 0.0f, 0.0f);

	glBegin(GL_LINES);
	for (float angle = (GLfloat)M_PI; angle >= (GLfloat)M_PI - angle_end_Ku3; angle = angle - 0.01f)
	{
		glVertex3f(cos(angle) / scale_fac, sin(angle) / scale_fac, 0.0f);
		glVertex3f(cos(angle + 0.01f) / scale_fac, sin(angle + 0.01f) / scale_fac, 0.0f);
	}

	glEnd();

}

void Draw_Ra(float b, float height)
{
	//3ay2 = x(x -a)2

	float y1, y2, x, x1;

	glBegin(GL_LINES);
	for (x = 0.0f; x < 2.0f; x = x + 0.01f)
	{
		y1 = (x - b) * sqrt(x / (3.0f * b));
		if (y1 >= height)
		{
			continue;
		}
		x1 = x + 0.01f;
		y2 = (x1 - b) * sqrt(x1 / (3.0f * b));
		//glVertex3f(x,y1,0);
		//glVertex3f(x1, y2, 0);
	}

	glEnd();
	glBegin(GL_LINES);
	for (x = 0.04f; x < 2.0f; x = x + 0.01f)
	{
		y1 = (x - b) * sqrt(x / (3.0f * b));
		if (y1 >= height)
		{
			continue;
		}
		x1 = x + 0.01f;
		y2 = (x1 - b) * sqrt(x1 / (3.0f * b));
		glVertex3f(x, -y1, 0.0f);
		glVertex3f(x1, -y2, 0.0f);
	}
	glEnd();
}

void Draw_La(float scal_fac_1)
{
	float scaling_factor;
	scaling_factor = scal_fac_1;
	glPushMatrix();
	glTranslatef(0.31f, 0.0f, 0.0f);
	glScalef(0.83f, 1.03f, 1.0f);
	glBegin(GL_LINES);
	for (float angle = (GLfloat)M_PI / 4.0f; angle <angle_end_La3; angle = angle + 0.01f)
	{
		glVertex3f(cos(angle) / scal_fac_1, sin(angle) / scal_fac_1, 0.0f);
		glVertex3f(cos(angle + 0.01f) / scal_fac_1, sin(angle + 0.01f) / scal_fac_1, 0.0f);
	}
	glEnd();

	glPopMatrix();
	glScalef(0.69f, 1.0f, 1.0f);
	glTranslatef(-0.04f, 0.02f, 0.0f);
	if (En_La)
	{
		glBegin(GL_LINES);
		for (float angle = (GLfloat)M_PI; angle > (GLfloat)M_PI - angle_end_La2; angle = angle - 0.01f)
		{
			glVertex3f(angle / scaling_factor, sin(angle) / scaling_factor, 0.0f);
			glVertex3f((angle + 0.01f) / scaling_factor, sin(angle + 0.01f) / scaling_factor, 0.0f);
		}
		glEnd();
	}
	glTranslatef(3.15f / (scaling_factor), 0.0f, 0.0f);

	glBegin(GL_LINES);
	for (float angle = (GLfloat)M_PI / 2.0f; angle >= ((GLfloat)M_PI / 2.0f - angle_end_La1); angle = angle - 0.01f)
	{
		glVertex3f(angle / scaling_factor, sin(angle) / scaling_factor, 0.0f);
		glVertex3f((angle + 0.01f) / scaling_factor, sin(angle + 0.01f) / scaling_factor, 0.0f);
	}
	glEnd();
}

void drawGURUKUL(void)
{
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glPushMatrix();


	/*Upper Horizontal Line*/
	glLineWidth(2.0);
	/*Color for the upper Horizontal Line*/
	glColor3f(1, 1, 1);
	Draw_Line(-2, -2 + vertex_end_HL, 0.5, 0.5);
	/*Upper Horizontal Line*/

	/*Color for Rest of the characters*/
	glColor3f(1, 1, 1);


	/*Line width for rest of the letters*/
	//glLineWidth(4);

	/*========Start Letter Ga================*/
	glTranslatef(-1.25, 0, 0);
	/*Verticle Line for Ga*/

	Draw_Line(-0.02f, -0.02f, 0.5f, 0.5f - vertex_end_Ga);

	glTranslatef(-0.65f, -0.30f, 0.0f);
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
	if (En_Ga1)
	{
		Draw_Spiral(0.78f, -0.62f, angle_end_Ga1, 0.03f, 0.0f);//4
	}

	glPopMatrix();
	glPushMatrix();
	glTranslatef(-1.33f, -0.58f, 0.0f);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	if (En_Ga2)
		Draw_Spiral(0.04f, 0.45f, angle_end_Ga2, 2.22f, 0.0f);//2

	/*========End Letter Ga================*/

	glPopMatrix();
	glPushMatrix();

	/*========Start Letter Ra================*/
	glTranslatef(-0.93f, 0.0f, 0.0f);

	glTranslatef(-0.1f, 0.04f, 0.0f);
	glPushMatrix();
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);

	if (En_Ra1)
		Draw_Spiral(1.09f, -0.65f, angle_end_Ra1, -0.79f, 0.0f);//2
	glPopMatrix();
	glTranslatef(-0.05f, -0.01f, 0.0f);
	if (En_Ra3)
		Draw_Ra(0.14f, vertex_end_Ra); //0.7

	glTranslatef(0.5f, -0.22f, 0.0f);
	if (En_Ra2)
		Draw_Spiral(0.45f, -0.407f, angle_end_Ra2, 0.15f, 0.0f);//3
	/*========End Letter Ra================*/

	glPopMatrix();
	glPushMatrix();

	/*========Start Letter Ku================*/

	glTranslatef(0.595f, 0.0f, 0.0f);

	Draw_Line(0.0f, 0.0f, 0.5f, 0.5f - vertex_end_Ka);

	glTranslatef(-0.05f, -0.69f, 0.0f);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

	if (En_Ku1)
		Draw_Spiral(0.04f, 0.45f, angle_end_Ku1, 2.22f, 0.0f);//2

	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.3f, -0.06f, 0.0f);
	glScalef(1.05f, 1.01f, 1.0f);

	if (En_Ku2)
		Draw_Ka(3.57f);

	/*========End Letter Ku================*/

	glPopMatrix();
	glPushMatrix();

	/*========Start Letter La================*/
	glTranslatef(1.27f, -0.18f, 0.0f);

	glScalef(0.96f, 1.14f, 1.0f);
	Draw_La(3.2f);
	glPopMatrix();
	glPushMatrix();

	glTranslatef(2.24f, 0.0f, 0.0f);
	Draw_Line(-0.02f, -0.02f, 0.5f, 0.5f - vertex_end_La);
	/*========End Letter La================*/

	glPopMatrix();
}

void Draw_Gurukul(void)
{


	{
		if (en_Ga1)
		{
			angle_end_Ga1 = angle_end_Ga1 + 0.10f;

			if (angle_end_Ga1 >= 6.0f)
			{
				angle_end_Ga1 = 6.0f;
				en_Ga1 = 0;
				en_Ga3 = 1;

			}
		}

		if (en_Ga3)
		{
			vertex_end_Ga = vertex_end_Ga + 0.05f;

			if (vertex_end_Ga >= 1.0f)
			{
				vertex_end_Ga = 1.0f;
				en_Ga3 = 0;
				en_Ga2 = 1;
				En_Ga2 = 1;
			}
		}


		if (en_Ga2)
		{
			angle_end_Ga2 = angle_end_Ga2 + 0.10f;
			if (angle_end_Ga2 >= 2.0f * M_PI)
			{
				en_Ga2 = 0;
				en_Ra1 = 1;
				En_Ra1 = 1;
			}
		}

		if (en_Ra1)
		{
			angle_end_Ra1 = angle_end_Ra1 + 0.10f;
			if (angle_end_Ra1 >= 2 * M_PI)
			{
				en_Ra1 = 0;
				en_Ra3 = 1;
				En_Ra3 = 1;
			}
		}

		if (en_Ra3)
		{
			vertex_end_Ra = vertex_end_Ra + 0.05f;

			if (vertex_end_Ra >= 0.7)
			{
				vertex_end_Ra = 0.7f;
				en_Ra2 = 1;
				en_Ra3 = 0;
				En_Ra2 = 1;
			}
		}

		if (en_Ra2)
		{
			angle_end_Ra2 = angle_end_Ra2 + 0.10f;
			if (angle_end_Ra2 >= 6)
			{
				angle_end_Ra2 = 6;
				en_Ra2 = 0;
				en_Ku4 = 1;


			}
		}
		if (en_Ku4)
		{
			vertex_end_Ka = vertex_end_Ka + 0.05f;

			if (vertex_end_Ka >= 1.1)
			{
				vertex_end_Ka = 1.1f;
				en_Ku4 = 0;
				En_Ku2 = 1;
				en_Ku2 = 1;

			}
		}
		if (en_Ku2)
		{
			angle_end_Ku2 = angle_end_Ku2 + 0.10f;
			if (angle_end_Ku2 >= 2.0f * M_PI)
			{
				angle_end_Ku2 = 2.0f * (GLfloat)M_PI;
				en_Ku2 = 0;
				en_Ku3 = 1;

			}
		}
		if (en_Ku3)
		{
			angle_end_Ku3 = angle_end_Ku3 + 0.10f;
			if (angle_end_Ku3 >= (M_PI + M_PI / 4))
			{
				angle_end_Ku3 = (GLfloat)M_PI + (GLfloat)M_PI / 4.0f;
				en_Ku3 = 0;
				en_Ku1 = 1;
				En_Ku1 = 1;
			}
		}



		if (en_Ku1)
		{
			angle_end_Ku1 = angle_end_Ku1 + 0.10f;
			if (angle_end_Ku1 >= 2.0f * M_PI)
			{
				angle_end_Ku1 = 2.0f * (GLfloat)M_PI;
				en_Ku1 = 0;
				en_La4 = 1;
			}
		}
		if (en_La4)
		{

			vertex_end_La = vertex_end_La + 0.05f;
			if (vertex_end_La >= 1.1f)
			{
				vertex_end_La = 1.1f;
				en_La4 = 0;
				en_La1 = 1;
			}


		}
		if (en_La1)
		{
			angle_end_La1 = angle_end_La1 + 0.10f;
			if (angle_end_La1 >= M_PI / 2.0f)
			{
				angle_end_La1 = (GLfloat)M_PI / 2.0f;
				en_La1 = 0;
				en_La2 = 1;
				En_La = 1;
			}
		}

		if (en_La2)
		{
			angle_end_La2 = angle_end_La2 + 0.10f;
			if (angle_end_La2 >= M_PI / 4.0f)
			{
				angle_end_La2 = (GLfloat)M_PI / 4.0f;
				en_La2 = 0;
				en_La3 = 1;
			}
		}
		if (en_La3)
		{
			angle_end_La3 = angle_end_La3 + 0.10f;
			if (angle_end_La3 >= 3.0f * M_PI / 2.0f)
			{
				angle_end_La3 = 3.0f * (GLfloat)M_PI / 2.0f;
				en_La3 = 0;
				en_HL = 1;
			}


		}

		if (en_HL)
		{
			vertex_end_HL = vertex_end_HL + 0.05f;
			if (vertex_end_HL >= 4.26f)
			{
				en_HL = 0;
				vertex_end_HL = 4.26f;
				en_Gurukul = 0;

			}
		}




	}


}


/////////////////////////////////////-------------Aashish
void drawHallUpperBeam(void)
{

	void drawUpperDesign(void);
	glPushMatrix();
	glTranslatef(0.500000f, 61.500000f, -18.000000f);
	glScalef(1.400000f, 0.500000f, 0.300000f);
	drawUpperDesign();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-39.000000f, 61.500000f, -47.500000f);
	glScalef(1.100000f, 0.500000f, 1.100000f);
	glRotatef(90.099197f, 0.0f, 1.0f, 0.0f);
	drawUpperDesign();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.000000f, 61.500000f, -77.500000f);
	glScalef(1.400000f, 0.500000f, 0.900000f);
	//glRotatef(90.099197, 0.0f, 1.0f, 0.0f);
	drawUpperDesign();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(39.000000f, 61.500000f, -48.000000f);
	glScalef(1.100000f, 0.500000f, 1.100000f);
	glRotatef(269.994507f, 0.0f, 1.0f, 0.0f);
	glRotatef(0.100000f, 1.0f, 0.0f, 0.0f);
	drawUpperDesign();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.000000f, 62.0000f, -60.500000f);
	glScalef(1.300000f, 0.350000f, 1.000000f);
	//glRotatef(269.994507f, 0.0f, 1.0f, 0.0f);
	//glRotatef(0.100000, 1.0f, 0.0f, 0.0f);
	drawUpperDesign();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.500000f, 62.0000f, -37.500000f);
	glScalef(1.300000f, 0.350000f, 1.000000f);
	//glRotatef(269.994507f, 0.0f, 1.0f, 0.0f);
	//glRotatef(0.100000, 1.0f, 0.0f, 0.0f);
	drawUpperDesign();
	glPopMatrix();


	glPushMatrix();
	glTranslatef(-14.500000f, 61.500000f, -48.000000f);
	glScalef(1.400000f, 0.500000f, 1.100000f);
	glRotatef(269.194458f, 0.0f, 1.0f, 0.0f);
	//glRotatef(0.100000, 1.0f, 0.0f, 0.0f);
	drawUpperDesign();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(14.500000f, 61.500000f, -48.000000f);
	glScalef(1.400000f, 0.500000f, 1.100000f);
	glRotatef(269.194458f, 0.0f, 1.0f, 0.0f);
	//glRotatef(0.100000, 1.0f, 0.0f, 0.0f);
	drawUpperDesign();
	glPopMatrix();

}

void drawUpperDesign(void)
{
	void drawCube(GLfloat height, GLfloat width, GLfloat depth);

	glPushMatrix();
	glTranslatef(0.0f, 5.0f, 0.0f);
	drawCube(0.95f, 57.0f, 3.5f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.4f, 0.4f, 0.4f);
	glTranslatef(0.0f, 4.0f, 0.0f);
	drawCube(0.95f, 57.0f, 3.0f);
	glPopMatrix();


	glPushMatrix();
	glColor3f(0.6f, 0.6f, 0.6f);
	glTranslatef(0.0f, 3.3f, 0.0f);
	drawCube(1.5f, 57.0f, 3.0f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.4f, 0.4f, 0.4f);
	glTranslatef(0.0f, 2.1f, 0.0f);
	drawCube(0.90f, 57.0f, 2.8f);
	glPopMatrix();
}
void drawInnerLeftSide()
{
	void WhiteBlackDoor1Block(GLfloat);
	void LibraryWallWithWindow(GLfloat, GLfloat, GLfloat);
	void LibraryRoof(GLfloat, GLfloat, GLfloat);
	glPushMatrix();
	{
		for (GLfloat i = 0.0f; i < 40.0f * 4; i += 40.0f)
		{
			for (GLfloat j = 0.0f; j <= 70; j += 66.0f)
			{
				glPushMatrix();
				glTranslatef(0.0f, j, -i + 3.00f);
				WhiteBlackDoor1Block(1.0f);
				glPopMatrix();
			}
		}
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-103.0f, 38.0f, -133.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		LibraryWallWithWindow(14.0f, 20.0f, 0.0f);
		glTranslatef(21.0f, 0.0f, 0.0f);
		LibraryWallWithWindow(14.0f, 20.0f, 0.0f);
		glTranslatef(21.0f, 0.0f, 0.0f);
		LibraryWallWithWindow(14.0f, 20.0f, 0.0f);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-102.0f, 60.0f, -154.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(-50.0f, 1.0f, 0.0f, 0.0f);
		LibraryRoof(56.0f, 20.0f, 1.0f);
	}
	glPopMatrix();

	glPushMatrix();//back side coverup wall
	glTranslatef(-124.0f, 63.0f, -268.0f);
	glScalef(2.0f, 65.0f, 75.2f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-117.0f, 63.0f, -191.0f);
	glScalef(7.5f, 68.0f, 2.0f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-117.0f, 63.0f, -343.0f);
	glScalef(7.5f, 68.0f, 2.0f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//Library Lower wall

	glPushMatrix();
	glColor3f(0.87f, 0.764f, 0.611f);
	glTranslatef(-100.0f, 17.0f, -148.5f);
	glScalef(1.0f, 18.5f, 38.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.65f, 0.325f, 0.168f);
	glTranslatef(-99.0f, 17.0f, -130.0f);
	glScalef(1.0f, 18.2f, 1.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();


	glPushMatrix();
	glColor3f(0.65f, 0.325f, 0.168f);
	glTranslatef(-99.0f, 17.0f, -148.0f);
	glScalef(1.0f, 18.2f, 1.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.65f, 0.325f, 0.168f);
	glTranslatef(-99.0f, 17.0f, -166.0f);
	glScalef(1.0f, 18.2f, 1.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.65f, 0.325f, 0.168f);
	glTranslatef(-99.0f, 17.0f, -183.8f);
	glScalef(1.0f, 18.2f, 1.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();



}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void drawInnerRightSide()
{
	void WhiteBlackDoor1Block(GLfloat);
	void LibraryWallWithWindow(GLfloat, GLfloat, GLfloat);
	void LibraryRoof(GLfloat, GLfloat, GLfloat);
	void drawFilledBlanks(void);
	glPushMatrix();//Global

				   //glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
				   //glTranslatef(200.0f, 0.0f, 0.0f);

	glPushMatrix();
	{
		for (GLfloat i = 0.0f; i < 40.0f * 4; i += 40.0f)
		{
			for (GLfloat j = 0.0f; j <= 70; j += 66.0f)
			{
				glPushMatrix();
				glTranslatef(0.0f, j, -i + 3.0f);
				WhiteBlackDoor1Block(-1.0f);
				glPopMatrix();
			}
		}
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(103.0f, 38.0f, -133.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		LibraryWallWithWindow(14.0f, 20.0f, 0.0f);
		glTranslatef(-21.0f, 0.0f, 0.0f);
		LibraryWallWithWindow(14.0f, 20.0f, 0.0f);
		glTranslatef(-21.0f, 0.0f, 0.0f);
		LibraryWallWithWindow(14.0f, 20.0f, 0.0f);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(102.0f, 60.0f, -154.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(-50.0f, 1.0f, 0.0f, 0.0f);
		LibraryRoof(56.0f, 20.0f, 1.0f);
	}
	glPopMatrix();

	glPushMatrix();//back side coverup wall
	glTranslatef(124.0f, 63.0f, -268.0f);
	glScalef(2.0f, 65.0f, 75.2f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(117.0f, 63.0f, -191.0f);
	glScalef(7.5f, 68.0f, 2.0f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(117.0f, 63.0f, -343.0f);
	glScalef(7.5f, 68.0f, 2.0f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	//Library Lower wall

	glPushMatrix();
	glColor3f(0.87f, 0.764f, 0.611f);
	glTranslatef(100.0f, 17.0f, -148.5f);
	glScalef(1.0f, 18.5f, 38.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.65f, 0.325f, 0.168f);
	glTranslatef(99.0f, 17.0f, -130.0f);
	glScalef(1.0f, 18.2f, 1.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();


	glPushMatrix();
	glColor3f(0.65f, 0.325f, 0.168f);
	glTranslatef(99.0f, 17.0f, -148.0f);
	glScalef(1.0f, 18.2f, 1.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.65f, 0.325f, 0.168f);
	glTranslatef(99.0f, 17.0f, -166.0f);
	glScalef(1.0f, 18.2f, 1.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.65f, 0.325f, 0.168f);
	glTranslatef(99.0f, 17.0f, -183.8f);
	glScalef(1.0f, 18.2f, 1.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	drawFilledBlanks();

	glPopMatrix();

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void drawFilledBlanks(void)
{
	glPushMatrix();//left 
				   //give color
	glColor3f(0.9f, 0.9f, 0.9f);
	glTranslatef(96.000000f, 17.500000f, -112.000000f);
	glScalef(3.199999f, 13.900017f, 1.000000f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f);
	glTranslatef(91.500000f, 1.000000f, -112.000000f);
	glScalef(7.399995f, 2.899999f, 1.000000f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.9f, 0.9f, 0.9f);
	glTranslatef(-97.000000f, 17.500000f, -112.000000f);
	glScalef(3.199999f, 13.900017f, 1.000000f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f);
	glTranslatef(-92.500000f, 1.000000f, -112.000000f);
	glScalef(7.399995f, 2.899999f, 1.000000f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f);
	glTranslatef(-128.000000f, 35.500000f, -136.000000f);
	glScalef(2.300000f, 37.500000f, 61.899628f);
	glCallList(dsCubeRegular);
	glPopMatrix();


	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f);
	glTranslatef(128.000000f, 35.500000f, -136.000000f);
	glScalef(2.300000f, 37.500000f, 61.899628f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.941f, 0.902f, 0.549f);
	glTranslatef(-113.000000f, 57.000000f, -111.500000f);
	glScalef(11.100006f, 22.900051f, 1.100000f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.941f, 0.902f, 0.549f);
	glTranslatef(111.000000f, 57.500000f, -113.500000f);
	glScalef(9.400000f, 22.300049f, 1.100000f);
	glCallList(dsCubeRegular);
	glPopMatrix();
}



void WhiteBlackDoor1Block(GLfloat i)
{

	void DrawShape(GLfloat, GLfloat, GLint);
	void Doors(void);
	void DrawPiller(GLfloat, GLfloat, GLfloat);


	glPushMatrix();
	{
		glTranslatef(i * (-100.0f), 34.0f, -210.0f);
		glRotatef(i *90.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.0f, 1.0f, 0.2f);
		glColor3f(0.25f, 0.25f, 0.25f);
		DrawShape(40.0f, 30.0f, 10);

		glTranslatef(0.0f, 0.0f, 5.0f);
		Doors();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(i * (-100.0f), 16.0f, -192.5f);
		glColor3f(0.25f, 0.25f, 0.25f);
		DrawPiller(2.0f, 36.0f, 5.0f);

		glTranslatef(0.0f, 0.0f, -36.0f);
		glColor3f(0.25f, 0.25f, 0.25f);
		DrawPiller(2.0f, 36.0f, 5.0f);
	}
	glPopMatrix();
}

void LibraryWallWithWindow(GLfloat width, GLfloat height, GLfloat depth)
{
	void BaseQuad(GLfloat, GLfloat, GLfloat);
	void WhiteLine1(void);
	void WhiteLine2(void);

	glPushMatrix();
	glTranslatef(0.0f, 2.5f * height / 4.0f, 0.0f);
	glScalef(0.7f, 0.8f, 1.0f);
	drawWindowLibrary();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, -2.5f, 0.0f);
	glColor3f(0.913f, 0.184f, 0.196f);
	BaseQuad(width + 10.0f, height + 5.0f, depth);

	glTranslatef(0.0f, (height + 5.0f) / 2.0f, -2.03f);
	glScalef((width + 10.0f) / 2.0f, (height + 5.0f) / 2.0f, 2.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	for (GLfloat i = -width / 2.0f; i <= width / 2.0f; i += 7.0f)
	{
		for (GLfloat j = 0.0f; j <= height - 5.0f; j += 10.0f)
		{
			glPushMatrix();
			{
				glTranslatef(i, j, 0.05f);
				glColor3f(1.0f, 1.0f, 1.0f);
				WhiteLine1();
				glTranslatef(0.0f, 5.0f, 0.0f);
				glColor3f(1.0f, 1.0f, 1.0f);
				WhiteLine2();
			}
			glPopMatrix();
		}
	}

}

void LibraryRoof(GLfloat width, GLfloat height, GLfloat depth)
{
	void BaseQuad(GLfloat, GLfloat, GLfloat);
	void WhiteLine1(void);
	void WhiteLine2(void);

	glPushMatrix();
	glTranslatef(0.0f, -2.5f, 0.0f);
	glColor3f(0.521f, 0.223f, 0.160f);
	BaseQuad(width + 10.0f, height + 5.0f, depth);

	glTranslatef(0.0f, (height + 5.0f) / 2.0f, -1.03f);
	glScalef((width + 10.0f) / 2.0f, (height + 5.0f) / 2.0f, 1.0f);
	glCallList(dsCubeRegular);
	glPopMatrix();

	for (GLfloat i = -width / 2.0f; i <= width / 2.0f; i += 7.0f)
	{
		for (GLfloat j = 0.0f; j <= height - 5.0f; j += 10.0f)
		{
			glPushMatrix();
			{
				glTranslatef(i, j, 1.0f);
				glColor3f(0.819f, 0.537f, 0.537f);
				WhiteLine1();
				glTranslatef(0.0f, 5.0f, 0.0f);
				glColor3f(0.819f, 0.537f, 0.537f);
				WhiteLine2();
			}
			glPopMatrix();
		}
	}

}

void WhiteLine1(void)
{
	void BaseQuad(GLfloat, GLfloat, GLfloat);

	glPushMatrix();
	{
		glTranslatef(0.0f, 0.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		BaseQuad(7.0f, 0.2f, 0.0f);
		glTranslatef(0.0f, 4.8f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		BaseQuad(7.0f, 0.2f, 0.0f);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(3.4f, 0.2f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		BaseQuad(0.2f, 4.6f, 0.0f);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(-3.4f, 0.2f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		BaseQuad(0.2f, 4.6f, 0.0f);
	}
	glPopMatrix();

}

void WhiteLine2(void)
{
	void BaseQuad(GLfloat, GLfloat, GLfloat);

	glColor3f(1.0f, 1.0f, 1.0f);
	BaseQuad(7.0f, 0.2f, 0.0f);

	glPushMatrix();
	{
		glTranslatef(0.0f, 4.8f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		BaseQuad(7.0f, 0.2f, 0.0f);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(0.0f, 0.2f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		BaseQuad(0.2f, 4.6f, 0.0f);
	}
	glPopMatrix();
}

void Doors(void)
{
	GLUquadric *quadric;
	quadric = gluNewQuadric();

	glColor3f(0.0f, 0.0f, 0.0f);
	gluPartialDisk(quadric, 15.0f, 20.0f, 26, 13, -90, 20);
	glColor3f(0.5f, 0.5f, 0.5f);
	gluPartialDisk(quadric, 15.0f, 20.0f, 26, 13, -70, 20);

	glColor3f(0.0f, 0.0f, 0.0f);
	gluPartialDisk(quadric, 15.0f, 20.0f, 26, 13, -50, 20);
	glColor3f(0.5f, 0.5f, 0.5f);
	gluPartialDisk(quadric, 15.0f, 20.0f, 26, 13, -30, 20);

	glColor3f(0.0f, 0.0f, 0.0f);
	gluPartialDisk(quadric, 15.0f, 20.0f, 26, 13, -10, 20);
	glColor3f(0.5f, 0.5f, 0.5f);
	gluPartialDisk(quadric, 15.0f, 20.0f, 26, 13, 10, 20);

	glColor3f(0.0f, 0.0f, 0.0f);
	gluPartialDisk(quadric, 15.0f, 20.0f, 26, 13, 30, 20);
	glColor3f(0.5f, 0.5f, 0.5f);
	gluPartialDisk(quadric, 15.0f, 20.0f, 26, 13, 50, 20);


	glColor3f(0.0f, 0.0f, 0.0f);
	gluPartialDisk(quadric, 15.0f, 20.0f, 26, 13, 70, 20);

	gluDeleteQuadric(quadric);
}

void Draw_Leaves(float radius, int index)
{
	GLUquadric *quadric = NULL;
	glBindTexture(GL_TEXTURE_2D, biLeafTexture);
	quadric = gluNewQuadric();
	gluQuadricTexture(quadric, TRUE);
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluSphere(quadric, radius, 30, 30);

}

void DrawTree(void)
{
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//quadric = gluNewQuadric();
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//glRotatef(90, 1, 0, 0);
	//glPushMatrix();
	///*=============== 1 Start: First Layer==================== */
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 21, 0.02 * 21, 1.5, 30, 30);
	///*=============== 1 End: First Layer 1==================== */

	//glPopMatrix();
	//glPushMatrix();

	///*=============== 3 Start: Second Layer RHS==================== */

	///*Right Side Branch*/
	//glTranslatef(0.29, 0, -1);
	//glRotatef(-7.5, 0, 1, 0);
	//glPushMatrix();
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 13, 0.02 * 13, 1.05, 30, 30);

	///*=============== 6 Start: Third Layer RHS RHS==================== */
	//glTranslatef(0.17 + 0.06, 0, -1);
	//glRotatef(-4 - 3, 0, 1, 0);
	//glPushMatrix();
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 8, 0.02 * 8, 1.05, 30, 30);
	///*===============End: Third Layer RHS RHS==================== */

	///*=============== 10 Start: Fourth Layer RHS RHS LHS==================== */
	//glTranslatef(-0.26, 0, -1);
	//glRotatef(9.5, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 3, 0.02 * 3, 1.05, 30, 30);
	///*===============End: Third Layer  RHS RHS LHS==================== */

	///*=============== 17 Start: Fifth Layer RHS RHS LHS==================== */
	//glTranslatef(-0.2, 0, -1);
	//glRotatef(11, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 3, 0.02 * 3, 1.05, 30, 30);
	///*===============End: Third Fifth  RHS RHS LHS==================== */
	//glPushMatrix();
	//glTranslatef(0, -2.05, -2.94 - 0.83);
	///*Draw Leaves*/
	//glScalef(1, 1, 1 - 0.54);
	//Draw_Leaves(0.34 + 0.44 + 0.2 + 1.87, 2);
	///*Draw Leaves*/
	//glPopMatrix();


	///*=============== 11 Start: Fourth Layer RHS RHS LHS==================== */
	//glPopMatrix();
	//glTranslatef(0.16, 0, -1);
	//glRotatef(-6, 0, 1, 0);
	//glPushMatrix();
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 5, 0.02 * 5, 1.05, 30, 30);
	///*===============End: Fourth Layer  RHS RHS LHS==================== */
	///*=============== 18 Start: Fifth Layer RHS RHS LHS==================== */

	//glTranslatef(-0.62, 0, -1);
	//glRotatef(29.5, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 2, 0.02 * 2, 1.05 + 0.08, 30, 30);
	///*===============End: Fifth Layer  RHS RHS LHS==================== */
	//glPushMatrix();
	//glTranslatef(0, 0, -0.24);
	///*Draw Leaves*/
	//glScalef(1, 1, 1 - 0.54);
	///*Draw Leaves*/

	//glTranslatef(0, 0, -1.74);
	//glPopMatrix();



	///*=============== 19 Start: Fifth Layer RHS RHS LHS==================== */
	//glPopMatrix();
	//glTranslatef(-0.11, 0, -1 - 0.23);
	//glRotatef(6.5, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 3, 0.02 * 3, 1.05 + 0.08 + 0.23, 30, 30);
	///*=============== 19 End: Fifth Layer  RHS RHS LHS==================== */
	//glPushMatrix();
	//glTranslatef(0.67, 0, -0.34);
	///*Draw Leaves*/
	//glScalef(1, 1, 1 - 0.54);
	////Draw_Leaves(0.34 + 0.44 + 0.2 + 0.14, 0);
	//glTranslatef(0.75 - 1.65 - 0.17, 0, -2.37 + 0.54 + 1.28);
	//Draw_Leaves(0.34 + 0.44 + 0.2 + 0.14 + 0.59, 2);
	//glTranslatef(0.75 - 1.65 - 0.17 + 2.1 - 0.72, 0.58, -2.37 + 0.54 + 1.28 - 2.07 + 0.87);
	//Draw_Leaves(0.34 + 0.44 + 0.2 + 0.14 + 0.59 + 0.19, 2);
	//glTranslatef(-0.91 + 1.65 + 0.17 - 2.1 - 0.05 + 0.72, 0.17 - 0.58, -2.41 - 0.54 - 1.28 + 2.07 - 0.22 - 0.87);
	//Draw_Leaves(0.34 + 0.44 + 0.2 + 0.14 + 0.59 + 0.57, 0);
	///*Draw Leaves*/
	//glPopMatrix();


	///*=============== 5 Start: Third Layer RHS LHS==================== */
	//glPopMatrix();
	//glTranslatef(-0.37, 0, -1);
	//glRotatef(12.5, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 5, 0.02 * 5, 1.05, 30, 30);
	///*=============== 5 End: Third Layer RHS LHS==================== */

	///*=============== 9 Start: Fourth Layer RHS LHS==================== */
	//glTranslatef(-0.05, 0, -1);
	//glRotatef(2.5, 0, 1, 0);
	//glPushMatrix();
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 5, 0.02 * 5, 1.05, 30, 30);
	///*=============== 9 End: Fourth Layer RHS LHS==================== */

	///*=============== 15 Start: Fifth Layer RHS LHS==================== */
	//glTranslatef(-0.2, 0, -1);
	//glRotatef(7.5, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 2, 0.02 * 2, 1.05, 30, 30);
	///*=============== 15 End: Fifth Layer RHS LHS==================== */


	///*=============== 16 Start: Fifth Layer RHS LHS==================== */
	//glPopMatrix();
	//glTranslatef(0.29, 0, -1);
	//glRotatef(-13.5, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 3, 0.02 * 3, 1.05, 30, 30);
	///*=============== 16 End: Fifth Layer RHS LHS==================== */

	///*=============== 2 Start: Second Layer LHS==================== */
	///*Left Side Branch*/
	//glPopMatrix();
	//glTranslatef(-0.56 - 0.09, 0, -1);
	//glRotatef(16 + 6.5, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 8, 0.02 * 8, 1.03 + 0.07 + 0.04, 30, 30);

	///*===============End: Second Layer LHS==================== */

	///*=============== 4 Start: Third Layer LHS==================== */

	//glTranslatef(0.08 + 0.16, 0, -1);
	//glRotatef(-4 + -9.5, 0, 1, 0);
	//glPushMatrix();
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 8, 0.02 * 8, 1.03 + 0.03, 30, 30);

	///*===============End: Third Layer LHS==================== */

	///*=============== 7 Start: Fourth Layer LHS LHS==================== */

	//glTranslatef(-0.34, 0, -1);
	//glRotatef(15, 0, 1, 0);
	//glPushMatrix();
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 5, 0.02 * 5, 1.03 + 0.03, 30, 30);

	///*=============== 7 End:  Fourth Layer LHS LHS==================== */

	///*=============== 12 Start: Fifth Layer LHS LHS==================== */

	//glTranslatef(0, 0, -1);
	//glRotatef(-2.5, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 3, 0.02 * 3, 1.03 + 0.03, 30, 30);

	///*=============== 12 End:  Fifth Layer LHS LHS==================== */
	//glPushMatrix();
	//glTranslatef(-0.4, 0, -0.24);
	///*Draw Leaves*/
	//glScalef(1, 1, 1 - 0.54);
	//glTranslatef(0.75 + 0.26, 0 - 1.33, -1.57 + 0.41);
	//Draw_Leaves(0.34 + 0.44 + 0.2 + 0.69 + 0.39, 2);
	//glTranslatef(1.65, 0.82, 0);
	//Draw_Leaves(0.34 + 0.44 + 0.2 + 0.69 + 0.39, 2);
	//glTranslatef(0.5 + 0.07 - 0.26 - 1.65, -2.96 + 1.33 - 0.82, -2.13 - 1 - 0.41);
	//Draw_Leaves(0.34 + 0.44 + 0.2 + 0.69 + 0.94 - 0.34, 0);
	///*Draw Leaves*/
	//glPopMatrix();
	///*=============== 13 Start: Fifth Layer LHS LHS==================== */
	//glPopMatrix();
	//glTranslatef(0.43, 0, -1);
	//glRotatef(-21.5, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 2, 0.02 * 2, 1.03 + 0.03, 30, 30);

	///*=============== 13 End:  Fifth Layer LHS LHS==================== */

	///*=============== 8 Start: Fourth Layer LHS RHS==================== */
	//glPopMatrix();
	//glTranslatef(0.21 + 0.09, 0, -1);
	//glRotatef(-6 - 13, 0, 1, 0);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 3, 0.02 * 3, 1.03 + 0.03, 30, 30);

	///*=============== 8 End: Fourth Layer LHS RHS==================== */

	///*=============== 14 Start: Fifth Layer LHS RHS==================== */
	//glTranslatef(-0.12f, 0.0f, -1.0f);
	//glRotatef(6.5f, 0.0f, 1.0f, 0.0f);
	//glBindTexture(GL_TEXTURE_2D, biBranchTexture);
	//gluQuadricTexture(quadric, TRUE);
	//gluQuadricNormals(quadric, GLU_SMOOTH);
	//gluCylinder(quadric, 0.02 * 3.0, 0.02 * 3.0, 1.03 + 0.03 + 1.0, 30, 30);

	///*=============== 14 End: Fifth Layer LHS RHS==================== */
	//glPushMatrix();
	//glTranslatef(0.0f, 0.0f, -0.24f);
	///*Draw Leaves*/
	//glScalef(1.0f, 1.0f, 1.0f - 0.54f);
	//Draw_Leaves(0.34f + 0.44f, 0);
	///*Draw Leaves*/
	//glPopMatrix();
}

void drawLibraryMainDoor(void)
{
	glPushMatrix();
	{
		glColor3f(0.367059f, 0.254706f, 0.204706f);
		drawWindowBorder(10.0f, 5.0f, 0.55f, 0.7f);

		drawCube(19.0f, 0.5f, 0.5f);

		glColor3f(0.96f, 0.95f, 0.60f);
		drawCube(19.0f, 9.5f, 0.3f);

		glColor3f(0.467059f, 0.354706f, 0.304706f);
		glPushMatrix();
		{
			glTranslatef(0.0f, 9.2f, 0.0f);
			drawCube(1.0f, 9.5f, 0.5f);

		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.0f, -9.2f, 0.0f);
			drawCube(1.0f, 9.5f, 0.5f);

		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.0f, 3.2f, 0.0f);
			drawCube(0.5f, 9.5f, 0.5f);

		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.0f, -3.2f, 0.0f);
			drawCube(0.5f, 9.5f, 0.5f);

		}
		glPopMatrix();

	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(7.0f, 0.0f, 0.0f);
		glColor3f(0.367059f, 0.254706f, 0.204706f);

		drawWindowBorder(10.0f, 2.2f, 0.5f, 0.5f);

		glPushMatrix();
		{
			glTranslatef(-0.8f, 10.0f, 0.0f);

			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

			glColor3f(0.75f, 0.75f, 0.75f);
			quadric = gluNewQuadric();

			gluCylinder(quadric, 0.1f, 0.1f, 14.0f, 26, 13);

			glTranslatef(1.6f, 0.0f, 0.0f);
			gluCylinder(quadric, 0.1f, 0.1f, 14.0f, 26, 13);

		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.0f, -4.0f, 0.0f);
			glColor3f(0.367059f, 0.254706f, 0.204706f);

			drawCube(0.5f, 4.0f, 0.5f);

			glTranslatef(0.0f, -2.8f, 0.0f);
			glColor3f(0.96f, 0.95f, 0.60f);
			drawCube(5.5f, 4.0f, 0.3f);
		}
		glPopMatrix();
	}
	glPopMatrix();
	/////////////////////////////
	glPushMatrix();
	{
		glTranslatef(-7.0f, 0.0f, 0.0f);
		glColor3f(0.367059f, 0.254706f, 0.204706f);

		drawWindowBorder(10.0f, 2.2f, 0.5f, 0.5f);

		glPushMatrix();
		{
			glTranslatef(-0.8f, 10.0f, 0.0f);

			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

			glColor3f(0.75f, 0.75f, 0.75f);
			quadric = gluNewQuadric();

			gluCylinder(quadric, 0.1f, 0.1f, 14.0f, 26, 13);

			glTranslatef(1.6f, 0.0f, 0.0f);
			gluCylinder(quadric, 0.1f, 0.1f, 14.0f, 26, 13);

		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.0f, -4.0f, 0.0f);
			glColor3f(0.367059f, 0.254706f, 0.204706f);

			drawCube(0.5f, 4.0f, 0.5f);

			glTranslatef(0.0f, -2.8f, 0.0f);
			glColor3f(0.96f, 0.95f, 0.60f);
			drawCube(5.5f, 4.0f, 0.3f);
		}
		glPopMatrix();
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslatef(0.0f, 13.0f, 0.0f);
		glColor3f(0.367059f, 0.254706f, 0.204706f);

		drawWindowBorder(3.2f, 9.2f, 0.5f, 0.5f);

		glPushMatrix();
		{
			glTranslatef(-8.5f, 3.0f, 0.0f);
			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

			glColor3f(0.75f, 0.75f, 0.75f);
			quadric = gluNewQuadric();

			gluCylinder(quadric, 0.1f, 0.1f, 6.0f, 26, 13);

			for (int i = 0; i < 15; i++)
			{
				glTranslatef(1.1f, 0.0f, 0.0f);
				gluCylinder(quadric, 0.1f, 0.1f, 6.0f, 26, 13);
			}
		}
		glPopMatrix();

	}
	glPopMatrix();
}

int LoadGLTextures(GLuint * texture, TCHAR ImageResourceId[])
{
	HBITMAP hBitmap;
	BITMAP bmp;
	int iStatus = -1;

	glGenTextures(1, texture);

	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), ImageResourceId, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hBitmap)
	{
		iStatus = 0;
		GetObject(hBitmap, sizeof(bmp), &bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

		DeleteObject(hBitmap);
	}

	return(iStatus);
}

void update(void)
{
	gfFanAngle = gfFanAngle + 0.9f;
	if (gfFanAngle == 360.0f)
	{
		gfFanAngle = 0.0f;
	}

	if (giTImerGlobalCount > 0 && giTImerGlobalCount <= 200)
	{
		cam.MoveForward(-0.8);
		PositionLight0.Z -= 0.9f;
		PositionLight0.X += 0.05f;
		if (giTImerGlobalCount == 200 && gbResetAttenuation == false)
			gbResetAttenuation = true;
	}

	if (giTImerGlobalCount == 200)
		PlaySound(TEXT("../../Resources/Audio/Gurukul.wav"), NULL, SND_ASYNC | SND_LOOP);

	if (AttenuationLight0.Constant == 1.0f && AttenuationLight0.Linear == 0.0f && gfLightSphereAlpha == 0.0f && AmbientLight0.R == 0.3f && AmbientLight0.R == 0.3f && AmbientLight0.R == 0.3f && BackgroundColor.R == 0.529f && BackgroundColor.G == 0.808f && BackgroundColor.B == 0.980f)
		gbResetAttenuation = false;

	if (gbResetAttenuation == true)
	{
		AttenuationLight0.Constant -= 0.04f;
		if (AttenuationLight0.Constant < 1.0f)
			AttenuationLight0.Constant = 1.0f;

		AttenuationLight0.Linear -= 0.01f;
		if (AttenuationLight0.Linear < 0.0f)
			AttenuationLight0.Linear = 0.0f;

		gfLightSphereAlpha -= 0.02f;
		if (gfLightSphereAlpha < 0.0f)
			gfLightSphereAlpha = 0.0f;

		AmbientLight0.R += 0.004f;
		if (AmbientLight0.R > 0.3f)
			AmbientLight0.R = 0.3f;

		AmbientLight0.G += 0.004f;
		if (AmbientLight0.R > 0.3f)
			AmbientLight0.R = 0.3f;

		AmbientLight0.B += 0.004f;
		if (AmbientLight0.R > 0.3f)
			AmbientLight0.R = 0.3f;

		// bring light sky blue color from black
		// 0.529 0.808 0.980

		//BackgroundColor.R += 0.01058f;
		BackgroundColor.R += 0.00529f;
		if (BackgroundColor.R > 0.529f)
			BackgroundColor.R = 0.529f;

		//BackgroundColor.G += 0.01616f;
		BackgroundColor.G += 0.00808f;
		if (BackgroundColor.G > 0.808f)
			BackgroundColor.G = 0.808f;

		//BackgroundColor.B += 0.0196f;
		BackgroundColor.B += 0.00980f;
		if (BackgroundColor.B > 0.980f)
			BackgroundColor.B = 0.980f;
	}

	if (AttenuationLight0.Constant == 1.0f && AttenuationLight0.Linear == 0.0f && (giTImerGlobalCount > 300 && giTImerGlobalCount <= 600))
	{
		gbLightOnCamera = true;
		cam.MoveForward(-0.7);
	}

	if (giTImerGlobalCount > 600)
	{
		gfMainDoorAngle = gfMainDoorAngle + 1.0f;
		if (gfMainDoorAngle >= 90.0f)
			gfMainDoorAngle = 90.0f;
	}

	if (gfMainDoorAngle == 90.0f && giTImerGlobalCount > 650 && giTImerGlobalCount <= 1100)
	{
		cam.MoveForward(-0.7);
		if (giTImerGlobalCount > 900 && giTImerGlobalCount <= 950)
			cam.Yaw(1.0);
		if (giTImerGlobalCount > 950 && giTImerGlobalCount <= 1000)
			cam.Yaw(-1.2);
	}

	if (giTImerGlobalCount > 1100 && giTImerGlobalCount <= 1230)
	{
		cam.MoveUpward(0.35);
		cam.Yaw(-1.30);
	}

	if (giTImerGlobalCount > 1235 && giTImerGlobalCount <= 1450)
	{
		cam.MoveForward(-0.15);
		if (giTImerGlobalCount == 1340)
		{
			//gbSetAttenuation = true;
			gbFadeout = true;
		}	
	}
	

	if (gbFadeout == true)
	{
		fprintf_s(stream, "in fadeout\n");
		gfHUDScreenAlpha += 0.01f;
		if (gfHUDScreenAlpha >= 1.0f)
		{
			gfHUDScreenAlpha = 1.0f;
			fprintf_s(stream, "setting fadeout to false\n");
			gbFadeout = false;
		}
		fprintf_s(stream, "ghHUDSceenAlpha : %f\n", gfHUDScreenAlpha);
	}

	if (gfHUDScreenAlpha == 0.0f)
		gbFadeReset = false;

	if (gbFadeReset == true)
	{
		fprintf_s(stream, "in fadereset\n");
		gfHUDScreenAlpha -= 0.01f;
		if (gfHUDScreenAlpha == 0.0f)
			gfHUDScreenAlpha = 0.0f;

		fprintf_s(stream, "ghHUDSceenAlpha : %f\n", gfHUDScreenAlpha);
	}

	if (gfHUDScreenAlpha == 1.0f && giTImerGlobalCount >= 1450)
	{
		gbFadeout = false;
		cam.CameraPosition.X = 0.0;
		cam.CameraPosition.Y = 5.0;
		cam.CameraPosition.Z = -150.0;
		gbFadeReset = true;
	}
		
	if (gbSetAttenuation == true)
	{
		fprintf_s(stream, "in set attenuation\n\n");
		AttenuationLight0.Constant += 0.04f;
		if (AttenuationLight0.Constant > 2.0f)
			AttenuationLight0.Constant = 2.0f;

		AttenuationLight0.Linear += 0.01f;
		if (AttenuationLight0.Linear > 0.5f)
			AttenuationLight0.Linear = 0.5f;

		fprintf_s(stream, "constant attenuation : %f\n", AttenuationLight0.Constant);
		fprintf_s(stream, "linear attenuation : %f\n", AttenuationLight0.Linear);

		// Set ambient to 0
		AmbientLight0.R -= 0.004f;
		if (AmbientLight0.R < 0.0f)
			AmbientLight0.R = 0.0f;

		AmbientLight0.G -= 0.004f;
		if (AmbientLight0.R < 0.0f)
			AmbientLight0.R = 0.0f;

		AmbientLight0.B += 0.004f;
		if (AmbientLight0.R < 0.0f)
			AmbientLight0.R = 0.0f;

		// from sky blue to black
		// 0.529 0.808 0.980

		//BackgroundColor.R += 0.01058f;
		BackgroundColor.R -= 0.0529f;
		if (BackgroundColor.R < 0.0f)
			BackgroundColor.R = 0.0f;

		//BackgroundColor.G += 0.01616f;
		BackgroundColor.G -= 0.0808f;
		if (BackgroundColor.G < 0.0f)
			BackgroundColor.G = 0.0f;

		//BackgroundColor.B += 0.0196f;
		BackgroundColor.B -= 0.0980f;
		if (BackgroundColor.B < 0.0f)
			BackgroundColor.B = 0.0f;
	}

	//if (AttenuationLight0.Constant == 2.0f && AttenuationLight0.Linear == 0.5f && AmbientLight0.R == 0.0f && AmbientLight0.R == 0.0f && AmbientLight0.R == 0.0f && BackgroundColor.R == 0.0f && BackgroundColor.G == 0.0f && BackgroundColor.B == 0.0f && giTImerGlobalCount >= 1450)
	//{
	//	gbSetAttenuation = false;
	//	cam.CameraPosition.X = 0.0;
	//	cam.CameraPosition.Y = 5.0;
	//	cam.CameraPosition.Z = -150.0;
	//	gbResetAttenuation = true;
	//}

	


		
	if (giTImerGlobalCount > 1460 && giTImerGlobalCount <= 1700)
	{
		cam.MoveForward(-0.5);

		if (giTImerGlobalCount > 1470 && giTImerGlobalCount <= 1600)
		{
			cam.Yaw(-0.70);
			cam.MoveUpward(0.12);
		}

		if (giTImerGlobalCount > 1530 && giTImerGlobalCount <= 1600)
			cam.Pitch(0.5);
		
		if (giTImerGlobalCount > 1640 && giTImerGlobalCount <= 1700)
			cam.Pitch(-0.6);
	}

	if (giTImerGlobalCount > 1700 && giTImerGlobalCount <= 1900)
		cam.Yaw(0.9);

	if (giTImerGlobalCount > 1900 && giTImerGlobalCount <= 2080)
	{
		cam.Pitch(-0.05);
		cam.MoveUpward(0.05);
		cam.MoveForward(-0.5);
	}

	if (giTImerGlobalCount > 2080 && giTImerGlobalCount <= 2200)
		cam.Yaw(-0.83);

	/*if (giTImerGlobalCount > 2200 && giTImerGlobalCount <= 2300)
		cam.MoveForward(0.2);*/

	if (giTImerGlobalCount > 2200)
	{
		gfWindowAngle = gfWindowAngle + 1.0f;
		if (gfWindowAngle >= 120.0f)
			gfWindowAngle = 120.0f;

		gfDoorAngle = gfDoorAngle + 1.0f;
		if (gfDoorAngle >= 90.0f)
			gfDoorAngle = 90.0f;

		gfUpperFlapAngle = gfUpperFlapAngle + 1.0f;
		if (gfUpperFlapAngle >= 30.0f)
			gfUpperFlapAngle = 30.0f;
	}

	if (giTImerGlobalCount > 2250 && giTImerGlobalCount <= 2350)
		cam.MoveForward(-0.25);

	if (giTImerGlobalCount > 2350 && giTImerGlobalCount <= 2400)
	{
		cam.Pitch(0.1);
		cam.Yaw(0.1);
	}

	if (giTImerGlobalCount > 2400 && giTImerGlobalCount <= 2500)
	{
		cam.MoveForward(-0.25);
		cam.Yaw(0.1);
	}
		


	if (giTImerGlobalCount == 2500)
	{
		gbChairShow = true;
		en_Gurukul = 1;
		En_Ga1 = 1;
		en_Ga1 = 1;

		SetTimer(ghWnd, TIMER_ID_1, 20, NULL);
		gbLight0FinalAttenuation = true;
	}

	if (gbLight0FinalAttenuation == true)
	{
		AttenuationLight0.Linear += 0.001f;
		if (AttenuationLight0.Linear >= 2.5f)
			AttenuationLight0.Linear = 2.5f;

		AttenuationLight1.Constant -= 0.05f;
		if (AttenuationLight1.Constant <= 1.0f)
			AttenuationLight1.Constant = 1.0f;

		AttenuationLight1.Linear -= 0.05f;
		if (AttenuationLight1.Linear <= 0.0f)
			AttenuationLight1.Linear = 0.0f;

	}

	//LIGHT0 final linear attenuation 5.0
	//LIGHT1 initial 
}
