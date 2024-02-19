#include <spectr/desktop_app/CmdArgumentParser.h>

#include <spectr/utils/Exception.h>

#include <boost/program_options.hpp>
#include <fmt/format.h>

#include <string>

#define OPTION_HELP "help"
#define OPTION_VERSION "version"
#define OPTION_INPUT_PATH "input"
#define OPTION_FFT_POWER "fft-size-power"
#define OPTION_CPS "cps"

namespace po = boost::program_options;

namespace spectr::desktop_app
{
namespace
{
size_t parseNumber(const char* str)
{
    const char* fftPowerOfTwoStrEnd = str + std::strlen(str);
    char* parsedEnd = nullptr;
    const auto value = std::strtoull(str, &parsedEnd, 10);

    if (parsedEnd != fftPowerOfTwoStrEnd)
    {
        throw utils::Exception("Failed to parse the number: {}", str);
    }

    if (errno == ERANGE && value == ULLONG_MAX)
    {
        throw utils::Exception("Out of range exception, too big number specified: {}", str);
    }

    return value;
}
}

DesktopAppSettings CmdArgumentParser::parse(int argc, char* argv[])
{
    DesktopAppSettings settings;

    size_t fftSizePowerOfTwo;

    po::options_description desc("Spectr tool for signal spectrum analysis");
    desc.add_options()
      // clang-format off
      (OPTION_HELP ",h", "show help message")
      (OPTION_VERSION ",v", "show tool version")
      (OPTION_INPUT_PATH ",i", po::value<std::filesystem::path>(&settings.audioFilePath), "path of input signal WAV audio file")
      (OPTION_FFT_POWER ",p", po::value<size_t>(&fftSizePowerOfTwo)->default_value(12), "power P of 2 of the FFT size - 2^P.")
      (OPTION_CPS ",c", po::value<size_t>(&settings.fftCalculationPerSecond)->default_value(20), "FFT calculations per second")
      // clang-format on
      ;

    po::positional_options_description p;
    p.add(OPTION_INPUT_PATH, -1);

    po::variables_map vm;

    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

    po::notify(vm);

    std::stringstream ss;
    ss << desc;
    settings.helpDescription = ss.str();

    if (vm.count(OPTION_HELP))
    {
        settings.command = Command::PrintHelp;
        return settings;
    }

    if (vm.count(OPTION_VERSION))
    {
        settings.command = Command::PrintVersion;
        return settings;
    }

    settings.command = Command::Execute;

    const auto isSingleConfigPath = vm.count(OPTION_INPUT_PATH) == 1;
    if (vm.count(OPTION_INPUT_PATH) == 0)
    {
        throw utils::Exception("Expected one argument of input signal file.");
    }

    if (fftSizePowerOfTwo <= 6)
    {
        throw utils::Exception("FFT size must be bigger 64.");
    }

    if (settings.fftCalculationPerSecond == 0)
    {
        throw utils::Exception("FFT calculations per second can't be zero.");
    }

    const auto fftSize = 1u << fftSizePowerOfTwo;
    settings.fftSize = fftSize;

    return settings;
}
}
