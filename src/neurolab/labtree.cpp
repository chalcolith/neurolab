#include "labtree.h"
#include "neuroitem.h"
#include "neurolinkitem.h"
#include "neuronodeitem.h"
#include "mainwindow.h"

namespace NeuroLab
{
    
    // LabTreeNode
    int LabTreeNode::NEXT_ID = 0;
    
    LabTreeNode::LabTreeNode(LabTree *tree, LabTreeNode *parent)
        : _id(NEXT_ID++), tree(tree), parent(parent)
    {
        scene = QSharedPointer<LabScene>(new LabScene());
        scene->setSceneRect(0, 0, 1000000, 1000000);
        
        view = QSharedPointer<LabView>(new LabView(scene.data(), tree ? tree->parent : 0));
        
        view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    }
    
    LabTreeNode::LabTreeNode(LabScene *scene, LabView *view, LabTree *tree, LabTreeNode *parent)
        : _id(NEXT_ID++), tree(tree), parent(parent), scene(scene), view(view)
    {
    }
    
    LabTreeNode::~LabTreeNode()
    {
        tree = 0;
        parent = 0;
        
        QListIterator<LabTreeNode *> i(children);
        while (i.hasNext())
        {
            delete i.next();
        }
        children.clear();
    }
    
    static int get_item_type(NeuroItem const * const item)
    {
        if (dynamic_cast<NeuroNodeItem const *>(item))
            return NeuroItem::NEURO_NODE;
        if (dynamic_cast<NeuroExcitoryLinkItem const *>(item))
            return NeuroItem::NEURO_EXCITORY_LINK;
        if (dynamic_cast<NeuroInhibitoryLinkItem const *>(item))
            return NeuroItem::NEURO_INHIBITORY_LINK;
        return NeuroItem::NEURO_NULL;
    }
    
    static NeuroItem * get_new_item(int type)
    {
        switch (type)
        {
        case NeuroItem::NEURO_NODE:
            return new NeuroNodeItem();
        case NeuroItem::NEURO_EXCITORY_LINK:
            return new NeuroExcitoryLinkItem();
        case NeuroItem::NEURO_INHIBITORY_LINK:
            return new NeuroInhibitoryLinkItem();
        }
        
        return 0;
    }
    
    QDataStream & operator<< (QDataStream & data, const LabTreeNode & node)
    {
        data << node._id;
        
        // graphics items in this scene
        QList<QGraphicsItem *> items = node.scene->items();
        data << items.size();
        
        for (int i = 0; i < items.size(); ++i)
        {
            NeuroItem const * const item = dynamic_cast<NeuroItem const *>(items[i]);
            if (!item)
                continue;
            
            data << get_item_type(item);
            data << *item;
        }
        
        // children
        data << node.children.size();
        
        for (int i = 0; i < node.children.size(); ++i)
        {
            data << *node.children[i];
        }
        
        //
        return data;
    }
    
    QDataStream & operator>> (QDataStream & data, LabTreeNode & node)
    {
        data >> node._id;
        
        // graphics items in this scene
        int num_items;
        data >> num_items;
        
        for (int i = 0; i < num_items; ++i)
        {
            int type;
            data >> type;
            
            NeuroItem *new_item = get_new_item(type);
            if (new_item)
            {
                data >> *new_item;
                node.scene->addItem(new_item);
            }
            else
            {
                throw new Exception(QObject::tr("Error loading; unknown type %1").arg(type));
            }
        }
        
        // children
        int num_children;
        data >> num_children;
        
        for (int i = 0; i < num_children; ++i)
        {
            LabTreeNode *child = new LabTreeNode(node.tree, &node);
            data >> *child;
        }
        
        //
        return data;
    }
    
    // LabTree
    
    LabTree::LabTree(QWidget *parent)
        : parent(parent), root(new LabTreeNode(this, 0))
    {
        current = root;
    }
    
    LabTree::~LabTree()
    {
        parent = 0;
        delete root;
        current = 0;
    }
    
    QDataStream & operator<< (QDataStream & data, const LabTree & ctree)
    {
        LabTree & tree = const_cast<LabTree &>(ctree);
        data << tree.getCurrent()->_id;
        data << *tree.getRoot();
        return data;
    }
    
    QDataStream & operator>> (QDataStream & data, LabTree & tree)
    {
        int current_id;
        data >> current_id;
        
        LabTreeNode *node = new LabTreeNode(&tree, 0);
        tree.root = tree.current = node;
        
        data >> *node;
        return data;
    }
    
} // namespace NeuroLab
