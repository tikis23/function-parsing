#include "SEval.h"

#include <stack>
#include <functional>

namespace SEval
{
	// operators
	static double func_add (double a, double b) { return a + b; }
	static double func_sub (double a, double b) { return a - b; }
	static double func_mult(double a, double b) { return a * b; }
	static double func_div (double a, double b) { return a / b; }
	static double func_mod (int a, int b) { return fmod(a, b); }
	static double func_pow (double a, double b) { return pow(a, b); }
	static double func_root(double a, double b) { return pow(b, 1.0 / a); }
	static double func_log (double a, double b) { return log2(b)/log2(a); }		  

	// functions  
	static double func_sin (double a, double b) { return sin(a); };
	static double func_asin(double a, double b) { return asin(a); };
	static double func_cos (double a, double b) { return cos(a); };
	static double func_acos(double a, double b) { return acos(a); };
	static double func_tan (double a, double b) { return tan(a); };
	static double func_atan(double a, double b) { return atan(a); };
	static double func_ctg (double a, double b) { return tan(1.57079632679489661923 - a); };
	static double func_actg(double a, double b) { return 1.57079632679489661923 - atan(a); };
	static double func_abs (double a, double b) { return abs(a); };
	static double func_degToRad (double a, double b) { return a * 0.01745329251994329576; };
	static double func_radToDeg (double a, double b) { return a * 57.2957795130823208768; };	
	static double func_floor (double a, double b) { return floor(a); };	
	static double func_ceil  (double a, double b) { return ceil(a); };	
	
	
	// constants
	static double func_pi(double a, double b) { return 3.14159265358979323846; }
	static double func_e (double a, double b) { return 2.71828182845904523536; }

	// function list
	struct _internal_function
	{
		std::string name = "";
		std::function<double(double, double)> function;

		// operator priority
		int precedence = 0;

		// 0 - left; 1 - right
		int associativity = 0;
	};
	const _internal_function functionList[] =
	{
		{"sin"  , func_sin },
		{"asin" , func_asin},
		{"cos"  , func_cos },
		{"acos" , func_acos},
		{"tan"  , func_tan },
		{"atan" , func_atan},
		{"ctg"  , func_ctg },
		{"actg" , func_actg},
		{"abs"  , func_abs},
		{"dtr"  , func_degToRad},
		{"rtd"  , func_radToDeg},
		{"floor", func_floor},
		{"ceil" , func_ceil}
	};
	const _internal_function constantList[] =
	{
		{"pi", func_pi},
		{"e" , func_e }
	};
	const _internal_function operatorList[] =
	{
		{"+", func_add , 1, 0},
		{"-", func_sub , 1, 0},
		{"*", func_mult, 2, 0},
		{"/", func_div , 2, 0},
		{"%", func_mod , 2, 0},
		{"$", func_root, 3, 1},
		{"^", func_pow , 3, 0},
		{"l", func_log , 3, 1}
	};

	// helper functions
	int findFunction(const std::string& function, int startIndex)
	{
		for (int i = 0; i < sizeof(functionList)/sizeof(_internal_function); i++)
			if (function.compare(startIndex, functionList[i].name.length(), functionList[i].name) == 0)
				return i;
		return -1;
	}
	int findConstant(const std::string& function, int startIndex)
	{
		for (int i = 0; i < sizeof(constantList) / sizeof(_internal_function); i++)
			if (function.compare(startIndex, constantList[i].name.length(), constantList[i].name) == 0)
				return i;
		return -1;
	}
	int findOperator(const char op)
	{
		for (int i = 0; i < sizeof(operatorList) / sizeof(_internal_function); i++)
			if (op == operatorList[i].name[0])
				return i;
		return -1;
	}

