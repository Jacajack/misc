#pragma once
#include <string>
#include <mutex>
#include <iosfwd>
#include <fmt/core.h>
#include <chrono>

struct logger_message_style
{
	std::string title;
	int title_color;
	int message_color;
};

struct logger_config
{
	std::chrono::time_point<std::chrono::steady_clock> ref_time;
	std::ostream *term_stream;
	std::mutex *term_stream_mutex;
	std::ostream *file_stream;
	std::mutex *file_stream_mutex;
};

class logger
{
public:
	logger(logger_config cfg) :
		m_config(std::move(cfg))
	{
	}

	template <typename ...T>
	void operator()(const logger_message_style &style, fmt::format_string<T...> f, T &&...args)
	{
		auto content = fmt::format(f, std::forward<T>(args)...);
		if (m_config.file_stream) file_dispatch(style, content);
		if (m_config.term_stream) term_dispatch(style, content);
	}

private:
	std::string format_message(const logger_message_style &style, const std::string &s, bool colors = false);
	void file_dispatch(const logger_message_style &style, const std::string &s);
	void term_dispatch(const logger_message_style &style, const std::string &s);

	logger_config m_config;
};

namespace logger_styles
{
	extern const logger_message_style debug;
	extern const logger_message_style info;
	extern const logger_message_style warning;
	extern const logger_message_style error;
}
