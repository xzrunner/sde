#include "Leaf.h"
#include "MapPartition.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Network;

//
// ISerializable interface
//

size_t Leaf::getByteArraySize() const
{
	size_t ret = GRID_ID_SIZE + PARTITION_SIZE;

	ret += m_verPartition.size() * sizeof(double);

	ret += m_horPartition.size() * PARTITION_SIZE;
	for (size_t i = 0; i < m_horPartition.size(); ++i)
		ret += m_horPartition.at(i).size() * sizeof(double);

	return ret;
}

void Leaf::loadFromByteArray(const byte* data)
{
	memcpy(&m_firstGridID, data, GRID_ID_SIZE);
	data += GRID_ID_SIZE;

	size_t verSize = 0;
	memcpy(&verSize, data, PARTITION_SIZE);
	data += PARTITION_SIZE;
	m_verPartition.reserve(verSize);
	m_horPartition.reserve(verSize + 1);

	double tmp;
	for (size_t i = 0; i < verSize; ++i)
	{
		memcpy(&tmp, data, sizeof(double));
		data += sizeof(double);
		m_verPartition.push_back(tmp);

		std::vector<double> horPartition;
		size_t horSize = 0;
		memcpy(&horSize, data, PARTITION_SIZE);
		data += PARTITION_SIZE;
		horPartition.reserve(horSize);
		for (size_t j = 0; j < horSize; ++j)
		{
			memcpy(&tmp, data, sizeof(double));
			data += sizeof(double);
			horPartition.push_back(tmp);
		}
		m_horPartition.push_back(horPartition);
	}

	std::vector<double> horPartition;
	size_t horSize = 0;
	memcpy(&horSize, data, PARTITION_SIZE);
	data += PARTITION_SIZE;
	horPartition.reserve(horSize);
	for (size_t j = 0; j < horSize; ++j)
	{
		memcpy(&tmp, data, sizeof(double));
		data += sizeof(double);
		horPartition.push_back(tmp);
	}
	m_horPartition.push_back(horPartition);
}

void Leaf::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	memcpy(ptr, &m_firstGridID, GRID_ID_SIZE);
	ptr += GRID_ID_SIZE;

	size_t verSize = m_verPartition.size();
	assert(verSize + 1 == m_horPartition.size());
	memcpy(ptr, &verSize, PARTITION_SIZE);
	ptr += PARTITION_SIZE;

	for (size_t i = 0; i < verSize; ++i)
	{
		memcpy(ptr, &m_verPartition.at(i), sizeof(double));
		ptr += sizeof(double);

		size_t horSize = m_horPartition.at(i).size();
		memcpy(ptr, &horSize, PARTITION_SIZE);
		ptr += PARTITION_SIZE;
		for (size_t j = 0; j < horSize; ++j)
		{
			memcpy(ptr, &m_horPartition.at(i).at(j), sizeof(double));
			ptr += sizeof(double);
		}
	}

	size_t horSize = m_horPartition.at(verSize).size();
	memcpy(ptr, &horSize, PARTITION_SIZE);
	ptr += PARTITION_SIZE;

	for (size_t j = 0; j < horSize; ++j)
	{
		memcpy(ptr, &m_horPartition.at(verSize).at(j), sizeof(double));
		ptr += sizeof(double);
	}
}

//
// Node interface
//

void Leaf::split(PartitionMap& pm)
{
	throw Tools::IllegalStateException(
		"Leaf::split: should never be called. "
		);
}

