#include <spectr/render_gl/RtsaContainer.h>

namespace spectr::render_gl
{
float RtsaContainerSettings::getMaxFrequency() const
{
    return frequencyValuesCount / valuesInOneHertz;
}

RtsaContainer::RtsaContainer(RtsaContainerSettings settings)
  : m_settings{ settings }
{
    glGenBuffers(1, &m_rtsaHeatmapSsbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_rtsaHeatmapSsbo);

    const auto bufferSize =
      settings.frequencyValuesCount * settings.magnitudeRangeValuesCount * sizeof(float) * 2;
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

RtsaContainer::~RtsaContainer()
{
    glDeleteBuffers(1, &m_rtsaHeatmapSsbo);
}

GLuint RtsaContainer::getBuffer() const
{
    return m_rtsaHeatmapSsbo;
}

const RtsaContainerSettings& RtsaContainer::getSettings() const
{
    return m_settings;
}
}
