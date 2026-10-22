// SPDX-License-Identifier: LGPL-3.0-or-other
// ===========================================================================
// test/jf_feeder.cpp -- synthetic slsReceiver stand-in.
//
// Publishes the same <json header><binary frame> ZMQ message pair that
// slsReceiver sends, so jf_online_display_gui can be exercised without hardware.
//
// Every 3rd frame carries NO signal; the other frames carry a 40-pixel
// cluster at ADC 300 on a baseline of 100.  With
//     ./jf_online_display_gui 400 127.0.0.1 30777 --stats --pede /nonexistent
// the gate threshold is 200, so exactly 2/3 of the frames must pass.
//
//   make feeder
//   ./test/jf_feeder tcp://*:30777 200 20 &
//   ./jf_online_display_gui 400 127.0.0.1 30777 --stats --pede /nonexistent
//
// Subscriber mode (counts the frames republished by --pub):
//   ./test/jf_feeder sub tcp://127.0.0.1:30778 8
//
// 4th positional arg = initial silent period in seconds (idle-receiver test).
// ===========================================================================
#include <zmq.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

namespace {

int runSubscriber(const char *addr, int seconds) {
    void *ctx = zmq_ctx_new();
    void *sub = zmq_socket(ctx, ZMQ_SUB);
    int rcvtimeo = 200;
    zmq_setsockopt(sub, ZMQ_RCVTIMEO, &rcvtimeo, sizeof(rcvtimeo));
    zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "", 0);
    if (zmq_connect(sub, addr) != 0) {
        fprintf(stderr, "connect %s failed: %s\n", addr,
                zmq_strerror(zmq_errno()));
        return 1;
    }
    printf("subscriber: connected to %s for %d s\n", addr, seconds);
    fflush(stdout);

    std::vector<char> data(2 * 1024 * 1024);
    char hdr[16384];
    int frames = 0, endMarkers = 0;
    int dumped = 0;
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    while (std::chrono::steady_clock::now() < deadline) {
        const int hn = zmq_recv(sub, hdr, sizeof(hdr) - 1, 0);
        if (hn < 0)
            continue;
        hdr[hn] = '\0';
        const bool isData = (strstr(hdr, "\"data\":1") != nullptr);
        if (isData)
            frames++;
        else if (strstr(hdr, "\"data\":0") != nullptr)
            endMarkers++;
        int more = 0;
        size_t moreSize = sizeof(more);
        zmq_getsockopt(sub, ZMQ_RCVMORE, &more, &moreSize);
        int dn = -1;
        if (more)
            dn = zmq_recv(sub, data.data(), data.size(), 0);
        if (dumped < 3) {
            dumped++;
            char head[400];
            snprintf(head, sizeof(head), "%.399s", hdr);
            printf("subscriber: hdr_len=%d data_len=%d more=%d head=%s\n", hn,
                   dn, more, head);
            fflush(stdout);
        }
    }
    printf("subscriber: received %d data frames, %d end markers\n", frames,
           endMarkers);
    fflush(stdout);
    zmq_close(sub);
    zmq_ctx_term(ctx);
    return frames > 0 ? 0 : 1;
}

