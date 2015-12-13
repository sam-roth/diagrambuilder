/**
 * @file   InsertItemCommand.cpp
 *
 * @date   Jan 3, 2013
 * @author Sam Roth <>
 */

#include "InsertItemCommand.hpp"
#include "DiagramItem.hpp"
#include "../DiagramScene.hpp"
#include "DiagramScene.hpp"
#include "DiagramContext.hpp"
#include "DiagramComponent.hpp"

namespace dbuilder {
InsertItemCommand::InsertItemCommand(DiagramScene *scene, QString kind, QPointF pos, QUndoCommand* parent)
: QUndoCommand("insert " + kind, parent)
, _scene(scene)
, _item(scene->context()->kind(kind)->create(scene)) //new DiagramItem(scene, kind))
{
	// set model's pos because we haven't added the
	// item to the scene yet and its position
	// will be overwritten
	_item->model()->setScenePos(pos);
}

void InsertItemCommand::undo()
{
	_scene->removeDiagramItem(_item);
}

void InsertItemCommand::redo()
{
	_scene->addDiagramItem(_item);
	_item->ensureVisible(_item->boundingRect(), 10, 10);
}

InsertItemCommand::~InsertItemCommand()
{
}
} // namespace dbuilder

