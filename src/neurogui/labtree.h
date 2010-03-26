#ifndef LABTREE_H
#define LABTREE_H

#include "neurogui_global.h"

#include <QList>

class QDataStream;
class QWidget;

namespace NeuroLab
{

    class LabScene;
    class LabView;
    class LabTree;
    class LabNetwork;

    /// A node in the hierarchy of scenes.
    class NEUROGUISHARED_EXPORT LabTreeNode
    {
        int _id;
        static int NEXT_ID;

        LabTree *_tree;
        LabTreeNode *_parent;

        LabScene *_scene;
        LabView *_view;

        QList<LabTreeNode *> _children;

    public:
        /// Constructor.
        LabTreeNode(LabTree *tree, LabTreeNode *parent = 0);
        LabTreeNode(LabScene *scene, LabView *view, LabTree *tree, LabTreeNode *parent = 0);
        virtual ~LabTreeNode();

        LabScene *scene() { return _scene; }
        LabView *view() { return _view; }
        QList<LabTreeNode *> & children() { return _children; }

        /// Resets all the items in the scene.
        void reset();

        /// Updates all the items in the scene.
        void update();

        friend class LabTree;
        friend QDataStream & operator<< (QDataStream &, const LabTreeNode &);
        friend QDataStream & operator>> (QDataStream &, LabTreeNode &);
        friend QDataStream & operator<< (QDataStream &, const LabTree &);
        friend QDataStream & operator>> (QDataStream &, LabTree &);
    };

    extern NEUROGUISHARED_EXPORT QDataStream & operator<< (QDataStream &, const LabTreeNode &);
    extern NEUROGUISHARED_EXPORT QDataStream & operator>> (QDataStream &, LabTreeNode &);

    /// Encapsulates the hierarchy of scenes.  A NeuroItem may contain a subnetwork within it, whose scene can be opened
    /// by double-clicking.  The lab tree encapsulates this.
    class NEUROGUISHARED_EXPORT LabTree
    {
        QWidget *_parent;

        LabNetwork *_network;
        LabTreeNode *_root;
        LabTreeNode *_current;

        friend class LabTreeNode;

    public:
        /// Constructor.
        LabTree(QWidget *_parent, LabNetwork *_network);
        virtual ~LabTree();

        /// \return The tree's network object.
        LabNetwork *network() { return _network; }

        /// \return The root node of the scene tree.
        LabTreeNode *root() { return _root; }

        /// \return The current node whose scene is being displayed.
        LabTreeNode *current() { return _current; }
        void setCurrent(LabTreeNode *node) { _current = node; }

        /// \return The current node's scene.
        LabScene *scene() { return _current ? _current->scene() : 0; }

        /// \return The current node's scene's view.
        LabView *view() { return _current ? _current->view() : 0; }

        /// Resets all the items in the network.
        void reset(LabTreeNode *n = 0);

        /// Updates all the items in the network.
        void update(LabTreeNode *n = 0);

        static const int SCENE_WIDTH;
        static const int SCENE_HEIGHT;

        friend QDataStream & operator<< (QDataStream &, const LabTree &);
        friend QDataStream & operator>> (QDataStream &, LabTree &);
    };

    extern NEUROGUISHARED_EXPORT QDataStream & operator<< (QDataStream &, const LabTree &);
    extern NEUROGUISHARED_EXPORT QDataStream & operator>> (QDataStream &, LabTree &);

} // namespace NeuroLab

#endif // LABTREE_H
