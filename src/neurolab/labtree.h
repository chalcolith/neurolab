#ifndef LABTREE_H
#define LABTREE_H

#include "labscene.h"
#include "labview.h"

#include <QList>
#include <QSharedPointer>

namespace NeuroLab
{
    
    class LabTree;
    
    /// A node in the hierarchy of scenes.
    class LabTreeNode
    {
        LabTree *tree;
        LabTreeNode *parent;
        
        QSharedPointer<LabScene> scene;
        QSharedPointer<LabView> view;
                
        QList<LabTreeNode> children;
        
    public:
        LabTreeNode(LabTree *tree, LabTreeNode *parent = 0);
        LabTreeNode(LabScene *scene, LabView *view, LabTree *tree, LabTreeNode *parent = 0);
        LabTreeNode(const LabTreeNode & node);
        LabTreeNode & operator= (const LabTreeNode & node);
        
        virtual ~LabTreeNode();
        
        LabScene *getScene() { return scene.data(); }
        LabView *getView() { return view.data(); }
        QList<LabTreeNode> & getChildren() { return children; }
    };
    
    /// Encapsulate the hierarchy of scenes.
    class LabTree
    {
        QWidget *parent;
        LabTreeNode root;
        LabTreeNode *current;
        
        friend class LabTreeNode;
        
    public:
        LabTree(QWidget *parent);
        virtual ~LabTree();
        
        LabTreeNode & getRoot() { return root; }
        LabTreeNode & getCurrent() { return *current; }
        void setCurrent(LabTreeNode & node) { current = &node; }
        
        LabScene *getScene() { return current ? current->getScene() : 0; }
        LabView *getView() { return current ? current->getView() : 0; }
    };
    
} // namespace NeuroLab

#endif // LABTREE_H
