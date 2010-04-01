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
        data << static_cast<qint32>(node._view->horizontalScrollBar() ? node._view->horizontalScrollBar()->sliderPosition() : 0);
        data << static_cast<qint32>(node._view->verticalScrollBar() ? node._view->verticalScrollBar()->sliderPosition() : 0);

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
        qint32 horiz, vert;

        data >> matrix;
        data >> horiz;
        data >> vert;

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
        if (node._view->horizontalScrollBar())
            node._view->horizontalScrollBar()->setSliderPosition(horiz);
        if (node._view->verticalScrollBar())
            node._view->verticalScrollBar()->setSliderPosition(vert);

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

    void LabTree::reset(LabTreeNode *n)
    {
        if (!n)
            n = _root;

        n->reset();

        for (QListIterator<LabTreeNode *> i(n->_children); i.hasNext(); i.next())
            reset(i.peekNext());
    }

    void LabTree::update(LabTreeNode *n)
    {
        if (!n)
            n = _root;

        for (QListIterator<LabTreeNode *> i(n->_children); i.hasNext(); i.next())
            update(i.peekNext());

        n->update();
    }

    QDataStream & operator<< (QDataStream & data, const LabTree & ctree)
    {
        LabTree & tree = const_cast<LabTree &>(ctree);
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
        return data;
    }

} // namespace NeuroLab
