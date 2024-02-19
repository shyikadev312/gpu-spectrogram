#include <spectr/calc_cpu/FftCooleyTukeyRadix2.h>

#include <spectr/audio_loader/SignalDataGenerator.h>

#include <benchmark/benchmark.h>

#include <vector>

namespace spectr::calc_cpu::benchmark
{
const std::vector<audio_loader::SineWaveInfo> FrequenciesData{
    audio_loader::SineWaveInfo(4),
    audio_loader::SineWaveInfo(7),
    audio_loader::SineWaveInfo(9),
    audio_loader::SineWaveInfo(13),
};

void FftCooleyTukeyRadix2CpuBenchmark(::benchmark::State& state)
{
    const auto powerOfTwo = state.range(0);
    const auto fftSize = 1 << powerOfTwo;
    const auto duration = 1.0f;
    const auto signalData =
      audio_loader::SignalDataGenerator::generate<float>(fftSize, duration, FrequenciesData);
    const auto& values = signalData.getSampleDataFloat(0);

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
