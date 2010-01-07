#ifndef LABTREE_H
#define LABTREE_H

#include "labscene.h"
#include "labview.h"

#include <QList>
#include <QSharedPointer>
#include <QDataStream>

namespace NeuroLab
{
    
    class LabTree;
    class LabNetwork;
    
    /// A node in the hierarchy of scenes.
    class LabTreeNode
    {
        int _id;
        static int NEXT_ID;
                
        LabTree *_tree;
        LabTreeNode *_parent;
        
        QSharedPointer<LabScene> _scene;
        QSharedPointer<LabView> _view;
                
        QList<LabTreeNode *> _children;
        
    public:
        LabTreeNode(LabTree *tree, LabTreeNode *parent = 0);
        LabTreeNode(LabScene *scene, LabView *view, LabTree *tree, LabTreeNode *parent = 0);        
        virtual ~LabTreeNode();
        
        LabScene *scene() { return _scene.data(); }
        LabView *view() { return _view.data(); }
        QList<LabTreeNode *> & children() { return _children; }
        
        friend QDataStream & operator<< (QDataStream &, const LabTreeNode &);
        friend QDataStream & operator>> (QDataStream &, LabTreeNode &);
        friend QDataStream & operator<< (QDataStream &, const LabTree &);
        friend QDataStream & operator>> (QDataStream &, LabTree &);
    };
    
    extern QDataStream & operator<< (QDataStream &, const LabTreeNode &);
    extern QDataStream & operator>> (QDataStream &, LabTreeNode &);
    
    /// Encapsulate the hierarchy of scenes.
    class LabTree
    {
        QWidget *_parent;

        LabNetwork *_network;
        LabTreeNode *_root;
        LabTreeNode *_current;
        
        friend class LabTreeNode;
        
    public:
        LabTree(QWidget *_parent, LabNetwork *_network);
        virtual ~LabTree();
        
        LabNetwork *network() { return _network; }

        LabTreeNode *root() { return _root; }
        LabTreeNode *current() { return _current; }
        void setCurrent(LabTreeNode *node) { _current = node; }
        
        LabScene *scene() { return _current ? _current->scene() : 0; }
        LabView *view() { return _current ? _current->view() : 0; }
        
        friend QDataStream & operator<< (QDataStream &, const LabTree &);
        friend QDataStream & operator>> (QDataStream &, LabTree &);
    };
    
    extern QDataStream & operator<< (QDataStream &, const LabTree &);
    extern QDataStream & operator>> (QDataStream &, LabTree &);

} // namespace NeuroLab

#endif // LABTREE_H
