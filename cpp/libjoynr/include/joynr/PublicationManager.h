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
#ifndef PUBLICATIONMANAGER_H
#define PUBLICATIONMANAGER_H
#include "joynr/PrivateCopyAssign.h"

#include "joynr/JoynrExport.h"

#include "joynr/joynrlogging.h"

#include <QMultiMap>
#include <QSharedPointer>
#include <QString>
#include <QVariant>
#include <QMutex>
#include <QReadWriteLock>
#include <QThreadPool>
#include <memory>

namespace joynr
{

class DelayedScheduler;
class SubscriptionRequest;
class BroadcastSubscriptionRequest;
class BroadcastSubscriptionRequestInformation;
class SubscriptionRequestInformation;
class SubscriptionInformation;
class IPublicationSender;
class RequestCaller;
class QtSubscriptionQos;
class IBroadcastFilter;

/**
  * \class PublicationManager
  * \brief Publication manager receives subscription requests and prepares publications,
  * which are send back to the subscription manager.
  * Responsible for deleting SubscriptionRequests and PublicationStates (the runnable notifies the
  * SubscriptionManager when it terminates - this triggeres the delete).
  */
class JOYNR_EXPORT PublicationManager
{
public:
    explicit PublicationManager(int maxThreads = 2);
    explicit PublicationManager(DelayedScheduler* scheduler, int maxThreads = 2);
    virtual ~PublicationManager();
    /**
     * @brief Adds the SubscriptionRequest and starts runnable to poll attributes.
     * @param requestCaller
     * @param subscriptionRequest
     * @param publicationSender
     */
    void add(const QString& proxyParticipantId,
             const QString& providerParticipantId,
             QSharedPointer<RequestCaller> requestCaller,
             SubscriptionRequest& subscriptionRequest,
             IPublicationSender* publicationSender);

    /**
     * @brief Adds SubscriptionRequest when the Provider is not yet registered
     *   and there is no RequestCaller as yet.
     *
     * @param subscriptionRequest
     */
    void add(const QString& proxyParticipantId,
             const QString& providerParticipantId,
             SubscriptionRequest& subscriptionRequest);

    /**
     * @brief Adds the BroadcastSubscriptionRequest and starts runnable to poll attributes.
     * @param requestCaller
     * @param subscriptionRequest
     * @param publicationSender
     */
    void add(const QString& proxyParticipantId,
             const QString& providerParticipantId,
             QSharedPointer<RequestCaller> requestCaller,
             BroadcastSubscriptionRequest& subscriptionRequest,
             IPublicationSender* publicationSender);

    /**
     * @brief Adds BroadcastSubscriptionRequest when the Provider is not yet registered
     *   and there is no RequestCaller as yet.
     *
     * @param subscriptionRequest
     */
    void add(const QString& proxyParticipantId,
             const QString& providerParticipantId,
             BroadcastSubscriptionRequest& subscriptionRequest);

    /**
     * @brief Stops the sending of publications
     *
     * @param subscriptionId
     */
    void stopPublication(const QString& subscriptionId);

    /**
     * @brief Stops all publications for a provider
     *
     * @param providerId
     */
    void removeAllSubscriptions(const QString& providerId);

    /**
     * @brief Called by the Dispatcher every time a provider is registered to check whether there
     * are already subscriptionRequests waiting.
     *
     * @param providerId
     * @param requestCaller
     * @param publicationSender
     */
    void restore(const QString& providerId,
                 QSharedPointer<RequestCaller> requestCaller,
                 IPublicationSender* publicationSender);

    /**
      * @brief Publishes an onChange message when an attribute value changes
      *
      * This method is virtual so that it can be overridden by a mock object.
      * @param subscriptionId A subscription that was listening on the attribute
      * @param value The new attribute value
      */
    virtual void attributeValueChanged(const QString& subscriptionId, const QVariant& value);

    /**
      * @brief Publishes an broadcast publication message when a broadcast occurs
      *
      * This method is virtual so that it can be overridden by a mock object.
      * @param subscriptionId A subscription that was listening on the broadcast
      * @param values The new broadcast values
      */
    virtual void broadcastOccurred(const QString& subscriptionId,
                                   const QList<QVariant>& values,
                                   const QList<std::shared_ptr<IBroadcastFilter>>& filters);

private:
    DISALLOW_COPY_AND_ASSIGN(PublicationManager);

    // A class that groups together the information needed for a publication
    class Publication;

    // Information for each publication is keyed by subcriptionId
    QMap<QString, QSharedPointer<Publication>> publications;
    QMap<QString, QSharedPointer<SubscriptionRequestInformation>>
            subscriptionId2SubscriptionRequest;
    QMap<QString, QSharedPointer<BroadcastSubscriptionRequestInformation>>
            subscriptionId2BroadcastSubscriptionRequest;

    // .. and protected with a read/write lock
    mutable QReadWriteLock subscriptionLock;

    QMutex fileWriteLock;
    // Publications are scheduled to run on a thread pool
    QThreadPool publishingThreadPool;
    DelayedScheduler* delayedScheduler;

