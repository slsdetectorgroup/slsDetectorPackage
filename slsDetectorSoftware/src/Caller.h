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

  IpAddr getDstIpFromAuto();
  IpAddr getSrcIpFromAuto();
  UdpDestination getUdpEntry();
  void GetLevelAndUpdateArgIndex(int action, std::string levelSeparatedCommand,
                                 int &level, int &iArg, size_t nGetArgs,
                                 size_t nPutArgs);
  void WrongNumberOfParameters(size_t expected);

  std::string list(int action);

  std::string activate(int action);
  std::string adcclk(int action);
  std::string adcenable(int action);
  std::string adcenable10g(int action);
  std::string adcindex(int action);
  std::string adcinvert(int action);
  std::string adclist(int action);
  std::string adcname(int action);
  std::string adcphase(int action);
  std::string adcpipeline(int action);
  std::string adcreg(int action);
  std::string adcvpp(int action);
  std::string apulse(int action);
  std::string asamples(int action);
  std::string autocompdisable(int action);
  std::string blockingtrigger(int action);
  std::string burstperiod(int action);
  std::string bursts(int action);
  std::string burstsl(int action);
  std::string bustest(int action);
  std::string cdsgain(int action);
  std::string chipversion(int action);
  std::string clearbit(int action);
  std::string clearbusy(int action);
  std::string clearroi(int action);
  std::string clientversion(int action);
  std::string clkdiv(int action);
  std::string clkfreq(int action);
  std::string clkphase(int action);
  std::string column(int action);
  std::string compdisabletime(int action);
  std::string confadc(int action);
  std::string config(int action);
  std::string dac(int action);
  std::string dacindex(int action);
  std::string daclist(int action);
  std::string dacname(int action);
  std::string datastream(int action);
  std::string dbitclk(int action);
  std::string dbitphase(int action);
  std::string dbitpipeline(int action);
  std::string defaultdac(int action);
  std::string defaultpattern(int action);
  std::string delay(int action);
  std::string delayl(int action);
  std::string detectorserverversion(int action);
  std::string detsize(int action);
  std::string diodelay(int action);
  std::string dpulse(int action);
  std::string dr(int action);
  std::string drlist(int action);
  std::string dsamples(int action);
  std::string exptime(int action);
  std::string exptime1(int action);
  std::string exptime2(int action);
  std::string exptime3(int action);
  std::string exptimel(int action);
  std::string extrastoragecells(int action);
  std::string extsampling(int action);
  std::string extsamplingsrc(int action);
  std::string extsig(int action);
  std::string fformat(int action);
  std::string filtercells(int action);
  std::string filterresistor(int action);
  std::string findex(int action);
  std::string firmwaretest(int action);
  std::string firmwareversion(int action);
  std::string fliprows(int action);
  std::string flowcontrol10g(int action);
  std::string fmaster(int action);
  std::string fname(int action);
  std::string foverwrite(int action);
  std::string fpath(int action);
  std::string framecounter(int action);
  std::string frames(int action);
  std::string framesl(int action);
  std::string frametime(int action);
  std::string fwrite(int action);
  std::string gainmode(int action);
  std::string gappixels(int action);
  std::string gatedelay(int action);
  std::string gatedelay1(int action);
  std::string gatedelay2(int action);
  std::string gatedelay3(int action);
  std::string gates(int action);
  std::string getbit(int action);
  std::string hardwareversion(int action);
  std::string highvoltage(int action);
  std::string im_a(int action);
  std::string im_b(int action);
  std::string im_c(int action);
  std::string im_d(int action);
  std::string im_io(int action);
  std::string imagetest(int action);
  std::string initialchecks(int action);
  std::string inj_ch(int action);
  std::string interpolation(int action);
  std::string interruptsubframe(int action);
  std::string kernelversion(int action);
  std::string lastclient(int action);
  std::string led(int action);
  std::string lock(int action);
  std::string master(int action);
  std::string maxadcphaseshift(int action);
  std::string maxclkphaseshift(int action);
  std::string maxdbitphaseshift(int action);
  std::string measuredperiod(int action);
  std::string measuredsubperiod(int action);
  std::string moduleid(int action);
  std::string nextframenumber(int action);
  std::string nmod(int action);
  std::string numinterfaces(int action);
  std::string overflow(int action);
  std::string packageversion(int action);
  std::string parallel(int action);
  std::string parameters(int action);
  std::string partialreset(int action);
  std::string patfname(int action);
  std::string patioctrl(int action);
  std::string patlimits(int action);
  std::string patloop(int action);
  std::string patloop0(int action);
  std::string patloop1(int action);
  std::string patloop2(int action);
  std::string patmask(int action);
  std::string patnloop(int action);
  std::string patnloop0(int action);
  std::string patnloop1(int action);
  std::string patnloop2(int action);
  std::string patsetbit(int action);
  std::string pattern(int action);
  std::string patternstart(int action);
  std::string patwait(int action);
  std::string patwait0(int action);
  std::string patwait1(int action);
  std::string patwait2(int action);
  std::string patwaittime(int action);
  std::string patwaittime0(int action);
  std::string patwaittime1(int action);
  std::string patwaittime2(int action);
  std::string patword(int action);
  std::string period(int action);
  std::string periodl(int action);
  std::string polarity(int action);
  std::string port(int action);
  std::string powerchip(int action);
  std::string programfpga(int action);
  std::string pulse(int action);
  std::string pulsechip(int action);
  std::string pulsenmove(int action);
  std::string pumpprobe(int action);
  std::string quad(int action);
  std::string readnrows(int action);
  std::string readout(int action);
  std::string readoutspeed(int action);
  std::string readoutspeedlist(int action);
  std::string rebootcontroller(int action);
  std::string reg(int action);
  std::string resetdacs(int action);
  std::string resetfpga(int action);
  std::string roi(int action);
  std::string romode(int action);
  std::string row(int action);
  std::string runclk(int action);
  std::string runtime(int action);
  std::string rx_arping(int action);
  std::string rx_clearroi(int action);
  std::string rx_dbitoffset(int action);
  std::string rx_discardpolicy(int action);
  std::string rx_fifodepth(int action);
  std::string rx_frameindex(int action);
  std::string rx_framescaught(int action);
  std::string rx_framesperfile(int action);
  std::string rx_jsonpara(int action);
  std::string rx_lastclient(int action);
  std::string rx_lock(int action);
  std::string rx_missingpackets(int action);
  std::string rx_padding(int action);
  std::string rx_printconfig(int action);
  std::string rx_realudpsocksize(int action);
  std::string rx_silent(int action);
  std::string rx_start(int action);
  std::string rx_status(int action);
  std::string rx_stop(int action);
  std::string rx_tcpport(int action);
  std::string rx_threads(int action);
  std::string rx_udpsocksize(int action);
  std::string rx_version(int action);
  std::string rx_zmqfreq(int action);
  std::string rx_zmqhwm(int action);
  std::string rx_zmqip(int action);
  std::string rx_zmqport(int action);
  std::string rx_zmqstartfnum(int action);
  std::string rx_zmqstream(int action);
  std::string savepattern(int action);
  std::string scan(int action);
  std::string scanerrmsg(int action);
  std::string selinterface(int action);
  std::string serialnumber(int action);
  std::string setbit(int action);
  std::string settings(int action);
  std::string settingslist(int action);
  std::string settingspath(int action);
  std::string signalindex(int action);
  std::string signallist(int action);
  std::string signalname(int action);
  std::string slowadcindex(int action);
  std::string slowadclist(int action);
  std::string slowadcname(int action);
  std::string slowadcvalues(int action);
  std::string start(int action);
  std::string status(int action);
  std::string stop(int action);
  std::string stopport(int action);
  std::string storagecell_delay(int action);
  std::string storagecell_start(int action);
  std::string subdeadtime(int action);
  std::string subexptime(int action);
  std::string sync(int action);
  std::string syncclk(int action);
  std::string temp_10ge(int action);
  std::string temp_adc(int action);
  std::string temp_control(int action);
  std::string temp_dcdc(int action);
  std::string temp_event(int action);
  std::string temp_fpga(int action);
  std::string temp_fpgaext(int action);
  std::string temp_fpgafl(int action);
  std::string temp_fpgafr(int action);
  std::string temp_slowadc(int action);
  std::string temp_sodl(int action);
  std::string temp_sodr(int action);
  std::string temp_threshold(int action);
  std::string templist(int action);
  std::string tempvalues(int action);
  std::string tengiga(int action);
  std::string timing(int action);
  std::string timinglist(int action);
  std::string timingsource(int action);
  std::string top(int action);
  std::string transceiverenable(int action);
  std::string trigger(int action);
  std::string triggers(int action);
  std::string triggersl(int action);
  std::string trimbits(int action);
  std::string trimval(int action);
  std::string tsamples(int action);
  std::string txdelay(int action);
  std::string txdelay_frame(int action);
  std::string txdelay_left(int action);
  std::string txdelay_right(int action);
  std::string type(int action);
  std::string udp_cleardst(int action);
  std::string udp_dstlist(int action);
  std::string udp_dstmac(int action);
  std::string udp_dstmac2(int action);
  std::string udp_dstport(int action);
  std::string udp_dstport2(int action);
  std::string udp_firstdst(int action);
  std::string udp_numdst(int action);
  std::string udp_reconfigure(int action);
  std::string udp_srcmac(int action);
  std::string udp_srcmac2(int action);
  std::string udp_validate(int action);
  std::string update(int action);
  std::string updatedetectorserver(int action);
  std::string updatekernel(int action);
  std::string updatemode(int action);
  std::string user(int action);
  std::string v_a(int action);
  std::string v_b(int action);
  std::string v_c(int action);
  std::string v_chip(int action);
  std::string v_d(int action);
  std::string v_io(int action);
  std::string v_limit(int action);
  std::string vchip_comp_adc(int action);
  std::string vchip_comp_fe(int action);
  std::string vchip_cs(int action);
  std::string vchip_opa_1st(int action);
  std::string vchip_opa_fd(int action);
  std::string vchip_ref_comp_fe(int action);
  std::string veto(int action);
  std::string vetoalg(int action);
  std::string vetofile(int action);
  std::string vetophoton(int action);
  std::string vetoref(int action);
  std::string virtualFunction(int action);
  std::string vm_a(int action);
  std::string vm_b(int action);
  std::string vm_c(int action);
  std::string vm_d(int action);
  std::string vm_io(int action);
  std::string voltageindex(int action);
  std::string voltagelist(int action);
  std::string voltagename(int action);
  std::string voltagevalues(int action);
  std::string zmqhwm(int action);
  std::string zmqip(int action);
  std::string zmqport(int action);

  std::vector<std::string> args;
  std::string cmd;
  Detector *det;
  int det_id{ -1 };
  int rx_id{ -1 };

