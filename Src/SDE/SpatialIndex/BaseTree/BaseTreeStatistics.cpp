#include "BaseTreeStatistics.h"

using namespace IS_SDE::SpatialIndex;

BaseTreeStatistics::BaseTreeStatistics()
{
	reset();
}

BaseTreeStatistics::BaseTreeStatistics(const BaseTreeStatistics& s)
{
	m_reads  = s.m_reads;
	m_writes = s.m_writes;
	m_hits   = s.m_hits;
	m_misses = s.m_misses;
	m_nodes  = s.m_nodes;
	m_queryResults = s.m_queryResults;
	m_data = s.m_data;
	m_treeHeight = s.m_treeHeight;
	m_nodesInLevel = s.m_nodesInLevel;
}

BaseTreeStatistics::~BaseTreeStatistics()
{
}

BaseTreeStatistics& BaseTreeStatistics::operator = (const BaseTreeStatistics& s)
{
	if (this != &s)
	{
		m_reads  = s.m_reads;
		m_writes = s.m_writes;
		m_hits   = s.m_hits;
		m_misses = s.m_misses;
		m_nodes  = s.m_nodes;
		m_queryResults = s.m_queryResults;
		m_data = s.m_data;
		m_treeHeight = s.m_treeHeight;
		m_nodesInLevel = s.m_nodesInLevel;
	}

	return *this;
}

size_t BaseTreeStatistics::getReads() const
{
	return m_reads;
}

size_t BaseTreeStatistics::getWrites() const
{
	return m_writes;
}

size_t BaseTreeStatistics::getNumberOfNodes() const
{
	return m_nodes;
}

size_t BaseTreeStatistics::getNumberOfData() const
{
	return m_data;
}

size_t BaseTreeStatistics::getHits() const
{
	return m_hits;
}

size_t BaseTreeStatistics::getMisses() const
{
	return m_misses;
}

size_t BaseTreeStatistics::getQueryResults() const
{
	return m_queryResults;
}

size_t BaseTreeStatistics::getTreeHeight() const
{
	return m_treeHeight;
}

size_t BaseTreeStatistics::getNumberOfNodesInLevel(size_t l) const
{
	size_t cNodes;
	try
	{
		cNodes = m_nodesInLevel.at(l);
	}
	catch (...)
	{
		throw Tools::IndexOutOfBoundsException(l);
	}

	return cNodes;
}

void BaseTreeStatistics::reset()
{
	m_reads  = 0;
	m_writes = 0;
	m_hits   = 0;
	m_misses = 0;
	m_nodes  = 0;
	m_queryResults = 0;
	m_data = 0;
	m_treeHeight = 0;
	m_nodesInLevel.clear();
}
