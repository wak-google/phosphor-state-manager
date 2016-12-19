#include <iostream>
#include <string>
#include <log.hpp>
#include "bmc_state_manager.hpp"

namespace phosphor
{
namespace state
{
namespace manager
{

// When you see server:: you know we're referencing our base class
namespace server = sdbusplus::xyz::openbmc_project::State::server;

using namespace phosphor::logging;

/* Map a transition to it's systemd target */
const std::map<server::BMC::Transition, const char*> SYSTEMD_TABLE =
{
        {server::BMC::Transition::Reboot, "reboot.target"}
};

constexpr auto SYSTEMD_SERVICE   = "org.freedesktop.systemd1";
constexpr auto SYSTEMD_OBJ_PATH  = "/org/freedesktop/systemd1";
constexpr auto SYSTEMD_INTERFACE = "org.freedesktop.systemd1.Manager";

void BMC::subscribeToSystemdSignals()
{
    auto method = this->bus.new_method_call(SYSTEMD_SERVICE,
                                            SYSTEMD_OBJ_PATH,
                                            SYSTEMD_INTERFACE,
                                            "Subscribe");
    this->bus.call(method);

    return;
}

void BMC::executeTransition(const Transition tranReq)
{
    //Check to make sure it can be found
    auto iter = SYSTEMD_TABLE.find(tranReq);
    if (iter == SYSTEMD_TABLE.end()) return;

    const auto& sysdUnit = iter->second;

    auto method = this->bus.new_method_call(SYSTEMD_SERVICE,
                                            SYSTEMD_OBJ_PATH,
                                            SYSTEMD_INTERFACE,
                                            "StartUnit");

    method.append(sysdUnit, "replace");

    this->bus.call(method);

    return;
}

BMC::Transition BMC::requestedBMCTransition(Transition value)
{
    log<level::INFO>(
            "Setting the RequestedBMCTransition field",
            entry("REQUESTED_BMC_TRANSITION=0x%s",
                  convertForMessage(value).c_str()));

    executeTransition(value);
    return server::BMC::requestedBMCTransition(value);
}


} // namespace manager
} // namespace state
} // namepsace phosphor
