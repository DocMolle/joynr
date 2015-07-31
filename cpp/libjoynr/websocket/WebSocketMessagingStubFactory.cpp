/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2014 BMW Car IT GmbH
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
#include "WebSocketMessagingStubFactory.h"

#include <QtCore/QMutexLocker>
#include <QtCore/QDebug>
#include <QtCore/QEventLoop>
#include <QtWebSockets/QWebSocket>
#include <assert.h>

#include "websocket/WebSocketMessagingStub.h"
#include "joynr/system/QtAddress.h"
#include "joynr/system/QtWebSocketAddress.h"
#include "joynr/system/QtWebSocketClientAddress.h"

namespace joynr
{

joynr_logging::Logger* WebSocketMessagingStubFactory::logger =
        joynr_logging::Logging::getInstance()->getLogger("MSG", "WebSocketMessagingStubFactory");

WebSocketMessagingStubFactory::WebSocketMessagingStubFactory(QObject* parent)
        : QObject(parent), serverStubMap(), clientStubMap(), mutex()
{
}

bool WebSocketMessagingStubFactory::canCreate(const joynr::system::QtAddress& destAddress)
{
    return destAddress.inherits(system::QtWebSocketAddress::staticMetaObject.className()) ||
           destAddress.inherits(system::QtWebSocketClientAddress::staticMetaObject.className());
}

QSharedPointer<IMessaging> WebSocketMessagingStubFactory::create(
        const joynr::system::QtAddress& destAddress)
{
    // if destination is a WS client address
    if (destAddress.inherits(system::QtWebSocketClientAddress::staticMetaObject.className())) {
        const system::QtWebSocketClientAddress* webSocketClientAddress =
                qobject_cast<const system::QtWebSocketClientAddress*>(&destAddress);
        // lookup address
        {
            QMutexLocker locker(&mutex);
            if (!clientStubMap.contains(*webSocketClientAddress)) {
                LOG_ERROR(logger,
                          QString("No websocket found for address %0")
                                  .arg(webSocketClientAddress->toString()));
            }
        }
        return clientStubMap.value(*webSocketClientAddress, QSharedPointer<IMessaging>());
    }
    // if destination is a WS server address
    if (destAddress.inherits(system::QtWebSocketAddress::staticMetaObject.className())) {
        const system::QtWebSocketAddress* webSocketServerAddress =
                qobject_cast<const system::QtWebSocketAddress*>(&destAddress);
        // lookup address
        {
            QMutexLocker locker(&mutex);
            if (!serverStubMap.contains(*webSocketServerAddress)) {
                LOG_ERROR(logger,
                          QString("No websocket found for address %0")
                                  .arg(webSocketServerAddress->toString()));
            }
        }
        return serverStubMap.value(*webSocketServerAddress, QSharedPointer<IMessaging>());
    }

    return QSharedPointer<IMessaging>();
}

void WebSocketMessagingStubFactory::addClient(
        const joynr::system::QtWebSocketClientAddress& clientAddress,
        QWebSocket* webSocket)
{
    WebSocketMessagingStub* wsClientStub = new WebSocketMessagingStub(
            new joynr::system::QtWebSocketClientAddress(clientAddress), webSocket);
    connect(wsClientStub,
            &WebSocketMessagingStub::closed,
            this,
            &WebSocketMessagingStubFactory::onMessagingStubClosed);
    QSharedPointer<IMessaging> clientStub(wsClientStub);
    clientStubMap.insert(clientAddress, clientStub);
}

void WebSocketMessagingStubFactory::removeClient(
        const joynr::system::QtWebSocketClientAddress& clientAddress)
{
    clientStubMap.remove(clientAddress);
}

void WebSocketMessagingStubFactory::addServer(const system::QtWebSocketAddress& serverAddress,
                                              QWebSocket* webSocket)
{
    WebSocketMessagingStub* wsServerStub = new WebSocketMessagingStub(
            new joynr::system::QtWebSocketAddress(serverAddress), webSocket);
    connect(wsServerStub,
            &WebSocketMessagingStub::closed,
            this,
            &WebSocketMessagingStubFactory::onMessagingStubClosed);
    QSharedPointer<IMessaging> serverStub(wsServerStub);
    serverStubMap.insert(serverAddress, serverStub);
}

void WebSocketMessagingStubFactory::onMessagingStubClosed(const system::QtAddress& address)
{
    LOG_DEBUG(logger, QString("removing messaging stub for addres: %0").arg(address.toString()));
    if (address.inherits(system::QtWebSocketClientAddress::staticMetaObject.className())) {
        const system::QtWebSocketClientAddress* wsClientAddress =
                qobject_cast<const system::QtWebSocketClientAddress*>(&address);
        clientStubMap.remove(*wsClientAddress);
    }
    if (address.inherits(system::QtWebSocketAddress::staticMetaObject.className())) {
        const system::QtWebSocketAddress* wsServerAddress =
                qobject_cast<const system::QtWebSocketAddress*>(&address);
        serverStubMap.remove(*wsServerAddress);
    }
}

QUrl WebSocketMessagingStubFactory::convertWebSocketAddressToUrl(
        const system::QtWebSocketAddress& address)
{
    return QUrl(QString("%0://%1:%2%3")
                        .arg(address.getProtocolInternal().toLower())
                        .arg(address.getHost())
                        .arg(address.getPort())
                        .arg(address.getPath()));
}

} // namespace joynr