private:
  using FunctionMap = std::map<std::string, std::string (Caller::*)(int)>;
  Detector *ptr; // pointer to the detector that executes the command

  FunctionMap functions{ { "list", &Caller::list },
                         { "activate", &Caller::activate },
                         { "adcclk", &Caller::adcclk },
                         { "adcenable", &Caller::adcenable },
                         { "adcenable10g", &Caller::adcenable10g },
                         { "adcindex", &Caller::adcindex },
                         { "adcinvert", &Caller::adcinvert },
                         { "adclist", &Caller::adclist },
                         { "adcname", &Caller::adcname },
                         { "adcphase", &Caller::adcphase },
                         { "adcpipeline", &Caller::adcpipeline },
                         { "adcreg", &Caller::adcreg },
                         { "adcvpp", &Caller::adcvpp },
                         { "apulse", &Caller::apulse },
                         { "asamples", &Caller::asamples },
                         { "autocompdisable", &Caller::autocompdisable },
                         { "blockingtrigger", &Caller::blockingtrigger },
                         { "burstperiod", &Caller::burstperiod },
                         { "bursts", &Caller::bursts },
                         { "burstsl", &Caller::burstsl },
                         { "bustest", &Caller::bustest },
                         { "cdsgain", &Caller::cdsgain },
                         { "chipversion", &Caller::chipversion },
                         { "clearbit", &Caller::clearbit },
                         { "clearbusy", &Caller::clearbusy },
                         { "clearroi", &Caller::clearroi },
                         { "clientversion", &Caller::clientversion },
                         { "clkdiv", &Caller::clkdiv },
                         { "clkfreq", &Caller::clkfreq },
                         { "clkphase", &Caller::clkphase },
                         { "column", &Caller::column },
                         { "compdisabletime", &Caller::compdisabletime },
                         { "confadc", &Caller::confadc },
                         { "config", &Caller::config }, { "dac", &Caller::dac },
                         { "dacindex", &Caller::dacindex },
                         { "daclist", &Caller::daclist },
                         { "dacname", &Caller::dacname },
                         { "datastream", &Caller::datastream },
                         { "dbitclk", &Caller::dbitclk },
                         { "dbitphase", &Caller::dbitphase },
                         { "dbitpipeline", &Caller::dbitpipeline },
                         { "defaultdac", &Caller::defaultdac },
                         { "defaultpattern", &Caller::defaultpattern },
                         { "delay", &Caller::delay },
                         { "delayl", &Caller::delayl },
                         { "detectorserverversion",
                           &Caller::detectorserverversion },
                         { "detsize", &Caller::detsize },
                         { "diodelay", &Caller::diodelay },
                         { "dpulse", &Caller::dpulse }, { "dr", &Caller::dr },
                         { "drlist", &Caller::drlist },
                         { "dsamples", &Caller::dsamples },
                         { "exptime", &Caller::exptime },
                         { "exptime1", &Caller::exptime1 },
                         { "exptime2", &Caller::exptime2 },
                         { "exptime3", &Caller::exptime3 },
                         { "exptimel", &Caller::exptimel },
                         { "extrastoragecells", &Caller::extrastoragecells },
                         { "extsampling", &Caller::extsampling },
                         { "extsamplingsrc", &Caller::extsamplingsrc },
                         { "extsig", &Caller::extsig },
                         { "fformat", &Caller::fformat },
                         { "filtercells", &Caller::filtercells },
                         { "filterresistor", &Caller::filterresistor },
                         { "findex", &Caller::findex },
                         { "firmwaretest", &Caller::firmwaretest },
                         { "firmwareversion", &Caller::firmwareversion },
                         { "fliprows", &Caller::fliprows },
                         { "flowcontrol10g", &Caller::flowcontrol10g },
                         { "fmaster", &Caller::fmaster },
                         { "fname", &Caller::fname },
                         { "foverwrite", &Caller::foverwrite },
                         { "fpath", &Caller::fpath },
                         { "framecounter", &Caller::framecounter },
                         { "frames", &Caller::frames },
                         { "framesl", &Caller::framesl },
                         { "frametime", &Caller::frametime },
                         { "fwrite", &Caller::fwrite },
                         { "gainmode", &Caller::gainmode },
                         { "gappixels", &Caller::gappixels },
                         { "gatedelay", &Caller::gatedelay },
                         { "gatedelay1", &Caller::gatedelay1 },
                         { "gatedelay2", &Caller::gatedelay2 },
                         { "gatedelay3", &Caller::gatedelay3 },
                         { "gates", &Caller::gates },
                         { "getbit", &Caller::getbit },
                         { "hardwareversion", &Caller::hardwareversion },
                         { "highvoltage", &Caller::highvoltage },
                         { "im_a", &Caller::im_a }, { "im_b", &Caller::im_b },
                         { "im_c", &Caller::im_c }, { "im_d", &Caller::im_d },
                         { "im_io", &Caller::im_io },
                         { "imagetest", &Caller::imagetest },
                         { "initialchecks", &Caller::initialchecks },
                         { "inj_ch", &Caller::inj_ch },
                         { "interpolation", &Caller::interpolation },
                         { "interruptsubframe", &Caller::interruptsubframe },
                         { "kernelversion", &Caller::kernelversion },
                         { "lastclient", &Caller::lastclient },
                         { "led", &Caller::led }, { "lock", &Caller::lock },
                         { "master", &Caller::master },
                         { "maxadcphaseshift", &Caller::maxadcphaseshift },
                         { "maxclkphaseshift", &Caller::maxclkphaseshift },
                         { "maxdbitphaseshift", &Caller::maxdbitphaseshift },
                         { "measuredperiod", &Caller::measuredperiod },
                         { "measuredsubperiod", &Caller::measuredsubperiod },
                         { "moduleid", &Caller::moduleid },
                         { "nextframenumber", &Caller::nextframenumber },
                         { "nmod", &Caller::nmod },
                         { "numinterfaces", &Caller::numinterfaces },
                         { "overflow", &Caller::overflow },
                         { "packageversion", &Caller::packageversion },
                         { "parallel", &Caller::parallel },
                         { "parameters", &Caller::parameters },
                         { "partialreset", &Caller::partialreset },
                         { "patfname", &Caller::patfname },
                         { "patioctrl", &Caller::patioctrl },
                         { "patlimits", &Caller::patlimits },
                         { "patloop", &Caller::patloop },
                         { "patloop0", &Caller::patloop0 },
                         { "patloop1", &Caller::patloop1 },
                         { "patloop2", &Caller::patloop2 },
                         { "patmask", &Caller::patmask },
                         { "patnloop", &Caller::patnloop },
                         { "patnloop0", &Caller::patnloop0 },
                         { "patnloop1", &Caller::patnloop1 },
                         { "patnloop2", &Caller::patnloop2 },
                         { "patsetbit", &Caller::patsetbit },
                         { "pattern", &Caller::pattern },
                         { "patternstart", &Caller::patternstart },
                         { "patwait", &Caller::patwait },
                         { "patwait0", &Caller::patwait0 },
                         { "patwait1", &Caller::patwait1 },
                         { "patwait2", &Caller::patwait2 },
                         { "patwaittime", &Caller::patwaittime },
                         { "patwaittime0", &Caller::patwaittime0 },
                         { "patwaittime1", &Caller::patwaittime1 },
                         { "patwaittime2", &Caller::patwaittime2 },
                         { "patword", &Caller::patword },
                         { "period", &Caller::period },
                         { "periodl", &Caller::periodl },
                         { "polarity", &Caller::polarity },
                         { "port", &Caller::port },
                         { "powerchip", &Caller::powerchip },
                         { "programfpga", &Caller::programfpga },
                         { "pulse", &Caller::pulse },
                         { "pulsechip", &Caller::pulsechip },
                         { "pulsenmove", &Caller::pulsenmove },
                         { "pumpprobe", &Caller::pumpprobe },
                         { "quad", &Caller::quad },
                         { "readnrows", &Caller::readnrows },
                         { "readout", &Caller::readout },
                         { "readoutspeed", &Caller::readoutspeed },
                         { "readoutspeedlist", &Caller::readoutspeedlist },
                         { "rebootcontroller", &Caller::rebootcontroller },
                         { "reg", &Caller::reg },
                         { "resetdacs", &Caller::resetdacs },
                         { "resetfpga", &Caller::resetfpga },
                         { "roi", &Caller::roi }, { "romode", &Caller::romode },
                         { "row", &Caller::row }, { "runclk", &Caller::runclk },
                         { "runtime", &Caller::runtime },
                         { "rx_arping", &Caller::rx_arping },
                         { "rx_clearroi", &Caller::rx_clearroi },
                         { "rx_dbitoffset", &Caller::rx_dbitoffset },
                         { "rx_discardpolicy", &Caller::rx_discardpolicy },
                         { "rx_fifodepth", &Caller::rx_fifodepth },
                         { "rx_frameindex", &Caller::rx_frameindex },
                         { "rx_framescaught", &Caller::rx_framescaught },
                         { "rx_framesperfile", &Caller::rx_framesperfile },
                         { "rx_jsonpara", &Caller::rx_jsonpara },
                         { "rx_lastclient", &Caller::rx_lastclient },
                         { "rx_lock", &Caller::rx_lock },
                         { "rx_missingpackets", &Caller::rx_missingpackets },
                         { "rx_padding", &Caller::rx_padding },
                         { "rx_printconfig", &Caller::rx_printconfig },
                         { "rx_realudpsocksize", &Caller::rx_realudpsocksize },
                         { "rx_silent", &Caller::rx_silent },
                         { "rx_start", &Caller::rx_start },
                         { "rx_status", &Caller::rx_status },
                         { "rx_stop", &Caller::rx_stop },
                         { "rx_tcpport", &Caller::rx_tcpport },
                         { "rx_threads", &Caller::rx_threads },
                         { "rx_udpsocksize", &Caller::rx_udpsocksize },
                         { "rx_version", &Caller::rx_version },
                         { "rx_zmqfreq", &Caller::rx_zmqfreq },
                         { "rx_zmqhwm", &Caller::rx_zmqhwm },
                         { "rx_zmqip", &Caller::rx_zmqip },
                         { "rx_zmqport", &Caller::rx_zmqport },
                         { "rx_zmqstartfnum", &Caller::rx_zmqstartfnum },
                         { "rx_zmqstream", &Caller::rx_zmqstream },
                         { "savepattern", &Caller::savepattern },
                         { "scan", &Caller::scan },
                         { "scanerrmsg", &Caller::scanerrmsg },
                         { "selinterface", &Caller::selinterface },
                         { "serialnumber", &Caller::serialnumber },
                         { "setbit", &Caller::setbit },
                         { "settings", &Caller::settings },
                         { "settingslist", &Caller::settingslist },
                         { "settingspath", &Caller::settingspath },
                         { "signalindex", &Caller::signalindex },
                         { "signallist", &Caller::signallist },
                         { "signalname", &Caller::signalname },
                         { "slowadcindex", &Caller::slowadcindex },
                         { "slowadclist", &Caller::slowadclist },
                         { "slowadcname", &Caller::slowadcname },
                         { "slowadcvalues", &Caller::slowadcvalues },
                         { "start", &Caller::start },
                         { "status", &Caller::status },
                         { "stop", &Caller::stop },
                         { "stopport", &Caller::stopport },
                         { "storagecell_delay", &Caller::storagecell_delay },
                         { "storagecell_start", &Caller::storagecell_start },
                         { "subdeadtime", &Caller::subdeadtime },
                         { "subexptime", &Caller::subexptime },
                         { "sync", &Caller::sync },
                         { "syncclk", &Caller::syncclk },
                         { "temp_10ge", &Caller::temp_10ge },
                         { "temp_adc", &Caller::temp_adc },
                         { "temp_control", &Caller::temp_control },
                         { "temp_dcdc", &Caller::temp_dcdc },
                         { "temp_event", &Caller::temp_event },
                         { "temp_fpga", &Caller::temp_fpga },
                         { "temp_fpgaext", &Caller::temp_fpgaext },
                         { "temp_fpgafl", &Caller::temp_fpgafl },
                         { "temp_fpgafr", &Caller::temp_fpgafr },
                         { "temp_slowadc", &Caller::temp_slowadc },
                         { "temp_sodl", &Caller::temp_sodl },
                         { "temp_sodr", &Caller::temp_sodr },
                         { "temp_threshold", &Caller::temp_threshold },
                         { "templist", &Caller::templist },
                         { "tempvalues", &Caller::tempvalues },
                         { "tengiga", &Caller::tengiga },
                         { "timing", &Caller::timing },
                         { "timinglist", &Caller::timinglist },
                         { "timingsource", &Caller::timingsource },
                         { "top", &Caller::top },
                         { "transceiverenable", &Caller::transceiverenable },
                         { "trigger", &Caller::trigger },
                         { "triggers", &Caller::triggers },
                         { "triggersl", &Caller::triggersl },
                         { "trimbits", &Caller::trimbits },
                         { "trimval", &Caller::trimval },
                         { "tsamples", &Caller::tsamples },
                         { "txdelay", &Caller::txdelay },
                         { "txdelay_frame", &Caller::txdelay_frame },
                         { "txdelay_left", &Caller::txdelay_left },
                         { "txdelay_right", &Caller::txdelay_right },
                         { "type", &Caller::type },
                         { "udp_cleardst", &Caller::udp_cleardst },
                         { "udp_dstlist", &Caller::udp_dstlist },
                         { "udp_dstmac", &Caller::udp_dstmac },
                         { "udp_dstmac2", &Caller::udp_dstmac2 },
                         { "udp_dstport", &Caller::udp_dstport },
                         { "udp_dstport2", &Caller::udp_dstport2 },
                         { "udp_firstdst", &Caller::udp_firstdst },
                         { "udp_numdst", &Caller::udp_numdst },
                         { "udp_reconfigure", &Caller::udp_reconfigure },
                         { "udp_srcmac", &Caller::udp_srcmac },
                         { "udp_srcmac2", &Caller::udp_srcmac2 },
                         { "udp_validate", &Caller::udp_validate },
                         { "update", &Caller::update },
                         { "updatedetectorserver",
                           &Caller::updatedetectorserver },
                         { "updatekernel", &Caller::updatekernel },
                         { "updatemode", &Caller::updatemode },
                         { "user", &Caller::user }, { "v_a", &Caller::v_a },
                         { "v_b", &Caller::v_b }, { "v_c", &Caller::v_c },
                         { "v_chip", &Caller::v_chip }, { "v_d", &Caller::v_d },
                         { "v_io", &Caller::v_io },
                         { "v_limit", &Caller::v_limit },
                         { "vchip_comp_adc", &Caller::vchip_comp_adc },
                         { "vchip_comp_fe", &Caller::vchip_comp_fe },
                         { "vchip_cs", &Caller::vchip_cs },
                         { "vchip_opa_1st", &Caller::vchip_opa_1st },
                         { "vchip_opa_fd", &Caller::vchip_opa_fd },
                         { "vchip_ref_comp_fe", &Caller::vchip_ref_comp_fe },
                         { "veto", &Caller::veto },
                         { "vetoalg", &Caller::vetoalg },
                         { "vetofile", &Caller::vetofile },
                         { "vetophoton", &Caller::vetophoton },
                         { "vetoref", &Caller::vetoref },
                         { "virtual", &Caller::virtualFunction },
                         { "vm_a", &Caller::vm_a }, { "vm_b", &Caller::vm_b },
                         { "vm_c", &Caller::vm_c }, { "vm_d", &Caller::vm_d },
                         { "vm_io", &Caller::vm_io },
                         { "voltageindex", &Caller::voltageindex },
                         { "voltagelist", &Caller::voltagelist },
                         { "voltagename", &Caller::voltagename },
                         { "voltagevalues", &Caller::voltagevalues },
                         { "zmqhwm", &Caller::zmqhwm },
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