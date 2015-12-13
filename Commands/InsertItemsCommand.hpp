#include "DiagramItem.hpp"
#include "DiagramScene.hpp"
#include <qundostack.h>
#pragma once
/**
 * @file   InsertItemsCommand.hpp
 *
 * @date   Mar 8, 2013
 * @author Sam Roth <>
 */

namespace dbuilder
{

class InsertItemsCommand: public QUndoCommand
{
	DiagramScene *_scene;
	QList<DiagramItem *> _items;
	bool _done;
public:
	InsertItemsCommand(DiagramScene *scene, QList<DiagramItem *> items, QUndoCommand *parent=0);

	void undo();
	void redo();

	virtual ~InsertItemsCommand();
};

} /* namespace dbuilder */
