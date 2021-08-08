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

#include "model.h"

#include <QFile>
#include <QTextStream>
#include <QVarLengthArray>

#include <QtOpenGL>

Model::Model(const QString &filePath)
    : m_fileName(QFileInfo(filePath).fileName())
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    Point3d boundsMin( 1e9, 1e9, 1e9);
    Point3d boundsMax(-1e9,-1e9,-1e9);

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString input = in.readLine();
        if (input.isEmpty() || input[0] == '#')
            continue;

        QTextStream ts(&input);
        QString id;
        ts >> id;
        if (id == "v") {
            Point3d p;
            for (int i = 0; i < 3; ++i) {
                ts >> p[i];
                boundsMin[i] = qMin(boundsMin[i], p[i]);
                boundsMax[i] = qMax(boundsMax[i], p[i]);
            }
            m_points << p;
        } else if (id == "f" || id == "fo") {
            QVarLengthArray<int, 4> p;

            while (!ts.atEnd()) {
                QString vertex;
                ts >> vertex;
                const int vertexIndex = vertex.split('/').value(0).toInt();
                if (vertexIndex)
                    p.append(vertexIndex > 0 ? vertexIndex - 1 : m_points.size() + vertexIndex);
            }

            for (int i = 0; i < p.size(); ++i) {
                const int edgeA = p[i];
                const int edgeB = p[(i + 1) % p.size()];

                if (edgeA < edgeB)
                    m_edgeIndices << edgeA << edgeB;
            }

            for (int i = 0; i < 3; ++i)
                m_pointIndices << p[i];

            if (p.size() == 4)
                for (int i = 0; i < 3; ++i)
                    m_pointIndices << p[(i + 2) % 4];
        }
    }

    const Point3d bounds = boundsMax - boundsMin;
    const qreal scale = 1 / qMax(bounds.x, qMax(bounds.y, bounds.z));
    for (int i = 0; i < m_points.size(); ++i)
        m_points[i] = (m_points[i] - (boundsMin + bounds * 0.5)) * scale;

    m_normals.resize(m_points.size());
    for (int i = 0; i < m_pointIndices.size(); i += 3) {
        const Point3d a = m_points.at(m_pointIndices.at(i));
        const Point3d b = m_points.at(m_pointIndices.at(i+1));
        const Point3d c = m_points.at(m_pointIndices.at(i+2));

        const Point3d normal = cross(b - a, c - a).normalize();

        for (int j = 0; j < 3; ++j)
            m_normals[m_pointIndices.at(i + j)] += normal;
    }

    for (int i = 0; i < m_normals.size(); ++i)
        m_normals[i] = m_normals[i].normalize();
}

void Model::render(bool wireframe, bool normals) const
{
    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    if (wireframe) {
        glVertexPointer(3, GL_FLOAT, 0, (float *)m_points.data());
        glDrawElements(GL_LINES, m_edgeIndices.size(), GL_UNSIGNED_INT, m_edgeIndices.data());
    } else {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        glShadeModel(GL_SMOOTH);

        glEnableClientState(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, (float *)m_points.data());
        glNormalPointer(GL_FLOAT, 0, (float *)m_normals.data());
        glDrawElements(GL_TRIANGLES, m_pointIndices.size(), GL_UNSIGNED_INT, m_pointIndices.data());

        glDisableClientState(GL_NORMAL_ARRAY);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHTING);
    }

    if (normals) {
        QVector<Point3d> normals;
        for (int i = 0; i < m_normals.size(); ++i)
            normals << m_points.at(i) << (m_points.at(i) + m_normals.at(i) * 0.02f);
        glVertexPointer(3, GL_FLOAT, 0, (float *)normals.data());
        glDrawArrays(GL_LINES, 0, normals.size());
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_DEPTH_TEST);
}

