#pragma once
/**
 * @file   MoveItemCommand.h
 *
 * @date   Jan 16, 2013
 * @author Sam Roth <>
 */

#include <qundostack.h>
#include <QPointF>
#include "CoreForward.hpp"

namespace dbuilder {
class MoveItemCommand: public QUndoCommand
{
	DiagramItem *_item;
	QPointF _prevPos, _nextPos;
public:
	MoveItemCommand(DiagramItem *item, QPointF prevPos, QPointF nextPos, QUndoCommand *parent=0);

	void undo();
	void redo();

	virtual ~MoveItemCommand();
};
} // namespace dbuilder

