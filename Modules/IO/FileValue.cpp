#include "FileValue.h"

FileValue::FileValue()
{
}

/*virtual*/ FileValue::~FileValue()
{
	if (this->fileStream.is_open())
		this->fileStream.close();
}

/*virtual*/ bool FileValue::AsBoolean() const
{
	return this->fileStream.is_open();
}

/*virtual*/ std::string FileValue::ToString() const
{
	return "File Stream";
}

/*virtual*/ std::string FileValue::GetTypeString() const
{
	return "file";
}