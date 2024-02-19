# Spectr architecture

Spectr project consists of several conceptual parts:

* Computations part
    * CPU computations - used as a reference implementation, because it's easy to debug.
    * GPU computations - computations-heavy algorithms are imlemented for execution on GPU using the OpenCL platform.

* Rendering part - provides functionality for rendering waterfall time-frequency heatmap of signal and signal density view (frequency-amplitude heatmap, also called RTSA - real-time spectrum analysis).

* Application part - ties all parts together, adds parsing of command-line arguments, loading data to process, creates Window, creates UI windows with settings etc.

## Rendering parts

There are 2 spectrograms rendered in the program:

* Waterfall spectrogram (time-frequency heatmap).
* Signal density spectrogram (frequency-amplitude heatmap) (RTSA view - real-time spectrum analysis view).

## How to use the Spectr application

You can launch the Spectr application from command line with arguments:
* 

## Modules list:

All source code files are placed under "./src" dir. The project is splitted into bunch of submodules that are built into libs and executables.

All assets are placed under "./assets" dir. It includes shaders for OpenGL, kernels for OpenCL, fonts for ImGUI, audio samples for debugging. When project is built, a symlink to "assets" folder is placed to the "bin" folder. It allows the application to access the source code of shaders/kernels from "assets" folder.

List of the project modules (libs):
* spectr.utils - utility methods and classes helpful in all cases.
* spectr.audio_loader - audio loader that can load WAV audio files as raw signal buffers.
* spectr.calc_cpu - Implementation of calculations on CPU.
* spectr.calc_opencl - Implementation of calculations on GPU with OpenCL.
* spectr.render_gl - Implementation of rendering with OpenGL.
* spectr.desktop_app - Main executable, application for rendering the waterfall diagram.

## Supported signal sources

* Signal can be loaded from WAV audio files using audio_loader module.
* Signal can be generated using the audio_loader::SignalDataGenerator from audio_loader module. It is useful for debugging, testing and verifying FFT calculations.
    * You can generate a signal from sine waves of specified frequency, amplitude and phase.
    * You can generate a signal from constant value.

## Application flow

1. Application loads.
1. Command-line arguments are parsed.
1. Audio file is loaded.
1. Application initializes.
1. Application launches the "worker".
1. Worker invokes the calculations and 
1. Application renders the current state of spectrogram.
