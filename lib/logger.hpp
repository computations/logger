#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <bitset>
#include <chrono>
#include <filesystem>
#include <format>
#include <mutex>
#include <stacktrace>
#include <stdio.h>
#include <vector>
#include <version>

#ifndef __cpp_lib_format_ranges
template <typename T> struct std::formatter<std::vector<T>> {
  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    auto it = ctx.begin();
    if (it == ctx.end()) { return it; }
    ++it;

    if (it != ctx.end() && *it != '}') {
      throw std::format_error("Invalid format args");
    }
    return it;
  }

  template <class FmtContext>
  FmtContext::iterator format(std::vector<T> vec, FmtContext &ctx) const {
    auto out = ctx.out();
    *(out++) = '[';
    for (size_t idx = 0; idx < vec.size(); ++idx) {
      auto &ele = vec[idx];
      out       = std::format_to(out, "{}", ele);
      if (idx != vec.size() - 1) {
        *(out++) = ',';
        *(out++) = ' ';
      }
    }
    *(out++) = ']';
    return out;
  }
};
#endif

namespace logger {

typedef std::chrono::high_resolution_clock::time_point logger_time_point;
const logger_time_point CLOCK_START = std::chrono::high_resolution_clock::now();

constexpr size_t                     LOG_LEVEL_COUNT = 7;
typedef std::bitset<LOG_LEVEL_COUNT> log_level_set;

static std::mutex print_mutex;

#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_MAGENTA  "\x1b[35m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_DARK_RED "\x1b[38;5;88m"
#define ANSI_COLOR_LAVENDER "\x1b[38;5;219m"
#define ANSI_EFFECT_BLINK   "\x1b[5m"
#define ANSI_COLOR_RESET    "\x1b[0m"

enum class print_clock { full, none };

class log_level {
public:
  constexpr operator log_level_set() const {
    return std::bitset<LOG_LEVEL_COUNT>(_ll);
  }

  constexpr operator uint16_t() const { return _ll; }

  constexpr log_level() = default;
  constexpr log_level(uint16_t ll) : _ll{ll} {}

  constexpr log_level_set operator|(log_level l2) const {
    return l2 | log_level_set(_ll);
  }

  constexpr log_level_set operator&(log_level l2) const {
    return l2 & log_level_set(_ll);
  }

  constexpr log_level_set operator|(log_level_set l2) const {
    return l2 | log_level_set(_ll);
  }

  constexpr log_level_set operator&(log_level_set l2) const {
    return l2 & log_level_set(_ll);
  }

private:
  constexpr static log_level_set
  convert_log_level_value_to_bitset(uint16_t ll) {
    return 1lu << static_cast<size_t>(ll);
  }

  uint16_t _ll;
};

constexpr log_level_set operator|(log_level_set ls, log_level ll) {
  return ll | ls;
}

constexpr log_level_set operator&(log_level_set ls, log_level ll) {
  return ll & ls;
}

constexpr log_level debug{1ul << 0};
constexpr log_level info{1ul << 1};
constexpr log_level progress{1ul << 2};
constexpr log_level important{1ul << 3};
constexpr log_level warning{1ul << 4};
constexpr log_level error{1ul << 5};
constexpr log_level stacktrace{1ul << 6};

constexpr log_level_set defaults = stacktrace | error | warning | important;

static_assert((1ul << (LOG_LEVEL_COUNT - 1)) == stacktrace,
              "Log level const doesn't match the actual log levels");

class log_level_state_t {
public:
  log_level_state_t() : _stream{nullptr}, _log_levels{} {}

  log_level_state_t(const log_level_state_t &) = delete;
  log_level_state_t(log_level_state_t &&other) {
    _stream       = other._stream;
    _log_levels   = std::move(other._log_levels);
    other._stream = nullptr;
  }

  ~log_level_state_t() {
    if (_stream) { fclose(_stream); }
  }

  void set_stream(FILE *s) { _stream = s; }

  void add_level(log_level ll) { _log_levels |= log_level_set(ll); }

  void add_level(log_level_set ls) { _log_levels |= ls; }

  bool print_ok(log_level ll) const { return (ll & _log_levels).any(); }

  FILE *get_stream() const { return _stream; }

  bool operator&&(const log_level_set &ll) const {
    return (_log_levels & ll).any();
  }

