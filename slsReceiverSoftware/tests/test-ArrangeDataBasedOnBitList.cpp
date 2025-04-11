// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2025 Contributors to the SLS Detector Package
/************************************************
 * @file test-ArrangeDataBasedOnBitList.cpp
 * @short test case for DataProcessor rearrange functions,
 ***********************************************/

#include "DataProcessor.h"
#include "GeneralData.h"
#include "catch.hpp"
#include <vector>

namespace sls {

// dummy GeneralData class for testing
class GeneralDataTest : public GeneralData {

  public:
    GeneralDataTest() { detType = slsDetectorDefs::CHIPTESTBOARD; }

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

// dummy DataProcessor class for testing
class DataProcessorTest : public DataProcessor {
  public:
    DataProcessorTest() : DataProcessor(0) {};
    ~DataProcessorTest() {};
    void ArrangeDbitData(size_t &size, char *data) {
        DataProcessor::ArrangeDbitData(size, data);
    }

    void RemoveTrailingBits(size_t &size, char *data) {
        DataProcessor::RemoveTrailingBits(size, data);
    }
};

/**
 * test fixture for Testing,
 * num_analog_bytes = 1 byte has a value of 125
 * num_transceiver_bytes = 2 both bytes have a value of 125
 * num_digital_bytes is variable and is defined by number of samples
 * default num sample is 5
 * all bytes in digital data take a value of 255
 */
class DataProcessorTestFixture {
  public:
    DataProcessorTestFixture() {
        // setup Test Fixture
        dataprocessor = new DataProcessorTest;
        generaldata = new GeneralDataTest;

        // set_num_samples(num_samples);

        generaldata->SetNumberOfAnalogDatabytes(num_analog_bytes);
        generaldata->SetNumberOfTransceiverDatabytes(num_transceiver_bytes);
        generaldata->SetNumberOfDigitalDatabytes(num_digital_bytes +
                                                 num_random_offset_bytes);

        dataprocessor->SetGeneralData(generaldata);
    }

    ~DataProcessorTestFixture() {
        delete[] data;
        delete dataprocessor;
        delete generaldata;
    }

    size_t get_size() const {
        return num_analog_bytes + num_digital_bytes + num_transceiver_bytes +
               num_random_offset_bytes;
    }

    void set_num_samples(const size_t value) {
        num_samples = value;
        num_digital_bytes = num_samples * 8; // 64 (8 bytes) per sample

        generaldata->SetNumberOfDigitalDatabytes(num_digital_bytes +
                                                 num_random_offset_bytes);
    }

    void set_random_offset_bytes(const size_t value) {
        num_random_offset_bytes = value;
        generaldata->SetNumberOfDigitalDatabytes(num_digital_bytes +
                                                 num_random_offset_bytes);
    }

    void set_data() {
        delete[] data;
        uint64_t max_bytes_per_bit =
            num_samples % 8 == 0 ? num_samples / 8 : num_samples / 8 + 1;
        uint64_t reserved_size =
            get_size() - num_digital_bytes + max_bytes_per_bit * 64;
        data = new char[reserved_size];

        // set testing data
        memset(data, dummy_value, num_analog_bytes); // set to dummy value
        memset(data + num_analog_bytes, 0,
               num_random_offset_bytes); // set to zero
        memset(data + num_analog_bytes + num_random_offset_bytes, 0xFF,
               num_digital_bytes); // all digital bits are one
        memset(data + num_digital_bytes + num_analog_bytes +
                   num_random_offset_bytes,
               dummy_value,
               num_transceiver_bytes); // set to dummy value
    }

