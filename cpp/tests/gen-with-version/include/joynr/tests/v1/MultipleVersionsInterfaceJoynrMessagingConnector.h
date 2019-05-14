/*
 *
 * Copyright (C) 2011 - 2018 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// #####################################################
//#######################################################
//###                                                 ###
//##    WARNING: This file is generated. DO NOT EDIT   ##
//##             All changes will be lost!             ##
//###                                                 ###
//#######################################################
// #####################################################

#ifndef GENERATED_INTERFACE_JOYNR_TESTS_V1_MULTIPLEVERSIONSINTERFACEJOYNRMESSAGINGCONNECTOR_H
#define GENERATED_INTERFACE_JOYNR_TESTS_V1_MULTIPLEVERSIONSINTERFACEJOYNRMESSAGINGCONNECTOR_H

#include <cstdint>
#include "joynr/tests/v1/AnonymousVersionedStruct.h"
#include "joynr/tests/v1/MultipleVersionsTypeCollection/VersionedStruct.h"
#include "joynr/tests/v1/InterfaceVersionedStruct.h"
#include <string>

#include <memory>
#include <functional>
#include "joynr/tests/v1/IMultipleVersionsInterfaceConnector.h"
#include "joynr/AbstractJoynrMessagingConnector.h"
#include "joynr/JoynrMessagingConnectorFactory.h"
#include "joynr/SubscriptionQos.h"
#include "joynr/OnChangeSubscriptionQos.h"
#include "joynr/MulticastSubscriptionQos.h"

namespace joynr {
	class MessagingQos;
	class IMessageSender;
	class ISubscriptionManager;
	class SubscriptionRequest;
	template <class ... Ts> class Future;
	template <typename... Ts> class ISubscriptionListener;

namespace types
{
	class DiscoveryEntryWithMetaInfo;
} // namespace types

namespace exceptions
{
	class JoynrException;
	class JoynrRuntimeException;
} // namespace exceptions
}

namespace joynr { namespace tests { namespace v1 { 


/** @brief JoynrMessagingConnector for interface MultipleVersionsInterface */
class  MultipleVersionsInterfaceJoynrMessagingConnector : public IMultipleVersionsInterfaceConnector, virtual public joynr::AbstractJoynrMessagingConnector {
private:
	/**
	 * @brief creates a new subscription or updates an existing subscription to attribute 
	 * UInt8Attribute1
	 * @param subscriptionListener The listener callback providing methods to call on publication and failure
	 * @param subscriptionQos The subscription quality of service settings
	 * @param subscriptionRequest The subscription request
	 * @return a future representing the result (subscription id) as string. It provides methods to wait for
	 			 * completion, to get the subscription id or the request status object. The subscription id will be available
	 			 * when the subscription is successfully registered at the provider.
	 */
	std::shared_ptr<joynr::Future<std::string>> subscribeToUInt8Attribute1(
			std::shared_ptr<joynr::ISubscriptionListener<std::uint8_t> > subscriptionListener,
			std::shared_ptr<joynr::SubscriptionQos> subscriptionQos,
			SubscriptionRequest& subscriptionRequest);
public:
	/**
	 * @brief Parameterized constructor
	 * @param messageSender The message sender
	 * @param subscriptionManager Pointer to subscription manager instance
	 * @param domain the provider domain
	 * @param proxyParticipantId The participant id of the proxy
	 * @param providerParticipantId The participant id of the provider
	 * @param qosSettings The quality of service settings
	 */
	MultipleVersionsInterfaceJoynrMessagingConnector(
		std::weak_ptr<joynr::IMessageSender> messageSender,
		std::weak_ptr<joynr::ISubscriptionManager> subscriptionManager,
		const std::string& domain,
		const std::string& proxyParticipantId,
		const joynr::MessagingQos &qosSettings,
		const joynr::types::DiscoveryEntryWithMetaInfo& providerDiscoveryInfo);

	/** @brief Destructor */
	~MultipleVersionsInterfaceJoynrMessagingConnector() override = default;

	
	/**
	* @brief Synchronous getter for the uInt8Attribute1 attribute.
	*
	* @param result The result that will be returned to the caller.
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @throws JoynrException if the request is not successful
	*/
	void getUInt8Attribute1(std::uint8_t& uInt8Attribute1, boost::optional<joynr::MessagingQos> qos = boost::none)
	override;
	
	
	/**
	* @brief Asynchronous getter for the uInt8Attribute1 attribute.
	*
	* @param onSuccess A callback function to be called once the asynchronous computation has
	* finished successfully. It must expect the method out parameters.
	* @param onRuntimeError A callback function to be called once the asynchronous computation has
	* failed with an unexpected non-modeled exception. It must expect a JoynrRuntimeException object.
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @returns A future representing the result of the asynchronous method call. It provides methods
	* to wait for completion, to get the result or the request status object.
	*/
	std::shared_ptr<joynr::Future<std::uint8_t> > getUInt8Attribute1Async(
				std::function<void(const std::uint8_t& uInt8Attribute1)> onSuccess = nullptr,
				std::function<void(const joynr::exceptions::JoynrRuntimeException& error)> onError = nullptr,
				boost::optional<joynr::MessagingQos> qos = boost::none)
				noexcept
	override;
	
