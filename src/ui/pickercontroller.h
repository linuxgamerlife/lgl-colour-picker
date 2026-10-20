#pragma once

#include "core/colour.h"

#include <functional>
#include <memory>
#include <string>

namespace colourpicker {

class PickerController final {
public:
    struct State;
    using Completion = std::function<void(const std::optional<Colour> &colour, const std::string &message)>;

    explicit PickerController(Completion completion);
    ~PickerController();
    PickerController(const PickerController &) = delete;
    PickerController &operator=(const PickerController &) = delete;

    void start();
    bool active() const;

private:
    std::shared_ptr<State> m_state;
};

} // namespace colourpicker
