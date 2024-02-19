#include <spectr/audio_loader/SignalDataGenerator.h>

namespace spectr::audio_loader
{
SineWaveInfo::SineWaveInfo(float frequency, float amplitude, float phase)
  : frequency{ frequency }
  , amplitude{ amplitude }
  , phase{ phase }
{
}
}
