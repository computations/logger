#include <format>
#include <logger.hpp>

int main() {
  logger::get_log_states().add_stream(
      stdout,
      logger::log_level::info | logger::log_level::error
          | logger::log_level::warning | logger::log_level::debug);
  logger::get_log_states().add_file_stream(std::filesystem::path("test.txt"),
                                           logger::log_level::info
                                               | logger::log_level::debug);
  logger::get_log_states().add_level_to_all_streams(
      logger::log_level::important);
  LOG_IMPORTANT("info test");
  LOG_DEBUG("debug test");
  LOG_ERROR("error test");
  LOG_WARNING("warning test");
  LOG(logger::log_level::error, "test");
  LOG(INFO, "test");
  LOG(INFO, "{}", "hello world");
  int bar = 1231;
  LOG(INFO, "{}: {}", "bar", bar);
  LOG_IMPORTANT("{}: {}", "bar", bar);
}
