#pragma once
/**
 * @date   Jul 19, 2013
 * @author Sam Roth <>
 */

#include <QDialog>
#include <boost/optional.hpp>

class Ui_ExportComponentOptions;

namespace dbuilder
{


class ExportComponentOptions: public QDialog
{
	Ui_ExportComponentOptions *_ui;
public:
	struct Options
	{
		QString author;
		QString componentName;
	};

	ExportComponentOptions(QWidget *parent=nullptr);
	virtual ~ExportComponentOptions();

	Options options() const;


	static boost::optional<Options> getOptions(QWidget *parent=nullptr);
};

} /* namespace dbuilder */
