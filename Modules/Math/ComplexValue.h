#pragma once

#include "Value.h"

class ComplexValue : public Powder::Value
{
public:
	ComplexValue();
	ComplexValue(double real, double imag);
	virtual ~ComplexValue();

	virtual std::string ToString() const override;
	virtual Value* CombineWith(const Value* value, Powder::MathInstruction::MathOp mathOp, Powder::Executor* executor) const override;

	double GetMagnitude() const;
	double GetSquareMagnitude() const;
	void SetCopy(const ComplexValue* complexValue);
	void SetSum(const ComplexValue* leftComplexValue, const ComplexValue* rightComplexValue);
	void SetDifference(const ComplexValue* leftComplexValue, const ComplexValue* rightComplexValue);
	void SetProduct(const ComplexValue* leftComplexValue, const ComplexValue* rightComplexValue);
	bool SetQuotient(const ComplexValue* leftComplexValue, const ComplexValue* rightComplexValue);
	void SetConjugate(const ComplexValue* complexValue);
	bool SetInverse(const ComplexValue* complexValue);
	void SetExponent(const ComplexValue* complexValue);
	void SetLog(const ComplexValue* complexValue);

	double real, imag;
};