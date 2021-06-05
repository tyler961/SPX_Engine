#include "../pch.h"

#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

void Log::init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");

	s_CoreLogger = spdlog::stderr_color_mt("SPX");
	s_CoreLogger->set_level(spdlog::level::trace);

	s_ClientLogger = spdlog::stderr_color_mt("APP");
	s_ClientLogger->set_level(spdlog::level::trace);
}