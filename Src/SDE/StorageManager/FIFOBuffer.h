#ifndef _IS_SDE_STORAGEMANAGER_FIFO_BUFFER_H_
#define _IS_SDE_STORAGEMANAGER_FIFO_BUFFER_H_
#include "Buffer.h"

namespace IS_SDE
{
	namespace StorageManager
	{
		class FIFOBuffer : public Buffer
		{
		public:
			FIFOBuffer(IStorageManager&, Tools::PropertySet& ps);
			virtual ~FIFOBuffer();

			void addEntry(id_type id, Entry* pEntry);
			void removeEntry();

		private:
			std::queue<id_type> m_queue;
		}; // FIFOBuffer
	}
}

#endif // _IS_SDE_STORAGEMANAGER_FIFO_BUFFER_H_
