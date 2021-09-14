#pragma once
#include <vector>
#include <string>

struct e_operand
{
	std::string operand;
	double value;
};

struct e_token
{
	int type;
	std::string value;
};

std::vector<e_token> e_compile(std::string function, int& error);
std::vector<e_operand> e_get_operands(std::vector<e_token>& tokens);
double e_evaluate(std::vector<e_token>& tokens, std::vector<e_operand>& operands, int& error);
std::string e_get_error_message(int error);