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

namespace GridItems
{

    GridViewer::GridViewer(const QGLFormat & format, QWidget *parent)
        : QGLWidget(format, parent)
    {
    }

    GridViewer::~GridViewer()
    {
    }

    void GridViewer::initializeGL()
    {
        glClearColor(1.0, 1.0, 1.0, 1.0);
    }

    void GridViewer::resizeGL(int w, int h)
    {
        glViewport(0, 0, w, h);
        glFrustum(-50, 50, -50, 50, 1, 100);
    }

    void GridViewer::paintGL()
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glColor3f(0.5, 0.5, 0.5);
        glBegin(GL_QUADS);
        glVertex3f(-25, -25, -10);
        glVertex3f(25, -25, -10);
        glVertex3f(25, 25, -10);
        glVertex3f(-25, 25, -10);
        glEnd();
    }

} // namespace GridItems