    DataProcessorTest *dataprocessor;
    GeneralDataTest *generaldata;
    const size_t num_analog_bytes = 1;
    const size_t num_transceiver_bytes = 2;
    const char dummy_value = static_cast<char>(125);
    size_t num_digital_bytes = 40; // num_samples * 8 = 5 * 8 = 40
    size_t num_random_offset_bytes = 0;
    size_t num_samples = 5;
    char *data = nullptr;
};

TEST_CASE_METHOD(DataProcessorTestFixture, "Remove Trailing Bits",
                 "[.dataprocessor][.bitoffset]") {

    const size_t num_random_offset_bytes = 3;
    set_random_offset_bytes(num_random_offset_bytes);
    set_data();

    dataprocessor->SetCtbDbitOffset(num_random_offset_bytes);

    size_t expected_size = get_size() - num_random_offset_bytes;

    char *expected_data = new char[expected_size];
    memset(expected_data, dummy_value, num_analog_bytes); // set to 125
    memset(expected_data + num_analog_bytes, 0xFF,
           num_digital_bytes); // set to 1
    memset(expected_data + num_digital_bytes + num_analog_bytes, dummy_value,
           num_transceiver_bytes); // set to 125

    size_t size = get_size();
    dataprocessor->RemoveTrailingBits(size, data);

    CHECK(size == expected_size);

    CHECK(memcmp(data, expected_data, expected_size) == 0);

    delete[] expected_data;
}

// parametric test tested with num_samples = 5, num_samples = 10, num_samples =
// 8
TEST_CASE_METHOD(DataProcessorTestFixture, "Reorder all",
                 "[.dataprocessor][.reorder]") {
    // parameters: num_samples, expected_num_digital_bytes,
    // expected_digital_part
    auto parameters = GENERATE(
        std::make_tuple(5, 64, std::vector<uint8_t>{0b00011111}),
        std::make_tuple(10, 2 * 64, std::vector<uint8_t>{0xFF, 0b00000011}),
        std::make_tuple(8, 64, std::vector<uint8_t>{0xFF}));

    size_t num_samples, expected_num_digital_bytes;
    std::vector<uint8_t> expected_digital_part;
    std::tie(num_samples, expected_num_digital_bytes, expected_digital_part) =
        parameters;

    // set number of samples for test fixture -> create data
    set_num_samples(num_samples);
    set_data();

    std::vector<int> bitlist(64);
    std::iota(bitlist.begin(), bitlist.end(), 0);
    dataprocessor->SetCtbDbitList(bitlist);
    dataprocessor->SetCtbDbitReorder(true); // set reorder to true

    const size_t expected_size =
        num_analog_bytes + num_transceiver_bytes + expected_num_digital_bytes;

    // create expected data
    char *expected_data = new char[expected_size];

    memset(expected_data, dummy_value, num_analog_bytes); // set to 125
    for (size_t bit = 0; bit < 64; ++bit) {
        memcpy(expected_data + num_analog_bytes +
                   expected_digital_part.size() * bit,
               expected_digital_part.data(), expected_digital_part.size());
    }
    memset(expected_data + expected_num_digital_bytes + num_analog_bytes,
           dummy_value,
           num_transceiver_bytes); // set to 125

    size_t size = get_size();
    dataprocessor->ArrangeDbitData(size, data); // call reorder

    CHECK(size == expected_size);
    CHECK(memcmp(data, expected_data, expected_size) == 0);

    delete[] expected_data;
}

TEST_CASE_METHOD(DataProcessorTestFixture,
                 "Reorder all and remove trailing bits",
                 "[.dataprocessor][.reorder]") {

    // set number of samples for test fixture -> create data
    const size_t num_random_offset_bytes = 3;
    set_random_offset_bytes(num_random_offset_bytes);
    set_data();

    std::vector<int> bitlist(64);
    std::iota(bitlist.begin(), bitlist.end(), 0);
    dataprocessor->SetCtbDbitList(bitlist);
    dataprocessor->SetCtbDbitOffset(num_random_offset_bytes);
    dataprocessor->SetCtbDbitReorder(true); // set reorder to true

    const size_t expected_num_digital_bytes = 64;
    std::vector<uint8_t> expected_digital_part{0b00011111};

    const size_t expected_size =
        num_analog_bytes + num_transceiver_bytes + expected_num_digital_bytes;

    // create expected data
    char *expected_data = new char[expected_size];

    memset(expected_data, dummy_value, num_analog_bytes); // set to 125
    for (size_t bit = 0; bit < 64; ++bit) {
        memcpy(expected_data + num_analog_bytes +
                   expected_digital_part.size() * bit,
               expected_digital_part.data(), expected_digital_part.size());
    }
    memset(expected_data + expected_num_digital_bytes + num_analog_bytes,
           dummy_value,
           num_transceiver_bytes); // set to 125

    size_t size = get_size();
    dataprocessor->ArrangeDbitData(size, data); // call reorder

    CHECK(size == expected_size);
    CHECK(memcmp(data, expected_data, expected_size) == 0);

    delete[] expected_data;
}

TEST_CASE_METHOD(DataProcessorTestFixture, "Arrange bitlist with reorder false",
                 "[.dataprocessor][.retrievebitlist]") {
    // parameters: num_samples, bitlist, expected_num_digital_bytes,
    // expected_digital_part
    auto parameters = GENERATE(
        std::make_tuple(5, std::vector<int>{1, 4, 5}, 5,
                        std::vector<uint8_t>{0b00000111}),
        std::make_tuple(5, std::vector<int>{1, 5, 3, 7, 8, 50, 42, 60, 39}, 10,
                        std::vector<uint8_t>{0xFF, 0b00000001}),
        std::make_tuple(5, std::vector<int>{1, 5, 3, 7, 8, 50, 42, 60}, 5,
                        std::vector<uint8_t>{0xFF}));

    size_t num_samples, expected_num_digital_bytes;
    std::vector<uint8_t> expected_digital_part;
    std::vector<int> bitlist;
    std::tie(num_samples, bitlist, expected_num_digital_bytes,
             expected_digital_part) = parameters;

    dataprocessor->SetCtbDbitList(bitlist);

    dataprocessor->SetCtbDbitReorder(false);

    set_num_samples(num_samples);
    set_data();

    size_t expected_size =
        num_analog_bytes + num_transceiver_bytes + expected_num_digital_bytes;

    // create expected data
    char *expected_data = new char[expected_size];

    memset(expected_data, dummy_value, num_analog_bytes);

    for (size_t sample = 0; sample < num_samples; ++sample) {
        memcpy(expected_data + num_analog_bytes +
                   expected_digital_part.size() * sample,
               expected_digital_part.data(), expected_digital_part.size());
    }

    memset(expected_data + expected_num_digital_bytes + num_analog_bytes,
           dummy_value, num_transceiver_bytes);

    size_t size = get_size();
    dataprocessor->ArrangeDbitData(size, data);

    CHECK(size == expected_size);

    CHECK(memcmp(data, expected_data, expected_size) == 0);

    delete[] expected_data;
}

TEST_CASE_METHOD(DataProcessorTestFixture, "Arrange bitlist with reorder true",
                 "[.dataprocessor][.retrievebitlist]") {
    // parameters: num_samples, bitlist, expected_num_digital_bytes,
    // expected_digital_part
    auto parameters = GENERATE(
        std::make_tuple(5, std::vector<int>{1, 4, 5}, 3,
                        std::vector<uint8_t>{0b00011111}),
        std::make_tuple(10, std::vector<int>{1, 4, 5}, 6,
                        std::vector<uint8_t>{0xFF, 0b00000011}),
        std::make_tuple(8, std::vector<int>{1, 5, 3, 7, 8, 50, 42, 60, 39}, 9,
                        std::vector<uint8_t>{0xFF}));

    size_t num_samples, expected_num_digital_bytes;
    std::vector<uint8_t> expected_digital_part;
    std::vector<int> bitlist;
    std::tie(num_samples, bitlist, expected_num_digital_bytes,
             expected_digital_part) = parameters;

    dataprocessor->SetCtbDbitList(bitlist);

    dataprocessor->SetCtbDbitReorder(true);

    set_num_samples(num_samples);
    set_data();

    size_t expected_size =
        num_analog_bytes + num_transceiver_bytes + expected_num_digital_bytes;

    // create expected data
    char *expected_data = new char[expected_size];

    memset(expected_data, dummy_value, num_analog_bytes);

    for (size_t sample = 0; sample < bitlist.size(); ++sample) {
        memcpy(expected_data + num_analog_bytes +
                   expected_digital_part.size() * sample,
               expected_digital_part.data(), expected_digital_part.size());
    }

    memset(expected_data + expected_num_digital_bytes + num_analog_bytes,
           dummy_value, num_transceiver_bytes);

    size_t size = get_size();
    dataprocessor->ArrangeDbitData(size, data);

    CHECK(size == expected_size);

    CHECK(memcmp(data, expected_data, expected_size) == 0);

    delete[] expected_data;
}

TEST_CASE_METHOD(DataProcessorTestFixture,
                 "Arrange bitlist and remove trailing bits",
                 "[.dataprocessor][.retrievebitlist]") {

    size_t num_random_offset_bytes = 3;
    std::vector<int> bitlist{1, 4, 5};

    set_random_offset_bytes(num_random_offset_bytes);
    set_data();

    dataprocessor->SetCtbDbitList(bitlist);

    dataprocessor->SetCtbDbitReorder(false);

    dataprocessor->SetCtbDbitOffset(num_random_offset_bytes);

    std::vector<uint8_t> expected_digital_part{0b00000111};
    const size_t expected_num_digital_bytes = 5;

    size_t expected_size =
        num_analog_bytes + num_transceiver_bytes + expected_num_digital_bytes;

    // create expected data
    char *expected_data = new char[expected_size];

    memset(expected_data, dummy_value, num_analog_bytes);

    for (size_t sample = 0; sample < num_samples; ++sample) {
        memcpy(expected_data + num_analog_bytes +
                   expected_digital_part.size() * sample,
               expected_digital_part.data(), expected_digital_part.size());
    }

    memset(expected_data + expected_num_digital_bytes + num_analog_bytes,
           dummy_value, num_transceiver_bytes);

    size_t size = get_size();
    dataprocessor->ArrangeDbitData(size, data);

    CHECK(size == expected_size);

    CHECK(memcmp(data, expected_data, expected_size) == 0);

    delete[] expected_data;
}

} // namespace sls
