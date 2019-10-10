#include "catch.hpp"
#include "multiSlsDetector.h"
#include "multiSlsDetectorClient.h"
#include "sls_detector_defs.h"
#include <sstream>

#include "tests/globals.h"

auto GET = slsDetectorDefs::GET_ACTION;
auto PUT = slsDetectorDefs::PUT_ACTION;


TEST_CASE("rx_hostname", "[.cmd]") {
    std::string s;
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_hostname", GET, nullptr, oss));
        s = oss.str();
    }
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_hostname", GET, nullptr, oss));
        REQUIRE(oss.str() == s);
    } 
    // save rx_hostame's ip somewhere (getent hosts [rx_hostname])
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_hostname 129.129.205.80", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_hostname", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_hostname 129.129.205.80\n");
    }
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_hostname none", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_hostname", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_hostname none\n");
    }        
}

TEST_CASE("txndelay", "[.cmd][.eiger][.jungfrau]") {
    if (test::type == slsDetectorDefs::EIGER) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_frame 50000", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_frame", GET, nullptr, oss));
            REQUIRE(oss.str() == "txndelay_frame 50000\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("txndelay_frame 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_frame", GET, nullptr, oss));
            REQUIRE(oss.str() == "txndelay_frame 0\n");
        } 
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_left 50000", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_left", GET, nullptr, oss));
            REQUIRE(oss.str() == "txndelay_left 50000\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("txndelay_left 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_left", GET, nullptr, oss));
            REQUIRE(oss.str() == "txndelay_left 0\n");
        } 
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_right 50000", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_right", GET, nullptr, oss));
            REQUIRE(oss.str() == "txndelay_right 50000\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("txndelay_right 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_right", GET, nullptr, oss));
            REQUIRE(oss.str() == "txndelay_right 0\n");
        }     
    } else if (test::type == slsDetectorDefs::JUNGFRAU) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("txndelay_frame 5", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_frame", GET, nullptr, oss));
            REQUIRE(oss.str() == "txndelay_frame 5\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("txndelay_frame 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:txndelay_frame", GET, nullptr, oss));
            REQUIRE(oss.str() == "txndelay_frame 0\n");
        } 
        REQUIRE_THROWS(multiSlsDetectorClient("txndelay_frame 32", PUT));  
        REQUIRE_THROWS(multiSlsDetectorClient("txndelay_left 32", PUT));  
        REQUIRE_THROWS(multiSlsDetectorClient("txndelay_right 32", PUT));  
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("txndelay_frame", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("txndelay_left 32", PUT));  
        REQUIRE_THROWS(multiSlsDetectorClient("txndelay_right 32", PUT));  
    }
}


TEST_CASE("flowcontrol_10g", "[.cmd][.eiger][.jungfrau]") {
    if (test::type == slsDetectorDefs::EIGER || test::type == slsDetectorDefs::JUNGFRAU) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("flowcontrol_10g 1", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:flowcontrol_10g", GET, nullptr, oss));
            REQUIRE(oss.str() == "flowcontrol_10g 1\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("flowcontrol_10g 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:flowcontrol_10g", GET, nullptr, oss));
            REQUIRE(oss.str() == "flowcontrol_10g 0\n");
        } 
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("flowcontrol_10g", GET));
    }
}

TEST_CASE("tengiga", "[.cmd][.eiger][.ctb]") {
    if (test::type == slsDetectorDefs::EIGER || test::type == slsDetectorDefs::CHIPTESTBOARD) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("tengiga 1", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:tengiga", GET, nullptr, oss));
            REQUIRE(oss.str() == "tengiga 1\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("tengiga 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:tengiga", GET, nullptr, oss));
            REQUIRE(oss.str() == "tengiga 0\n");
        } 
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("tengiga", GET));
    }
}

TEST_CASE("rx_printconfig", "[.cmd]") {
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_printconfig", GET));
}

