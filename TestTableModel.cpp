/**
 * @file   TestTableModel.cpp
 *
 * @date   Jan 24, 2013
 * @author Sam Roth <>
 */

#include "TestTableModel.hpp"
#include <QStringListModel>

namespace dbuilder {



QAbstractItemModel *makeTestTableModel()
{
	auto model = new QStringListModel;
	model->insertRows(0, 3);
	model->setData(model->index(0, 0), "a");
	model->setData(model->index(1, 0), "b");
	model->setData(model->index(2, 0), "c");

	return model;
}

}  // namespace dbuilder


