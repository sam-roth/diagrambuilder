#pragma once
#include <QSettings>
#include <QObject>
#include <functional>
#include <QSharedPointer>
#include <QColor>
#include "CoreForward.hpp"
#include <QList>
/**
 * @file   Application.hpp
 *
 * @date   Mar 10, 2013
 * @author Sam Roth <>
 */



namespace dbuilder {
class MainWindow;


class Application: public QObject
{
	Q_OBJECT
	QList<QSharedPointer<MainWindow>> mainWindows;
	QList<DiagramContext *> activeContexts;
	QList<QObject *> _plugins;


	friend class ReplaceMain;
	QSettings _settings;
	static void setMain(std::function<int(int, char**)> f);
	static Application *_instance;

	QColor _portOutlineColor, _portHighlightColor;
public:
	static Application *instance();

	Application();
	virtual ~Application();

	QSettings &settings() { return _settings; }

	void saveSettings();
	void readSettings();

	DiagramContext *createContext();

	QSet<QString> libraries() const;
	void setLibraries(const QSet<QString> &);

	void run();

	const QColor& portOutlineColor() const
	{
		return _portOutlineColor;
	}

	const QColor &defaultPortOutlineColor() const
	{
		static QColor result("DarkViolet");
		return result;
	}

	void setPortOutlineColor(const QColor& portOutlineColor)
	{
		_portOutlineColor = portOutlineColor;
	}

	const QColor& portHighlightColor() const
	{
		return _portHighlightColor;
	}

	void setPortHighlightColor(const QColor& portHighlightColor)
	{
		_portHighlightColor = portHighlightColor;
	}

	class ReplaceMain
	{
	public:
		ReplaceMain(std::function<int(int, char**)> f)
		{
			Application::setMain(f);
		}
	};

	const QList<QObject *> &plugins() const
	{
		return _plugins;
	}
signals:
	void settingsChanged();

public slots:
	QSharedPointer<MainWindow> addMainWindow();
};


} /* namespace dbuilder */