TEST_CASE("network", "[.cmd]") {
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip 129.129.202.84", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip", GET, nullptr, oss));
        REQUIRE(oss.str() == "udp_srcip 129.129.202.84\n");
    }
    std::string udp_dstip;
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstip", GET, nullptr, oss));
        udp_dstip = oss.str();
    }
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient(udp_dstip, PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstip", GET, nullptr, oss));
        REQUIRE(oss.str() == udp_dstip);
    }   
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstmac 10:e7:c6:48:bd:3f", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstmac", GET, nullptr, oss));
        REQUIRE(oss.str() == "udp_dstmac 10:e7:c6:48:bd:3f\n");
    }      
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport 6200", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport", GET, nullptr, oss));
        REQUIRE(oss.str() == "udp_dstport 6200\n");
    }  
    REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip 0.0.0.0", PUT));
    REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip 124586954", PUT));
    REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip 999.999.0.0.0.5", PUT));

    if (test::type == slsDetectorDefs::JUNGFRAU) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip2 129.129.202.84", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip2", GET, nullptr, oss));
            REQUIRE(oss.str() == "udp_srcip2 129.129.202.84\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstip2", GET, nullptr, oss));
            udp_dstip = oss.str();
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient(udp_dstip, PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstip2", GET, nullptr, oss));
            REQUIRE(oss.str() == udp_dstip);
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstmac2 10:e7:c6:48:bd:3f", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstmac2", GET, nullptr, oss));
            REQUIRE(oss.str() == "udp_dstmac2 10:e7:c6:48:bd:3f\n");
        }  
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport2 6400", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport2", GET, nullptr, oss));
            REQUIRE(oss.str() == "udp_dstport2 6400\n");
        }  
    } else if (test::type == slsDetectorDefs::EIGER) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport2 6400", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport2", GET, nullptr, oss));
            REQUIRE(oss.str() == "udp_dstport2 6400\n");
        } 
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip2", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("udp_dstip2", GET));  
        REQUIRE_THROWS(multiSlsDetectorClient("udp_srcmac2", GET));         
        REQUIRE_THROWS(multiSlsDetectorClient("udp_dstmac2", GET));      
        REQUIRE_THROWS(multiSlsDetectorClient("udp_dstport2", GET));              
    }
}


TEST_CASE("selinterface", "[.cmd][.jungfrau]") {
    if (test::type == slsDetectorDefs::JUNGFRAU) {
        REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 1", PUT));
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("selinterface 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:selinterface", GET, nullptr, oss));
            REQUIRE(oss.str() == "selinterface 0\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("selinterface 1", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:selinterface", GET, nullptr, oss));
            REQUIRE(oss.str() == "selinterface 1\n");
        }
       REQUIRE_THROWS(multiSlsDetectorClient("selinterface 2", PUT)); 
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("selinterface", GET));
    }
}

TEST_CASE("numinterfaces", "[.cmd][.jungfrau]") {
    if (test::type == slsDetectorDefs::JUNGFRAU) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 2", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:numinterfaces", GET, nullptr, oss));
            REQUIRE(oss.str() == "numinterfaces 2\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 1", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:numinterfaces", GET, nullptr, oss));
            REQUIRE(oss.str() == "numinterfaces 1\n");
        }
    } else {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:numinterfaces", GET, nullptr, oss));
        REQUIRE(oss.str() == "numinterfaces 1\n");
    }
    REQUIRE_THROWS(multiSlsDetectorClient("numinterfaces 3", PUT));
    REQUIRE_THROWS(multiSlsDetectorClient("numinterfaces 0", PUT));
}

TEST_CASE("timing", "[.cmd]") {
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
        std::ostringstream oss;
        multiSlsDetectorClient("timing", GET, nullptr, oss);
        REQUIRE(oss.str() == "timing auto\n");
    }
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("timing trigger", PUT));
        std::ostringstream oss;
        multiSlsDetectorClient("timing", GET, nullptr, oss);
        REQUIRE(oss.str() == "timing trigger\n");
    }
    if (test::type == slsDetectorDefs::EIGER) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("timing gating", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("timing", GET, nullptr, oss);
            REQUIRE(oss.str() == "timing gating\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("timing burst_trigger", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("timing", GET, nullptr, oss);
            REQUIRE(oss.str() == "timing burst_trigger\n");
        }
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("timing gating", PUT));
        REQUIRE_THROWS(multiSlsDetectorClient("timing burst_trigger", PUT));
    }
}


