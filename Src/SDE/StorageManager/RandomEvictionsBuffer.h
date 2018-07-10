#ifndef _IS_SDE_STORAGEMANAGER_RANDOM_EVICTIONS_BUFFER_H_
#define _IS_SDE_STORAGEMANAGER_RANDOM_EVICTIONS_BUFFER_H_
#include "Buffer.h"

namespace IS_SDE
{
	namespace StorageManager
	{
		class RandomEvictionsBuffer : public Buffer
		{
		public:
			RandomEvictionsBuffer(IStorageManager&, Tools::PropertySet& ps);
			// see Buffer.h for available properties.

			virtual ~RandomEvictionsBuffer();

			virtual void addEntry(id_type id, Buffer::Entry* pEntry);
			virtual void removeEntry();
		}; // RandomEvictionsBuffer
	}
}

#endif // _IS_SDE_STORAGEMANAGER_RANDOM_EVICTIONS_BUFFER_H_
