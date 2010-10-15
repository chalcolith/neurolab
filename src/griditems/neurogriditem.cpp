/*
POSSIBILITY OF SUCH DAMAGE.
*/

#include "neurogriditem.h"
#include "../neurogui/labnetwork.h"
#include "../neurogui/labscene.h"

using namespace NeuroGui;

namespace GridItems
{

    NEUROITEM_DEFINE_CREATOR(NeuroGridItem, QString("Debug"), QObject::tr("Grid Item"));

    NeuroGridItem::NeuroGridItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroItem(network, scenePos, context)
    {
    }

    NeuroGridItem::~NeuroGridItem()
    {
    }

    void NeuroGridItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroItem::addToShape(drawPath, texts);
        drawPath.addRect(-20, -20, 40, 40);
    }

} // namespace GridItems
