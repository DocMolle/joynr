/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2019 BMW Car IT GmbH
 * %%
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
 * #L%
 */

#include "joynr/CcMessageRouter.h"

#include <cassert>
#include <functional>
#include <typeinfo>

#include "joynr/ClusterControllerSettings.h"
#include "joynr/IMessageSender.h"
#include "joynr/IMessagingMulticastSubscriber.h"
#include "joynr/IMessagingStubFactory.h"
#include "joynr/IMulticastAddressCalculator.h"
#include "joynr/IPlatformSecurityManager.h"
#include "joynr/ImmutableMessage.h"
#include "joynr/InProcessMessagingAddress.h"
#include "joynr/Message.h"
#include "joynr/MessageQueue.h"
#include "joynr/MulticastMessagingSkeletonDirectory.h"
#include "joynr/MulticastReceiverDirectory.h"
#include "joynr/SubscriptionPublication.h"
#include "joynr/SubscriptionStop.h"
#include "joynr/Util.h"
#include "joynr/access-control/IAccessController.h"
#include "joynr/exceptions/JoynrException.h"
#include "joynr/serializer/Serializer.h"
#include "joynr/system/MessageNotificationAbstractProvider.h"
#include "joynr/system/MessageNotificationMessageQueuedForDeliveryBroadcastFilter.h"
#include "joynr/system/MessageNotificationMessageQueuedForDeliveryBroadcastFilterParameters.h"
#include "joynr/system/RoutingProxy.h"
#include "joynr/system/RoutingTypes/Address.h"
#include "joynr/system/RoutingTypes/BrowserAddress.h"
#include "joynr/system/RoutingTypes/ChannelAddress.h"
#include "joynr/system/RoutingTypes/MqttAddress.h"
#include "joynr/system/RoutingTypes/WebSocketAddress.h"
#include "joynr/system/RoutingTypes/WebSocketClientAddress.h"

namespace joynr
{

//------ ConsumerPermissionCallback --------------------------------------------

class ConsumerPermissionCallback : public IAccessController::IHasConsumerPermissionCallback
{
public:
    ConsumerPermissionCallback(
            std::weak_ptr<CcMessageRouter> owningMessageRouter,
            std::shared_ptr<ImmutableMessage> message,
            std::shared_ptr<const joynr::system::RoutingTypes::Address> destination,
            bool aclAudit,
            std::uint32_t tryCount);

    void hasConsumerPermission(IAccessController::Enum hasPermission);

