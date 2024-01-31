#pragma once
#include <Audio/TargetDrive.h>
#include <GUI/Components/VComponent.h>
#include <GUI/Theme/Theme.h>

namespace VSTZ::Editor {
class DriveMeter final : public GUI::VComponent {
public:
  DriveMeter(TargetDrive* target);
  auto paint(juce::Graphics &g) -> void override;

  const TargetDrive *DriveTarget{nullptr};
  Theme::Theme* GlobalTheme{nullptr};
};
} // namespace VSTZ::Editor