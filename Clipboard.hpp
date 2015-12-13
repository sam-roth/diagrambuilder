#pragma once
/**
 * @file   Clipboard.hpp
 *
 * @date   Jan 20, 2013
 * @author Sam Roth <>
 */

#include <QList>
#include "CoreForward.hpp"

namespace dbuilder {
void copyToClipboard(DiagramLoader *, const QList<DiagramItem *> &);
QList<DiagramItem *> pasteFromClipboard(DiagramLoader *, DiagramScene *);
} // namespace dbuilder




