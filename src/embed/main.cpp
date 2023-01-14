#include <filesystem>
#include <fstream>
#include <locale>

#include "argparse.hpp"
#include "include/embed.hpp"

namespace line {

static constexpr auto pragmaOnce = "#pragma once";

static constexpr auto includeEmbed = "#include <embed.hpp>";

static constexpr auto namespaceEmbedBegin = "namespace embedded {";
static constexpr auto namespaceEmbedEnd = "}  // namespace embedded";

static constexpr auto namespaceDataBegin = "namespace data {";
static constexpr auto namespaceDataEnd = "}  // namespace data";

static constexpr auto size1 = "inline constexpr std::size_t ";
static constexpr auto size2 = " = ";
static constexpr auto size3 = "_uz;";

static constexpr auto arrayBegin1 = "inline constexpr std::array<std::uint32_t, ";
static constexpr auto arrayBegin2 = "> ";
static constexpr auto arrayBegin3 = " = {";
static constexpr auto arrayEnd = "};";

static constexpr auto strBegin = "inline constexpr const char ";

static constexpr auto definition1 = "inline constexpr auto ";
static constexpr auto definition2Arr = " = detail::array::makeEmbedded<\n";
static constexpr auto definition2Str = " = detail::string::makeEmbedded<\n";
static constexpr auto definition3 = ">();";

static constexpr auto dataPrefix = "data::";

}  // namespace line

