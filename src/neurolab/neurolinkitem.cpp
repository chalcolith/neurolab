#include "neurolinkitem.h"

namespace NeuroLab
{
    
    NeuroLinkItem::NeuroLinkItem()
            : QGraphicsLineItem()
    {
    }
    
    NeuroLinkItem::~NeuroLinkItem()
    {
    }
    
    NeuroExcitoryLinkItem::NeuroExcitoryLinkItem()
            : NeuroLinkItem()
    {
    }
    
    NeuroExcitoryLinkItem::~NeuroExcitoryLinkItem()
    {
    }
    
    NeuroInhibitoryLinkItem::NeuroInhibitoryLinkItem()
            : NeuroLinkItem()
    {
    }
    
    NeuroInhibitoryLinkItem::~NeuroInhibitoryLinkItem()
    {
    }
    
} // namespace NeuroLab
