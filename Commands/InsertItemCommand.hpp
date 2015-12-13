#pragma once
/**
 * @file   InsertItemCommand.h
 *
 * @date   Jan 3, 2013
 * @author Sam Roth <>
 */

#include <QUndoCommand>
#include <QPoint>
#include "CoreForward.hpp"

namespace dbuilder {
class InsertItemCommand: public QUndoCommand
{
	DiagramScene *_scene;
	DiagramItem *_item;
public:
	InsertItemCommand(DiagramScene *scene, QString kind, QPointF pos, QUndoCommand *parent=nullptr);

	DiagramItem *item() const { return _item; }

	virtual void undo();
	virtual void redo();

	virtual ~InsertItemCommand();

};
} // namespace dbuilder

