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

#ifndef GENERATED_INTERFACE_JOYNR_TESTS_MULTIPLEVERSIONSINTERFACE1PROXYBASE_H
#define GENERATED_INTERFACE_JOYNR_TESTS_MULTIPLEVERSIONSINTERFACE1PROXYBASE_H

#include "joynr/PrivateCopyAssign.h"
#include "joynr/tests/InterfaceVersionedStruct1.h"
#include <cstdint>
#include "joynr/tests/MultipleVersionsTypeCollection/VersionedStruct1.h"
#include <string>
#include "joynr/tests/AnonymousVersionedStruct1.h"
#include <memory>

#include "joynr/ProxyBase.h"
#include "joynr/tests/IMultipleVersionsInterface1Connector.h"

namespace joynr
{
namespace types
{
	class DiscoveryEntryWithMetaInfo;
} // namespace types
} // namespace joynr

namespace joynr { namespace tests { 
/**
 * @brief Proxy base class for interface MultipleVersionsInterface1
 *
 * @version 1.0
 */
class  MultipleVersionsInterface1ProxyBase: virtual public joynr::ProxyBase, virtual public joynr::tests::IMultipleVersionsInterface1Subscription {
public:
	/**
	 * @brief Parameterized constructor
	 * @param connectorFactory The connector factory
	 * @param domain The provider domain
	 * @param qosSettings The quality of service settings
	 */
	MultipleVersionsInterface1ProxyBase(
			std::weak_ptr<joynr::JoynrRuntimeImpl> runtime,
			std::shared_ptr<joynr::JoynrMessagingConnectorFactory> connectorFactory,
			const std::string& domain,
			const joynr::MessagingQos& qosSettings
	);

	/**
	 * @brief Called when arbitration is finished
	 * @param participantId The id of the participant
	 * @param connection The kind of connection
	 */
	void handleArbitrationFinished(
			const joynr::types::DiscoveryEntryWithMetaInfo& providerDiscoveryEntry
	) override;

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
	

protected:
	/** @brief The kind of connector */
	std::unique_ptr<IMultipleVersionsInterface1Connector> connector;

private:
	DISALLOW_COPY_AND_ASSIGN(MultipleVersionsInterface1ProxyBase);
};

} // namespace tests
} // namespace joynr
#endif // GENERATED_INTERFACE_JOYNR_TESTS_MULTIPLEVERSIONSINTERFACE1PROXYBASE_H
