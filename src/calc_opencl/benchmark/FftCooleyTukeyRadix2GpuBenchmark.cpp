#include <spectr/calc_opencl/FftCooleyTukeyRadix2.h>

#include <spectr/audio_loader/AudioDataGenerator.h>
#include <spectr/calc_opencl/OpenclManager.h>

#include <benchmark/benchmark.h>

#include <vector>

namespace spectr::calc_opencl::benchmark
{
const std::vector<audio_loader::OscillationData> FrequenciesData{
    audio_loader::OscillationData(4),
    audio_loader::OscillationData(7),
    audio_loader::OscillationData(9),
    audio_loader::OscillationData(13),
};

void FftCooleyTukeyRadix2OpenclBenchmark(::benchmark::State& state)
{
    OpenclManager openclManager;
    openclManager.initContext();

    const auto fftSizePowerOfTwo = state.range(0);
    const auto fftSize = 1u << fftSizePowerOfTwo;
    const auto audioData =
      audio_loader::AudioDataGenerator::generate<float>(fftSize, 1, FrequenciesData);
    const auto& values = audioData.getSampleDataFloat(0);

    FftCooleyTukeyRadix2 fftCalculator{ openclManager.getContext(), fftSize };

    for (auto _ : state)
    {
        fftCalculator.execute(values);
    }
}
}

BENCHMARK(spectr::calc_opencl::benchmark::FftCooleyTukeyRadix2OpenclBenchmark)
  ->Unit(benchmark::kMillisecond)
  ->Arg(1)
  ->Arg(2)
  ->Arg(3)
  ->Arg(4)
  ->Arg(5)
  ->Arg(6)
  ->Arg(7)
  ->Arg(8)
  ->Arg(9)
  ->Arg(10)
  ->Arg(11)
  ->Arg(12)
  ->Arg(13)
  ->Arg(14)
  ->Arg(15)
  ->Arg(16)
  ->Arg(17)
  ->Arg(18)
  ->Arg(19)
  ->Arg(20)
  ->Arg(21)
  ->Arg(22);

BENCHMARK_MAIN();
