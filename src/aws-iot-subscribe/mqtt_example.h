///
/// \brief Manages communication with AWS IoT Greengrass Nucleus
///
/// \file mqtt_handler.h
///

#ifndef MQTT_HANDLER_H_
#define MQTT_HANDLER_H_

#include <aws/crt/Api.h>
#include <aws/crt/io/HostResolver.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <utility>

// Define Status enum
enum class Status
{
    kSuccess,
    kFailure,
    kNotInitialized
};

class SubscriptionHandler; // Forward declaration

/// \brief MqttHandler class for handling MQTT communication.
class MqttHandler
{
public:
    /// \brief Constructor for the MqttHandler class.
    MqttHandler();

    /// \brief Destructor for the MqttHandler class.
    ~MqttHandler();

    /// \brief Start the initialization step.
    ///
    /// \return Status indicating the success or failure of the operation.
    Status Setup();

    ///
    /// \brief Stop the MQTT handler.
    ///
    /// \return Status
    ///
    Status Stop();

    ///
    /// \brief Subscribe to the specified topic.
    ///
    /// \param topic_name
    /// \param subscription_handler
    /// \return Status
    ///
    Status SubscribeToTopic(std::string topic_name,
                            std::shared_ptr<Aws::Greengrass::SubscribeToIoTCoreStreamHandler> subscription_handler);

    /// \brief Check if the MQTT handler is running.
    ///
    /// \return True if running, false otherwise.
    bool IsRunning() const;

private:
    /// \brief Main thread function for handling MQTT operations.
    void MainThread();

    /// \brief Set up the MQTT connection and configuration.
    Status SetupMQTT();

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
    std::shared_ptr<sv::IpcClientLifecycleHandler>
        ipc_lifecycle_handler_; ///< Pointer to an instance of IpcClientLifecycleHandler for managing IPC client
                                ///< lifecycle.
    std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient>
        ipc_client_; ///< Pointer to an instance of GreengrassCoreIpcClient for Greengrass Core IPC communication.
    /// Component update stream handler
    std::shared_ptr<sv::IpcClientComponentUpdatesStreamHandler> ipc_update_handler_ =
        std::make_shared<sv::IpcClientComponentUpdatesStreamHandler>();
    /// \brief Pointer to an instance of SubscribeToComponentUpdatesOperation for subscribing to component updates.
    std::shared_ptr<Aws::Greengrass::SubscribeToIoTCoreOperation> subscribe_to_upload_operation_;
}; // class MqttHandler

#endif // MQTT_HANDLER_H_
