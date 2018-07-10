#include "Statistics.h"

using namespace IS_SDE::SpatialIndex::QuadTree;

Statistics::Statistics()
	: BaseTreeStatistics()
{
	reset();
}

Statistics::Statistics(const Statistics& s)
	: BaseTreeStatistics(s)
{
	m_duplicateTotal = s.m_duplicateTotal;
	m_duplicateObj = s.m_duplicateObj;
	m_split = s.m_split;
	m_treeRebuild = s.m_treeRebuild;
}

Statistics::~Statistics()
{
}

Statistics& Statistics::operator=(const Statistics& s)
{
	if (this != &s)
	{
		BaseTreeStatistics::operator = (s); 
		m_duplicateTotal = s.m_duplicateTotal;
		m_duplicateObj = s.m_duplicateObj;
		m_split = s.m_split;
		m_treeRebuild = s.m_treeRebuild;
	}

	return *this;
}

size_t Statistics::getDuplicateTotal() const
{
	return m_duplicateTotal;
}

size_t Statistics::getDuplicateObj() const
{
	return m_duplicateObj;
}

size_t Statistics::getSplit() const
{
	return m_split;
}

size_t Statistics::getTreeRebuild() const
{
	return m_treeRebuild;
}

void Statistics::reset()
{
	resetIOInfo();
	resetTreeInfo();
	resetSearchInfo();
	m_treeRebuild = 0;
}

void Statistics::resetIOInfo()
{
	m_reads  = 0;
	m_writes = 0;
	m_hits   = 0;
	m_misses = 0;
}

void Statistics::resetTreeInfo()
{
	m_nodes  = 0;
	m_data = 0;
	m_duplicateTotal = 0;
	m_duplicateObj = 0;
	m_treeHeight = 0;
	m_nodesInLevel.clear();
	m_split = 0;
}

void Statistics::resetSearchInfo()
{
	m_queryResults = 0;
}

std::ostream& IS_SDE::SpatialIndex::QuadTree::operator<<(std::ostream& os, const Statistics& s)
{
	os	<< "Reads: " << s.m_reads << std::endl
		<< "Writes: " << s.m_writes << std::endl
		<< "Hits: " << s.m_hits << std::endl
		<< "Misses: " << s.m_misses << std::endl
		<< "Tree height: " << s.m_treeHeight << std::endl
		<< "Number of data: " << s.m_data << std::endl
		<< "Number of total duplicate: " << s.m_duplicateTotal << std::endl
		<< "Number of object duplicate: " << s.m_duplicateObj << std::endl
		<< "Number of nodes: " << s.m_nodes << std::endl
		<< "Number of split: " << s.m_split << std::endl
		<< "Number of rebuild tree: " << s.m_treeRebuild << std::endl;

	for (size_t cLevel = 0; cLevel < s.m_treeHeight; cLevel++)
	{
		os << "Level " << cLevel << " pages: " << s.m_nodesInLevel[cLevel] << std::endl;
	}

	os << "Query results: " << s.m_queryResults << std::endl;

	return os;
}

