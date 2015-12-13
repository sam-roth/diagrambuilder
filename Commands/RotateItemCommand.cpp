/**
 * @file   TransformItemCommand.cpp
 *
 * @date   Jan 16, 2013
 * @author Sam Roth <>
 */

#include "RotateItemCommand.hpp"
#include "DiagramItem.hpp"

namespace dbuilder {
RotateItemCommand::RotateItemCommand(DiagramScene* scene,
		DiagramItem* item, qreal dRotation, QUndoCommand *parent)
: QUndoCommand(parent)
, _scene(scene)
, _item(item)
, _dRotation(dRotation)
{
}

void RotateItemCommand::undo()
{
	_item->setRotation(_item->rotation() - _dRotation);
}

void RotateItemCommand::redo()
{
	_item->setRotation(_item->rotation() + _dRotation);
}

RotateItemCommand::~RotateItemCommand()
{
}

} // namespace dbuilder
