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

    QList<QtVariantProperty *> PropertyObject::properties() const
    {
        QList<QtVariantProperty *> results;

        for (QListIterator<PropertyBase *> i(_properties); i.hasNext(); i.next())
        {
            PropertyBase *p = i.peekNext();
            if (p->_property)
                results.append(p->_property);
        }

        return results;
    }

    void PropertyObject::setPropertyValue(const QString &propertyName, const QVariant &value)
    {
        for (QListIterator<PropertyBase *> i(_properties); i.hasNext(); i.next())
        {
            PropertyBase *p = i.peekNext();
            if (p->_property && p->_property->propertyName() == propertyName)
            {
                p->_property->setValue(value);
                p->valueChanged(value);
            }
        }
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
