/**
 * @file   Application.cpp
 *
 * @date   Mar 10, 2013
 * @author Sam Roth <>
 */

#include "Application.hpp"
#include <QApplication>
#include "Main/MainWindow.hpp"
#include "Util/Log.hpp"
#include "DiagramContext.hpp"
#include "Components/PathComponent.hpp"
#include "Components/BoxComponent.hpp"
#include "Components/TextComponent.hpp"
#include "Components/PathConnectorComponent.hpp"
#include "Components/ConnectorComponent.hpp"
#include "ComponentFileReader.hpp"
#include <QtCore>
#include <fstream>
#include <sstream>
#include "Components/ImageComponent.hpp"
#include "DiagramIO/ComponentFile.hpp"
#include "Util/QtUtil.hpp"
#include "Util/Backtrace.hpp"

Q_IMPORT_PLUGIN(dbuilder_TextComponentPlugin)
Q_IMPORT_PLUGIN(dbuilder_ImageComponentPlugin)

namespace dbuilder {

static std::function<int(int, char**)> *replacedMain = nullptr;
Application *Application::_instance = nullptr;

Application::Application()
{
	_instance = this;
	readSettings();


	QStringList candidatePlugins;
	auto addCandidates = [&](const QDir &d) {
		if(d.isReadable())
		{
			for(auto entry : d.entryList())
			{
				candidatePlugins << d.absoluteFilePath(entry);
			}
		}
	};

	auto userAppDir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
	DBDebug(DBDump(userAppDir.path()));
	addCandidates(userAppDir);

	for(const auto &pluginPath : candidatePlugins)
	{
		QPluginLoader loader(pluginPath);
		if(auto pluginObject = loader.instance())
		{
			if(auto plugin = qobject_cast<BasicPlugin *>(pluginObject))
			{
				DBInfo("Loading dynamic plugin: ", plugin->pluginInfo());
				_plugins << pluginObject;
			}
			else
			{
				DBWarning("Unknown dynamic plugin: ", pluginObject->metaObject()->className());
			}
		}
	}

	for(auto pluginObject : QPluginLoader::staticInstances())
	{
		if(auto plugin = qobject_cast<BasicPlugin *>(pluginObject))
		{
			DBInfo("Loading static plugin: ", plugin->pluginInfo());
			_plugins << pluginObject;
		}
		else if(pluginObject)
		{
			DBWarning("Unknown static plugin: ", pluginObject->metaObject()->className());
		}
	}

}

DiagramContext *Application::createContext()
{
	auto result = new DiagramContext;

	for(auto plugin : plugins())
	if(auto componentPlugin = qobject_cast<DiagramComponentPlugin *>(plugin))
	{
		DBInfo("Loading components from plugin: ", componentPlugin->pluginInfo());
		for(auto component : componentPlugin->createComponents(result))
		{
			DBDebug("--> ", component->name());
			result->registerKind(component);
		}
	}

	result->registerKind(new ConnectorComponent(result));
	result->registerKind(new dbuilder::PathConnectorComponent(result));
	result->registerKind(new dbuilder::BoxComponent(result));
	result->registerKind(new PathComponent(result));

	auto registerAll = [&](ComponentFile &cf) {
		for(const auto &componentSpec : cf.components())
		{
			result->registerKind(componentSpec.createKind(result));
		}
	};

	std::stringstream builtinComponentsStream(readFile("://components.info").toStdString());
	ComponentFile builtinComponents(builtinComponentsStream);
	registerAll(builtinComponents);


	for(auto library : libraries())
	{
		QDir libraryDir(library);
		auto entries = libraryDir.entryInfoList({"*.dbcomponent"}, QDir::Files | QDir::NoDotAndDotDot);
		for(const auto &entry : entries)
		{
			std::ifstream is(entry.filePath().toStdString());
			if(is)
			{
				ComponentFile cf(is);
				registerAll(cf);
			}
			else
			{
				DBError("Cannot open '", entry.filePath(), "' for reading.");
			}
		}

	}

	return result;
}

Application::~Application()
{
}

Application* Application::instance()
{
	return _instance;
}

void Application::saveSettings()
{
	_settings.setValue("colors/ports/highlight", _portHighlightColor);
	_settings.setValue("colors/ports/outline", _portOutlineColor);
	_settings.setValue("log/level", log::levelName(log::level()));
	_settings.sync();

	emit settingsChanged();
}

void Application::readSettings()
{
	_portHighlightColor = _settings.value("colors/ports/highlight", QColor("DarkViolet")).value<QColor>();
	_portOutlineColor   = _settings.value("colors/ports/outline",   defaultPortOutlineColor()).value<QColor>();
	log::setLevel(log::levelForName(_settings.value("log/level").toString().toStdString()).get_value_or(log::Debug));
}

void Application::run()
{
	auto mw = addMainWindow();

	if(qApp->argc() > 1)
	{
		auto args = qApp->arguments();
		for(int i = 1; i < args.size(); ++i)
		{
			if(!args[i].startsWith('-'))
			{
				mw->openFile(args[i]);
			}
		}
	}
}

void Application::setMain(std::function<int(int, char**)> f)
{
	replacedMain = new std::function<int(int, char**)>(f);
}

QSharedPointer<MainWindow> Application::addMainWindow()
{
	DBDebug("opening main window");
	QSharedPointer<MainWindow> mainWindow(new MainWindow(this));
	mainWindow->show();
	mainWindow->activateWindow();
	qApp->setActiveWindow(mainWindow.data());
	mainWindow->raise();
	mainWindows << mainWindow;

	return mainWindow;
}

QSet<QString> Application::libraries() const
{
	auto settingsValue = _settings.value("libraries", QStringList());
	return settingsValue.toStringList().toSet();
}

void Application::setLibraries(const QSet<QString> &value)
{
	QStringList convertedValue;
	for(const auto &s : value)
	{
		convertedValue << s;
	}

	_settings.setValue("libraries", convertedValue);
}


} /* namespace dbuilder */


static void signalHandler(int)
{
	sroth::terminateHandler();
}

static void msgHandler(QtMsgType ty, const char *msg)
{
	namespace log = dbuilder::log;
	switch (ty)
	{
		case QtDebugMsg:
			log::writev(log::Debug, "<QT>", 0, msg);
			break;
		case QtWarningMsg:
			log::writev(log::Warning, "<QT>", 0, msg);
			break;
		case QtCriticalMsg:
			log::writev(log::Error, "<QT>", 0, msg);
			break;
		case QtFatalMsg:
			log::writev(log::Critical, "<QT>", 0, msg);
			std::terminate();
	}
}


int main(int argc, char **argv)
{
	qInstallMsgHandler(&msgHandler);
	std::set_terminate(&sroth::terminateHandler);
	for(auto sig : {SIGSEGV, SIGBUS, SIGILL, SIGABRT})
		signal(sig, &signalHandler);

	setBacktraceLevel(dbuilder::log::Warning);

	QApplication::setOrganizationName("saroth");
	QApplication::setOrganizationDomain("saroth");
	QApplication::setApplicationName("DiagramBuilder");

	if(dbuilder::replacedMain)
	{
		return (*dbuilder::replacedMain)(argc, argv);
	}

	QApplication qapp(argc, argv);
	dbuilder::Application app;
	app.run();
	return qapp.exec();
}

