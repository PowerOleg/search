#pragma once
#include <string>

namespace crawler
{
	struct Link
	{
		std::string string_link = "";
		size_t crawler_depth = 0;

		Link(std::string link, size_t recursion_level_) : string_link{ link }, crawler_depth{recursion_level_}
		{}
	};
}