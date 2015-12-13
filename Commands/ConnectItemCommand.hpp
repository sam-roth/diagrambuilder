#pragma once
/**
 * @file   ConnectItemCommand.h
 *
 * @date   Jan 3, 2013
 * @author Sam Roth <>
 */

#include <qundostack.h>
#include "DiagramItemModel.hpp"
#include "CoreForward.hpp"

namespace dbuilder {



class ConnectItemCommand: public QUndoCommand
{
	DiagramScene *_scene;
	DiagramItem *_connectionItem;
	QString _connectorType;
public:
	ConnectItemCommand(DiagramScene *scene, Connection conn, QString connectorType);

	void undo();
	void redo();

	virtual ~ConnectItemCommand();
};


}  // namespace dbuilder
