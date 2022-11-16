#ifndef H_PARSER_TRACING_H
#define H_PARSER_TRACING_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

namespace parser
{
	static int traceLevel = 0;
	const std::string traceIdentPlaceholder = "\t";

	std::string identLevel()
	{
		std::stringstream oss;
		for (int i = 1; i < traceLevel; i++)
		{
			oss << traceIdentPlaceholder;
		}

		return oss.str();
	}

	void tracePrint(const std::string &str)
	{
		std::cout << identLevel() << str << std::endl;
	}

	void incIdent()
	{
		traceLevel += 1;
	}

	void decIdent()
	{
		traceLevel -= 1;
	}

	std::string trace(const std::string &msg)
	{
		incIdent();
		tracePrint("BEGIN " + msg);
		return msg;
	}

	void untrace(const std::string &msg)
	{
		tracePrint("END " + msg);
		decIdent();
	}
}

#endif // H_PARSER_TRACING_H
