#pragma once

#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <vector>
#include <exception>
#include <stdexcept>
#include <utility>
#include <queue>
#include <filesystem>

namespace jdt {
class CLA {

public:
        enum class Options {
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

public:
        // Handle Commandline Arguments
        inline auto handleArg(std::deque<std::string>& CLADeque) -> bool
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
        inline auto getUsageState(Options inputArg) const -> bool
        {
                for (auto& [arg, a, usageState, b, c, enumType] : CLATuple) {
                        if (inputArg == enumType)
                                return usageState;
                }
        }

        // Get the ouput names of the info sections that will be parsed 
        inline auto getOuputName(Options inputArg) const -> std::string
        {
                for (auto& [arg, outputName, a, b, c, enumType] : CLATuple) {
                        if (inputArg == enumType)
                                return outputName;
                }
        }

        // Get the argument data for a given CLA Option
        inline auto getArgData(Options inputArg) const -> std::string
        {
                for (auto& [arg, a, b, c, data, enumType] : CLATuple) {
                        if (inputArg == enumType)
                                return data;
                }
        }

        // Set the input file name 
        inline auto setInputFileName(std::deque< std::string> & CLADeque) -> bool
        {
                for (auto& [a, b, c, d, data, enumType] : CLATuple) {
                        if (enumType == Options::InputFileName) {
                                data = CLADeque.front();
                                CLADeque.pop_front();
                                break;
                        }
                }
                return true;
        }

