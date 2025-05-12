#ifndef SHADOW_HANDLER_H
#define SHADOW_HANDLER_H

#include <string>
#include <aws/crt/Types.h>
#include <sv/Status.h>

class ShadowExample {
public:
    ShadowExample(const Aws::Crt::String& thing_name) : thing_name_(thing_name) {}

    /// \brief Initialize the ShadowExample
    sv::Status Initialize();

    /// \brief Get the Thing Shadow
    sv::Status GetThingShadow(const Aws::Crt::String& shadow_name);

    /// \brief Update the Thing Shadow
    sv::Status UpdateThingShadow(const Aws::Crt::String& shadow_name, const std::string& shadow_payload);

    /// \brief List named shadows for the thing
    sv::Status ListNamedShadowsForThing();

    /// \brief Print help for command-line usage
    static void PrintHelp();

private:
    Aws::Crt::String thing_name_;
    // ...existing private members...
};

#endif // SHADOW_HANDLER_H
