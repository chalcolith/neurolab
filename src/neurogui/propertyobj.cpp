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

    void PropertyObject::buildProperties(QtVariantPropertyManager *manager, QtProperty *topProperty)
    {
        for (QListIterator<Property *> i(_properties); i.hasNext(); i.next())
        {
            Property *p = i.peekNext();
            if (!p->_property)
                p->create(manager);
            topProperty->addSubProperty(p->_property);
        }
    }
    
    void PropertyObject::updateProperties()
    {
        for (QListIterator<Property *> i(_properties); i.hasNext(); i.next())
        {
            Property *p = i.peekNext();
            if (p->_property)
                p->update();
        }
    }
    
    void PropertyObject::propertyValueChanged(QtProperty *property, const QVariant & value)
    {
        QtVariantProperty *vprop = dynamic_cast<QtVariantProperty>(property);
        if (!vprop)
            return;
        
        for (QListIterator<Property *> i(_properties); i.hasNext(); i.next())
        {
            Property *p = i.peekNext();
            
            if (p->_property == vprop)
                p->valueChanged(value);
        }
    }

}
