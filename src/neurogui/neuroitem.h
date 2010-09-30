#ifndef NEUROITEM_H
#define NEUROITEM_H

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

#include "neurogui_global.h"
#include "propertyobj.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>
#include <QColor>
#include <QList>
#include <QMap>
#include <QPair>
#include <typeinfo>

namespace NeuroGui
{

    class LabNetwork;
    class LabScene;

    /// Base class for items that can be displayed in a NeuroLab scene.
    class NEUROGUISHARED_EXPORT NeuroItem
        : public PropertyObject, public QGraphicsItem
    {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)

    public:
        typedef qint32 IdType;

    protected:
        /// Used to store strings that should be drawn by the item.
        struct TextPathRec
        {
            QPointF pos;
            QString text;
            QFont font;
            QPen pen;

            TextPathRec(const QPointF & pos, const QString & text, const QFont & font = QApplication::font(), const QPen & pen = QPen())
                : pos(pos), text(text), font(font), pen(pen) {}
        };

        Property<NeuroItem, QVariant::String, QString, QString> _label_property;

    private:
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

        enum CreateContext
        {
            CREATE_NONE = 0,     ///< No create context
            CREATE_UI,           ///< Created by the UI; will generate a new network node in the automata network.
            CREATE_LOADFILE,     ///< Created by loading from a file; will not create a new node, as the nodes are already in the NNN file.
            NUM_CREATE_CONTEXTS
        };

        /// Constructor.
        explicit NeuroItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroItem();

        /// Creates an item of the correct type, given an RTTI type name that has been registered.
        static NeuroItem *create(const QString & typeName, LabScene *scene, const QPointF & pos, const CreateContext & context);

        const LabNetwork *network() const { return _network; }
        LabNetwork *network() { return _network; }

        /// The label is drawn to the right of the item's scene position.
        /// \return The item's label.
        QString label() const { return _label; }

        /// \return The node's ID, for use when saving and loading.
        int id() const { return _id; }

        /// \return Incoming links.
        const QList<NeuroItem *> & incoming() const { return _incoming; }

        /// \return Outgoing links.
        const QList<NeuroItem *> & outgoing() const { return _outgoing; }

        virtual void setChanged(bool changed = true);
        virtual void updateProperties();

        /// Used to write data values to the data file.
        virtual QString dataValue() const { return QString(); }

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
        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap);

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
        static void buildActionMenu(LabScene *scene, NeuroItem *item, const QPointF & pos, QMenu & menu);

        /// Writes the item's data to a data stream.  Derived classes should call the base class version to correctly save item data.
        virtual void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;

        /// Reads the item's data from a data stream.  Derived classes should call the base class version to correctly read item data.
        virtual void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);

        /// Whether or not the item can be cut and pasted.
        virtual bool canCutAndPaste() const { return true; }

        /// Writes the item's data to a clipboard data stream.
        /// \param ds The data stream.
        /// \param id_map A map from existing item ids to normalized ids in the clipboard.
        virtual void writeClipboard(QDataStream & ds, const QMap<int, int> & id_map) const;

        /// Reads the item's data from a clipboard data stream.
        /// \param ds The data stream.
        /// \param id_map A map from ids in the clipboard data to the new items that were created.
        virtual void readClipboard(QDataStream & ds, const QMap<int, NeuroItem *> & id_map);

        /// Writes the ids of incoming and outgoing items.
        virtual void writePointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version) const;

        /// Reads the ids of incoming and outgoing items.
        virtual void readPointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version);

        /// Called after all nodes have been loaded.
        virtual void postLoad() {}

        /// Allows mixins to call prepareGeometryChange().
        inline void prepGeomChange() { prepareGeometryChange(); }

        /// Gets the friendly type name of this item type (if it is a registered type).
        QString getTypeName() const;

        static QString getTypeName(const QString & mangledName);
        static void registerTypeName(const QString & mangledName, const QString & friendlyName);

        /// Function type for static item creators.
        typedef NeuroItem * (*CreateFT) (LabScene *scene, const QPointF & pos, const CreateContext & context);

        static void registerItemCreator(const QString & typeName, const QString & description, CreateFT createFunc);
        static void removeItemCreator(const QString & typeName);

    signals:
        void labelChanged(NeuroItem *item, const QString & newLabel);

    public slots:
        /// Set the item's label.
        void setLabel(const QString & s) { emit labelChanged(this, s); _label = s; updateShape(); update(); }

        virtual void reset() {}

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

        /// Handles transforming ids to actual pointers.
        virtual void idsToPointersAux(QList<NeuroItem *> & list, const QMap<NeuroItem::IdType, NeuroItem *> & idMap);

        /// Linear interpolation for colors.
        /// \return A color that is \c t of the way between \c a and \c b.
        static QColor lerp(const QColor & a, const QColor & b, const qreal & t);

        /// Can be overridden to add actions to the context menu.
        virtual void buildActionMenu(LabScene *, const QPointF &, QMenu &) { }

        /// Can be overridden to control whether or not a new item with a given type name can be
        /// created on top of an already selected item.
        virtual bool canCreateNewOnMe(const QString &, const QPointF &) const { return false; }

        //////////////////////////////////////////////////////////////

        /// Maps mangled names to friendly type names.
        static QMap<QString, QString> *_typeNames;

        /// Registers item creator functions.
        static QMap<QString, QPair<QString, CreateFT> > *_itemCreators;
    };


    /// A helper class for registering item types.
    /// Use the macro \ref NEUROITEM_DEFINE_CREATOR to define an item creator, and then implement
    /// a static method called \c create_new() in your class.
    class NEUROGUISHARED_EXPORT NeuroItemRegistrator
    {
        QString _typeName;
        QString _mangledName;

    public:
        NeuroItemRegistrator(const QString & typeName, const QString & mangledName, const QString & description, NeuroItem::CreateFT create_func)
            : _typeName(typeName), _mangledName(mangledName)
        {
            NeuroItem::registerTypeName(mangledName, typeName);
            NeuroItem::registerItemCreator(typeName, description, create_func);
            NeuroItem::registerItemCreator(mangledName, description, create_func); // this is for backwards compatibility with the old file format
        }

        virtual ~NeuroItemRegistrator()
        {
            NeuroItem::removeItemCreator(_typeName);
            NeuroItem::removeItemCreator(_mangledName);
        }
    };

    /// Use this macro in the header file for a class derived from \ref NeuroLab::NeuroItem in order to have it show up in the context menu.
    #define NEUROITEM_DECLARE_CREATOR \
    static NeuroGui::NeuroItem *_create_(NeuroGui::LabScene *scene, const QPointF & scenePos, const NeuroItem::CreateContext & context); \
    static NeuroGui::NeuroItemRegistrator _static_registrator;

    /// Use this macro in the source file for a class derived from \ref NeuroLab::NeuroItem in order to have it show up in the context menu.
    #define NEUROITEM_DEFINE_CREATOR(TypeName, Description) \
    NeuroGui::NeuroItemRegistrator TypeName::_static_registrator(#TypeName, typeid(TypeName).name(), Description, &TypeName::_create_); \
    NeuroGui::NeuroItem *TypeName::_create_(NeuroGui::LabScene *scene, const QPointF & scenePos, const NeuroItem::CreateContext & context) \
    { \
        if (!(scene && scene->network() && scene->network()->neuronet())) \
            return 0; \
        NeuroItem *item = new TypeName(scene->network(), scenePos, context); \
        return item; \
    }

} // namespace NeuroGui

#endif // NEUROITEM_H
