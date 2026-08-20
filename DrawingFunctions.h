#pragma once

#ifndef _OPEN_GL_DRAWING_FUNCTIONS_H_
#define _OPEN_GL_DRAWING_FUNCTIONS_H_

#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <cmath>

namespace DrawingFunctions
{

    void Normalize(float* vector);

    void DrawTriangle(const float *a, const float *b, const float *c, int div, float r);

    void DrawSphere(int ndiv, float radius=1.0);

    void DrawCube(float scale, bool active, bool selected);

};

#endif