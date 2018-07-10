#ifndef _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_QUAD_TREE_TABLE_H_
#define _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_QUAD_TREE_TABLE_H_
#include "../Base.h"
#include "../../BasicType/Scope.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Utility
		{
			class QPNode;
		}

		namespace LookUpTable
		{
			class QuadTreeTable : public ILookUp
			{
			public:
				QuadTreeTable(IStorageManager& storage);
				QuadTreeTable(IStorageManager& tableStorage, IStorageManager& nwStorage, 
					size_t capacity, const MapScope& scope);
				virtual ~QuadTreeTable();

				virtual double queryMinDistance(const MapPos2D& sNodePos, size_t dGridID) const;

				virtual void loadTable(size_t destGridID, byte** data) const;
				virtual double queryMinDistance(const MapPos2D& src, const byte* table) const;

				virtual size_t getGridsCount() const { return m_gridNum; }
				virtual size_t getGridID(const MapPos2D& pos) const;
				virtual void outputIndexInfo(size_t& nextPage);

				virtual void drawTable(Render::GDIRender* render) const;

			public:
				static const size_t HEADER_PAGE		= 0;

				static const size_t NODE_ID_SIZE	= 2;
				static const size_t NODE_ID_GAP		= 0x7fff;
				static const size_t NODE_ID_CARRY	= 0x8000;
				static const size_t NODE_ID_ENDLESS	= 0xffff;

			private:
				class Node
				{
				public:
					Node(const Utility::QPNode* src);
					Node(byte** ptr);
					~Node();

					bool isLeaf() const { return m_child[0] == NULL; }
					void countAndSetIDInPreorder(size_t& c);

					void preorderTraversal(std::vector<const Node*>& list) const;

				private:
					Node* m_child[4];
					size_t m_id;

					friend class QuadTreeTable;

				}; // Node

			private:
				void storeHeader();
				void loadHeader();

				void initTableIndex(IStorageManager& nwStorage, size_t capacity, const MapScope& scope);

			private:
				IStorageManager* m_buffer;

				size_t m_gridNum;

				MapScope m_mapScope;
				size_t m_dataStartPage;
				Node* m_root;

			}; // QuadTreeTable
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_QUAD_TREE_TABLE_H_
