/**
 * @date   Jul 27, 2013
 * @author Sam Roth <>
 */


#include "BasicPlugin.hpp"
#include "Util/StreamOps.hpp"
namespace dbuilder
{

std::ostream& operator <<(std::ostream& os, const PluginInfo& what)
{
	return os << what.name
	          << " by " << what.author
	          << " (" << what.url.toString() << ")"
	          << " v." << what.majorVersion << "."
	          << what.minorVersion;
}

}
