#pragma once
//Function Declarations//

void DrawClassRoom(void);

void DrawFrontWall(void);
void DrawLeftSideWall(void);
void DrawRightSideWall(void);
void DrawCarpate(void);
void DrawShape(GLfloat width, GLfloat height, GLint slices);

void DrawMiddleArch(GLfloat cx, GLfloat cy, GLfloat start_angle, GLfloat arc_angle, int num_segments, int arc_length);
