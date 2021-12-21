#pragma once
#include <vector>
#include <string>

namespace SEval
{
	// token type
	enum class _internal_type
	{
		OPERATOR,
		OPERAND,
		FUNCTION,
		CONSTANT,
		NUMBER,
		BRACKET
	};
	// changeable token value
	struct operand
	{
		char symbol;
		double value;
	};
	// value used in evaluation
	struct token
	{
		_internal_type type;
		int indexValue = 0;
		double value = 0;

		bool operator!=(token const& other)
		{
			if (this->type == other.type && this->indexValue == other.indexValue && this->value == other.value)
				return false;
			return true;
		}
		bool operator==(token const& other)
		{
			if (this->type == other.type && this->indexValue == other.indexValue && this->value == other.value)
				return true;
			return false;
		}
	};

	// parses string to tokens
	std::vector<token> parseString(std::string function, bool correctInput = true, int* errorCode = nullptr);
	// returns operands from token vector
	std::vector<operand> getOperands(const std::vector<token>& tokens);
	// evaluates tokens
	double evaluate(const std::vector<token>& tokens, int* errorCode = nullptr);
	double evaluate(const std::vector<token>& tokens, const std::vector<operand>& operands, int* errorCode = nullptr);
	// converts error code to string
	std::string getErrorString(int errorCode);
}