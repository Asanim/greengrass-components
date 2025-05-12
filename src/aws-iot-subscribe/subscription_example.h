///
/// \brief Manages communication with AWS IoT Greengrass Nucleus
///
/// \file request_upload_handler.h
///

#ifndef SUBSCRIPTION_HANDLER_H_
#define SUBSCRIPTION_HANDLER_H_

#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>

class MqttHandler; // Forward declaration

/// \brief The SubscriptionHandler class.
///
/// This class is responsible for handling message topic responses from the Greengrass Core IPC client

class SubscriptionHandler : public Aws::Greengrass::SubscribeToIoTCoreStreamHandler
{
private:
  /// \brief Called when the IPC client receives a message from the service.
  ///
  /// \param response The message received from the service.
  ///
  void OnStreamEvent(Aws::Greengrass::IoTCoreMessage *response) override;

  /// \brief Called when the IPC client disconnects from the service.
  ///
  /// \param error An object representing the error, if any, during disconnection.
  /// \return true Error has been handled. Here, we simply log the fault to file.
  /// \return false Error has not been handled
  ///
  bool OnStreamError(Aws::Eventstreamrpc::OperationError *error) override;

  /// \brief Called when the IPC client closes the stream.
  ///
  void OnStreamClosed() override;
};

#endif // SUBSCRIPTION_HANDLER_H_
