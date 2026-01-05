// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

/* This example assumes that you have a ctb configured or using the virtual ctb detector server*/

#include "sls/Detector.h"
#include "sls/bit_utils.h"
#include <iostream>


void somefunc(uint32_t addr){
    std::cout << "somefunc called with: " << addr << std::endl;
}

int main(){

    // Config file has the following defines
    // define addr somereg 0x5
    // define bit mybit somereg 7
    
    sls::Detector d;
    auto somereg = d.getRegisterDefinition("somereg");
    d.writeRegister(somereg, sls::RegisterValue(0));
    auto val = d.readRegister(somereg);

    std::cout << "somereg has the address: " << somereg <<  " and value " << val.squash() << std::endl;


    auto mybit = d.getBitDefinition("mybit");
    std::cout << "mybit refers to register: " << mybit.address() << " bit nr: " << mybit.bitPosition() << std::endl;
    d.setBit(mybit);
    val = d.readRegister(somereg);
    std::cout << "somereg has the address: " << somereg <<  " and value " << val.squash() << std::endl;
    std::cout << "mybit: " << d.getBit(mybit) << std::endl;


    //Let's define a bit 
    sls::BitAddress newbit(sls::RegisterAddress(0x6), 4);
    d.setBitDefinition("newbit", newbit);
    //This can now be usef from command line "g getbit newbit"




    uint32_t addr = somereg; //I'm not sure this should compile
    somefunc(somereg); //This should also not compile 

    
}