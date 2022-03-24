
#include "SharedMemory.h"
#include "ctb_named_dacs.h"
#include "sls/string_utils.h"
#include "sls/ToString.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace sls {

std::vector<std::string> get_ctb_dac_names(int det_id) {
    std::ifstream ifs(ctb_dac_fname(det_id));
    if (!ifs)
        return {};

    std::string dacnames;
    ifs.seekg(0, std::ios::end);
    dacnames.resize(ifs.tellg());
    ifs.seekg(0, std::ios::beg);
    ifs.read(&dacnames[0], dacnames.size());

    std::string chars = "[] ";

    dacnames.erase(std::remove_if(dacnames.begin(), dacnames.end(),
                                  [&chars](const char &c) {
                                      return chars.find(c) != std::string::npos;
                                  }),
                   dacnames.end());
    auto names = sls::split(dacnames, ',');
    return names;
}

std::string ctb_dac_fname(int det_id) {
    std::string sEnvPath;
    char *envpath = getenv(SHM_ENV_NAME);
    if (envpath != nullptr) {
        sEnvPath.assign(envpath);
        sEnvPath.append("_");
    }
    sEnvPath.insert(0, "_");
    std::stringstream oss;
    oss << "/dev/shm" << SHM_DETECTOR_PREFIX  << det_id << sEnvPath << "ctbdacs";
    return oss.str();
}

void set_ctb_dac_names(const std::vector<std::string>& names, int det_id){
    if (names.size() != 18)
        throw RuntimeError("Need to set all 18 dacs when naming dacs");

    
    std::ofstream ofs(ctb_dac_fname(det_id));
    if(!ofs)
        throw RuntimeError("Could not open dacnames file for writing");
    std::string s = sls::ToString(names);
    ofs.write(&s[0], s.size());
}


void remove_ctb_dacnames(int det_id){
    unlink(ctb_dac_fname(det_id).c_str());
}

} // namespace sls