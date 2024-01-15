#include <spectr/audio_loader/AudioDataGenerator.h>

namespace spectr::audio_loader
{
OscillationData::OscillationData(float frequency, float amplitude, float phase)
  : frequency{ frequency }
  , amplitude{ amplitude }
  , phase{ phase }
{
}
}
