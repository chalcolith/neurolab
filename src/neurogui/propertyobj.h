#ifndef PROPERTYOBJ_H
#define PROPERTYOBJ_H

#include "neurogui_global.h"

#include <QList>
#include <QObject>
#include <QVariant>

#include <QtVariantProperty>

class QtVariantPropertyManager;

namespace NeuroLab
{

    /// Base class for objects that can be edited via the property widget.
    class NEUROGUISHARED_EXPORT PropertyObject
        : public QObject
    {
        Q_OBJECT

    protected:
        bool _updating;

        class PropertyBase
        {
        public:
            QtVariantProperty *_property;

            PropertyBase() : _property(0) {}
            virtual ~PropertyBase() {}

            virtual void create(QtVariantPropertyManager *manager) = 0;
            virtual void update() = 0;
            virtual void valueChanged(const QVariant & value) = 0;
        };

        QList<PropertyBase *> _properties;

        template <typename CType, int TypeID, typename VType, typename DType>
        class Property : public PropertyBase
        {
            CType *_container;

            DType (CType::*_getter) () const;
            void (CType::*_setter)(const DType &);

            QString _name;
            bool _enabled;

        public:
            Property(CType *container, DType (CType::*getter)() const, void (CType::*setter)(const DType &), const QString & name, bool enabled = true)
                : PropertyBase(),
                _container(container), _getter(getter), _setter(setter), _name(name), _enabled(enabled)
            {
                Q_ASSERT(_container != 0);
                container->_properties.append(this);
            }

            virtual void create(QtVariantPropertyManager *manager)
            {
                Q_ASSERT(manager != 0);

                if (!_property)
                {
                    _property = manager->addProperty(TypeID, _name);
                    _property->setEnabled(_enabled);
                    QObject::connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)), _container, SLOT(propertyValueChanged(QtProperty*,const QVariant &)));
                }
            }

            virtual void update()
            {
                if (_getter)
                    _property->setValue(QVariant(static_cast<VType>((_container->*_getter)())));
            }

            virtual void valueChanged(const QVariant & value)
            {
                if (_setter && _getter && value != (_container->*_getter)())
                {
                    (_container->*_setter)(static_cast<DType>(value.value<VType>()));
                    _container->setChanged(true);
                }
            }
        };

    public:
        PropertyObject(QObject *parent);
        virtual ~PropertyObject();

        virtual QString uiName() const { return QString("?Unknown?"); }

        /// Add properties to the parent item.
        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);

        /// Update the properties from the object's state.
        virtual void updateProperties();

    public slots:
        /// Handle changes to the property values.
        virtual void propertyValueChanged(QtProperty *, const QVariant &);
    };

}

#endif
