/**
 * @author "Sam Roth <s>"
 * @date   Sep 16, 2013
 */

#pragma once

namespace dbuilder {

class ColorButton;

class PropertyBinder: public QObject
{
	Q_OBJECT
	QObjectList _objects;
	QUndoStack *_undoStack;
	QMap<QCheckBox *, QByteArray> _propertyForCheckBox;
	QMap<QSpinBox *, QByteArray> _propertyForSpinBox;
	QMap<QSlider *, QByteArray> _propertyForSlider;
	QMap<QRadioButton *, QByteArray> _propertyForRadioButton;
	QMap<QRadioButton *, QVariant> _valueForRadioButton;
	QMap<QLineEdit *, QByteArray> _propertyForLineEdit;
	QMap<QDoubleSpinBox *, QByteArray> _propertyForDSpinBox;
	QMap<ColorButton *, QByteArray> _propertyForColorButton;

	bool _ignore;
	static QUndoCommand *makeSetPropertyCommand(QPointer<QObject> obj,
	                                            QPointer<PropertyBinder> binder,
	                                            const QByteArray &prop,
	                                            QVariant val,
	                                            QUndoCommand *parent=0);
	static QUndoCommand *makeSetPropertyCommand(const QObjectList &objects,
	                                            QPointer<PropertyBinder> binder,
	                                            const QByteArray &prop,
	                                            QVariant val,
	                                            QUndoCommand *parent=0);
public slots:
	void update();
public:
	PropertyBinder(QObject *parent=0);
	virtual ~PropertyBinder();

	const QObjectList& objects() const
	{
		return _objects;
	}

	void setObjects(const QObjectList& objects);
	void bind(QCheckBox *checkbox, const char *property);
	void bind(QSpinBox *spinbox, const char *property);
	void bind(QSlider *slider, const char *property);
	void bind(QRadioButton *radiobutton, const char *property, const QVariant &value);
	void bind(QLineEdit *lineedit, const char *property);
	void bind(QDoubleSpinBox *dspinbox, const char *property);
	void bind(ColorButton *colorbutton, const char *property);

	QUndoStack* undoStack() const
	{
		return _undoStack;
	}

	void setUndoStack(QUndoStack* undoStack)
	{
		_undoStack = undoStack;
	}

	void update(QObject *obj);
private slots:
	void clicked();
};

} /* namespace dbuilder */
