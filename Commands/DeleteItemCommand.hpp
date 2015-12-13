#pragma once
#include <QUndoCommand>
#include <QList>
#include "CoreForward.hpp"
/**
 * @file   DeleteItemCommand.h
 *
 * @date   Jan 4, 2013
 * @author Sam Roth <>
 */

namespace dbuilder {
class DeleteItemCommand: public QUndoCommand
{
	DiagramScene *_scene;
	DiagramItem *_item;
	QList<DiagramItem *> _itemsRemoved;
public:
	DeleteItemCommand(DiagramScene *scene, DiagramItem *item, QUndoCommand *parent=nullptr);
	void undo();
	void redo();
	virtual ~DeleteItemCommand();
};
} // namespace dbuilder

