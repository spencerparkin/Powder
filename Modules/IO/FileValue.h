#pragma once

#include "Value.h"
#include <fstream>

class FileValue : public Powder::Value
{
public:
	FileValue();
	virtual ~FileValue();

	std::fstream fileStream;
};