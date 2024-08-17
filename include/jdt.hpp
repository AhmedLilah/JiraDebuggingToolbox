#pragma once

#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <vector>
#include <exception>
#include <utility>
#include <queue>
#include <filesystem>

namespace jdt {

enum class CLAOptions {
        InputFileName,
        Errors,
        Warnings,
        Options,
        Commandline,
        Analyses,
        InfoCard,
        Timings,
        Summary,
        Version,
        OutputFileName,
};

class CLA {
public:
        // Handle Commandline Arguments
        auto handleArg(std::deque<std::string>& CLADeque) -> bool
        {
                auto inputArg { CLADeque.front() };
                for (auto& [arg, a, usageState, acceptData, data, b] : CLATuple) {
                        if (arg != inputArg)
                                continue;
                        usageState = true;
                        CLADeque.pop_front();
                        if (not acceptData)
                                return true;
                        if (CLADeque.size() > 0) {
                                data = CLADeque.front();
                                CLADeque.pop_front();
                                return true;
                        }
                        std::cerr << std::format("Error: expects argument data after {}.\n", inputArg);
                        return true;
                }
                std::cerr << std::format("Error: Invalid argument \"{}\".\n", inputArg);
                std::terminate();
        }

        // Get the usage state for a given CLA Option
        auto getUsageState(CLAOptions inputArg) const -> bool
        {
                for (auto& [arg, a, usageState, b, c, enumType] : CLATuple) {
                        if (inputArg == enumType)
                                return usageState;
                }
        }

        // Get the ouput names of the info sections that will be parsed 
        auto getOuputName(CLAOptions inputArg) const -> std::string
        {
                for (auto& [arg, outputName, a, b, c, enumType] : CLATuple) {
                        if (inputArg == enumType)
                                return outputName;
                }
        }

        // Get the argument data for a given CLA Option
        auto getArgData(CLAOptions inputArg) const -> std::string
        {
                for (auto& [arg, a, b, c, data, enumType] : CLATuple) {
                        if (inputArg == enumType)
                                return data;
                }
        }

        // Set the input file name 
        auto setInputFileName(std::deque< std::string> & CLADeque) -> bool
        {
                for (auto& [a, b, c, d, data, enumType] : CLATuple) {
                        if (enumType == CLAOptions::InputFileName) {
                                data = CLADeque.front();
                                CLADeque.pop_front();
                                break;
                        }
                }
                return true;
        }

        // Get a vector of CLA Options in the order of the output they should be put in if printed to screen 
        auto getOutputOrder() const -> std::vector<CLAOptions>
        {
                return {
                        CLAOptions::Version,
                        CLAOptions::InfoCard,
                        CLAOptions::Commandline,
                        CLAOptions::Options,
                        CLAOptions::Analyses,
                        CLAOptions::Summary,
                        CLAOptions::Errors,
                        CLAOptions::Warnings,
                        CLAOptions::Timings,
                };
        }


private:
        // vector of tuples that containt all the state neede for CLA parsing
        std::vector<std::tuple<const std::string, const std::string, bool, const bool, std::string, const CLAOptions>> CLATuple = {
                // {cla, fullName, usageState, argdata, dataAcceptionState, CLAOptionsEnum}
                { {},   "inputFileName",        false, false, {}, CLAOptions::InputFileName },
                { "-e", "Errors",               false, false, {}, CLAOptions::Errors },
                { "-w", "Warnings",             false, false, {}, CLAOptions::Warnings },
                { "-o", "Options",              false, false, {}, CLAOptions::Options },
                { "-c", "Commandline",          false, false, {}, CLAOptions::Commandline },
                { "-a", "Analyses",             false, false, {}, CLAOptions::Analyses },
                { "-i", "InfoCard",             false, false, {}, CLAOptions::InfoCard },
                { "-t", "Timings",              false, false, {}, CLAOptions::Timings },
                { "-s", "Summary",              false, false, {}, CLAOptions::Summary },
                { "-v", "Version",              false, false, {}, CLAOptions::Version },
                { "-O", "OutputFileName",       false, true,  {}, CLAOptions::OutputFileName }
        };
};


// Vector containing the regexes for every info section needed
// @TODO: this should be moved to clas and should be initalised from an external file to facilitate ease of modification.
const std::vector<std::pair<const CLAOptions, std::regex>> optionsToRegex = {
        {CLAOptions::Errors,      std::regex("(SIM_ERROR.*\n((( \t)|(\t)){1,}.*\n)*)|(SIM_ERROR.*\n)")},
        {CLAOptions::Warnings,    std::regex("(SIM_WARNING.*\n((( \t)|(\t)){1,}.*\n)*)|(SIM_WARNING.*\n)")},
        {CLAOptions::Options,     std::regex("[\*]{1,} Options (Used|Ignored).*\n( .*\n)*\n")},
        {CLAOptions::Commandline, std::regex(".*Command-Line Options Used.*\n.*\n")},
        {CLAOptions::Analyses,    std::regex("")},
        {CLAOptions::InfoCard,    std::regex("[ ][\*]{60,}DEVELOPER\n(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)[ ][\*]{60,}\n")},
        {CLAOptions::Timings,     std::regex("")},
        {CLAOptions::Summary,     std::regex(".*SIMULATION finished with.*\n(.*\n)*")}
};


// main program flow functions
auto parseCommandlineArgs(int argc, char** argv) -> CLA;
auto parseLogFile(std::string&, CLA&) -> std::vector<std::pair<CLAOptions, std::string>>;
auto outputResults(std::vector<std::pair<CLAOptions, std::string>> &, CLA &) -> bool;

// support functions
auto openInputFile(std::string &, CLA &) -> std::ifstream;
auto convertInputFileToString(std::ifstream &) -> std::string;
auto createOutputFile(std::filesystem::path &) -> std::ofstream;
auto printOutputToScreen(std::vector<std::pair<CLAOptions, std::string>> &, CLA &) -> bool;
auto writeToOutputFile(std::ofstream &, std::string const &) -> void;

// parsing function
auto findPatern(std::regex &, std::string &) -> std::string;
}
