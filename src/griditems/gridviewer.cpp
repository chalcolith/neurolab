/*
Neurocognitive Linguistics Lab
Copyright (c) 2010,2011 Gordon Tisher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 - Neither the name of the Neurocognitive Linguistics Lab nor the
   names of its contributors may be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "gridviewer.h"
#include "../neurogui/mainwindow.h"

#include <QMouseEvent>
#include <QWheelEvent>

using namespace NeuroGui;

namespace GridItems
{

    static qreal MAX_DISTANCE = 12;
    static qreal MIN_DISTANCE = 2;

    GridViewer::GridViewer(const QGLFormat & format, QWidget *parent)
        : QGLWidget(format, parent),
          _distance(8), _angle(0),
          _grid_item(0)
    {
        bool res = connect(MainWindow::instance(), SIGNAL(itemSelected(NeuroItem*)), this, SLOT(selectedItem(NeuroItem*)));
        res = connect(MainWindow::instance(), SIGNAL(postStep()), this, SLOT(postStep()));
        res = true;
    }

    GridViewer::~GridViewer()
    {
    }

    void GridViewer::loadSettings(QSettings & settings)
    {
        settings.beginGroup("GridViewer.View");
        //_distance = settings.value("distance", _distance).toReal();
        //_angle = settings.value("angle", _angle).toReal();
        settings.endGroup();
    }

    void GridViewer::saveSettings(QSettings & settings)
    {
        settings.beginGroup("GridViewer.View");
        settings.setValue("distance", _distance);
        settings.setValue("angle", _angle);
        settings.endGroup();
    }

    void GridViewer::selectedItem(NeuroGui::NeuroItem *item)
    {
        NeuroGridItem *gi = dynamic_cast<NeuroGridItem *>(item);
        _grid_item = gi;
        updateGL();
    }

    void GridViewer::postStep()
    {
        if (_grid_item)
            updateGL();
    }

    void GridViewer::initializeGL()
    {
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

        glEnable(GL_FOG);
        glFogf (GL_FOG_DENSITY, 0.35);
        glHint(GL_FOG_HINT, GL_NICEST);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        static GLfloat fc[4] = { 0.5, 0.5, 0.5, 0.5 };
        glFogfv(GL_FOG_COLOR, fc);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glPointSize(3.5);
        glLineWidth(1.0);
    }

    void GridViewer::resizeGL(int w, int h)
    {
        glViewport(0, 0, w, h);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60, (double) w / (double) h, 0.1, MAX_DISTANCE + 1.5);
    }

#ifdef DEBUG
    static void draw_cube()
    {
        // front
        glBegin(GL_LINE_STRIP);
        glVertex3d(-1, -1, 1);
        glVertex3d(1, -1, 1);
        glVertex3d(1, 1, 1);
        glVertex3d(-1, 1, 1);
        glVertex3d(-1, -1, 1);
        glEnd();

        // left
        glBegin(GL_LINE_STRIP);
        glVertex3d(-1, -1, -1);
        glVertex3d(-1, -1, 1);
        glVertex3d(-1, 1, 1);
        glVertex3d(-1, 1, -1);
        glVertex3d(-1, -1, -1);
        glEnd();

        // back
        glBegin(GL_LINE_STRIP);
        glVertex3d(1, -1, -1);
        glVertex3d(-1, -1, -1);
        glVertex3d(-1, 1, -1);
        glVertex3d(1, 1, -1);
        glVertex3d(1, -1, -1);
        glEnd();

        // right
        glBegin(GL_LINE_STRIP);
        glVertex3d(1, -1, -1);
        glVertex3d(1, -1, 1);
        glVertex3d(1, 1, 1);
        glVertex3d(1, 1, -1);
        glVertex3d(1, -1, -1);
        glEnd();
    }
#endif

    void GridViewer::paintGL()
    {
        glFogf(GL_FOG_START, _distance - 1.0);
        glFogf(GL_FOG_END, _distance + 1.0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslated(0, 0, -_distance);
        glRotated(_angle, 0, 1, 0.01);

#ifdef DEBUG
        glColor3f(0, 0, 0);
        draw_cube();
#endif

        if (_grid_item)
        {
            if (_grid_item->glPointArray().size() > 0)
            {
                glVertexPointer(3, GL_FLOAT, 0, _grid_item->glPointArray().data());
                glColorPointer(3, GL_FLOAT, 0, _grid_item->glPointColorArray().data());
                glDrawArrays(GL_POINTS, 0, _grid_item->glPointArray().size() / 3);
            }

            if (_grid_item->glLineArray().size() > 0)
            {
                glVertexPointer(3, GL_FLOAT, 0, _grid_item->glLineArray().data());
                glColorPointer(3, GL_FLOAT, 0, _grid_item->glLineColorArray().data());
                glDrawArrays(GL_LINES, 0, _grid_item->glLineArray().size() / 3);
            }
        }
    }

    void GridViewer::mousePressEvent(QMouseEvent *e)
    {
        _last_mouse_pos = e->posF();
    }

    void GridViewer::mouseMoveEvent(QMouseEvent *e)
    {
        QPointF new_pos = e->posF();

        qreal delta = new_pos.x() - _last_mouse_pos.x();
        _angle += delta / 4;

        _last_mouse_pos = new_pos;

        updateGL();
    }

    void GridViewer::mouseReleaseEvent(QMouseEvent *e)
    {
        _last_mouse_pos = e->posF();
    }

    void GridViewer::wheelEvent(QWheelEvent *e)
    {
        _distance = qBound(MIN_DISTANCE, _distance - (qreal)e->delta() / 500, MAX_DISTANCE);
        updateGL();
    }

} // namespace GridItems
