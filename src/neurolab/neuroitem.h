#ifndef NEUROITEM_H
#define NEUROITEM_H

#include "propertyobj.h"

#include <QGraphicsItem>
#include <QPainterPath>
#include <QColor>
#include <QList>
#include <QMap>
#include <typeinfo>

class QtProperty;
class QtVariantProperty;
class QtVariantPropertyManager;

namespace NeuroLab
{

    class LabNetwork;
    class LabScene;

    class NeuroItem
        : public QObject, public QGraphicsItem, public PropertyObject
    {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)

        static int NEXT_ID;

        QtVariantProperty *label_property;

    protected:
        LabNetwork *_network;

        typedef qint64 IdType;

        IdType _id;
        QList<NeuroItem *> _incoming;
        QList<NeuroItem *> _outgoing;

        mutable QPainterPath *_path, *_textPath;

        struct TextPathRec
        {
            QPointF pos; QString text;

            TextPathRec(const QPointF & pos, const QString & text) : pos(pos), text(text) {}
        };

        QList<TextPathRec> _texts;
        QString _label;

    public:
        static const QColor NORMAL_LINE_COLOR;
        static const QColor UNLINKED_LINE_COLOR;
        static const QColor BACKGROUND_COLOR;
        static const QColor ACTIVE_COLOR;

        static const int NORMAL_LINE_WIDTH;
        static const int HOVER_LINE_WIDTH;

        static const int NODE_WIDTH;
        static const int ELLIPSE_WIDTH;

        NeuroItem(LabNetwork *network);
        virtual ~NeuroItem();

        const QString & label() const { return _label; }
        void setLabel(const QString & s) { _label = s; buildShape(); update(); }

        int id() { return _id; }

        const QList<NeuroItem *> incoming() const { return _incoming; }
        const QList<NeuroItem *> outgoing() const { return _outgoing; }

        static NeuroItem *create(const QString & typeName, LabScene *scene, const QPointF & pos);

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void updateProperties();

        virtual bool addIncoming(NeuroItem *linkItem);
        virtual bool removeIncoming(NeuroItem *linkItem);

        virtual bool addOutgoing(NeuroItem *linkItem);
        virtual bool removeOutgoing(NeuroItem *linkItem);

        virtual bool canAttachTo(const QPointF &, NeuroItem *);
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);

        virtual void attachTo(NeuroItem *) { }
        virtual void onAttachedBy(NeuroItem *) { }

        virtual void adjustLinks() { }
        virtual bool handleMove(const QPointF & mousePos, QPointF & movePos);

        virtual void idsToPointers(QGraphicsScene *);

        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;
        virtual void buildShape();

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        void bringToFront();

    public slots:
        virtual void propertyValueChanged(QtProperty *, const QVariant &);

    protected:
        void buildTextPath() const;

        virtual void setPenProperties(QPen & pen);
        virtual void setBrushProperties(QBrush & brush);

        virtual bool shouldHighlight() const;

        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);

        virtual void writePointerIds(QDataStream &) const;
        virtual void readPointerIds(QDataStream &);

        virtual void idsToPointersAux(QList<NeuroItem *> & list, QGraphicsScene *sc);

        static QColor lerp(const QColor & a, const QColor & b, const qreal & t);

        //
        typedef NeuroItem * (*CreateFT) (LabScene *scene, const QPointF & pos);
        static QMap<QString, CreateFT> itemCreators;
        friend class NeuroItemRegistrator;

        //
        friend QDataStream & operator<< (QDataStream &, const NeuroItem &);
        friend QDataStream & operator>> (QDataStream &, NeuroItem &);
    };

    //

    extern QDataStream & operator<< (QDataStream &, const NeuroItem &);
    extern QDataStream & operator>> (QDataStream &, NeuroItem &);

    //

    class NeuroItemRegistrator
    {
        QString _name;

    public:
        NeuroItemRegistrator(const char *name, NeuroItem::CreateFT create_func)
            : _name(name)
        {
            if (create_func)
                NeuroItem::itemCreators[_name] = create_func;
        }

        virtual ~NeuroItemRegistrator()
        {
            NeuroItem::itemCreators.remove(_name);
        }
    };

#define NEUROITEM_DEFINE_CREATOR(TypeName) static NeuroItemRegistrator TypeName ## _static_registrator(typeid(TypeName).name(), &TypeName::create_new)

} // namespace NeuroLab

#endif // NEUROITEM_H
