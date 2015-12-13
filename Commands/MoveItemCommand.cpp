/**
 * @file   MoveItemCommand.cpp
 *
 * @date   Jan 16, 2013
 * @author Sam Roth <>
 */

#include "MoveItemCommand.hpp"
#include "DiagramItem.hpp"

namespace dbuilder {
MoveItemCommand::MoveItemCommand(DiagramItem* item,
		QPointF prevPos, QPointF nextPos, QUndoCommand *parent)
: QUndoCommand(parent)
, _item(item)
, _prevPos(prevPos)
, _nextPos(nextPos)
{
}

void MoveItemCommand::undo()
{
	_item->setPos(_prevPos);
}

void MoveItemCommand::redo()
{
	_item->setPos(_nextPos);
}

MoveItemCommand::~MoveItemCommand()
{
}
} // namespace dbuilder

