/*
Neurocognitive Linguistics Lab
Copyright (c) 2010, Gordon Tisher
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

    void PropertyObject::writeClipboard(QDataStream & ds) const
    {
        ds << static_cast<qint32>(_properties.size());
        for (QListIterator<PropertyBase *> i(_properties); i.hasNext(); i.next())
        {
            const PropertyBase *p = i.peekNext();

            ds << p->name();
            ds << p->tooltip();
            ds << p->enabled();
            ds << p->value();
        }
    }

    void PropertyObject::readClipboard(QDataStream & ds, const QMap<int, int> &)
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

            for (QListIterator<PropertyBase *> i(_properties); i.hasNext(); i.next())
            {
                PropertyBase *p = const_cast<PropertyBase *>(i.peekNext());
                if (p->name() == name)
                {
                    p->setTooltip(tooltip);
                    p->setEnabled(enabled);
                    p->setValue(value);
                }
            }
        }
    }

} // namespace NeuroLab
