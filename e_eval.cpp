#include "e_eval.h"

#include <iostream>
#include <stack>
#include <functional>

enum e_type
{
	e_operator_type,
	e_function_type,
	e_operand_type,
	e_constant_type,
	e_number_type
};

struct e_operator
{
	std::string sign;
	std::function<double(double, double)> function;
	int priority;
	int associativity;
};

static double e_add(double a, double b) { return a + b; }
static double e_sub(double a, double b) { return a - b; }
static double e_mult(double a, double b) { return a * b; }
static double e_div(double a, double b) { return a / b; }
static double e_pow(double a, double b) { return pow(a, b); }
static double e_mod(int a, int b) { return a % b; }
static double e_sin(double a, double b) { return sin(a); }
static double e_cos(double a, double b) { return cos(a); }
static double e_tan(double a, double b) { return tan(a); }

static double e_pi() { return 3.14159265358979323846; }
static double e_e() { return 2.71828182845904523536; }

const e_operator e_list_operators[] = {
	{"+",   e_add , 1, 0},
	{"-",   e_sub , 1, 0},
	{"*",   e_mult, 2, 0},
	{"/",   e_div , 2, 0},
	{"^",   e_pow , 3, 1},
	{"%",   e_mod , 3, 0},
	{"sin", e_sin , 0, 0},
	{"cos", e_cos , 0, 0},
	{"tan", e_tan , 0, 0}
};

int e_get_list_id(const char* value)
{
	for (int i = 0; i < sizeof(e_list_operators) / sizeof(e_operator); i++)
	{
		if (e_list_operators[i].sign == value)
			return i;
	}
	return -1;
}

int e_get_associativity(const char* value)
{
	for (int i = 0; i < sizeof(e_list_operators) / sizeof(e_operator); i++)
	{
		if (e_list_operators[i].sign == value)
			return e_list_operators[i].associativity;
	}
	return 0;
}

int e_get_priority(const char* value)
{
	for (int i = 0; i < sizeof(e_list_operators) / sizeof(e_operator); i++)
	{
		if (e_list_operators[i].sign == value)
			return e_list_operators[i].priority;
	}
	return 0;
}

bool e_is_operator(const char* value)
{
	for (int i = 0; i < sizeof(e_list_operators) / sizeof(e_operator); i++)
	{
		if (e_list_operators[i].sign == value)
			return true;
	}
	return false;
}

std::vector<e_token> e_compile(std::string function, int& error)
{
	if (function.empty())
	{
		error = 5;
		return {};
	}
	std::vector<e_token> tokens;
	std::vector<e_token> output;
	std::stack<e_token> op_stack;
	std::string num;
	std::string op;

	// remove spaces
	for (int i = 0; i < function.size(); i++)
	{
		if (function[i] == ' ')
		{
			function.erase(function.begin() + i);
			i--;
		}
	}

	// convert to tokens
	for (int i = 0; i < function.size(); i++)
	{
		// check if number
		if ((function[i] >= '0' && function[i] <= '9') || function[i] == '.')
			num.push_back(function[i]);
		else
		{
			if (num.size() != 0)
				tokens.push_back({ e_number_type, num });
			num.clear();

			// check if function
			if (i + 2 < function.size())
			{
				op = function[i];
				op.push_back(function[i + 1]);
				op.push_back(function[i + 2]);
			}
			if (i + 2 < function.size() && e_is_operator(op.c_str()))
			{
				tokens.push_back({ e_function_type, op });
				i += 2;
			}
			else
			{
				// check if operator
				op = function[i];
				if (e_is_operator(op.c_str()))
					tokens.push_back({ e_operator_type, op });
				else
				{
					// check if parentheses
					op = function[i];
					if (op == "(" || op == ")")
						tokens.push_back({ e_operator_type, op });
					else
					{
						// check if constant operand
						op = function[i];
						if (op == "e")
							tokens.push_back({ e_constant_type, op });
						if (i + 1 < function.size())
						{
							op = function[i];
							op.push_back(function[i + 1]);
						}
						if (op == "pi")
						{
							tokens.push_back({ e_constant_type, op });
							i++;
						}
						else
						{
							// check if operand
							op = function[i];
							if (!op.empty() && isalpha((unsigned char)op[0]))
								tokens.push_back({ e_operand_type, op });
						}
					}
				}
			}
		}
	}
	if (num.size() != 0)
		tokens.push_back({ e_number_type, num });

	// convert tokens to RPN
	for (int i = 0; i < tokens.size(); i++)
	{
		// fix user input
		if (i + 1 < tokens.size())
		{
			if (tokens[i].type == e_number_type)
			{
				if (tokens[i + 1].type == e_constant_type || tokens[i + 1].type == e_operand_type || tokens[i + 1].type == e_function_type || tokens[i + 1].value == "(")
				{
					tokens.insert(tokens.begin() + i + 1, { e_operator_type, "*" });
				}
			}
			else if (tokens[i].type == e_constant_type || tokens[i].type == e_operand_type)
			{
				if (tokens[i + 1].type == e_constant_type || tokens[i + 1].type == e_operand_type || tokens[i + 1].type == e_function_type || tokens[i + 1].value == "(")
				{
					tokens.insert(tokens.begin() + i + 1, { e_operator_type, "*" });
				}
			}
			else if (tokens[i].type == e_function_type)
			{
				if (tokens[i + 1].value != "(")
				{
					tokens.insert(tokens.begin() + i + 1, { e_operator_type, "(" });
					if (i + 2 < tokens.size() && (tokens[i + 2].type == e_constant_type || tokens[i + 2].type == e_operand_type || tokens[i + 2].type == e_number_type))
						tokens.insert(tokens.begin() + i + 3, { e_operator_type, ")" });
					else
					{
						error = 1;
						return {};
					}
				}
			}
			else if (tokens[i].type == e_operator_type && (tokens[i].value == "-" || tokens[i].value == "+"))
			{
				if ((i - 1 >= 0 && (tokens[i - 1].type == e_operator_type || tokens[i - 1].value == "(")) || i - 1 < 0)
				{
					if (tokens[i].value == "+")
						tokens.erase(tokens.begin() + i);
					else if (tokens[i].value == "-")
					{
						tokens.erase(tokens.begin() + i);
						tokens.insert(tokens.begin() + i, { e_operator_type, "*" });
						tokens.insert(tokens.begin() + i, { e_number_type, "-1" });
						i--;
						continue;
					}
				}
			}
			else if (tokens[i].value == ")")
			{
				if (tokens[i + 1].value == "(" || tokens[i + 1].type == e_constant_type || tokens[i + 1].type == e_operand_type || tokens[i + 1].type == e_function_type ||
					tokens[i + 1].type == e_number_type)
				{
					tokens.insert(tokens.begin() + i + 1, { e_operator_type, "*" });
				}
			}
		}

		// RPN
		if (tokens[i].type == e_number_type || tokens[i].type == e_constant_type || tokens[i].type == e_operand_type)
			output.push_back(tokens[i]);
		else if (tokens[i].type == e_function_type)
			op_stack.push(tokens[i]);
		else if (tokens[i].type == e_operator_type && tokens[i].value != "(" && tokens[i].value != ")")
		{
			while ((!op_stack.empty() && op_stack.top().type == e_operator_type && op_stack.top().value != "(") && (e_get_priority(op_stack.top().value.c_str()) > e_get_priority(tokens[i].value.c_str())
				|| (e_get_priority(op_stack.top().value.c_str()) == e_get_priority(tokens[i].value.c_str()) && e_get_associativity(tokens[i].value.c_str()) == 0)))
			{
				output.push_back(op_stack.top());
				op_stack.pop();
			}
			op_stack.push(tokens[i]);
		}
		else if (tokens[i].value == "(")
			op_stack.push(tokens[i]);
		else if (tokens[i].value == ")")
		{
			while (!op_stack.empty() && op_stack.top().value != "(")
			{
				output.push_back(op_stack.top());
				op_stack.pop();
			}
			// check for error
			if (op_stack.empty() || op_stack.top().value != "(")
			{
				error = 1;
				return {};
			}
			else
				op_stack.pop();
			if (!op_stack.empty() && op_stack.top().type == e_function_type)
			{
				output.push_back(op_stack.top());
				op_stack.pop();
			}
		}
	}
	while (!op_stack.empty())
	{
		// check for error
		if (op_stack.top().value == "(" || op_stack.top().value == ")")
		{
			error = 1;
			return {};
		}
		output.push_back(op_stack.top());
		op_stack.pop();
	}
	error = 0;
	return output;
}

