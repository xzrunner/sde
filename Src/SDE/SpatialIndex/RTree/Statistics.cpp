#include "Statistics.h"

using namespace IS_SDE::SpatialIndex::RTree;

Statistics::Statistics()
	: BaseTreeStatistics()
{
	reset();
}

Statistics::Statistics(const Statistics& s)
	: BaseTreeStatistics(s)
{
	m_splits = s.m_splits;
	m_adjustments = s.m_adjustments;
}

Statistics::~Statistics()
{
}

Statistics& Statistics::operator=(const Statistics& s)
{
	if (this != &s)
	{
		BaseTreeStatistics::operator = (s); 
		m_splits = s.m_splits;
		m_adjustments = s.m_adjustments;
	}

	return *this;
}

size_t Statistics::getSplits() const
{
	return m_splits;
}

size_t Statistics::getAdjustments() const
{
	return m_adjustments;
}

void Statistics::reset()
{
	BaseTreeStatistics::reset();
	m_splits = 0;
	m_adjustments = 0;
}

std::ostream& IS_SDE::SpatialIndex::RTree::operator<<(std::ostream& os, const Statistics& s)
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

	os	<< "Splits: " << s.m_splits << std::endl
		<< "Adjustments: " << s.m_adjustments << std::endl
		<< "Query results: " << s.m_queryResults << std::endl;

	return os;
}

