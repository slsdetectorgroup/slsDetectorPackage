#include "catch.hpp"
#include "multiSlsDetector.h"
#include "multiSlsDetectorClient.h"
#include "sls_detector_defs.h"
#include <sstream>

#include "tests/globals.h"

auto GET = slsDetectorDefs::GET_ACTION;
auto PUT = slsDetectorDefs::PUT_ACTION;

TEST_CASE("detectormode", "[.cmd][.moench]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("detectormode counting", PUT, nullptr, oss));
        REQUIRE(oss.str() == "detectormode counting\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("detectormode interpolating", PUT, nullptr, oss));
        REQUIRE(oss.str() == "detectormode interpolating\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("detectormode analog", PUT, nullptr, oss));
        REQUIRE(oss.str() == "detectormode analog\n");
    }         
    REQUIRE_NOTHROW(multiSlsDetectorClient("detectormode counting", PUT));
    REQUIRE_THROWS(multiSlsDetectorClient("detectormode pedestal", PUT));
}

TEST_CASE("framemode", "[.cmd][.moench]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("framemode pedestal", PUT, nullptr, oss));
        REQUIRE(oss.str() == "framemode pedestal\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("framemode newpedestal", PUT, nullptr, oss));
        REQUIRE(oss.str() == "framemode newpedestal\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("framemode flatfield", PUT, nullptr, oss));
        REQUIRE(oss.str() == "framemode flatfield\n");
    }   
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("framemode newflatfield", PUT, nullptr, oss));
        REQUIRE(oss.str() == "framemode newflatfield\n");
    }        
    REQUIRE_NOTHROW(multiSlsDetectorClient("framemode pedestal", PUT));
    REQUIRE_THROWS(multiSlsDetectorClient("framemode counting", PUT));
}

TEST_CASE("emin", "[.cmd][.moench]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("emin 100", PUT, nullptr, oss));
        REQUIRE(oss.str() == "emin 100\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("emax 200", PUT, nullptr, oss));
        REQUIRE(oss.str() == "emax 200\n");
    }   
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonpara emax", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_jsonpara 200\n");
    }
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"\"", PUT));
}


TEST_CASE("rx_jsonpara", "[.cmd][.moench]") {
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"key1\":\"value1\"", PUT)); 
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonpara key1", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_jsonpara value1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonpara key1 value2", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_jsonpara [key1, value2]\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonpara key2 98", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_jsonpara [key2, 98]\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_jsonaddheader \"key1\":\"value2\",\"key2\":98\n");
    }        
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"\"", PUT));
}

TEST_CASE("rx_jsonaddheader", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"\"", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_jsonaddheader \"\"\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"what\":\"bla\"", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_jsonaddheader \"what\":\"bla\"\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"what2\":\"bla2\"", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_jsonaddheader \"what2\":\"bla2\"\n");
    }        
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"\"", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_jsonaddheader \"\"\n");
    }
}


TEST_CASE("patsetbit", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
        uint64_t val = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patsetbit", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("patsetbit "));
            val = stoul(s, 0, 16);        
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patsetbit 0x842f020204200dc0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "patsetbit 0x842f020204200dc0\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("patsetbit " + sls::ToStringHex(val), PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("patsetbit", GET));
    }
}


TEST_CASE("patmask", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
        uint64_t val = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patmask", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("patmask "));
            val = stoul(s, 0, 16);        
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patmask 0x842f020204200dc0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "patmask 0x842f020204200dc0\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("patmask " + sls::ToStringHex(val), PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("patmask", GET));
    }
}


TEST_CASE("patwaittime", "[.cmd][.ctb]") {
    for (int loop = 0; loop < 3; ++loop) {
        if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
            uint64_t val = 0;   
            {
                std::ostringstream oss;
                REQUIRE_NOTHROW(multiSlsDetectorClient("patwaittime" + std::to_string(loop), GET, nullptr, oss));
                std::string s = (oss.str()).erase (0, strlen("patwaittime") + 2);
                val = std::stoul(s);       
            }
            {
                std::ostringstream oss;
                REQUIRE_NOTHROW(multiSlsDetectorClient("patwaittime" + std::to_string(loop) + " 8589936640", PUT, nullptr, oss));
                REQUIRE(oss.str() == "patwaittime" + std::to_string(loop) + " 8589936640\n");
            }
            REQUIRE_NOTHROW(multiSlsDetectorClient("patwaittime" + std::to_string(loop) + ' ' + std::to_string(val), PUT));
        } else {
            REQUIRE_THROWS(multiSlsDetectorClient("patwaittime" + std::to_string(loop), GET));
        }
    }
}


TEST_CASE("patwait", "[.cmd][.ctb]") {
    for (int loop = 0; loop < 3; ++loop) {
        if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
            int val = 0;   
            {
                std::ostringstream oss;
                REQUIRE_NOTHROW(multiSlsDetectorClient("patwait" + std::to_string(loop), GET, nullptr, oss));
                std::string s = (oss.str()).erase (0, strlen("patwait") + 2);
                val = stoul(s, 0, 16);       
            }
            {
                std::ostringstream oss;
                REQUIRE_NOTHROW(multiSlsDetectorClient("patwait" + std::to_string(loop) + " 0x5c", PUT, nullptr, oss));
                REQUIRE(oss.str() == "patwait" + std::to_string(loop) + " 0x5c\n");
            }
            REQUIRE_NOTHROW(multiSlsDetectorClient("patwait" + std::to_string(loop) + ' ' + sls::ToStringHex(val), PUT));
        } else {
            REQUIRE_THROWS(multiSlsDetectorClient("patwait" + std::to_string(loop), GET));
        }
    }
}

TEST_CASE("patnloop", "[.cmd][.ctb]") {
    for (int loop = 0; loop < 3; ++loop) {
        if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
            int val = 0;   
            {
                std::ostringstream oss;
                REQUIRE_NOTHROW(multiSlsDetectorClient("patnloop" + std::to_string(loop), GET, nullptr, oss));
                std::string s = (oss.str()).erase (0, strlen("patnloop") + 2);
                val = std::stoi(s);           
            }
            {
                std::ostringstream oss;
                REQUIRE_NOTHROW(multiSlsDetectorClient("patnloop" + std::to_string(loop) + " 5", PUT, nullptr, oss));
                REQUIRE(oss.str() == "patnloop" + std::to_string(loop) + " 5\n");
            }
            REQUIRE_NOTHROW(multiSlsDetectorClient("patnloop" + std::to_string(loop) + ' ' + std::to_string(val), PUT));
        } else {
            REQUIRE_THROWS(multiSlsDetectorClient("patnloop" + std::to_string(loop), GET));
        }
    }
}

TEST_CASE("patloop", "[.cmd][.ctb]") {
    for (int loop = 0; loop < 3; ++loop) {
        if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
            uint32_t limit1 = 0, limit2 = 0;   
            {
                std::ostringstream oss;
                REQUIRE_NOTHROW(multiSlsDetectorClient("patloop" + std::to_string(loop), GET, nullptr, oss));
                std::string s = oss.str();
                auto t = sls::split(s, ' ');
                s = t[1].erase (0, 1);
                limit1 = stoul(s, 0, 16);
                limit2 = stoul(t[2], 0, 16);            
            }
            {
                std::ostringstream oss;
                REQUIRE_NOTHROW(multiSlsDetectorClient("patloop" + std::to_string(loop) + " 0x20 0x5c", PUT, nullptr, oss));
                REQUIRE(oss.str() == "patloop" + std::to_string(loop) + " [0x20, 0x5c]\n");
            }
            REQUIRE_NOTHROW(multiSlsDetectorClient("patloop" + std::to_string(loop) + ' ' + sls::ToStringHex(limit1) + ' ' + sls::ToStringHex(limit2), PUT));     
            REQUIRE_THROWS(multiSlsDetectorClient("patloop" + std::to_string(loop) + " 0x3", PUT));
        } else {
            REQUIRE_THROWS(multiSlsDetectorClient("patloop" + std::to_string(loop), GET));
        }
    }
}