void Leaf::queryAdjacencyListID(const MapPos2D& p, std::vector<size_t>* IDs) const
{
	if (!m_bBuilt)
		return;

	assert(PartitionMap::isPointInScope(p, m_scope));
	size_t currID = m_firstGridID;

	for (size_t i = 0; i < m_verPartition.size(); ++i)
	{
		if (p.d0 > m_verPartition.at(i))
			currID += (m_horPartition.at(i).size() + 1);
		else
		{
			for (size_t j = 0; j < m_horPartition.at(i).size(); ++j)
			{
				if (p.d1 > m_horPartition.at(i).at(j))
					++currID;
				else
				{
					IDs->push_back(currID);
					if (p.d1 == m_horPartition.at(i).at(j))
						IDs->push_back(currID + 1);
					currID += m_horPartition.at(i).size() - j;
					break;
				}
			}

			IDs->push_back(currID);

			++currID;
			if (p.d0 != m_verPartition.at(i))
				return;
		}
	}

	for (size_t i = 0; i < m_horPartition.back().size(); ++i)
	{
		if (p.d1 > m_horPartition.back().at(i))
			++currID;
		else
		{
			IDs->push_back(currID);
			if (p.d1 == m_horPartition.back().at(i))
				IDs->push_back(currID + 1);
			return;
		}
	}

	IDs->push_back(currID);
}

void Leaf::queryAdjacencyListID(const MapPos2D& p, const byte* data, std::vector<size_t>* IDs)
{
	size_t currID;
	memcpy(&currID, data, GRID_ID_SIZE);
	data += GRID_ID_SIZE;

	size_t verSize = 0;
	memcpy(&verSize, data, PARTITION_SIZE);
	data += PARTITION_SIZE;
	for (size_t i = 0; i < verSize; ++i)
	{
		double verPartition;
		memcpy(&verPartition, data, sizeof(double));
		data += sizeof(double);

		size_t horSize = 0;
		memcpy(&horSize, data, PARTITION_SIZE);
		data += PARTITION_SIZE;

		if (p.d0 > verPartition)
		{
			data += horSize * sizeof(double);
			currID += (horSize + 1);
		}
		else
		{
			for (size_t j = 0; j < horSize; ++j)
			{
				double horPartition;
				memcpy(&horPartition, data, sizeof(double));
				data += sizeof(double);
				if (p.d1 > horPartition)
					++currID;
				else
				{
					IDs->push_back(currID);
					if (p.d1 == horPartition)
						IDs->push_back(currID + 1);
					currID += horSize - j;
					data += (horSize - j) * sizeof(double);
					break;
				}
			}

			IDs->push_back(currID);

			++currID;
			if (p.d0 != verPartition)
				return;
		}
	}

	size_t horSize = 0;
	memcpy(&horSize, data, PARTITION_SIZE);
	data += PARTITION_SIZE;
	for (size_t i = 0; i < horSize; ++i)
	{
		double horPartition;
		memcpy(&horPartition, data, sizeof(double));
		data += sizeof(double);
		if (p.d1 > horPartition)
			++currID;
		else
		{
			IDs->push_back(currID);
			if (p.d1 == horPartition)
				IDs->push_back(currID + 1);
			return;
		}
	}
	IDs->push_back(currID);
}

void Leaf::getAllAdjacencyListIDs(const byte* data, size_t& startID, size_t& num)
{
	startID = 0;
	memcpy(&startID, data, GRID_ID_SIZE);
	data += GRID_ID_SIZE;

	num = 0;
	size_t verSize = 0;
	memcpy(&verSize, data, PARTITION_SIZE);
	data += PARTITION_SIZE;
	for (size_t i = 0; i < verSize; ++i)
	{
		data += sizeof(double);

		size_t horSize = 0;
		memcpy(&horSize, data, PARTITION_SIZE);
		data += PARTITION_SIZE;
		num += horSize + 1;

		data += sizeof(double) * horSize;
	}

	size_t horSize = 0;
	memcpy(&horSize, data, PARTITION_SIZE);
	//data += PARTITION_SIZE;
	num += horSize + 1;
}

void Leaf::setID(size_t page, size_t offset)
{
	assert(page <= (1 << GRID_SIZE_BIT) - 1
		&& offset <= (1 << OFFSET_SIZE_BIT) - 1);
	m_id = page << OFFSET_SIZE_BIT | offset;
}