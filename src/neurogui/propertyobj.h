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

        /// Base class for properties of various types.
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

        /// Holds a property with specific type information.
        /// \param CType The type of the containing object.
        /// \param TypeID The variant type (should be a value from the \ref QVariant::Type enum).
        /// \param VType The type used to pass values in and out of the QVariant.
        /// \param DType The actual data type used by the property's getter and setter functions.
        template <typename CType, int TypeID, typename VType, typename DType>
        class Property : public PropertyBase
        {
            CType *_container;

            DType (CType::*_getter) () const;
            void (CType::*_setter)(const DType &);

            QString _name, _tooltip;
            bool _enabled;

        public:
            /// Constructor.
            /// \param container The containing object.
            /// \param getter Accessor function for actual data.
            /// \param setter Setter function for the actual data.
            /// \param name The name of the property.
            /// \param tootip A tooltip for the property.
            /// \param enabled Whether or not to enable the property for editing.
            Property(CType *container, DType (CType::*getter)() const, void (CType::*setter)(const DType &), const QString & name, const QString & tooltip = QString(), bool enabled = true)
                : PropertyBase(),
                _container(container), _getter(getter), _setter(setter),
                _name(name), _tooltip(tooltip), _enabled(enabled)
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
                    _property->setToolTip(_tooltip);
                    _property->setEnabled(_enabled);
                    QObject::connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)), _container, SLOT(propertyValueChanged(QtProperty*,const QVariant &)), Qt::UniqueConnection);
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

        QList<QtVariantProperty *> properties() const;

        void setPropertyValue(const QString & propertyName, const QVariant & value);

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