        // Get a vector of CLA Options in the order of the output they should be put in if printed to screen 
        inline auto getOutputOrder() const -> std::vector<Options>
        {
                return {
                        Options::Version,
                        Options::InfoCard,
                        Options::Commandline,
                        Options::Options,
                        Options::Analyses,
                        Options::Summary,
                        Options::Errors,
                        Options::Warnings,
                        Options::Timings,
                };
        }


private:
        // vector of tuples that containt all the state neede for CLA parsing
        std::vector<std::tuple<const std::string, const std::string, bool, const bool, std::string, const Options>> CLATuple = {
                // {cla, fullName, usageState, argdata, dataAcceptionState, CLAOptionsEnum}
                { {},   "inputFileName",        false, false, {}, Options::InputFileName },
                { "-e", "Errors",               false, false, {}, Options::Errors },
                { "-w", "Warnings",             false, false, {}, Options::Warnings },
                { "-o", "Options",              false, false, {}, Options::Options },
                { "-c", "Commandline",          false, false, {}, Options::Commandline },
                { "-a", "Analyses",             false, false, {}, Options::Analyses },
                { "-i", "InfoCard",             false, false, {}, Options::InfoCard },
                { "-t", "Timings",              false, false, {}, Options::Timings },
                { "-s", "Summary",              false, false, {}, Options::Summary },
                { "-v", "Version",              false, false, {}, Options::Version },
                { "-O", "OutputFileName",       false, true,  {}, Options::OutputFileName }
        };
};

// main program flow functions
inline auto parseCommandlineArgs(int argc, char** argv) -> CLA;
inline auto parseLogFile(std::string&, CLA&) -> std::vector<std::pair<CLA::Options, std::string>>;
inline auto outputResults(std::vector<std::pair<CLA::Options, std::string>> &, CLA &) -> bool;

// support functions
static inline auto openInputFile(std::string &, CLA &) -> std::ifstream;
static inline auto convertInputFileToString(std::ifstream &) -> std::string;
static inline auto createOutputFile(std::filesystem::path &) -> std::ofstream;
static inline auto printOutputToScreen(std::vector<std::pair<CLA::Options, std::string>> &, CLA &) -> bool;
static inline auto writeToOutputFile(std::ofstream &, std::string const &) -> void;

// parsing function
inline auto findPatern(std::regex &, std::string &) -> std::string;

}



namespace jdt {

// Vector containing the regexes for every info section needed
// @TODO: this should be moved to clas and should be initalised from an external file to facilitate ease of modification.
const std::vector<std::pair<const CLA::Options, std::regex>> optionsToRegex = {
        {CLA::Options::Errors,      std::regex("(SIM_ERROR.*\n((( \t)|(\t)){1,}.*\n)*)|(SIM_ERROR.*\n)")},
        {CLA::Options::Warnings,    std::regex("(SIM_WARNING.*\n((( \t)|(\t)){1,}.*\n)*)|(SIM_WARNING.*\n)")},
        {CLA::Options::Options,     std::regex("[\*]{1,} Options (Used|Ignored).*\n( .*\n)*\n")},
        {CLA::Options::Commandline, std::regex(".*Command-Line Options Used.*\n.*\n")},
        {CLA::Options::Analyses,    std::regex("")},
        {CLA::Options::InfoCard,    std::regex("[ ][\*]{60,}DEVELOPER\n(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)(.*\n)[ ][\*]{60,}\n")},
        {CLA::Options::Timings,     std::regex("")},
        {CLA::Options::Summary,     std::regex(".*SIMULATION finished with.*\n(.*\n)*")}
};


inline auto openInputFile(std::string& inputFileName, CLA & cla) -> std::ifstream {
        if (inputFileName.empty()) {
                std::cerr << "Error: missing input file name\n"
                        "Usage: program_name [options] input_file\n";
                std::terminate();
        }
        std::ifstream inputFile(std::filesystem::current_path() / inputFileName);
        if (inputFile)
                return inputFile;
        if (cla.getUsageState(CLA::Options::Version))
                return {};
        std::cerr << std::format("Error: \"{}{}\" does not exist.", std::filesystem::current_path().string(), inputFileName);
        std::terminate();
}


inline auto convertInputFileToString(std::ifstream& inputFile) -> std::string {
        std::string inputString(std::istreambuf_iterator(inputFile), {});
        return inputString;
}


inline auto createOutputFile(std::filesystem::path & outputFilePath) -> std::ofstream 
{
        std::ofstream outputFile(outputFilePath);
        if (outputFile)
                return outputFile;
        std::cerr << std::format("Error: coulden't creat the output file: {}", outputFilePath.string());
        std::terminate();
}


inline auto writeToOutputFile(std::ofstream& outputFile, std::string const & outputString) -> void {
        outputFile << outputString;
        outputFile.close();
}


// parsing functions
inline auto findPatern(std::regex & rgx, std::string& inputString) -> std::string {
        std::string foundString;
        if (std::regex_search(inputString, rgx)) {
                auto begin = std::sregex_iterator(inputString.begin(), inputString.end(), rgx);
                auto end = std::sregex_iterator();
                for (auto itr = begin; itr != end; ++itr) {
                        foundString.append((*itr).str());
                }
        }
        return foundString;
}


inline auto parseCommandlineArgs(int argc, char** argv) -> CLA {
        std::deque<std::string> argsDeque;
        for (int i{}; i < argc; ++i) {
                argsDeque.push_back(std::string(argv[i]));
        }
        argsDeque.pop_front();

        CLA cla;
        while (argsDeque.size() > 0) {
                auto arg = argsDeque.front();
                if (arg.find("-") == 0) {
                        cla.handleArg(argsDeque);
                } else  cla.setInputFileName(argsDeque);
        }
        return cla;
}


inline auto parseLogFile(std::string& inputString, CLA& cla) -> std::vector<std::pair<CLA::Options, std::string>> {
        std::vector<std::pair<CLA::Options, std::string>> outputsVect;
        if (cla.getUsageState(CLA::Options::Version)) {
                std::string versionData {
                        "JDT : Jira Debugging Toolbox\n"
                        "Version : 0.0.8.0\n"
                        "Date : "  __DATE__  "\n"
                        "Developer : Ahmed S. Lilah\n"
                        "Contacts : ahmed.lilah.ext@siemens.com\n"
                };
                outputsVect.push_back({ CLA::Options::Version, versionData});
                return outputsVect;
        }
        for (auto [option, rgx] : optionsToRegex) {
                if (cla.getUsageState(option)) {
                        try {
                                outputsVect.push_back({ option, findPatern(rgx, inputString) });
                        }
                        catch (std::exception& e) {
                                std::cerr << e.what() << '\n';
                        }
                }
        }
        return outputsVect;
}


inline auto printOutputToScreen(std::vector<std::pair<CLA::Options, std::string>> & outputsVect, CLA & cla) -> bool {
        std::string outputString;
        for (auto option : cla.getOutputOrder()) {
                auto getDataFromVect = [&outputsVect = outputsVect](CLA::Options option) -> std::string & {
                        for (auto & [o, d] : outputsVect) {
                                if (option == o) {
                                        return d;
                                        break;
                                }
                        }
                };
                if (not cla.getUsageState(option))
                        continue;
                if (option == CLA::Options::Version) {
                        outputString.append(getDataFromVect(option));
                        return (std::cout << outputString).good();
                }
                if (auto data = getDataFromVect(option); not data.empty())
                        outputString.append("########################################[[" + cla.getOuputName(option) + " ]]]]########################################\n" + data + "\n\n\n");
        }
        return (std::cout << outputString).good();
}


inline auto outputResults(std::vector<std::pair<CLA::Options, std::string>> & outputsVect, CLA & cla) -> bool {
        if (bool goingToFile = cla.getUsageState(CLA::Options::OutputFileName); goingToFile) {
                auto const & outputDirectoryPath {std::filesystem::current_path()/cla.getArgData(CLA::Options::OutputFileName)};
                std::filesystem::create_directory(outputDirectoryPath);
                for (auto const & [option, outputString] : outputsVect) {
                        std::string outputFileName = cla.getOuputName(option) + ".txt";
                        auto tempOutputFilePath {outputDirectoryPath / outputFileName};
                        std::ofstream outputFile{ createOutputFile(tempOutputFilePath) };
                        writeToOutputFile(outputFile, outputString);
                }
                return true;
        } 
        else [[likely]]{
                printOutputToScreen(outputsVect, cla);
                return true;
        }
        return false;
}
}        
