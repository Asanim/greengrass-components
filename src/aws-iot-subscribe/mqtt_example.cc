///
/// \brief
///
/// \file mqtt_example.cc
///

#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>
#include "mqtt_example.h"

#include <chrono>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <vector>


using namespace Aws::Crt;
using namespace Aws::Greengrass;

MqttExample::MqttExample() { Setup(); }

MqttExample::~MqttExample() { Stop(); }

ResultCode MqttExample::Stop()
{
  std::cout << "Closing the MQTT handler";
  if (subscribe_to_upload_operation_)
  {
    subscribe_to_upload_operation_->Close();
  }
  return ResultCode::kSuccess;
}

ResultCode MqttExample::Setup()
{
  // Retrieve environment variables
  thing_name_ = getenv("AWS_IOT_THING_NAME");
  ggc_version_ = getenv("GGC_VERSION");

  // Check if required environment variables are not set
  if (!thing_name_ || !ggc_version_)
  {
    // Log an error and disable datalogging
    std::cerr << "AWS Environment variables 'AWS_IOT_THING_NAME' 'GGC_VERSION' not set. You must run this as a Greengrass component";
    return ResultCode::kFailure;
  }
  // Log the retrieved thing name and GGC version
  std::cout << "Thing name: " << thing_name_ << "\nGGC version: " << ggc_version_;

  // Check if MQTT setup is successful
  if (MqttExample::SetupMQTT() != ResultCode::kSuccess)
  {
    // Log an error and disable datalogging
    std::cerr << "Could not initialize IoT Core. MQTT publish is disabled";
    return ResultCode::kFailure;
  }

  return ResultCode::kSuccess;
}

ResultCode MqttExample::SetupMQTT()
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
  ipc_lifecycle_handler_ = std::make_unique<IpcClientLifecycleHandler>();

  // Initialize GreengrassCoreIpcClient
  ipc_client_ = std::make_shared<Aws::Greengrass::GreengrassCoreIpcClient>(*client_bootstrap_);

  // Connect to IPC
  // Check IPC connection status
  if (auto connectionStatus = ipc_client_->Connect(*ipc_lifecycle_handler_).get(); !connectionStatus)
  {
    std::cerr << "Failed to establish IPC connection: " << connectionStatus.StatusToString();
    return ResultCode::kFailure;
  }
  else
  {
    is_running_ = true;
  }

  return ResultCode::kSuccess;
}

bool MqttExample::IsRunning() const
{
  return is_running_;
}

// Keep the main thread alive, or the process will exit.
// Later, generalize this class to subscribe to mutliple topics
ResultCode MqttExample::SubscribeToTopic(
    std::string topic_name, std::shared_ptr<Aws::Greengrass::SubscribeToIoTCoreStreamHandler> subscription_handler)
{
  Aws::Crt::String topic(topic_name);
  QOS qos = Aws::Greengrass::QOS::QOS_AT_LEAST_ONCE;
  int timeout = 10;

  SubscribeToIoTCoreRequest request;
  request.SetTopicName(topic);
  request.SetQos(qos);

  subscribe_to_upload_operation_ = ipc_client_->NewSubscribeToIoTCore(subscription_handler);
  auto activate = subscribe_to_upload_operation_->Activate(request, nullptr);
  activate.wait();

  auto responseFuture = subscribe_to_upload_operation_->GetResult();
  if (responseFuture.wait_for(std::chrono::seconds(timeout)) == std::future_status::timeout)
  {
    std::cerr << "Operation timed out while waiting for response from Greengrass Core." << std::endl;
    return ResultCode::kFailure;
  }

  if (auto response = responseFuture.get(); !response)
  {
    // Handle error.
    if (auto errorType = response.GetResultType(); errorType == OPERATION_ERROR)
    {
      auto *error = response.GetOperationError();
      (void)error;
      // Handle operation error.
      std::cerr << "Operation error: " << error->GetMessage().value() << std::endl;
      return ResultCode::kFailure;
    }
    else
    {
      // Handle RPC error.
      std::cerr << "RPC error: " << response.GetRpcError() << std::endl;
      return ResultCode::kFailure;
    }

    std::cerr << "Failed to subscribe to topic: " << topic << std::endl;
    return ResultCode::kFailure;
  }

  return ResultCode::kSuccess;
}