TEST_CASE("patlimits", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
        uint32_t patlimit1 = 0, patlimit2 = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patlimits", GET, nullptr, oss));
            std::string s = oss.str();
            auto t = sls::split(s, ' ');
            s = t[1].erase (0, 1);
            patlimit1 = stoul(s, 0, 16);
            patlimit2 = stoul(t[2], 0, 16);            
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patlimits 0x20 0x5c", PUT, nullptr, oss));
            REQUIRE(oss.str() == "patlimits [0x20, 0x5c]\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("patlimits " + sls::ToStringHex(patlimit1) + ' ' + sls::ToStringHex(patlimit2), PUT));     
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("patlimits", GET));
    }
}


TEST_CASE("patword", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
        uint64_t prev_value = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patword 0x23", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("patword "));
            prev_value = stoul(s, 0, 16);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patword 0x23 0xc15004808d0a21a4", PUT, nullptr, oss));
            REQUIRE(oss.str() == "patword [0x23, 0xc15004808d0a21a4]\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patword 0x23 0x0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "patword [0x23, 0x0]\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patword 0x23", GET, nullptr, oss));
            REQUIRE(oss.str() == "patword 0x0\n");
        }      
        REQUIRE_NOTHROW(multiSlsDetectorClient("patword 0x23 " + std::to_string(prev_value), PUT));       
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("patword 0x23", GET));
    }
}

TEST_CASE("patclkctrl", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
        uint64_t prev_value = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patclkctrl", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("patclkctrl "));
            prev_value = stoul(s, 0, 16);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patclkctrl 0xc15004808d0a21a4", PUT, nullptr, oss));
            REQUIRE(oss.str() == "patclkctrl 0xc15004808d0a21a4\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patclkctrl 0x0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "patclkctrl 0x0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patclkctrl", GET, nullptr, oss));
            REQUIRE(oss.str() == "patclkctrl 0x0\n");
        }      
        REQUIRE_NOTHROW(multiSlsDetectorClient("patclkctrl " + std::to_string(prev_value), PUT));  
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("patclkctrl", GET));
    }
}


TEST_CASE("patioctrl", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
        uint64_t prev_value = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patioctrl", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("patioctrl "));
            prev_value = stoul(s, 0, 16);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patioctrl 0xc15004808d0a21a4", PUT, nullptr, oss));
            REQUIRE(oss.str() == "patioctrl 0xc15004808d0a21a4\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patioctrl 0x0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "patioctrl 0x0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("patioctrl", GET, nullptr, oss));
            REQUIRE(oss.str() == "patioctrl 0x0\n");
        }      
        REQUIRE_NOTHROW(multiSlsDetectorClient("patioctrl " + std::to_string(prev_value), PUT));  
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("patioctrl", GET));
    }
}


TEST_CASE("savepattern", "[.cmd][.ctb]") {
    REQUIRE_THROWS(multiSlsDetectorClient("savepattern", GET));
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("savepattern /tmp/pat.txt", PUT, nullptr, oss));
            REQUIRE(oss.str() == "savepattern /tmp/pat.txt\n");
        }
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("savepattern /tmp/pat.txt", PUT));
    }
}

TEST_CASE("pattern", "[.cmd][.ctb]") {
    REQUIRE_THROWS(multiSlsDetectorClient("pattern", GET));
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
        ;// todo test with real file?
    }
}

TEST_CASE("led", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("led 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "led 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("led 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "led 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("led", GET, nullptr, oss));
            REQUIRE(oss.str() == "led 0\n");
        }      
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("led", GET));
    }
}


TEST_CASE("diodelay", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        REQUIRE_NOTHROW(multiSlsDetectorClient("diodelay 0x01010 125", PUT)); 
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("diodelay 0x01010 775", PUT, nullptr, oss));
            REQUIRE(oss.str() == "diodelay [0x01010, 775]\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("diodelay 0x01010 0", PUT));
        REQUIRE_THROWS(multiSlsDetectorClient("diodelay [0x01010, 776]", PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("diodelay", GET));
    }
}


TEST_CASE("rx_dbitoffset", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "rx_dbitoffset 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "rx_dbitoffset 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset 15", PUT, nullptr, oss));
            REQUIRE(oss.str() == "rx_dbitoffset 15\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset", GET, nullptr, oss));
            REQUIRE(oss.str() == "rx_dbitoffset 15\n");
        } 
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset 0", PUT));   
    } else {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitoffset", GET, nullptr, oss));
            REQUIRE(oss.str() == "rx_dbitoffset 0\n");
    }
}


TEST_CASE("rx_dbitlist", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitlist 0 4 5 8 9 10 52 63", PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_dbitlist", GET, nullptr, oss));
            REQUIRE(oss.str() == "rx_dbitlist [0, 4, 5, 8, 9, 10, 52, 63]\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitlist all", PUT));
    } else {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_dbitlist", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_dbitlist []\n");
    }
}

TEST_CASE("extsampling", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("extsampling 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "extsampling 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("extsampling", GET, nullptr, oss));
            REQUIRE(oss.str() == "extsampling 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("extsampling 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "extsampling 0\n");
        }
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("extsampling", GET));
    }
}

TEST_CASE("extsamplingsrc", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("extsamplingsrc 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "extsamplingsrc 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("extsamplingsrc 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "extsamplingsrc 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("extsamplingsrc 15", PUT, nullptr, oss));
            REQUIRE(oss.str() == "extsamplingsrc 15\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("extsamplingsrc", GET, nullptr, oss));
            REQUIRE(oss.str() == "extsamplingsrc 15\n");
        }   
    REQUIRE_THROWS(multiSlsDetectorClient("extsamplingsrc 64", PUT));    
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("extsamplingsrc", GET));
    }
}


TEST_CASE("adcinvert", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        std::string s;
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcinvert", GET, nullptr, oss));
            s = oss.str();
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcinvert 0x8d0a21d4", PUT, nullptr, oss));
            REQUIRE(oss.str() == "adcinvert 0x8d0a21d4\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
            REQUIRE(oss.str() == s);
        }
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("adcinvert", GET));
    }
}


TEST_CASE("adcenable", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        std::string s;
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcenable", GET, nullptr, oss));
            s = oss.str();
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcenable 0x8d0a21d4", PUT, nullptr, oss));
            REQUIRE(oss.str() == "adcenable 0x8d0a21d4\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
            REQUIRE(oss.str() == s);
        }
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("adcenable", GET));
    }
}


TEST_CASE("vm_a", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        REQUIRE_NOTHROW(multiSlsDetectorClient("vm_a", GET));
        REQUIRE_NOTHROW(multiSlsDetectorClient("vm_b", GET));
        REQUIRE_NOTHROW(multiSlsDetectorClient("vm_c", GET));
        REQUIRE_NOTHROW(multiSlsDetectorClient("vm_d", GET));
        REQUIRE_NOTHROW(multiSlsDetectorClient("vm_io", GET));
        REQUIRE_NOTHROW(multiSlsDetectorClient("im_a", GET));
        REQUIRE_NOTHROW(multiSlsDetectorClient("im_b", GET));
        REQUIRE_NOTHROW(multiSlsDetectorClient("im_c", GET));
        REQUIRE_NOTHROW(multiSlsDetectorClient("im_d", GET));
        REQUIRE_NOTHROW(multiSlsDetectorClient("im_io", GET));        
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("vm_a", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("vm_b", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("vm_c", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("vm_d", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("vm_io", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("im_a", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("im_b", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("im_c", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("im_d", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("im_io", GET));        
    }
}

TEST_CASE("v_a", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        std::string s;
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_limit", GET, nullptr, oss));
            s = oss.str();
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("v_limit 1500", PUT, nullptr, oss));
            REQUIRE(oss.str() == "v_limit 1500\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("v_limit", GET, nullptr, oss));
            REQUIRE(oss.str() == "v_limit 1500\n");
        }     
        REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_a", GET, nullptr, oss));
            s = oss.str();
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
            REQUIRE(oss.str() == s);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_b", GET, nullptr, oss));
            s = oss.str();
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
            REQUIRE(oss.str() == s);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_c", GET, nullptr, oss));
            s = oss.str();
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
            REQUIRE(oss.str() == s);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_d", GET, nullptr, oss));
            s = oss.str();
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
            REQUIRE(oss.str() == s);
        } 
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_io", GET));
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:v_chip", GET)); // do not set vchip
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("v_limit", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("v_a", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("v_b", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("v_c", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("v_d", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("v_io", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("v_chip", GET));
    }
}

TEST_CASE("adcvpp", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        int prev_val = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("adcvpp "));
            prev_val = std::stoi(s);
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp 1", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp", GET, nullptr, oss));
            REQUIRE(oss.str() == "adcvpp 1\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp 1140 mv", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp mv", GET, nullptr, oss));
            REQUIRE(oss.str() == "adcvpp 1140 mv\n");
        }            
        REQUIRE_NOTHROW(multiSlsDetectorClient("adcvpp " + std::to_string(prev_val), PUT));  
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("adcvpp", GET));
    }
}


TEST_CASE("dbitpipeline", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitpipeline 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "dbitpipeline 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitpipeline 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "dbitpipeline 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitpipeline 15", PUT, nullptr, oss));
            REQUIRE(oss.str() == "dbitpipeline 15\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitpipeline", GET, nullptr, oss));
            REQUIRE(oss.str() == "dbitpipeline 15\n");
        }   
    REQUIRE_NOTHROW(multiSlsDetectorClient("dbitpipeline 0", PUT));    
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("dbitpipeline", GET));
    }
}

