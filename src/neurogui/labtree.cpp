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
#include "labexception.h"
#include "labview.h"
#include "labscene.h"
#include "labnetwork.h"

#include <Qaction>
#include <QLayout>
#include <typeinfo>

namespace NeuroGui
{

    // LabTreeNode

    LabTreeNode::LabTreeNode(LabTree *_tree, LabTreeNode *_parent)
        : _id(_tree->NEXT_ID++), _tree(_tree), _parent(_parent), _scene(0), _view(0), _currentAction(0), _ui_delete(false)
    {
        _scene = new LabScene(_tree->network());
        _view = new LabView(_scene, _tree ? _tree->_parent : 0);
    }

    LabTreeNode::LabTreeNode(LabScene *_scene, LabView *_view, LabTree *_tree, LabTreeNode *_parent)
        : _id(_tree->NEXT_ID++), _tree(_tree), _parent(_parent), _scene(_scene), _view(_view), _currentAction(0)
    {
    }

    LabTreeNode::~LabTreeNode()
    {
        if (_parent)
        {
            _parent->_children.removeAll(this);
        }

        QList<LabTreeNode *> childrenCopy = _children; // deleting the children will remove them from our list
        foreach (LabTreeNode *node, childrenCopy)
        {
            delete node;
        }
        _children.clear();

        if (_ui_delete && _scene)
        {
            foreach (QGraphicsItem *gi, _scene->items())
            {
                NeuroItem *ni = dynamic_cast<NeuroItem *>(gi);
                ni->setUIDelete(_ui_delete);
            }
        }

        delete _currentAction;
        _currentAction = 0;

        delete _view;
        _view = 0;

        delete _scene;
        _scene = 0;

        _tree = 0;
        _parent = 0;
    }

    QString LabTreeNode::label() const
    {
        if (!_label.isEmpty() && !_label.isNull())
            return _label;
        return tr("Subnetwork %1").arg(_id);
    }

    void LabTreeNode::setLabel(const QString & s)
    {
        _label = s;
        if (_currentAction)
            _currentAction->setText(s);
        emit labelChanged(s);
    }

    void LabTreeNode::setCurrentAction(QAction *action)
    {
        if (_currentAction)
        {
            disconnect(_currentAction, SIGNAL(destroyed()), this, SLOT(actionDestroyed()));
            disconnect(_currentAction, SIGNAL(triggered(bool)), this, SLOT(actionTriggered(bool)));
            delete _currentAction;
        }

        _currentAction = action;
        if (_currentAction)
        {
            connect(_currentAction, SIGNAL(destroyed()), this, SLOT(actionDestroyed()));
            connect(_currentAction, SIGNAL(triggered(bool)), this, SLOT(actionTriggered(bool)));
        }
    }

    void LabTreeNode::actionDestroyed(QObject *)
    {
        _currentAction = 0;
    }

    void LabTreeNode::actionTriggered(bool checked)
    {
        if (!checked && _currentAction && _tree->current() == this)
        {
            _currentAction->setChecked(true);
        }
        else
        {
            emit nodeSelected(this);
        }
    }

    LabTreeNode *LabTreeNode::createChild(const QString &label)
    {
        LabTreeNode *child = new LabTreeNode(_tree, this);
        _children.append(child);
        return child;
    }

