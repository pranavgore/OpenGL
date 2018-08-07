#include <windows.h>
#include "Shape.h"

//#pragma comment(lib,"opengl32.lib")
//#pragma comment(lib,"glu32.lib")

#define NO_ROTATE	0

#define NUM_OF_STAIRS 17

#define STAIR_HEIGHT 0.1f
#define STAIR_WIDTH 0.8f

#define STAIR_COVER_HEIGHT 0.01f			// y
#define STAIR_COVER_WIDTH 0.4f				// x
#define STAIR_COVER_LENGTH 0.01f				// z

#define BAR_HEIGHT	0.6f
#define BAR_WIDTH 0.02f

#define STAIR_HANDLE_HEIGHT	4.0f
#define STAIR_HANDLE_WIDTH	0.07f

#define DISTANCE_BETWEEEN_TWO_STAIRCASE 20.0f


GLfloat g_z_translate = -120.0f;	//-15.0f;
GLfloat g_zoom_scale = 100.0f;		// No zoom 


#if NO_ROTATE != 1
	GLfloat g_x_angleStair = 0.0f;
	GLfloat g_y_angleStair = 0.0f;
#endif

cShape *pCube = NULL;
cShape *pStairCover = NULL;
cShape *pStairBar = NULL;
cShape *pStairHandle = NULL;

