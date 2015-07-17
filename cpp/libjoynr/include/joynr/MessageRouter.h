/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
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
#ifndef MESSAGEROUTER_H
#define MESSAGEROUTER_H
#include "joynr/PrivateCopyAssign.h"

#include "joynr/JoynrExport.h"
#include "joynr/ObjectWithDecayTime.h"
#include "joynr/JoynrMessage.h"
#include "joynr/IMessaging.h"
#include "joynr/MessagingSettings.h"
#include "joynr/system/RoutingProxy.h"
#include "joynr/system/RoutingAbstractProvider.h"
#include "joynr/RequestStatus.h"
#include "joynr/Directory.h"
#include "joynr/MessageQueue.h"

#include <QSharedPointer>
#include <QDateTime>
#include <QThreadPool>
#include <QSemaphore>
#include <QPair>
#include <QMap>
#include <QSet>
#include <QMutex>
#include <string>
#include <QReadWriteLock>

namespace joynr
{

class IMessagingStubFactory;
class JoynrMessagingEndpointAddress;
class IAccessController;
class IPlatformSecurityManager;
namespace joynr_logging
{
class Logger;
}
class MessageQueueCleanerRunnable;
namespace system
{
class Address;
}

/**
  * Class MessageRouter receives incoming JoynrMessages on the ClusterController
  * and forwards them either to a remote ClusterController or to a LibJoynr on the machine.
  *
  *  1 extracts the destination participant ID and looks up the EndpointAddress in the
  *MessagingEndpointDirectory
  *  2 creates a <Middleware>MessagingStub by calling MessagingStubFactory.create(EndpointAddress
  *addr)
  *  3 forwards the message using the <Middleware>MessagingStub.send(JoynrMessage msg)
  *
  *  In sending, a ThreadPool of default size 6 is used with a 500ms default retry interval.
  */

class JOYNR_EXPORT MessageRouter : public joynr::system::RoutingAbstractProvider
{
public:
    MessageRouter(IMessagingStubFactory* messagingStubFactory,
                  IPlatformSecurityManager* securityManager,
                  int maxThreads = 6,
                  MessageQueue* messageQueue = new MessageQueue());

    MessageRouter(IMessagingStubFactory* messagingStubFactory,
                  QSharedPointer<joynr::system::Address> incomingAddress,
                  int maxThreads = 6,
                  MessageQueue* messageQueue = new MessageQueue());

    virtual ~MessageRouter();

    /**
     * @brief Forwards the message towards its destination (determined by inspecting the message
     * header). NOTE: the init method must be called before the first message is routed.
     *
     * @param message the message to route.
     * @param qos the QoS used to route the message.
     */
    virtual void route(const JoynrMessage& message);

    virtual void addNextHop(
            const std::string& participantId,
            const joynr::system::ChannelAddress& channelAddress,
            std::function<void(const joynr::RequestStatus& joynrInternalStatus)> callbackFct);
    virtual void addNextHop(
            const std::string& participantId,
            const joynr::system::CommonApiDbusAddress& commonApiDbusAddress,
            std::function<void(const joynr::RequestStatus& joynrInternalStatus)> callbackFct);
    virtual void addNextHop(
            const std::string& participantId,
            const joynr::system::BrowserAddress& browserAddress,
            std::function<void(const joynr::RequestStatus& joynrInternalStatus)> callbackFct);
    virtual void addNextHop(
            const std::string& participantId,
            const joynr::system::WebSocketAddress& webSocketAddress,
            std::function<void(const joynr::RequestStatus& joynrInternalStatus)> callbackFct);
    virtual void addNextHop(
            const std::string& participantId,
            const joynr::system::WebSocketClientAddress& webSocketClientAddress,
            std::function<void(const joynr::RequestStatus& joynrInternalStatus)> callbackFct);
    virtual void removeNextHop(const std::string& participantId,
                               std::function<void(const joynr::RequestStatus& joynrInternalStatus)>
                                       callbackFct = nullptr);
    virtual void resolveNextHop(const std::string& participantId,
                                std::function<void(const joynr::RequestStatus& joynrInternalStatus,
                                                   const bool& resolved)> callbackFct);

    void addProvisionedNextHop(std::string participantId,
                               QSharedPointer<joynr::system::Address> address);

    void setAccessController(QSharedPointer<IAccessController> accessController);

    void setParentRouter(joynr::system::RoutingProxy* parentRouter,
                         QSharedPointer<joynr::system::Address> parentAddress,
                         std::string parentParticipantId);

    virtual void addNextHop(const std::string& participantId,
                            const QSharedPointer<joynr::system::Address>& inprocessAddress,
                            std::function<void(const joynr::RequestStatus& joynrInternalStatus)>
                                    callbackFct = nullptr);

    friend class MessageRunnable;
    friend class ConsumerPermissionCallback;

private:
    DISALLOW_COPY_AND_ASSIGN(MessageRouter);
    IMessagingStubFactory* messagingStubFactory;
    Directory<std::string, joynr::system::Address> routingTable;
    QReadWriteLock routingTableLock;
    QThreadPool threadPool;
    joynr::system::RoutingProxy* parentRouter;
    QSharedPointer<joynr::system::Address> parentAddress;
    QSharedPointer<joynr::system::Address> incomingAddress;
    static joynr_logging::Logger* logger;

    MessageQueue* messageQueue;
    MessageQueueCleanerRunnable* messageQueueCleanerRunnable;
    QSet<QString>* runningParentResolves;
    QSharedPointer<IAccessController> accessController;
    IPlatformSecurityManager* securityManager;
    mutable QMutex parentResolveMutex;

    void init(int maxThreads);
    void addNextHopToParent(
            std::string participantId,
            std::function<void(const joynr::RequestStatus& status)> callbackFct = nullptr);

    void sendMessage(const JoynrMessage& message,
                     QSharedPointer<joynr::system::Address> destAddress);

    void sendMessages(const std::string& destinationPartId,
                      QSharedPointer<joynr::system::Address> address);

    bool isChildMessageRouter();

    void addToRoutingTable(std::string participantId,
                           QSharedPointer<joynr::system::Address> address);

    void removeRunningParentResolvers(const QString& destinationPartId);
};

/**
 * Class to send message
 */
class MessageRunnable : public QRunnable, public ObjectWithDecayTime
{
public:
    MessageRunnable(const JoynrMessage& message, QSharedPointer<IMessaging> messagingStub);
    void run();

private:
    JoynrMessage message;
    QSharedPointer<IMessaging> messagingStub;
    static joynr_logging::Logger* logger;
};

} // namespace joynr
#endif // MESSAGEROUTER_H
