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

#include "gridsplugin.h"
#include "gridviewer.h"

#include <QDockWidget>
#include <QVBoxLayout>
#include <QMenu>
#include <QSettings>

namespace GridItems
{

    static GridsPlugin GridsPluginInstance;
    GridsPlugin *GridsPlugin::_instance = 0;

    GridsPlugin::GridsPlugin()
        : NeuroGui::Plugin("GridItems", GridItems::VERSION())
    {
        // don't try to check for errors here; throwing an exception when loading a library is bad
        _instance = this;
    }

    GridsPlugin::~GridsPlugin()
    {
    }

    void GridsPlugin::initialize()
    {
        initDockWidget();

        // get settings
        QSettings settings;
        settings.beginGroup("GridsPlugin");
        bool visible = settings.value("visible").toBool();
        _dock_widget->setVisible(visible);
        settings.endGroup();

        _viewer->loadSettings(settings);
    }

    void GridsPlugin::cleanup()
    {
        // save settings
        QSettings settings;
        settings.beginGroup("GridsPlugin");
        bool visible = _dock_widget->isVisible();
        settings.setValue("visible", visible);
        settings.endGroup();

        _viewer->saveSettings(settings);
    }

    void GridsPlugin::initDockWidget()
    {
        // create and add doc widget
        _dock_widget = new QDockWidget(QObject::tr("Grid Viewer"));
        _dock_widget->setAllowedAreas(Qt::RightDockWidgetArea);

        QGLFormat format;
        format.setVersion(2, 1);
        format.setDoubleBuffer(true);
        format.setDepth(true);
        format.setRgba(true);
        format.setAlpha(false);
        format.setAccum(false);
        format.setStencil(false);
        format.setStereo(false);
        format.setDirectRendering(true);
        format.setOverlay(false);

        _viewer = new GridViewer(format);
        _viewer->setMinimumSize(250, 0);
        _viewer->setObjectName("gridViewer");

        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(_viewer);

        QWidget *widget = new QWidget();
        widget->setLayout(layout);

        _dock_widget->setWidget(widget);

        NeuroGui::MainWindow::instance()->addDockWidget(Qt::RightDockWidgetArea, _dock_widget);

        // add to view menu
        NeuroGui::MainWindow::instance()->toolBarsMenu()->addAction(_dock_widget->toggleViewAction());
    }

} // namespace GridItems