TEST_CASE("adcpipeline", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcpipeline 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "adcpipeline 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcpipeline 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "adcpipeline 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcpipeline 15", PUT, nullptr, oss));
            REQUIRE(oss.str() == "adcpipeline 15\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcpipeline", GET, nullptr, oss));
            REQUIRE(oss.str() == "adcpipeline 15\n");
        }   
    REQUIRE_NOTHROW(multiSlsDetectorClient("adcpipeline 0", PUT));      
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("adcpipeline", GET));
    }
}

TEST_CASE("maxdbitphaseshift", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD ) {
        REQUIRE_NOTHROW(multiSlsDetectorClient("maxdbitphaseshift", GET));        
    } else { 
        REQUIRE_THROWS(multiSlsDetectorClient("maxdbitphaseshift", GET));        
            
    } 
}

TEST_CASE("dbitphase", "[.cmd][.ctb]") {
    if (test::type != slsDetectorDefs::CHIPTESTBOARD) {
       REQUIRE_THROWS(multiSlsDetectorClient("dbitphase", GET));       
    } else { 
        int prev_val = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("dbitphase "));
            prev_val = std::stoi(s);
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase 20", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase", GET, nullptr, oss));
            REQUIRE(oss.str() == "dbitphase 20\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase", GET, nullptr, oss));
            REQUIRE(oss.str() == "dbitphase 0\n");
        }     
        if (test::type == slsDetectorDefs::GOTTHARD) {
            REQUIRE_THROWS(multiSlsDetectorClient("dbitphase deg", GET));
        } else {
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase 20 deg", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase deg", GET, nullptr, oss));
            REQUIRE(oss.str() == "dbitphase 20 deg\n");
        }          
        REQUIRE_NOTHROW(multiSlsDetectorClient("dbitphase " + std::to_string(prev_val), PUT));                    
    } 
}

TEST_CASE("romode", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) { 
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("romode digital", PUT, nullptr, oss));
            REQUIRE(oss.str() == "romode digital\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("romode analog_digital", PUT, nullptr, oss));
            REQUIRE(oss.str() == "romode analog_digital\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("romode analog", PUT, nullptr, oss));
            REQUIRE(oss.str() == "romode analog\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("romode", GET, nullptr, oss));
            REQUIRE(oss.str() == "romode analog\n");
        } 
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("romode", GET));     
    }
}


TEST_CASE("samples", "[.cmd][.ctb]") {
    if (test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        uint64_t prev_value1 = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("asamples", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("asamples "));
            prev_value1 = std::stoi(s);
        }     
        std::cout<<"asamples:"<<prev_value1<<std::endl;
        uint64_t prev_value2 = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dsamples", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("dsamples "));
            prev_value2 = std::stoi(s);
        }           
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("samples 1200", PUT, nullptr, oss));
            REQUIRE(oss.str() == "samples 1200\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("samples 1000", PUT, nullptr, oss));
            REQUIRE(oss.str() == "samples 1000\n");
        }   
         {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("asamples 2200", PUT, nullptr, oss));
            REQUIRE(oss.str() == "asamples 2200\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("asamples 4000", PUT, nullptr, oss));
            REQUIRE(oss.str() == "asamples 4000\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dsamples 1200", PUT, nullptr, oss));
            REQUIRE(oss.str() == "dsamples 1200\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dsamples 1000", PUT, nullptr, oss));
            REQUIRE(oss.str() == "dsamples 1000\n");
        }    
        REQUIRE_THROWS(multiSlsDetectorClient("samples", GET));   // different values       
        REQUIRE_NOTHROW(multiSlsDetectorClient("asamples " + std::to_string(prev_value1), PUT));    
        REQUIRE_NOTHROW(multiSlsDetectorClient("dsamples " + std::to_string(prev_value2), PUT));    
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("samples", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("asamples", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("dsamples", GET));        
    }
}

TEST_CASE("imagetest", "[.cmd][.gotthard]") {
    if (test::type == slsDetectorDefs::GOTTHARD) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("imagetest 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "imagetest 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("imagetest 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "imagetest 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("imagetest", GET, nullptr, oss));
            REQUIRE(oss.str() == "imagetest 0\n");
        }      
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("imagetest", GET));
    }
}

TEST_CASE("extsig", "[.cmd][.gotthard]") {
    if (test::type == slsDetectorDefs::GOTTHARD) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("extsig trigger_in_falling_edge", PUT, nullptr, oss));
            REQUIRE(oss.str() == "extsig trigger_in_falling_edge\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("extsig trigger_in_rising_edge", PUT, nullptr, oss));
            REQUIRE(oss.str() == "extsig trigger_in_rising_edge\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("extsig", GET, nullptr, oss));
            REQUIRE(oss.str() == "extsig trigger_in_rising_edge\n");
        }
        REQUIRE_THROWS(multiSlsDetectorClient("extsig gating", PUT));         
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("extsig", GET));
    }
}


TEST_CASE("exptimel", "[.cmd][.gotthard]") {
    if (test::type == slsDetectorDefs::GOTTHARD) {   
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("exptime 5 s", PUT));        
        REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:exptimel s", GET, nullptr, oss));
            std::string st = oss.str();
            std::string s = st.erase (0, strlen("exptimel "));
            double val = std::stod(s);
            REQUIRE(val >= 0);
            REQUIRE(val < 1000);
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));      
        REQUIRE_NOTHROW(multiSlsDetectorClient("exptime 1 ms", PUT));        
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("exptimel", GET));
    }
}


TEST_CASE("periodl", "[.cmd][.gotthard]") {
    if (test::type == slsDetectorDefs::GOTTHARD || test::type == slsDetectorDefs::JUNGFRAU || test::type == slsDetectorDefs::CHIPTESTBOARD) {   
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 2", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("period 5", PUT));        
        REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:periodl s", GET, nullptr, oss));
            std::string st = oss.str();
            std::string s = st.erase (0, strlen("periodl "));
            double val = std::stod(s);
            REQUIRE(val >= 0);
            REQUIRE(val < 1000);
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));      
        REQUIRE_NOTHROW(multiSlsDetectorClient("period 1 s", PUT));     
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));   
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("periodl", GET));
    }
}

TEST_CASE("roi", "[.cmd][.gotthard]") {
    if (test::type == slsDetectorDefs::GOTTHARD) {  
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("roi 0 255", PUT, nullptr, oss));
            REQUIRE(oss.str() == "roi [0, 255] \n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("roi 256 511", PUT, nullptr, oss));
            REQUIRE(oss.str() == "roi [256, 511] \n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("clearroi", PUT, nullptr, oss));
            REQUIRE(oss.str() == "clearroi [-1, -1] \n");
        }
    REQUIRE_THROWS(multiSlsDetectorClient("roi 0 256", PUT));       
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("roi", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("clearroi", PUT));
    }
}

