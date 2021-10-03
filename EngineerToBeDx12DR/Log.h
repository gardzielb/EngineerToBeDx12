#pragma once

class Log
{
public:
	static Log & GetInstance();

	inline std::shared_ptr<spdlog::logger> GetLogger() const
	{
		return m_logger;
	}

	Log(Log & other) = delete;

private:
	Log();

	static Log * instance;
	std::shared_ptr<spdlog::logger> m_logger;
};


#define HUDSON_LOG_INFO(...) Log::GetInstance().GetLogger()->info(__VA_ARGS__)
#define HUDSON_LOG_WARN(...) Log::GetInstance().GetLogger()->warn(__VA_ARGS__)
#define HUDSON_LOG_ERROR(...) Log::GetInstance().GetLogger()->error(__VA_ARGS__)
