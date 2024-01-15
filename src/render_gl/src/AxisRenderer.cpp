#include <spectr/render_gl/AxisRenderer.h>

#include <spectr/render_gl/ImguiUtils.h>
#include <spectr/render_gl/OpenGlUtils.h>

#include <spectr/utils/Asset.h>
#include <spectr/utils/File.h>

#include <fmt/format.h>

#include <cmath>
#include <mutex>
#include <vector>

namespace spectr::render_gl
{
namespace
{
const std::string FontAssetPath = "fonts/intel_one/fonts/ttf/intelone-mono-font-family-regular.ttf";
constexpr auto FontSizeInPixels = 20.f;
const auto MarksColor = Color(0, 0.5, 1, 1);

// Line rendering logic
const auto LineVertexShaderPath = "shaders/line/Vertex.glsl";
const auto LineFragmentShaderPath = "shaders/line/Fragment.glsl";

GLuint LineShaderProgram = NoShaderProgram;
GLuint LineVbo = NoBuffer;
GLuint LineVao = NoBuffer;

const std::vector<float> Vertices{
    -1.0f, -1.0f, 0.0f, // left-down
    1.0f,  -1.0f, 0.0f, // right-down
    1.0f,  1.0f,  0.0f, // right-top

    -1.0f, -1.0f, 0.0f, // left-down
    1.0f,  1.0f,  0.0f, // right-top
    -1.0f, 1.0f,  0.0f, // left-top
};

void initLineRendering()
{
    const auto vertexShaderSource = utils::File::read(utils::Asset::getPath(LineVertexShaderPath));

    const auto fragmentShaderSource =
      utils::File::read(utils::Asset::getPath(LineFragmentShaderPath));

    LineShaderProgram = OpenGlUtils::createShaderProgram(vertexShaderSource, fragmentShaderSource);

    glGenBuffers(1, &LineVbo);
    glGenVertexArrays(1, &LineVao);

    glBindVertexArray(LineVao);

    glBindBuffer(GL_ARRAY_BUFFER, LineVbo);
    glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(float), Vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NoBuffer);
    glBindVertexArray(NoBuffer);
}

std::once_flag LineRenderInitFlag;

void drawLine(const glm::vec2& lineStart,
              const glm::vec2& lineEnd,
              float lineWidth,
              const Color& lineColor,
              const RenderContext& renderContext)
{
    std::call_once(LineRenderInitFlag, initLineRendering);

    const auto lineHeight = glm::distance(lineStart, lineEnd);

    const auto scaleX = lineWidth / renderContext.viewportSize.x;
    const auto scaleY = lineHeight / 2.0f;
    const auto scaleMatrix = glm::mat3({ scaleX, 0, 0 }, { 0, scaleY, 0 }, { 0, 0, 1 });

    const auto lineDirection = glm::normalize(lineEnd - lineStart);
    const auto cosA = glm::dot(glm::vec2(0, 1), lineDirection);
    const auto sinA = glm::sqrt(1 - std::pow(cosA, 2));
    const auto rotationMatrix = glm::mat3({ cosA, sinA, 0 }, { -sinA, cosA, 0 }, { 0, 0, 1 });

    const auto lineMiddle = (lineStart + lineEnd) / 2.0f;
    const auto translation = lineMiddle;
    const auto translationMatrix =
      glm::mat3({ 1, 0, 0 }, { 0, 1, 0 }, { translation.x, translation.y, 1 });

    glUseProgram(LineShaderProgram);

    const auto lineColorId = glGetUniformLocation(LineShaderProgram, "lineColor");
    // glUniform1fv(lineColorId, lineColor.length(), &lineColor[0]);
    glUniform4f(lineColorId, lineColor.r, lineColor.g, lineColor.b, lineColor.a);

    const auto mvpId = glGetUniformLocation(LineShaderProgram, "mvp");
    const auto mvp = translationMatrix * rotationMatrix * scaleMatrix;
    const auto mvpPtr = &mvp[0][0];
    glUniformMatrix3fv(mvpId, 1, GL_FALSE, mvpPtr);

    glBindVertexArray(LineVao);
    glBindBuffer(GL_ARRAY_BUFFER, LineVbo);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, NoBuffer);
    glBindVertexArray(NoBuffer);
}

void ShowLabel(const std::string& title,
               const std::string& text,
               const glm::ivec2& pixelCoords,
               AxisRenderMode mode)
{
    static int location = 0;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    // const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    /*window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
    window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);*/

    window_pos.x = static_cast<float>(pixelCoords.x);
    window_pos.y = work_size.y - pixelCoords.y;

    if (mode == AxisRenderMode::Horizontal)
    {
        window_pos_pivot.x = 0.5f;
        window_pos_pivot.y = 0.5f;
    }
    else
    {
        window_pos_pivot.x = 0.0f;
        window_pos_pivot.y = 0.5f;
    }

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    window_flags |= ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowBgAlpha(0.8f);
    ImGui::Begin(title.c_str(), nullptr, window_flags);
    ImGui::Text(text.c_str());
    ImGui::End();
}
}