std::vector<e_operand> e_get_operands(std::vector<e_token>& tokens)
{
	std::vector<e_operand>output;
	for (auto& token : tokens)
	{
		if (token.type == e_operand_type)
		{
			for (auto& it : output)
				if (it.operand == token.value)
					goto duplicate;
			output.push_back({ token.value, 0 });
		}
	duplicate:;
	}
	return output;
}

double e_evaluate(std::vector<e_token>& tokens, std::vector<e_operand>& operands, int& error)
{
	if (!tokens.empty())
	{
		std::stack<double>stack;
		for (auto& token : tokens)
		{
			double a, b;
			if (token.type == e_number_type)
				stack.push(stod(token.value));
			else if (token.type == e_constant_type)
			{
				if (token.value == "pi")
					stack.push(e_pi());
				if (token.value == "e")
					stack.push(e_e());
			}
			else if (token.type == e_operand_type)
			{
				if(!operands.empty())
					for (auto& operand : operands)
					{
						if (token.value == operand.operand)
						{
							stack.push(operand.value);
							goto skip;
						}
					}
				// ERROR
				error = 2;
				return nan("");
			skip:;
			}
			else if (token.type == e_function_type)
			{
				if(stack.empty())
				{
					// ERROR
					error = 3;
					return nan("");
				}
				a = stack.top();
				stack.pop();
				int id = e_get_list_id(token.value.c_str());
				if (id >= 0)
					stack.push(e_list_operators[id].function(a, 0));
				else
				{
					// ERROR
					error = 3;
					return nan("");
				}
			}
			else if (token.type == e_operator_type)
			{
				if (stack.empty())
				{
					// ERROR
					error = 4;
					return nan("");
				}
				a = stack.top();
				stack.pop();
				if (stack.empty())
				{
					// ERROR
					error = 4;
					return nan("");
				}
				b = stack.top();
				stack.pop();
				if (token.value == "/" && a == 0)
				{
					// ERROR
					error = 6;
					return nan("");
				}

				int id = e_get_list_id(token.value.c_str());
				if (id >= 0)
					stack.push(e_list_operators[id].function(b, a));
				else
				{
					// ERROR
					error = 4;
					return nan("");
				}
			}
		}
		if (!stack.empty())
			return stack.top();
	}
	error = 5;
	return nan("");
}

std::string e_get_error_message(int error)
{
	if (error == 1)
		return "Parenthesis error!";
	else if (error == 2)
		return "Operand error!";
	else if (error == 3)
		return "Function error!";
	else if (error == 4)
		return "Operator error!";
	else if (error == 5)
		return "Token error!";
	else if (error == 6)
		return "Division by 0!";
	return "";
}
