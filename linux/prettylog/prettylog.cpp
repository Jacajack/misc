#include <string>
#include <algorithm>
#include <iostream>
#include <cctype>
#include <optional>
#include <fstream>
#include "log.hpp"

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [output file]" << std::endl;
		return 1;
	}
	
	std::optional<std::ofstream> logfile;
	if (argc > 1)
	{
		logfile = std::ofstream(argv[1]);
		if (!logfile->good())
		{
			std::cerr << "Could not open '" << argv[1] << "' for writing!" << std::endl;
			return 1;
		}
	}
	
	
	logger_config log_config{};
	log_config.ref_time = std::chrono::steady_clock::now();
	log_config.term_stream = &std::cerr;
	log_config.file_stream = logfile.has_value() ? &*logfile : nullptr;
	logger log(log_config);
	
	std::string line;
	while (std::getline(std::cin, line))
	{
		std::string lower_line = line;
		std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), [](char c){return std::tolower(c);});
		
		if (lower_line.find("error") != std::string::npos)
			log(logger_styles::error, "{}", line);
		else if (lower_line.find("warn") != std::string::npos)
			log(logger_styles::warning, "{}", line);
		else if (lower_line.find("debug") != std::string::npos)
			log(logger_styles::debug, "{}", line);
		else if (lower_line.find("assert") != std::string::npos)
			log(logger_styles::assertion, "{}", line);
		else
			log(logger_styles::info, "{}", line);
	}
}
