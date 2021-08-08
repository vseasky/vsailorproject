/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the documentation of Qt. It was originally
** published as part of Qt Quarterly.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef POINT3D_H
#define POINT3D_H

#include "math.h"
#include <qglobal.h>

struct Point3d
{
    float x, y, z;

    Point3d()
        : x(0)
        , y(0)
        , z(0)
    {
    }

    Point3d(float x_, float y_, float z_)
        : x(x_)
        , y(y_)
        , z(z_)
    {
    }

    Point3d operator+(const Point3d &p) const
    {
        return Point3d(*this) += p;
    }

    Point3d operator-(const Point3d &p) const
    {
        return Point3d(*this) -= p;
    }

    Point3d operator*(float f) const
    {
        return Point3d(*this) *= f;
    }


    Point3d &operator+=(const Point3d &p)
    {
        x += p.x;
        y += p.y;
        z += p.z;
        return *this;
    }

    Point3d &operator-=(const Point3d &p)
    {
        x -= p.x;
        y -= p.y;
        z -= p.z;
        return *this;
    }

    Point3d &operator*=(float f)
    {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    Point3d normalize() const
    {
        float r = 1. / sqrt(x * x + y * y + z * z);
        return Point3d(x * r, y * r, z * r);
    }
    float &operator[](unsigned int index) {
        Q_ASSERT(index < 3);
        return (&x)[index];
    }

    const float &operator[](unsigned int index) const {
        Q_ASSERT(index < 3);
        return (&x)[index];
    }
};

inline float dot(const Point3d &a, const Point3d &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Point3d cross(const Point3d &a, const Point3d &b)
{
    return Point3d(a.y * b.z - a.z * b.y,
                   a.z * b.x - a.x * b.z,
                   a.x * b.y - a.y * b.x);
}

#endif
