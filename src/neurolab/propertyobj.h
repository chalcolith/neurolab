#ifndef PROPERTYOBJ_H
#define PROPERTYOBJ_H

#include <QList>

class QVariant;
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
        virtual void propertyValueChanged(QtProperty *, const QVariant &) {}
        virtual void updateProperties() {}
    };

}

#endif
