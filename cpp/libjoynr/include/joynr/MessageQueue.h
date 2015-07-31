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
#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include "joynr/PrivateCopyAssign.h"
#include "joynr/JoynrExport.h"

#include "joynr/JoynrMessage.h"
#include "joynr/ContentWithDecayTime.h"

#include <QMutex>
#include <QRunnable>
#include <string>

namespace joynr
{

typedef ContentWithDecayTime<JoynrMessage> MessageQueueItem;

class JOYNR_EXPORT MessageQueue
{
public:
    MessageQueue();

    ~MessageQueue();

    qint64 getQueueLength();

    qint64 queueMessage(const JoynrMessage& message);

    MessageQueueItem* getNextMessageForParticipant(const std::string destinationPartId);

    qint64 removeOutdatedMessages();

private:
    DISALLOW_COPY_AND_ASSIGN(MessageQueue);

    QMap<std::string, MessageQueueItem*>* queue;
    mutable QMutex queueMutex;
};

/**
 * Runnable to remove outdated message from message queue
 */
class JOYNR_EXPORT MessageQueueCleanerRunnable : public QRunnable
{
public:
    MessageQueueCleanerRunnable(MessageQueue& messageQueue, qint64 sleepInterval = 1000);
    void run();
    void stop();

private:
    MessageQueue& messageQueue;
    bool stopped;
    qint64 sleepInterval;
};
}

#endif // MESSAGEQUEUE_H
