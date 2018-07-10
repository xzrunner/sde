#include "SpatialDB.h"
using namespace IS_SDE;
using namespace IS_SDE::DBFeature;

void SpatialDB::open(const WString& filePath)
{
	
}

size_t SpatialDB::getByteArraySize() const
{
	size_t ret = 0;
	ret += sizeof(size_t);			// all num

	std::map<WString, size_t>::const_iterator itr = m_dataSrcName.begin(); 
	for ( ; itr != m_dataSrcName.end(); ++itr)
		ret += itr->first.getByteArraySize();

	return ret;
}

void SpatialDB::loadFromByteArray(const byte* data)
{
	size_t len = getByteArraySize(), nameNum;
	memcpy(&nameNum, data, sizeof(size_t));
	data += sizeof(size_t);

	for (size_t i = 0; i < nameNum; ++i)
	{
		WString wstr;
		wstr.loadFromByteArray(data);
		createDataSrc(wstr);
	}
}

void SpatialDB::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	size_t nameNum = m_dataSrcName.size();
	memcpy(ptr, &nameNum, sizeof(size_t));
	ptr += sizeof(size_t);

	std::vector<WString> vctWstr;
	getDataSrcList(&vctWstr);
	for (size_t i = 0; i < nameNum; ++i)
		vctWstr.at(i).storeToByteArray(ptr);
}

void SpatialDB::createDataSrc(const WString& name)
{
	assert(m_dataSrcName.find(name) == m_dataSrcName.end());
	m_dataSrcName.insert(std::make_pair(name, m_dataSrcName.size()));
}

void SpatialDB::deleteDataSrc(const WString& name)
{
	std::map<WString, size_t>::iterator itrFind = m_dataSrcName.find(name);
	assert(itrFind != m_dataSrcName.end());

	size_t deleteIndex = itrFind->second;
	std::map<WString, size_t>::iterator itr = m_dataSrcName.begin(); 
	for ( ; itr != m_dataSrcName.end(); ++itr)
		if (itr->second > deleteIndex)
			--itr->second;
	m_dataSrcName.erase(itrFind);
}

void SpatialDB::getDataSrcList(std::vector<WString>* result) const
{
	result->resize(m_dataSrcName.size());

	std::map<WString, size_t>::const_iterator itr = m_dataSrcName.begin(); 
	for ( ; itr != m_dataSrcName.end(); ++itr)
		result->at(itr->second) = itr->first;
}