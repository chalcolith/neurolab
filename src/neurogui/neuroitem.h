#ifndef NEUROITEM_H
#define NEUROITEM_H

#include "neurogui_global.h"
#include "propertyobj.h"

#include <QGraphicsItem>
#include <QPainterPath>
#include <QColor>
#include <QList>
#include <QMap>
#include <QPair>
#include <typeinfo>

class QtProperty;
class QtVariantProperty;
class QtVariantPropertyManager;

namespace NeuroLab
{

    class LabNetwork;
    class LabScene;

    /// Base class for items that can be displayed in a NeuroLab scene.
    class NEUROGUISHARED_EXPORT NeuroItem
        : public QObject, public QGraphicsItem, public PropertyObject
    {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)

    protected:
        typedef qint64 IdType;

        /// Used to store strings that should be drawn by the item.
        struct TextPathRec
        {
            QPointF pos; QString text;
            TextPathRec(const QPointF & pos, const QString & text) : pos(pos), text(text) {}
        };

    private:
        QtVariantProperty *label_property;

        LabNetwork *_network; ///< The network this item is a part of.

        IdType _id; ///< Used to remember links when saving and loading.
        QList<NeuroItem *> _incoming; ///< Incoming links.
        QList<NeuroItem *> _outgoing; ///< Outgoing links.

        mutable QPainterPath _drawPath; ///< Painter path used to draw the item.
        mutable QPainterPath _shapePath; ///< Painter path used for collision detection.

        mutable QList<TextPathRec> _texts; ///< Holds the strings to be drawn by the item.
        QString _label; ///< The item's label.

        static IdType NEXT_ID;

    public:
        static const QColor NORMAL_LINE_COLOR;
        static const QColor UNLINKED_LINE_COLOR;
        static const QColor BACKGROUND_COLOR;
        static const QColor ACTIVE_COLOR;

        static const int NORMAL_LINE_WIDTH;
        static const int HOVER_LINE_WIDTH;

        static const int NODE_WIDTH;
        static const int ELLIPSE_WIDTH;

        /// Constructor.
        NeuroItem(LabNetwork *network);
        virtual ~NeuroItem();

        const LabNetwork *network() const { return _network; }
        LabNetwork *network() { return _network; }

        /// The label is drawn to the right of the item's scene position.
        /// \return The item's label.
        const QString & label() const { return _label; }

        /// Set the item's label.
        void setLabel(const QString & s) { _label = s; updateShape(); update(); }

        /// \return The node's ID, for use when saving and loading.
        int id() { return _id; }

        /// \return Incoming links.
        const QList<NeuroItem *> & incoming() const { return _incoming; }

        /// \return Outgoing links.
        const QList<NeuroItem *> & outgoing() const { return _outgoing; }

        /// Creates an item of the correct type, given an RTTI type name that has been registered.
        static NeuroItem *create(const QString & typeName, LabScene *scene, const QPointF & pos);