    std::weak_ptr<CcMessageRouter> _owningMessageRouter;
    std::shared_ptr<ImmutableMessage> _message;
    std::shared_ptr<const joynr::system::RoutingTypes::Address> _destination;

private:
    const bool _aclAudit;
    std::uint32_t _tryCount;
    ADD_LOGGER(ConsumerPermissionCallback)
};

//------ MessageNotification ---------------------------------------------------

class CcMessageNotificationProvider : public joynr::system::MessageNotificationAbstractProvider
{
public:
    virtual ~CcMessageNotificationProvider() = default;
    using MessageNotificationAbstractProvider::fireMessageQueuedForDelivery;
};

class MessageQueuedForDeliveryBroadcastFilter
        : public joynr::system::MessageNotificationMessageQueuedForDeliveryBroadcastFilter
{
    bool filter(const std::string& participantId,
                const std::string& messageType,
                const joynr::system::
                        MessageNotificationMessageQueuedForDeliveryBroadcastFilterParameters&
                                filterParameters) override
    {
        const bool isParticipantIdSet = !filterParameters.getParticipantId().empty();
        const bool isMessageTypeSet = !filterParameters.getMessageType().empty();

        // if no filter parameters are set, always send broadcast
        if (!isParticipantIdSet && !isMessageTypeSet) {
            return true;
        }
        // if message type is empty, check if participant id matches
        if (!isMessageTypeSet) {
            return filterParameters.getParticipantId() == participantId;
        }
        // if participant type is empty, check if message type matches
        if (!isParticipantIdSet) {
            return filterParameters.getMessageType() == messageType;
        }
        return filterParameters.getParticipantId() == participantId &&
               filterParameters.getMessageType() == messageType;
    }
};

//------ MessageRouter ---------------------------------------------------------

CcMessageRouter::CcMessageRouter(
        MessagingSettings& messagingSettings,
        ClusterControllerSettings& clusterControllerSettings,
        std::shared_ptr<IMessagingStubFactory> messagingStubFactory,
        std::shared_ptr<MulticastMessagingSkeletonDirectory> multicastMessagingSkeletonDirectory,
        std::unique_ptr<IPlatformSecurityManager> securityManager,
        boost::asio::io_service& ioService,
        std::unique_ptr<IMulticastAddressCalculator> addressCalculator,
        const std::string& globalClusterControllerAddress,
        const std::string& messageNotificationProviderParticipantId,
        bool persistRoutingTable,
        std::vector<std::shared_ptr<ITransportStatus>> transportStatuses,
        std::unique_ptr<MessageQueue<std::string>> messageQueue,
        std::unique_ptr<MessageQueue<std::shared_ptr<ITransportStatus>>> transportNotAvailableQueue,
        const system::RoutingTypes::Address& ownGlobalAddress)
        : AbstractMessageRouter(messagingSettings,
                                std::move(messagingStubFactory),
                                ioService,
                                std::move(addressCalculator),
                                persistRoutingTable,
                                std::move(transportStatuses),
                                std::move(messageQueue),
                                std::move(transportNotAvailableQueue)),
          joynr::system::RoutingAbstractProvider(),
          _multicastMessagingSkeletonDirectory(multicastMessagingSkeletonDirectory),
          _securityManager(std::move(securityManager)),
          _accessController(),
          _multicastReceiverDirectoryFilename(),
          _globalClusterControllerAddress(globalClusterControllerAddress),
          _messageNotificationProvider(std::make_shared<CcMessageNotificationProvider>()),
          _messageNotificationProviderParticipantId(messageNotificationProviderParticipantId),
          _clusterControllerSettings(clusterControllerSettings),
          _multicastReceiverDirectoryPersistencyEnabled(
                  clusterControllerSettings.isMulticastReceiverDirectoryPersistencyEnabled()),
          _ownGlobalAddress(ownGlobalAddress),
          _messageSender()
{
    _messageNotificationProvider->addBroadcastFilter(
            std::make_shared<MessageQueuedForDeliveryBroadcastFilter>());
}

CcMessageRouter::~CcMessageRouter()
{
}

void CcMessageRouter::setAccessController(std::weak_ptr<IAccessController> accessController)
{
    this->_accessController = std::move(accessController);
}

void CcMessageRouter::setMessageSender(std::weak_ptr<IMessageSender> messageSender)
{
    this->_messageSender = std::move(messageSender);
}

void CcMessageRouter::saveMulticastReceiverDirectory() const
{
    if (!_multicastReceiverDirectoryPersistencyEnabled) {
        return;
    }

    if (_multicastReceiverDirectoryFilename.empty()) {
        JOYNR_LOG_INFO(
                logger(), "Did not save multicast receiver directory: No filename specified");
        return;
    }

    try {
        joynr::util::saveStringToFile(
                _multicastReceiverDirectoryFilename,
                joynr::serializer::serializeToJson(_multicastReceiverDirectory));
    } catch (const std::runtime_error& ex) {
        JOYNR_LOG_INFO(logger(), ex.what());
    }
}

void CcMessageRouter::loadMulticastReceiverDirectory(std::string filename)
{
    if (!_multicastReceiverDirectoryPersistencyEnabled) {
        return;
    }

    _multicastReceiverDirectoryFilename = std::move(filename);

    if (_multicastReceiverDirectoryFilename.empty()) {
        JOYNR_LOG_INFO(
                logger(), "Did not load multicast receiver directory: No filename specified");
        return;
    }

    try {
        joynr::serializer::deserializeFromJson(
                _multicastReceiverDirectory,
                joynr::util::loadStringFromFile(_multicastReceiverDirectoryFilename));
    } catch (const std::runtime_error& ex) {
        JOYNR_LOG_ERROR(logger(), ex.what());
        return;
    } catch (const std::invalid_argument& ex) {
        JOYNR_LOG_ERROR(logger(), "Deserialization from JSON failed: {}", ex.what());
        return;
    }

    reestablishMulticastSubscriptions();
}

std::shared_ptr<system::MessageNotificationProvider> CcMessageRouter::
        getMessageNotificationProvider() const
{
    return _messageNotificationProvider;
}

void CcMessageRouter::getGlobalAddress(
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    if (_globalClusterControllerAddress.empty()) {
        onError(joynr::exceptions::ProviderRuntimeException(
                "No cluster-controller global address available."));
    } else {
        onSuccess(_globalClusterControllerAddress);
    }
}

void CcMessageRouter::getReplyToAddress(
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    if (_globalClusterControllerAddress.empty()) {
        onError(joynr::exceptions::ProviderRuntimeException(
                "No cluster-controller reply address available."));
    } else {
        onSuccess(_globalClusterControllerAddress);
    }
}

void CcMessageRouter::reestablishMulticastSubscriptions()
{
    for (const auto& multicastId : _multicastReceiverDirectory.getMulticastIds()) {
        std::string providerParticipantId;

        try {
            providerParticipantId = util::extractParticipantIdFromMulticastId(multicastId);
        } catch (std::invalid_argument& ex) {
            JOYNR_LOG_ERROR(logger(), "Persisted multicast receivers: {}", ex.what());
            continue;
        }

        ReadLocker lock(_routingTableLock);
        const auto routingEntry =
                _routingTable.lookupRoutingEntryByParticipantId(providerParticipantId);
        if (!routingEntry) {
            JOYNR_LOG_WARN(logger(),
                           "Persisted multicast receivers: No provider address found for "
                           "multicast ID {}",
                           multicastId);
            continue;
        }

        const auto providerAddress = routingEntry->address;
        std::shared_ptr<IMessagingMulticastSubscriber> multicastSubscriber =
                _multicastMessagingSkeletonDirectory->getSkeleton(providerAddress);

        if (!multicastSubscriber) {
            JOYNR_LOG_WARN(logger(),
                           "Persisted multicast receivers: No multicast subscriber found for "
                           "multicast ID {}",
                           multicastId);
            continue;
        }

        multicastSubscriber->registerMulticastSubscription(multicastId);
    }
}

void CcMessageRouter::sendQueuedMessages(
        const std::string& destinationPartId,
        std::shared_ptr<const joynr::system::RoutingTypes::Address> address,
        const WriteLocker& messageQueueRetryWriteLock)
{
    assert(messageQueueRetryWriteLock.owns_lock());
    std::ignore = messageQueueRetryWriteLock;
    JOYNR_LOG_TRACE(logger(),
                    "sendMessages: sending messages for destinationPartId {} and {}",
                    destinationPartId,
                    address->toString());
    while (true) {
        std::shared_ptr<ImmutableMessage> item(_messageQueue->getNextMessageFor(destinationPartId));
        if (!item) {
            break;
        }
        doAccessControlCheckOrScheduleMessage(item, address);
    }
}

void CcMessageRouter::doAccessControlCheckOrScheduleMessage(
        std::shared_ptr<ImmutableMessage> message,
        std::shared_ptr<const joynr::system::RoutingTypes::Address> destAddress,
        std::uint32_t tryCount)
{
    if (auto gotAccessController = _accessController.lock()) {
        // Access control checks are asynchronous, callback will send message
        // if access is granted
        auto callback = std::make_shared<ConsumerPermissionCallback>(
                std::dynamic_pointer_cast<CcMessageRouter>(shared_from_this()),
                message,
                destAddress,
                _clusterControllerSettings.aclAudit(),
                tryCount);
        gotAccessController->hasConsumerPermission(message, callback);
    } else {
        // If this point is reached, the message can be sent without delay
        scheduleMessage(message, destAddress, tryCount);
    }
}

/**
 * Q (RDZ): What happens if the message cannot be forwarded? Exception? Log file entry?
 * Q (RDZ): When are messagingstubs removed? They are stored indefinitely in the factory
 */
void CcMessageRouter::routeInternal(std::shared_ptr<ImmutableMessage> message,
                                    std::uint32_t tryCount)
{
    assert(message);
    // Validate the message if possible
    if (_securityManager != nullptr && !_securityManager->validate(*message)) {
        std::string errorMessage("messageId " + message->getId() + " failed validation");
        JOYNR_LOG_ERROR(logger(), errorMessage);
        throw exceptions::JoynrMessageNotSentException(errorMessage);
    }

    JOYNR_LOG_TRACE(logger(), "Route message with Id {}", message->getId());
    AbstractMessageRouter::AddressUnorderedSet destAddresses;
    {
        ReadLocker lock(_messageQueueRetryLock);
        // search for the destination addresses
        destAddresses = getDestinationAddresses(*message, lock);
        // if destination address is not known
        if (destAddresses.empty()) {
            if (message->getType() == Message::VALUE_MESSAGE_TYPE_MULTICAST()) {
                // Do not queue multicast messages for future multicast receivers.
                return;
            }

            if (_messagingSettings.getDiscardUnroutableRepliesAndPublications() &&
                ((message->getType() == Message::VALUE_MESSAGE_TYPE_REPLY()) ||
                 (message->getType() == Message::VALUE_MESSAGE_TYPE_SUBSCRIPTION_REPLY()) ||
                 (message->getType() == Message::VALUE_MESSAGE_TYPE_PUBLICATION()))) {
                // Do not queue reply & publication messages if the proxy is not known.
                // Prequisite is that for every proxy the associated routing entry has
                // been added before any request is made.
                JOYNR_LOG_WARN(
                        logger(),
                        "No routing information found for proxy destination participant ID \"{}\" "
                        "of reply or publication message. "
                        "Discarding message (ID : {})",
                        message->getRecipient(),
                        message->getId());
                return;
            }

            // save the message for later delivery
            JOYNR_LOG_WARN(logger(),
                           "No routing information found for destination participant ID \"{}\" "
                           "so far. Waiting for participant registration. "
                           "Queueing message (ID : {})",
                           message->getRecipient(),
                           message->getId());
            queueMessage(std::move(message), lock);
            return;
        }
    }

    for (std::shared_ptr<const joynr::system::RoutingTypes::Address> destAddress : destAddresses) {
        doAccessControlCheckOrScheduleMessage(message, destAddress, tryCount);
    }
}

bool CcMessageRouter::publishToGlobal(const ImmutableMessage& message)
{
    // Caution: Do not lock routingTableLock here, it must have been called from outside
    // method gets called from AbstractMessageRouter
    const std::string& participantId = message.getSender();
    const auto routingEntry = _routingTable.lookupRoutingEntryByParticipantId(participantId);
    if (routingEntry && routingEntry->isGloballyVisible) {
        return true;
    }
    return false;
}

bool CcMessageRouter::isValidForRoutingTable(
        std::shared_ptr<const joynr::system::RoutingTypes::Address> address)
{
    if (dynamic_cast<const system::RoutingTypes::WebSocketAddress*>(address.get()) != nullptr) {
        JOYNR_LOG_ERROR(logger(),
                        "WebSocketAddress will not be used for CC Routing Table: {}",
                        address->toString());
        return false;
    }
    if (dynamic_cast<const system::RoutingTypes::MqttAddress*>(address.get()) != nullptr &&
        typeid(_ownGlobalAddress) == typeid(system::RoutingTypes::MqttAddress)) {
        const auto mqttAddress =
                std::dynamic_pointer_cast<const joynr::system::RoutingTypes::MqttAddress>(address);
        const joynr::system::RoutingTypes::MqttAddress& ownMqttAddress =
                static_cast<const joynr::system::RoutingTypes::MqttAddress&>(_ownGlobalAddress);
        if (mqttAddress->getTopic() == ownMqttAddress.getTopic()) {
            // an address with a topic starting with the own topic is theoretically also possible
            // but very unlikely
            JOYNR_LOG_TRACE(
                    logger(),
                    "MqttAddress will not be used for Routing Table since it refers to ourselves");
            return false;
        }
    }
    // add check for ChannelAddress here if required
    return true;
}

bool CcMessageRouter::allowRoutingEntryUpdate(const routingtable::RoutingEntry& oldEntry,
                                              const system::RoutingTypes::Address& newAddress)
{
    // precedence: InProcessAddress > WebSocketClientAddress > MqttAddress/ChannelAddress >
    // WebSocketAddress
    if (typeid(newAddress) == typeid(InProcessMessagingAddress)) {
        return true;
    }
    if (dynamic_cast<const InProcessMessagingAddress*>(oldEntry.address.get()) == nullptr) {
        if (typeid(newAddress) == typeid(system::RoutingTypes::WebSocketClientAddress)) {
            return true;
        } else if (dynamic_cast<const system::RoutingTypes::WebSocketClientAddress*>(
                           oldEntry.address.get()) == nullptr) {
            // old address is MqttAddress/ChannelAddress or WebSocketAddress
            if (typeid(newAddress) == typeid(system::RoutingTypes::MqttAddress) ||
                typeid(newAddress) == typeid(system::RoutingTypes::ChannelAddress)) {
                return true;
            } else if (dynamic_cast<const system::RoutingTypes::WebSocketAddress*>(
                               oldEntry.address.get()) != nullptr) {
                // old address is WebSocketAddress
                if (typeid(newAddress) == typeid(system::RoutingTypes::WebSocketAddress)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// inherited from joynr::IMessageRouter and joynr::system::RoutingProvider
void CcMessageRouter::removeNextHop(
        const std::string& participantId,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    std::ignore = onError;

    {
        WriteLocker lock(_routingTableLock);
        _routingTable.remove(participantId);
    }

    saveRoutingTable();

    if (onSuccess) {
        onSuccess();
    }
}

void CcMessageRouter::addNextHop(
        const std::string& participantId,
        const std::shared_ptr<const joynr::system::RoutingTypes::Address>& address,
        bool isGloballyVisible,
        const std::int64_t expiryDateMs,
        const bool isSticky,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    assert(address);
    WriteLocker lock(_messageQueueRetryLock);
    bool addToRoutingTableSuccessful =
            addToRoutingTable(participantId, isGloballyVisible, address, expiryDateMs, isSticky);

    if (addToRoutingTableSuccessful) {
        sendQueuedMessages(participantId, address, lock);

        lock.unlock();

        if (onSuccess) {
            onSuccess();
        }
    } else {
        lock.unlock();

        if (onError) {
            onError(exceptions::ProviderRuntimeException(
                    "unable to addNextHop, as addToRoutingTable failed"));
        }
    }
}

// inherited from joynr::system::RoutingProvider
void CcMessageRouter::addNextHop(
        const std::string& participantId,
        const system::RoutingTypes::ChannelAddress& channelAddress,
        const bool& isGloballyVisible,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    std::ignore = onError;
    auto address =
            std::make_shared<const joynr::system::RoutingTypes::ChannelAddress>(channelAddress);
    constexpr std::int64_t expiryDateMs = std::numeric_limits<std::int64_t>::max();
    const bool isSticky = false;
    addNextHop(participantId,
               std::move(address),
               isGloballyVisible,
               expiryDateMs,
               isSticky,
               std::move(onSuccess));
}

// inherited from joynr::system::RoutingProvider
void CcMessageRouter::addNextHop(
        const std::string& participantId,
        const system::RoutingTypes::MqttAddress& mqttAddress,
        const bool& isGloballyVisible,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    std::ignore = onError;
    constexpr std::int64_t expiryDateMs = std::numeric_limits<std::int64_t>::max();
    const bool isSticky = false;
    auto address = std::make_shared<const joynr::system::RoutingTypes::MqttAddress>(mqttAddress);
    addNextHop(participantId,
               std::move(address),
               isGloballyVisible,
               expiryDateMs,
               isSticky,
               std::move(onSuccess));
}

// inherited from joynr::system::RoutingProvider
void CcMessageRouter::addNextHop(
        const std::string& participantId,
        const system::RoutingTypes::BrowserAddress& browserAddress,
        const bool& isGloballyVisible,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    std::ignore = onError;
    constexpr std::int64_t expiryDateMs = std::numeric_limits<std::int64_t>::max();
    const bool isSticky = false;
    auto address =
            std::make_shared<const joynr::system::RoutingTypes::BrowserAddress>(browserAddress);
    addNextHop(participantId,
               std::move(address),
               isGloballyVisible,
               expiryDateMs,
               isSticky,
               std::move(onSuccess));
}

// inherited from joynr::system::RoutingProvider
void CcMessageRouter::addNextHop(
        const std::string& participantId,
        const system::RoutingTypes::WebSocketAddress& webSocketAddress,
        const bool& isGloballyVisible,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    std::ignore = onError;
    constexpr std::int64_t expiryDateMs = std::numeric_limits<std::int64_t>::max();
    const bool isSticky = false;
    auto address =
            std::make_shared<const joynr::system::RoutingTypes::WebSocketAddress>(webSocketAddress);
    addNextHop(participantId,
               std::move(address),
               isGloballyVisible,
               expiryDateMs,
               isSticky,
               std::move(onSuccess));
}

// inherited from joynr::system::RoutingProvider
void CcMessageRouter::addNextHop(
        const std::string& participantId,
        const system::RoutingTypes::WebSocketClientAddress& webSocketClientAddress,
        const bool& isGloballyVisible,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    std::ignore = onError;
    constexpr std::int64_t expiryDateMs = std::numeric_limits<std::int64_t>::max();
    const bool isSticky = false;
    auto address = std::make_shared<const joynr::system::RoutingTypes::WebSocketClientAddress>(
            webSocketClientAddress);
    addNextHop(participantId,
               std::move(address),
               isGloballyVisible,
               expiryDateMs,
               isSticky,
               std::move(onSuccess));
}

// inherited from joynr::system::RoutingProvider
void CcMessageRouter::addNextHop(
        const std::string& participantId,
        const system::RoutingTypes::BinderAddress& binderAddress,
        const bool& isGloballyVisible,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    std::ignore = onError;
    constexpr std::int64_t expiryDateMs = std::numeric_limits<std::int64_t>::max();
    const bool isSticky = false;
    auto address =
            std::make_shared<const joynr::system::RoutingTypes::BinderAddress>(binderAddress);
    addNextHop(participantId,
               std::move(address),
               isGloballyVisible,
               expiryDateMs,
               isSticky,
               std::move(onSuccess));
}

void CcMessageRouter::resolveNextHop(
        const std::string& participantId,
        std::function<void(const bool& resolved)> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    std::ignore = onError;

    bool resolved;
    {
        ReadLocker lock(_routingTableLock);
        resolved = _routingTable.containsParticipantId(participantId);
    }
    onSuccess(resolved);
}

void CcMessageRouter::registerMulticastReceiver(
        const std::string& multicastId,
        const std::string& subscriberParticipantId,
        const std::string& providerParticipantId,
        std::shared_ptr<const joynr::system::RoutingTypes::Address> providerAddress,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::JoynrRuntimeException&)> onError)
{
    std::ignore = subscriberParticipantId;
    std::ignore = providerParticipantId;

    std::shared_ptr<IMessagingMulticastSubscriber> skeleton =
            _multicastMessagingSkeletonDirectory->getSkeleton(providerAddress);
    if (skeleton) {
        try {
            skeleton->registerMulticastSubscription(multicastId);
            onSuccess();
        } catch (const exceptions::JoynrRuntimeException& error) {
            onError(error);
        }
    } else {
        JOYNR_LOG_TRACE(logger(),
                        "No messaging skeleton found for multicast "
                        "provider (address=" +
                                providerAddress->toString() + ").");
        onSuccess();
    }
}

void CcMessageRouter::addMulticastReceiver(
        const std::string& multicastId,
        const std::string& subscriberParticipantId,
        const std::string& providerParticipantId,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    boost::optional<routingtable::RoutingEntry> routingEntry;
    std::shared_ptr<const joynr::system::RoutingTypes::Address> providerAddress;
    {
        ReadLocker lock(_routingTableLock);
        routingEntry = _routingTable.lookupRoutingEntryByParticipantId(providerParticipantId);
    }

    std::function<void()> onSuccessWrapper = [
        thisWeakPtr = joynr::util::as_weak_ptr(
                std::dynamic_pointer_cast<CcMessageRouter>(shared_from_this())),
        multicastId,
        subscriberParticipantId,
        onSuccess = std::move(onSuccess)
    ]()
    {
        if (auto thisSharedPtr = thisWeakPtr.lock()) {
            thisSharedPtr->_multicastReceiverDirectory.registerMulticastReceiver(
                    multicastId, subscriberParticipantId);
            JOYNR_LOG_TRACE(logger(),
                            "added multicast receiver={} for multicastId={}",
                            subscriberParticipantId,
                            multicastId);
            thisSharedPtr->saveMulticastReceiverDirectory();
            if (onSuccess) {
                onSuccess();
            }
        } else {
            JOYNR_LOG_ERROR(logger(),
                            "error adding multicast receiver={} for multicastId={} because "
                            "CcMessageRouter is no longer available",
                            subscriberParticipantId,
                            multicastId);
        }
    };
    std::function<void(const exceptions::JoynrRuntimeException&)> onErrorWrapper =
            [ onError = std::move(onError), subscriberParticipantId, multicastId ](
                    const exceptions::JoynrRuntimeException& error)
    {
        JOYNR_LOG_ERROR(logger(),
                        "error adding multicast receiver={} for multicastId={}, error: {}",
                        subscriberParticipantId,
                        multicastId,
                        error.getMessage());
        if (onError) {
            onError(joynr::exceptions::ProviderRuntimeException(error.getMessage()));
        }
    };

    if (!routingEntry) {
        exceptions::ProviderRuntimeException exception(
                "No routing entry for multicast provider (providerParticipantId=" +
                providerParticipantId + ") found.");
        onErrorWrapper(exception);
        return;
    }
    providerAddress = routingEntry->address;

    if (dynamic_cast<const system::RoutingTypes::MqttAddress*>(providerAddress.get()) != nullptr) {
        registerMulticastReceiver(multicastId,
                                  subscriberParticipantId,
                                  providerParticipantId,
                                  std::move(providerAddress),
                                  std::move(onSuccessWrapper),
                                  std::move(onErrorWrapper));
    } else {
        onSuccessWrapper();
    }
}

void CcMessageRouter::stopSubscription(std::shared_ptr<ImmutableMessage> message)
{
    // since there currently is no subscriptionId field available via SMRF header
    // it is only possible to extract the subscriptionId out of an unencrypted
    // message payload containing a publication
    if (message->isEncrypted()) {
        JOYNR_LOG_TRACE(logger(),
                        "stopSubscription: cannot get subscriptionId since message is encrypted.");
        return;
    }

    SubscriptionPublication publication;
    try {
        joynr::serializer::deserializeFromJson(publication, message->getUnencryptedBody());
    } catch (const std::invalid_argument& e) {
        JOYNR_LOG_ERROR(
                logger(),
                "Unable to deserialize subscription publication object from: {} - error: {}",
                message->toLogMessage(),
                e.what());
        return;
    }

    const std::string& subscriptionId = publication.getSubscriptionId();

    if (auto messageSenderSharedPtr = _messageSender.lock()) {
        SubscriptionStop subscriptionStop;
        subscriptionStop.setSubscriptionId(subscriptionId);
        joynr::MessagingQos qos;
        qos.setTtl(120000);
        const std::string& recipient = message->getRecipient();
        const std::string& sender = message->getSender();
        JOYNR_LOG_TRACE(logger(),
                        "stopSubscription: trying to send SubscriptionStop proxy {}, provider {}, "
                        "subscriptionId {}",
                        recipient,
                        sender,
                        subscriptionId);
        messageSenderSharedPtr->sendSubscriptionStop(recipient, sender, qos, subscriptionStop);
    } else {
        JOYNR_LOG_TRACE(logger(), "stopSubscription: messageSender not available");
    }
}

void CcMessageRouter::removeMulticastReceiver(
        const std::string& multicastId,
        std::shared_ptr<const joynr::system::RoutingTypes::Address> destAddress,
        const std::string& providerParticipantId)
{
    JOYNR_LOG_INFO(
            logger(),
            "removeMulticastReceiver: multicastId {}, destAddress {}, providerParticipantId {}",
            multicastId,
            destAddress->toString(),
            providerParticipantId);

    // we need the list of all participantIds that match this destAddress
    std::unordered_set<std::string> multicastReceivers =
            _multicastReceiverDirectory.getReceivers(multicastId);
    for (const auto& participantId : multicastReceivers) {
        const auto routingEntry = _routingTable.lookupRoutingEntryByParticipantId(participantId);
        if (routingEntry && destAddress == routingEntry->address) {
            // for the time being, just do it async
            JOYNR_LOG_INFO(logger(),
                           "removeMulticastReceiver: calling removeMulticastReceiver "
                           "multicastId {}, participantId {}, providerParticipantId {}",
                           multicastId,
                           participantId,
                           providerParticipantId);
            removeMulticastReceiver(
                    multicastId, participantId, providerParticipantId, nullptr, nullptr);
        }
    }
}

void CcMessageRouter::removeMulticastReceiver(
        const std::string& multicastId,
        const std::string& subscriberParticipantId,
        const std::string& providerParticipantId,
        std::function<void()> onSuccess,
        std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError)
{
    _multicastReceiverDirectory.unregisterMulticastReceiver(multicastId, subscriberParticipantId);

    saveMulticastReceiverDirectory();

    boost::optional<routingtable::RoutingEntry> routingEntry;
    {
        ReadLocker lock(_routingTableLock);
        routingEntry = _routingTable.lookupRoutingEntryByParticipantId(providerParticipantId);
    }

    if (!routingEntry) {
        exceptions::ProviderRuntimeException exception(
                "No routing entry for multicast provider (providerParticipantId=" +
                providerParticipantId + ") found.");
        JOYNR_LOG_ERROR(logger(), exception.getMessage());
        if (onError) {
            onError(exception);
        }
        return;
    }
    const auto providerAddress = routingEntry->address;
    if (dynamic_cast<const system::RoutingTypes::MqttAddress*>(providerAddress.get()) != nullptr) {
        std::shared_ptr<IMessagingMulticastSubscriber> skeleton =
                _multicastMessagingSkeletonDirectory->getSkeleton(providerAddress);
        if (skeleton) {
            skeleton->unregisterMulticastSubscription(multicastId);
        } else {
            JOYNR_LOG_TRACE(logger(),
                            "No messaging skeleton found for multicast "
                            "provider (address=" +
                                    providerAddress->toString() + ").");
        }
    }
    if (onSuccess) {
        onSuccess();
    }
}

void CcMessageRouter::queueMessage(std::shared_ptr<ImmutableMessage> message,
                                   const ReadLocker& messageQueueRetryReadLock)
{
    assert(messageQueueRetryReadLock.owns_lock());
    std::ignore = messageQueueRetryReadLock;
    JOYNR_LOG_TRACE(logger(), "message queued: {}", message->toLogMessage());
    // do not fire a broadcast for an undeliverable message sent by
    // messageNotificationProvider (e.g. messageQueueForDelivery publication)
    // since it may cause an endless loop
    if (message->getSender() != _messageNotificationProviderParticipantId) {
        _messageNotificationProvider->fireMessageQueuedForDelivery(
                message->getRecipient(), message->getType());
    }
    std::string recipient = message->getRecipient();
    _messageQueue->queueMessage(std::move(recipient), std::move(message));
}

bool CcMessageRouter::canMessageBeTransmitted(std::shared_ptr<ImmutableMessage> message) const
{
    if (auto gotAccessController = _accessController.lock()) {
        return message->isAccessControlChecked();
    }
    return true;
}

/**
 * IMPLEMENTATION of ConsumerPermissionCallback class
 */

ConsumerPermissionCallback::ConsumerPermissionCallback(
        std::weak_ptr<CcMessageRouter> owningMessageRouter,
        std::shared_ptr<ImmutableMessage> message,
        std::shared_ptr<const joynr::system::RoutingTypes::Address> destination,
        bool aclAudit,
        std::uint32_t tryCount)
        : _owningMessageRouter(owningMessageRouter),
          _message(message),
          _destination(destination),
          _aclAudit(aclAudit),
          _tryCount(tryCount)
{
}

void ConsumerPermissionCallback::hasConsumerPermission(IAccessController::Enum hasPermission)
{
    if (_aclAudit) {
        if (hasPermission == IAccessController::Enum::NO) {
            JOYNR_LOG_ERROR(logger(),
                            "ACL AUDIT: message with id '{}' is not allowed by ACL",
                            _message->getId());
            hasPermission = IAccessController::Enum::YES;
        } else if (hasPermission == IAccessController::Enum::YES) {
            JOYNR_LOG_TRACE(logger(),
                            "ACL AUDIT: message with id '{}' is allowed by ACL, ",
                            _message->getId());
        }
    }
    if (hasPermission == IAccessController::Enum::YES) {
        _message->setAccessControlChecked();
        if (auto owningMessageRouterSharedPtr = _owningMessageRouter.lock()) {
            owningMessageRouterSharedPtr->scheduleMessage(_message, _destination);
        } else {
            JOYNR_LOG_ERROR(logger(),
                            "Message with Id {} could not be sent because messageRouter is not "
                            "available",
                            _message->getId());
        }
    } else if (hasPermission == IAccessController::Enum::RETRY) {
        if (auto owningMessageRouterSharedPtr = _owningMessageRouter.lock()) {
            JOYNR_LOG_TRACE(logger(),
                            "ACL RETRY: check cannot be performed, message with id '{}' will be "
                            "rescheduled.",
                            _message->getId());
            owningMessageRouterSharedPtr->scheduleMessage(
                    _message,
                    _destination,
                    _tryCount + 1,
                    owningMessageRouterSharedPtr->createDelayWithExponentialBackoff(
                            owningMessageRouterSharedPtr->_messagingSettings
                                    .getSendMsgRetryInterval(),
                            _tryCount));
        } else {
            JOYNR_LOG_ERROR(logger(),
                            "Message with Id {} could not be sent because messageRouter is not "
                            "available",
                            _message->getId());
        }
    } else {
        JOYNR_LOG_TRACE(logger(),
                        "ACL NO Permission: message with id '{}' will be dropped.",
                        _message->getId());
    }
}

} // namespace joynr
