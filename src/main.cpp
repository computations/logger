#include <iostream>
#include <logger.hpp>

int main() {
  get_log_states().add_stream(stdout, log_level::info);
  get_log_states().add_file_stream(std::filesystem::path("test.txt"),
                                   log_level::info | log_level::debug);
  get_log_states().add_level_to_all_streams(log_level::important);
  MESSAGE_IMPORTANT("test");
}
