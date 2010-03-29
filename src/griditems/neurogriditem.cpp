#include "neurogriditem.h"
#include "../neurogui/labnetwork.h"
#include "../neurogui/labscene.h"

using namespace NeuroLab;

namespace GridItems
{

    NEUROITEM_DEFINE_CREATOR(NeuroGridItem, QObject::tr("Grid|Grid Item"));

    NeuroGridItem::NeuroGridItem(LabNetwork *network, const QPointF & scenePos)
        : NeuroItem(network, scenePos)
    {
    }

    NeuroGridItem::~NeuroGridItem()
    {
    }

    bool NeuroGridItem::canAttachTo(const QPointF &, NeuroItem *)
    {
        return false;
    }

    bool NeuroGridItem::canBeAttachedBy(const QPointF &, NeuroItem *)
    {
        return false;
    }

    void NeuroGridItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroItem::addToShape(drawPath, texts);
        drawPath.addRect(-20, -20, 40, 40);
    }

} // namespace GridItems
