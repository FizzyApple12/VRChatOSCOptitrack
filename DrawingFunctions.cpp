#include "DrawingFunctions.h"

#include <math.h>
#include <time.h>

namespace DrawingFunctions
{

    const float X1 = .5257311F;
    const float Z1 = .8506508F;

    const float vdata[12][3] = {
      {-X1, 0.0, Z1}, {X1, 0.0, Z1}, {-X1, 0.0, -Z1}, {X1, 0.0, -Z1},
      {0.0, Z1, X1}, {0.0, Z1, -X1}, {0.0, -Z1, X1}, {0.0, -Z1, -X1},
      {Z1, X1, 0.0}, {-Z1, X1, 0.0}, {Z1, -X1, 0.0}, {-Z1, -X1, 0.0} };

    const int tindices[20][3] = {
      {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
      {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
      {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
      {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };


    void Normalize(float* vector)
    {
        float length = std::sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);

        vector[0] /= length;
        vector[1] /= length;
        vector[2] /= length;
    }

    void DrawTriangle(const float* a, const float* b, const float* c, int div, float r)
    {
        if (div <= 0)
        {
            glNormal3fv(a); glVertex3f(a[0] * r, a[1] * r, a[2] * r);
            glNormal3fv(b); glVertex3f(b[0] * r, b[1] * r, b[2] * r);
            glNormal3fv(c); glVertex3f(c[0] * r, c[1] * r, c[2] * r);
        }
        else
        {
            GLfloat ab[3], ac[3], bc[3];
            for (int i = 0; i < 3; i++)
            {
                ab[i] = (a[i] + b[i]) / 2;
                ac[i] = (a[i] + c[i]) / 2;
                bc[i] = (b[i] + c[i]) / 2;
            }
            Normalize(ab); Normalize(ac); Normalize(bc);
            DrawTriangle(a, ab, ac, div - 1, r);
            DrawTriangle(b, bc, ab, div - 1, r);
            DrawTriangle(c, ac, bc, div - 1, r);
            DrawTriangle(ab, bc, ac, div - 1, r);
        }
    }

    void DrawSphere(int ndiv, float radius) {
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < 20; i++)
            DrawTriangle(vdata[tindices[i][0]], vdata[tindices[i][1]], vdata[tindices[i][2]], ndiv, radius);
        glEnd();
    }

    void DrawCube(float scale, bool active, bool selected)
    {
        const float sizex = 0.5f * scale;
        const float sizey = 0.5f * scale;
        const float sizez = 0.5f * scale;

        float selectionColorBump = 0.05f;

        if (active)
            selectionColorBump = 0.5f;
        if (selected)
            selectionColorBump = 1.0f;

        glBegin(GL_QUADS);

        // FRONT
        glColor3f(0.0f, 0.0f, selectionColorBump);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-sizex, -sizey, sizez);
        glVertex3f(sizex, -sizey, sizez);
        glVertex3f(sizex, sizey, sizez);
        glVertex3f(-sizex, sizey, sizez);

        // BACK
        glNormal3f(0.0f, 0.0f, selectionColorBump);
        glVertex3f(-sizex, -sizey, -sizez);
        glVertex3f(-sizex, sizey, -sizez);
        glVertex3f(sizex, sizey, -sizez);
        glVertex3f(sizex, -sizey, -sizez);


        // LEFT
        glColor3f(selectionColorBump, 0.0f, 0.0f);
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-sizex, -sizey, sizez);
        glVertex3f(-sizex, sizey, sizez);
        glVertex3f(-sizex, sizey, -sizez);
        glVertex3f(-sizex, -sizey, -sizez);

        // RIGHT
        glNormal3f(selectionColorBump, 0.0f, 0.0f);
        glVertex3f(sizex, -sizey, -sizez);
        glVertex3f(sizex, sizey, -sizez);
        glVertex3f(sizex, sizey, sizez);
        glVertex3f(sizex, -sizey, sizez);


        // TOP
        glColor3f(0.0f, selectionColorBump, 0.0f);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-sizex, sizey, sizez);
        glVertex3f(sizex, sizey, sizez);
        glVertex3f(sizex, sizey, -sizez);
        glVertex3f(-sizex, sizey, -sizez);

        // BOTTOM
        glNormal3f(0.0f, selectionColorBump, 0.0f);
        glVertex3f(-sizex, -sizey, sizez);
        glVertex3f(-sizex, -sizey, -sizez);
        glVertex3f(sizex, -sizey, -sizez);
        glVertex3f(sizex, -sizey, sizez);

        glEnd();

    }

}