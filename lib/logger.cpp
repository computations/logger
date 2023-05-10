#include "logger.hpp"

namespace logger {

log_level_set operator|(log_level l1, log_level l2) {
  return convert_log_level_to_bitset(l1) | convert_log_level_to_bitset(l2);
}

log_level_set operator|(log_level_set l1, log_level l2) {
  return l1 | convert_log_level_to_bitset(l2);
}

log_level_set operator|(log_level l1, log_level_set l2) { return l2 | l1; }

log_level_set operator&(log_level l1, log_level l2) {
  return convert_log_level_to_bitset(l1) & convert_log_level_to_bitset(l2);
}

log_level_set operator&(log_level_set l1, log_level l2) {
  return l1 & convert_log_level_to_bitset(l2);
}

log_level_set operator&(log_level l1, log_level_set l2) { return l2 & l1; }

log_state_list_t &get_log_states() {
  static log_state_list_t lls;
  return lls;
}
} // namespace logger