AxisRenderer::AxisRenderer(AxisRenderMode mode)
  : m_mode{ mode }
{
    ImGuiIO& io = ImGui::GetIO();
    const auto path = utils::Asset::getPath(FontAssetPath).string();
    m_font = io.Fonts->AddFontFromFileTTF(path.c_str(), FontSizeInPixels);
}

AxisRenderer::~AxisRenderer()
{
    //
}

void AxisRenderer::render(const RenderContext& renderContext)
{
    if (!m_isEnabled)
    {
        return;
    }

    glm::ivec2 marksStartPixelPoint{};
    glm::ivec2 marksEndPixelPoint{};

    if (m_mode == AxisRenderMode::Horizontal)
    {
        marksStartPixelPoint = { 0, 0 };
        marksEndPixelPoint = { renderContext.viewportSize.x, 0 };
    }
    else
    {
        marksStartPixelPoint = { 0, 0 };
        marksEndPixelPoint = { 0, renderContext.viewportSize.y };
    }

    const auto startWorldCoord = renderContext.pixelToWorld(marksStartPixelPoint);
    const auto endWorldCoord = renderContext.pixelToWorld(marksEndPixelPoint);

    const auto lineUnitDir = glm::normalize(endWorldCoord - startWorldCoord);

    const auto originPoint = m_mode == AxisRenderMode::Horizontal ? glm::vec2(0, startWorldCoord.y)
                                                                  : glm::vec2(startWorldCoord.x, 0);

    const auto startMarksCount = glm::dot(startWorldCoord, lineUnitDir) / m_marksStep;
    const auto endMarksCount = glm::dot(endWorldCoord, lineUnitDir) / m_marksStep;

    const auto startMarksCountRounded = std::floor(startMarksCount);
    const auto endMarksCountRounded = std::ceil(endMarksCount);

    const auto startWorldCoordRounded =
      originPoint + lineUnitDir * startMarksCountRounded * m_marksStep;
    const auto endWorldCoordRounded =
      originPoint + lineUnitDir * endMarksCountRounded * m_marksStep;

    const auto marksCount = std::abs(startMarksCount) + std::abs(endMarksCount);

    ImGui::PushFont(m_font);
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));

    for (size_t i = 0; i <= marksCount; ++i)
    {
        const auto markWorldCoord =
          startWorldCoordRounded + lineUnitDir * m_marksStep * static_cast<float>(i);
        auto markPixelCoord = renderContext.worldToPixel(markWorldCoord);

        if (m_mode == AxisRenderMode::Horizontal)
        {
            markPixelCoord.y += static_cast<int>(FontSizeInPixels * 2);
        }
        else
        {
            // markPixelCoord.x += 100;
        }

        if (m_isRenderLabels)
        {
            const auto value =
              m_mode == AxisRenderMode::Horizontal ? markWorldCoord.x : markWorldCoord.y;

            auto digitsCount = static_cast<int>(std::log10(m_marksStep));
            digitsCount = digitsCount > 0 ? 0 : -digitsCount;

            const auto formatString = fmt::format("{{:.{}f}}", digitsCount);
            const auto text = fmt::format(fmt::runtime(formatString), value);
            const auto title =
              (m_mode == AxisRenderMode::Vertical ? "Vertical" : "Horizontal") + std::to_string(i);
            ShowLabel(title, text, markPixelCoord, m_mode);
        }

        if (m_isRenderMarks)
        {
            const auto lineStartPixelPoint = markPixelCoord;
            const auto lineEndPixelPoint =
              lineStartPixelPoint + (m_mode == AxisRenderMode::Horizontal
                                       ? glm::ivec2(0, m_markHeight)
                                       : glm::ivec2(m_markHeight, 0));

            const auto lineStart = renderContext.pixelToNdc(lineStartPixelPoint);
            const auto lineEnd = renderContext.pixelToNdc(lineEndPixelPoint);
            const auto lineWidth = m_markWidth;

            drawLine(lineStart, lineEnd, m_markWidth, MarksColor, renderContext);
        }
    }

    ImGui::PopStyleColor();
    ImGui::PopFont();
}

bool AxisRenderer::getIsEnabled() const
{
    return m_isEnabled;
}

void AxisRenderer::setIsEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}

bool AxisRenderer::getIsRenderLabels() const
{
    return m_isRenderLabels;
}

void AxisRenderer::setIsRenderLabels(bool isRenderLabels)
{
    m_isRenderLabels = isRenderLabels;
}

bool AxisRenderer::getIsRenderMarks() const
{
    return m_isRenderMarks;
}

void AxisRenderer::setIsRenderMarks(bool isRenderMarks)
{
    m_isRenderMarks = isRenderMarks;
}

float AxisRenderer::getMarksStep() const
{
    return m_marksStep;
}

void AxisRenderer::setMarksStep(float marksStep)
{
    m_marksStep = marksStep;
}
}
