#pragma once
#include <QString>
#include <QObject>
#include <QIcon>
#include "Util/Printable.hpp"
#include "CoreForward.hpp"

/**
 * @file   DiagramComponent.h
 *
 * @date   Jan 2, 2013
 * @author Sam Roth <>
 */

namespace dbuilder {



class DiagramComponent: public QObject, public Printable
{
	Q_OBJECT
	QString _name;
	bool _hidden;
public:
	DiagramComponent(QString name, QObject *parent=nullptr);

	virtual ~DiagramComponent();

	const QString& name() const
	{
		return _name;
	}

	virtual void configure(DiagramItem *) const = 0;
	virtual void dependencyPosChanged(DiagramItem *item, DiagramItem *dependency) const { }
	virtual QIcon icon() const;

	bool hidden() const
	{
		return _hidden;
	}

	void setHidden(bool hidden)
	{
		_hidden = hidden;
	}

	virtual void print(std::ostream &os) const;

	DiagramItem *create(DiagramScene *scene) const;
	virtual DiagramItem *createFromModel(DiagramItemModel *model) const;

	virtual PropertyWidget *makePropertyWidget(QWidget *parent=nullptr) const;
};


}  // namespace dbuilder
