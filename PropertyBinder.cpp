/**
 * @author "Sam Roth <s>"
 * @date   Sep 16, 2013
 */

#include "PropertyBinder.hpp"
#include "Util/ScopeExit.hpp"
#include "Commands/FunctionUndoCommand.hpp"
#include "Util/Log.hpp"
#include "ColorButton.hpp"

namespace dbuilder {

PropertyBinder::PropertyBinder(QObject* parent)
: QObject(parent)
, _undoStack(nullptr)
, _ignore(false)
{
}

PropertyBinder::~PropertyBinder()
{
}

void PropertyBinder::setObjects(const QObjectList& objects)
{
	_objects = objects;
	update();
}

void PropertyBinder::bind(QCheckBox* checkbox, const char* property)
{
	_propertyForCheckBox[checkbox] = property;
	connect(checkbox, SIGNAL(clicked()), SLOT(clicked()));
}
void PropertyBinder::bind(QSpinBox* spinbox, const char* property)
{
	_propertyForSpinBox[spinbox] = property;
	connect(spinbox, SIGNAL(valueChanged(int)), SLOT(clicked()));
}

void PropertyBinder::bind(QSlider* slider, const char* property)
{
	_propertyForSlider[slider] = property;
	connect(slider, SIGNAL(valueChanged(int)), SLOT(clicked()));
}

void PropertyBinder::bind(QRadioButton* radiobutton,
                          const char* property,
                          const QVariant& value)
{
	_propertyForRadioButton[radiobutton] = property;
	_valueForRadioButton[radiobutton] = value;
	connect(radiobutton, SIGNAL(clicked()), SLOT(clicked()));
}

void PropertyBinder::bind(QLineEdit *lineedit, const char* property)
{
	_propertyForLineEdit[lineedit] = property;
	connect(lineedit, SIGNAL(editingFinished()), SLOT(clicked()));
}
void PropertyBinder::bind(QDoubleSpinBox* dspinbox, const char* property)
{
	_propertyForDSpinBox[dspinbox] = property;
	connect(dspinbox, SIGNAL(valueChanged(double)), SLOT(clicked()));
}

void PropertyBinder::bind(ColorButton* colorbutton, const char* property)
{
	_propertyForColorButton[colorbutton] = property;
	connect(colorbutton, SIGNAL(colorChanged(QColor)), SLOT(clicked()));
}


inline QVariant survey(const QObjectList &objects, const char *property)
{
	QVariant res;

	for(auto obj : objects)
	{
		if(!res.isValid())
		{
			res = obj->property(property);
		}
		else if(res != obj->property(property))
		{
			res.clear();
			break;
		}
	}
	return res;
}


template <typename I>
class IteratorIterator
{
	I it;
public:
	IteratorIterator(I it)
	: it(std::move(it)) { }


	bool operator ==(const IteratorIterator &other) const { return it == other.it; }
	bool operator !=(const IteratorIterator &other) const { return it != other.it; }
	IteratorIterator &operator ++()
	{
		++it;
		return *this;
	}

	const I &operator *() const
	{
		return it;
	}

	const I *operator ->() const
	{
		return &it;
	}
};

template <typename T>
class IteratorContainer
{
	const T &container;
public:
	IteratorContainer(const T &c)
	: container(c)
	{ }