TEST_CASE("adc", "[.cmd][.ctb]") {
    if (test::type != slsDetectorDefs::CHIPTESTBOARD) {
        REQUIRE_THROWS(multiSlsDetectorClient("adc 8", GET));      
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("adc", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("adc 5", PUT));
        for(int i = 0; i <= 8; ++i) {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adc " + std::to_string(i), GET));       
        }
    }
}



TEST_CASE("temp_fpga", "[.cmd][.eiger][.jungfrau][.gotthard]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {
        REQUIRE_THROWS(multiSlsDetectorClient("temp_fpga", GET));      
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("temp_fpga 0", PUT));      
        std::ostringstream oss;
        multiSlsDetectorClient("0:temp_fpga", GET, nullptr, oss);
        std::string s = (oss.str()).erase (0, strlen("temp_fpga "));
        REQUIRE(std::stoi(s) != -1);
    }
}

TEST_CASE("temp_adc", "[.cmd][.jungfrau][.gotthard]") {
    if (test::type != slsDetectorDefs::GOTTHARD && test::type != slsDetectorDefs::JUNGFRAU ) {
        REQUIRE_THROWS(multiSlsDetectorClient("temp_adc", GET));      
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("temp_adc 0", PUT));      
        std::ostringstream oss;
        multiSlsDetectorClient("0:temp_adc", GET, nullptr, oss);
        std::string s = (oss.str()).erase (0, strlen("temp_adc "));
        REQUIRE(std::stoi(s) != -1);
    }
}


