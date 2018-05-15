#pragma once

#include <memory>

#include <spdlog/spdlog.h>

namespace ic 
{
    namespace spd = spdlog;
    extern std::shared_ptr<spd::logger> Log;

    void initialize_logger();
}