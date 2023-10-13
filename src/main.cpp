#include <iostream>
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
  MESSAGE_IMPORTANT("info test");
  MESSAGE_DEBUG("debug test");
  MESSAGE_ERROR("error test");
  MESSAGE_WARNING("warning test");
}