	std::vector<token> parseString(std::string function, int* errorCode)
	{
		// if empty return
		if (function.empty())
			return std::vector<token>();

		// correct format 		
		for (int i = 0; i < function.size(); i++)
		{
			// remove spaces
			if (isspace(function[i]))
			{
				function.erase(function.begin() + i);
				i--;
				continue;
			}

			// + && - correction
			if (i + 1 < function.size())
			{
				if (function[i] == '+' && (function[i + 1] == '+' || function[i + 1] == '-'))
				{
					function.erase(function.begin() + i);
					i--;
					continue;
				}
				if (function[i] == '-' && (function[i + 1] == '+' || function[i + 1] == '-'))
				{
					function.erase(function.begin() + i);
					if (function[i] == '-')
						function[i] = '+';
					else
						function[i] = '-';
					i--;
					continue;
				}
			}

			// correct decimal point
			if (function[i] == ',')
				function[i] = '.';
			if (i + 1 < function.size() && function[i] == '.' && !isdigit(function[i + 1]))
				function.insert(function.begin() + i + 1, 0);
		}

		// parse string to tokens
		std::vector<token> tokens;
		std::string tempWord;
		for (int i = 0; i < function.size(); i++)
		{
			// if operator
			int operatorIndex = findOperator(function[i]);
			if (operatorIndex >= 0)
			{
				tokens.push_back({ _internal_type::OPERATOR, operatorIndex, 0 });
				continue;
			}

			// if number
			if (isdigit(function[i]))
			{
				bool decimal = false;
				std::string tempNumber = { function[i] };

				// find digits
				while (i + 1 < function.size() && (isdigit(function[i + 1]) || function[i + 1] == '.'))
				{
					if (function[i + 1] == '.')
					{
						// error 1
						if (decimal)
						{
							if (errorCode != nullptr)
								*errorCode = 1;
							return std::vector<token>();
						}
						decimal = true;
					}
					tempNumber.push_back(function[i + 1]);
					i++;
				}

				tokens.push_back({ _internal_type::NUMBER, 0, std::stod(tempNumber) });
				continue;
			}

			// if letter
			if (isalpha(function[i]))
			{
				// check for function
				int functionIndex = findFunction(function, i);
				if (functionIndex >= 0)
				{
					tokens.push_back({ _internal_type::FUNCTION, functionIndex, 0 });
					i += functionList[functionIndex].name.length() - 1;
					continue;
				}

				// check for constant
				int constantIndex = findConstant(function, i);
				if (constantIndex >= 0)
				{
					tokens.push_back({ _internal_type::CONSTANT, constantIndex, 0 });
					i += constantList[constantIndex].name.length() - 1;
					continue;
				}

				// otherwise it is operand
				tokens.push_back({ _internal_type::OPERAND, function[i], 0 });
			}

			// if bracket
			if (function[i] == '(')
				tokens.push_back({ _internal_type::BRACKET, 0, 0 });			
			else if (function[i] == ')')
				tokens.push_back({ _internal_type::BRACKET, 1, 0 });
		}

		// correction in tokens
		for (int i = 0; i < tokens.size(); i++)
		{
			// if current token is operand or constant or number
			if ((tokens[i].type == SEval::_internal_type::OPERAND || tokens[i].type == SEval::_internal_type::CONSTANT 
				|| tokens[i].type == SEval::_internal_type::NUMBER || (tokens[i].type == SEval::_internal_type::BRACKET && tokens[i].indexValue == 1))
				&& i + 1 < tokens.size())
			{
				// if next token is operand or constant or left bracket
				if (tokens[i + 1].type == SEval::_internal_type::OPERAND || tokens[i + 1].type == SEval::_internal_type::CONSTANT
					|| (tokens[i + 1].type == SEval::_internal_type::BRACKET && tokens[i + 1].indexValue == 0))
				{
					tokens.insert(tokens.begin() + i + 1, token{ SEval::_internal_type::OPERATOR, 2, 0 });
					i--;
					continue;
				}
			}

			// if previous token is operator
			if (i - 1 >= 0 && tokens[i - 1].type == SEval::_internal_type::OPERATOR)
			{
				// if operator
				if (tokens[i].type == SEval::_internal_type::OPERATOR)
				{
					// if current token is -
					if (tokens[i].indexValue == 1)
					{
						tokens[i] = token{ SEval::_internal_type::NUMBER, 0, -1.0 };
						tokens.insert(tokens.begin() + i + 1, token{ SEval::_internal_type::OPERATOR, 2, 0 });
						i--;
						continue;
					}
					// if current token is +
					if (tokens[i].indexValue == 0)
					{
						tokens.erase(tokens.begin() + i);
						i--;
						continue;
					}
				}
			}

			// if current token is operator and previous token is left bracket or index == 0
			if (tokens[i].type == SEval::_internal_type::OPERATOR && 
				(i == 0 || (i - 1 >= 0 && tokens[i - 1].type == SEval::_internal_type::BRACKET && tokens[i - 1].indexValue == 0)))
			{
				// if current token is -
				if (tokens[i].indexValue == 1)
				{
					tokens[i] = token{ SEval::_internal_type::NUMBER, 0, -1.0 };
					tokens.insert(tokens.begin() + i + 1, token{ SEval::_internal_type::OPERATOR, 2, 0 });
					i--;
					continue;
				}
				// if current token is +
				if (tokens[i].indexValue == 0)
				{
					tokens.erase(tokens.begin() + i);
					i--;
					continue;
				}
			}
		}

		// shunting yard algorithm
		std::vector<token> output;
		std::stack<token> operatorStack;
		for (int i = 0; i < tokens.size(); i++)
		{
			// if token is number, operand or constant push to output
			if (tokens[i].type == SEval::_internal_type::NUMBER || tokens[i].type == SEval::_internal_type::OPERAND
				|| tokens[i].type == SEval::_internal_type::CONSTANT)
			{
				output.push_back(tokens[i]);
				continue;
			}

			// if token is function push to operator stack
			if (tokens[i].type == SEval::_internal_type::FUNCTION)
			{
				operatorStack.push(tokens[i]);
				continue;
			}

			// if token is operator o1
			if (tokens[i].type == SEval::_internal_type::OPERATOR)
			{
				while ((!operatorStack.empty() && operatorStack.top() != token{ SEval::_internal_type::BRACKET, 0, 0 })
					&& (operatorList[operatorStack.top().indexValue].precedence > operatorList[tokens[i].indexValue].precedence
						|| (operatorList[operatorStack.top().indexValue].precedence == operatorList[tokens[i].indexValue].precedence
							&& operatorList[tokens[i].indexValue].associativity == 0)))
				{
					// push operator o2 to operator stack
					output.push_back(operatorStack.top());
					operatorStack.pop();
				}
				operatorStack.push(tokens[i]);
				continue;
			}

			// if token is left bracket
			if (tokens[i].type == SEval::_internal_type::BRACKET && tokens[i].indexValue == 0)
			{
				operatorStack.push(tokens[i]);
				continue;
			}

			// if token is right bracket
			if (tokens[i].type == SEval::_internal_type::BRACKET && tokens[i].indexValue == 1)
			{
				while (!operatorStack.empty() && operatorStack.top() != token{ SEval::_internal_type::BRACKET, 0, 0 })
				{
					output.push_back(operatorStack.top());
					operatorStack.pop();
				}

				// if left bracket at operator stack top
				if (!operatorStack.empty() && operatorStack.top() == token{ SEval::_internal_type::BRACKET, 0, 0 })
				{
					operatorStack.pop();
				
				}// if operator stack is empty without left bracket error 2
				else if (operatorStack.empty())
				{
					if (errorCode != nullptr)
						*errorCode = 2;
					return std::vector<token>();
				}

				// if operator stack top is function
				if (!operatorStack.empty() && operatorStack.top().type == SEval::_internal_type::FUNCTION)
				{
					output.push_back(operatorStack.top());
					operatorStack.pop();
				}

				continue;
			}
		}

		// empty out operator stack
		while (!operatorStack.empty())
		{
			// if operator stack top is a bracket error 2
			if (operatorStack.top().type == SEval::_internal_type::BRACKET)
			{
				if (errorCode != nullptr)
					*errorCode = 2;
				return std::vector<token>();
			}
			output.push_back(operatorStack.top());
			operatorStack.pop();
		}

		return output;
	}
	std::vector<operand> getOperands(const std::vector<token>& tokens)
	{
		std::vector<operand>output;
		for (int i = 0; i < tokens.size(); i++)
		{
			if (tokens[i].type == SEval::_internal_type::OPERAND)
			{
				bool exists = false;
				for (int j = 0; j < output.size(); j++)
				{
					if (output[j].symbol == tokens[i].indexValue)
					{
						exists = true;
						break;
					}
				}
				if(!exists)
					output.push_back({(char)tokens[i].indexValue, 0});
			}
		}
		return output;
	}
	double evaluate(const std::vector<token>& tokens, int* errorCode)
	{
		std::stack<double>output;

		for (int i = 0; i < tokens.size(); i++)
		{
			// if token is bracket or operand error 3
			if (tokens[i].type == SEval::_internal_type::BRACKET || tokens[i].type == SEval::_internal_type::OPERAND)
			{
				if (errorCode != nullptr)
					*errorCode = 3;
				return 0;
			}

			// if token is number or constant
			if (tokens[i].type == SEval::_internal_type::NUMBER)
			{
				output.push(tokens[i].value);
				continue;
			}
			if (tokens[i].type == SEval::_internal_type::CONSTANT)
			{
				output.push(constantList[tokens[i].indexValue].function(0, 0));
				continue;
			}

			// if token is operator
			if (tokens[i].type == SEval::_internal_type::OPERATOR)
			{
				// if output is wrong error 3
				if (output.size() < 2)
				{
					if (errorCode != nullptr)
						*errorCode = 3;
					return 0;
				}

				// use operator on 2 top items from stack
				double b = output.top();
				output.pop();
				output.top() = operatorList[tokens[i].indexValue].function(output.top(), b);
				continue;
			}

			// if token is function
			if (tokens[i].type == SEval::_internal_type::FUNCTION)
			{
				// if output is wrong error 3
				if (output.empty())
				{
					if (errorCode != nullptr)
						*errorCode = 3;
					return 0;
				}

				// use function on stack top
				output.top() = functionList[tokens[i].indexValue].function(output.top(), 0);
				continue;
			}
		}

		// if output is wrong error 3
		if (output.size() != 1)
		{
			if (errorCode != nullptr)
				*errorCode = 3;
			return 0;
		}
		return output.top();
	}
	double evaluate(const std::vector<token>& tokens, const std::vector<operand>& operands, int* errorCode)
	{
		std::stack<double>output;

		for (int i = 0; i < tokens.size(); i++)
		{
			// if token is bracket error 3
			if (tokens[i].type == SEval::_internal_type::BRACKET)
			{
				if (errorCode != nullptr)
					*errorCode = 3;
				return 0;
			}

			// if token is number, constant or operand
			if (tokens[i].type == SEval::_internal_type::NUMBER)
			{
				output.push(tokens[i].value);
				continue;
			}
			if (tokens[i].type == SEval::_internal_type::CONSTANT)
			{
				output.push(constantList[tokens[i].indexValue].function(0, 0));
				continue;
			}
			if (tokens[i].type == SEval::_internal_type::OPERAND)
			{
				// find operand vector index
				int index = -1;
				for (int j = 0; j < operands.size(); j++)
				{
					if (tokens[i].indexValue == operands[j].symbol)
					{
						index = j;
						break;
					}
				}

				// if index out of range error 4
				if (index < 0 || index > operands.size() - 1)
				{
					if (errorCode != nullptr)
						*errorCode = 4;
					return 0;
				}

				output.push(operands[index].value);
				continue;
			}

			// if token is operator
			if (tokens[i].type == SEval::_internal_type::OPERATOR)
			{
				// if output is wrong error 3
				if (output.size() < 2)
				{
					if (errorCode != nullptr)
						*errorCode = 3;
					return 0;
				}

				// use operator on 2 top items from stack
				double b = output.top();
				output.pop();
				output.top() = operatorList[tokens[i].indexValue].function(output.top(), b);
				continue;
			}

			// if token is function
			if (tokens[i].type == SEval::_internal_type::FUNCTION)
			{
				// if output is wrong error 3
				if (output.empty())
				{
					if (errorCode != nullptr)
						*errorCode = 3;
					return 0;
				}

				// use function on stack top
				output.top() = functionList[tokens[i].indexValue].function(output.top(), 0);
				continue;
			}
		}

		// if output is wrong error 3
		if (output.size() != 1)
		{
			if (errorCode != nullptr)
				*errorCode = 3;
			return 0;
		}
		return output.top();
	}
	std::string getErrorString(int errorCode)
	{
		switch (errorCode)
		{
		case 1:
			return "decimal point mismatch";
		case 2:
			return "bracket mismatch";
		case 3:
			return "wrong input";
		case 4:
			return "undefined operand";
		}
	}
}