#include "inferAction.h"

#include "sls/sls_detector_defs.h"

namespace sls {

int InferAction::infer(sls::CmdParser &parser, std::ostream &os) {

    args = parser.arguments();

    cmd = parser.command();

    auto it = functions.find(parser.command());

    if (it != functions.end()) {

        return ((*this).*(it->second))();

    } else {

        throw RuntimeError(

            "sls_detector not implemented for command: " + parser.command() +

            ". Use sls_detector_get or sls_detector_put.");
    }
}

int InferAction::acquire() {

    if (args.size() == 0) {
        throw RuntimeError(
            "sls_detector is disabled for command: acquire with number of "
            "arguments 0. Use sls_detector_get or sls_detector_put");
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::activate() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::adcclk() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::adcenable() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::adcenable10g() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::adcindex() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::adcinvert() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::adclist() {

    throw RuntimeError("sls_detector is disabled for command: adclist. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::adcname() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::adcphase() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: adcphase with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::adcpipeline() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::adcreg() {

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::adcvpp() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: adcvpp with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::apulse() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::asamples() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::autocompdisable() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::badchannels() {

    throw RuntimeError("sls_detector is disabled for command: badchannels. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::blockingtrigger() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::burstmode() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::burstperiod() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: burstperiod with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::bursts() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::burstsl() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::bustest() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::cdsgain() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::chipversion() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::clearbit() {

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::clearbusy() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::clearroi() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::clientversion() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::clkdiv() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::clkfreq() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::clkphase() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        throw RuntimeError(
            "sls_detector is disabled for command: clkphase with number of "
            "arguments 2. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::collectionmode() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::column() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::compdisabletime() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: compdisabletime with number "
            "of arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::confadc() {

    if (args.size() == 2) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::config() {

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::configtransceiver() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::counters() {

    throw RuntimeError("sls_detector is disabled for command: counters. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::currentsource() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 4) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::dac() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        throw RuntimeError(
            "sls_detector is disabled for command: dac with number of "
            "arguments 2. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::dacindex() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::daclist() {

    throw RuntimeError("sls_detector is disabled for command: daclist. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::dacname() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::dacvalues() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::datastream() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::dbitclk() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::dbitphase() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: dbitphase with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::dbitpipeline() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::defaultdac() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        throw RuntimeError(
            "sls_detector is disabled for command: defaultdac with number of "
            "arguments 2. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::defaultpattern() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::delay() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: delay with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::delayl() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::detectorserverversion() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::detsize() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::diodelay() {

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::dpulse() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::dr() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::drlist() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::dsamples() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::execcommand() {

    throw RuntimeError("sls_detector is disabled for command: execcommand. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::exptime() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: exptime with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::exptime1() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: exptime1 with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::exptime2() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: exptime2 with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::exptime3() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: exptime3 with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::exptimel() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::extrastoragecells() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::extsampling() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::extsamplingsrc() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::extsig() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::fformat() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::filtercells() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::filterresistor() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::findex() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::firmwaretest() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::firmwareversion() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::fliprows() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::flowcontrol10g() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::fmaster() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::fname() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::foverwrite() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::fpath() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::framecounter() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::frames() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::framesl() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::frametime() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::free() {

    if (args.size() == 0) {
        throw RuntimeError(
            "sls_detector is disabled for command: free with number of "
            "arguments 0. Use sls_detector_get or sls_detector_put");
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::fwrite() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::gaincaps() {

    throw RuntimeError("sls_detector is disabled for command: gaincaps. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::gainmode() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::gappixels() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::gatedelay() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: gatedelay with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::gatedelay1() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: gatedelay1 with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::gatedelay2() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: gatedelay2 with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::gatedelay3() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: gatedelay3 with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::gates() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::getbit() {

    if (args.size() == 2) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::hardwareversion() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::highvoltage() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::hostname() {

    throw RuntimeError("sls_detector is disabled for command: hostname. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::im_a() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::im_b() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::im_c() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::im_d() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::im_io() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::imagetest() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::initialchecks() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::inj_ch() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::interpolation() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::interruptsubframe() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::kernelversion() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::lastclient() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::led() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::lock() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::master() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::maxadcphaseshift() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::maxclkphaseshift() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::maxdbitphaseshift() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::measuredperiod() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::measuredsubperiod() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::moduleid() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::nextframenumber() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::nmod() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::numinterfaces() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::overflow() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::packageversion() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::parallel() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::parameters() {

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::partialreset() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::patfname() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::patioctrl() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::patlimits() {

    throw RuntimeError("sls_detector is disabled for command: patlimits. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patloop() {

    throw RuntimeError("sls_detector is disabled for command: patloop. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patloop0() {

    throw RuntimeError("sls_detector is disabled for command: patloop0. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patloop1() {

    throw RuntimeError("sls_detector is disabled for command: patloop1. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patloop2() {

    throw RuntimeError("sls_detector is disabled for command: patloop2. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patmask() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::patnloop() {

    throw RuntimeError("sls_detector is disabled for command: patnloop. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patnloop0() {

    throw RuntimeError("sls_detector is disabled for command: patnloop0. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patnloop1() {

    throw RuntimeError("sls_detector is disabled for command: patnloop1. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patnloop2() {

    throw RuntimeError("sls_detector is disabled for command: patnloop2. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patsetbit() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::pattern() {

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::patternstart() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::patwait() {

    throw RuntimeError("sls_detector is disabled for command: patwait. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patwait0() {

    throw RuntimeError("sls_detector is disabled for command: patwait0. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patwait1() {

    throw RuntimeError("sls_detector is disabled for command: patwait1. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patwait2() {

    throw RuntimeError("sls_detector is disabled for command: patwait2. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patwaittime() {

    throw RuntimeError("sls_detector is disabled for command: patwaittime. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::patwaittime0() {

    throw RuntimeError("sls_detector is disabled for command: patwaittime0. "
                       "Use sls_detector_get or sls_detector_put");
}

int InferAction::patwaittime1() {

    throw RuntimeError("sls_detector is disabled for command: patwaittime1. "
                       "Use sls_detector_get or sls_detector_put");
}

int InferAction::patwaittime2() {

    throw RuntimeError("sls_detector is disabled for command: patwaittime2. "
                       "Use sls_detector_get or sls_detector_put");
}

int InferAction::patword() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::pedestalmode() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::period() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: period with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::periodl() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::polarity() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::port() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::powerchip() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::powerindex() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::powerlist() {

    throw RuntimeError("sls_detector is disabled for command: powerlist. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::powername() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::powervalues() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::programfpga() {

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::pulse() {

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::pulsechip() {

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::pulsenmove() {

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::pumpprobe() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::quad() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::ratecorr() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::readnrows() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::readout() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::readoutspeed() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::readoutspeedlist() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rebootcontroller() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::reg() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::resetdacs() {

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::resetfpga() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::roi() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::romode() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::row() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::runclk() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::runtime() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_arping() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_clearroi() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_dbitlist() {

    throw RuntimeError("sls_detector is disabled for command: rx_dbitlist. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::rx_dbitoffset() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_discardpolicy() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_fifodepth() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_frameindex() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_framescaught() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_framesperfile() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_hostname() {

    throw RuntimeError("sls_detector is disabled for command: rx_hostname. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::rx_jsonaddheader() {

    throw RuntimeError(
        "sls_detector is disabled for command: rx_jsonaddheader. Use "
        "sls_detector_get or sls_detector_put");
}

int InferAction::rx_jsonpara() {

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: rx_jsonpara with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_lastclient() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_lock() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_missingpackets() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_padding() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_printconfig() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_realudpsocksize() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_roi() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 4) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_silent() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_start() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_status() {

    throw RuntimeError("sls_detector is disabled for command: rx_status. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::rx_stop() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_tcpport() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_threads() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_udpsocksize() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_version() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_zmqfreq() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_zmqhwm() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_zmqip() {

    throw RuntimeError("sls_detector is disabled for command: rx_zmqip. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::rx_zmqport() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_zmqstartfnum() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::rx_zmqstream() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::samples() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::savepattern() {

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::scan() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 4) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 5) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::scanerrmsg() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::selinterface() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::serialnumber() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::setbit() {

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::settings() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::settingslist() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::settingspath() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::signalindex() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::signallist() {

    throw RuntimeError("sls_detector is disabled for command: signallist. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::signalname() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::sleep() {

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::slowadc() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::slowadcindex() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::slowadclist() {

    throw RuntimeError("sls_detector is disabled for command: slowadclist. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::slowadcname() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::slowadcvalues() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::start() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::status() {

    throw RuntimeError("sls_detector is disabled for command: status. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::stop() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::stopport() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::storagecell_delay() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: storagecell_delay with "
            "number of arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::storagecell_start() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::subdeadtime() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: subdeadtime with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::subexptime() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: subexptime with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::sync() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::syncclk() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_10ge() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_adc() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_control() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_dcdc() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_event() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_fpga() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_fpgaext() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_fpgafl() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_fpgafr() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_slowadc() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_sodl() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_sodr() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::temp_threshold() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::templist() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::tempvalues() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::tengiga() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::threshold() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 3) {
        return slsDetectorDefs::PUT_ACTION;
    }

    if (args.size() == 4) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::timing() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::timing_info_decoder() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::timinglist() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::timingsource() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::top() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::transceiverenable() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::trigger() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::triggers() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::triggersl() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::trimbits() {

    if (args.size() == 1) {
        throw RuntimeError(
            "sls_detector is disabled for command: trimbits with number of "
            "arguments 1. Use sls_detector_get or sls_detector_put");
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::trimen() {

    throw RuntimeError("sls_detector is disabled for command: trimen. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::trimval() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::tsamples() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::txdelay() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::txdelay_frame() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::txdelay_left() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::txdelay_right() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::type() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_cleardst() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_dstip() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_dstip2() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_dstlist() {

    throw RuntimeError("sls_detector is disabled for command: udp_dstlist. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::udp_dstmac() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_dstmac2() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_dstport() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_dstport2() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_firstdst() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_numdst() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_reconfigure() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_srcip() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_srcip2() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_srcmac() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_srcmac2() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::udp_validate() {

    if (args.size() == 0) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::update() {

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::updatedetectorserver() {

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::updatekernel() {

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::updatemode() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::user() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::v_a() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::v_b() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::v_c() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::v_chip() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::v_d() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::v_io() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::v_limit() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vchip_comp_adc() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vchip_comp_fe() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vchip_cs() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vchip_opa_1st() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vchip_opa_fd() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vchip_ref_comp_fe() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::versions() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::veto() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vetoalg() {

    if (args.size() == 1) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vetofile() {

    throw RuntimeError("sls_detector is disabled for command: vetofile. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::vetophoton() {

    if (args.size() == 2) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 4) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vetoref() {

    throw RuntimeError("sls_detector is disabled for command: vetoref. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::vetostream() {

    throw RuntimeError("sls_detector is disabled for command: vetostream. Use "
                       "sls_detector_get or sls_detector_put");
}

int InferAction::virtualFunction() {

    if (args.size() == 2) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vm_a() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vm_b() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vm_c() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vm_d() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::vm_io() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::zmqhwm() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::zmqip() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

int InferAction::zmqport() {

    if (args.size() == 0) {
        return slsDetectorDefs::GET_ACTION;
    }

    if (args.size() == 1) {
        return slsDetectorDefs::PUT_ACTION;
    }

    else {

        throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
}

} // namespace sls
