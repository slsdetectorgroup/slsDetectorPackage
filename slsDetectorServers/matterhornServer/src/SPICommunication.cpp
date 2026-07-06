#include "SPICommunication.h"
#include <algorithm>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace sls {

void HardwareSPICommunication::open_spi() {

    // TODO device can change
    spi_filedescriptor = open("/dev/spidev2.0", O_RDWR); // TODO use O_SYNC?

    if (spi_filedescriptor < 0) {
        throw RuntimeError("Could not open /dev/spidev2.0");
    }

    LOG(logINFO) << fmt::format("SPI Read: opened spidev2.0 with fd={}",
                                spi_filedescriptor);
}

void HardwareSPICommunication::close_spi() {
    if (spi_filedescriptor >= 0) {
        close(spi_filedescriptor);
        LOG(logINFO) << "SPI Read: closed spidev2.0";
        spi_filedescriptor = -1;
    }
}

HardwareSPICommunication::~HardwareSPICommunication() { close_spi(); }

std::vector<std::byte>
HardwareSPICommunication::spi_read(const size_t n_bytes, const uint8_t chip_id,
                                   const uint8_t register_id) const {

    // allocate dummy data to shift out the data (first byte is command byte)
    if (n_bytes == std::numeric_limits<size_t>::max()) {
        throw RuntimeError("SPI read size overflow");
    }

    std::vector<std::byte> dummy_data(
        n_bytes + 1, std::byte{0x00}); // +1 for the command byte

    // First byte of the message is 4 bits chip_id then 4 bits register_id
    dummy_data[0] =
        static_cast<std::byte>(((chip_id & 0xF) << 4) | (register_id & 0xF));

    // allocate data buffer to read out data into
    std::vector<std::byte> read_data_buffer(n_bytes + 1, std::byte{0x00});

    spi_ioc_transfer send_cmd{};
    send_cmd.len = n_bytes + 1; // +1 for the command byte
    send_cmd.tx_buf = reinterpret_cast<std::uintptr_t>(dummy_data.data());
    send_cmd.rx_buf = reinterpret_cast<std::uintptr_t>(read_data_buffer.data());

    // 0 - Normal operation, 1 - CSN remains zero after operation
    // We use cs_change = 1 to not close the SPI transaction and
    // allow for shifting the read out data back in to restore the
    // register
    send_cmd.cs_change = 1;

    // transfer here
    if (ioctl(spi_filedescriptor, SPI_IOC_MESSAGE(1), &send_cmd) < 0) {

        throw RuntimeError(
            fmt::format("SPI write failed with {}:{}", errno, strerror(errno)));
    }

    // copy data to output buffer
    std::vector<std::byte> output_data(n_bytes);
    std::memcpy(output_data.data(), read_data_buffer.data() + 1, n_bytes);

    // copy the read out data back to the dummy data buffer to shift it back in
    send_cmd.tx_buf = send_cmd.rx_buf;

    send_cmd.cs_change =
        0; // end the SPI transaction after shifting back in the data

    if (ioctl(spi_filedescriptor, SPI_IOC_MESSAGE(1), &send_cmd) < 0) {
        throw RuntimeError(
            fmt::format("SPI write failed with {}:{}", errno, strerror(errno)));
    }

    return output_data;
}

void HardwareSPICommunication::spi_write(const uint8_t chip_id,
                                         const uint8_t register_id,
                                         const std::vector<std::byte> &data) {

    const size_t n_bytes = data.size();

    if (n_bytes == std::numeric_limits<size_t>::max()) {
        throw RuntimeError("SPI read size overflow");
    }

    // First byte of the message is 4 bits chip_id then 4 bits register_id
    std::vector<std::byte> write_data(n_bytes + 1); // +1 for the command byte

    write_data[0] =
        static_cast<std::byte>(((chip_id & 0xF) << 4) | (register_id & 0xF));

    std::memcpy(write_data.data() + 1, data.data(), n_bytes);

    spi_ioc_transfer send_cmd{};
    send_cmd.len = n_bytes + 1; // +1 for the command byte
    send_cmd.tx_buf = reinterpret_cast<std::uintptr_t>(write_data.data());

    send_cmd.cs_change =
        0; // end the SPI transaction after the write (we dont need to shift
           // back in data here since we are not doing a read)

    if (ioctl(spi_filedescriptor, SPI_IOC_MESSAGE(1), &send_cmd) < 0) {
        throw RuntimeError(
            fmt::format("SPI write failed with {}:{}", errno, strerror(errno)));
    }
}

} // namespace sls