    // Support for clean shutdowns
    QMutex shutDownMutex;
    bool shuttingDown;

    // Subscription persistence
    QString subscriptionRequestStorageFileName;
    QString broadcastSubscriptionRequestStorageFileName;

    // Queues all subscription requests that are either received by the
    // dispatcher or restored from the subscription storage file before
    // the corresponding provider is added
    QMultiMap<QString, QSharedPointer<SubscriptionRequestInformation>> queuedSubscriptionRequests;
    QMutex queuedSubscriptionRequestsMutex;

    // Queues all broadcast subscription requests that are either received by the
    // dispatcher or restored from the subscription storage file before
    // the corresponding provider is added
    QMultiMap<QString, QSharedPointer<BroadcastSubscriptionRequestInformation>>
            queuedBroadcastSubscriptionRequests;
    QMutex queuedBroadcastSubscriptionRequestsMutex;

    // Logging
    static joynr_logging::Logger* logger;

    // List of subscriptionId's of runnables scheduled with delay <= qos.getMinInterval_ms()
    QList<QString> currentScheduledPublications;
    QMutex currentScheduledPublicationsMutex;

    // Filters registered for broadcasts. Keyed by broadcast name.
    QMap<QString, QList<std::shared_ptr<IBroadcastFilter>>> broadcastFilters;

    // Read/write lock for broadcast filters
    mutable QReadWriteLock broadcastFilterLock;

    // PublisherRunnables are used to send publications via a ThreadPool
    class PublisherRunnable;

    // PublicationEndRunnables finish a publication
    class PublicationEndRunnable;

    // Functions called by runnables
    void pollSubscription(const QString& subscriptionId);
    void removePublication(const QString& subscriptionId);
    void removeAttributePublication(const QString& subscriptionId);
    void removeBroadcastPublication(const QString& subscriptionId);

    // Helper functions
    bool publicationExists(const QString& subscriptionId) const;
    void createPublishRunnable(const QString& subscriptionId);
    void saveAttributeSubscriptionRequestsMap(const QList<QVariant>& subscriptionList);
    void loadSavedAttributeSubscriptionRequestsMap();
    void saveBroadcastSubscriptionRequestsMap(const QList<QVariant>& subscriptionList);
    void loadSavedBroadcastSubscriptionRequestsMap();

    void reschedulePublication(const QString& subscriptionId, qint64 nextPublication);

    bool isPublicationAlreadyScheduled(const QString& subscriptionId);

    /**
     * @brief getTimeUntilNextPublication determines the time to wait until the next publication
     * can be sent base on the QOS information.
     * @param subscriptionId
     * @param qos
     * @return  0 if publication can immediately be sent;
     *          amount of ms to wait, if interval was too short;
     *          -1 on error
     */
    qint64 getTimeUntilNextPublication(QSharedPointer<Publication> publication,
                                       QSharedPointer<QtSubscriptionQos> qos);

    void saveSubscriptionRequestsMap(const QList<QVariant>& subscriptionList,
                                     const QString& storageFilename);

    template <class RequestInformationType>
    void loadSavedSubscriptionRequestsMap(
            const QString& storageFilename,
            QMutex& mutex,
            QMultiMap<QString, QSharedPointer<RequestInformationType>>& queuedSubscriptions);

    template <class RequestInformationType>
    QList<QVariant> subscriptionMapToListCopy(
            const QMap<QString, QSharedPointer<RequestInformationType>>& map);

    bool isShuttingDown();
    qint64 getPublicationTtl(QSharedPointer<SubscriptionRequest> subscriptionRequest) const;
    void sendPublication(QSharedPointer<Publication> publication,
                         QSharedPointer<SubscriptionInformation> subscriptionInformation,
                         QSharedPointer<SubscriptionRequest> subscriptionRequest,
                         const QList<QVariant>& value);
    void handleAttributeSubscriptionRequest(
            QSharedPointer<SubscriptionRequestInformation> requestInfo,
            QSharedPointer<RequestCaller> requestCaller,
            IPublicationSender* publicationSender);

    void handleBroadcastSubscriptionRequest(
            QSharedPointer<BroadcastSubscriptionRequestInformation> requestInfo,
            QSharedPointer<RequestCaller> requestCaller,
            IPublicationSender* publicationSender);

    void addOnChangePublication(const QString& subscriptionId,
                                QSharedPointer<SubscriptionRequestInformation> request,
                                QSharedPointer<Publication> publication);
    void addBroadcastPublication(const QString& subscriptionId,
                                 QSharedPointer<BroadcastSubscriptionRequestInformation> request,
                                 QSharedPointer<Publication> publication);
    void removeOnChangePublication(const QString& subscriptionId,
                                   QSharedPointer<SubscriptionRequestInformation> request,
                                   QSharedPointer<Publication> publication);
    void removePublicationEndRunnable(QSharedPointer<Publication> publication);

    bool processFilterChain(const QString& subscriptionId,
                            const QList<QVariant>& broadcastValues,
                            const QList<std::shared_ptr<IBroadcastFilter>>& filters);
};

} // namespace joynr

#endif // PUBLICATIONMANAGER_H
