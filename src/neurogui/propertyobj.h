#ifndef PROPERTYOBJ_H
#define PROPERTYOBJ_H

#include "neurogui_global.h"

#include <QList>

class QVariant;
class QtProperty;
class QtVariantPropertyManager;

namespace NeuroLab
{

    /// Base class for objects that can be edited via the property widget.
    class NEUROGUISHARED_EXPORT PropertyObject
    {
    protected:
        QList<QtProperty *> _properties;
        bool _updating;

    public:
        PropertyObject();
        virtual ~PropertyObject();

        /// Add properties to the parent item.
        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);

        /// Handle changes to the property values.
        virtual void propertyValueChanged(QtProperty *, const QVariant &) {}

        /// Update the properties from the object's state.
        virtual void updateProperties() {}
    };

}

#endif
