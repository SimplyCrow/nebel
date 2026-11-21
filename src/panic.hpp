#pragma once
#ifndef NEBEL_PANIC_HPP
#define NEBEL_PANIC_HPP

#include <spdlog/spdlog.h>
#include <stdexcept>
#include <source_location>

#define PANIC(logger_namespace, msg, ...) \
        do { \
                std::source_location loc = std::source_location::current(); \
                ::logger_namespace::critical("panicked at {}:{}:{} function: {}", loc.file_name(), loc.line(), loc.column(), loc.function_name()); \
                ::logger_namespace::critical(msg __VA_OPT__(,) __VA_ARGS__); \
                std::exit(EXIT_FAILURE); \
                throw std::runtime_error("Could not panic"); \
        } while(0)


#define PANICF(msg, ...) PANIC(logger, msg __VA_OPT__(,) __VA_ARGS__)

#endif // NEBEL_PANIC_HPP
