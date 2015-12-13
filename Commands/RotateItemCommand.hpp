#pragma once
/**
 * @file   TransformItemCommand.h
 *
 * @date   Jan 16, 2013
 * @author Sam Roth <>
 */

#include <qundostack.h>
#include <qglobal.h>
#include <QPoint>
#include "CoreForward.hpp"

namespace dbuilder {
class RotateItemCommand: public QUndoCommand
{
	DiagramScene *_scene;
	DiagramItem *_item;
	qreal _dRotation;
public:
	RotateItemCommand(DiagramScene *scene, DiagramItem *item, qreal dRotation, QUndoCommand *parent=0);

	void undo();
	void redo();

	virtual ~RotateItemCommand();
};
} // namespace dbuilder

