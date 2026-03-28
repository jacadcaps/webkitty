#include "config.h"
#include "MappedFileData.h"

namespace WTF::FileSystemImpl {

MappedFileData::MappedFileData(MallocSpan<uint8_t>&& fileData)
    : m_fileData(WTF::move(fileData))
{
}

MappedFileData::~MappedFileData()
{
}

} // namespace WTF::FileSystemImpl
