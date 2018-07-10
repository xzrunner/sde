#include "Statistics.h"

using namespace IS_SDE::SpatialIndex::NVRTree;

Statistics::Statistics()
	: BaseTreeStatistics()
{
	reset();
}

Statistics::Statistics(const Statistics& s)
	: BaseTreeStatistics(s)
{
}

Statistics::~Statistics()
{
}

Statistics& Statistics::operator=(const Statistics& s)
{
	if (this != &s)
		BaseTreeStatistics::operator = (s); 

	return *this;
}

void Statistics::reset()
{
	BaseTreeStatistics::reset();
}

std::ostream& IS_SDE::SpatialIndex::NVRTree::operator << (std::ostream& os, const Statistics& s)
{
	os	<< "Reads: " << s.m_reads << std::endl
		<< "Writes: " << s.m_writes << std::endl
		<< "Hits: " << s.m_hits << std::endl
		<< "Misses: " << s.m_misses << std::endl
		<< "Tree height: " << s.m_treeHeight << std::endl
		<< "Number of data: " << s.m_data << std::endl
		<< "Number of nodes: " << s.m_nodes << std::endl;

	for (size_t cLevel = 0; cLevel < s.m_treeHeight; cLevel++)
	{
		os << "Level " << cLevel << " pages: " << s.m_nodesInLevel[cLevel] << std::endl;
	}

	os << "Query results: " << s.m_queryResults << std::endl;

	return os;
}