	/**
	* @brief Synchronous setter for the uInt8Attribute1 attribute.
	*
	* @param uInt8Attribute1 The value to set.
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @throws JoynrException if the request is not successful
	*/
	void setUInt8Attribute1(const std::uint8_t& uInt8Attribute1, boost::optional<joynr::MessagingQos> qos = boost::none)
	override;
	
	/**
	* @brief Asynchronous setter for the uInt8Attribute1 attribute.
	*
	* @param uInt8Attribute1 The value to set.
	* @param onSuccess A callback function to be called once the asynchronous computation has
	* finished successfully. It must expect the method out parameters.
	* @param onRuntimeError A callback function to be called once the asynchronous computation has
	* failed with an unexpected non-modeled exception. It must expect a JoynrRuntimeException object.
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @returns A future representing the result of the asynchronous method call. It provides methods
	* to wait for completion, to get the result or the request status object.
	*/
	std::shared_ptr<joynr::Future<void> > setUInt8Attribute1Async(
				std::uint8_t uInt8Attribute1,
				std::function<void(void)> onSuccess = nullptr,
				std::function<void(const joynr::exceptions::JoynrRuntimeException& error)> onError = nullptr,
				boost::optional<joynr::MessagingQos> qos = boost::none)
				noexcept
	override;

	/**
	* @brief Synchronous operation getTrue.
	*
	* @param bool result this is an output parameter
	*        and will be set within function getTrue
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @throws JoynrException if the request is not successful
	*/
	void getTrue(
				bool& result,
				boost::optional<joynr::MessagingQos> qos = boost::none
	)
	override;
	/**
	* @brief Synchronous operation getVersionedStruct.
	*
	* @param joynr::tests::v1::MultipleVersionsTypeCollection::VersionedStruct result this is an output parameter
	*        and will be set within function getVersionedStruct
	* @param joynr::tests::v1::MultipleVersionsTypeCollection::VersionedStruct input
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @throws JoynrException if the request is not successful
	*/
	void getVersionedStruct(
				joynr::tests::v1::MultipleVersionsTypeCollection::VersionedStruct& result,
				const joynr::tests::v1::MultipleVersionsTypeCollection::VersionedStruct& input,
				boost::optional<joynr::MessagingQos> qos = boost::none
	)
	override;
	/**
	* @brief Synchronous operation getAnonymousVersionedStruct.
	*
	* @param joynr::tests::v1::AnonymousVersionedStruct result this is an output parameter
	*        and will be set within function getAnonymousVersionedStruct
	* @param joynr::tests::v1::AnonymousVersionedStruct input
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @throws JoynrException if the request is not successful
	*/
	void getAnonymousVersionedStruct(
				joynr::tests::v1::AnonymousVersionedStruct& result,
				const joynr::tests::v1::AnonymousVersionedStruct& input,
				boost::optional<joynr::MessagingQos> qos = boost::none
	)
	override;
	/**
	* @brief Synchronous operation getInterfaceVersionedStruct.
	*
	* @param joynr::tests::v1::InterfaceVersionedStruct result this is an output parameter
	*        and will be set within function getInterfaceVersionedStruct
	* @param joynr::tests::v1::InterfaceVersionedStruct input
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @throws JoynrException if the request is not successful
	*/
	void getInterfaceVersionedStruct(
				joynr::tests::v1::InterfaceVersionedStruct& result,
				const joynr::tests::v1::InterfaceVersionedStruct& input,
				boost::optional<joynr::MessagingQos> qos = boost::none
	)
	override;
	/**
	* @brief Asynchronous operation getTrue.
	*
	* @param onSuccess A callback function to be called once the asynchronous computation has
	* finished successfully. It must expect the method out parameters.
	* @param onRuntimeError A callback function to be called once the asynchronous computation has
	* failed with an unexpected non-modeled exception. It must expect a JoynrRuntimeException object.
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @returns A future representing the result of the asynchronous method call. It provides methods
	* to wait for completion, to get the result or the request status object.
	*/
	std::shared_ptr<joynr::Future<bool>>  getTrueAsync(
				std::function<void(const bool& result)> onSuccess = nullptr,
				std::function<void(const joynr::exceptions::JoynrRuntimeException& error)> onRuntimeError = nullptr,
				boost::optional<joynr::MessagingQos> qos = boost::none
	) noexcept
	override;
	/**
	* @brief Asynchronous operation getVersionedStruct.
	*
	* @param onSuccess A callback function to be called once the asynchronous computation has
	* finished successfully. It must expect the method out parameters.
	* @param onRuntimeError A callback function to be called once the asynchronous computation has
	* failed with an unexpected non-modeled exception. It must expect a JoynrRuntimeException object.
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @returns A future representing the result of the asynchronous method call. It provides methods
	* to wait for completion, to get the result or the request status object.
	*/
	std::shared_ptr<joynr::Future<joynr::tests::v1::MultipleVersionsTypeCollection::VersionedStruct>>  getVersionedStructAsync(
				const joynr::tests::v1::MultipleVersionsTypeCollection::VersionedStruct& input,
				std::function<void(const joynr::tests::v1::MultipleVersionsTypeCollection::VersionedStruct& result)> onSuccess = nullptr,
				std::function<void(const joynr::exceptions::JoynrRuntimeException& error)> onRuntimeError = nullptr,
				boost::optional<joynr::MessagingQos> qos = boost::none
	) noexcept
	override;
	/**
	* @brief Asynchronous operation getAnonymousVersionedStruct.
	*
	* @param onSuccess A callback function to be called once the asynchronous computation has
	* finished successfully. It must expect the method out parameters.
	* @param onRuntimeError A callback function to be called once the asynchronous computation has
	* failed with an unexpected non-modeled exception. It must expect a JoynrRuntimeException object.
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @returns A future representing the result of the asynchronous method call. It provides methods
	* to wait for completion, to get the result or the request status object.
	*/
	std::shared_ptr<joynr::Future<joynr::tests::v1::AnonymousVersionedStruct>>  getAnonymousVersionedStructAsync(
				const joynr::tests::v1::AnonymousVersionedStruct& input,
				std::function<void(const joynr::tests::v1::AnonymousVersionedStruct& result)> onSuccess = nullptr,
				std::function<void(const joynr::exceptions::JoynrRuntimeException& error)> onRuntimeError = nullptr,
				boost::optional<joynr::MessagingQos> qos = boost::none
	) noexcept
	override;
	/**
	* @brief Asynchronous operation getInterfaceVersionedStruct.
	*
	* @param onSuccess A callback function to be called once the asynchronous computation has
	* finished successfully. It must expect the method out parameters.
	* @param onRuntimeError A callback function to be called once the asynchronous computation has
	* failed with an unexpected non-modeled exception. It must expect a JoynrRuntimeException object.
	* @param qos optional MessagingQos parameter; if specified, this will overwrite the MessagingQos that
	* was specified when building the proxy.
	* @returns A future representing the result of the asynchronous method call. It provides methods
	* to wait for completion, to get the result or the request status object.
	*/
	std::shared_ptr<joynr::Future<joynr::tests::v1::InterfaceVersionedStruct>>  getInterfaceVersionedStructAsync(
				const joynr::tests::v1::InterfaceVersionedStruct& input,
				std::function<void(const joynr::tests::v1::InterfaceVersionedStruct& result)> onSuccess = nullptr,
				std::function<void(const joynr::exceptions::JoynrRuntimeException& error)> onRuntimeError = nullptr,
				boost::optional<joynr::MessagingQos> qos = boost::none
	) noexcept
	override;

