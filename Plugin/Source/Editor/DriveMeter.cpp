#include "DriveMeter.h"

#include <Core/Config.h>
#include <VUtils/Math.h>

namespace VSTZ::Editor {
DriveMeter::DriveMeter(TargetDrive *target) : DriveTarget(target) {
  GlobalTheme = Core::Config::get().theme().Get();
}
void DriveMeter::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::black);
  g.setColour(GlobalTheme->getColor(Theme::Colors::accent));

  const auto drive = static_cast<float>(DriveTarget->GetDriveGain());
  const float height =
      VUtils::MathF::Map(drive, -TargetDrive::MaxDrive, TargetDrive::MaxDrive,
                         static_cast<float>(getHeight()), 0);

  const float centerY = 0.5f * static_cast<float>(getHeight());
  const float y = centerY > height ? height : centerY;
  const float rectHeight = std::abs(centerY - height);

  g.fillRect(0.0f, y, static_cast<float>(getWidth()), rectHeight);
}
} // namespace VSTZ::Editor