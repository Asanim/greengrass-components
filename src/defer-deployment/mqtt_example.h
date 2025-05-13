///
/// \brief Manages communication with AWS IoT Greengrass Nucleus
///
/// \file mqtt_example.h
///

#ifndef MQTT_EXAMPLE_H_
#define MQTT_EXAMPLE_H_

#include <aws/crt/Api.h>
#include <aws/crt/io/HostResolver.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include "component_update_example.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <utility>

// Define ResultCode enum
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

/// \brief MqttExample class for handling MQTT communication.
class MqttExample
{
public:
  /// \brief Constructor for the MqttExample class.
  MqttExample();

  /// \brief Destructor for the MqttExample class.
  ~MqttExample();

  /// \brief Start the initialization step.
  ///
  /// \return ResultCode indicating the success or failure of the operation.
  ResultCode Setup();

  ///
  /// \brief Stop the MQTT handler.
  ///
  /// \return Status
  ///
  ResultCode Stop();

  ///
  /// \brief Subscribe to the specified topic.
  ///
  /// \param topic_name
  /// \param subscription_handler
  /// \return Status
  ///
  ResultCode SubscribeToCompponentUpdates();

private:
  /// \brief Set up the MQTT connection and configuration.
  ResultCode SetupMQTT();

  // Member variables
  std::unique_ptr<std::thread> thread_;  ///< Thread for the main MQTT handling logic.
  std::atomic<bool> is_running_ = false; ///< Flag indicating whether the MQTT handler is running.
  const char *ggc_version_;              ///< Greengrass Core version information.
  const char *thing_name_;               ///< Name of the thing associated with MQTT.

  std::unique_ptr<Aws::Crt::ApiHandle> apiHandle_; ///< Pointer to an instance of ApiHandle for API interactions.
  std::unique_ptr<Aws::Crt::Io::EventLoopGroup>
      event_loop_group_; ///< Pointer to an instance of EventLoopGroup for managing event loops.
  std::unique_ptr<Aws::Crt::Io::DefaultHostResolver>
      socket_resolver_; ///< Pointer to an instance of DefaultHostResolver for socket resolution.
  std::unique_ptr<Aws::Crt::Io::ClientBootstrap>
      client_bootstrap_; ///< Pointer to an instance of ClientBootstrap for setting up client connections.
  std::shared_ptr<IpcClientLifecycleHandler>
      ipc_lifecycle_handler_; ///< Pointer to an instance of IpcClientLifecycleHandler for managing IPC client
                              ///< lifecycle.
  std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient>
      ipc_client_; ///< Pointer to an instance of GreengrassCoreIpcClient for Greengrass Core IPC communication.
  /// \brief Pointer to an instance of ComponentUpdateExample for subscribing to component updates.
  std::shared_ptr<ComponentUpdateExample> component_update_handler_;
}; // class MqttExample

#endif // MQTT_EXAMPLE_H_
