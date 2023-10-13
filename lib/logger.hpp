#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <bitset>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <stdio.h>
#include <variant>
#include <vector>

namespace logger {

typedef std::chrono::high_resolution_clock::time_point logger_time_point;
const logger_time_point CLOCK_START = std::chrono::high_resolution_clock::now();

constexpr size_t                     LOG_LEVEL_COUNT = 6;
typedef std::bitset<LOG_LEVEL_COUNT> log_level_set;

class log_level {
public:
  enum log_level_value {
    debug     = 1ul << 0,
    info      = 1ul << 1,
    progress  = 1ul << 2,
    important = 1ul << 3,
    warning   = 1ul << 4,
    error     = 1ul << 5
  };

  operator log_level_set() const { return std::bitset<LOG_LEVEL_COUNT>(_ll); }

  log_level(log_level_value ll) : _ll{ll} {}

  log_level_set operator|(log_level_set l2) const {
    return l2 | log_level_set(_ll);
  }

  log_level_set operator&(log_level_set l2) const {
    return l2 & log_level_set(_ll);
  }

private:
  constexpr static log_level_set
  convert_log_level_value_to_bitset(log_level_value ll) {
    return 1lu << static_cast<size_t>(ll);
  }

  log_level_value _ll;
};

static_assert((1ul << (LOG_LEVEL_COUNT - 1)) == log_level::error,
              "Log level const doesn't match the actual log levels");

class log_level_state_t {
public:
  log_level_state_t() = default;

  void set_stream(FILE *s) { _stream = s; }

  void add_level(log_level ll) { _log_levels |= log_level_set(ll); }

  void add_level(log_level_set ls) { _log_levels |= ls; }

  bool print_ok(log_level ll) const { return (ll & _log_levels).any(); }

  FILE *get_stream() const { return _stream; }

  bool operator&&(const log_level_set &ll) const {
    return (_log_levels & ll).any();
  }

private:
  FILE *_stream = nullptr;

  log_level_set _log_levels;
};

class log_state_list_t {
public:
  log_state_list_t() = default;

  log_state_list_t(const log_state_list_t &) = delete;
  log_state_list_t(log_state_list_t &&)      = delete;

  log_state_list_t &operator=(const log_state_list_t &) = delete;
  log_state_list_t &operator=(log_state_list_t &&)      = delete;

  ~log_state_list_t() {
    for (auto f : _files) { fclose(f); }
  }

  void add_stream(FILE *s, log_level_set ll) {
    _streams.emplace_back();
    _streams.back().set_stream(s);
    _streams.back().add_level(ll);
  }

  void add_file_stream(const std::filesystem::path &log_filename,
                       log_level_set                ll) {
    FILE *s = fopen(log_filename.c_str(), "w");
    _files.push_back(s);
    _streams.emplace_back();
    _streams.back().set_stream(s);
    _streams.back().add_level(ll);
  }

  void add_level_to_all_streams(log_level_set ll) {
    for (auto &s : _streams) { s.add_level(ll); }
  }

  auto begin() const { return _streams.begin(); }
  auto end() const { return _streams.end(); }

private:
  std::vector<log_level_state_t> _streams;
  std::vector<FILE *>            _files;
};

log_state_list_t &get_log_states();

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define print_clock(stream)                                                    \
  do {                                                                         \
    std::chrono::duration<double> diff                                         \
        = std::chrono::high_resolution_clock::now() - logger::CLOCK_START;     \
    fprintf(stream, "[%6.2fs] ", diff.count());                                \
  } while (0)

#define DEBUG_PRINT_WITH_LEVEL(level, fmt, ...)                                \
  do {                                                                         \
    for (auto &s : logger::get_log_states()) {                                 \
      if (s.print_ok(level)) {                                                 \
        print_clock(s.get_stream());                                           \
        if (s && logger::log_level::debug) {                                   \
          fprintf(s.get_stream(), "[%s:%d] ", __func__, __LINE__);             \
        }                                                                      \
        if (level == logger::log_level::error) {                               \
          fprintf(s.get_stream(), ANSI_COLOR_RED "[ERR] " ANSI_COLOR_RESET);   \
        } else if (level == logger::log_level::warning) {                      \
          fprintf(s.get_stream(),                                              \
                  ANSI_COLOR_YELLOW "[WARN] " ANSI_COLOR_RESET);               \
        }                                                                      \
        fprintf(s.get_stream(), fmt "\n", __VA_ARGS__);                        \
      }                                                                        \
    }                                                                          \
  } while (0)

#define LOG_DEBUG(fmt, ...)                                                    \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::debug, fmt, __VA_ARGS__);

#define LOG_INFO(fmt, ...)                                                     \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::info, fmt, __VA_ARGS__);

#define LOG_PROGRESS(fmt, ...)                                                 \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::progress, fmt, __VA_ARGS__);

#define LOG_IMPORTANT(fmt, ...)                                                \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::important, fmt, __VA_ARGS__);

#define LOG_WARNING(fmt, ...)                                                  \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::warning, fmt, __VA_ARGS__);

#define LOG_ERROR(fmt, ...)                                                    \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::error, fmt, __VA_ARGS__);

#define MESSAGE_WITH_LEVEL(level, str)                                         \
  DEBUG_PRINT_WITH_LEVEL(level, "%s", (str));

#define MESSAGE_DEBUG(str)                                                     \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::debug, "%s", (str));

#define MESSAGE_INFO(str)                                                      \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::info, "%s", (str));

#define MESSAGE_PROGRESS(str)                                                  \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::progress, "%s", (str));

#define MESSAGE_IMPORTANT(str)                                                 \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::important, "%s", (str));

#define MESSAGE_WARNING(str)                                                   \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::warning, "%s", (str));

#define MESSAGE_ERROR(str)                                                     \
  DEBUG_PRINT_WITH_LEVEL(logger::log_level::error, "%s", (str));
} // namespace logger

#endif