// Dual-interface stand-in: two PUB sockets on base and base+1, each sending
// one 1024x256 Y-slab of the same frame (the slsReceiver multi-UDP-interface
// layout).  The bottom slab is filled with botVal, the top slab with topVal,
// so the assembled image can be verified from the gate counters.
//   ./test/jf_feeder dual <basePort> <nframes> <fps> [delaySec] [botVal]
//                       [topVal] [topLagFrames]
int runDual(const char *baseStr, int nframes, int fps, int delayMs, int botVal,
            int topVal, int topLag) {
    const int base = atoi(baseStr);
    const int W = 1024, H = 256;
    const int bytes = W * H * 2;

    void *ctx = zmq_ctx_new();
    void *bot = zmq_socket(ctx, ZMQ_PUB);
    void *top = zmq_socket(ctx, ZMQ_PUB);
    int hwm = 1000;
    zmq_setsockopt(bot, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    zmq_setsockopt(top, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    const std::string a0 = "tcp://*:" + std::to_string(base);
    const std::string a1 = "tcp://*:" + std::to_string(base + 1);
    if (zmq_bind(bot, a0.c_str()) != 0 || zmq_bind(top, a1.c_str()) != 0) {
        fprintf(stderr, "dual bind failed: %s\n", zmq_strerror(zmq_errno()));
        return 1;
    }
    printf("feeder: dual mode on %s and %s, %d frames at %d fps "
           "(bot=%d top=%d topLag=%d)\n",
           a0.c_str(), a1.c_str(), nframes, fps, botVal, topVal, topLag);
    fflush(stdout);

    std::vector<short> slab(W * H);
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

    auto sendSlab = [&](void *sock, int value, int frameNo) {
        for (size_t i = 0; i < slab.size(); i++)
            slab[i] = static_cast<short>(value);
        char hdr[512];
        // detSpec3 bits 8-11 carry the storage cell; cycle 0..15 so the SC
        // filter can be exercised end to end
        const unsigned detSpec3 = ((unsigned)(frameNo % 16) & 0xF) << 8;
        const int hn = snprintf(
            hdr, sizeof(hdr),
            "{\"jsonversion\":5,\"data\":1,\"size\":%d,"
            "\"shape\":[%d,%d],\"frameNumber\":%d,\"frameIndex\":%d,"
            "\"detSpec1\":0,\"detSpec3\":%u}",
            bytes, W, H, frameNo, frameNo, detSpec3);
        zmq_send(sock, hdr, hn, ZMQ_SNDMORE);
        zmq_send(sock, slab.data(), bytes, 0);
    };

    for (int f = 0; f < nframes; f++) {
        sendSlab(bot, botVal, f);
        const int topFrame = (f - topLag >= 0) ? f - topLag : f;
        sendSlab(top, topVal, topFrame);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(1000 / (fps > 0 ? fps : 1)));
    }
    const char *endhdr = "{\"jsonversion\":5,\"data\":0}";
    zmq_send(bot, endhdr, strlen(endhdr), 0);
    zmq_send(top, endhdr, strlen(endhdr), 0);
    printf("feeder: dual done\n");
    fflush(stdout);
    zmq_close(bot);
    zmq_close(top);
    zmq_ctx_term(ctx);
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    if (argc > 1 && std::string(argv[1]) == "sub") {
        const char *addr = (argc > 2) ? argv[2] : "tcp://127.0.0.1:30778";
        const int seconds = (argc > 3) ? atoi(argv[3]) : 8;
        return runSubscriber(addr, seconds);
    }
    if (argc > 1 && std::string(argv[1]) == "dual") {
        return runDual(argc > 2 ? argv[2] : "30777", argc > 3 ? atoi(argv[3]) : 60,
                       argc > 4 ? atoi(argv[4]) : 25,
                       argc > 5 ? atoi(argv[5]) * 1000 : 800,
                       argc > 6 ? atoi(argv[6]) : 100,
                       argc > 7 ? atoi(argv[7]) : 150,
                       argc > 8 ? atoi(argv[8]) : 0);
    }

    const char *addr = (argc > 1) ? argv[1] : "tcp://*:30777";
    const int nframes = (argc > 2) ? atoi(argv[2]) : 200;
    const int fps = (argc > 3) ? atoi(argv[3]) : 20;
    // optional 4th arg: stay bound and connected but SILENT for N seconds
    // first (checks that an idle-but-alive receiver is not called "stopped")
    const int delayMs = (argc > 4) ? atoi(argv[4]) * 1000 : 800;
    const int NCH = 524288;

    void *ctx = zmq_ctx_new();
    void *pub = zmq_socket(ctx, ZMQ_PUB);
    int hwm = 1000;
    zmq_setsockopt(pub, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    if (zmq_bind(pub, addr) != 0) {
        fprintf(stderr, "bind %s failed: %s\n", addr,
                zmq_strerror(zmq_errno()));
        return 1;
    }
    printf("feeder: bound %s, sending %d frames at %d fps\n", addr, nframes,
           fps);
    fflush(stdout);

    std::vector<short> frame(NCH);
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

    // detSpec1 packs the receiver ROI as 4 x uint16: xmin|xmax|ymin|ymax
    const unsigned long long detSpec1 = ((unsigned long long)10u << 48) |
                                        ((unsigned long long)400u << 32) |
                                        ((unsigned long long)20u << 16) | 300u;

    for (int f = 0; f < nframes; f++) {
        const bool signal = (f % 3) != 0;
        for (int i = 0; i < NCH; i++)
            frame[i] = 100; // baseline, gain 0
        if (signal)
            for (int k = 0; k < 40; k++)
                frame[1000 + k] = 300; // 40 px above the 200 threshold

        char hdr[512];
        const int hn = snprintf(
            hdr, sizeof(hdr),
            "{\"jsonversion\":5,\"data\":1,\"size\":%d,\"detSpec1\":%llu,"
            "\"frameIndex\":%d,\"bitmode\":1,\"fileIndex\":0,"
            "\"detshape\":[1,1],\"shape\":[1024,512],\"acqIndex\":%d,"
            "\"frameNumber\":%d,\"expLength\":0,\"packetNumber\":0,"
            "\"timestamp\":0,\"modId\":0,\"row\":0,\"column\":0,\"detSpec2\":0,"
            "\"detSpec3\":%u,\"detSpec4\":0,\"detType\":0,\"version\":0,"
            "\"flipRows\":0,\"quad\":0,\"completeImage\":1}",
            NCH * 2, detSpec1, f, f, f,
            ((unsigned)(f % 16) & 0xF) << 8);
        zmq_send(pub, hdr, hn, ZMQ_SNDMORE);
        zmq_send(pub, frame.data(), NCH * 2, 0);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(1000 / (fps > 0 ? fps : 1)));
    }

    const char *endhdr = "{\"jsonversion\":5,\"data\":0}";
    zmq_send(pub, endhdr, strlen(endhdr), 0);
    printf("feeder: done\n");
    fflush(stdout);
    zmq_close(pub);
    zmq_ctx_term(ctx);
    return 0;
}