namespace {

bool validateVariableName(const std::string& variableName) {
    // Explicitly use C locale for std::isalpha.
    std::locale loc("C");

    const auto begin = variableName.begin();
    auto       it = begin;
    while (it != variableName.end()) {
        // Variable name can only contain letters, digits and underscores.
        if (!std::isalpha(*it, loc) && !std::isdigit(*it, loc) && *it != '_') {
            return false;
        }
        // Additionally, variable names can not start with a digit.
        if (it == begin && std::isdigit(*it, loc)) {
            return false;
        }
        ++it;
    }
    return true;
}

std::string transformVariableNameToHeaderGuard(const std::string& variableName) {
    // Explicitly use C locale for std::isalpha.
    std::locale loc("C");

    std::stringstream ss;

    auto itm1 = variableName.begin();
    auto it = variableName.begin() + 1;

    ss << std::toupper(variableName[0], loc);

    while (it != variableName.end()) {
        // Add an underscore between contiguous lower and upper case letters.
        if (*itm1 != '_' && std::islower(*itm1, loc) && std::isupper(*it, loc)) {
            ss << '_';
        }
        ss << std::toupper(*it, loc);  // Transform to upper case before adding.
        itm1 = it;
        ++it;
    }

    ss.flush();

    return ss.str();
}

}  // namespace

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program(argv[0], "1.0");

    program.add_argument("input").required().help("specify the input file");
    program.add_argument("output").required().help("specify the output file");
    program.add_argument("name").required().help("specify the variable name");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(EXIT_FAILURE);
    }

    const std::string inputFilePath = program.get("input");
    const std::string outputFilePath = program.get("output");
    const std::string variableName = program.get("name");

    // Check that the input file exists.
    if (!std::filesystem::exists(inputFilePath)) {
        std::cerr << "The input file does not exist." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Check that the input file is, well... a file.
    if (!std::filesystem::is_regular_file(inputFilePath)) {
        std::cerr << "The input file is not a file." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Check that the variable name is a valid variable name.
    if (!validateVariableName(variableName)) {
        std::cerr << "The variable name is not a valid C identifier." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Try to open the input file.
    FILE* infile;
    infile = std::fopen(inputFilePath.c_str(), "rb");
    if (infile == nullptr) {
        std::perror("Error opening the input file.");
        return EXIT_FAILURE;
    }

    // Try to open the output file.
    std::ofstream outfile;
    outfile.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    try {
        outfile.open(outputFilePath, std::ios::out | std::ios::trunc);
    } catch (const std::ios_base::failure& err) {
        std::cerr << "Error opening the output file." << std::endl;
        std::cerr << err.what() << std::endl;
        std::fclose(infile);
        std::exit(EXIT_FAILURE);
    }

    // Get the file size in bytes.
    std::uintmax_t fileSize;
    try {
        fileSize = std::filesystem::file_size(inputFilePath);
    } catch (const std::filesystem::filesystem_error& err) {
        std::cerr << "Error getting the input file size." << std::endl;
        std::cerr << err.what() << std::endl;
        std::fclose(infile);
        std::exit(EXIT_FAILURE);
    }

    // Get the file size in multiples of u32 (4 bytes), rounded up.
    std::uintmax_t fileSizeU32 = fileSize / 4 + (fileSize % 4 != 0);

    // Transform variableName to VARIABLE_NAME for the header guard.
    const std::string headerGuard = transformVariableNameToHeaderGuard(variableName);
    const std::string byteCountVariableName = variableName + "__byteCount";
    const std::string u32CountVariableName = variableName + "__u32Count";
    const std::string u32ArrVariableName = variableName + "__u32Arr";
    const std::string dataVariableName = variableName + "__data";

    // Output the first few lines.
    outfile << line::pragmaOnce << "\n\n"
            << line::includeEmbed << "\n\n"
            << line::namespaceEmbedBegin << "\n"
            << line::namespaceDataBegin << "\n"
            << line::size1 << byteCountVariableName << line::size2 << fileSize
            << line::size3 << '\n';

#if EMBED_ARRAY
    outfile << line::size1 << u32CountVariableName << line::size2 << fileSizeU32
            << line::size3 << '\n'
            << line::arrayBegin1 << u32CountVariableName << line::arrayBegin2
            << u32ArrVariableName << line::arrayBegin3 << '\n';
#else
    outfile << line::strBegin << dataVariableName << "[" << byteCountVariableName
            << " + 1] = \n";
#endif

    try {
        // Read (and write) the input (output) file in chunks.
        constexpr std::size_t                chunkSize = 4096;
        std::array<std::uint32_t, chunkSize> bytes;

        std::uint8_t lineCount = 0;

        outfile << std::hex << std::setfill('0');

        while (!std::feof(infile)) {
            const std::size_t bytesRead =
                std::fread(bytes.data(), sizeof(std::uint32_t), chunkSize, infile);

            if (bytesRead > 0) {
                for (std::size_t i = 0; i < bytesRead; ++i) {
#if EMBED_ARRAY
                    if (lineCount == 0) {
                        outfile << "  ";
                    }
                    outfile << "0x" << std::setw(8) << bytes[i] << ",";
                    ++lineCount;
                    if (lineCount == 8) {
                        outfile << '\n';
                        lineCount = 0;
                    }
#else
                    if (lineCount == 0) {
                        outfile << "  \"";
                    }
                    // Write each byte separately as a \xFF literal.
                    for (int j = 0; j < 4; ++j) {
                        outfile << "\\x" << std::setw(2)
                                << (0xFF & (bytes[i] >> (8 * j)));
                    }
                    ++lineCount;
                    if (lineCount == 8) {
                        outfile << "\"\n";
                        lineCount = 0;
                    }
#endif
                }
            } else if (std::ferror(infile) != 0) {
                std::perror("Error reading/writing the bytes.");
                std::fclose(infile);
                std::exit(EXIT_FAILURE);
            }
        }
#if !EMBED_ARRAY
        if (lineCount != 0) {
            outfile << '"';
        }
#endif
    } catch (const std::ios_base::failure& err) {
        std::cerr << "Error reading/writing the bytes." << std::endl;
        std::cerr << err.what() << std::endl;
        std::fclose(infile);
        std::exit(EXIT_FAILURE);
    }

// Output the last few lines.
#if EMBED_ARRAY
    outfile << '\n'
            << line::arrayEnd << '\n'
            << line::namespaceDataEnd << "\n\n"
            << line::definition1 << variableName << line::definition2Arr << "  "
            << line::dataPrefix << byteCountVariableName << ", " << line::dataPrefix
            << u32CountVariableName << ", " << line::dataPrefix << u32ArrVariableName;
#else
    outfile << ";\n"
            << line::namespaceDataEnd << "\n\n"
            << line::definition1 << variableName << line::definition2Str << " "
            << line::dataPrefix << byteCountVariableName << ", " << line::dataPrefix
            << dataVariableName;
#endif
    outfile << line::definition3 << "\n\n" << line::namespaceEmbedEnd;

    outfile << std::endl;

    std::fclose(infile);

    return EXIT_SUCCESS;
}