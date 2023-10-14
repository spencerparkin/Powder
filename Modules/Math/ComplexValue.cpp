#include "ComplexValue.h"
#include "NumberValue.h"
#include "NullValue.h"

ComplexValue::ComplexValue()
{
	this->real = 0.0;
	this->imag = 0.0;
}

ComplexValue::ComplexValue(double real, double imag)
{
	this->real = real;
	this->imag = imag;
}

/*virtual*/ ComplexValue::~ComplexValue()
{
}

/*virtual*/ std::string ComplexValue::ToString() const
{
	return std::format("{} + {}*i", this->real, this->imag);
}

/*virtual*/ Powder::Value* ComplexValue::CombineWith(const Value* value, Powder::MathInstruction::MathOp mathOp, Powder::Executor* executor) const
{
	Powder::Value* resultValue = nullptr;

	// Unary operation?
	if (!value)
	{
		switch (mathOp)
		{
			case Powder::MathInstruction::MathOp::EXPONENTIATE:
			{
				ComplexValue* expValue = new ComplexValue();
				expValue->SetExponent(this);
				resultValue = expValue;
				break;
			}
			// TODO: We should also be able to take the natural logarithm of a complex number.
		}
	}

	// Consider binary operations...
	if (!resultValue)
	{
		auto complexValue = dynamic_cast<const ComplexValue*>(value);
		if (complexValue)
		{
			switch (mathOp)
			{
				case Powder::MathInstruction::MathOp::ADD:
				{
					ComplexValue* sumValue = new ComplexValue();
					sumValue->SetSum(this, complexValue);
					resultValue = sumValue;
					break;
				}
				case Powder::MathInstruction::MathOp::SUBTRACT:
				{
					ComplexValue* differenceValue = new ComplexValue();
					differenceValue->SetDifference(this, complexValue);
					resultValue = differenceValue;
					break;
				}
				case Powder::MathInstruction::MathOp::MULTIPLY:
				{
					ComplexValue* productValue = new ComplexValue();
					productValue->SetProduct(this, complexValue);
					resultValue = productValue;
					break;
				}
				case Powder::MathInstruction::MathOp::DIVIDE:
				{
					ComplexValue* quotientValue = new ComplexValue();
					if (quotientValue->SetQuotient(this, complexValue))
						resultValue = quotientValue;
					else
					{
						delete quotientValue;
						resultValue = new Powder::NullValue();
					}
					break;
				}
				case Powder::MathInstruction::EXPONENTIATE:
				{
					// TODO: Take one complex number to the power of another.
					break;
				}
				// TODO: We should also be able to take the log (base a complex number) of this complex number.
			}
		}

		auto realValue = dynamic_cast<const Powder::NumberValue*>(value);
		if (realValue)
		{
			switch (mathOp)
			{
				case Powder::MathInstruction::MathOp::ADD:
				{
					ComplexValue* sumValue = new ComplexValue();
					sumValue->SetCopy(this);
					sumValue->real += realValue->AsNumber();
					resultValue = sumValue;
					break;
				}
				case Powder::MathInstruction::MathOp::SUBTRACT:
				{
					ComplexValue* differenceValue = new ComplexValue();
					differenceValue->SetCopy(this);
					differenceValue->real -= realValue->AsNumber();
					resultValue = differenceValue;
					break;
				}
				case Powder::MathInstruction::MathOp::MULTIPLY:
				{
					ComplexValue* productValue = new ComplexValue();
					productValue->SetCopy(this);
					productValue->real *= realValue->AsNumber();
					productValue->imag *= realValue->AsNumber();
					resultValue = productValue;
					break;
				}
				case Powder::MathInstruction::MathOp::DIVIDE:
				{
					ComplexValue* quotientValue = new ComplexValue();
					quotientValue->SetCopy(this);
					quotientValue->real /= realValue->AsNumber();
					quotientValue->imag /= realValue->AsNumber();
					resultValue = quotientValue;
					break;
				}
				case Powder::MathInstruction::MathOp::EXPONENTIATE:
				{
					// TODO: Take this complex number to the power of the given number.
					break;
				}
			}
		}
	}

	return resultValue;
}

double ComplexValue::GetMagnitude() const
{
	return ::sqrt(this->GetSquareMagnitude());
}

double ComplexValue::GetSquareMagnitude() const
{
	return this->real * this->real + this->imag * this->imag;
}

void ComplexValue::SetCopy(const ComplexValue* complexValue)
{
	this->real = complexValue->real;
	this->imag = complexValue->imag;
}

void ComplexValue::SetSum(const ComplexValue* leftComplexValue, const ComplexValue* rightComplexValue)
{
	this->real = leftComplexValue->real + rightComplexValue->real;
	this->imag = leftComplexValue->imag + rightComplexValue->imag;
}

void ComplexValue::SetDifference(const ComplexValue* leftComplexValue, const ComplexValue* rightComplexValue)
{
	this->real = leftComplexValue->real - rightComplexValue->real;
	this->imag = leftComplexValue->imag - rightComplexValue->imag;
}

void ComplexValue::SetProduct(const ComplexValue* leftComplexValue, const ComplexValue* rightComplexValue)
{
	double localReal = leftComplexValue->real * rightComplexValue->real - leftComplexValue->imag * rightComplexValue->imag;
	double localImag = leftComplexValue->real * rightComplexValue->imag + leftComplexValue->imag * rightComplexValue->real;

	this->real = localReal;
	this->imag = localImag;
}

bool ComplexValue::SetQuotient(const ComplexValue* leftComplexValue, const ComplexValue* rightComplexValue)
{
	ComplexValue rightComplexValueInv;
	if (!rightComplexValueInv.SetInverse(rightComplexValue))
		return false;

	this->SetProduct(leftComplexValue, &rightComplexValueInv);
	return true;
}

void ComplexValue::SetConjugate(const ComplexValue* complexValue)
{
	this->real = complexValue->real;
	this->imag = -complexValue->imag;
}

bool ComplexValue::SetInverse(const ComplexValue* complexValue)
{
	double squareMag = complexValue->GetSquareMagnitude();
	if (squareMag == 0.0)
		return false;

	double scale = 1.0 / squareMag;
	if (::isnan(scale) || ::isinf(scale))
		return false;

	this->SetConjugate(complexValue);
	
	this->real *= scale;
	this->imag *= scale;

	return true;
}

void ComplexValue::SetExponent(const ComplexValue* complexValue)
{
	double expReal = ::exp(complexValue->real);
	double cosImag = ::cos(complexValue->imag);
	double sinImag = ::sin(complexValue->imag);

	this->real = expReal * cosImag;
	this->imag = expReal * sinImag;
}

void ComplexValue::SetLog(const ComplexValue* complexValue)
{
	double mag = complexValue->GetMagnitude();

	this->imag = ::atan2(complexValue->imag, complexValue->real);
	this->real = ::log(mag);
}