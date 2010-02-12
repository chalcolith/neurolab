#ifndef NEUROITEM_H
#define NEUROITEM_H

#include "../neurolib/neurocell.h"

#include <QGraphicsItem>
#include <QPainterPath>
#include <QColor>
#include <QList>
#include <QMap>
#include <typeinfo>

class QtProperty;
class QtVariantPropertyManager;

namespace NeuroLab
{

    class LabNetwork;
    class LabScene;

    class NeuroItem
        : public QObject, public QGraphicsItem
    {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)
        
        static int NEXT_ID;

    protected:
        LabNetwork *_network;

        typedef qint64 IdType;

        IdType _id;
        QList<NeuroItem *> _incoming;
        QList<NeuroItem *> _outgoing;

        QPainterPath *_path, *_textPath;
        QString _label;

        NeuroLib::NeuroCell::NeuroIndex _cellIndex;

    public:
        static const QColor NORMAL_LINE_COLOR;
        static const QColor UNLINKED_LINE_COLOR;
        static const QColor BACKGROUND_COLOR;
        static const QColor ACTIVE_COLOR;

        static const int NORMAL_LINE_WIDTH;
        static const int HOVER_LINE_WIDTH;

        static const int NODE_WIDTH;
        static const int ELLIPSE_WIDTH;

        NeuroItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex = -1);
        virtual ~NeuroItem();

        const QString & label() const { return _label; }
        void setLabel(const QString & s) { _label = s; buildShape(); update(); }

        int id() { return _id; }

        virtual void buildProperties(QtVariantPropertyManager *propertyManager);
        
        const QList<NeuroItem *> incoming() const { return _incoming; }
        const QList<NeuroItem *> outgoing() const { return _outgoing; }

        static NeuroItem *create(const QString & typeName, LabScene *scene, const QPointF & pos);

        void bringToFront();

        virtual void addIncoming(NeuroItem *linkItem);
        virtual void removeIncoming(NeuroItem *linkItem);

        virtual void addOutgoing(NeuroItem *linkItem);
        virtual void removeOutgoing(NeuroItem *linkItem);

        struct EditInfo
        {
            bool linkFront;
            QPointF scenePos;
            NeuroItem *movingItem;

            EditInfo() : linkFront(true), scenePos(0,0), movingItem(0) {}
        };

        virtual bool canLinkTo(EditInfo & info, NeuroItem *item) = 0;
        virtual bool handlePickup(EditInfo & info) = 0;
        virtual void handleMove(EditInfo & info) = 0;
        virtual void adjustLinks() = 0;

        virtual void idsToPointers(QGraphicsScene *);

        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;
        virtual void buildShape();

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual void reset();
        virtual void activate();
        virtual void deactivate();
        virtual void toggleFrozen();
        
    public slots:
        virtual void changeProperty(QtProperty *property);

    protected:
        virtual void setPenWidth(QPen & pen);
        virtual void setPenColor(QPen & pen);

        virtual bool shouldHighlight() const;

        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

        virtual void writeBinary(QDataStream &) const = 0;
        virtual void readBinary(QDataStream &) = 0;

        virtual void writePointerIds(QDataStream &) const;
        virtual void readPointerIds(QDataStream &);

        virtual void idsToPointersAux(QList<NeuroItem *> & list, QGraphicsScene *sc);

        //
        NeuroLib::NeuroCell *getCell();
        QtProperty *_input_threshold;
        QtProperty *_output_weight;
        QtProperty *_output_value;

        //
        typedef NeuroItem * (*CreateFT) (LabScene *scene, const QPointF & pos);
        static QMap<QString, CreateFT> itemCreators;
        friend class NeuroItemRegistrator;

        //
        friend QDataStream & operator<< (QDataStream &, const NeuroItem &);
        friend QDataStream & operator>> (QDataStream &, NeuroItem &);
    };

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

    //

    extern QDataStream & operator<< (QDataStream &, const NeuroItem &);
    extern QDataStream & operator>> (QDataStream &, NeuroItem &);

} // namespace NeuroLab

#endif // NEUROITEM_H