TEST_CASE("storagecell_delay", "[.cmd][.jungfrau]") {
    if (test::type == slsDetectorDefs::JUNGFRAU) {  
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_delay 1.62ms", PUT, nullptr, oss));
            REQUIRE(oss.str() == "storagecell_delay 1.62ms\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_delay", GET, nullptr, oss));
            REQUIRE(oss.str() == "storagecell_delay 1.62ms\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_delay 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "storagecell_delay 0\n");
        }
    REQUIRE_THROWS(multiSlsDetectorClient("storagecell_delay 1638376ns", PUT));       
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("storagecell_delay", GET));
    }
}

TEST_CASE("storagecell_start", "[.cmd][.jungfrau]") {
    if (test::type == slsDetectorDefs::JUNGFRAU) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_start 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "storagecell_start 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_start 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "storagecell_start 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_start 15", PUT, nullptr, oss));
            REQUIRE(oss.str() == "storagecell_start 15\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecell_start", GET, nullptr, oss));
            REQUIRE(oss.str() == "storagecell_start 15\n");
        }   
    REQUIRE_THROWS(multiSlsDetectorClient("storagecell_start 16", PUT));    
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("storagecell_start", GET));
    }
}

TEST_CASE("storagecells", "[.cmd][.jungfrau]") {
    if (test::type == slsDetectorDefs::JUNGFRAU) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecells 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "storagecells 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecells 15", PUT, nullptr, oss));
            REQUIRE(oss.str() == "storagecells 15\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecells 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "storagecells 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storagecells", GET, nullptr, oss));
            REQUIRE(oss.str() == "storagecells 0\n");
        }   
    REQUIRE_THROWS(multiSlsDetectorClient("storagecells 16", PUT));    
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("storagecells", GET));
    }
}


TEST_CASE("auto_comp_disable", "[.cmd][.jungfrau]") {
    if (test::type == slsDetectorDefs::JUNGFRAU) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("auto_comp_disable 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "auto_comp_disable 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("auto_comp_disable 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "auto_comp_disable 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("auto_comp_disable", GET, nullptr, oss));
            REQUIRE(oss.str() == "auto_comp_disable 0\n");
        }   
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("auto_comp_disable", GET));
    }
}

TEST_CASE("powerchip", "[.cmd][.jungfrau]") {
    if (test::type == slsDetectorDefs::JUNGFRAU) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("powerchip 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "powerchip 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("powerchip 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "powerchip 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("powerchip", GET, nullptr, oss));
            REQUIRE(oss.str() == "powerchip 0\n");
        }   
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("powerchip", GET));
    }
}

TEST_CASE("temp_", "[.cmd][.jungfrau]") {
    if (test::type == slsDetectorDefs::JUNGFRAU) {   
        std::string s;
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_threshold", GET, nullptr, oss));
            s = oss.str();
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT, nullptr, oss));
            REQUIRE(oss.str() == s);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("temp_control 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "temp_control 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("temp_control 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "temp_control 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("temp_control", GET, nullptr, oss));
            REQUIRE(oss.str() == "temp_control 0\n");
        }       
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("temp_event", GET, nullptr, oss));
            REQUIRE(oss.str() == "temp_event 0\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("temp_event 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "temp_event cleared\n");
        }
        REQUIRE_THROWS(multiSlsDetectorClient("temp_event 1", PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("temp_threshold", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("temp_control", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("temp_event", GET));       
    }
}

TEST_CASE("quad", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("quad", GET, nullptr, oss));
            REQUIRE(oss.str() == "quad 0\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("quad 0", PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("quad", GET));
    }
}




TEST_CASE("pulse", "[.cmd][.eiger]") {
    REQUIRE_THROWS(multiSlsDetectorClient("pulse", GET));
    REQUIRE_THROWS(multiSlsDetectorClient("pulsenmove", GET));       
    REQUIRE_THROWS(multiSlsDetectorClient("pulsechip", GET));      
    if (test::type == slsDetectorDefs::EIGER) { 
        REQUIRE_NOTHROW(multiSlsDetectorClient("pulse 1 1 5", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("pulsenmove 1 1 5", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("pulsechip 1", PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("pulse 1 1 5", PUT));
        REQUIRE_THROWS(multiSlsDetectorClient("pulsenmove 1 1 5", PUT));       
        REQUIRE_THROWS(multiSlsDetectorClient("pulsechip 1", PUT));          
    }
}


TEST_CASE("partialreset", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("partialreset 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "partialreset 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("partialreset", GET, nullptr, oss));
            REQUIRE(oss.str() == "partialreset 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("partialreset 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "partialreset 0\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("partialreset 1", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("partialreset 0", PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("partialreset", GET));
    }
}


TEST_CASE("activate", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "activate 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate 1 nopadding", PUT, nullptr, oss));
            REQUIRE(oss.str() == "activate 1 nopadding\n");
        }        
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate 0 padding", PUT, nullptr, oss));
            REQUIRE(oss.str() == "activate 0 padding\n");
        } 
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate 0 nopadding", PUT, nullptr, oss));
            REQUIRE(oss.str() == "activate 0 nopadding\n");
        } 
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate 1 padding", PUT, nullptr, oss));
            REQUIRE(oss.str() == "activate 1 padding\n");
        }     
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:activate", GET, nullptr, oss));
            REQUIRE(oss.str() == "activate 1 padding\n");
        }               
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("activate", GET));
    }
}

TEST_CASE("measuredsubperiod", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("dr 32", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
        sleep(3);
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:measuredsubperiod ms", GET, nullptr, oss));
            std::string st = oss.str();
            std::string s = st.erase (0, strlen("measuredsubperiod "));
            double val = std::stod(s);
            REQUIRE(val >= 0);
            REQUIRE(val < 1000);
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("dr 16", PUT)); 
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("measuredsubperiod", GET));
    }
}

TEST_CASE("measuredperiod", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 2", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("period 1", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
        sleep(3);
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:measuredperiod", GET, nullptr, oss));
            std::string st = oss.str();
            std::string s = st.erase (0, strlen("measuredperiod "));
            double val = std::stod(s);
            REQUIRE(val >= 1.0);
            REQUIRE(val < 2.0);
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("measuredperiod", GET));
    }
}

TEST_CASE("interruptsubframe", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("interruptsubframe 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "interruptsubframe 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("interruptsubframe", GET, nullptr, oss));
            REQUIRE(oss.str() == "interruptsubframe 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("interruptsubframe 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "interruptsubframe 0\n");
        }
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("interruptsubframe", GET));
    }
}

TEST_CASE("readnlines", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("readnlines 256", PUT, nullptr, oss));
            REQUIRE(oss.str() == "readnlines 256\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("readnlines", GET, nullptr, oss));
            REQUIRE(oss.str() == "readnlines 256\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("readnlines 16", PUT, nullptr, oss));
            REQUIRE(oss.str() == "readnlines 16\n");
        }
        REQUIRE_THROWS(multiSlsDetectorClient("readnlines 0", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("readnlines 256", PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("readnlines", GET));
    }
}


TEST_CASE("ratecorr", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("ratecorr 120", PUT, nullptr, oss));
            REQUIRE(oss.str() == "ratecorr 120ns\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("ratecorr", GET, nullptr, oss));
            REQUIRE(oss.str() == "ratecorr 120ns\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("ratecorr 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "ratecorr 0ns\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("ratecorr -1", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("ratecorr 0", PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("ratecorr", GET));
    }
}


TEST_CASE("trimen", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        // require globals for trimen?
        REQUIRE_NOTHROW(multiSlsDetectorClient("trimen 4500 5400 6400", PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:trimen", GET, nullptr, oss));
            REQUIRE(oss.str() == "trimen [4500, 5400, 6400]\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("threshold 4500 standard", PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("trimen", GET));
    }
}

TEST_CASE("trimval", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("trimval 63", PUT, nullptr, oss));
            REQUIRE(oss.str() == "trimval 63\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("trimval", GET, nullptr, oss));
            REQUIRE(oss.str() == "trimval 63\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("trimval 31", PUT, nullptr, oss));
            REQUIRE(oss.str() == "trimval 31\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("trimval 0", PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("trimval", GET));
    }
}



TEST_CASE("flippeddatax", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:flippeddatax", GET, nullptr, oss));
            REQUIRE(oss.str() == "flippeddatax 0\n");
        }
        multiSlsDetector d;
        if (d.size() > 1) {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("1:flippeddatax", GET, nullptr, oss));
            REQUIRE(oss.str() == "flippeddatax 1\n");
        }
    } else {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("flippeddatax", GET, nullptr, oss));
        REQUIRE(oss.str() == "flippeddatax 0\n");
    }
}


