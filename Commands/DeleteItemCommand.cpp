/**
 * @file   DeleteItemCommand.cpp
 *
 * @date   Jan 4, 2013
 * @author Sam Roth <>
 */

#include "DeleteItemCommand.hpp"
#include "DiagramScene.hpp"
namespace dbuilder {
void DeleteItemCommand::redo()
{
	_itemsRemoved = _scene->removeDiagramItem(this->_item);
}

void DeleteItemCommand::undo()
{
	_scene->addDiagramItemsInOrder(_itemsRemoved);
	_itemsRemoved.clear();
}

DeleteItemCommand::DeleteItemCommand(DiagramScene* scene, DiagramItem* item, QUndoCommand *parent)
: QUndoCommand("delete item", parent)
, _scene(scene)
, _item(item)
{
}

DeleteItemCommand::~DeleteItemCommand()
{
}
} // namespace dbuilder

