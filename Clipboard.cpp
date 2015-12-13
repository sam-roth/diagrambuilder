#include "DiagramIO/DiagramLoader.hpp"
#include "DiagramItem.hpp"
#include "DiagramScene.hpp"
#include <QApplication>
#include <QClipboard>
#include <sstream>
#include <QMimeData>
#include "DiagramIO/DiagramLoader.hpp"
#include "UUIDMapper.hpp"
#include "DiagramItemModel.hpp"
#include "DiagramComponent.hpp"


/**
 * @file   Clipboard.cpp
 *
 * @date   Jan 20, 2013
 * @author Sam Roth <>
 */



namespace dbuilder {
void copyToClipboard(DiagramLoader *loader, const QList<DiagramItem *> &items)
{
	std::stringstream itemStream;
	loader->save(itemStream, items);

	auto mimeData = new QMimeData;
	mimeData->setData("application/vnd.saroth.dbuilder.document", itemStream.str().c_str());

	QApplication::clipboard()->setMimeData(mimeData);
}

QList<DiagramItem *> pasteFromClipboard(DiagramLoader *loader, DiagramScene *scene)
{
	auto mimeData = QApplication::clipboard()->mimeData();
	auto data = mimeData->data("application/vnd.saroth.dbuilder.document");
	if(data.isNull()) return {};

	std::stringstream itemStream(std::string(data.begin(), data.end()));
	QObject parent;
	auto modelsLoaded = loader->loadModels(itemStream, &parent);

	QList<DiagramItem *> results;
	UUIDMapper mapper;
	for(auto modelLoaded : modelsLoaded)
	{
//		auto result = new DiagramItem(scene);
		auto mappedModel = modelLoaded->clone(&mapper, &parent);
		assert(mappedModel->kind());
		auto result = mappedModel->kind()->createFromModel(mappedModel);
//		result->setModel(mappedModel);


		results << result;
	}

	return results;
}
} // namespace dbuilder
