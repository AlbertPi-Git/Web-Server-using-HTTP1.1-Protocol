cmake_minimum_required(VERSION 3.10)
project(github224)

set(CMAKE_CXX_STANDARD 11)

include_directories(dependencies)
include_directories(dependencies/include)
include_directories(dependencies/include/inih)
include_directories(dependencies/include/spdlog)
include_directories(dependencies/include/spdlog/details)
include_directories(dependencies/include/spdlog/fmt)
include_directories(dependencies/include/spdlog/fmt/bundled)
include_directories(dependencies/include/spdlog/sinks)
include_directories(src)

add_executable(github224
        dependencies/include/inih/INIReader.h
        dependencies/include/spdlog/details/async_logger_impl.h
        dependencies/include/spdlog/details/circular_q.h
        dependencies/include/spdlog/details/console_globals.h
        dependencies/include/spdlog/details/file_helper.h
        dependencies/include/spdlog/details/fmt_helper.h
        dependencies/include/spdlog/details/log_msg.h
        dependencies/include/spdlog/details/logger_impl.h
        dependencies/include/spdlog/details/mpmc_blocking_q.h
        dependencies/include/spdlog/details/null_mutex.h
        dependencies/include/spdlog/details/os.h
        dependencies/include/spdlog/details/pattern_formatter.h
        dependencies/include/spdlog/details/periodic_worker.h
        dependencies/include/spdlog/details/registry.h
        dependencies/include/spdlog/details/thread_pool.h
        dependencies/include/spdlog/fmt/bundled/colors.h
        dependencies/include/spdlog/fmt/bundled/core.h
        dependencies/include/spdlog/fmt/bundled/format-inl.h
        dependencies/include/spdlog/fmt/bundled/format.h
        dependencies/include/spdlog/fmt/bundled/ostream.h
        dependencies/include/spdlog/fmt/bundled/posix.h
        dependencies/include/spdlog/fmt/bundled/printf.h
        dependencies/include/spdlog/fmt/bundled/ranges.h
        dependencies/include/spdlog/fmt/bundled/time.h
        dependencies/include/spdlog/fmt/bin_to_hex.h
        dependencies/include/spdlog/fmt/fmt.h
        dependencies/include/spdlog/fmt/ostr.h
        dependencies/include/spdlog/sinks/android_sink.h
        dependencies/include/spdlog/sinks/ansicolor_sink.h
        dependencies/include/spdlog/sinks/base_sink.h
        dependencies/include/spdlog/sinks/basic_file_sink.h
        dependencies/include/spdlog/sinks/daily_file_sink.h
        dependencies/include/spdlog/sinks/dist_sink.h
        dependencies/include/spdlog/sinks/msvc_sink.h
        dependencies/include/spdlog/sinks/null_sink.h
        dependencies/include/spdlog/sinks/ostream_sink.h
        dependencies/include/spdlog/sinks/rotating_file_sink.h
        dependencies/include/spdlog/sinks/sink.h
        dependencies/include/spdlog/sinks/stdout_color_sinks.h
        dependencies/include/spdlog/sinks/stdout_sinks.h
        dependencies/include/spdlog/sinks/syslog_sink.h
        dependencies/include/spdlog/sinks/wincolor_sink.h
        dependencies/include/spdlog/async.h
        dependencies/include/spdlog/async_logger.h
        dependencies/include/spdlog/common.h
        dependencies/include/spdlog/formatter.h
        dependencies/include/spdlog/logger.h
        dependencies/include/spdlog/spdlog.h
        dependencies/include/spdlog/tweakme.h
        dependencies/include/spdlog/version.h
        src/HttpdServer.cc
        src/HttpdServer.hpp
        src/logger.cc
        src/logger.hpp
        src/server-main.cc)
