#pragma once

#include <aws/crt/Api.h>
#include <aws/greengrass/GreengrassCoreIpcClient.h>

#include <chrono>
#include <string>
#include <thread>
#include <memory>

using namespace Aws::Crt;
using namespace Aws::Greengrass;

/**
 * @brief Handles component update events from AWS Greengrass Core.
 *
 * This class subscribes to component update events and provides functionality
 * to defer updates when conditions are not safe for updating.
 */
class ComponentUpdateExample : public SubscribeToComponentUpdatesStreamHandler
{
public:
  /**
   * @brief Destructor
   */
  virtual ~ComponentUpdateExample() {}

  void SetIPCClient(
      std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient> ipc_client);

  /**
   * @brief Pointer to an instance of GreengrassCoreIpcClient for Greengrass Core IPC communication.
   */
  std::shared_ptr<Aws::Greengrass::GreengrassCoreIpcClient> ipc_client_;

private:
  /**
   * @brief Flag indicating if it's safe to perform an update
   */
  bool is_update_safe_ = false;

  /**
   * @brief Time in milliseconds to wait before rechecking if an update can proceed
   */
  int32_t component_update_recheck_ms_ = 5000;

  /**
   * @brief Current deployment ID
   */
  std::string deployment_id_ = "";

  /**
   * @brief Thread for handling update processes
   */
  std::unique_ptr<std::thread> update_thread_;

  /**
   * @brief Handles component update events
   *
   * @param response The component update policy event received
   */
  void OnStreamEvent(Aws::Greengrass::ComponentUpdatePolicyEvents *response) override;

  /**
   * @brief Handles RPC errors
   *
   * @param rpcError The RPC error that occurred
   * @return bool Whether to continue handling events
   */
  bool OnStreamError(RpcError rpcError) override;

  /**
   * @brief Handles service errors
   *
   * @param operationError The service error that occurred
   * @return bool Whether to continue handling events
   */
  bool OnStreamError(Aws::Greengrass::ServiceError *operationError) override;

  /**
   * @brief Handles resource not found errors
   *
   * @param operationError The resource not found error that occurred
   * @return bool Whether to continue handling events
   */
  bool OnStreamError(Aws::Greengrass::ResourceNotFoundError *operationError) override;

  /**
   * @brief Handles operation errors
   *
   * @param operationError The operation error that occurred
   * @return bool Whether to continue handling events
   */
  bool OnStreamError(Aws::Eventstreamrpc::OperationError *operationError) override;

  /**
   * @brief Sends a request to defer component update
   *
   * @param deployment_id The ID of the deployment to defer
   * @param recheck_after_ms Time in milliseconds to wait before rechecking
   */
  void DeferSystemComponentUpdate(const Aws::Crt::String &deployment_id, int32_t recheck_after_ms);
};
