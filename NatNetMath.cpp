#include <windows.h>
#include <tchar.h>
#include <math.h>
#include "NatNetMath.h"
#include <float.h>

#include <stdio.h>
#include <stdlib.h>

#pragma warning( disable : 4244 )


namespace NatNetMath
{

    float NormalizeAngle(float angle)
    {
        float modAngle = fmodf(angle, 360.0f);

        if (modAngle < 0.0f)
            return modAngle + 360.0f;
        else
            return modAngle;
    }

    EulerAngles NormalizeAngles(EulerAngles angles)
    {
        return { 
            NormalizeAngle(angles.x * (180.0 / MATH_PI)),
            NormalizeAngle(angles.y * (180.0 / MATH_PI)),
            NormalizeAngle(angles.z * (180.0 / MATH_PI))
        };
    }

    /* Convert quaternion to Euler angles (in radians). */
    EulerAngles Eul_FromQuat(Quaternion rotation)
    {
        float sqw = rotation.w * rotation.w;
        float sqx = rotation.x * rotation.x;
        float sqy = rotation.y * rotation.y;
        float sqz = rotation.z * rotation.z;
        float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
        float test = rotation.x * rotation.w - rotation.y * rotation.z;
        EulerAngles v;

        if (test > 0.4995f * unit)
        { // singularity at north pole
            v.y = 2.0 * atan2(rotation.y, rotation.x);
            v.x = MATH_PI / 2;
            v.z = 0;
            return NormalizeAngles(v);
        }
        if (test < -0.4995f * unit)
        { // singularity at south pole
            v.y = -2.0 * atan2(rotation.y, rotation.x);
            v.x = -MATH_PI / 2;
            v.z = 0;
            return NormalizeAngles(v);
        }
        Quaternion q = {
            rotation.w,
            rotation.z,
            rotation.x,
            rotation.y
        };
        v.y = (float)atan2(2.0 * q.x * q.w + 2.0 * q.y * q.z, 1 - 2.0 * (q.z * q.z + q.w * q.w));     // Yaw
        v.x = (float)asin(2.0 * (q.x * q.z - q.w * q.y));                             // Pitch
        v.z = (float)atan2(2.0 * q.x * q.y + 2.0 * q.z * q.w, 1 - 2.0 * (q.y * q.y + q.z * q.z));      // Roll
        return NormalizeAngles(v);
    }

    void ConvertRHSPosZupToYUp(float& x, float& y, float& z)
    {
        /*
        [RHS, Y-Up]     [RHS, Z-Up]

                              Y
         Y                 Z /
         |__ X             |/__ X
         /
        Z

        Xyup  =  Xzup
        Yyup  =  Zzup
        Zyup  =  -Yzup
        */
        float yOriginal = y;
        y = z;
        z = -yOriginal;
    }

    void ConvertRHSRotZUpToYUp(float& qx, float& qy, float& qz, float& qw)
    {
        // -90 deg rotation about +X
        float qRx, qRy, qRz, qRw;
        float angle = -90.0f * MATH_PI / 180.0f;
        qRx = sin(angle / 2.0f);
        qRy = 0.0f;
        qRz = 0.0f;
        qRw = cos(angle / 2.0f);

        // rotate quat using quat multiply
        float qxNew, qyNew, qzNew, qwNew;
        qxNew = qw * qRx + qx * qRw + qy * qRz - qz * qRy;
        qyNew = qw * qRy - qx * qRz + qy * qRw + qz * qRx;
        qzNew = qw * qRz + qx * qRy - qy * qRx + qz * qRw;
        qwNew = qw * qRw - qx * qRx - qy * qRy - qz * qRz;

        qx = qxNew;
        qy = qyNew;
        qz = qzNew;
        qw = qwNew;
    }

}