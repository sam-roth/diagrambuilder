#pragma once
#include <QUndoCommand>
/**
 * @date   Jul 17, 2013
 * @author Sam Roth <>
 */


namespace dbuilder {

class FunctionUndoCommand: public QUndoCommand
{
public:
	typedef std::function<void()> FunctionT;
private:
	FunctionT _undo, _redo;
public:
	FunctionUndoCommand(FunctionT redo,
	                    FunctionT undo,
	                    QUndoCommand *parent=nullptr)
	: QUndoCommand(parent)
	, _undo(std::move(undo))
	, _redo(std::move(redo))
	{

	}

	void undo()
	{
		_undo();
	}

	void redo()
	{
		_redo();
	}
};

}  // namespace dbuilder



