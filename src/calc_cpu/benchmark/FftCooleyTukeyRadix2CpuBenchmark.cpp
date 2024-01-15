#include <spectr/calc_cpu/FftCooleyTukeyRadix2.h>

#include <spectr/audio_loader/AudioDataGenerator.h>

#include <benchmark/benchmark.h>

#include <vector>

namespace spectr::calc_cpu::benchmark
{
const std::vector<audio_loader::OscillationData> FrequenciesData{
    audio_loader::OscillationData(4),
    audio_loader::OscillationData(7),
    audio_loader::OscillationData(9),
    audio_loader::OscillationData(13),
};

void FftCooleyTukeyRadix2CpuBenchmark(::benchmark::State& state)
{
    const auto fftSizePowerOfTwo = state.range(0);
    const auto fftSize = 1 << fftSizePowerOfTwo;
    const auto audioData =
      audio_loader::AudioDataGenerator::generate<float>(fftSize, 1, FrequenciesData);
    const auto& values = audioData.getSampleDataFloat(0);

    for (auto _ : state)
    {
        const auto fft = calc_cpu::FftCooleyTukeyRadix2::getFFT(values);
        ::benchmark::DoNotOptimize(fft[0]);
    }
}
}

BENCHMARK(spectr::calc_cpu::benchmark::FftCooleyTukeyRadix2CpuBenchmark)
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
  ->Arg(18);

BENCHMARK_MAIN();
