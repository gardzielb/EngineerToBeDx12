#include "pch.h"
#include "Log.h"

#include <spdlog/sinks/msvc_sink.h>


Log & Log::GetInstance()
{
	if (instance)
	{
		return *instance;
	}

	instance = new Log();
	return *instance;
}

Log::Log()
{
	auto debugSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
	debugSink->set_pattern("%^[%T] %n: %v%$");

	m_logger = std::make_shared<spdlog::logger>("HUDSON", debugSink);
	m_logger->set_level(spdlog::level::debug);
}

Log * Log::instance = nullptr;
