///
/// \brief
///
/// \file mqtt_handler.cc
///

#include "awsiot/mqtt_handler.h"
#include "awsiot/subscription_handler.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>
#include <curl/curl.h>

#include <chrono>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using namespace Aws::Crt;
using namespace Aws::Greengrass;

using namespace Aws::Auth;

namespace sc
{

  MqttHandler::MqttHandler() { Setup(); }

  MqttHandler::~MqttHandler() { Stop(); }

  Status MqttHandler::Stop()
  {
    LOG(INFO) << "Closing the MQTT handler";
    if (SubscribeToCertificateUpdatesOperation_)
    {
      SubscribeToCertificateUpdatesOperation_->Close();
    }
    return Status::kSuccess;
  }

  Status MqttHandler::Setup()
  {
    // Retrieve environment variables
    thing_name_ = getenv("AWS_IOT_THING_NAME");
    ggc_version_ = getenv("GGC_VERSION");

    // Check if required environment variables are not set
    if (!thing_name_ || !ggc_version_)
    {
      // Log an error and disable datalogging
      LOG(ERROR) << "AWS Environment variables 'AWS_IOT_THING_NAME' 'GGC_VERSION' not set. You must run this as a Greengrass component";
      return Status::kFailure;
    }
    // Log the retrieved thing name and GGC version
    DLOG(INFO) << "Thing name: " << thing_name_ << "\nGGC version: " << ggc_version_;

    // Check if MQTT setup is successful
    if (MqttHandler::SetupMQTT() != Status::kSuccess)
    {
      // Log an error and disable datalogging
      LOG(ERROR) << "Could not initialize IoT Core. MQTT publish is disabled";
      return Status::kFailure;
    }

    return Status::kSuccess;
  }

  std::shared_ptr<sv::IpcClientComponentUpdatesStreamHandler> MqttHandler::GetComponentUpdateExample() const
  {
    return ipc_update_handler_;
  }

  Status MqttHandler::SetupMQTT()
  {
    // Initialize ApiHandle
    apiHandle_ = std::make_unique<Aws::Crt::ApiHandle>(DefaultAllocator());

    // Initialize EventLoopGroup
    event_loop_group_ = std::make_unique<Aws::Crt::Io::EventLoopGroup>(1);

    // Initialize DefaultHostResolver
    socket_resolver_ = std::make_unique<Aws::Crt::Io::DefaultHostResolver>(*event_loop_group_, 64, 30);

    // Initialize ClientBootstrap
    client_bootstrap_ = std::make_unique<Aws::Crt::Io::ClientBootstrap>(*event_loop_group_, *socket_resolver_);

    // Initialize IpcClientLifecycleHandler
    ipc_lifecycle_handler_ = std::make_unique<sv::IpcClientLifecycleHandler>();

    // Initialize GreengrassCoreIpcClient
    ipc_client_ = std::make_shared<Aws::Greengrass::GreengrassCoreIpcClient>(*client_bootstrap_);

    // Connect to IPC
    // Check IPC connection status
    if (auto connectionStatus = ipc_client_->Connect(*ipc_lifecycle_handler_).get(); !connectionStatus)
    {
      LOG(ERROR) << "Failed to establish IPC connection: " << connectionStatus.StatusToString();
      return Status::kFailure;
    }
    else
    {
      is_running_ = true;
    }

    return Status::kSuccess;
  }

  bool MqttHandler::IsRunning() const
  {
    return is_running_;
  }

  // Keep the main thread alive, or the process will exit.
  // Later, generalize this class to subscribe to mutliple topics
  Status MqttHandler::SubscribeToTopic(
      std::string topic_name, std::shared_ptr<Aws::Greengrass::SubscribeToIoTCoreStreamHandler> subscription_handler)
  {
    Aws::Crt::String topic(topic_name);
    QOS qos = Aws::Greengrass::QOS::QOS_AT_LEAST_ONCE;
    int timeout = 10;

    SubscribeToIoTCoreRequest request;
    request.SetTopicName(topic);
    request.SetQos(qos);

    if (subscription_handler == nullptr)
    {
      LOG(ERROR) << "Stream handler is not initialized";
      return Status::kNotInitialized;
    }

    subscribe_to_upload_operation_ = ipc_client_->NewSubscribeToIoTCore(subscription_handler);
    auto activate = subscribe_to_upload_operation_->Activate(request, nullptr);
    activate.wait();

    auto responseFuture = subscribe_to_upload_operation_->GetResult();
    if (responseFuture.wait_for(std::chrono::seconds(timeout)) == std::future_status::timeout)
    {
      LOG(ERROR) << "Operation timed out while waiting for response from Greengrass Core." << std::endl;
      return Status::kFailure;
    }

    if (auto response = responseFuture.get(); !response)
    {
      // Handle error.
      if (auto errorType = response.GetResultType(); errorType == OPERATION_ERROR)
      {
        auto *error = response.GetOperationError();
        (void)error;
        // Handle operation error.
        LOG(ERROR) << "Operation error: " << error->GetMessage().value() << std::endl;
        return Status::kFailure;
      }
      else
      {
        // Handle RPC error.
        LOG(ERROR) << "RPC error: " << response.GetRpcError() << std::endl;
        return Status::kFailure;
      }

      LOG(ERROR) << "Failed to subscribe to topic: " << topic << std::endl;
      return Status::kFailure;
    }

    return Status::kSuccess;
  }

} // namespace sc
