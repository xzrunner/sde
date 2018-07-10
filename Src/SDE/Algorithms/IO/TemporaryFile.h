#ifndef _IS_SDE_IO_ALGORITHMS_TEMPORARYFILE_H_
#define _IS_SDE_IO_ALGORITHMS_TEMPORARYFILE_H_
#include "../../Tools/Tools.h"

namespace IS_SDE
{
	namespace Algorithms
	{
		class TemporaryFile
		{
		public:
			TemporaryFile();
			virtual ~TemporaryFile();

			void storeNextObject(Tools::ISerializable* r);
			void storeNextObject(size_t len, const byte* const data);
			void loadNextObject(Tools::ISerializable* r);
			void loadNextObject(byte** data, size_t& len);

			void rewindForReading();
			void rewindForWriting();

		private:
			std::fstream m_file;
			std::vector<std::string> m_strFileName;
			size_t m_currentFile;
			size_t m_fileSize;
			bool m_bEOF;
		}; // TemporaryFile
	}
}

#endif // _IS_SDE_IO_ALGORITHMS_TEMPORARYFILE_H_ 

