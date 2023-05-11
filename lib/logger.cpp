#include "logger.hpp"

namespace logger {

log_state_list_t &get_log_states() {
  static log_state_list_t lls;
  return lls;
}
} // namespace logger
