/**
 * @file   ConnectItemCommand.cpp
 *
 * @date   Jan 3, 2013
 * @author Sam Roth <>
 */

#include "ConnectItemCommand.hpp"
#include "../DiagramScene.hpp"
#include "DiagramItem.hpp"
#include "DiagramScene.hpp"
#include "DiagramContext.hpp"
#include "DiagramComponent.hpp"
namespace dbuilder {

ConnectItemCommand::~ConnectItemCommand()
{
}

ConnectItemCommand::ConnectItemCommand(DiagramScene *scene, Connection conn, QString connectorType)
: QUndoCommand("connection")
, _scene(scene)
, _connectionItem(scene->context()->kind(connectorType)->create(scene))
, _connectorType(connectorType)
{
	_connectionItem->model()->setConnection(conn);
}

void ConnectItemCommand::undo()
{
	_scene->removeItem(_connectionItem);
}

void ConnectItemCommand::redo()
{
	_scene->addDiagramItem(_connectionItem);
}

} // namespace dbuilder