void Init_Stairs_Drawing_Objects(void)
{
	cQuad *pFrontQuad = NULL;
	cQuad *pBackQuad = NULL;
	cQuad *pRightQuad = NULL;
	cQuad *pLeftQuad = NULL;
	cQuad *pTopQuad = NULL;
	cQuad *pBottomQuad = NULL;

	GLfloat height = STAIR_HEIGHT, width = STAIR_WIDTH, length = 5.0;

	// Stair - Cube cordinates
	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, 1.0f);
		cVertex v2(-width, height, 1.0f);
		cVertex v3(-width, -height, 1.0f);
		cVertex v4(width, -height, 1.0f);
		pFrontQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, -1.0f);
		cVertex v2(-width, height, -1.0f);
		cVertex v3(-width, -height, -1.0f);
		cVertex v4(width, -height, -1.0f);
		pBackQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, -1.0f);
		cVertex v2(-width, height, -1.0f);
		cVertex v3(-width, height, 1.0f);
		cVertex v4(width, height, 1.0f);
		pTopQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, -height, -1.0f);
		cVertex v2(-width, -height, -1.0f);
		cVertex v3(-width, -height, 1.0f);
		cVertex v4(width, -height, 1.0f);
		pBottomQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, -1.0f);
		cVertex v2(width, height, 1.0f);
		cVertex v3(width, -height, 1.0f);
		cVertex v4(width, -height, -1.0f);
		pRightQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(-width, height, -1.0f);
		cVertex v2(-width, height, 1.0f);
		cVertex v3(-width, -height, 1.0f);
		cVertex v4(-width, -height, -1.0f);
		pLeftQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	pCube = new cCube(*pFrontQuad, *pBackQuad, *pTopQuad, *pBottomQuad, *pLeftQuad, *pRightQuad);
	delete pFrontQuad;
	delete pBackQuad;
	delete pTopQuad;
	delete pBottomQuad;
	delete pLeftQuad;
	delete pRightQuad;

	/******************/
	// Stair - Cover cube cordinates
	height = STAIR_COVER_HEIGHT, width = STAIR_COVER_WIDTH, length = STAIR_COVER_LENGTH;
	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, length);
		cVertex v2(-width, height, length);
		cVertex v3(-width, -height, length);
		cVertex v4(width, -height, length);
		pFrontQuad = new cQuad(v1, v2, v3, v4, WHITE);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, -length);
		cVertex v2(-width, height, -length);
		cVertex v3(-width, -height, -length);
		cVertex v4(width, -height, -length);
		pBackQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, -length);
		cVertex v2(-width, height, -length);
		cVertex v3(-width, height, length);
		cVertex v4(width, height, length);
		pTopQuad = new cQuad(v1, v2, v3, v4, WHITE);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, -height, -length);
		cVertex v2(-width, -height, -length);
		cVertex v3(-width, -height, length);
		cVertex v4(width, -height, length);
		pBottomQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, -length);
		cVertex v2(width, height, length);
		cVertex v3(width, -height, length);
		cVertex v4(width, -height, -length);
		pRightQuad = new cQuad(v1, v2, v3, v4, WHITE);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(-width, height, -length);
		cVertex v2(-width, height, length);
		cVertex v3(-width, -height, length);
		cVertex v4(-width, -height, -length);
		pLeftQuad = new cQuad(v1, v2, v3, v4, WHITE);
	}

	pStairCover = new cCube(*pFrontQuad, *pBackQuad, *pTopQuad, *pBottomQuad, *pLeftQuad, *pRightQuad);
	delete pFrontQuad;
	delete pBackQuad;
	delete pTopQuad;
	delete pBottomQuad;
	delete pLeftQuad;
	delete pRightQuad;
	/******************/

	// StairBar - Cube cordinates
	height = BAR_HEIGHT, width = BAR_WIDTH;
	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, 1.0f);
		cVertex v2(-width, height, 1.0f);
		cVertex v3(-width, -height, 1.0f);
		cVertex v4(width, -height, 1.0f);
		pFrontQuad = new cQuad(v1, v2, v3, v4, WHITE);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, 1.0f - width);
		cVertex v2(-width, height, 1.0f - width);
		cVertex v3(-width, -height, 1.0f - width);
		cVertex v4(width, -height, 1.0f - width);
		pBackQuad = new cQuad(v1, v2, v3, v4, WHITE);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, 1.0f - width);
		cVertex v2(-width, height, 1.0f - width);
		cVertex v3(-width, height, 1.0f);
		cVertex v4(width, height, 1.0f);
		pTopQuad = new cQuad(v1, v2, v3, v4, WHITE);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, -height, 1.0f - width);
		cVertex v2(-width, -height, 1.0f - width);
		cVertex v3(-width, -height, 1.0f);
		cVertex v4(width, -height, 1.0f);
		pBottomQuad = new cQuad(v1, v2, v3, v4, WHITE);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, 1.0f - width);
		cVertex v2(width, height, 1.0f);
		cVertex v3(width, -height, 1.0f);
		cVertex v4(width, -height, 1.0f - width);
		pRightQuad = new cQuad(v1, v2, v3, v4, WHITE);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(-width, height, 1.0f - width);
		cVertex v2(-width, height, 1.0f);
		cVertex v3(-width, -height, 1.0f);
		cVertex v4(-width, -height, 1.0f - width);
		pLeftQuad = new cQuad(v1, v2, v3, v4, WHITE);
	}

	pStairBar = new cCube(*pFrontQuad, *pBackQuad, *pTopQuad, *pBottomQuad, *pLeftQuad, *pRightQuad);
	delete pFrontQuad;
	delete pBackQuad;
	delete pTopQuad;
	delete pBottomQuad;
	delete pLeftQuad;
	delete pRightQuad;

	// StairHandle - Cube cordinates
	height = STAIR_HANDLE_HEIGHT, width = STAIR_HANDLE_WIDTH;
	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, 1.0f);
		cVertex v2(-width, height, 1.0f);
		cVertex v3(-width, -height, 1.0f);
		cVertex v4(width, -height, 1.0f);
		pFrontQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, 1.0f - width);
		cVertex v2(-width, height, 1.0f - width);
		cVertex v3(-width, -height, 1.0f - width);
		cVertex v4(width, -height, 1.0f - width);
		pBackQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, 1.0f - width);
		cVertex v2(-width, height, 1.0f - width);
		cVertex v3(-width, height, 1.0f);
		cVertex v4(width, height, 1.0f);
		pTopQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, -height, 1.0f - width);
		cVertex v2(-width, -height, 1.0f - width);
		cVertex v3(-width, -height, 1.0f);
		cVertex v4(width, -height, 1.0f);
		pBottomQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(width, height, 1.0f - width);
		cVertex v2(width, height, 1.0f);
		cVertex v3(width, -height, 1.0f);
		cVertex v4(width, -height, 1.0f - width);
		pRightQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	{// To limit scope of v1, v2, v3, v4
		cVertex v1(-width, height, 1.0f - width);
		cVertex v2(-width, height, 1.0f);
		cVertex v3(-width, -height, 1.0f);
		cVertex v4(-width, -height, 1.0f - width);
		pLeftQuad = new cQuad(v1, v2, v3, v4, BROWN);
	}

	pStairHandle = new cCube(*pFrontQuad, *pBackQuad, *pTopQuad, *pBottomQuad, *pLeftQuad, *pRightQuad);
	delete pFrontQuad;
	delete pBackQuad;
	delete pTopQuad;
	delete pBottomQuad;
	delete pLeftQuad;
	delete pRightQuad;
}

