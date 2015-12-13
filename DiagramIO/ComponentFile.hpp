#pragma once
#include <boost/property_tree/ptree.hpp>
/**
 * @date   Jul 19, 2013
 * @author Sam Roth <>
 */

namespace dbuilder {

class DiagramComponent;

struct ComponentSpec
{
	QString name;            /**< display name of component */

	QString svgData;         /**< SVG data (either specify this or svgFile) */
	QString svgFile;         /**< path to the SVG file (either specify this or svgData) */
	QList<QPointF> ports;    /**< port locations (relative to item) */

	DiagramComponent *createKind(QObject *parent) const;
};

class ComponentFile
{
	boost::property_tree::ptree pt;
public:
	ComponentFile(std::istream &is);
	ComponentFile();
	virtual ~ComponentFile();

	void setAuthor(const QString &author);
	QString author() const;

	void addComponent(const QString &name, const QString &svgData, const QList<QPointF> &ports);
	void add(const ComponentSpec &component);

	QList<ComponentSpec> components() const;

	const boost::property_tree::ptree &ptree() const { return pt; }
	void write(std::ostream &os) const;
	void read(std::istream &is);
private:
	ComponentSpec component(const boost::property_tree::ptree &subtree) const;
	boost::optional<const boost::property_tree::ptree &> findAbstract(const std::string &name) const;

};

} /* namespace dbuilder */