TEST_CASE("temp", "[.cmd][.eiger]") {
    if (test::type != slsDetectorDefs::EIGER) {
        REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgaext", GET));  
        REQUIRE_THROWS(multiSlsDetectorClient("temp_10ge", GET)); 
        REQUIRE_THROWS(multiSlsDetectorClient("temp_dcdc", GET)); 
        REQUIRE_THROWS(multiSlsDetectorClient("temp_sodl", GET)); 
        REQUIRE_THROWS(multiSlsDetectorClient("temp_sodr", GET)); 
        REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgafl", GET));     
        REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgafr", GET));     
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgaext 0", PUT));  
        REQUIRE_THROWS(multiSlsDetectorClient("temp_10ge 0", PUT)); 
        REQUIRE_THROWS(multiSlsDetectorClient("temp_dcdc 0", PUT)); 
        REQUIRE_THROWS(multiSlsDetectorClient("temp_sodl 0", PUT)); 
        REQUIRE_THROWS(multiSlsDetectorClient("temp_sodr 0", PUT)); 
        REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgafl 0", PUT));     
        REQUIRE_THROWS(multiSlsDetectorClient("temp_fpgafr 0", PUT));         
        {
            std::ostringstream oss;
            multiSlsDetectorClient("0:temp_fpgaext", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("temp_fpgaext "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            multiSlsDetectorClient("0:temp_10ge", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("temp_10ge "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            multiSlsDetectorClient("0:temp_dcdc", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("temp_dcdc "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            multiSlsDetectorClient("0:temp_sodl", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("temp_sodl "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            multiSlsDetectorClient("0:temp_sodr", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("temp_sodr "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            multiSlsDetectorClient("0:temp_fpgafl", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("temp_fpgafl "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            multiSlsDetectorClient("0:temp_fpgafr", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("temp_fpgafr "));
            REQUIRE(std::stoi(s) != -1);
        }
    }
}


TEST_CASE("vhighvoltage", "[.cmd]") {
    int prev_val = 0;
    {
        std::ostringstream oss;
        multiSlsDetectorClient("vhighvoltage", GET, nullptr, oss);
        std::string s = (oss.str()).erase (0, strlen("vhighvoltage "));
        // std::stoi doesnt convert [0,-999] beccause of [
        if (s.find('[') != std::string::npos) {
            s.erase(0, 1);
        }
        prev_val = std::stoi(s);
    }
    if (test::type == slsDetectorDefs::GOTTHARD) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 90", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("vhighvoltage", GET, nullptr, oss);
            REQUIRE(oss.str() == "vhighvoltage 90\n");
        }    
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 0", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("vhighvoltage", GET, nullptr, oss);
            REQUIRE(oss.str() == "vhighvoltage 0\n");
        } 
        REQUIRE_THROWS(multiSlsDetectorClient("vhighvoltage 50", PUT));      
    } else { 
        if (test::type != slsDetectorDefs::EIGER) {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 50", PUT));            
        } else {
            REQUIRE_THROWS(multiSlsDetectorClient("vhighvoltage 50", PUT));   
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 120", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("vhighvoltage", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("vhighvoltage "));
            REQUIRE(s.find("120") != std::string::npos); // due to different values for highvoltage for eiger
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 0", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("vhighvoltage", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("vhighvoltage "));
            REQUIRE(s.find("0") != std::string::npos); // due to different values for highvoltage for eiger
        }         
        REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage " + std::to_string(prev_val), PUT));                    
    } 
}



TEST_CASE("maxadcphaseshift", "[.cmd][.ctb][.jungfrau]") {
    if (test::type != slsDetectorDefs::CHIPTESTBOARD && test::type != slsDetectorDefs::JUNGFRAU) {
       REQUIRE_THROWS(multiSlsDetectorClient("maxadcphaseshift", GET));       
    } else { 
        REQUIRE_NOTHROW(multiSlsDetectorClient("maxadcphaseshift", GET));              
    } 
}

TEST_CASE("adcphase", "[.cmd][.ctb][.jungfrau][.gotthard]") {
    if (test::type != slsDetectorDefs::CHIPTESTBOARD && test::type != slsDetectorDefs::JUNGFRAU && test::type != slsDetectorDefs::GOTTHARD) {
       REQUIRE_THROWS(multiSlsDetectorClient("adcphase", GET));       
    } else { 
        int prev_val = 0;   
        {
            std::ostringstream oss;
            multiSlsDetectorClient("adcphase", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("adcphase "));
            prev_val = std::stoi(s);
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 20", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("adcphase", GET, nullptr, oss);
            REQUIRE(oss.str() == "adcphase 20\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 0", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("adcphase", GET, nullptr, oss);
            REQUIRE(oss.str() == "adcphase 0\n");
        }     
        if (test::type != slsDetectorDefs::GOTTHARD) {
            REQUIRE_THROWS(multiSlsDetectorClient("adcphase deg", GET));
        } else {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 20 deg", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("adcphase deg", GET, nullptr, oss);
            REQUIRE(oss.str() == "adcphase 20 deg\n");
        }          
        REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase " + std::to_string(prev_val), PUT));                    
    } 
}

TEST_CASE("runclk", "[.cmd][.ctb]") {
    if(test::type != slsDetectorDefs::CHIPTESTBOARD) {
       ;// REQUIRE_THROWS(multiSlsDetectorClient("runclk", GET)); Only once setspeed is split into many  (runclk = speed for now)      
    } else { 
        int prev_runclk = 0;   
        {
            std::ostringstream oss;
            multiSlsDetectorClient("runclk", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("runclk "));
            prev_runclk = std::stoi(s);
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("runclk 20", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("runclk", GET, nullptr, oss);
            REQUIRE(oss.str() == "runclk 20\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("runclk 0", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("runclk", GET, nullptr, oss);
            REQUIRE(oss.str() == "runclk 0\n");
        }        
        REQUIRE_NOTHROW(multiSlsDetectorClient("runclk " + std::to_string(prev_runclk), PUT));                    
    } 
}

TEST_CASE("speed", "[.cmd][.eiger][.jungfrau]") {
    if(test::type != slsDetectorDefs::EIGER && test::type != slsDetectorDefs::JUNGFRAU) {
        REQUIRE_THROWS(multiSlsDetectorClient("speed", GET));         
    } else {    
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed 0", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("speed", GET, nullptr, oss);
            REQUIRE(oss.str() == "speed full_speed\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed full_speed", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("speed", GET, nullptr, oss);
            REQUIRE(oss.str() == "speed full_speed\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed 1", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("speed", GET, nullptr, oss);
            REQUIRE(oss.str() == "speed half_speed\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed half_speed", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("speed", GET, nullptr, oss);
            REQUIRE(oss.str() == "speed half_speed\n");
        }  
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed 2", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("speed", GET, nullptr, oss);
            REQUIRE(oss.str() == "speed quarter_speed\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed quarter_speed", PUT));
            std::ostringstream oss;
            multiSlsDetectorClient("speed", GET, nullptr, oss);
            REQUIRE(oss.str() == "speed quarter_speed\n");
        }
        REQUIRE_THROWS(multiSlsDetectorClient("speed 3", PUT));                         
    } 
}

TEST_CASE("triggers", "[.cmd]") {
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 10", PUT));
        std::ostringstream oss;
        multiSlsDetectorClient("triggers", GET, nullptr, oss);
        REQUIRE(oss.str() == "triggers 10\n");
    } 
}

TEST_CASE("settings", "[.cmd]") {
    switch(test::type) {
        case slsDetectorDefs::EIGER:
            REQUIRE_THROWS(multiSlsDetectorClient("settings mediumgain", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings standard", PUT));
            break;
        case slsDetectorDefs::JUNGFRAU:
            REQUIRE_THROWS(multiSlsDetectorClient("settings standard", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings dynamicgain", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings dynamichg0", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings fixgain1", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings fixgain2", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings forceswitchg1", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings forceswitchg2", PUT));
        break;
        default:
            break;
    }
    REQUIRE_NOTHROW(multiSlsDetectorClient("settings", GET));
}

TEST_CASE("threshold", "[.cmd]") {
    REQUIRE_NOTHROW(multiSlsDetectorClient("threshold 6400 standard", PUT));
    REQUIRE_NOTHROW(multiSlsDetectorClient("thresholdnotb 6400 standard", PUT));
    REQUIRE_NOTHROW(multiSlsDetectorClient("threshold 6400", PUT));
    REQUIRE_NOTHROW(multiSlsDetectorClient("thresholdnotb 6400", PUT));
    {
        std::ostringstream oss;
        multiSlsDetectorClient("threshold", GET, nullptr, oss);
        std::string s = (oss.str()).erase (0, strlen("threshold "));
        REQUIRE(std::stoi(s) == 6400);
        REQUIRE_THROWS(multiSlsDetectorClient("thresholdnotb", GET));
    } 
}


TEST_CASE("detsize", "[.cmd]") {
    CHECK_NOTHROW(multiSlsDetectorClient("detize", GET));
}

TEST_CASE("type", "[.cmd]") {
    CHECK_NOTHROW(multiSlsDetectorClient("type", GET));
}

TEST_CASE("firmwareversion", "[.cmd]") {
    {
        std::ostringstream oss;
        CHECK_NOTHROW(multiSlsDetectorClient("firmwareversion", GET, nullptr, oss));
    }
}
TEST_CASE("status", "[.cmd]") {

    multiSlsDetectorClient("timing auto", PUT);
    multiSlsDetectorClient("frames 10", PUT);
    multiSlsDetectorClient("period 1", PUT);

    {
        std::ostringstream oss;
        multiSlsDetectorClient("start", PUT, nullptr, oss);
        REQUIRE(oss.str() == "start successful\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("status", GET, nullptr, oss);
        REQUIRE(oss.str() == "status running\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("stop", PUT, nullptr, oss);
        REQUIRE(oss.str() == "stop successful\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("status", GET, nullptr, oss);
        REQUIRE(oss.str() == "status idle\n");
    }
}

TEST_CASE("trigger", "[.cmd][.eiger]") {
    // trigger
    {
        std::ostringstream oss;
        multiSlsDetectorClient("timing trigger", PUT, nullptr, oss);
        REQUIRE(oss.str() == "timing trigger\n");
    }
    int startingfnum = 0;
    {
        std::ostringstream oss;
        multiSlsDetectorClient("startingfnum", GET, nullptr, oss);
        std::string s = (oss.str()).erase (0, strlen("startingfnum "));
        startingfnum = std::stoi(s);
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("start", PUT, nullptr, oss);
        REQUIRE(oss.str() == "start successful\n");
    } 
    {
        std::ostringstream oss;
        multiSlsDetectorClient("status", GET, nullptr, oss);
        REQUIRE(oss.str() == "status running\n");
    }   
    {
        std::ostringstream oss;
        multiSlsDetectorClient("trigger", PUT, nullptr, oss);
        REQUIRE(oss.str() == "trigger successful\n");
    }
    multiSlsDetectorClient("stop", PUT);
    int currentfnum = 0;
    {
        std::ostringstream oss;
        multiSlsDetectorClient("startingfnum", GET, nullptr, oss);
        std::string s = (oss.str()).erase (0, strlen("startingfnum "));
        currentfnum = std::stoi(s);
    } 
    REQUIRE((startingfnum + 1) == currentfnum);


     multiSlsDetectorClient("timing auto", PUT);
}


TEST_CASE("framesl", "[.cmd][.jungfrau][gotthard][ctb]") {
    if(test::type == slsDetectorDefs::EIGER) {
        REQUIRE_THROWS(multiSlsDetectorClient("framesl", GET));         
    } else {
        multiSlsDetectorClient("timing auto", PUT);
        multiSlsDetectorClient("frames 10", PUT);
        multiSlsDetectorClient("period 1", PUT);
        multiSlsDetectorClient("status start", PUT);
        {
            std::ostringstream oss;
            multiSlsDetectorClient("framesl", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("framesl "));
            int framesl = std::stoi(s);
            REQUIRE(framesl > 0);
        }
        multiSlsDetectorClient("stop", PUT);
    }
}

TEST_CASE("triggersl", "[.cmd][.jungfrau][gotthard][ctb]") {
    if(test::type == slsDetectorDefs::EIGER) {
        REQUIRE_THROWS(multiSlsDetectorClient("triggersl", GET));       
    } else {
        multiSlsDetectorClient("timing trigger", PUT);
        multiSlsDetectorClient("frames 1", PUT);
        multiSlsDetectorClient("triggers 10", PUT);   
        multiSlsDetectorClient("status start", PUT);
        {
            std::ostringstream oss;
            multiSlsDetectorClient("triggersl", GET, nullptr, oss);
            std::string s = (oss.str()).erase (0, strlen("framesl "));
            int triggersl = std::stoi(s);
            REQUIRE(triggersl  == 10);
        }
        multiSlsDetectorClient("stop", PUT);
    }
}

TEST_CASE("delayl", "[.cmd][.jungfrau][gotthard][ctb]") {
    if(test::type == slsDetectorDefs::EIGER) {
        REQUIRE_THROWS(multiSlsDetectorClient("delayl", GET));       
    } else {
        multiSlsDetectorClient("timing trigger", PUT);
        multiSlsDetectorClient("frames 1", PUT);
        multiSlsDetectorClient("triggers 2", PUT);  
         multiSlsDetectorClient("delay 1", PUT);          
        multiSlsDetectorClient("status start", PUT);
        {
            std::ostringstream oss;
            multiSlsDetectorClient("delayl s", GET, nullptr, oss);
            REQUIRE(oss.str()  == "delayl 1s\n");
        }
        multiSlsDetectorClient("stop", PUT);
    }
}

TEST_CASE("rx_fifodepth", "[.cmd]") {

    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_fifodepth 10", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_fifodepth 10\n");
    }

    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_fifodepth 100", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_fifodepth 100\n");
    }

    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_fifodepth", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_fifodepth 100\n");
    }
}

TEST_CASE("frames", "[.cmd]") {

    {
        std::ostringstream oss;
        multiSlsDetectorClient("frames 1000", PUT, nullptr, oss);
        REQUIRE(oss.str() == "frames 1000\n");
    }

    {
        std::ostringstream oss;
        multiSlsDetectorClient("frames", GET, nullptr, oss);
        REQUIRE(oss.str() == "frames 1000\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("frames 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "frames 1\n");
    }
}

TEST_CASE("rx_status", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_start", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_start successful\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_status", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_status running\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_stop", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_stop successful\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_status", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_status idle\n");
    }
}

TEST_CASE("fwrite", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fwrite 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fwrite", GET, nullptr, oss);
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fwrite 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fwrite 0\n");
    }
}

TEST_CASE("enablefoverwrite", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("foverwrite 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("foverwrite", GET, nullptr, oss);
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("foverwrite 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "foverwrite 0\n");
    }
}

// EIGER ONLY
// TEST_CASE("activatecmd", "[.cmd]") {

//     {
//         // TODO! read padding from somewhere
//         std::ostringstream oss;
//         multiSlsDetectorClient("activate 0", PUT, nullptr, oss);
//         REQUIRE(oss.str() == "activate 0 padding\n");
//     }
//     {
//         std::ostringstream oss;
//         multiSlsDetectorClient("activate", GET, nullptr, oss);
//         REQUIRE(oss.str() == "activate 0 padding\n");
//     }
//     {
//         std::ostringstream oss;
//         multiSlsDetectorClient("activate 1", PUT, nullptr, oss);
//         REQUIRE(oss.str() == "activate 1 padding\n");
//     }
// }

TEST_CASE("fmaster", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fmaster 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fmaster", GET, nullptr, oss);
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fmaster 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fmaster 1\n");
    }
}

TEST_CASE("findex", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("findex 57", PUT, nullptr, oss);
        REQUIRE(oss.str() == "findex 57\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("findex", GET, nullptr, oss);
        REQUIRE(oss.str() == "findex 57\n");
    }
    {

        std::ostringstream oss;
        multiSlsDetectorClient("findex 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "findex 0\n");
    }
}

TEST_CASE("rx_tcpport", "[.cmd]") {
    multiSlsDetector d;
    int port = 3500;
    multiSlsDetectorClient("rx_tcpport " + std::to_string(port), PUT);
    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        multiSlsDetectorClient(std::to_string(i) + ":rx_tcpport", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');   
    }
    REQUIRE_THROWS(multiSlsDetectorClient("rx_tcpport 15", PUT));
    port = 1954;
    multiSlsDetectorClient("rx_tcpport " + std::to_string(port), PUT);
    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        multiSlsDetectorClient(std::to_string(i) + ":rx_tcpport", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');   
    }
}

TEST_CASE("fname", "[.cmd]") {

    {
        std::ostringstream oss;
        multiSlsDetectorClient("fname somename", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fname", GET, nullptr, oss);
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("fname run", PUT, nullptr, oss);
        REQUIRE(oss.str() == "fname run\n");
    }
}

TEST_CASE("rx_framescaught", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("resetframescaught 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "resetframescaught successful\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_framescaught", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_framescaught 0\n");
    }
    REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
    REQUIRE_NOTHROW(multiSlsDetectorClient("exptime 1", PUT));    
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_start", PUT));
    REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
    sleep(2);
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_stop", PUT)); 
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_framescaught", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_framescaught 1\n");
    }      
}

TEST_CASE("rx_silent", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_silent 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_silent 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_silent", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_silent 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_silent 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_silent 0\n");
    }
}

// TEST_CASE("rx_jsonaddheader", "[.cmd]") {
//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_jsonaddheader \"hej\":\"5\"", PUT, nullptr,
//     oss); REQUIRE(oss.str() == "rx_jsonaddheader \"hej\":\"5\"\n");

//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_jsonaddheader", GET, nullptr, oss);
//     REQUIRE(oss.str() == "rx_jsonaddheader \"hej\":\"5\"\n");

//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_jsonaddheader \"\"", PUT, nullptr, oss);
//     REQUIRE(oss.str() == "rx_jsonaddheader\n");
// }

// TEST_CASE("rx_udpsocksize", "[.cmd]") {
//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_udpsocksize 4857600", PUT, nullptr, oss);
//     REQUIRE(oss.str() == "rx_udpsocksize 4857600\n");

//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_udpsocksize", GET, nullptr, oss);
//     REQUIRE(oss.str() == "rx_udpsocksize 4857600\n");

//     std::ostringstream oss;
//     multiSlsDetectorClient("rx_udpsocksize 104857600", PUT, nullptr, oss);
//     REQUIRE(oss.str() == "rx_udpsocksize 104857600\n");
// }

TEST_CASE("rx_framesperfile", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_framesperfile 50", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_framesperfile 50\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_framesperfile", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_framesperfile 50\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_framesperfile 10000", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_framesperfile 10000\n");
    }
}

TEST_CASE("rx_discardpolicy", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_discardpolicy discardempty", PUT, nullptr,
                               oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardempty\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_discardpolicy", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardempty\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_discardpolicy discardpartial", PUT, nullptr,
                               oss);
        REQUIRE(oss.str() == "rx_discardpolicy discardpartial\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_discardpolicy nodiscard", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_discardpolicy nodiscard\n");
    }
}

TEST_CASE("rx_padding", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_padding 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_padding 0\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_padding", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_padding 0\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_padding 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_padding 1\n");
    }
}

