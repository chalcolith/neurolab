#ifndef NEUROLINKITEM_H
#define NEUROLINKITEM_H

#include <QGraphicsLineItem>

namespace NeuroLab
{
    
    class NeuroLinkItem 
            : public QGraphicsLineItem
    {
    public:
        NeuroLinkItem();
        virtual ~NeuroLinkItem();
    };
    
    class NeuroExcitoryLinkItem
            : public NeuroLinkItem
    {
    public:
        NeuroExcitoryLinkItem();
        virtual ~NeuroExcitoryLinkItem();
    };
    
    class NeuroInhibitoryLinkItem
            : public NeuroLinkItem
    {
    public:
        NeuroInhibitoryLinkItem();
        virtual ~NeuroInhibitoryLinkItem();
    };
    
} // namespace NeuroLab

#endif // NEUROLINKITEM_H
