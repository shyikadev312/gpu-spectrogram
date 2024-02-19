#pragma once

#include <spectr/audio_loader/SignalData.h>

#include <filesystem>
#include <iostream>

namespace spectr::audio_loader
{
class WavLoader
{
public:
    static SignalData load(std::istream& wavFileStream);
};
}
