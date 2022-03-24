#pragma once
#include <string>
#include <vector>
namespace sls{

std::vector<std::string> get_ctb_dac_names(int det_id);
void set_ctb_dac_names(const std::vector<std::string>& names, int det_id);
std::string ctb_dac_fname(int det_id);
void remove_ctb_dacnames(int det_id);


}

