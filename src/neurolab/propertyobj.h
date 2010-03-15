#ifndef PROPERTYOBJ_H
#define PROPERTYOBJ_H

#include <QList>

class QtProperty;
class QtVariantPropertyManager;

namespace NeuroLab
{
    
    class PropertyObject
    {
    protected:
        QList<QtProperty *> _properties;
        bool _updating;
        
    public:
        PropertyObject();
        virtual ~PropertyObject();        
        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
    };
    
}

#endif