        /// Derived classes should override this to add properties to the parent item.
        /// It is recommended to call the base class version first to add the label property.
        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);

        /// Derived classes should override this to update their properties from their data.
        /// It is recommended to call the base class version from the overridden one in order to update the label property.
        virtual void updateProperties();

        /// Add an incoming link.  Derived classes may override this to provide custom behavior.
        /// \see NeuroNarrowItem::addIncoming()
        virtual bool addIncoming(NeuroItem *linkItem);

        /// Remove an incoming link.
        virtual bool removeIncoming(NeuroItem *linkItem);

        /// Add an outgoing link.
        virtual bool addOutgoing(NeuroItem *linkItem);

        /// Remove an outgoing link.
        virtual bool removeOutgoing(NeuroItem *linkItem);

        /// Used to decide when a moving item can attach to another item it collides with.
        virtual bool canAttachTo(const QPointF &, NeuroItem *);

        /// Used to decide if a stationary item can have a moving item attach to it.
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);

        /// Called when a moving item is attached to a stationary one.
        virtual void attachTo(NeuroItem *) { }

        /// Called when a stationary item has been attached to by a moving one.
        virtual void onAttachedBy(NeuroItem *) { }

        /// Called when an item moves, so it can adjust the position or shape of any incoming or outgoing items.
        virtual void adjustLinks() { }

        /// Called when an item moves.
        virtual bool handleMove(const QPointF & mousePos, QPointF & movePos);

        /// Used to translate ids of incoming and outgoing items (which are used when saving an item) to valid memory pointers.
        virtual void idsToPointers(QGraphicsScene *);

        /// Updates the item's drawing and collision painter paths.
        /// Calls addToShape(), which when overridden should add text records to be drawn.
        void updateShape() const;

        /// Returns the bounding rectangle of the item's collision painter path.
        virtual QRectF boundingRect() const;

        /// Returns the item's collision painter path.
        virtual QPainterPath shape() const;

        /// Paints the item and draws any text records associated with it.
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        /// Brings the item to the front of the z-order.
        void bringToFront();

        /// Adds actions to the context menu for adding new item types that are registered.
        static void buildNewMenu(LabScene *scene, NeuroItem *item, const QPointF & pos, QMenu & menu);

        /// Adds actions to the context menu depending on the current item.
        /// \see buildActionMenuAux()
        static void buildActionMenu(LabScene *scene, NeuroItem *item, const QPointF & pos, QMenu & menu);

        /// Function type for static item creators.
        typedef NeuroItem * (*CreateFT) (LabScene *scene, const QPointF & pos);

        static void registerItemCreator(const QString & typeName, const QString & description, CreateFT createFunc);
        static void removeItemCreator(const QString & typeName);

    public slots:
        /// Can be overridden to handle changes made in the properties in the property widget.
        /// The base class version should be called in order to properly modify the item's label.
        virtual void propertyValueChanged(QtProperty *, const QVariant &);

    protected:
        /// Should be overridden to add to the drawing painter path.
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;

        /// Should be overridden if the item needs special properties for drawing.
        virtual void setPenProperties(QPen & pen) const;

        /// Should be overridden if the item needs special properties for drawing.
        virtual void setBrushProperties(QBrush & brush) const;

        /// Whether or not to highlight the item.
        /// The default implementation highlights an item if it is selected or if the mouse is over it.
        virtual bool shouldHighlight() const;

        /// Used to track whether or not the mouse is over an item.
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        /// Used to track whether or not the mouse is over an item.
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

        /// Used to handle movement.
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

        /// Writes the item's data to a data stream.  Derived classes should call the base class version to correctly save item data.
        virtual void writeBinary(QDataStream &) const;

        /// Reads the item's data from a data stream.  Derived classes should call the base class version to correctly read item data.
        virtual void readBinary(QDataStream &);

        /// Writes the ids of incoming and outgoing items.
        virtual void writePointerIds(QDataStream &) const;

        /// Reads the ids of incoming and outgoing items.
        virtual void readPointerIds(QDataStream &);

        /// Handles transforming ids to actual pointers.
        virtual void idsToPointersAux(QList<NeuroItem *> & list, QGraphicsScene *sc);

        /// Linear interpolation for colors.
        /// \return A color that is \c t of the way between \c a and \c b.
        static QColor lerp(const QColor & a, const QColor & b, const qreal & t);

        //////////////////////////////////////////////////////////////

        /// Registers item creator functions.
        static QMap<QString, QPair<QString, CreateFT> > _itemCreators;

        /// Can be overridden to add actions to the context menu.
        virtual void buildActionMenuAux(LabScene *, const QPointF &, QMenu &) { }

        /// Can be overridden to control whether or not a new item with a given type name can be
        /// created on top of an already selected item.
        virtual bool canCreateNewOnMe(const QString &, const QPointF &) const { return true; }

        //////////////////////////////////////////////////////////////

        friend QDataStream & operator<< (QDataStream &, const NeuroItem &);
        friend QDataStream & operator>> (QDataStream &, NeuroItem &);
    };

    //

    extern NEUROGUISHARED_EXPORT QDataStream & operator<< (QDataStream &, const NeuroItem &);
    extern NEUROGUISHARED_EXPORT QDataStream & operator>> (QDataStream &, NeuroItem &);

    //

    /// A helper class for registering item types.
    /// Use the macro \ref NEUROITEM_DEFINE_CREATOR to define an item creator, and then implement
    /// a static method called \c create_new() in your class.
    class NEUROGUISHARED_EXPORT NeuroItemRegistrator
    {
        QString _name;

    public:
        NeuroItemRegistrator(const QString & name, const QString & description, NeuroItem::CreateFT create_func)
            : _name(name)
        {
            NeuroItem::registerItemCreator(name, description, create_func);
        }

        virtual ~NeuroItemRegistrator()
        {
            NeuroItem::removeItemCreator(_name);
        }
    };

    /// Use this macro in the source file for a class derived from \ref NeuroItem in order to have it show up in the context menu.
#define NEUROITEM_DEFINE_CREATOR(TypeName, Description) static NeuroItemRegistrator TypeName ## _static_registrator(typeid(TypeName).name(), Description, &TypeName::create_new)

} // namespace NeuroLab

#endif // NEUROITEM_H
