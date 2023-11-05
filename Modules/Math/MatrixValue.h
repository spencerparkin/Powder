#pragma once

#include "Value.h"

class MatrixValue : public Powder::Value
{
public:
	MatrixValue();
	MatrixValue(int rows, int cols);
	virtual ~MatrixValue();

	virtual std::string ToString() const override;
	virtual Value* CombineWith(const Value* value, Powder::MathInstruction::MathOp mathOp, Powder::Executor* executor) const override;

	void Transpose();
	bool Invert();
	double Determinant() const;

private:
	double** element;
	int rows, cols;
};