TEST_CASE("storeinram", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storeinram 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "storeinram 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storeinram", GET, nullptr, oss));
            REQUIRE(oss.str() == "storeinram 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("storeinram 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "storeinram 0\n");
        }
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("storeinram", GET));
    }
}


TEST_CASE("overflow", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("overflow 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "overflow 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("overflow", GET, nullptr, oss));
            REQUIRE(oss.str() == "overflow 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("overflow 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "overflow 0\n");
        }
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("overflow", GET));
    }
}

TEST_CASE("parallel", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("parallel 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "parallel 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("parallel", GET, nullptr, oss));
            REQUIRE(oss.str() == "parallel 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("parallel 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "parallel 0\n");
        }
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("parallel", GET));
    }
}


TEST_CASE("gappixels", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("gappixels 1", PUT, nullptr, oss));
            REQUIRE(oss.str() == "gappixels 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("gappixels", GET, nullptr, oss));
            REQUIRE(oss.str() == "gappixels 1\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("gappixels 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "gappixels 0\n");
        }
    } else {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("gappixels", GET, nullptr, oss));
        REQUIRE(oss.str() == "gappixels 0\n");
        REQUIRE_THROWS(multiSlsDetectorClient("gappixels 1", PUT));
    }
}

TEST_CASE("settingspath", "[.cmd][.eiger]") {
    std::string s;
    std::ostringstream oss;
    REQUIRE_NOTHROW(multiSlsDetectorClient("settingspath", GET, nullptr, oss));
    s = oss.str();
    REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));
}


TEST_CASE("subdeadtime", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {
        std::string s;
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("subdeadtime", GET, nullptr, oss));
        s = oss.str();
        REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("subdeadtime", GET));
    }
}

TEST_CASE("subexptime", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {
        std::string s;
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("subexptime", GET, nullptr, oss));
        s = oss.str();
        REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("subexptime", GET));
    }
}


TEST_CASE("dr", "[.cmd][.eiger]") {
    if (test::type == slsDetectorDefs::EIGER) {
        int vals[4] = {4, 8, 16, 32};
        for (int i = 0; i < 4; ++i) {
            REQUIRE_NOTHROW(multiSlsDetectorClient("dr " + std::to_string(vals[i]), PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dr", GET, nullptr, oss));
            REQUIRE(oss.str() == "dr " + std::to_string(vals[i]) + '\n');
        } 
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("dr 4", PUT));
        REQUIRE_THROWS(multiSlsDetectorClient("dr 8", PUT));
        REQUIRE_THROWS(multiSlsDetectorClient("dr 32", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("dr 16", PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dr", GET, nullptr, oss));
            REQUIRE(oss.str() == "dr " + std::to_string(16) + '\n');
        } 
    }
}


TEST_CASE("zmqip", "[.cmd]") {
    std::string s;
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:zmqip", GET, nullptr, oss));
        s = oss.str();
    }
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:zmqip", GET, nullptr, oss));
        REQUIRE(oss.str() == s);
    } 
}

TEST_CASE("rx_zmqip", "[.cmd]") {
    std::string s;
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_zmqip", GET, nullptr, oss));
        s = oss.str();
    }
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_zmqip", GET, nullptr, oss));
        REQUIRE(oss.str() == s);
    } 
}


TEST_CASE("zmqport", "[.cmd]") {
    multiSlsDetector d;
    int socketsperdetector = 1;
    if (test::type == slsDetectorDefs::EIGER) {
        socketsperdetector *= 2;
    } else if (test::type == slsDetectorDefs::JUNGFRAU) {
        REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 2", PUT));      
        socketsperdetector *= 2;
    }
    int port = 3500;
    REQUIRE_NOTHROW(multiSlsDetectorClient("zmqport " + std::to_string(port), PUT));
    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) + ":zmqport", GET, nullptr, oss));
        REQUIRE(oss.str() == "zmqport " + std::to_string(port + i * socketsperdetector) + '\n');   
    }
    port = 1954;
    REQUIRE_NOTHROW(multiSlsDetectorClient("zmqport " + std::to_string(port), PUT));
    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) + ":zmqport", GET, nullptr, oss));
        REQUIRE(oss.str() == "zmqport " + std::to_string(port + i * socketsperdetector) + '\n');   
    }
    if (test::type == slsDetectorDefs::JUNGFRAU) {
        REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 1", PUT));      
    }
}

TEST_CASE("rx_zmqport", "[.cmd]") {
    multiSlsDetector d;
    int socketsperdetector = 1;
    if (test::type == slsDetectorDefs::EIGER) {
        socketsperdetector *= 2;
    } else if (test::type == slsDetectorDefs::JUNGFRAU) {
        REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 2", PUT));      
        socketsperdetector *= 2;
    }
    int port = 3500;
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_zmqport " + std::to_string(port), PUT));
    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) + ":rx_zmqport", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_zmqport " + std::to_string(port + i * socketsperdetector) + '\n');   
    }
    port = 30001;
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_zmqport " + std::to_string(port), PUT));
    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) + ":rx_zmqport", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_zmqport " + std::to_string(port + i * socketsperdetector) + '\n');   
    }
    if (test::type == slsDetectorDefs::JUNGFRAU) {
        REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 1", PUT));      
    }    
}

TEST_CASE("rx_datastream", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_datastream 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_datastream 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_datastream", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_datastream 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_datastream 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_datastream 0\n");
    }
}

TEST_CASE("fpath", "[.cmd]") {
    std::string s;
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:fpath", GET, nullptr, oss));
        s = oss.str();
    }
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient(s, PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fpath", GET, nullptr, oss));
        REQUIRE(oss.str() == s);
    }
}

