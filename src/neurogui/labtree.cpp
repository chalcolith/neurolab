/*
Neurocognitive Linguistics Lab
Copyright (c) 2010, Gordon Tisher
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

#include "labtree.h"
#include "labview.h"
#include "labscene.h"
#include "neuronarrowitem.h"
#include "mainwindow.h"
#include "../automata/exception.h"

#include <QScrollBar>

#include <typeinfo>

namespace NeuroLab
{

    // LabTreeNode
    quint32 LabTreeNode::NEXT_ID = 1;

    LabTreeNode::LabTreeNode(LabTree *_tree, LabTreeNode *_parent)
        : _id(NEXT_ID++), _tree(_tree), _parent(_parent), _scene(0), _view(0)
    {
        _scene = new LabScene(_tree->network());
        _view = new LabView(_scene, _tree ? _tree->_parent : 0);
    }

    LabTreeNode::LabTreeNode(LabScene *_scene, LabView *_view, LabTree *_tree, LabTreeNode *_parent)
        : _id(NEXT_ID++), _tree(_tree), _parent(_parent), _scene(_scene), _view(_view)
    {
    }

    LabTreeNode::~LabTreeNode()
    {
        for (QListIterator<LabTreeNode *> i(_children); i.hasNext(); )
        {
            delete i.next();
        }
        _children.clear();

        delete _view;
        _view = 0;

        delete _scene;
        _scene = 0;

        _tree = 0;
        _parent = 0;
    }

    void LabTreeNode::reset()
    {
        for (QListIterator<QGraphicsItem *> i(_scene->items()); i.hasNext(); )
        {
            NeuroNarrowItem *item = dynamic_cast<NeuroNarrowItem *>(i.next());
            if (item)
                item->reset();
        }
    }

    void LabTreeNode::update()
    {
        _scene->update();
    }

    void LabTreeNode::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        Q_ASSERT(_scene);
        Q_ASSERT(_view);

        if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_OLD)
        {
            ds << _id;

            // view
            if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_2)
            {
                _view->writeBinary(ds, file_version);
            }
            else
            {
                ds << _view->matrix();
            }

            // graphics items in this scene
            QList<QGraphicsItem *> items = _scene->items();
            quint32 num_items = static_cast<quint32>(items.size());

            ds << num_items;
            for (quint32 i = 0; i < num_items; ++i)
            {
                NeuroItem const * const item = dynamic_cast<NeuroItem const *>(items[i]);
                if (!item)
                    continue;

                QString typeName = item->getTypeName();
                ds << typeName;
                item->writeBinary(ds, file_version);
                item->writePointerIds(ds, file_version);
            }

            // children
            ds << static_cast<quint32>(_children.size());

            for (int i = 0; i < _children.size(); ++i)
            {
                _children[i]->writeBinary(ds, file_version);
            }
        }
    }

    void LabTreeNode::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_OLD)
        {
            ds >> _id;

            if (_id >= NEXT_ID)
                NEXT_ID = _id + 1;

            // view matrix
            if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_2)
            {
                _view->readBinary(ds, file_version);
            }
            else
            {
                QMatrix matrix;
                ds >> matrix;
                _view->setMatrix(matrix);
            }

            // graphics items in this scene
            quint32 num_items;
            ds >> num_items;

            for (quint32 i = 0; i < num_items; ++i)
            {
                QString type;
                ds >> type;

                NeuroItem *new_item = NeuroItem::create(type, _scene, QPointF(0,0), NeuroItem::CREATE_LOADFILE);
                if (new_item)
                {
                    new_item->readBinary(ds, file_version);
                    new_item->readPointerIds(ds, file_version);

                    _scene->addItem(new_item);
                }
                else
                {
                    throw LabException(QObject::tr("Error loading; unknown type %1").arg(type));
                }
            }

            // turn ids into pointers
            for (QListIterator<QGraphicsItem *> i(_scene->items()); i.hasNext(); )
            {
                NeuroItem *item = dynamic_cast<NeuroItem *>(i.next());
                if (item)
                    item->idsToPointers(_scene);
            }

            for (QListIterator<QGraphicsItem *> i(_scene->items()); i.hasNext(); )
            {
                NeuroItem *item = dynamic_cast<NeuroItem *>(i.next());
                if (item)
                    item->updateShape();
            }

            // children
            quint32 num_children;
            ds >> num_children;

            for (quint32 i = 0; i < num_children; ++i)
            {
                LabTreeNode *child = new LabTreeNode(_tree, this);
                if (child)
                    child->readBinary(ds, file_version);
            }

            // update scene and view
            _scene->update();
        }
    }


    //////////////////////////////////////////////////////////////////
    // LabTree

    LabTree::LabTree(QWidget *_parent, LabNetwork *_network)
        : _parent(_parent), _network(_network), _root(new LabTreeNode(this, 0)), _current(0)
    {
        _current = _root;
    }

    LabTree::~LabTree()
    {
        _parent = 0;
        _network = 0;
        _current = 0;
        delete _root;
    }

    QList<QGraphicsItem *> LabTree::items(LabTreeNode *n) const
    {
        if (!n)
            n = _root;

        Q_ASSERT(n != 0);
        Q_ASSERT(n->scene() != 0);

        QList<QGraphicsItem *> results = n->scene()->items();

        for (QListIterator<LabTreeNode *> i(n->children()); i.hasNext(); )
            results.append(items(i.next()));

        return results;
    }

    void LabTree::reset(LabTreeNode *n)
    {
        if (!n)
            n = _root;

        Q_ASSERT(n != 0);

        n->reset();

        for (QListIterator<LabTreeNode *> i(n->children()); i.hasNext(); )
            reset(i.next());
    }

    void LabTree::update(LabTreeNode *n)
    {
        if (!n)
            n = _root;

        Q_ASSERT(n != 0);

        for (QListIterator<LabTreeNode *> i(n->children()); i.hasNext(); )
            update(i.next());

        n->update();
    }

    void LabTree::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        Q_ASSERT(_root);

        ds << _current->id();
        _root->writeBinary(ds, file_version);
    }

    static LabTreeNode *find_current(LabTreeNode *n, const quint32 & id)
    {
        if (n->id() == id)
            return n;

        for (QListIterator<LabTreeNode *> i(n->children()); i.hasNext(); )
        {
            LabTreeNode *c = find_current(i.next(), id);
            if (c)
                return c;
        }

        return 0;
    }

    void LabTree::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_OLD)
        {
            // current node
            quint32 current_id;
            ds >> current_id;

            // root node
            LabTreeNode *n = new LabTreeNode(this, 0);
            _root = _current = n;

            n->readBinary(ds, file_version);

            // find current node
            LabTreeNode *cur = find_current(_root, current_id);
            if (cur)
                _current = cur;
        }
        else
        {
            throw new Automata::FileFormatError();
        }
    }

} // namespace NeuroLab
