#include "../include/jdt.hpp"

namespace jdt {
auto openInputFile(std::string& inputFileName, CLA & cla) -> std::ifstream {
        if (inputFileName.empty()) {
                std::cerr << "Error: missing input file name\n"
                        "Usage: program_name [options] input_file\n";
                std::terminate();
        }
        std::ifstream inputFile(std::filesystem::current_path() / inputFileName);
        if (inputFile)
                return inputFile;
        if (cla.getUsageState(CLAOptions::Version))
                return {};
        std::cerr << std::format("Error: \"{}{}\" does not exist.", std::filesystem::current_path().string(), inputFileName);
        std::terminate();
}


auto convertInputFileToString(std::ifstream& inputFile) -> std::string {
        std::string inputString(std::istreambuf_iterator(inputFile), {});
        return inputString;
}


auto createOutputFile(std::filesystem::path & outputFilePath) -> std::ofstream 
{
        std::ofstream outputFile(outputFilePath);
        if (outputFile)
                return outputFile;
        std::cerr << std::format("Error: coulden't creat the output file: {}", outputFilePath.string());
        std::terminate();
}


auto writeToOutputFile(std::ofstream& outputFile, std::string const & outputString) -> void {
        outputFile << outputString;
        outputFile.close();
}


// parsing functions
auto findPatern(std::regex & rgx, std::string& inputString) -> std::string {
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


auto parseCommandlineArgs(int argc, char** argv) -> CLA {
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


auto parseLogFile(std::string& inputString, CLA& cla) -> std::vector<std::pair<CLAOptions, std::string>> {
        std::vector<std::pair<CLAOptions, std::string>> outputsVect;
        if (cla.getUsageState(CLAOptions::Version)) {
                std::string versionData {
                        "JDT : Jira Debugging Toolbox\n"
                        "Version : 0.0.8.0\n"
                        "Date : "  __DATE__  "\n"
                        "Developer : Ahmed S. Lilah\n"
                        "Contacts : ahmed.lilah.ext@siemens.com\n"
                };
                outputsVect.push_back({ CLAOptions::Version, versionData});
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


auto printOutputToScreen(std::vector<std::pair<CLAOptions, std::string>> & outputsVect, CLA & cla) -> bool {
        std::string outputString;
        for (auto option : cla.getOutputOrder()) {
                auto getDataFromVect = [&outputsVect = outputsVect](CLAOptions option) -> std::string & {
                        for (auto & [o, d] : outputsVect) {
                                if (option == o) {
                                        return d;
                                        break;
                                }
                        }
                };
                if (not cla.getUsageState(option))
                        continue;
                if (option == CLAOptions::Version) {
                        outputString.append(getDataFromVect(option));
                        return (std::cout << outputString).good();
                }
                if (auto data = getDataFromVect(option); not data.empty())
                        outputString.append("########################################[[" + cla.getOuputName(option) + " ]]]]########################################\n" + data + "\n\n\n");
        }
        return (std::cout << outputString).good();
}


auto outputResults(std::vector<std::pair<CLAOptions, std::string>> & outputsVect, CLA & cla) -> bool {
        if (bool goingToFile = cla.getUsageState(CLAOptions::OutputFileName); goingToFile) {
                auto const & outputDirectoryPath {std::filesystem::current_path()/cla.getArgData(CLAOptions::OutputFileName)};
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
