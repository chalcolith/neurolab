/*
Neurocognitive Linguistics Lab
Copyright (c) 2010,2011 Gordon Tisher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 - Neither the name of the Neurocognitive Linguistics Lab nor the
   names of its contributors may be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "propertyobj.h"

#include <QtProperty>

#include <QSet>

namespace NeuroGui
{

    PropertyObject::PropertyObject(QObject *parent)
        : QObject(parent), _updating(false)
    {
    }

    PropertyObject::~PropertyObject()
    {
        // properties will be deleted when the manager is deleted
    }

    void PropertyObject::setPropertyValue(const QString &propertyName, const QVariant &value)
    {
        foreach (PropertyBase *p, _properties)
        {
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

        foreach (PropertyBase *p, _properties)
        {
            p->create(manager);

            if (p->visible())
                topProperty->addSubProperty(p->_property);
        }

        updateProperties();
    }

    void PropertyObject::updateProperties()
    {
        foreach (PropertyBase *p, _properties)
        {
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

        foreach (PropertyBase *p, _properties)
        {
            if (p->_property == vprop)
            {
                changed = true;
                p->valueChanged(value);
            }
        }
    }

    void PropertyObject::writeClipboard(QDataStream & ds) const
    {
        ds << static_cast<qint32>(_properties.size());

        foreach (const PropertyBase *p, _properties)
        {
            ds << p->name();
            ds << p->tooltip();
            ds << p->editable();
            ds << p->value();
        }
    }

    void PropertyObject::readClipboard(QDataStream & ds)
    {
        qint32 num;
        ds >> num;

        for (qint32 i = 0; i < num; ++i)
        {
            QString name, tooltip;
            bool enabled;
            QVariant value;

            ds >> name;
            ds >> tooltip;
            ds >> enabled;
            ds >> value;

            foreach (PropertyBase *p, _properties)
            {
                if (p->name() == name)
                {
                    p->setTooltip(tooltip);
                    p->setEditable(enabled);
                    p->setValue(value);
                }
            }
        }
    }


    void PropertyObject::PropertyBase::create(QtVariantPropertyManager *manager)
    {
        Q_ASSERT(manager != 0);

        delete _property;

        _property = manager->addProperty(_type, _name);
        _property->setToolTip(_tooltip);
        _property->setEnabled(_editable);
        QObject::connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)), _container, SLOT(propertyValueChanged(QtProperty*,const QVariant &)), Qt::UniqueConnection);
    }


    //////////////////////////////////////////////////////////////////

    CommonPropertyObject::CommonPropertyObject(QObject *parent, const QList<PropertyObject *> &commonObjects)
        : PropertyObject(parent)
    {
        getCommonProperties(commonObjects);
    }

    CommonPropertyObject::~CommonPropertyObject()
    {
        cleanup();
    }

    // We don't use QSet with these, because we want to preserve the order of properties

    static QList<QList<QString> > get_property_name_sets(const QList<PropertyObject *> &commonObjects)
    {
        QList<QList<QString> > property_name_sets;

        foreach (PropertyObject *po, commonObjects)
        {
            QList<QString> names;

            foreach (PropertyObject::PropertyBase *prop, po->properties())
            {
                QString format("%1|%2|%3|%4");
                QString name = format.arg(prop->name(), prop->tooltip(), prop->editable() ? "1" : 0).arg(prop->type());

                names.append(name);
            }

            property_name_sets.append(names);
        }

        return property_name_sets;
    }

    static QList<QString> get_common_names(const QList<QList<QString> > & property_name_sets)
    {
        QList<QString> common_property_names;

        if (property_name_sets.size() > 0)
        {
            common_property_names = property_name_sets[0];

            for (int i = 1; i < property_name_sets.size(); ++i)
            {
                QList<QString> intersection;

                foreach (QString name, common_property_names)
                {
                    if (property_name_sets[i].contains(name))
                        intersection.append(name);
                }

                common_property_names = intersection;
            }
        }

        return common_property_names;
    }

    void CommonPropertyObject::getCommonProperties(const QList<PropertyObject *> & commonObjects)
    {
        cleanup();

        // get sets of property names for all the objects, then the intersection of those sets
        QList<QList<QString> > property_name_sets = get_property_name_sets(commonObjects);
        QList<QString> common_property_names = get_common_names(property_name_sets);

        // create a property for each name, and build lists of properties to update for each one
        foreach (QString name_i, common_property_names)
        {
            QList<QString> params = name_i.split("|");
            QString name = params[0];
            QString tooltip = params[1];
            bool enabled = (bool) params[2].toInt();
            int type = params[3].toInt();

            CommonProperty *common = new CommonProperty(this, name, tooltip, enabled, type);
            _properties.append(common);

            // find matching properties and add them
            foreach (PropertyObject *po, commonObjects)
            {
                foreach (PropertyBase *obj_prop, po->properties())
                {
                    if (obj_prop->name() == name
                        && obj_prop->tooltip() == tooltip
                        && obj_prop->editable() == enabled
                        && obj_prop->type() == type)
                    {
                        common->addSharedProperty(const_cast<PropertyBase *>(obj_prop));
                    }
                }
            }
        }
    }

    void CommonPropertyObject::cleanup()
    {
        foreach (PropertyBase *p, _properties)
            delete p;

        _properties.clear();
    }


    CommonPropertyObject::CommonProperty::CommonProperty(PropertyObject *container, const QString &name, const QString &tooltip, bool enabled, int type)
        : PropertyBase(container, name, tooltip, enabled, type, false)
    {
    }

    CommonPropertyObject::CommonProperty::~CommonProperty()
    {
    }

    void CommonPropertyObject::CommonProperty::addSharedProperty(PropertyBase *p)
    {
        if (!_shared_properties.contains(p))
            _shared_properties.append(p);
    }

    void CommonPropertyObject::CommonProperty::update()
    {
        QList<QVariant> all_values;

        foreach (PropertyBase *prop, _shared_properties)
            all_values.append(prop->value());

        bool values_are_equal = true;
        for (int i = 1; values_are_equal && i < all_values.size(); ++i)
            values_are_equal = all_values[i] == all_values[i-1];

        if (values_are_equal && all_values.size() > 0 && _property)
            _property->setValue(all_values[0]);
    }

    void CommonPropertyObject::CommonProperty::valueChanged(const QVariant &value)
    {
        foreach (PropertyBase *p, _shared_properties)
            p->valueChanged(value);
    }

} // namespace NeuroGui
