#include "CmdParser.h"
#include <iostream>
#include <vector>
#include <map>

namespace sls {
class InferAction {
public:
  InferAction() {}
  int infer(sls::CmdParser &parser, std::ostream &os = std::cout);
  std::vector<std::string> args;
  std::string cmd;

  // generated functions
  int activate();
  int adcclk();
  int adcenable();
  int adcenable10g();
  int adcindex();
  int adcinvert();
  int adclist();
  int adcname();
  int adcphase();
  int adcpipeline();
  int adcreg();
  int adcvpp();
  int apulse();
  int asamples();
  int autocompdisable();
  int blockingtrigger();
  int burstperiod();
  int bursts();
  int burstsl();
  int bustest();
  int cdsgain();
  int chipversion();
  int clearbit();
  int clearbusy();
  int clearroi();
  int clientversion();
  int clkdiv();
  int clkfreq();
  int clkphase();
  int column();
  int compdisabletime();
  int confadc();
  int config();
  int dac();
  int dacindex();
  int daclist();
  int dacname();
  int datastream();
  int dbitclk();
  int dbitphase();
  int dbitpipeline();
  int defaultdac();
  int defaultpattern();
  int delay();
  int delayl();
  int detectorserverversion();
  int detsize();
  int diodelay();
  int dpulse();
  int dr();
  int drlist();
  int dsamples();
  int exptime();
  int exptime1();
  int exptime2();
  int exptime3();
  int exptimel();
  int extrastoragecells();
  int extsampling();
  int extsamplingsrc();
  int extsig();
  int fformat();
  int filtercells();
  int filterresistor();
  int findex();
  int firmwaretest();
  int firmwareversion();
  int fliprows();
  int flowcontrol10g();
  int fmaster();
  int fname();
  int foverwrite();
  int fpath();
  int framecounter();
  int frames();
  int framesl();
  int frametime();
  int fwrite();
  int gainmode();
  int gappixels();
  int gatedelay();
  int gatedelay1();
  int gatedelay2();
  int gatedelay3();
  int gates();
  int getbit();
  int hardwareversion();
  int highvoltage();
  int im_a();
  int im_b();
  int im_c();
  int im_d();
  int im_io();
  int imagetest();
  int initialchecks();
  int inj_ch();
  int interpolation();
  int interruptsubframe();
  int kernelversion();
  int lastclient();
  int led();
  int lock();
  int master();
  int maxadcphaseshift();
  int maxclkphaseshift();
  int maxdbitphaseshift();
  int measuredperiod();
  int measuredsubperiod();
  int moduleid();
  int nextframenumber();
  int nmod();
  int numinterfaces();
  int overflow();
  int packageversion();
  int parallel();
  int parameters();
  int partialreset();
  int patfname();
  int patioctrl();
  int patlimits();
  int patloop();
  int patloop0();
  int patloop1();
  int patloop2();
  int patmask();
  int patnloop();
  int patnloop0();
  int patnloop1();
  int patnloop2();
  int patsetbit();
  int pattern();
  int patternstart();
  int patwait();
  int patwait0();
  int patwait1();
  int patwait2();
  int patwaittime();
  int patwaittime0();
  int patwaittime1();
  int patwaittime2();
  int patword();
  int period();
  int periodl();
  int polarity();
  int port();
  int powerchip();
  int programfpga();
  int pulse();
  int pulsechip();
  int pulsenmove();
  int pumpprobe();
  int quad();
  int readnrows();
  int readout();
  int readoutspeed();
  int readoutspeedlist();
  int rebootcontroller();
  int reg();
  int resetdacs();
  int resetfpga();
  int roi();
  int romode();
  int row();
  int runclk();
  int runtime();
  int rx_arping();
  int rx_clearroi();
  int rx_dbitoffset();
  int rx_discardpolicy();
  int rx_fifodepth();
  int rx_frameindex();
  int rx_framescaught();
  int rx_framesperfile();
  int rx_jsonpara();
  int rx_lastclient();
  int rx_lock();
  int rx_missingpackets();
  int rx_padding();
  int rx_printconfig();
  int rx_realudpsocksize();
  int rx_silent();
  int rx_start();
  int rx_status();
  int rx_stop();
  int rx_tcpport();
  int rx_threads();
  int rx_udpsocksize();
  int rx_version();
  int rx_zmqfreq();
  int rx_zmqhwm();
  int rx_zmqip();
  int rx_zmqport();
  int rx_zmqstartfnum();
  int rx_zmqstream();
  int savepattern();
  int scan();
  int scanerrmsg();
  int selinterface();
  int serialnumber();
  int setbit();
  int settings();
  int settingslist();
  int settingspath();
  int signalindex();
  int signallist();
  int signalname();
  int slowadcindex();
  int slowadclist();
  int slowadcname();
  int slowadcvalues();
  int start();
  int status();
  int stop();
  int stopport();
  int storagecell_delay();
  int storagecell_start();
  int subdeadtime();
  int subexptime();
  int sync();
  int syncclk();
  int temp_10ge();
  int temp_adc();
  int temp_control();
  int temp_dcdc();
  int temp_event();
  int temp_fpga();
  int temp_fpgaext();
  int temp_fpgafl();
  int temp_fpgafr();
  int temp_slowadc();
  int temp_sodl();
  int temp_sodr();
  int temp_threshold();
  int templist();
  int tempvalues();
  int tengiga();
  int timing();
  int timinglist();
  int timingsource();
  int top();
  int transceiverenable();
  int trigger();
  int triggers();
  int triggersl();
  int trimbits();
  int trimval();
  int tsamples();
  int txdelay();
  int txdelay_frame();
  int txdelay_left();
  int txdelay_right();
  int type();
  int udp_cleardst();
  int udp_dstlist();
  int udp_dstmac();
  int udp_dstmac2();
  int udp_dstport();
  int udp_dstport2();
  int udp_firstdst();
  int udp_numdst();
  int udp_reconfigure();
  int udp_srcmac();
  int udp_srcmac2();
  int udp_validate();
  int update();
  int updatedetectorserver();
  int updatekernel();
  int updatemode();
  int user();
  int v_a();
  int v_b();
  int v_c();
  int v_chip();
  int v_d();
  int v_io();
  int v_limit();
  int vchip_comp_adc();
  int vchip_comp_fe();
  int vchip_cs();
  int vchip_opa_1st();
  int vchip_opa_fd();
  int vchip_ref_comp_fe();
  int veto();
  int vetoalg();
  int vetofile();
  int vetophoton();
  int vetoref();
  int virtualFunction();
  int vm_a();
  int vm_b();
  int vm_c();
  int vm_d();
  int vm_io();
  int voltageindex();
  int voltagelist();
  int voltagename();
  int voltagevalues();
  int zmqhwm();
  int zmqip();
  int zmqport();
  //     int frames();

private:
  using FunctionMap = std::map<std::string, int (InferAction::*)()>;
  FunctionMap functions{
    // generated functions
    { "activate", &InferAction::activate }, { "adcclk", &InferAction::adcclk },
    { "adcenable", &InferAction::adcenable },
    { "adcenable10g", &InferAction::adcenable10g },
    { "adcindex", &InferAction::adcindex },
    { "adcinvert", &InferAction::adcinvert },
    { "adclist", &InferAction::adclist }, { "adcname", &InferAction::adcname },
    { "adcphase", &InferAction::adcphase },
    { "adcpipeline", &InferAction::adcpipeline },
    { "adcreg", &InferAction::adcreg }, { "adcvpp", &InferAction::adcvpp },
    { "apulse", &InferAction::apulse }, { "asamples", &InferAction::asamples },
    { "autocompdisable", &InferAction::autocompdisable },
    { "blockingtrigger", &InferAction::blockingtrigger },
    { "burstperiod", &InferAction::burstperiod },
    { "bursts", &InferAction::bursts }, { "burstsl", &InferAction::burstsl },
    { "bustest", &InferAction::bustest }, { "cdsgain", &InferAction::cdsgain },
    { "chipversion", &InferAction::chipversion },
    { "clearbit", &InferAction::clearbit },
    { "clearbusy", &InferAction::clearbusy },
    { "clearroi", &InferAction::clearroi },
    { "clientversion", &InferAction::clientversion },
    { "clkdiv", &InferAction::clkdiv }, { "clkfreq", &InferAction::clkfreq },
    { "clkphase", &InferAction::clkphase }, { "column", &InferAction::column },
    { "compdisabletime", &InferAction::compdisabletime },
    { "confadc", &InferAction::confadc }, { "config", &InferAction::config },
    { "dac", &InferAction::dac }, { "dacindex", &InferAction::dacindex },
    { "daclist", &InferAction::daclist }, { "dacname", &InferAction::dacname },
    { "datastream", &InferAction::datastream },
    { "dbitclk", &InferAction::dbitclk },
    { "dbitphase", &InferAction::dbitphase },
    { "dbitpipeline", &InferAction::dbitpipeline },
    { "defaultdac", &InferAction::defaultdac },
    { "defaultpattern", &InferAction::defaultpattern },
    { "delay", &InferAction::delay }, { "delayl", &InferAction::delayl },
    { "detectorserverversion", &InferAction::detectorserverversion },
    { "detsize", &InferAction::detsize },
    { "diodelay", &InferAction::diodelay }, { "dpulse", &InferAction::dpulse },
    { "dr", &InferAction::dr }, { "drlist", &InferAction::drlist },
    { "dsamples", &InferAction::dsamples },
    { "exptime", &InferAction::exptime },
    { "exptime1", &InferAction::exptime1 },
    { "exptime2", &InferAction::exptime2 },
    { "exptime3", &InferAction::exptime3 },
    { "exptimel", &InferAction::exptimel },
    { "extrastoragecells", &InferAction::extrastoragecells },
    { "extsampling", &InferAction::extsampling },
    { "extsamplingsrc", &InferAction::extsamplingsrc },
    { "extsig", &InferAction::extsig }, { "fformat", &InferAction::fformat },
    { "filtercells", &InferAction::filtercells },
    { "filterresistor", &InferAction::filterresistor },
    { "findex", &InferAction::findex },
    { "firmwaretest", &InferAction::firmwaretest },
    { "firmwareversion", &InferAction::firmwareversion },
    { "fliprows", &InferAction::fliprows },
    { "flowcontrol10g", &InferAction::flowcontrol10g },
    { "fmaster", &InferAction::fmaster }, { "fname", &InferAction::fname },
    { "foverwrite", &InferAction::foverwrite },
    { "fpath", &InferAction::fpath },
    { "framecounter", &InferAction::framecounter },
    { "frames", &InferAction::frames }, { "framesl", &InferAction::framesl },
    { "frametime", &InferAction::frametime },
    { "fwrite", &InferAction::fwrite }, { "gainmode", &InferAction::gainmode },
    { "gappixels", &InferAction::gappixels },
    { "gatedelay", &InferAction::gatedelay },
    { "gatedelay1", &InferAction::gatedelay1 },
    { "gatedelay2", &InferAction::gatedelay2 },
    { "gatedelay3", &InferAction::gatedelay3 },
    { "gates", &InferAction::gates }, { "getbit", &InferAction::getbit },
    { "hardwareversion", &InferAction::hardwareversion },
    { "highvoltage", &InferAction::highvoltage },
    { "im_a", &InferAction::im_a }, { "im_b", &InferAction::im_b },
    { "im_c", &InferAction::im_c }, { "im_d", &InferAction::im_d },
    { "im_io", &InferAction::im_io }, { "imagetest", &InferAction::imagetest },
    { "initialchecks", &InferAction::initialchecks },
    { "inj_ch", &InferAction::inj_ch },
    { "interpolation", &InferAction::interpolation },
    { "interruptsubframe", &InferAction::interruptsubframe },
    { "kernelversion", &InferAction::kernelversion },
    { "lastclient", &InferAction::lastclient }, { "led", &InferAction::led },
    { "lock", &InferAction::lock }, { "master", &InferAction::master },
    { "maxadcphaseshift", &InferAction::maxadcphaseshift },
    { "maxclkphaseshift", &InferAction::maxclkphaseshift },
    { "maxdbitphaseshift", &InferAction::maxdbitphaseshift },
    { "measuredperiod", &InferAction::measuredperiod },
    { "measuredsubperiod", &InferAction::measuredsubperiod },
    { "moduleid", &InferAction::moduleid },
    { "nextframenumber", &InferAction::nextframenumber },
    { "nmod", &InferAction::nmod },
    { "numinterfaces", &InferAction::numinterfaces },
    { "overflow", &InferAction::overflow },
    { "packageversion", &InferAction::packageversion },
    { "parallel", &InferAction::parallel },
    { "parameters", &InferAction::parameters },
    { "partialreset", &InferAction::partialreset },
    { "patfname", &InferAction::patfname },
    { "patioctrl", &InferAction::patioctrl },
    { "patlimits", &InferAction::patlimits },
    { "patloop", &InferAction::patloop },
    { "patloop0", &InferAction::patloop0 },
    { "patloop1", &InferAction::patloop1 },
    { "patloop2", &InferAction::patloop2 },
    { "patmask", &InferAction::patmask },
    { "patnloop", &InferAction::patnloop },
    { "patnloop0", &InferAction::patnloop0 },
    { "patnloop1", &InferAction::patnloop1 },
    { "patnloop2", &InferAction::patnloop2 },
    { "patsetbit", &InferAction::patsetbit },
    { "pattern", &InferAction::pattern },
    { "patternstart", &InferAction::patternstart },
    { "patwait", &InferAction::patwait },
    { "patwait0", &InferAction::patwait0 },
    { "patwait1", &InferAction::patwait1 },
    { "patwait2", &InferAction::patwait2 },
    { "patwaittime", &InferAction::patwaittime },
    { "patwaittime0", &InferAction::patwaittime0 },
    { "patwaittime1", &InferAction::patwaittime1 },
    { "patwaittime2", &InferAction::patwaittime2 },
    { "patword", &InferAction::patword }, { "period", &InferAction::period },
    { "periodl", &InferAction::periodl },
    { "polarity", &InferAction::polarity }, { "port", &InferAction::port },
    { "powerchip", &InferAction::powerchip },
    { "programfpga", &InferAction::programfpga },
    { "pulse", &InferAction::pulse }, { "pulsechip", &InferAction::pulsechip },
    { "pulsenmove", &InferAction::pulsenmove },
    { "pumpprobe", &InferAction::pumpprobe }, { "quad", &InferAction::quad },
    { "readnrows", &InferAction::readnrows },
    { "readout", &InferAction::readout },
    { "readoutspeed", &InferAction::readoutspeed },
    { "readoutspeedlist", &InferAction::readoutspeedlist },
    { "rebootcontroller", &InferAction::rebootcontroller },
    { "reg", &InferAction::reg }, { "resetdacs", &InferAction::resetdacs },
    { "resetfpga", &InferAction::resetfpga }, { "roi", &InferAction::roi },
    { "romode", &InferAction::romode }, { "row", &InferAction::row },
    { "runclk", &InferAction::runclk }, { "runtime", &InferAction::runtime },
    { "rx_arping", &InferAction::rx_arping },
    { "rx_clearroi", &InferAction::rx_clearroi },
    { "rx_dbitoffset", &InferAction::rx_dbitoffset },
    { "rx_discardpolicy", &InferAction::rx_discardpolicy },
    { "rx_fifodepth", &InferAction::rx_fifodepth },
    { "rx_frameindex", &InferAction::rx_frameindex },
    { "rx_framescaught", &InferAction::rx_framescaught },
    { "rx_framesperfile", &InferAction::rx_framesperfile },
    { "rx_jsonpara", &InferAction::rx_jsonpara },
    { "rx_lastclient", &InferAction::rx_lastclient },
    { "rx_lock", &InferAction::rx_lock },
    { "rx_missingpackets", &InferAction::rx_missingpackets },
    { "rx_padding", &InferAction::rx_padding },
    { "rx_printconfig", &InferAction::rx_printconfig },
    { "rx_realudpsocksize", &InferAction::rx_realudpsocksize },
    { "rx_silent", &InferAction::rx_silent },
    { "rx_start", &InferAction::rx_start },
    { "rx_status", &InferAction::rx_status },
    { "rx_stop", &InferAction::rx_stop },
    { "rx_tcpport", &InferAction::rx_tcpport },
    { "rx_threads", &InferAction::rx_threads },
    { "rx_udpsocksize", &InferAction::rx_udpsocksize },
    { "rx_version", &InferAction::rx_version },
    { "rx_zmqfreq", &InferAction::rx_zmqfreq },
    { "rx_zmqhwm", &InferAction::rx_zmqhwm },
    { "rx_zmqip", &InferAction::rx_zmqip },
    { "rx_zmqport", &InferAction::rx_zmqport },
    { "rx_zmqstartfnum", &InferAction::rx_zmqstartfnum },
    { "rx_zmqstream", &InferAction::rx_zmqstream },
    { "savepattern", &InferAction::savepattern },
    { "scan", &InferAction::scan }, { "scanerrmsg", &InferAction::scanerrmsg },
    { "selinterface", &InferAction::selinterface },
    { "serialnumber", &InferAction::serialnumber },
    { "setbit", &InferAction::setbit }, { "settings", &InferAction::settings },
    { "settingslist", &InferAction::settingslist },
    { "settingspath", &InferAction::settingspath },
    { "signalindex", &InferAction::signalindex },
    { "signallist", &InferAction::signallist },
    { "signalname", &InferAction::signalname },
    { "slowadcindex", &InferAction::slowadcindex },
    { "slowadclist", &InferAction::slowadclist },
    { "slowadcname", &InferAction::slowadcname },
    { "slowadcvalues", &InferAction::slowadcvalues },
    { "start", &InferAction::start }, { "status", &InferAction::status },
    { "stop", &InferAction::stop }, { "stopport", &InferAction::stopport },
    { "storagecell_delay", &InferAction::storagecell_delay },
    { "storagecell_start", &InferAction::storagecell_start },
    { "subdeadtime", &InferAction::subdeadtime },
    { "subexptime", &InferAction::subexptime }, { "sync", &InferAction::sync },
    { "syncclk", &InferAction::syncclk },
    { "temp_10ge", &InferAction::temp_10ge },
    { "temp_adc", &InferAction::temp_adc },
    { "temp_control", &InferAction::temp_control },
    { "temp_dcdc", &InferAction::temp_dcdc },
    { "temp_event", &InferAction::temp_event },
    { "temp_fpga", &InferAction::temp_fpga },
    { "temp_fpgaext", &InferAction::temp_fpgaext },
    { "temp_fpgafl", &InferAction::temp_fpgafl },
    { "temp_fpgafr", &InferAction::temp_fpgafr },
    { "temp_slowadc", &InferAction::temp_slowadc },
    { "temp_sodl", &InferAction::temp_sodl },
    { "temp_sodr", &InferAction::temp_sodr },
    { "temp_threshold", &InferAction::temp_threshold },
    { "templist", &InferAction::templist },
    { "tempvalues", &InferAction::tempvalues },
    { "tengiga", &InferAction::tengiga }, { "timing", &InferAction::timing },
    { "timinglist", &InferAction::timinglist },
    { "timingsource", &InferAction::timingsource },
    { "top", &InferAction::top },
    { "transceiverenable", &InferAction::transceiverenable },
    { "trigger", &InferAction::trigger },
    { "triggers", &InferAction::triggers },
    { "triggersl", &InferAction::triggersl },
    { "trimbits", &InferAction::trimbits },
    { "trimval", &InferAction::trimval },
    { "tsamples", &InferAction::tsamples },
    { "txdelay", &InferAction::txdelay },
    { "txdelay_frame", &InferAction::txdelay_frame },
    { "txdelay_left", &InferAction::txdelay_left },
    { "txdelay_right", &InferAction::txdelay_right },
    { "type", &InferAction::type },
    { "udp_cleardst", &InferAction::udp_cleardst },
    { "udp_dstlist", &InferAction::udp_dstlist },
    { "udp_dstmac", &InferAction::udp_dstmac },
    { "udp_dstmac2", &InferAction::udp_dstmac2 },
    { "udp_dstport", &InferAction::udp_dstport },
    { "udp_dstport2", &InferAction::udp_dstport2 },
    { "udp_firstdst", &InferAction::udp_firstdst },
    { "udp_numdst", &InferAction::udp_numdst },
    { "udp_reconfigure", &InferAction::udp_reconfigure },
    { "udp_srcmac", &InferAction::udp_srcmac },
    { "udp_srcmac2", &InferAction::udp_srcmac2 },
    { "udp_validate", &InferAction::udp_validate },
    { "update", &InferAction::update },
    { "updatedetectorserver", &InferAction::updatedetectorserver },
    { "updatekernel", &InferAction::updatekernel },
    { "updatemode", &InferAction::updatemode }, { "user", &InferAction::user },
    { "v_a", &InferAction::v_a }, { "v_b", &InferAction::v_b },
    { "v_c", &InferAction::v_c }, { "v_chip", &InferAction::v_chip },
    { "v_d", &InferAction::v_d }, { "v_io", &InferAction::v_io },
    { "v_limit", &InferAction::v_limit },
    { "vchip_comp_adc", &InferAction::vchip_comp_adc },
    { "vchip_comp_fe", &InferAction::vchip_comp_fe },
    { "vchip_cs", &InferAction::vchip_cs },
    { "vchip_opa_1st", &InferAction::vchip_opa_1st },
    { "vchip_opa_fd", &InferAction::vchip_opa_fd },
    { "vchip_ref_comp_fe", &InferAction::vchip_ref_comp_fe },
    { "veto", &InferAction::veto }, { "vetoalg", &InferAction::vetoalg },
    { "vetofile", &InferAction::vetofile },
    { "vetophoton", &InferAction::vetophoton },
    { "vetoref", &InferAction::vetoref },
    { "virtual", &InferAction::virtualFunction },
    { "vm_a", &InferAction::vm_a }, { "vm_b", &InferAction::vm_b },
    { "vm_c", &InferAction::vm_c }, { "vm_d", &InferAction::vm_d },
    { "vm_io", &InferAction::vm_io },
    { "voltageindex", &InferAction::voltageindex },
    { "voltagelist", &InferAction::voltagelist },
    { "voltagename", &InferAction::voltagename },
    { "voltagevalues", &InferAction::voltagevalues },
    { "zmqhwm", &InferAction::zmqhwm }, { "zmqip", &InferAction::zmqip },
    { "zmqport", &InferAction::zmqport }

    //        {"frames",&InferAction::frames}
  };
};
}