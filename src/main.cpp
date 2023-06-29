#include <iostream>
#include <logger.hpp>

int main() {
  logger::get_log_states().add_stream(stdout, logger::value::info);
  logger::get_log_states().add_file_stream(std::filesystem::path("test.txt"),
                                           logger::value::info |
                                               logger::value::debug);
  logger::get_log_states().add_level_to_all_streams(logger::value::important);
  std::cout << "debug: " << logger::log_level(logger::value::debug)
            << std::endl;
  std::cout << "debug: " << logger::log_level(logger::value::debug)
            << std::endl;
  MESSAGE_IMPORTANT("test");
}
