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
    
    /// A node in the hierarchy of scenes.
    class LabTreeNode
    {
        int _id;
        static int NEXT_ID;
                
        LabTree *tree;
        LabTreeNode *parent;
        
        QSharedPointer<LabScene> scene;
        QSharedPointer<LabView> view;
                
        QList<LabTreeNode *> children;
        
    public:
        LabTreeNode(LabTree *tree, LabTreeNode *parent = 0);
        LabTreeNode(LabScene *scene, LabView *view, LabTree *tree, LabTreeNode *parent = 0);        
        virtual ~LabTreeNode();
        
        LabScene *getScene() { return scene.data(); }
        LabView *getView() { return view.data(); }
        QList<LabTreeNode *> & getChildren() { return children; }
        
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
        QWidget *parent;
        LabTreeNode *root;
        LabTreeNode *current;
        
        friend class LabTreeNode;
        
    public:
        LabTree(QWidget *parent);
        virtual ~LabTree();
        
        LabTreeNode *getRoot() { return root; }
        LabTreeNode *getCurrent() { return current; }
        void setCurrent(LabTreeNode *node) { current = node; }
        
        LabScene *getScene() { return current ? current->getScene() : 0; }
        LabView *getView() { return current ? current->getView() : 0; }
        
        friend QDataStream & operator<< (QDataStream &, const LabTree &);
        friend QDataStream & operator>> (QDataStream &, LabTree &);
    };
    
    extern QDataStream & operator<< (QDataStream &, const LabTree &);
    extern QDataStream & operator>> (QDataStream &, LabTree &);

} // namespace NeuroLab

#endif // LABTREE_H
