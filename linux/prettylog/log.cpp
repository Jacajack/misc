#include "log.hpp"
#include <mutex>
#include <optional>
#include <fmt/ostream.h>
#include <fmt/format.h>
using namespace std::string_literals;
using namespace fmt::literals;

std::string logger::format_message(const logger_message_style &style, const std::string &message, bool is_term)
{
	auto ansi_code = [is_term](int n){return is_term ? fmt::format("\x1b[{}m", n) : ""s;};
	auto fg_color = [ansi_code](int n){return ansi_code((n > 7 ? 90 : 30) + n % 8);};
	auto term_reset = ansi_code(0);

	auto dt = std::chrono::steady_clock::now() - m_config.ref_time;
	auto dt_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(dt);
	return fmt::format("[{}{:0.3f}{}] {}{}:{} {}{}{}",
		fg_color(2), dt_seconds.count() / 1000.0, term_reset,
		fg_color(style.title_color), style.title, term_reset,
		fg_color(style.message_color), message, term_reset
		);
}

void logger::file_dispatch(const logger_message_style &style, const std::string &s)
{
	std::optional<std::scoped_lock<std::mutex>> lock;
	if (auto mutex = m_config.file_stream_mutex)
		lock.emplace(*mutex);

	*m_config.file_stream << format_message(style, s, false) << std::endl;
}

void logger::term_dispatch(const logger_message_style &style, const std::string &s)
{
	std::optional<std::scoped_lock<std::mutex>> lock;
	if (auto mutex = m_config.term_stream_mutex)
		lock.emplace(*mutex);

	*m_config.term_stream << format_message(style, s, true) << std::endl;
}

namespace logger_styles
{
	const logger_message_style debug
	{
		.title = "debug",
		.title_color = 13,
		.message_color = 13,
	};

	const logger_message_style info
	{
		.title = "info",
		.title_color = 12,
		.message_color = 15,
	};

	const logger_message_style warning
	{
		.title = "warning",
		.title_color = 3,
		.message_color = 15,
	};

	const logger_message_style error
	{
		.title = "error",
		.title_color = 1,
		.message_color = 15,
	};
}