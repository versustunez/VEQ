#include "Logo.h"

namespace VSTZ::Editor {
static std::string LogoData =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"470\" height=\"170\" "
    "style=\"isolation:isolate\"><path fill=\"#fff\" d=\"m73.2573 152.4053 "
    "59.4-139.32h133.704c1.944 0 3.672-1.728 3.672-3.672 "
    "0-2.16-1.728-3.672-3.672-3.672h-136.728s-.216 0-.216.216h-.432c-.216 "
    "0-.216.216-.216.216-.216 0-.216 0-.432.216 0 0-.216 0-.216.216-.216 "
    "0-.216 0-.432.216v.216c-.216 0-.216.216-.432.216 0 .216 0 "
    ".432-.216.432v.216l-57.024 "
    "133.92-57.24-133.92c-.648-1.728-2.808-2.592-4.752-1.944-1.728.864-2.592 "
    "3.024-1.944 4.752l60.48 141.696c.216.432.432.648.432.864.216 0 .216 0 "
    ".216.216l.648.648h.216c.216 0 .216.216.432.216h.216l.216.216h1.944s.216 0 "
    ".216-.216h.216c.216 0 "
    ".432-.216.432-.216h.216l.648-.648c0-.216.216-.216.216-.216.216-.216.216-."
    "432.432-.864zm106.488-1.296c0 1.944 1.512 3.456 3.672 3.456h82.944c1.944 "
    "0 3.672-1.512 3.672-3.456 0-2.16-1.728-3.672-3.672-3.672h-82.944c-2.16 "
    "0-3.672 1.512-3.672 3.672zm0-73.224c0 1.944 1.512 3.672 3.672 "
    "3.672h81.432c1.944 0 3.672-1.728 3.672-3.672 "
    "0-1.944-1.728-3.672-3.672-3.672h-81.432c-2.16 0-3.672 1.728-3.672 "
    "3.672z\"/><path fill=\"#2a7fff\" d=\"M435.5843 139.0133c17.712-13.608 "
    "28.944-34.776 28.944-58.752 0-41.04-33.264-74.52-74.304-74.52s-74.52 "
    "33.48-74.52 74.52 33.48 74.304 74.52 74.304c16.632 0 31.968-5.4 "
    "44.28-14.688.216 0 .648-.216.864-.432 "
    "0-.216.216-.216.216-.432zm-112.537-58.752c0-37.152 30.024-67.176 "
    "67.176-67.176 36.936 0 67.176 30.024 67.176 67.176 0 20.736-9.504 "
    "39.312-24.408 51.624l-23.976-23.976c-1.512-1.512-3.672-1.512-5.184 "
    "0-1.512 1.296-1.512 3.672 0 5.184l23.328 23.328c-10.584 6.912-23.328 "
    "11.016-36.936 11.016-37.152 0-67.176-30.24-67.176-67.176zm120.528 72.576 "
    "7.344 7.128c.648.864 1.512 1.08 2.592 1.08.864 0 1.728-.216 2.592-1.08 "
    "1.296-1.296 1.296-3.672 "
    "0-4.968l-7.344-7.344c-1.512-1.512-3.672-1.512-5.184 0-1.512 1.296-1.512 "
    "3.672 0 5.184z\"/></svg>";
Logo::Logo(VSTZ::InstanceID id) : m_ID(id) {
  m_Image =
      juce::Drawable::createFromImageData(LogoData.c_str(), LogoData.size());
}
void Logo::paint(juce::Graphics &g) {
  if (m_Image) {
    m_Image->draw(g, 1.0f);
  }
}

void Logo::resized() {
  juce::RectanglePlacement placement = juce::RectanglePlacement::centred;
  juce::Rectangle<float> react{0, 0, (float)getWidth(), (float)getHeight()};
  if (m_Image == nullptr) {
    return;
  }
  auto bounds = m_Image->getDrawableBounds();
  if (react.getWidth() == 0 || react.getHeight() == 0 ||
      bounds.getWidth() == 0 || bounds.getHeight() == 0) {
    return;
  }
  auto transform = placement.getTransformToFit(bounds, react);
  m_Image->setTransform(transform);
}
} // namespace VSTZ::Editor