TEST_CASE("fformat", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fformat", GET, nullptr, oss));
         REQUIRE(oss.str() == "fformat binary\n");
    }
}


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
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_hostname none", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_hostname", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_hostname none\n");
    }        
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_hostname 129.129.205.80", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:rx_hostname", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_hostname 129.129.205.80\n");
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
    /* {TODO custom srcip in globals
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip 129.129.205.203", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip", GET, nullptr, oss));
        REQUIRE(oss.str() == "udp_srcip 129.129.205.203\n");
    }*/
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
    /* {TODO custom dstip in globals
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstmac 10:e7:c6:48:bd:3f", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstmac", GET, nullptr, oss));
        REQUIRE(oss.str() == "udp_dstmac 10:e7:c6:48:bd:3f\n");
    }  */    
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport 6200", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport", GET, nullptr, oss));
        REQUIRE(oss.str() == "udp_dstport 6200\n");
    }  
    {
        multiSlsDetector d;
        int socketsperdetector = 1;
        if (test::type == slsDetectorDefs::EIGER) {
            socketsperdetector *= 2;
        } else if (test::type == slsDetectorDefs::JUNGFRAU) {
            REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 2", PUT));      
            socketsperdetector *= 2;
        }
        int port = 5500;
        REQUIRE_NOTHROW(multiSlsDetectorClient("udp_dstport " + std::to_string(port), PUT));
        for (size_t i = 0; i != d.size(); ++i) {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) + ":udp_dstport", GET, nullptr, oss));
            REQUIRE(oss.str() == "udp_dstport " + std::to_string(port + i * socketsperdetector) + '\n');   
        }
        port = 50001;
        REQUIRE_NOTHROW(multiSlsDetectorClient("udp_dstport " + std::to_string(port), PUT));
        for (size_t i = 0; i != d.size(); ++i) {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) + ":udp_dstport", GET, nullptr, oss));
            REQUIRE(oss.str() == "udp_dstport " + std::to_string(port + i * socketsperdetector) + '\n');   
        }
        if (test::type == slsDetectorDefs::JUNGFRAU) {
            REQUIRE_NOTHROW(multiSlsDetectorClient("numinterfaces 1", PUT));      
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("udp_dstport 50001", PUT)); 
    }
    REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip 0.0.0.0", PUT));
    REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip 124586954", PUT));
    REQUIRE_THROWS(multiSlsDetectorClient("udp_srcip 999.999.0.0.0.5", PUT));

    if (test::type == slsDetectorDefs::JUNGFRAU) {
       /*  {TODO custom srcip2 in globals
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip2 129.129.205.203", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_srcip2", GET, nullptr, oss));
            REQUIRE(oss.str() == "udp_srcip2 129.129.205.203\n");
       }*/
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
        REQUIRE_NOTHROW(multiSlsDetectorClient("udp_dstport2 50002", PUT));
    } else if (test::type == slsDetectorDefs::EIGER) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport2 6400", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:udp_dstport2", GET, nullptr, oss));
            REQUIRE(oss.str() == "udp_dstport2 6400\n");
        } 
        REQUIRE_NOTHROW(multiSlsDetectorClient("udp_dstport2 50002", PUT));
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
        REQUIRE_NOTHROW(multiSlsDetectorClient("timing", GET, nullptr, oss));
        REQUIRE(oss.str() == "timing auto\n");
    }
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("timing trigger", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("timing", GET, nullptr, oss));
        REQUIRE(oss.str() == "timing trigger\n");
    }
    if (test::type == slsDetectorDefs::EIGER) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("timing gating", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("timing", GET, nullptr, oss));
            REQUIRE(oss.str() == "timing gating\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("timing burst_trigger", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("timing", GET, nullptr, oss));
            REQUIRE(oss.str() == "timing burst_trigger\n");
        }
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("timing gating", PUT));
        REQUIRE_THROWS(multiSlsDetectorClient("timing burst_trigger", PUT));
    }
    REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
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
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_fpga", GET, nullptr, oss));
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
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_adc", GET, nullptr, oss));
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
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_fpgaext", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("temp_fpgaext "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_10ge", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("temp_10ge "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_dcdc", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("temp_dcdc "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_sodl", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("temp_sodl "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_sodr", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("temp_sodr "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_fpgafl", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("temp_fpgafl "));
            REQUIRE(std::stoi(s) != -1);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:temp_fpgafr", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("temp_fpgafr "));
            REQUIRE(std::stoi(s) != -1);
        }
    }
}


TEST_CASE("vhighvoltage", "[.cmd]") {
    if (test::type == slsDetectorDefs::GOTTHARD) {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 90", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage", GET, nullptr, oss));
            REQUIRE(oss.str() == "vhighvoltage 90\n");
        }    
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage", GET, nullptr, oss));
            REQUIRE(oss.str() == "vhighvoltage 0\n");
        } 
        REQUIRE_THROWS(multiSlsDetectorClient("vhighvoltage 50", PUT));      
    } else if (test::type == slsDetectorDefs::EIGER) {
        REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 50", PUT));   
        REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 120", PUT));
        sleep(2);
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("0:vhighvoltage", GET, nullptr, oss));
        REQUIRE(oss.str() == "vhighvoltage 120\n");
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 0", PUT));
            sleep(2);
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("0:vhighvoltage", GET, nullptr, oss));
            REQUIRE(oss.str() == "vhighvoltage 0\n");
        } 
    } else {
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 50", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage", GET, nullptr, oss));
            REQUIRE(oss.str() == "vhighvoltage 50\n"); 
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 120", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage", GET, nullptr, oss));
            REQUIRE(oss.str() == "vhighvoltage 120\n"); 
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("vhighvoltage", GET, nullptr, oss));
            REQUIRE(oss.str() == "vhighvoltage 0\n");
        }              
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

    if (test::type == slsDetectorDefs::GOTTHARD) { 
        REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 120", PUT));
        // get is -1
    } else if (test::type == slsDetectorDefs::CHIPTESTBOARD || test::type == slsDetectorDefs::JUNGFRAU) { 
        int prev_val = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("adcphase "));
            prev_val = std::stoi(s);
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 20", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase", GET, nullptr, oss));
            REQUIRE(oss.str() == "adcphase 20\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase", GET, nullptr, oss));
            REQUIRE(oss.str() == "adcphase 0\n");
        }     
        if (test::type == slsDetectorDefs::GOTTHARD) {
            REQUIRE_THROWS(multiSlsDetectorClient("adcphase deg", GET));
        } else {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase 20 deg", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase deg", GET, nullptr, oss));
            REQUIRE(oss.str() == "adcphase 20 deg\n");
        }          
        REQUIRE_NOTHROW(multiSlsDetectorClient("adcphase " + std::to_string(prev_val), PUT));                    
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("adcphase", GET));      
    }
}

TEST_CASE("syncclk", "[.cmd][.ctb]") {
    if(test::type != slsDetectorDefs::CHIPTESTBOARD) {
       REQUIRE_THROWS(multiSlsDetectorClient("syncclk", GET));     
    } else { 
        REQUIRE_NOTHROW(multiSlsDetectorClient("syncclk", GET));
    }  
}

TEST_CASE("adcclk", "[.cmd][.ctb]") {
    if(test::type != slsDetectorDefs::CHIPTESTBOARD) {
       REQUIRE_THROWS(multiSlsDetectorClient("adcclk", GET)); 
    } else { 
        int prev_clk = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("adcclk "));
            prev_clk = std::stoi(s);
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk 20", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk", GET, nullptr, oss));
            REQUIRE(oss.str() == "adcclk 20\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk 10", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk", GET, nullptr, oss));
            REQUIRE(oss.str() == "adcclk 10\n");
        }        
        REQUIRE_NOTHROW(multiSlsDetectorClient("adcclk " + std::to_string(prev_clk), PUT));      
    } 
}

TEST_CASE("dbitclk", "[.cmd][.ctb]") {
    if(test::type != slsDetectorDefs::CHIPTESTBOARD) {
       REQUIRE_THROWS(multiSlsDetectorClient("dbitclk", GET));     
    } else { 
        int prev_clk = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("dbitclk "));
            prev_clk = std::stoi(s);
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk 20", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk", GET, nullptr, oss));
            REQUIRE(oss.str() == "dbitclk 20\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk 10", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk", GET, nullptr, oss));
            REQUIRE(oss.str() == "dbitclk 10\n");
        }        
        REQUIRE_NOTHROW(multiSlsDetectorClient("dbitclk " + std::to_string(prev_clk), PUT));             
    } 
}

TEST_CASE("runclk", "[.cmd][.ctb]") {
    if(test::type != slsDetectorDefs::CHIPTESTBOARD) {
       ;// REQUIRE_THROWS(multiSlsDetectorClient("runclk", GET)); Only once setspeed is split into many  (runclk = speed for now)      
    } else { 
        int prev_runclk = 0;   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("runclk", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("runclk "));
            prev_runclk = std::stoi(s);
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("runclk 20", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("runclk", GET, nullptr, oss));
            REQUIRE(oss.str() == "runclk 20\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("runclk 10", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("runclk", GET, nullptr, oss));
            REQUIRE(oss.str() == "runclk 10\n");
        }        
        REQUIRE_NOTHROW(multiSlsDetectorClient("runclk " + std::to_string(prev_runclk), PUT));                    
    } 
}

