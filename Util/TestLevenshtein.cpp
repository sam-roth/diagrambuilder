/**
 * @file   TestLevenshtein.cpp
 *
 * @date   Mar 13, 2013
 * @author Sam Roth <>
 */
#include "Levenshtein.hpp"
#include "Main/Application.hpp"
#include "Util/Log.hpp"
#include <iomanip>
#include <vector>

namespace dbuilder {

int levenshteinTest(int argc, char **argv)
{
	log::setLevel(log::Debug);
	const std::vector<std::string> strings = {
		"the quick brown fox",
		"the slow red hen",
		"something else"
	};

	if(argc > 1)
	{
		const std::string query = argv[1];
		DBInfo("Query: ", query);
		for(const auto &s : strings)
		{
			size_t dist = levenshteinDistance(s, query);
			double normalizedDist = double(dist) / (double(s.size()) - double(query.size()));
			DBInfo(std::setw(10), std::left, normalizedDist, "  ", s);
		}
		DBInfo("Done");
	}

	return 0;
}

//namespace { Application::ReplaceMain r{levenshteinTest}; }

}  // namespace dbuilder


