/**
 * @file   InsertItemsCommand.cpp
 *
 * @date   Mar 8, 2013
 * @author Sam Roth <>
 */

#include "InsertItemsCommand.hpp"

namespace dbuilder
{


InsertItemsCommand::InsertItemsCommand(DiagramScene *scene, QList<DiagramItem*> items,
		QUndoCommand* parent)
: _scene(scene)
, _items(items)
, _done(false)
{

}

void InsertItemsCommand::undo()
{
	if(_done)
	{
		_done = false;
		for(auto item : _items)
		{
			_scene->removeDiagramItem(item);
		}
	}
}

void InsertItemsCommand::redo()
{
	if(!_done)
	{
		_done = true;
		_scene->addDiagramItemsInOrder(_items);
	}
}

InsertItemsCommand::~InsertItemsCommand()
{
}

} /* namespace dbuilder */
