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
    int LabTreeNode::NEXT_ID = 1;

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
        for (QListIterator<LabTreeNode *> i(_children); i.hasNext(); i.next())
        {
            delete i.peekNext();
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
        for (QListIterator<QGraphicsItem *> i(_scene->items()); i.hasNext(); i.next())
        {
            NeuroNarrowItem *item = dynamic_cast<NeuroNarrowItem *>(i.peekNext());
            if (item)
                item->reset();
        }
    }

    void LabTreeNode::update()
    {
        _scene->update();
    }

    static QString get_item_type(NeuroItem const * const item)
    {
        return QString(typeid(*item).name());
    }

    QDataStream & operator<< (QDataStream & data, const LabTreeNode & node)
    {
        Q_ASSERT(node._scene);
        Q_ASSERT(node._view);

        data << node._id;
        data << node._view->matrix();

        // graphics items in this scene
        QList<QGraphicsItem *> items = node._scene->items();
        data << items.size();

        for (int i = 0; i < items.size(); ++i)
        {
            NeuroItem const * const item = dynamic_cast<NeuroItem const *>(items[i]);
            if (!item)
                continue;

            QString type = get_item_type(item);
            data << type;
            data << *item;
        }

        // children
        data << node._children.size();

        for (int i = 0; i < node._children.size(); ++i)
        {
            data << *node._children[i];
        }

        //
        return data;
    }

    QDataStream & operator>> (QDataStream & data, LabTreeNode & node)
    {
        data >> node._id;

        QMatrix matrix;

        data >> matrix;

        // graphics items in this scene
        int num_items;
        data >> num_items;

        for (int i = 0; i < num_items; ++i)
        {
            QString type;
            data >> type;

            NeuroItem *new_item = NeuroItem::create(type, node._scene, QPointF(0,0));
            if (new_item)
            {
                data >> *new_item;
                node._scene->addItem(new_item);
            }
            else
            {
                throw LabException(QObject::tr("Error loading; unknown type %1").arg(type));
            }
        }

        // turn ids into pointers
        for (QListIterator<QGraphicsItem *> i(node._scene->items()); i.hasNext(); )
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(i.next());
            if (item)
                item->idsToPointers(node._scene);
        }

        for (QListIterator<QGraphicsItem *> i(node._scene->items()); i.hasNext(); )
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(i.next());
            if (item)
                item->updateShape();
        }

        // children
        int num_children;
        data >> num_children;

        for (int i = 0; i < num_children; ++i)
        {
            LabTreeNode *child = new LabTreeNode(node._tree, &node);
            data >> *child;
        }

        // update scene and view
        node._scene->update();
        node._view->setMatrix(matrix);

        //
        return data;
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

        for (QListIterator<LabTreeNode *> i(n->_children); i.hasNext(); i.next())
            results.append(items(i.peekNext()));

        return results;
    }

    void LabTree::reset(LabTreeNode *n)
    {
        if (!n)
            n = _root;

        Q_ASSERT(n != 0);

        n->reset();

        for (QListIterator<LabTreeNode *> i(n->_children); i.hasNext(); i.next())
            reset(i.peekNext());
    }

    void LabTree::update(LabTreeNode *n)
    {
        if (!n)
            n = _root;

        Q_ASSERT(n != 0);

        for (QListIterator<LabTreeNode *> i(n->_children); i.hasNext(); i.next())
            update(i.peekNext());

        n->update();
    }

    QDataStream & operator<< (QDataStream & data, const LabTree & ctree)
    {
        LabTree & tree = const_cast<LabTree &>(ctree);
        Q_ASSERT(tree.root() != 0);

        data << tree.current()->_id;
        data << *tree.root();
        return data;
    }

    QDataStream & operator>> (QDataStream & data, LabTree & tree)
    {
        int current_id;
        data >> current_id;

        LabTreeNode *node = new LabTreeNode(&tree, 0);
        tree._root = tree._current = node;

        data >> *node;

        /// \todo Find current node in the tree.

        return data;
    }

} // namespace NeuroLab
