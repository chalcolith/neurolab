#include "labtree.h"

namespace NeuroLab
{
    
    // LabTreeNode
    
    LabTreeNode::LabTreeNode(LabTree *tree, LabTreeNode *parent)
        : tree(tree), parent(parent)
    {
        scene = QSharedPointer<LabScene>(new LabScene());
        scene->setSceneRect(0, 0, 1000000, 1000000);
        
        view = QSharedPointer<LabView>(new LabView(scene.data(), tree ? tree->parent : 0));
        
        view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    }
    
    LabTreeNode::LabTreeNode(LabScene *scene, LabView *view, LabTree *tree, LabTreeNode *parent)
        : tree(tree), parent(parent), scene(scene), view(view)
    {
    }
    
    LabTreeNode::LabTreeNode(const LabTreeNode & node)
        : tree(node.tree), parent(node.parent), scene(node.scene), view(node.view), children(node.children)
    {
    }
    
    LabTreeNode & LabTreeNode::operator = (const LabTreeNode & node)
    {
        tree = node.tree;
        parent = node.parent;
        scene = node.scene;
        view = node.view;
        children = node.children;
        
        return *this;
    }
    
    LabTreeNode::~LabTreeNode()
    {
        tree = 0;
        parent = 0;
    }
    
    // LabTree
    
    LabTree::LabTree(QWidget *parent)
        : parent(parent), root(this), current(&root)
    {
    }
    
    LabTree::~LabTree()
    {
        parent = 0;
        current = 0;
    }
    
} // namespace NeuroLab
