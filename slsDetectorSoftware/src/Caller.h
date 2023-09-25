// This file is used as input to generate the caller class

#include "CmdParser.h"
#include "sls/Detector.h"
#include <string>
#include <vector>
#include <iostream>
namespace sls {

class Caller {
public:
  Caller(Detector *ptr) : det(ptr) {}
  void call(const CmdParser &parser, int action, std::ostream &os = std::cout);

  std::string list(int action);

  std::string activate(int action);
  std::string adcclk(int action);
  std::string adcenable(int action);
  std::string adcenable10g(int action);
  std::string adcinvert(int action);
  std::string adcpipeline(int action);
  std::string apulse(int action);
  std::string asamples(int action);
  std::string autocompdisable(int action);
  std::string burstperiod(int action);
  std::string cdsgain(int action);
  std::string column(int action);
  std::string compdisabletime(int action);
  std::string dbitclk(int action);
  std::string dbitpipeline(int action);
  std::string delay(int action);
  std::string delayl(int action);
  std::string dpulse(int action);
  std::string dsamples(int action);
  std::string exptime(int action);
  std::string exptime1(int action);
  std::string exptime2(int action);
  std::string exptime3(int action);
  std::string exptimel(int action);
  std::string extsampling(int action);
  std::string extsamplingsrc(int action);
  std::string fformat(int action);
  std::string filtercells(int action);
  std::string filterresistor(int action);
  std::string findex(int action);
  std::string fliprows(int action);
  std::string flowcontrol10g(int action);
  std::string fname(int action);
  std::string foverwrite(int action);
  std::string fpath(int action);
  std::string frames(int action);
  std::string frametime(int action);
  std::string fwrite(int action);
  std::string gainmode(int action);
  std::string gates(int action);
  std::string highvoltage(int action);
  std::string imagetest(int action);
  std::string interpolation(int action);
  std::string interruptsubframe(int action);
  std::string led(int action);
  std::string lock(int action);
  std::string master(int action);
  std::string measuredperiod(int action);
  std::string measuredsubperiod(int action);
  std::string nextframenumber(int action);
  std::string numinterfaces(int action);
  std::string overflow(int action);
  std::string parallel(int action);
  std::string partialreset(int action);
  std::string patioctrl(int action);
  std::string patmask(int action);
  std::string patsetbit(int action);
  std::string period(int action);
  std::string periodl(int action);
  std::string polarity(int action);
  std::string port(int action);
  std::string powerchip(int action);
  std::string pumpprobe(int action);
  std::string readnrows(int action);
  std::string romode(int action);
  std::string row(int action);
  std::string runclk(int action);
  std::string runtime(int action);
  std::string rx_arping(int action);
  std::string rx_dbitoffset(int action);
  std::string rx_discardpolicy(int action);
  std::string rx_fifodepth(int action);
  std::string rx_framesperfile(int action);
  std::string rx_lock(int action);
  std::string rx_padding(int action);
  std::string rx_silent(int action);
  std::string rx_tcpport(int action);
  std::string rx_udpsocksize(int action);
  std::string rx_zmqfreq(int action);
  std::string rx_zmqip(int action);
  std::string rx_zmqport(int action);
  std::string rx_zmqstartfnum(int action);
  std::string rx_zmqstream(int action);
  std::string selinterface(int action);
  std::string settings(int action);
  std::string settingspath(int action);
  std::string stopport(int action);
  std::string storagecell_delay(int action);
  std::string storagecell_start(int action);
  std::string subdeadtime(int action);
  std::string subexptime(int action);
  std::string temp_control(int action);
  std::string temp_threshold(int action);
  std::string tengiga(int action);
  std::string timing(int action);
  std::string timingsource(int action);
  std::string top(int action);
  std::string transceiverenable(int action);
  std::string trimval(int action);
  std::string tsamples(int action);
  std::string txdelay_frame(int action);
  std::string txdelay_left(int action);
  std::string txdelay_right(int action);
  std::string udp_dstmac(int action);
  std::string udp_dstmac2(int action);
  std::string udp_dstport(int action);
  std::string udp_dstport2(int action);
  std::string udp_firstdst(int action);
  std::string udp_srcmac(int action);
  std::string udp_srcmac2(int action);
  std::string updatemode(int action);
  std::string veto(int action);
  std::string zmqip(int action);
  std::string zmqport(int action);

  std::vector<std::string> args;
  std::string cmd;
  Detector *det;
  int det_id{};

private:
  using FunctionMap = std::map<std::string, std::string (Caller::*)(int)>;
  Detector *ptr; // pointer to the detector that executes the command

