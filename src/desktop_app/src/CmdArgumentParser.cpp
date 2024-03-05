#include <spectr/desktop_app/CmdArgumentParser.h>

#include <spectr/utils/Exception.h>

#include <spectr/utils/Options.h>

#include <format>
#include <string>
#include <cstring>

#include <stdarg.hpp>

constexpr const char* help_options[]       = { "--help",           "-h" };
constexpr const char* version_options[]    = { "--version",        "-v" };
constexpr const char* input_path_options[] = { "--input",          "-i" };
constexpr const char* fft_power_options[]  = { "--fft-size-power", "-p" };
constexpr const char* cps_options[]        = { "--cps",            "-c" };

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

DesktopAppSettings CmdArgumentParser::parse(int argc, const char* argv[])
{
    DesktopAppSettings settings;

    size_t fftSizePowerOfTwo = 12;

    settings.fftCalculationPerSecond = 20;
    
    stdarg::arg_parser parser({ (size_t)argc, argv }, "Spectr tool for signal spectrum analysis");

    std::string path;
    
    parser << stdarg::option<void()>({ help_options[0],    help_options[1]       }, "show help message", [parser]() { stdarg::arg_parser::help(parser); })
           << stdarg::option<void()>({ version_options[0], version_options[1]    }, "show tool version", [&]() { settings.command = Command::PrintVersion; })
           << stdarg::argument({ input_path_options[0],    input_path_options[1] }, "path of input signal WAV audio file", "path", path)
           << stdarg::argument({ fft_power_options[0],     fft_power_options[1]  }, "power P of 2 of the FFT size - 2^P.", "P", fftSizePowerOfTwo)
           << stdarg::argument({ cps_options[0],           cps_options[1]        }, "FFT calculations per second", "cps", settings.fftCalculationPerSecond);

    parser();

    if (path != "") settings.audioFilePath = path;

    settings.helpDescription = parser.getDescription();

    if (settings.command == Command::PrintVersion) return settings;

    settings.command = Command::Execute;

    /*const auto isSingleConfigPath = vm.count(OPTION_INPUT_PATH) == 1;
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
    }*/

    const auto fftSize = 1u << fftSizePowerOfTwo;
    settings.fftSize = fftSize;

    return settings;
}
}
