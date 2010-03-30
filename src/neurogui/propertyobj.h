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
        protected:
            QtVariantProperty *_property;
            
        public:
            PropertyBase() : _property(0) {}
            virtual ~PropertyBase() {}
            
            virtual void create(QtVariantPropertyManager *manager) = 0;
            virtual void update() = 0;
            virtual void valueChanged(const QVariant & value) = 0;
        };
        
        QList<PropertyBase *> _properties;
        
        template <typename CType, typename VType, typename DType>
        class Property : public PropertyBase
        {
            CType *_container;
                                    
            const DType & (CType::*_getter) ();
            void (CType::*_setter)(const DType &);
            
        public:
            Property(CType *container, const DType & (CType::*getter)(), void (CType::*setter)(const DType &))
                : _container(container),
                _getter(getter), _setter(setter),
                _manager(0), _property(0)
            {
                Q_ASSERT(container != 0);
                Q_ASSERT(getter != 0);
                Q_ASSERT(setter != 0);
                
                container->_properties.append(this);
            }
            
            virtual void create(QtVariantPropertyManager *manager)
            {
                Q_ASSERT(manager != 0);
                
                if (!_property)
                {
                    _property = new QtVariantProperty(manager);
                    QObject::connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)), container, SLOT(propertyValueChanged(QtProperty*,const QVariant &)));
                }
            }
            
            virtual void update()
            {
                _property->setValue(QVariant(static_cast<VType>(_access->*getter())));
            }
            
            virtual void valueChanged(const QVariant & value)
            {
                _access->*setter(static_cast<DType>(value.value<VType>()));
            }
        };

    public:
        PropertyObject();
        virtual ~PropertyObject();

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