  FunctionMap functions{ { "list", &Caller::list },
                         { "activate", &Caller::activate },
                         { "adcclk", &Caller::adcclk },
                         { "adcenable", &Caller::adcenable },
                         { "adcenable10g", &Caller::adcenable10g },
                         { "adcinvert", &Caller::adcinvert },
                         { "adcpipeline", &Caller::adcpipeline },
                         { "apulse", &Caller::apulse },
                         { "asamples", &Caller::asamples },
                         { "autocompdisable", &Caller::autocompdisable },
                         { "burstperiod", &Caller::burstperiod },
                         { "cdsgain", &Caller::cdsgain },
                         { "column", &Caller::column },
                         { "compdisabletime", &Caller::compdisabletime },
                         { "dbitclk", &Caller::dbitclk },
                         { "dbitpipeline", &Caller::dbitpipeline },
                         { "delay", &Caller::delay },
                         { "delayl", &Caller::delayl },
                         { "dpulse", &Caller::dpulse },
                         { "dsamples", &Caller::dsamples },
                         { "exptime", &Caller::exptime },
                         { "exptime1", &Caller::exptime1 },
                         { "exptime2", &Caller::exptime2 },
                         { "exptime3", &Caller::exptime3 },
                         { "exptimel", &Caller::exptimel },
                         { "extsampling", &Caller::extsampling },
                         { "extsamplingsrc", &Caller::extsamplingsrc },
                         { "fformat", &Caller::fformat },
                         { "filtercells", &Caller::filtercells },
                         { "filterresistor", &Caller::filterresistor },
                         { "findex", &Caller::findex },
                         { "fliprows", &Caller::fliprows },
                         { "flowcontrol10g", &Caller::flowcontrol10g },
                         { "fname", &Caller::fname },
                         { "foverwrite", &Caller::foverwrite },
                         { "fpath", &Caller::fpath },
                         { "frames", &Caller::frames },
                         { "frametime", &Caller::frametime },
                         { "fwrite", &Caller::fwrite },
                         { "gainmode", &Caller::gainmode },
                         { "gates", &Caller::gates },
                         { "highvoltage", &Caller::highvoltage },
                         { "imagetest", &Caller::imagetest },
                         { "interpolation", &Caller::interpolation },
                         { "interruptsubframe", &Caller::interruptsubframe },
                         { "led", &Caller::led },
                         { "lock", &Caller::lock },
                         { "master", &Caller::master },
                         { "measuredperiod", &Caller::measuredperiod },
                         { "measuredsubperiod", &Caller::measuredsubperiod },
                         { "nextframenumber", &Caller::nextframenumber },
                         { "numinterfaces", &Caller::numinterfaces },
                         { "overflow", &Caller::overflow },
                         { "parallel", &Caller::parallel },
                         { "partialreset", &Caller::partialreset },
                         { "patioctrl", &Caller::patioctrl },
                         { "patmask", &Caller::patmask },
                         { "patsetbit", &Caller::patsetbit },
                         { "period", &Caller::period },
                         { "periodl", &Caller::periodl },
                         { "polarity", &Caller::polarity },
                         { "port", &Caller::port },
                         { "powerchip", &Caller::powerchip },
                         { "pumpprobe", &Caller::pumpprobe },
                         { "readnrows", &Caller::readnrows },
                         { "romode", &Caller::romode },
                         { "row", &Caller::row },
                         { "runclk", &Caller::runclk },
                         { "runtime", &Caller::runtime },
                         { "rx_arping", &Caller::rx_arping },
                         { "rx_dbitoffset", &Caller::rx_dbitoffset },
                         { "rx_discardpolicy", &Caller::rx_discardpolicy },
                         { "rx_fifodepth", &Caller::rx_fifodepth },
                         { "rx_framesperfile", &Caller::rx_framesperfile },
                         { "rx_lock", &Caller::rx_lock },
                         { "rx_padding", &Caller::rx_padding },
                         { "rx_silent", &Caller::rx_silent },
                         { "rx_tcpport", &Caller::rx_tcpport },
                         { "rx_udpsocksize", &Caller::rx_udpsocksize },
                         { "rx_zmqfreq", &Caller::rx_zmqfreq },
                         { "rx_zmqip", &Caller::rx_zmqip },
                         { "rx_zmqport", &Caller::rx_zmqport },
                         { "rx_zmqstartfnum", &Caller::rx_zmqstartfnum },
                         { "rx_zmqstream", &Caller::rx_zmqstream },
                         { "selinterface", &Caller::selinterface },
                         { "settings", &Caller::settings },
                         { "settingspath", &Caller::settingspath },
                         { "stopport", &Caller::stopport },
                         { "storagecell_delay", &Caller::storagecell_delay },
                         { "storagecell_start", &Caller::storagecell_start },
                         { "subdeadtime", &Caller::subdeadtime },
                         { "subexptime", &Caller::subexptime },
                         { "temp_control", &Caller::temp_control },
                         { "temp_threshold", &Caller::temp_threshold },
                         { "tengiga", &Caller::tengiga },
                         { "timing", &Caller::timing },
                         { "timingsource", &Caller::timingsource },
                         { "top", &Caller::top },
                         { "transceiverenable", &Caller::transceiverenable },
                         { "trimval", &Caller::trimval },
                         { "tsamples", &Caller::tsamples },
                         { "txdelay_frame", &Caller::txdelay_frame },
                         { "txdelay_left", &Caller::txdelay_left },
                         { "txdelay_right", &Caller::txdelay_right },
                         { "udp_dstmac", &Caller::udp_dstmac },
                         { "udp_dstmac2", &Caller::udp_dstmac2 },
                         { "udp_dstport", &Caller::udp_dstport },
                         { "udp_dstport2", &Caller::udp_dstport2 },
                         { "udp_firstdst", &Caller::udp_firstdst },
                         { "udp_srcmac", &Caller::udp_srcmac },
                         { "udp_srcmac2", &Caller::udp_srcmac2 },
                         { "updatemode", &Caller::updatemode },
                         { "veto", &Caller::veto },
                         { "zmqip", &Caller::zmqip },
                         { "zmqport", &Caller::zmqport } };
  // some helper functions to print
  template <typename V> std::string OutStringHex(const V &value) {
    if (value.equal())
      return ToStringHex(value.front());
    return ToStringHex(value);
  }

  template <typename V> std::string OutStringHex(const V &value, int width) {
    if (value.equal())
      return ToStringHex(value.front(), width);
    return ToStringHex(value, width);
  }

  template <typename V> std::string OutString(const Result<V> &value) {
    if (value.equal())
      return ToString(value.front());
    return ToString(value);
  }

  template <typename V> std::string OutString(const V &value) {
    return ToString(value);
  }

  template <typename V>
  std::string OutString(const V &value, const std::string &unit) {
    if (value.equal())
      return ToString(value.front(), unit);
    return ToString(value, unit);
  }
};

} // namespace sls