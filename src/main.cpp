#include <format>
#include <logger.hpp>

int main() {
  logger::get_log_states().add_stream(stdout, logger::info | logger::defaults);
  logger::get_log_states().add_file_stream(std::filesystem::path("test.txt"),
                                           logger::info | logger::debug
                                               | logger::error);
  logger::get_log_states().add_level_to_all_streams(logger::important);
  LOG_IMPORTANT("info test");
  LOG_DEBUG("debug test");
  LOG_ERROR("error test");
  LOG_WARNING("warning test");
  LOG(logger::error, "test");
  int bar = 1'231;
  LOG_INFO(COLORIZE(ANSI_COLOR_BLUE "test {}"), bar);
  LOG_IMPORTANT("{}: {}", "bar", bar);

  std::vector<double> test_vec = {1.2, 3.4};

  LOG_INFO("{}", test_vec);

  LOG_INFO(COLORIZE(ANSI_EFFECT_BLINK, "test {}"), "asdf");
  LOG_ASSERT(false == true, "test");
  LOG_ASSERT(true == false, "test {}", "asdf");

  MESSAGE(logger::error, "test");
}