TEST_CASE("rx_readfreq", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_readfreq 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_readfreq 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_readfreq", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_readfreq 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_readfreq 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_readfreq 0\n");
    }
}

TEST_CASE("rx_lock", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_lock 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_lock 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_lock", GET, nullptr, oss);
        REQUIRE(oss.str() == "rx_lock 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("rx_lock 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "rx_lock 0\n");
    }
}

TEST_CASE("lock", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("lock 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("lock", GET, nullptr, oss);
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("lock 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "lock 0\n");
    }
}


TEST_CASE("rx_lastclient", "[.cmd]") {

    std::ostringstream oss;
    multiSlsDetectorClient("rx_lastclient", GET, nullptr, oss);
    REQUIRE(oss.str() == "rx_lastclient " + test::my_ip + "\n");
}



TEST_CASE("rx_checkversion", "[.cmd]") {

    std::ostringstream oss;
    multiSlsDetectorClient("rx_checkversion", GET, nullptr, oss);
    REQUIRE(oss.str() == "rx_checkversion compatible\n");
}

TEST_CASE("exptime", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("exptime 0.05", PUT, nullptr, oss);
        REQUIRE(oss.str() == "exptime 0.05\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("exptime", GET, nullptr, oss);
        REQUIRE(oss.str() == "exptime 50ms\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("exptime 1", PUT, nullptr, oss);
        REQUIRE(oss.str() == "exptime 1\n");
    }
}


TEST_CASE("period", "[.cmd]") {
    {
        std::ostringstream oss;
        multiSlsDetectorClient("period 1.25s", PUT, nullptr, oss);
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("period", GET, nullptr, oss);
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        multiSlsDetectorClient("period 0", PUT, nullptr, oss);
        REQUIRE(oss.str() == "period 0\n");
    }
}

TEST_CASE("delay", "[.cmd][.eiger]") {
    if(test::type == slsDetectorDefs::EIGER) {
        REQUIRE_THROWS(multiSlsDetectorClient("delay 1", PUT));
        REQUIRE_THROWS(multiSlsDetectorClient("delay", GET));       
    } else {
        {
            std::ostringstream oss;
            multiSlsDetectorClient("delay 1.25s", PUT, nullptr, oss);
            REQUIRE(oss.str() == "delay 1.25s\n");
        }
        {
            std::ostringstream oss;
            multiSlsDetectorClient("delay", GET, nullptr, oss);
            REQUIRE(oss.str() == "delay 1.25s\n");
        }
        {
            std::ostringstream oss;
            multiSlsDetectorClient("delay 0", PUT, nullptr, oss);
            REQUIRE(oss.str() == "delay 0\n");
        }
    }
}