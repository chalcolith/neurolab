#include "propertyobj.h"

#include <QtProperty>

namespace NeuroLab
{
    
    PropertyObject::PropertyObject()
        : _updating(false)
    {
    }

    PropertyObject::~PropertyObject()
    {
        // properties will be deleted when the manager is deleted
    }
    
    void PropertyObject::buildProperties(QtVariantPropertyManager *, QtProperty *parentItem)
    {        
        for (QListIterator<QtProperty *> i(_properties); i.hasNext(); )
            parentItem->addSubProperty(i.next());
    }
    
}