	IteratorIterator<typename T::const_iterator> begin()
	{
		return container.begin();
	}
	IteratorIterator<typename T::const_iterator> end()
	{
		return container.end();
	}
};

template <typename T>
IteratorContainer<T> makeIteratorContainer(const T &c)
{
	return c;
}



void PropertyBinder::update()
{
	if(_ignore) return;
	_ignore = true;
	DBScopeExit([this]() { _ignore = false; });
	for(auto it : makeIteratorContainer(_propertyForCheckBox))
	{
		auto v = survey(_objects, it.value());

		if(!v.isValid())
		{
			it.key()->setCheckState(Qt::PartiallyChecked);
		}
		else
		{
			it.key()->setChecked(qvariant_cast<bool>(v));
		}
	}

	for(auto it : makeIteratorContainer(_propertyForSpinBox))
	{
		auto v = survey(_objects, it.value());

		if(!v.isValid())
		{
			it.key()->clear();
		}
		else
		{
			it.key()->setValue(qvariant_cast<int>(v));
		}
	}

	for(auto it : makeIteratorContainer(_propertyForSlider))
	{
		auto v = survey(_objects, it.value());
		if(v.isValid())
		{
			it.key()->setValue(qvariant_cast<int>(v));
		}
	}

	for(auto it : makeIteratorContainer(_propertyForRadioButton))
	{
		auto v = survey(_objects, it.value());

		if(v.isValid())
		{
			it.key()->setChecked(v == _valueForRadioButton[it.key()]);
		}
		else
		{
			if(it.key()->group())
			{
				it.key()->group()->setExclusive(false);
			}
			it.key()->setChecked(false);
			if(it.key()->group())
			{
				it.key()->group()->setExclusive(true);
			}
		}
	}

	for(auto it : makeIteratorContainer(_propertyForLineEdit))
	{
		auto v = survey(_objects, it.value());
		if(v.isValid())
		{
			it.key()->setPlaceholderText({});
			it.key()->setText(v.toString());
		}
		else
		{
			it.key()->setPlaceholderText(tr("[multiple selection]"));
			it.key()->clear();
		}
	}

	for(auto it : makeIteratorContainer(_propertyForDSpinBox))
	{
		auto v = survey(_objects, it.value());

		if(!v.isValid())
		{
			it.key()->clear();
		}
		else
		{
			it.key()->setValue(qvariant_cast<double>(v));
		}
	}

	for(auto it : makeIteratorContainer(_propertyForColorButton))
	{
		auto v = survey(_objects, it.value());

		if(v.isValid())
		{
			it.key()->setColor(qvariant_cast<QColor>(v));
		}
	}
}


QUndoCommand *PropertyBinder::makeSetPropertyCommand(QPointer<QObject> obj,
                                                     QPointer<PropertyBinder> binder,
                                                     const QByteArray &prop,
                                                     QVariant val,
                                                     QUndoCommand *parent)
{

	auto oldValue = obj->property(prop);

	auto set = [obj, binder, prop](const QVariant &v) {
//		DBDebug("setting");
		if(obj) obj->setProperty(prop, v);
		if(binder) binder->update();
	};

	return new FunctionUndoCommand([=]() { set(val); },
	                               [=]() { set(oldValue); },
	                               parent);
}

QUndoCommand* PropertyBinder::makeSetPropertyCommand(const QObjectList& objects,
                                                     QPointer<PropertyBinder> binder,
                                                     const QByteArray& prop,
                                                     QVariant val,
                                                     QUndoCommand* parent)
{
	auto parentCmd = new QUndoCommand(parent);
	for(auto obj : objects)
	{
		makeSetPropertyCommand(obj, binder, prop, val, parentCmd);
	}
	return parentCmd;
}

void PropertyBinder::update(QObject* obj)
{
	assert(_undoStack);
	DBDebug("clicked()");
	if(_ignore) return;
	_ignore = true;
	DBScopeExit([this]() { _ignore = false; });
	if(auto checkbox = qobject_cast<QCheckBox *>(obj))
	{
		bool value = checkbox->isChecked();
		auto prop = _propertyForCheckBox[checkbox];
		if(value)
		{
			checkbox->setTristate(false); // checkboxes will be put into tristate mode if they are ever set to Qt::PartiallyChecked
		}
		_undoStack->push(makeSetPropertyCommand(_objects, this, prop, value));
	}
	else if(auto spinbox = qobject_cast<QSpinBox *>(obj))
	{
		int value = spinbox->value();
		auto prop = _propertyForSpinBox[spinbox];
		_undoStack->push(makeSetPropertyCommand(_objects, this, prop, value));
	}
	else if(auto slider = qobject_cast<QSlider *>(obj))
	{
		int value = slider->value();
		auto prop = _propertyForSlider[slider];
		_undoStack->push(makeSetPropertyCommand(_objects, this, prop, value));
	}
	else if(auto radiobutton = qobject_cast<QRadioButton *>(obj))
	{
		auto prop = _propertyForRadioButton[radiobutton];
		auto val = _valueForRadioButton[radiobutton];
		if(radiobutton->isChecked())
		{
			_undoStack->push(makeSetPropertyCommand(_objects, this, prop, val));
		}
	}
	else if(auto lineedit = qobject_cast<QLineEdit *>(obj))
	{
		auto prop = _propertyForLineEdit[lineedit];
		QString value = lineedit->text();
		_undoStack->push(makeSetPropertyCommand(_objects, this, prop, value));
	}
	else if(auto dspinbox = qobject_cast<QDoubleSpinBox *>(obj))
	{
		auto val = dspinbox->value();
		auto prop = _propertyForDSpinBox[dspinbox];
		_undoStack->push(makeSetPropertyCommand(_objects, this, prop, val));
	}
	else if(auto colorbutton = qobject_cast<ColorButton *>(obj))
	{
		auto val = colorbutton->color();
		auto prop = _propertyForColorButton[colorbutton];
		_undoStack->push(makeSetPropertyCommand(_objects, this, prop, val));
	}
}


void PropertyBinder::clicked()
{
	update(sender());
}

} /* namespace dbuilder */
