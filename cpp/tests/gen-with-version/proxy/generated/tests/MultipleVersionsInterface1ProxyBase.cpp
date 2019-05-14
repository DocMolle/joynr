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

#include "joynr/tests/MultipleVersionsInterface1ProxyBase.h"
#include "joynr/ISubscriptionListener.h"
#include "joynr/types/DiscoveryEntryWithMetaInfo.h"
#include "joynr/JoynrMessagingConnectorFactory.h"
#include "joynr/tests/MultipleVersionsInterface1JoynrMessagingConnector.h"

namespace joynr { namespace tests { 
MultipleVersionsInterface1ProxyBase::MultipleVersionsInterface1ProxyBase(
		std::weak_ptr<joynr::JoynrRuntimeImpl> runtime,
		std::shared_ptr<joynr::JoynrMessagingConnectorFactory> connectorFactory,
		const std::string &domain,
		const joynr::MessagingQos &qosSettings
) :
		joynr::ProxyBase(std::move(runtime), std::move(connectorFactory), domain, qosSettings),
		connector()
{
}

void MultipleVersionsInterface1ProxyBase::handleArbitrationFinished(
		const joynr::types::DiscoveryEntryWithMetaInfo& providerDiscoveryEntry
) {
	connector = connectorFactory->create<joynr::tests::IMultipleVersionsInterface1Connector>(
				domain,
				proxyParticipantId,
				qosSettings,
				providerDiscoveryEntry
	);

	joynr::ProxyBase::handleArbitrationFinished(providerDiscoveryEntry);
}

void MultipleVersionsInterface1ProxyBase::unsubscribeFromUInt8Attribute1(const std::string& subscriptionId)
{
	auto runtimeSharedPtr = runtime.lock();
	if (!runtimeSharedPtr || !connector) {
		if (!runtimeSharedPtr) {
			JOYNR_LOG_WARN(logger(), "proxy cannot unsubscribe from MultipleVersionsInterface1ProxyBase.uInt8Attribute1, "
					 "because the required runtime has been already destroyed.");
			return;
		} else {
			JOYNR_LOG_WARN(logger(), "proxy cannot unsubscribe from MultipleVersionsInterface1ProxyBase.uInt8Attribute1, "
					 "because the communication end partner is not (yet) known");
			return;
		}
	}
	connector->unsubscribeFromUInt8Attribute1(subscriptionId);
}

std::shared_ptr<joynr::Future<std::string>> MultipleVersionsInterface1ProxyBase::subscribeToUInt8Attribute1(
			std::shared_ptr<joynr::ISubscriptionListener<std::uint8_t> > subscriptionListener,
			std::shared_ptr<joynr::SubscriptionQos> subscriptionQos,
			const std::string& subscriptionId)
 {
	auto runtimeSharedPtr = runtime.lock();
	if (!runtimeSharedPtr || !connector) {
		std::string errorMsg;
		if (!runtimeSharedPtr) {
			errorMsg = "proxy cannot subscribe to MultipleVersionsInterface1ProxyBase.uInt8Attribute1, "
					"because the required runtime has been already destroyed.";
		} else {
			errorMsg = "proxy cannot subscribe to MultipleVersionsInterface1ProxyBase.uInt8Attribute1, "
					"because the communication end partner is not (yet) known";
		}
		JOYNR_LOG_WARN(logger(), errorMsg);
		auto error = std::make_shared<exceptions::JoynrRuntimeException>(errorMsg);
		auto future = std::make_shared<Future<std::string>>();
		future->onError(error);
		subscriptionListener->onError(*error);
		return future;
	}
	return connector->subscribeToUInt8Attribute1(
				subscriptionListener,
				subscriptionQos,
				subscriptionId);
}

std::shared_ptr<joynr::Future<std::string>> MultipleVersionsInterface1ProxyBase::subscribeToUInt8Attribute1(
			std::shared_ptr<joynr::ISubscriptionListener<std::uint8_t> > subscriptionListener,
			std::shared_ptr<joynr::SubscriptionQos> subscriptionQos)
 {
	auto runtimeSharedPtr = runtime.lock();
	if (!runtimeSharedPtr || !connector) {
		std::string errorMsg;
		if (!runtimeSharedPtr) {
			errorMsg = "proxy cannot subscribe to MultipleVersionsInterface1ProxyBase.uInt8Attribute1, "
					 "because the required runtime has been already destroyed.";
		} else {
			errorMsg = "proxy cannot subscribe to MultipleVersionsInterface1ProxyBase.uInt8Attribute1, "
					 "because the communication end partner is not (yet) known";
		}
		JOYNR_LOG_WARN(logger(), errorMsg);
		auto error = std::make_shared<exceptions::JoynrRuntimeException>(errorMsg);
		auto future = std::make_shared<Future<std::string>>();
		future->onError(error);
		subscriptionListener->onError(*error);
		return future;
	}
	return connector->subscribeToUInt8Attribute1(
				subscriptionListener,
				subscriptionQos);
}




} // namespace tests
} // namespace joynr
