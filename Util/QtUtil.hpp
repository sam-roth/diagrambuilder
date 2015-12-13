#pragma once
/**
 * @file   QtUtil.hpp
 *
 * @date   Jan 30, 2013
 * @author Sam Roth <>
 */
class QAction;
class QObject;

namespace dbuilder {

/**
 * Reads a file or resource into a QString using QFile.
 * @param filename
 * @return
 */
QString readFile(const QString &filename);

}  // namespace dbuilder
