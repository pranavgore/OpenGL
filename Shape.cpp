#include "Shape.h"

void cQuad::Draw(void)
{
	M3DVector3f vNormal;
	M3DVector3f vPoints[4] = {
		{ v[Q1].get_x(), v[Q1].get_y(), v[Q1].get_z() },
		{ v[Q2].get_x(), v[Q2].get_y(), v[Q2].get_z() },
		{ v[Q3].get_x(), v[Q3].get_y(), v[Q3].get_z() },
		{ v[Q4].get_x(), v[Q4].get_y(), v[Q4].get_z() } };

	glBegin(GL_QUADS);

	m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
	glNormal3fv(vNormal);

	glColor3f(clr.get_r(), clr.get_g(), clr.get_b());
	glVertex3f(v[Q1].get_x(), v[Q1].get_y(), v[Q1].get_z());
	glVertex3f(v[Q2].get_x(), v[Q2].get_y(), v[Q2].get_z());
	glVertex3f(v[Q3].get_x(), v[Q3].get_y(), v[Q3].get_z());
	glVertex3f(v[Q4].get_x(), v[Q4].get_y(), v[Q4].get_z());
	glEnd();

}

void cCube::Draw(void)
{
	fr_quad.Draw();
	bk_quad.Draw();
	tp_quad.Draw();
	bt_quad.Draw();
	lt_quad.Draw();
	rt_quad.Draw();
}
