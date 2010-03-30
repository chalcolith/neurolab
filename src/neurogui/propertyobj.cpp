#include "propertyobj.h"

#include <QtProperty>

namespace NeuroLab
{

    PropertyObject::PropertyObject(QObject *parent)
        : QObject(parent), _updating(false)
    {
    }

    PropertyObject::~PropertyObject()
    {
        // properties will be deleted when the manager is deleted
    }

    void PropertyObject::buildProperties(QtVariantPropertyManager *manager, QtProperty *topProperty)
    {
        Q_ASSERT(topProperty != 0);
        topProperty->setPropertyName(uiName());

        for (QListIterator<PropertyBase *> i(_properties); i.hasNext(); i.next())
        {
            PropertyBase *p = i.peekNext();
            if (!p->_property)
                p->create(manager);
            topProperty->addSubProperty(p->_property);
        }

        updateProperties();
    }

    void PropertyObject::updateProperties()
    {
        for (QListIterator<PropertyBase *> i(_properties); i.hasNext(); i.next())
        {
            PropertyBase *p = i.peekNext();
            if (p->_property)
                p->update();
        }
    }

    void PropertyObject::propertyValueChanged(QtProperty *property, const QVariant & value)
    {
        QtVariantProperty *vprop = dynamic_cast<QtVariantProperty *>(property);
        if (!vprop)
            return;

        bool changed;

        for (QListIterator<PropertyBase *> i(_properties); i.hasNext(); i.next())
        {
            PropertyBase *p = i.peekNext();
            if (p->_property == vprop)
            {
                changed = true;
                p->valueChanged(value);
            }
        }
    }

} // namespace NeuroLab
