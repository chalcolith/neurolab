#ifndef PROPERTYOBJ_H
#define PROPERTYOBJ_H

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

#include "neurogui_global.h"

#include <QMap>
#include <QList>
#include <QObject>
#include <QVariant>

#include <QtVariantProperty>

class QtVariantPropertyManager;

namespace NeuroGui
{

    class NEUROGUISHARED_EXPORT PropertyObject;

    /// Base class for properties of various types.
    class NEUROGUISHARED_EXPORT PropertyBase
        : public QObject
    {
        Q_OBJECT

    protected:
        QString _name;
        QString _tooltip;
        bool _editable;
        bool _remember;
        int _type;
        bool _visible;

        PropertyObject *_container;
        QtVariantProperty *_property;

        friend class PropertyObject;

    public:
        explicit PropertyBase(PropertyObject *container, const QString & name, const QString & tooltip,
                              bool editable, int type, bool remember)
            : QObject(),
              _name(name), _tooltip(tooltip),
              _editable(editable), _remember(remember),
              _type(type), _visible(true),
              _container(container), _property(0) {}
        virtual ~PropertyBase() { delete _property; }

        /// This creates the actual variant property in the property grid.
        virtual void createPropertyInBrowser(QtVariantPropertyManager *manager);
        virtual void updateBrowserValueFromContainer() = 0;

        QString name() const { return _name; }
        void setName(const QString & n) { _name = n; if (_property) _property->setPropertyName(n); }

        QString tooltip() const { return _tooltip; }
        void setTooltip(const QString & tt) { _tooltip = tt; if (_property) _property->setToolTip(tt); }

        bool editable() const { return _editable; }
        void setEditable(bool e) { _editable = e; if (_property) _property->setEnabled(e); }

        bool remember() const { return _remember; }
        void setRemember(bool r) { _remember = r; }

        int type() const { return _type; }

        bool visible() const { return _visible; }
        void setVisible(bool visible) { _visible = visible; }

        QtVariantProperty *propertyInBrowser() { return _property; }

        virtual QVariant valueFromPropertyBrowser() const { return _property ? _property->value() : QVariant(); }
        virtual void setValueInPropertyBrowser(const QVariant & val) { if (_property) _property->setValue(val); }

    signals:
        void valueInBrowserChanged(const QVariant & val);

    public slots:
        virtual void changeValueInContainer(const QVariant & val) = 0;
    }; // class PropertyBase

    /// Base class for objects that can be edited via the property widget.
    class NEUROGUISHARED_EXPORT PropertyObject
        : public QObject
    {
        Q_OBJECT

    protected:
        bool _updating;

        /// Holds a property with specific type information.
        /// \param CType The type of the containing object.
        /// \param TypeID The variant type (should be a value from the <tt>QVariant::Type</tt> enum).
        /// \param VType The type used to pass values in and out of the QVariant.
        /// \param DType The actual data type used by the property's getter and setter functions.
        template <typename CType, int TypeID, typename VType, typename DType>
        class Property : public PropertyBase
        {
            CType *_typed_container;

            DType (CType::*_getter) () const;
            void (CType::*_setter)(const DType &);

        public:
            /// Constructor.
            /// \param container The containing object.
            /// \param getter Accessor function for actual data.
            /// \param setter Setter function for the actual data.
            /// \param name The name of the property.
            /// \param tooltip A tooltip for the property.
            /// \param editable Whether or not to enable the property for editing.
            explicit Property(CType *container, DType (CType::*getter)() const, void (CType::*setter)(const DType &),
                              const QString & name, const QString & tooltip = QString(), bool editable = true, bool remember = false)
                : PropertyBase(container, name, tooltip, editable, TypeID, remember),
                _typed_container(container), _getter(getter), _setter(setter)
            {
                Q_ASSERT(_container != 0);
                container->_properties.append(this);
            }

            virtual void updateBrowserValueFromContainer()
            {
                if (_getter)
                    _property->setValue(QVariant(static_cast<VType>((_typed_container->*_getter)())));
            }

            virtual void changeValueInContainer(const QVariant & value)
            {
                if (_setter && _getter && value != (_typed_container->*_getter)())
                {
                    (_typed_container->*_setter)(static_cast<DType>(value.value<VType>()));
                    _typed_container->setChanged(true);

                    emit valueInBrowserChanged(value);
                }
            }
        }; // class Property

        QList<PropertyBase *> _properties;

    public:
        explicit PropertyObject(QObject *parent);
        virtual ~PropertyObject();

        QList<PropertyBase *> properties() const { return _properties; }

        void setPropertyValue(const QString & propertyName, const QVariant & value);

        virtual QString uiName() const { return tr("?Unknown?"); }

        /// Add properties to the parent item.
        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);

        /// Update the properties from the object's state.
        virtual void updateProperties();

        virtual void writeClipboard(QDataStream & ds) const;
        virtual void readClipboard(QDataStream & ds);

    public slots:
        /// Handle changes to the property values.
        virtual void propertyInBrowserChanged(QtProperty *, const QVariant &);
    };


    /// Hold common properties for a number of property objects, and can update them all at once.
    class NEUROGUISHARED_EXPORT CommonPropertyObject
        : public PropertyObject
    {
        Q_OBJECT

        class NEUROGUISHARED_EXPORT CommonProperty
            : public PropertyBase
        {
            QList<PropertyBase *> _shared_properties;

        public:
            explicit CommonProperty(PropertyObject *container, const QString & name, const QString & tooltip, bool enabled, int type);
            virtual ~CommonProperty();

            void addSharedProperty(PropertyBase *p);

            virtual void updateBrowserValueFromContainer();
            virtual void changeValueInContainer(const QVariant &valueFromPropertyBrowser);
        };

    public:
        explicit CommonPropertyObject(QObject *parent, const QList<PropertyObject *> & commonObjects);
        virtual ~CommonPropertyObject();

        virtual QString uiName() const { return tr("Multiple Items"); }

    private:
        void cleanup();
        void getCommonProperties(const QList<PropertyObject *> & commonObjects);
    }; // class CommonPropertyObject

} // namespace NeuroGui

#endif
