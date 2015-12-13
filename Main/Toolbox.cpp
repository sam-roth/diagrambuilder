/**
 * @file   Toolbox.cpp
 *
 * @date   Mar 14, 2013
 * @author Sam Roth <>
 */

#include <QtGui>
#include "Toolbox.hpp"
#include "Util/Optional.hpp"
#include "Util/Log.hpp"
#include <QtCore>
#include "Util/Levenshtein.hpp"
#include <cmath>
#include <ios>
#include <iomanip>
#include "ThirdParty/LineEdit.hpp"

namespace dbuilder {

class ToolboxData: public QObject
{
	Q_OBJECT
	Toolbox *that;
public:
	QMap<QAction *, QToolButton *> buttonForAction;
	QMap<QString, QToolButton *> buttonForName;
	QVBoxLayout *layout;
	QLineEdit *searchBar;
	QMultiMap<double, QAction *> actionsByRelevance;



	ToolboxData(Toolbox *parent)
	: QObject(parent)
	, that(parent)
	{
		layout = new QVBoxLayout(that);
		searchBar = new LineEdit(that);
		layout->addWidget(searchBar);
		connect(searchBar, SIGNAL(textChanged(const QString &)), this, SLOT(search(const QString &)));
		connect(searchBar, SIGNAL(returnPressed()), this, SLOT(insertFirst()));
	}
public slots:
	void insertFirst()
	{
		if(!actionsByRelevance.empty())
		{
			(**actionsByRelevance.begin()).trigger();
		}
	}
	void search(const QString &query)
	{
		for(auto item : buttonForName)
		{
			layout->removeWidget(item);
		}
		if(query.isEmpty())
		{
			QWidget *lastToolButton = searchBar;
			for(auto item : buttonForAction)
			{
				if(lastToolButton)
				{
					QWidget::setTabOrder(lastToolButton, item);
				}

				lastToolButton = item;

				layout->addWidget(item);
				item->setVisible(true);
			}
		}
		else
		{
			DBDebug("\nLIST OF ACTIONS FOLLOWS:\n");
			actionsByRelevance.clear();
			for(auto action : buttonForAction.keys())
			{
				size_t dist = levenshteinDistance(action->text().toStdString(), query.toStdString());
				double normalizedDist = dist / double(std::abs(action->text().size() - query.size()));
				if(std::isnan(normalizedDist)) normalizedDist = 0;
				actionsByRelevance.insert(normalizedDist, action);
			}


			size_t bestMatch = std::numeric_limits<size_t>::max();
			QWidget *lastToolButton = searchBar;
			auto it = actionsByRelevance.begin(), end = actionsByRelevance.end();

			for(; it != end; ++it)
			{
				if(bestMatch > it.key()) bestMatch = it.key();
				if(it.key() - bestMatch > 1) break;
				DBDebug("  ", std::setw(8), std::left, it.key(), (*it)->text());

				if(auto button = get(buttonForAction, *it))
				{
					if(lastToolButton)
					{
						QWidget::setTabOrder(lastToolButton, *button);
					}
					lastToolButton = *button;
					layout->addWidget(*button);
					(*button)->setVisible(true);
				}
			}

			for(; it != end; ++it)
			{
				if(auto button = get(buttonForAction, *it))
				{
					(*button)->setVisible(false);
				}
			}


		}
	}
};

Toolbox::Toolbox(QWidget* parent)
: QWidget(parent)
, d(new ToolboxData(this))
{
}

Toolbox::~Toolbox()
{
}

bool Toolbox::event(QEvent *event)
{
	if(auto actionEvent = dynamic_cast<QActionEvent *>(event))
	{
		if(actionEvent->type() == QEvent::ActionAdded)
		{
			auto button = new QToolButton(this);
			button->setDefaultAction(actionEvent->action());
			button->setIconSize(QSize(32, 32));
			button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
			button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
			d->layout->addWidget(button);
			d->buttonForAction[actionEvent->action()] = button;
			d->buttonForName[actionEvent->action()->text()] = button;
		}
		else if(actionEvent->type() == QEvent::ActionRemoved)
		{
			if(auto button = get(d->buttonForAction, actionEvent->action()))
			{
				delete *button;
				d->buttonForAction.remove(actionEvent->action());
				d->buttonForName.remove(actionEvent->action()->text());
			}
			else
			{
				DBWarning("No button found for action");
			}
		}
	}
	return Super::event(event);
}

} /* namespace dbuilder */
#include "Toolbox.moc"
