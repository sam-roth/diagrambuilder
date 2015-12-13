#pragma once
/**
 * @file   LineEdit.hpp
 *
 * @date   Mar 14, 2013
 * @author Sam Roth <>
 */

class QToolButton;

#include <QLineEdit>

namespace dbuilder {



class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LineEdit(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void updateCloseButton(const QString &text);

private:
    QToolButton *clearButton;
};


}  // namespace dbuilder