void Draw_Stairs(void)
{// draw cube
	static GLfloat x_translate = 0.0f, y_translate = -1.5f, z_translate = -6.0f;
	int num_stairs = NUM_OF_STAIRS;
#if 1
						// Staircase Left (1)
	glPushMatrix();
	{
		//glTranslatef(0.0f, y_translate, g_z_translate);
		glTranslatef(-18.0f, 5.0f, -106.0f);
		glTranslatef(-DISTANCE_BETWEEEN_TWO_STAIRCASE, 0.0f, 0.0f);
		glScalef(5.0f, 8.6f, 5.0f);
		glRotatef(90.0f, 0.0, 1.0f, 0.0f);
		
		#if NO_ROTATE != 1
			glRotatef(g_x_angleStair, 1.0f, 0.0f, 0.0f);
			glRotatef(g_y_angleStair, 0.0f, 1.0f, 0.0f);
		#endif

		glPushMatrix();
		{
			for (int i = 0; i < (num_stairs); i++)
			{
				//Stair cube
				pCube->Draw();

				//Stair cover
				glPushMatrix();
				{
					glTranslatef(0.0f, 0.1f, 1.0f);
					pStairCover->Draw();
				}
				glPopMatrix();

				//Stair right bar1
				glPushMatrix();
				{
					glTranslatef(0.95f, BAR_HEIGHT - 0.1f, -0.10f);	// x: bar moved to stair rt side, y: bar moved up by step ht, z: bar moved inside
					pStairBar->Draw();
				}
				glPopMatrix();

				//Stair right bar2
				glPushMatrix();
				{
					glTranslatef(0.95f, BAR_HEIGHT, -0.30f);	// x: bar moved to stair rt side, y: bar moved up by step ht, z: bar moved inside
					pStairBar->Draw();
				}
				glPopMatrix();

				//Stair left bar1
				glPushMatrix();
				{
					glTranslatef(-0.95f, BAR_HEIGHT - 0.1f, -0.10f);	// x: bar moved to stair rt side, y: bar moved up by step ht, z: bar moved inside
					pStairBar->Draw();
				}
				glPopMatrix();

				//Stair left bar2
				glPushMatrix();
				{
					glTranslatef(-0.95f, BAR_HEIGHT, -0.30f);	// x: bar moved to stair rt side, y: bar moved up by step ht, z: bar moved inside
					pStairBar->Draw();
				}
				glPopMatrix();

				// Translate up and deep inside to draw stair one above another
				glTranslatef(0.0f, 0.2f, -0.4f);
			}
		}
		glPopMatrix();

		//Stair left handle up bar
		glPushMatrix();
		{
			glTranslatef(-0.95f, 0.50f, 0.0f);			// stair handle (x: shift to left, y: shift to up, z: shift to back/front  )
			glRotatef(-63.5f, 1.0f, 0.0f, 0.0f);
			glTranslatef(0.0f, 3.0f, 0.0f);				// Shift up with y, cannot do in above translate
			pStairHandle->Draw();
		}
		glPopMatrix();
		
		//Stair right handle up bar
		glPushMatrix();
		{
			glTranslatef(+0.95f, 0.50f, 0.0f);			// stair handle (x: shift to right, y: shift to up, z: shift to back/front  )
			glRotatef(-63.5f, 1.0f, 0.0f, 0.0f);
			glTranslatef(0.0f, 3.0f, 0.0f);				// Shift up with y, cannot do in above translate
			pStairHandle->Draw();
		}
		glPopMatrix();
	}
	glPopMatrix();
#endif

#if 1
	// Staircase Right (2)
	glPushMatrix();
	{
		//glTranslatef(0.0f, y_translate, g_z_translate);
		glTranslatef(18.0f, 5.0f, -106.0f);
		glTranslatef(DISTANCE_BETWEEEN_TWO_STAIRCASE, 0.0f, 0.0f);
		glScalef(5.0f, 8.6f, 5.0f);
		glRotatef(-90.0f, 0.0, 1.0f, 0.0f);

#if NO_ROTATE != 1
		glRotatef(-g_x_angleStair, 1.0f, 0.0f, 0.0f);
		glRotatef(-g_y_angleStair, 0.0f, 1.0f, 0.0f);
#endif

		glPushMatrix();
		for (int i = 0; i < (num_stairs); i++)
		{
			//Stair cube
			pCube->Draw();

			//Stair cover
			glPushMatrix();
			{
				glTranslatef(0.0f, 0.1f, 1.0f);
				pStairCover->Draw();
			}
			glPopMatrix();

			//Stair right bar1
			glPushMatrix();
			{
				glTranslatef(0.95f, BAR_HEIGHT - 0.1f, -0.10f);	// x: bar moved to stair rt side, y: bar moved up by step ht, z: bar moved inside
				pStairBar->Draw();
			}
			glPopMatrix();

			//Stair right bar2
			glPushMatrix();
			{
				glTranslatef(0.95f, BAR_HEIGHT, -0.30f);	// x: bar moved to stair rt side, y: bar moved up by step ht, z: bar moved inside
				pStairBar->Draw();
			}
			glPopMatrix();

			//Stair left bar1
			glPushMatrix();
			{
				glTranslatef(-0.95f, BAR_HEIGHT - 0.1f, -0.10f);	// x: bar moved to stair rt side, y: bar moved up by step ht, z: bar moved inside
				pStairBar->Draw();
			}
			glPopMatrix();

			//Stair left bar2
			glPushMatrix();
			{
				glTranslatef(-0.95f, BAR_HEIGHT, -0.30f);	// x: bar moved to stair rt side, y: bar moved up by step ht, z: bar moved inside
				pStairBar->Draw();
			}
			glPopMatrix();

			// Translate up and deep inside to draw stair one above another
			glTranslatef(0.0f, 0.2f, -0.4f);
		}
		glPopMatrix();

		//Stair right handle up bar
		glPushMatrix();
		{
			glTranslatef(-0.95f, 0.50f, 0.0f);			// stair handle (x: shift to left, y: shift to up, z: shift to back/front  )
			glRotatef(-63.5f, 1.0f, 0.0f, 0.0f);
			glTranslatef(0.0f, 3.0f, 0.0f);				// Shift up with y, cannot do in above translate
			pStairHandle->Draw();
		}
		glPopMatrix();

		//Stair right handle up bar
		glPushMatrix();
		{
			glTranslatef(0.95f, 0.50f, 0.0f);			// stair handle (x: shift to right, y: shift to up, z: shift to back/front  )
			glRotatef(-63.5f, 1.0f, 0.0f, 0.0f);
			glTranslatef(0.0f, 3.0f, 0.0f);				// Shift up with y, cannot do in above translate
			pStairHandle->Draw();
		}
		glPopMatrix();
	}
	glPopMatrix();
#endif
}
