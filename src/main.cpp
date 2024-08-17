#include "../include/jdt.hpp"

int main(int argc, char **argv) {
	auto tic { std::chrono::high_resolution_clock::now() };
	auto cla { jdt::parseCommandlineArgs(argc, argv) };
        auto inputFileName {cla.getArgData(jdt::CLAOptions::InputFileName)};
	auto inputFile { jdt::openInputFile(inputFileName, cla)};
	auto inputString { jdt::convertInputFileToString(inputFile) };
	auto outputsVect { jdt::parseLogFile(inputString, cla)};
	jdt::outputResults(outputsVect, cla);
	auto toc = std::chrono::high_resolution_clock::now();
	auto duration = toc - tic;
	std::cout << "Requests served in : " << duration.count() * 1e-9 << " sec.\n";
}
