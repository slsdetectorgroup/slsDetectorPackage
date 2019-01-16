#ifndef CMD_LINE_PARSER_H
#define CMD_LINE_PARSER_H
#include <stdexcept>
#include <string>
#include <vector>

class CmdLineParser {
public:
    void Parse(int argc, char* argv[]);
    void Parse(std::string s);
    void Print();

    //getters
    int multi_id() const { return multi_id_; };
    int detector_id() const { return detector_id_; };
    std::string command() const { return command_; }
    const std::vector<std::string>& arguments() { return arguments_; };

private:
    void DecodeIdAndPosition(const char* c);
    int multi_id_ = 0;
    int detector_id_ = -1;
    std::string command_;
    std::vector<std::string> arguments_;
};

#endif // CMD_LINE_PARSER_H