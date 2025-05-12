#ifndef SHADOW_HANDLER_H
#define SHADOW_HANDLER_H

#include <string>
#include <memory>
#include <iostream>
#include <aws/crt/Types.h>
#include <aws/crt/io/EventLoopGroup.h>
#include <aws/crt/io/HostResolver.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>
#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

enum class ResultCode
{
    kSuccess,
    kFailure,
    kOperationError,
    kOperationTimeout,
    kRequestError,
    kNetworkError,
    kUnknownError
};

class IpcClientLifecycleHandler : public ConnectionLifecycleHandler
{
    void OnConnectCallback() override
    {
        std::cout << "Connected to IPC service." << std::endl;
        // Handle connection to IPC service.
    }

    void OnDisconnectCallback(RpcError error) override
    {
        std::cout << "Disconnected from IPC service. Error: " << std::endl;
        // Handle disconnection from IPC service.
    }

    bool OnErrorCallback(RpcError error) override
    {
        std::cout << "IPC service connection error: " << std::endl;
        // Handle IPC service connection error.
        return true;
    }
};

class ShadowExample
{
public:
    ShadowExample(const Aws::Crt::String &thing_name) : thing_name_(thing_name),
                                                        is_connected_(false),
                                                        is_running_(false) {}

    /// \brief Initialize the ShadowExample
    ResultCode Initialize();

    /// \brief Get the Thing Shadow
    ResultCode GetThingShadow(const Aws::Crt::String &shadow_name);

    /// \brief Update the Thing Shadow
    ResultCode UpdateThingShadow(const Aws::Crt::String &shadow_name, const std::string &shadow_payload);

    /// \brief List named shadows for the thing
    ResultCode ListNamedShadowsForThing();

    /// \brief Print help for command-line usage
    static void PrintHelp();

private:
    Aws::Crt::String thing_name_;
    std::unique_ptr<Aws::Crt::ApiHandle> apiHandle_;
    std::unique_ptr<Aws::Crt::Io::EventLoopGroup> event_loop_group_;
    std::unique_ptr<Aws::Crt::Io::DefaultHostResolver> socket_resolver_;
    std::unique_ptr<Aws::Crt::Io::ClientBootstrap> client_bootstrap_;
    std::unique_ptr<IpcClientLifecycleHandler> ipc_lifecycle_handler_;
    std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient> ipc_client_;
    bool is_connected_;
    bool is_running_;
};

#endif // SHADOW_HANDLER_H