TEST_CASE("speed", "[.cmd][.eiger][.jungfrau]") {
    if(test::type != slsDetectorDefs::EIGER && test::type != slsDetectorDefs::JUNGFRAU) {
        REQUIRE_THROWS(multiSlsDetectorClient("speed", GET));         
    } else {    
        /*{TODO : only for new boards
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed 0", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr, oss));
            REQUIRE(oss.str() == "speed full_speed\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed full_speed", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr, oss));
            REQUIRE(oss.str() == "speed full_speed\n");
        }*/
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed 1", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr, oss));
            REQUIRE(oss.str() == "speed half_speed\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed half_speed", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr, oss));
            REQUIRE(oss.str() == "speed half_speed\n");
        }  
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed 2", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr, oss));
            REQUIRE(oss.str() == "speed quarter_speed\n");
        }
        {
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed quarter_speed", PUT));
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("speed", GET, nullptr, oss));
            REQUIRE(oss.str() == "speed quarter_speed\n");
        }
        REQUIRE_THROWS(multiSlsDetectorClient("speed 3", PUT));                         
    } 
}

TEST_CASE("triggers", "[.cmd]") {
    {
        REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 10", PUT));
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("triggers", GET, nullptr, oss));
        REQUIRE(oss.str() == "triggers 10\n");
    } 
    REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 1", PUT));
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

        case slsDetectorDefs::GOTTHARD:
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings dynamicgain", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings highgain", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings lowgain", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings mediumgain", PUT));
            REQUIRE_NOTHROW(multiSlsDetectorClient("settings veryhighgain", PUT));     
        default:
            REQUIRE_THROWS(multiSlsDetectorClient("settings", GET));
            break;
    }
}

TEST_CASE("threshold", "[.cmd]") {
    if (test::type == slsDetectorDefs::EIGER) {
        REQUIRE_NOTHROW(multiSlsDetectorClient("threshold 6400 standard", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("thresholdnotb 6400 standard", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("threshold 6400", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("thresholdnotb 6400", PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("threshold", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("threshold "));
            REQUIRE(std::stoi(s) == 6400);
            REQUIRE_THROWS(multiSlsDetectorClient("thresholdnotb", GET));
        }
    } else {
         REQUIRE_THROWS(multiSlsDetectorClient("threshold", GET));
         REQUIRE_THROWS(multiSlsDetectorClient("thresholdnotb", GET));         
    }
}


TEST_CASE("detsize", "[.cmd]") {
    CHECK_NOTHROW(multiSlsDetectorClient("detsize", GET));
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

    REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
    REQUIRE_NOTHROW(multiSlsDetectorClient("frames 10", PUT));
    REQUIRE_NOTHROW(multiSlsDetectorClient("period 1", PUT));

    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT, nullptr, oss));
        REQUIRE(oss.str() == "start successful\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("status", GET, nullptr, oss));
        REQUIRE(oss.str() == "status running\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT, nullptr, oss));
        REQUIRE(oss.str() == "stop successful\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("status", GET, nullptr, oss));
        REQUIRE(oss.str() != "status running\n");
        REQUIRE(oss.str() != "status waiting\n");
        REQUIRE(oss.str() != "status transmitting\n");
    }
    REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
}

TEST_CASE("trigger", "[.cmd][.eiger]") {
    if(test::type != slsDetectorDefs::EIGER) {
        REQUIRE_THROWS(multiSlsDetectorClient("trigger", PUT));         
    } else {    
        // trigger
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("timing trigger", PUT, nullptr, oss));
            REQUIRE(oss.str() == "timing trigger\n");
        }
        int startingfnum = 0;
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("startingfnum", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("startingfnum "));
            startingfnum = std::stoi(s);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT, nullptr, oss));
            REQUIRE(oss.str() == "start successful\n");
        } 
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("status", GET, nullptr, oss));
            REQUIRE(oss.str() != "status idle\n");
            REQUIRE(oss.str() != "status stopped\n");        
        }   
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("trigger", PUT, nullptr, oss));
            REQUIRE(oss.str() == "trigger successful\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));
        int currentfnum = 0;
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("startingfnum", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("startingfnum "));
            currentfnum = std::stoi(s);
        } 
        REQUIRE((startingfnum + 1) == currentfnum);


        REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
    }
}


TEST_CASE("framesl", "[.cmd][.jungfrau][gotthard][ctb]") {
    if(test::type == slsDetectorDefs::EIGER) {
        REQUIRE_THROWS(multiSlsDetectorClient("framesl", GET));         
    } else {
        REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 10", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("period 1", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("framesl", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("framesl "));
            int framesl = std::stoi(s);
            REQUIRE(framesl > 0);
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
    }
}

TEST_CASE("triggersl", "[.cmd][.jungfrau][gotthard][ctb]") {
    if(test::type == slsDetectorDefs::EIGER) {
        REQUIRE_THROWS(multiSlsDetectorClient("triggersl", GET));       
    } else {
        REQUIRE_NOTHROW(multiSlsDetectorClient("timing trigger", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 10", PUT));   
        REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("triggersl", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("triggersl "));
            int triggersl = std::stoi(s);
            REQUIRE(triggersl == 8);
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 1", PUT));
    }
}

TEST_CASE("delayl", "[.cmd][.jungfrau][gotthard][ctb]") {
    if(test::type == slsDetectorDefs::EIGER) {
        REQUIRE_THROWS(multiSlsDetectorClient("delayl", GET));       
    } else  if(test::type == slsDetectorDefs::GOTTHARD) {
        REQUIRE_NOTHROW(multiSlsDetectorClient("delay 0", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("delayl", GET));
        // delayl always gives 0 for gotthard
    } else {
        REQUIRE_NOTHROW(multiSlsDetectorClient("timing trigger", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 2", PUT));  
         REQUIRE_NOTHROW(multiSlsDetectorClient("delay 1", PUT));          
        REQUIRE_NOTHROW(multiSlsDetectorClient("start", PUT));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("delayl s", GET, nullptr, oss));
            REQUIRE(oss.str()  == "delayl 1s\n");
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("stop", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("timing auto", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("triggers 1", PUT));
        REQUIRE_NOTHROW(multiSlsDetectorClient("delay 0", PUT));
    }
}
TEST_CASE("clk", "[.cmd]") {
    if(test::type == slsDetectorDefs::GOTTHARD2) {
        REQUIRE_THROWS(multiSlsDetectorClient("clkfreq 0 2", PUT)); // cannot get
        REQUIRE_THROWS(multiSlsDetectorClient("clkfreq", GET)); // requires clk index
        REQUIRE_THROWS(multiSlsDetectorClient("clkfreq 7", GET)); // 7 doesnt exist
        REQUIRE_THROWS(multiSlsDetectorClient("clkfreq 4", PUT)); // requires clk index and val
        REQUIRE_THROWS(multiSlsDetectorClient("clkfreq 7 4", PUT)); // 7 doesnt exist  
        REQUIRE_THROWS(multiSlsDetectorClient("clkphase", GET)); // requires clk index
        REQUIRE_THROWS(multiSlsDetectorClient("clkphase 7", GET)); // 7 doesnt exist
        REQUIRE_THROWS(multiSlsDetectorClient("clkphase 4", PUT)); // requires clk index and val
        REQUIRE_THROWS(multiSlsDetectorClient("clkphase 7 4", PUT)); // 7 doesnt exist        
        REQUIRE_THROWS(multiSlsDetectorClient("clkdiv", GET)); // requires clk index
        REQUIRE_THROWS(multiSlsDetectorClient("clkdiv 7", GET)); // 7 doesnt exist
        REQUIRE_THROWS(multiSlsDetectorClient("clkdiv 4", PUT)); // requires clk index and val
        REQUIRE_THROWS(multiSlsDetectorClient("clkdiv 7 4", PUT)); // 7 doesnt exist  

        int t = 0;
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("clkdiv 0", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("clkdiv "));
            t = std::stoi(s);
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("clkdiv 0 " + std::to_string(t), PUT, nullptr, oss));
            REQUIRE(oss.str() == "clkdiv " + std::to_string(t) + '\n');
        }
        REQUIRE_NOTHROW(multiSlsDetectorClient("clkfreq 0", GET));
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("clkphase 1 20", PUT, nullptr, oss));
            REQUIRE(oss.str() == "clkphase 20\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("clkphase 1", GET, nullptr, oss));
            REQUIRE(oss.str() == "clkphase 20\n");
        }  
    } else {
        REQUIRE_THROWS(multiSlsDetectorClient("clkfreq 0", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("clkphase 0", GET));
        REQUIRE_THROWS(multiSlsDetectorClient("clkdiv 0", GET));
    }
}

TEST_CASE("rx_fifodepth", "[.cmd]") {
    int prev_val = 0;   
    {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("rx_fifodepth", GET, nullptr, oss));
            std::string s = (oss.str()).erase (0, strlen("rx_fifodepth "));
            prev_val = std::stoi(s);
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_fifodepth 10", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_fifodepth 10\n");
    }

    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_fifodepth 100", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_fifodepth 100\n");
    }

    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_fifodepth", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_fifodepth 100\n");
    }
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_fifodepth " + std::to_string(prev_val), PUT)); 
}

