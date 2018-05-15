#include <Logger.hpp>

#include <spdlog/sinks/dist_sink.h>

namespace ic 
{
    std::shared_ptr<spd::logger> Log;
    
    void initialize_logger()
    {
        auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_st>();
        #if defined(_WIN32) || defined(_WIN64)
        const auto sink1 = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
        #else
        auto sink1 = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
        #endif
        const auto sink2 = std::make_shared<spdlog::sinks::simple_file_sink_st>("debug.log");
    
        dist_sink->add_sink(sink1);
        dist_sink->add_sink(sink2);
        Log = std::make_shared<spdlog::logger>("Log", dist_sink);
        Log->set_pattern("[%H:%M:%S.%e]<%l> : %v");
        Log->set_level(spd::level::trace);
        
        Log->trace("<Trace>");
        Log->debug("<Debug>");
        Log->info("<Info>");
        Log->warn("<Warning>");
        Log->error("<Error>");
        Log->critical("<Critical>");
    }
}