    void LabTreeNode::reset()
    {
        foreach (QGraphicsItem *gi, _scene->items())
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(gi);
            if (item)
                item->reset();
        }
    }

    void LabTreeNode::updateItemProperties()
    {
        if (_view)
        {
            _view->updateItemProperties();
            _view->update();
        }
    }

    void LabTreeNode::removeWidgetsFrom(QLayout *w)
    {
        foreach (LabTreeNode *child, _children)
        {
            child->removeWidgetsFrom(w);
        }

        if (_view)
            w->removeWidget(_view);
    }

    void LabTreeNode::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        Q_ASSERT(_scene);
        Q_ASSERT(_view);

        ds << _id;
        ds << _label;

        // view
        _view->writeBinary(ds, file_version);

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

    void LabTreeNode::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        // if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_OLD)
        {
            // id
            ds >> _id;

            if (_id >= _tree->NEXT_ID)
                _tree->NEXT_ID = _id + 1;

            // label
            if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_3)
            {
                ds >> _label;
            }

            // view matrix
            if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_2)
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

                    _tree->network()->idMap()[new_item->id()] = new_item;
                    _scene->addItem(new_item);
                }
                else
                {
                    throw LabException(QObject::tr("Error loading; unknown type %1").arg(type));
                }
            }

            // children
            quint32 num_children;
            ds >> num_children;

            for (quint32 i = 0; i < num_children; ++i)
            {
                LabTreeNode *child = new LabTreeNode(_tree, this);
                if (child)
                {
                    _children.append(child);
                    child->readBinary(ds, file_version);
                }
            }

            // update scene and view
            _scene->update();
        }
    }

    /// Handles things that need to be done after the whole tree is loaded.
    void LabTreeNode::postLoad()
    {
        // do this depth-first, since subnetwork items, for instance, need to have id maps for children
        foreach (LabTreeNode *node, _children)
            node->postLoad();

        // turn ids into pointers
        foreach (QGraphicsItem *gi, _scene->items())
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(gi);
            if (item)
                item->idsToPointers(_tree->network()->idMap());
        }

        // do further processing if necessary
        foreach (QGraphicsItem *gi, _scene->items())
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(gi);
            if (item)
                item->postLoad();
        }

        // build shapes for first draw
        foreach (QGraphicsItem *gi, _scene->items())
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(gi);
            if (item)
                item->updateShape();
        }
    }


    //////////////////////////////////////////////////////////////////
    // LabTree

    LabTree::LabTree(QWidget *_parent, LabNetwork *_network)
        : QObject(_parent), NEXT_ID(0), _parent(_parent), _network(_network), _root(0), _current(0)
    {
        _root = new LabTreeNode(this, 0);
        _current = _root;

        _root->setLabel(tr("Top-Level Network"));
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

        foreach (LabTreeNode *child, n->children())
            results.append(items(child));

        return results;
    }

    void LabTree::reset(LabTreeNode *n)
    {
        if (!n)
            n = _root;

        Q_ASSERT(n != 0);

        n->reset();

        foreach (LabTreeNode *child, n->children())
            reset(child);
    }

    void LabTree::updateItemProperties(LabTreeNode *n, bool all)
    {
        if (!n)
            n = all ? _root : _current;

        Q_ASSERT(n != 0);

        if (all)
        {
            foreach (LabTreeNode *child, n->children())
                updateItemProperties(child, all);
        }

        n->updateItemProperties();
    }

    static LabTreeNode *find_current(LabTreeNode *n, const quint32 & id)
    {
        if (n->id() == id)
            return n;

        foreach (LabTreeNode *child, n->children())
        {
            LabTreeNode *c = find_current(child, id);
            if (c)
                return c;
        }

        return 0;
    }

    LabTreeNode *LabTree::findSubNetwork(const quint32 & id)
    {
        return find_current(_root, id);
    }

    LabTreeNode *LabTree::newSubNetwork()
    {
        return _current ? _current->createChild() : 0;
    }

    void LabTree::removeWidgetsFrom(QLayout *w)
    {
        Q_ASSERT(_root);
        _root->removeWidgetsFrom(w);
    }

    void LabTree::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        Q_ASSERT(_root);
        Q_ASSERT(_current);

        ds << _current->id();
        _root->writeBinary(ds, file_version);
    }

    void LabTree::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        // if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_OLD)
        {
            // current node
            quint32 current_id;
            ds >> current_id;

            // root node
            LabTreeNode *n = new LabTreeNode(this, 0);
            _root = _current = n;

            n->readBinary(ds, file_version);
            n->postLoad();

            // find current node
            LabTreeNode *cur = find_current(_root, current_id);
            if (cur)
                _current = cur;
        }
    }

} // namespace NeuroGui
