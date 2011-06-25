#include "multiitem.h"

using namespace NeuroGui;

namespace GridItems
{

    MultiItem::MultiItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNetworkItem(network, scenePos, context)
    {
    }

    MultiItem::~MultiItem()
    {
    }

}
