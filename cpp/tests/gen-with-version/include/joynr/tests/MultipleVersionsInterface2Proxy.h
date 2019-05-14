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

#ifndef GENERATED_INTERFACE_JOYNR_TESTS_MULTIPLEVERSIONSINTERFACE2PROXY_H
#define GENERATED_INTERFACE_JOYNR_TESTS_MULTIPLEVERSIONSINTERFACE2PROXY_H

#include "joynr/PrivateCopyAssign.h"
#include "joynr/tests/InterfaceVersionedStruct2.h"
#include <cstdint>
#include "joynr/tests/MultipleVersionsTypeCollection/VersionedStruct2.h"
#include <string>
#include "joynr/tests/AnonymousVersionedStruct2.h"
#include <memory>

#include "joynr/tests/MultipleVersionsInterface2SyncProxy.h"
#include "joynr/tests/MultipleVersionsInterface2AsyncProxy.h"
#include "joynr/tests/IMultipleVersionsInterface2.h"

#ifdef _MSC_VER
	// Visual C++ gives a warning which is caused by diamond inheritance, but this is
	// not relevant when using pure virtual methods:
	// http://msdn.microsoft.com/en-us/library/6b3sy7ae(v=vs.80).aspx
	#pragma warning( disable : 4250 )
#endif

namespace joynr { namespace tests { 
/**
 * @brief Proxy class for interface MultipleVersionsInterface2
 *
 * @version 2.0
 */
class  MultipleVersionsInterface2Proxy : virtual public IMultipleVersionsInterface2, virtual public MultipleVersionsInterface2SyncProxy, virtual public MultipleVersionsInterface2AsyncProxy {
public:
	/**
	 * @brief Parameterized constructor
	 * @param connectorFactory The connector factory
	 * @param domain The provider domain
	 * @param qosSettings The quality of service settings
	 */
	MultipleVersionsInterface2Proxy(
			std::weak_ptr<joynr::JoynrRuntimeImpl> runtime,
			std::shared_ptr<joynr::JoynrMessagingConnectorFactory> connectorFactory,
			const std::string& domain,
			const joynr::MessagingQos& qosSettings
	);

	/**
	 * @brief unsubscribes from attribute UInt8Attribute1
	 * @param subscriptionId The subscription id returned earlier on creation of the subscription
	 */
	void unsubscribeFromUInt8Attribute1(const std::string &subscriptionId) override {
		MultipleVersionsInterface2ProxyBase::unsubscribeFromUInt8Attribute1(subscriptionId);
	}

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
	 override {
		return MultipleVersionsInterface2ProxyBase::subscribeToUInt8Attribute1(
					subscriptionListener,
					subscriptionQos);
	}

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
	 override{
		return MultipleVersionsInterface2ProxyBase::subscribeToUInt8Attribute1(
					subscriptionListener,
					subscriptionQos,
					subscriptionId);
	}

	/**
	 * @brief unsubscribes from attribute UInt8Attribute2
	 * @param subscriptionId The subscription id returned earlier on creation of the subscription
	 */
	void unsubscribeFromUInt8Attribute2(const std::string &subscriptionId) override {
		MultipleVersionsInterface2ProxyBase::unsubscribeFromUInt8Attribute2(subscriptionId);
	}

	/**
	 * @brief creates a new subscription to attribute UInt8Attribute2
	 * @param subscriptionListener The listener callback providing methods to call on publication and failure
	 * @param subscriptionQos The subscription quality of service settings
	 * @return a future representing the result (subscription id) as string. It provides methods to wait for
	 * completion, to get the subscription id or the request status object. The subscription id will be available
	 * when the subscription is successfully registered at the provider.
	 */
	std::shared_ptr<joynr::Future<std::string>> subscribeToUInt8Attribute2(
				std::shared_ptr<joynr::ISubscriptionListener<std::uint8_t> > subscriptionListener,
				std::shared_ptr<joynr::SubscriptionQos> subscriptionQos)
	 override {
		return MultipleVersionsInterface2ProxyBase::subscribeToUInt8Attribute2(
					subscriptionListener,
					subscriptionQos);
	}

	/**
	 * @brief updates an existing subscription to attribute UInt8Attribute2
	 * @param subscriptionListener The listener callback providing methods to call on publication and failure
	 * @param subscriptionQos The subscription quality of service settings
	 * @param subscriptionId The subscription id returned earlier on creation of the subscription
	 * @return a future representing the result (subscription id) as string. It provides methods to wait for
	 * completion, to get the subscription id or the request status object. The subscription id will be available
	 * when the subscription is successfully registered at the provider.
	 */
	std::shared_ptr<joynr::Future<std::string>> subscribeToUInt8Attribute2(
				std::shared_ptr<joynr::ISubscriptionListener<std::uint8_t> > subscriptionListener,
				std::shared_ptr<joynr::SubscriptionQos> subscriptionQos,
				const std::string& subscriptionId)
	 override{
		return MultipleVersionsInterface2ProxyBase::subscribeToUInt8Attribute2(
					subscriptionListener,
					subscriptionQos,
					subscriptionId);
	}


	/** @brief Destructor */
	~MultipleVersionsInterface2Proxy() override = default;

	// attributes
	using MultipleVersionsInterface2AsyncProxy::getUInt8Attribute1Async;
	using MultipleVersionsInterface2SyncProxy::getUInt8Attribute1;
	using MultipleVersionsInterface2AsyncProxy::setUInt8Attribute1Async;
	using MultipleVersionsInterface2SyncProxy::setUInt8Attribute1;
	using MultipleVersionsInterface2AsyncProxy::getUInt8Attribute2Async;
	using MultipleVersionsInterface2SyncProxy::getUInt8Attribute2;
	using MultipleVersionsInterface2AsyncProxy::setUInt8Attribute2Async;
	using MultipleVersionsInterface2SyncProxy::setUInt8Attribute2;

	using IMultipleVersionsInterface2Sync::getTrue;
	using IMultipleVersionsInterface2Async::getTrueAsync;
	using IMultipleVersionsInterface2Sync::getAnonymousVersionedStruct;
	using IMultipleVersionsInterface2Async::getAnonymousVersionedStructAsync;
	using IMultipleVersionsInterface2Sync::getInterfaceVersionedStruct;
	using IMultipleVersionsInterface2Async::getInterfaceVersionedStructAsync;
	using IMultipleVersionsInterface2Sync::getVersionedStruct;
	using IMultipleVersionsInterface2Async::getVersionedStructAsync;
private:
	DISALLOW_COPY_AND_ASSIGN(MultipleVersionsInterface2Proxy);
};


} // namespace tests
} // namespace joynr

#endif // GENERATED_INTERFACE_JOYNR_TESTS_MULTIPLEVERSIONSINTERFACE2PROXY_H
