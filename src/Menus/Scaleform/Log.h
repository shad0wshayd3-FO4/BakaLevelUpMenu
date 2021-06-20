#pragma once

class Scaleform
{
public:
	static void Register()
	{
		auto BSScaleformManager = RE::BSScaleformManager::GetSingleton();
		if (BSScaleformManager && BSScaleformManager->loader)
		{
			auto LogState = static_cast<RE::Scaleform::GFx::LogState*>(
				BSScaleformManager->loader->GetStateAddRef(RE::Scaleform::GFx::State::StateType::kLog));
			LogState->log.reset(&bakaScaleformLog);
		}
	}

private:
	class BakaScaleformLog :
		public RE::Scaleform::Log
	{
	public:
		BakaScaleformLog()
		{
			// Create log
			auto logPath = logger::log_directory();
			*logPath /= "BakaScaleform.log"sv;
			auto logSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath->string(), true);
			fileLog = std::make_shared<spdlog::logger>("scaleform_log"s, std::move(logSink));

			// Set log traits
			fileLog->set_pattern("%v"s);
			fileLog->flush_on(spdlog::level::debug);

			// Initial message
			fileLog->info("BakaScaleform log opened."sv);
			logger::info("BakaScaleform log opened."sv);
		}

		virtual void LogMessageVarg(RE::Scaleform::LogMessageId a_messageID, const char* a_fmt, std::va_list a_argList) override
		{
			std::array<char, 2048> buffer;
			vsnprintf(buffer.data(), buffer.size(), a_fmt, a_argList);

			std::string formatted{ buffer.data() };
			formatted.erase(
				std::remove_if(formatted.end() - 1, formatted.end(), [](const char x)
							   { return (x == '\n'); }),
				formatted.end());

			fileLog->info(formatted.data());
		}

	private:
		std::shared_ptr<spdlog::logger> fileLog;
	};

	static inline BakaScaleformLog bakaScaleformLog;
};
