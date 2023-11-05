#include "MatrixValue.h"

MatrixValue::MatrixValue()
{
}

MatrixValue::MatrixValue(int rows, int cols)
{
}

/*virtual*/ MatrixValue::~MatrixValue()
{
}

/*virtual*/ std::string MatrixValue::ToString() const
{
	return "";
}

/*virtual*/ Powder::Value* MatrixValue::CombineWith(const Value* value, Powder::MathInstruction::MathOp mathOp, Powder::Executor* executor) const
{
	return nullptr;
}

void MatrixValue::Transpose()
{
}

bool MatrixValue::Invert()
{
	return false;
}

double MatrixValue::Determinant() const
{
	return 0.0;
}