	/**
	 * @brief creates a new subscription to attribute UInt8Attribute1
	 * @param subscriptionListener The listener callback providing methods to call on publication and failure
	 * @param subscriptionQos The subscription quality of service settings
	 * @return a future representing the result (subscription id) as string. It provides methods to wait for
	 * completion, to get the subscription id or the request status object. The subscription id will be available
	 * when the subscription is successfully registered at the provider.
	 */
	std::shared_ptr<joynr::Future<std::string>> subscribeToUInt8Attribute1(
				std::shared_ptr<joynr::ISubscriptionListener<std::uint8_t> > subscriptionListener,
				std::shared_ptr<joynr::SubscriptionQos> subscriptionQos)
	 override;
	
	/**
	 * @brief updates an existing subscription to attribute UInt8Attribute1
	 * @param subscriptionListener The listener callback providing methods to call on publication and failure
	 * @param subscriptionQos The subscription quality of service settings
	 * @param subscriptionId The subscription id returned earlier on creation of the subscription
	 * @return a future representing the result (subscription id) as string. It provides methods to wait for
	 * completion, to get the subscription id or the request status object. The subscription id will be available
	 * when the subscription is successfully registered at the provider.
	 */
	std::shared_ptr<joynr::Future<std::string>> subscribeToUInt8Attribute1(
				std::shared_ptr<joynr::ISubscriptionListener<std::uint8_t> > subscriptionListener,
				std::shared_ptr<joynr::SubscriptionQos> subscriptionQos,
				const std::string& subscriptionId)
	 override;
	
	/**
	 * @brief unsubscribes from attribute UInt8Attribute1
	 * @param subscriptionId The subscription id returned earlier on creation of the subscription
	 */
	void unsubscribeFromUInt8Attribute1(const std::string& subscriptionId)
	 override;
	
};

} // namespace v1
} // namespace tests
} // namespace joynr


namespace joynr {

// specialization of traits class JoynrMessagingTraits
// this links IMultipleVersionsInterfaceConnector with MultipleVersionsInterfaceJoynrMessagingConnector
template <>
struct JoynrMessagingTraits<joynr::tests::v1::IMultipleVersionsInterfaceConnector>
{
	using Connector = joynr::tests::v1::MultipleVersionsInterfaceJoynrMessagingConnector;
};

} // namespace joynr
#endif // GENERATED_INTERFACE_JOYNR_TESTS_V1_MULTIPLEVERSIONSINTERFACEJOYNRMESSAGINGCONNECTOR_H
