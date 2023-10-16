#include "Error.h"

using namespace Powder;

Error::Error()
{
	this->errorMessageArray = new std::vector<std::string>();
}

/*virtual*/ Error::~Error()
{
	delete this->errorMessageArray;
}

Error::operator bool() const
{
	return this->errorMessageArray->size() > 0;
}

Error::operator std::string() const
{
	std::string errorListMsg;

	for (const std::string errorMsg : *this->errorMessageArray)
		errorListMsg += errorMsg + "\n";

	return errorListMsg;
}

void Error::Clear()
{
	this->errorMessageArray->clear();
}

void Error::Add(const std::string& errorMessage)
{
	this->errorMessageArray->push_back(errorMessage);
}

int Error::GetCount()
{
	return this->errorMessageArray->size();
}