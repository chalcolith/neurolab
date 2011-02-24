#ifndef NEUROITEM_H
#define NEUROITEM_H

/*
Neurocognitive Linguistics Lab
Copyright (c) 2010,2011 Gordon Tisher
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
#include "labtree.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>
#include <QColor>
#include <QList>
#include <QMap>
#include <QSet>
#include <QPair>
#include <typeinfo>

namespace NeuroGui
{

    class LabNetwork;
    class LabScene;
    class LabTreeNodeController;

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

            TextPathRec(const QPointF & pos, const QString & text,
                        const QFont & font = QApplication::font(), const QPen & pen = QPen())
                : pos(pos), text(text), font(font), pen(pen) {}
        };

        Property<NeuroItem, QVariant::String, QString, QString> _label_property;

        bool _setting_pos; ///< Whether or not our position is being set.
        bool _ui_delete; ///< Whether or not an item is being deleted from the UI, or else by program shutdown.

    private:
        LabNetwork *_network; ///< The network this item is a part of.

        IdType _id; ///< Used to remember links when saving and loading.
        QSet<NeuroItem *> _connections;

        mutable QPainterPath _drawPath; ///< Painter path used to draw the item.
        mutable QPainterPath _shapePath; ///< Painter path used for collision detection.

        mutable QList<TextPathRec> _texts; ///< Holds the strings to be drawn by the item.
        QString _label; ///< The item's label.
        QPointF _label_pos;

        static IdType NEXT_ID;
        static qreal HIGHEST_Z_VALUE;

    public:
        static const QColor NORMAL_LINE_COLOR;
        static const QColor UNLINKED_LINE_COLOR;
        static const QColor BACKGROUND_COLOR;
        static const QColor ACTIVE_COLOR;

        static const int NORMAL_LINE_WIDTH;
        static const int HOVER_LINE_WIDTH;

        static const int NODE_WIDTH;
        static const int ELLIPSE_WIDTH;

        /// Context in which a new item object is created.
        enum CreateContext
        {
            CREATE_NONE = 0,     ///< No create context
            CREATE_UI,           ///< Created by the UI; will generate a new network node in the automata network.
            CREATE_LOADFILE,     ///< Created by loading from a file; will not create a new node, as the nodes are already in the NNN file.
            NUM_CREATE_CONTEXTS
        };

        /// Constructor.
        /// \param network The network that this item is a part of.
        /// \param scenePos The position in the current scene at which to create the item.
        /// \param context Context in which to create the item.
        explicit NeuroItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroItem();

        /// \name Object Properties
        //@{

        /// Set whether or not the item will be deleted from the UI, or as a result of program shutdown.
        void setUIDelete(bool ui_delete = true) { _ui_delete = ui_delete; }

        const LabNetwork *network() const { return _network; }
        LabNetwork *network() { return _network; }

        /// \return The node's ID, for use when saving and loading.
        int id() const { return _id; }

        /// The label is drawn to the right of the item's scene position.
        /// \return The item's label.
        QString label() const { return _label; }

        const QPointF & labelPos() const { return _label_pos; }
        void setLabelPos(const QPointF & pos) { _label_pos = pos; }

        /// The node's connections.
        const QSet<NeuroItem *> & connections() const { return _connections; }

        /// Used to write data values to the data file.
        virtual QString dataValue() const { return QString(); }

        /// Whether or not the item is bidirectional.
        virtual bool isBidirectional() const { return false; }

        //@}

        /// \name UI Properties
        //@{

        /// Updates the UI properties based on the values of the cell.
        /// \note Forces a redraw of the scene.
        virtual void updateProperties();

        //@}

        /// \name UI Interaction
        //@{

        /// Called when an item moves.
        virtual bool handleMove(const QPointF & mousePos, QPointF & movePos);

        /// Used to decide when a moving item can attach to another item it collides with.
        virtual bool canAttachTo(const QPointF &, NeuroItem *);

        /// Used to determine if an item can attach to another item more than once.
        virtual bool canAttachTwice(NeuroItem *) { return false; }

        /// Used to decide if a stationary item can have a moving item attach to it.
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);

        /// Returns true if the item collides with the given scene pos (with a margin of error the same as used to attach).
        bool containsScenePos(const QPointF & scenePos);

        /// Returns the target point for an attached link (in scene coordinates; usually the center of the item).
        virtual QPointF targetPointFor(const NeuroItem *) const { return scenePos(); }

        /// Called when a moving item is attached to a stationary one.
        virtual void onAttachTo(NeuroItem *item) { _connections.insert(item); }

        /// Called when a stationary item has been attached to by a moving one.
        virtual void onAttachedBy(NeuroItem *item) { _connections.insert(item); }

        /// Called when two nodes are detached.
        virtual void onDetach(NeuroItem *item) { _connections.remove(item); }

        /// Called when an item moves, so it can adjust the position or shape of any incoming or outgoing items.
        virtual void adjustLinks() { }

        //@}

        /// \name Mouse Interaction
        //@{

        /// Whether or not to highlight the item.
        /// The default implementation highlights an item if it is selected or if the mouse is over it.
        virtual bool shouldHighlight() const;

        /// Used to track whether or not the mouse is over an item.
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

        /// Used to track whether or not the mouse is over an item.
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

        /// Can be overridden to control whether or not a new item with a given type name can be
        /// created on top of an already selected item.
        virtual bool canCreateNewOnMe(const QString &, const QPointF &) const { return false; }

        /// Can be overridden to add actions to the context menu.
        virtual void buildActionMenu(LabScene *, const QPointF &, QMenu &) { }

        /// Adds actions to the context menu for adding new item types that are registered.
        static void buildNewMenu(LabScene *scene, NeuroItem *item, const QPointF & pos, QMenu & menu);

        /// Adds actions to the context menu depending on the current item.
        static void buildActionMenu(LabScene *scene, NeuroItem *item, const QPointF & pos, QMenu & menu);

        //@}

        /// \name Drawing and Collision
        //@{

        /// Returns the bounding rectangle of the item's collision painter path.  Should not normally be overwritten.
        virtual QRectF boundingRect() const;

        /// Returns the item's collision painter path.  Should not normally be overwritten.
        virtual QPainterPath shape() const;

        /// Paints the item and draws any text records associated with it.  Should not normally be overwritten.
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        /// Should be overridden to add to the drawing painter path.
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;

        /// Should be overridden if the item needs special properties for drawing.
        virtual void setPenProperties(QPen & pen) const;

        /// Should be overridden if the item needs special properties for drawing.
        virtual void setBrushProperties(QBrush & brush) const;

        /// Updates the item's drawing and collision painter paths.
        /// Calls addToShape(), which when overridden should add text records to be drawn.
        void updateShape() const;

        //@}

        /// \name Serialization
        //@{

        /// Writes the item's data to a data stream.  Derived classes should call the base class version to correctly save item data.
        virtual void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;

        /// Reads the item's data from a data stream.  Derived classes should call the base class version to correctly read item data.
        virtual void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);

        /// Whether or not the item can be cut and pasted.
        virtual bool canCutAndPaste() const { return false; }

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

        /// Used to translate ids of incoming and outgoing items (which are used when saving an item) to valid memory pointers.
        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap);

        /// Called after all nodes have been loaded.
        virtual void postLoad() {}

        //@}

        /// \name Item Registration and Creation
        //@{

        /// Gets the friendly type name of this item type (if it is a registered type).
        QString getTypeName() const;

        /// Gets a description of the type for UI purposes.
        virtual QString uiName() const;

        /// Gets a friendly type name from the mangled name.
        static QString getTypeName(const QString & mangledName);

        /// Registers a mangled/friendly typename pair.
        static void registerTypeName(const QString & mangledName, const QString & friendlyName,
                                     const QString & menuPath, const QString & uiName);

        /// Function type for static item creator objects.
        typedef NeuroItem * (*CreateFT) (LabScene *scene, const QPointF & pos, const CreateContext & context);

        /// Function type for static item restrictor objects.
        typedef bool (*CanCreateFT) (NeuroGui::LabTreeNodeController *tnc);

        /// Creates an item of the correct type, given a type name that has been registered.
        static NeuroItem *create(const QString & typeName, LabScene *scene, const QPointF & pos, const CreateContext & context);

        /// Registers an item type creator with the global item broker.
        static void registerItemCreator(const QString & typeName, const QString & menuPath,
                                        const QString & uiName, CreateFT createFunc, CanCreateFT restrictFunc);

        /// Unregisters an item type creator from the global item broker.
        static void removeItemCreator(const QString & typeName);

        //@}

        /// Allows mixins to call prepareGeometryChange().
        inline void prepGeomChange() { prepareGeometryChange(); }

    signals:
        /// Emitted when the item's label changes.
        void labelChanged(const QString & newLabel);

        /// Emitted when the item's label changes.
        void labelChanged(NeuroItem *item, const QString & newLabel);

    public slots:
        /// Sets the items state to changed.
        void setChanged(bool changed = true);

        /// Set the item's label.
        void setLabel(const QString & s) { _label = s; emit labelChanged(s); emit labelChanged(this, s); updateShape(); update(); }

        /// Brings the item to the front of the z-order.
        void bringToFront();

        /// Called for all items when the user clicks Reset.
        virtual void reset() {}

        /// Called before destruction.
        virtual void cleanup();

    protected:
        /// Used to handle movement.
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

        /// Linear interpolation for colors.
        /// \return A color that is \c t of the way between \c a and \c b.
        static QColor lerp(const QColor & a, const QColor & b, const qreal & t);

        /// \name Item Registration and Creation
        //@{

        struct TypeNameRec
        {
            QString mangledName;
            QString typeName;
            QString uiName;
            QString menuPath;

            TypeNameRec() {}
            TypeNameRec(const QString & mangledName, const QString & typeName, const QString & uiName, const QString & menuPath)
                : mangledName(mangledName), typeName(typeName), uiName(uiName), menuPath(menuPath) {}
        };

        /// Maps mangled names to friendly type names.
        static QMap<QString, TypeNameRec> *_typeNames;

        /// Registers item creator functions.
        static QMap<QString, QPair<QString, CreateFT> > *_itemCreators;

        /// Registers item restrictor functions.
        static QMap<QString, CanCreateFT> *_itemRestrictors;

        //@}
    };


    /// A helper class for registering item types.
    /// Use the macro \ref NEUROITEM_DEFINE_CREATOR to define an item creator, and then implement
    /// a static method called \c create_new() in your class.
    class NEUROGUISHARED_EXPORT NeuroItemRegistrator
    {
        QString _typeName;
        QString _mangledName;

    public:
        NeuroItemRegistrator(const QString & typeName, const QString & mangledName,
                             const QString & menuPath, const QString & uiName,
                             NeuroItem::CreateFT create_func, NeuroItem::CanCreateFT restrict_func)
            : _typeName(typeName), _mangledName(mangledName)
        {
            NeuroItem::registerTypeName(mangledName, typeName, menuPath, uiName);
            NeuroItem::registerItemCreator(typeName, menuPath, uiName, create_func, restrict_func);

//            // this is for backwards compatibility with the old file format
//            NeuroItem::registerItemCreator(mangledName, menuPath, uiName, create_func, restrict_func);
        }

        virtual ~NeuroItemRegistrator()
        {
            NeuroItem::removeItemCreator(_typeName);
            NeuroItem::removeItemCreator(_mangledName);
        }
    };

    /// Use this macro in the header file for a class derived from \ref NeuroGui::NeuroItem
    /// in order to have it show up in the context menu.
#define NEUROITEM_DECLARE_CREATOR \
    static NeuroGui::NeuroItemRegistrator _static_registrator; \
    static NeuroGui::NeuroItem *_create_(NeuroGui::LabScene *scene, const QPointF & scenePos, const NeuroItem::CreateContext & context); \
    static bool _can_create_(NeuroGui::LabTreeNodeController *tnc);

    /// Use this macro in a plugin source file for a class derived from \ref NeuroGui::NeuroItem
    /// in order to have it show up in the context menu.  It will also check that the version of the plugin is correct.
    /// You can restrict the item to be created only in subnetworks of the given tree node controller type.
#define NEUROITEM_DEFINE_RESTRICTED_PLUGIN_CREATOR(TypeName, MenuPath, UIName, NeuroLabVersion, ControllerType) \
    NeuroGui::NeuroItemRegistrator TypeName::_static_registrator(#TypeName, typeid(TypeName).name(), MenuPath, UIName, &TypeName::_create_, &TypeName::_can_create_); \
    NeuroGui::NeuroItem *TypeName::_create_(NeuroGui::LabScene *scene, const QPointF & scenePos, const NeuroItem::CreateContext & context) \
    { \
        if (NeuroGui::VERSION != NeuroLabVersion) \
            throw NeuroGui::Exception(QObject::tr("The plugin that provides the %1 item is out of date.  It was built for NeuroLab version %2, and this is version %3").arg(UIName).arg(NeuroLabVersion).arg(NeuroGui::VERSION)); \
        if (!(scene && scene->network() && scene->network()->neuronet())) \
            return 0; \
        NeuroItem *item = new TypeName(scene->network(), scenePos, context); \
        return item; \
    } \
    bool TypeName::_can_create_(NeuroGui::LabTreeNodeController *tnc) \
    { \
        return dynamic_cast<ControllerType *>(tnc); \
    }

    /// Use this macro in a plugin source file for a class derived from \ref NeuroGui::NeuroItem
    /// in order to have it show up in the context menu.  It will also check that the version of the plugin is correct.
#define NEUROITEM_DEFINE_PLUGIN_CREATOR(TypeName, MenuPath, UIName, NeuroLabVersion) NEUROITEM_DEFINE_RESTRICTED_PLUGIN_CREATOR(TypeName, MenuPath, UIName, NeuroLabVersion, NeuroGui::LabTreeNodeController)

    /// Use this macro in the source file for a class derived from \ref NeuroGui::NeuroItem
    /// in order to have it show up in the context menu.
#define NEUROITEM_DEFINE_CREATOR(TypeName, MenuPath, UIName) NEUROITEM_DEFINE_PLUGIN_CREATOR(TypeName, MenuPath, UIName, NeuroGui::VERSION)

} // namespace NeuroGui

#endif // NEUROITEM_H
