// Standard includes
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

// Local includes
#include "proc_stat.hpp"
#include "constants.hpp"

namespace status_bar {

const char* const proc_stat_path = "/proc/stat";
const char* const proc_stat_cpu_field = "cpu";

cpu::cpu() {
    std::ifstream proc_stat{ proc_stat_path };

    for (std::string line; std::getline(proc_stat, line).good();) {
        std::string field;
        std::vector<std::string> raw_entries;

        for (char chr : line) {
            if (chr != ' ') {
                if (raw_entries.empty()) {
                    field.push_back(chr);
                } else {
                    raw_entries.back().push_back(chr);
                }
                continue;
            }
            if (raw_entries.empty()) {
                if (field != proc_stat_cpu_field) {
                    break;
                }
            } else if (raw_entries.back().empty()) {
                continue;
            }
            raw_entries.emplace_back();
        }

        if (field != proc_stat_cpu_field) {
            continue;
        }

        if (raw_entries.size() != index_count) {
            throw invalid_proc_stat(error_str,
              "Expected " + std::to_string(index_count)
                + " entries following field '" + proc_stat_cpu_field + "' in "
                + proc_stat_path + " but found "
                + std::to_string(raw_entries.size()) + " entries instead.");
        }

        this->entries_ = std::vector<size_t>{};
        for (const std::string& entry : raw_entries) {
            this->entries_.push_back(std::stoull(entry));
        }

        return;
    }

    throw invalid_proc_stat(error_str,
      "Failed to find field '" + std::string(proc_stat_cpu_field) + "' in "
        + proc_stat_path + ".");
}

size_t cpu::get_total() const {
    size_t total = 0;
    for (size_t entry : this->entries_) {
        total += entry;
    }
    return total;
}

size_t cpu::get_total(const std::vector<index>& indicies) const {
    size_t total = 0;
    for (enum index idx : indicies) {
        total += this->entries_.at(static_cast<size_t>(idx));
    }
    return total;
}

} // namespace status_bar
