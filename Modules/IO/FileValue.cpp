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