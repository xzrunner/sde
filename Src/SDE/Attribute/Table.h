#ifndef _IS_SDE_ATTRIBUTE_TABLE_H_
#define _IS_SDE_ATTRIBUTE_TABLE_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace Attribute
	{
		class Fields;
		class StaticRow;

		class Node : public Tools::ISerializable
		{
		public:
			Node(size_t capacity);
			~Node() {}

			//
			// Tools::ISerializable interface
			//
			virtual size_t getByteArraySize() const;
			virtual void loadFromByteArray(const byte* data);
			virtual void storeToByteArray(byte** data, size_t& len) const;

//			size_t getCapacity() const { return m_capacity; }
			bool insertData(id_type id, size_t len, const byte* pData);
			
		private:
			size_t m_capacity;
			std::map<id_type, StaticRow*> m_GOIDToRow;

			friend class Table;

		}; // Node

		// A table is a collection of rows, which have attributes
		// stored in columns. 
		class Table : public INonSpatialIndex<id_type>
		{
		public:
			Table(size_t nodeCapacity);
			~Table() {}

			//
			// ITable interface
			//
			virtual void insertData(const id_type& key, size_t len, const byte* pData);
			virtual bool deleteData(const id_type& key);
			virtual bool equalsQuery(const id_type& key, IRow& r);

		private:
			id_type writeNode(Node*);
			Node* readNode(id_type id);

		private:
			size_t m_nodeCapacity;
			id_type m_header;
			Node* m_currNode;
			id_type m_lastKey;
			std::map<id_type, id_type> m_GOIDToNodeID;

			IStorageManager* m_pStorageManager;
			Fields* m_columnsInfo;

		}; // Table
	}
}

#endif // _IS_SDE_ATTRIBUTE_TABLE_H_