  bool operator&&(const log_level &ll) const {
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

  void add_stream(FILE *s, log_level_set ll) {
    _streams.emplace_back();
    _streams.back().set_stream(s);
    _streams.back().add_level(ll);
  }

  void add_file_stream(const std::filesystem::path &log_filename,
                       log_level_set                ll) {
    FILE *s = fopen(log_filename.c_str(), "w");
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
};

log_state_list_t &get_log_states();
} // namespace logger

#define print_clock(stream)                                                    \
  do {                                                                         \
    std::chrono::duration<double> diff                                         \
        = std::chrono::high_resolution_clock::now() - logger::CLOCK_START;     \
    fprintf(stream, "[%6.2fs] ", diff.count());                                \
  } while (0)

#define COLORIZE(COLOR, ...) (COLOR __VA_ARGS__ ANSI_COLOR_RESET)

#define PRINT_LOG(level, clock, ...)                                           \
  do {                                                                         \
    const std::scoped_lock<std::mutex> print_lock{logger::print_mutex};        \
    for (auto &s : logger::get_log_states()) {                                 \
      if (s.print_ok(level)) {                                                 \
        if (clock == logger::print_clock::full) {                              \
          print_clock(s.get_stream());                                         \
        }                                                                      \
        if (s && logger::debug) {                                              \
          fprintf(s.get_stream(), "[%s:%d] ", __func__, __LINE__);             \
        }                                                                      \
        if (level == logger::stacktrace) {                                     \
          fprintf(s.get_stream(), COLORIZE(ANSI_COLOR_RED, "[STACKTRACE] "));  \
        } else if (level == logger::error) {                                   \
          fprintf(s.get_stream(), COLORIZE(ANSI_COLOR_RED, "[ERR] "));         \
        } else if (level == logger::warning) {                                 \
          fprintf(s.get_stream(), COLORIZE(ANSI_COLOR_YELLOW "[WARN] "));      \
        }                                                                      \
        fprintf(s.get_stream(), "%s\n", std::format(__VA_ARGS__).c_str());     \
      }                                                                        \
    }                                                                          \
  } while (0)

#define LOG(level, ...)                                                        \
  PRINT_LOG(level, logger::print_clock::full, __VA_ARGS__);

#define LOG_DEBUG(...)     LOG(logger::debug, __VA_ARGS__);
#define LOG_INFO(...)      LOG(logger::info, __VA_ARGS__);
#define LOG_PROGRESS(...)  LOG(logger::progress, __VA_ARGS__);
#define LOG_IMPORTANT(...) LOG(logger::important, __VA_ARGS__);
#define LOG_WARNING(...)   LOG(logger::warning, __VA_ARGS__);
#define LOG_ERROR(...)     LOG(logger::error, __VA_ARGS__);

#define MESSAGE(level, ...)                                                    \
  PRINT_LOG(level, logger::print_clock::none, __VA_ARGS__);

#define MESSAGE_DEBUG(...)    MESSAGE(logger::debug, __VA_ARGS__);
#define MESSAGE_INFO(...)     MESSAGE(logger::info, __VA_ARGS__);
#define MESSAGE_PROGRESS(...) MESSAGE(logger::progress, __VA_ARGS__);
#define MESSAGE_IMPORTANT(...)                                                 \
  MESSAGE(logger::important, __VA_ARGS__);
#define MESSAGE_WARNING(...) MESSAGE(logger::warning, __VA_ARGS__);
#define MESSAGE_ERROR(...)   MESSAGE(logger::error, __VA_ARGS__);

#ifdef LOGGER_ASSERT_THROW
#include <stdexcept>
#define LOG_ASSERT(condition, ...)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      MESSAGE_ERROR("ASSERT(" #condition ") " __VA_ARGS__);                    \
      throw std::runtime_error{"Assertion failed"};                            \
    }                                                                          \
  } while (0)
#elif __cpp_lib_stacktrace
#define LOG_ASSERT(condition, ...)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      MESSAGE_ERROR("ASSERT(" #condition ") " __VA_ARGS__);                    \
      for (auto &se : std::stacktrace::current()) {                            \
        MESSAGE(logger::stacktrace, "{}", se);                                 \
      }                                                                        \
      abort();                                                                 \
    }                                                                          \
  } while (0)
#else
#define LOG_ASSERT(condition, ...)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      MESSAGE_ERROR("ASSERT(" #condition ") " __VA_ARGS__);                    \
      abort();                                                                 \
    }                                                                          \
  } while (0)
#endif

#endif
