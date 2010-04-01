#ifndef LABSCENE_H
#define LABSCENE_H

#include "neurogui_global.h"

#include <QGraphicsScene>
#include <QStack>

namespace NeuroLab
{

    class LabNetwork;
    class NeuroItem;

    /// A derived class from QGraphicsScene that handles displaying the neural network items.
    class NEUROGUISHARED_EXPORT LabScene
        : public QGraphicsScene
    {
        Q_OBJECT

        LabNetwork *_network;
        NeuroItem *_itemUnderMouse;

        QPointF _lastMousePos;

    public:
        /// Constructor.
        /// \param _network The LabNetwork to display.
        LabScene(LabNetwork *_network);
        virtual ~LabScene();

        /// \return The LabNetwork that this scene displays.
        LabNetwork *network() { return _network; }

        /// \return The position of the mouse (in scene coordinates).
        const QPointF & lastMousePos() const { return _lastMousePos; }

        /// \return A pointer to the topmose item (if any) currently under the mouse.
        NeuroItem *itemUnderMouse() const { return _itemUnderMouse; }

        /// Used by items to set themselves as being the item under the mouse.
        void setItemUnderMouse(NeuroItem *item) { _itemUnderMouse = item; }

    public slots:
        /// Creates a new item with the given type name.
        /// \param typeName The C++ type name of the item to create.
        /// \param scenePos The position (in scene coordinates) to position the newly-created item.
        /// \see NeuroItem
        void newItem(const QString & typeName, const QPointF & scenePos);

    signals:
        void itemCreated(NeuroItem *item);

    protected:
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    };

} // namespace NeuroLab

#endif // LABSCENE_H
