#ifndef __SHAPE_H__
#define __SHAPE_H__
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Common\MyMath3D.h"

typedef enum eQUADRANT {Q1 = 0,	Q2,	Q3,	Q4,	}eQUADRANT_t;
typedef enum eCOLOR { RED = 0, GREEN, BLUE, CYAN, MAGENTA, YELLOW, BLACK, WHITE, BROWN, DEFINED_MAX_COLOR }eCOLOR_t;

class cColor
{
private:
	GLfloat r;
	GLfloat g;
	GLfloat b;

public:
	cColor() //by default white blue, even if something goes wron, you can recognise blue color
	{ 
		r = 0.0f; 
		g = 0.0f; 
		b = 1.0f; 
	}	

	cColor(GLfloat red, GLfloat green, GLfloat blue) : r(red), g(green), b(blue) 
	{

	}

	cColor& operator=(cColor &ref) 
	{ 
		this->r = ref.r; 
		this->g = ref.g; 
		this->b = ref.b; 
		return *this; 
	}

	GLfloat get_r(void) 
	{ 
		return r; 
	}

	GLfloat get_g(void) 
	{ 
		return g; 
	}
	
	GLfloat get_b(void) 
	{ 
		return b; 
	}

	~cColor() 
	{
		// Code
	}
};

// cColor(0.824f, 0.412f, 0.118f),
static cColor basicColor[DEFINED_MAX_COLOR] = { 
	//		red						green						blue
	cColor(1.0f, 0.0f, 0.0f), cColor(0.0f, 1.0f, 0.0f), cColor(0.0f, 0.0f, 1.0f),
	//		cyan					magenta						yellow
	cColor(0.0f, 1.0f, 1.0f), cColor(1.0f, 0.0f, 1.0f), cColor(1.0f, 1.0f, 0.0f),
	//		black					white				//brown or chocolate
	cColor(0.0f, 0.0f, 0.0f), cColor(1.0f, 1.0f, 1.0f), /*cColor(0.647059f, 0.164706f, 0.164706f)*/cColor(0.52f, 0.37f, 0.26f),
};

class cVertex
{
private:
	GLfloat x;
	GLfloat y;
	GLfloat z;

public:
	cVertex() 
	{
		// Code
	}

	cVertex(GLfloat x_pt, GLfloat y_pt, GLfloat z_pt) : x(x_pt), y(y_pt), z(z_pt) 
	{
		// Code
	}

	cVertex& operator=(cVertex& ref) 
	{ 
		this->x = ref.x; 
		this->y = ref.y; 
		this->z = ref.z; 
		return *this; 
	}

	GLfloat get_x(void) 
	{ 
		return x; 
	}

	GLfloat get_y(void) 
	{ 
		return y; 
	}

	GLfloat get_z(void) 
	{ 
		return z; 
	}

	~cVertex() 
	{
		// Code
	}
};

class cShape
{
public:
	virtual void Draw(void) = 0;
};

class cQuad : public cShape
{
private:
	cVertex v[4];
	cColor clr;

public:
	cQuad() 
	{
		// Code
	}

	cQuad(cVertex& vq1, cVertex& vq2, cVertex& vq3, cVertex& vq4) 
	{ 
		v[Q1] = vq1; 
		v[Q2] = vq2; 
		v[Q3] = vq3; 
		v[Q4] = vq4; 
	}

	cQuad(cVertex& vq1, cVertex& vq2, cVertex& vq3, cVertex& vq4, eCOLOR_t clr_type)
	{
		v[Q1] = vq1; 
		v[Q2] = vq2; 
		v[Q3] = vq3; 
		v[Q4] = vq4;  
		clr = basicColor[clr_type];
	}

	cQuad& operator=(cQuad &ref) 
	{	
		this->v[Q1] = ref.v[Q1];
		this->v[Q2] = ref.v[Q2];
		this->v[Q3] = ref.v[Q3];
		this->v[Q4] = ref.v[Q4];
		this->clr = ref.clr;
		return *this;
	}
						
	virtual void Draw(void);

	~cQuad() 
	{
		// Code
	}
};

class cCube : public cShape
{
private:
	cQuad fr_quad;
	cQuad bk_quad;
	cQuad tp_quad;
	cQuad bt_quad;
	cQuad lt_quad;
	cQuad rt_quad;
	cCube()	// Dont want to instantiate without any initialize value
	{
		// Code
	}

public:
	cCube(cQuad &fr, cQuad &bk, cQuad &tp, cQuad &bt, cQuad &lt, cQuad &rt)
	{
		fr_quad = fr;
		bk_quad = bk;
		tp_quad = tp;
		bt_quad = bt;
		lt_quad = lt;
		rt_quad = rt;
	}
	virtual void Draw(void);

	~cCube() 
	{
		// Code
	}
};

#endif	/*__SHAPE_H__*/