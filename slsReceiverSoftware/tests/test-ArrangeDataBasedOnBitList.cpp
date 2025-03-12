// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2025 Contributors to the SLS Detector Package
/************************************************
 * @file test-ArrangeDataBasedOnBitList.cpp
 * @short test case for DataProcessor member function ArrangeDbitData,
 ***********************************************/

#include "DataProcessor.h"
#include "GeneralData.h"
#include "catch.hpp"
#include <vector>

namespace sls {

// dummy GeneralData class for testing
class GeneralDataTest : public GeneralData {

  public:
    int GetNumberOfAnalogDatabytes() { return nAnalogBytes; };

    int GetNumberOfDigitalDatabytes() { return nDigitalBytes; };

    int GetNumberOfTransceiverDatabytes() { return nTransceiverBytes; };

    void SetNumberOfAnalogDatabytes(int value) { nAnalogBytes = value; }

    void SetNumberOfDigitalDatabytes(int value) { nDigitalBytes = value; }

    void SetNumberOfTransceiverDatabytes(int value) {
        nTransceiverBytes = value;
    }

  private:
    int nAnalogBytes;
    int nDigitalBytes;
    int nTransceiverBytes;
};

// oke maybe just make it static
class DataProcessorTest : public DataProcessor {
  public:
    DataProcessorTest() : DataProcessor(0){};
    ~DataProcessorTest(){};
    void ArrangeDbitData(size_t &size, char *data) {
        DataProcessor::ArrangeDbitData(size, data);
    }
};

TEST_CASE("Arrange with reorder false") {
    DataProcessorTest dataprocessor;

    std::vector<int> bitlist{1, 4, 5};
    dataprocessor.SetCtbDbitList(bitlist);

    size_t num_digital_samples = 5; // size_t or uint8_t ?
    size_t num_digital_bytes = num_digital_samples * 8;
    size_t num_analog_bytes = 1;
    size_t num_transceiver_bytes = 2;
    size_t random_offset_bytes = 0;

    size_t size = num_analog_bytes + num_digital_bytes + num_transceiver_bytes +
                  random_offset_bytes;

    char *data = new char[size];

    char dummy_value = static_cast<char>(125);
    memset(data, dummy_value, num_analog_bytes);
    memset(data + num_analog_bytes, 0xFF,
           num_digital_bytes); // all digital bits are one
    memset(data + num_digital_bytes + num_analog_bytes, dummy_value,
           num_transceiver_bytes);

    GeneralDataTest *generaldata = new GeneralDataTest;
    generaldata->SetNumberOfAnalogDatabytes(num_analog_bytes);
    generaldata->SetNumberOfDigitalDatabytes(num_digital_bytes);
    generaldata->SetNumberOfTransceiverDatabytes(num_transceiver_bytes);

    dataprocessor.SetGeneralData(generaldata);

    size_t new_num_digital_bytes =
        (bitlist.size() / 8 + static_cast<size_t>(bitlist.size() % 8 != 0)) *
        num_digital_samples;

    size_t new_size =
        num_analog_bytes + num_transceiver_bytes + new_num_digital_bytes;

    char *new_data = new char[new_size];

    memset(new_data, dummy_value, num_analog_bytes);

    // TODO: Make test more explicit and less generic
    size_t num_bytes_for_bitlist =
        bitlist.size() / 8 + static_cast<size_t>(bitlist.size() % 8 != 0);

    // 125 7 7 7 7 7 125 125
    for (size_t sample = 0; sample < num_digital_samples; ++sample) {
        if (bitlist.size() / 8 != 0) {
            memset(new_data + sample * num_bytes_for_bitlist + num_analog_bytes,
                   0xFF,
                   bitlist.size() / 8); // set to 1
        } else if (bitlist.size() % 8 != 0) {
            memset(new_data + sample * num_bytes_for_bitlist +
                       bitlist.size() / 8 + num_analog_bytes,
                   static_cast<char>(pow(2, (bitlist.size() % 8)) - 1),
                   1); // convert binary number to decimal
        }
    }

    memset(new_data + new_num_digital_bytes + num_analog_bytes, dummy_value,
           num_transceiver_bytes);

    dataprocessor.ArrangeDbitData(size, data);

    CHECK(size == new_size);

    CHECK(memcmp(data, new_data, size) == 0);

    // Free allocated memory
    delete[] data;
    delete[] new_data;
    delete generaldata;
}

// TEST_CASE("Arrange with reorder on") {

//}

// test case reorder on and bitoffset set

// test with different samples

// test with sample number not divisable and ctbitlist not divisable

} // namespace sls
