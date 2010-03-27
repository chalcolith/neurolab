#include "neurogriditem.h"
#include "../neurogui/labnetwork.h"
#include "../neurogui/labscene.h"

using namespace NeuroLab;

namespace GridItems
{

    NEUROITEM_DEFINE_CREATOR(NeuroGridItem, QObject::tr("Grid|Grid Item"));

    NeuroGridItem::NeuroGridItem(LabNetwork *network)
        : NeuroItem(network)
    {
    }

    NeuroGridItem::~NeuroGridItem()
    {
    }

    NeuroItem *NeuroGridItem::create_new(NeuroLab::LabScene *scene, const QPointF & pos)
    {
        if (!(scene && scene->network() && scene->network()->neuronet()))
            return 0;

        NeuroItem *item = new NeuroGridItem(scene->network());
        item->setPos(pos.x(), pos.y());
        return item;
    }

    void NeuroGridItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroItem::addToShape(drawPath, texts);

        drawPath.addRect(-20, -20, 40, 40);
    }

} // namespace GridItems
