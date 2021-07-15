#pragma once

#include "Value.h"
#include <fstream>

class FileValue : public Powder::Value
{
public:
	FileValue();
	virtual ~FileValue();

	virtual bool AsBoolean() const override;
	virtual std::string ToString() const override;

	std::fstream fileStream;
};