TEST_CASE("frames", "[.cmd]") {

    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1000", PUT, nullptr, oss));
        REQUIRE(oss.str() == "frames 1000\n");
    }

    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames", GET, nullptr, oss));
        REQUIRE(oss.str() == "frames 1000\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("frames 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "frames 1\n");
    }
}

TEST_CASE("rx_status", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_start", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_start successful\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_status", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_status running\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_stop", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_stop successful\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_status", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_status idle\n");
    }
}

TEST_CASE("fwrite", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fwrite 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fwrite", GET, nullptr, oss));
        REQUIRE(oss.str() == "fwrite 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fwrite 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "fwrite 0\n");
    }
}

TEST_CASE("foverwrite", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("foverwrite 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("foverwrite", GET, nullptr, oss));
        REQUIRE(oss.str() == "foverwrite 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("foverwrite 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "foverwrite 0\n");
    }
}


TEST_CASE("fmaster", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fmaster 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fmaster", GET, nullptr, oss));
        REQUIRE(oss.str() == "fmaster 0\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fmaster 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "fmaster 1\n");
    }
}

TEST_CASE("findex", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("findex 57", PUT, nullptr, oss));
        REQUIRE(oss.str() == "findex 57\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("findex", GET, nullptr, oss));
        REQUIRE(oss.str() == "findex 57\n");
    }
    {

        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("findex 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "findex 0\n");
    }
}

TEST_CASE("rx_tcpport", "[.cmd]") {
    multiSlsDetector d;
    int port = 3500;
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_tcpport " + std::to_string(port), PUT));
    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) + ":rx_tcpport", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');   
    }
    REQUIRE_THROWS(multiSlsDetectorClient("rx_tcpport 15", PUT));
    port = 1954;
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_tcpport " + std::to_string(port), PUT));
    for (size_t i = 0; i != d.size(); ++i) {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient(std::to_string(i) + ":rx_tcpport", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_tcpport " + std::to_string(port + i) + '\n');   
    }
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_tcpport 1954", PUT));
}

TEST_CASE("fname", "[.cmd]") {

    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fname somename", PUT, nullptr, oss));
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fname", GET, nullptr, oss));
        REQUIRE(oss.str() == "fname somename\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("fname run", PUT, nullptr, oss));
        REQUIRE(oss.str() == "fname run\n");
    }
}

TEST_CASE("rx_framescaught", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("resetframescaught 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "resetframescaught successful\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_framescaught", GET, nullptr, oss));
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
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_framescaught", GET, nullptr, oss));
       // REQUIRE(oss.str() == "rx_framescaught 1\n");
    }      
}

TEST_CASE("rx_silent", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_silent 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_silent 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_silent", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_silent 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_silent 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_silent 0\n");
    }
}

// TEST_CASE("rx_jsonaddheader", "[.cmd]") {
//     std::ostringstream oss;
//     REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"hej\":\"5\"", PUT, nullptr,
//     oss)); REQUIRE(oss.str() == "rx_jsonaddheader \"hej\":\"5\"\n");

//     std::ostringstream oss;
//     REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader", GET, nullptr, oss));
//     REQUIRE(oss.str() == "rx_jsonaddheader \"hej\":\"5\"\n");

//     std::ostringstream oss;
//     REQUIRE_NOTHROW(multiSlsDetectorClient("rx_jsonaddheader \"\"", PUT, nullptr, oss));
//     REQUIRE(oss.str() == "rx_jsonaddheader\n");
// }

TEST_CASE("rx_udpsocksize", "[.cmd]") {
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_udpsocksize 4857600", PUT));
    uint64_t val = 0;
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_udpsocksize", GET, nullptr, oss));
        std::string s = (oss.str()).erase (0, strlen("rx_udpsocksize "));
        val = std::stol(s);
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_realudpsocksize", GET, nullptr, oss));
        std::string s = (oss.str()).erase (0, strlen("rx_realudpsocksize "));
        uint64_t rval = std::stol(s);
        REQUIRE(rval == val * 2);
    }
}


TEST_CASE("rx_framesperfile", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_framesperfile 50", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_framesperfile 50\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_framesperfile", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_framesperfile 50\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_framesperfile 10000", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_framesperfile 10000\n");
    }
}

TEST_CASE("rx_discardpolicy", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_discardpolicy discardempty", PUT, nullptr,
                               oss));
        REQUIRE(oss.str() == "rx_discardpolicy discardempty\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_discardpolicy", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_discardpolicy discardempty\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_discardpolicy discardpartial", PUT, nullptr,
                               oss));
        REQUIRE(oss.str() == "rx_discardpolicy discardpartial\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_discardpolicy nodiscard", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_discardpolicy nodiscard\n");
    }
}

TEST_CASE("rx_padding", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_padding 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_padding 0\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_padding", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_padding 0\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_padding 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_padding 1\n");
    }
}

TEST_CASE("rx_readfreq", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_readfreq 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_readfreq 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_readfreq", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_readfreq 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_readfreq 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_readfreq 0\n");
    }
}

TEST_CASE("rx_lock", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_lock 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_lock 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_lock", GET, nullptr, oss));
        REQUIRE(oss.str() == "rx_lock 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("rx_lock 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "rx_lock 0\n");
    }
}

TEST_CASE("lock", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("lock 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("lock", GET, nullptr, oss));
        REQUIRE(oss.str() == "lock 1\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("lock 0", PUT, nullptr, oss));
        REQUIRE(oss.str() == "lock 0\n");
    }
}


TEST_CASE("lastclient", "[.cmd]") {
    REQUIRE_NOTHROW(multiSlsDetectorClient("lastclient", GET));
}

TEST_CASE("rx_lastclient", "[.cmd]") {
    std::ostringstream oss;
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_lastclient", GET, nullptr, oss));
    REQUIRE(oss.str() == "rx_lastclient " + test::my_ip + "\n");
}



TEST_CASE("rx_checkversion", "[.cmd]") {

    std::ostringstream oss;
    REQUIRE_NOTHROW(multiSlsDetectorClient("rx_checkversion", GET, nullptr, oss));
    REQUIRE(oss.str() == "rx_checkversion compatible\n");
}

TEST_CASE("exptime", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("exptime 0.05", PUT, nullptr, oss));
        REQUIRE(oss.str() == "exptime 0.05\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("exptime", GET, nullptr, oss));
        REQUIRE(oss.str() == "exptime 50ms\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("exptime 1", PUT, nullptr, oss));
        REQUIRE(oss.str() == "exptime 1\n");
    }
}


TEST_CASE("period", "[.cmd]") {
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("period 1.25s", PUT, nullptr, oss));
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("period", GET, nullptr, oss));
        REQUIRE(oss.str() == "period 1.25s\n");
    }
    {
        std::ostringstream oss;
        REQUIRE_NOTHROW(multiSlsDetectorClient("period 0", PUT, nullptr, oss));
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
            REQUIRE_NOTHROW(multiSlsDetectorClient("delay 1.25s", PUT, nullptr, oss));
            REQUIRE(oss.str() == "delay 1.25s\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("delay", GET, nullptr, oss));
            REQUIRE(oss.str() == "delay 1.25s\n");
        }
        {
            std::ostringstream oss;
            REQUIRE_NOTHROW(multiSlsDetectorClient("delay 0", PUT, nullptr, oss));
            REQUIRE(oss.str() == "delay 0\n");
        }
    }
}