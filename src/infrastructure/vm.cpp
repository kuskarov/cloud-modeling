#include "vm.h"

#include <loguru.hpp>

void
sim::resources::VirtualMachine::HandleEvent(
    const std::shared_ptr<events::Event>& event)
{
    try {
        auto vm_event = dynamic_cast<const VirtualMachineEvent*>(event.get());

        switch (vm_event->type) {
            default: {
                LOG_F(ERROR, "VM %s received VM event with invalid type",
                      name_.c_str());
                break;
            }
        }

    } catch (const std::bad_cast& bc) {
        LOG_F(ERROR, "VM %s received non-VM event!", name_.c_str());
    }
}
