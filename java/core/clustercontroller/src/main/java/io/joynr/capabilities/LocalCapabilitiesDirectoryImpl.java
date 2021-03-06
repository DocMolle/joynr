/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2017 BMW Car IT GmbH
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
package io.joynr.capabilities;

import static io.joynr.runtime.SystemServicesSettings.PROPERTY_CAPABILITIES_FRESHNESS_UPDATE_INTERVAL_MS;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.inject.Inject;
import com.google.inject.Singleton;
import com.google.inject.name.Named;

import io.joynr.exceptions.DiscoveryException;
import io.joynr.exceptions.JoynrException;
import io.joynr.exceptions.JoynrRuntimeException;
import io.joynr.messaging.MessagingPropertyKeys;
import io.joynr.messaging.routing.MessageRouter;
import io.joynr.messaging.routing.TransportReadyListener;
import io.joynr.provider.DeferredVoid;
import io.joynr.provider.Promise;
import io.joynr.provider.PromiseListener;
import io.joynr.proxy.CallbackWithModeledError;
import io.joynr.runtime.GlobalAddressProvider;
import io.joynr.runtime.ShutdownNotifier;
import joynr.exceptions.ApplicationException;
import joynr.exceptions.ProviderRuntimeException;
import joynr.infrastructure.GlobalCapabilitiesDirectory;
import joynr.system.RoutingTypes.Address;
import joynr.system.RoutingTypes.MqttAddress;
import joynr.types.DiscoveryEntry;
import joynr.types.DiscoveryEntryWithMetaInfo;
import joynr.types.DiscoveryError;
import joynr.types.DiscoveryQos;
import joynr.types.DiscoveryScope;
import joynr.types.GlobalDiscoveryEntry;
import joynr.types.ProviderScope;

@Singleton
public class LocalCapabilitiesDirectoryImpl extends AbstractLocalCapabilitiesDirectory
        implements TransportReadyListener {

    private static final Logger logger = LoggerFactory.getLogger(LocalCapabilitiesDirectoryImpl.class);

    private static final Set<DiscoveryScope> INCLUDE_LOCAL_SCOPES = new HashSet<>();
    static {
        INCLUDE_LOCAL_SCOPES.add(DiscoveryScope.LOCAL_ONLY);
        INCLUDE_LOCAL_SCOPES.add(DiscoveryScope.LOCAL_AND_GLOBAL);
        INCLUDE_LOCAL_SCOPES.add(DiscoveryScope.LOCAL_THEN_GLOBAL);
    }

    private static final Set<DiscoveryScope> INCLUDE_GLOBAL_SCOPES = new HashSet<>();
    static {
        INCLUDE_GLOBAL_SCOPES.add(DiscoveryScope.GLOBAL_ONLY);
        INCLUDE_GLOBAL_SCOPES.add(DiscoveryScope.LOCAL_AND_GLOBAL);
        INCLUDE_GLOBAL_SCOPES.add(DiscoveryScope.LOCAL_THEN_GLOBAL);
    }

    private ScheduledExecutorService freshnessUpdateScheduler;

    private DiscoveryEntryStore<DiscoveryEntry> localDiscoveryEntryStore;
    private GlobalCapabilitiesDirectoryClient globalCapabilitiesDirectoryClient;
    private DiscoveryEntryStore<GlobalDiscoveryEntry> globalDiscoveryEntryCache;
    private final Map<String, List<String>> globalProviderParticipantIdToGbidListMap;

    private MessageRouter messageRouter;

    private GlobalAddressProvider globalAddressProvider;

    private Address globalAddress;
    private Object globalAddressLock = new Object();

    private List<QueuedDiscoveryEntry> queuedDiscoveryEntries = new ArrayList<QueuedDiscoveryEntry>();

    private final String[] knownGbids;

    static class QueuedDiscoveryEntry {
        private DiscoveryEntry discoveryEntry;
        private String[] gbids;
        private Add1Deferred deferred;
        private boolean awaitGlobalRegistration;

        public QueuedDiscoveryEntry(DiscoveryEntry discoveryEntry,
                                    String[] gbids,
                                    Add1Deferred deferred,
                                    boolean awaitGlobalRegistration) {
            this.discoveryEntry = discoveryEntry;
            this.gbids = gbids;
            this.deferred = deferred;
            this.awaitGlobalRegistration = awaitGlobalRegistration;
        }

        public DiscoveryEntry getDiscoveryEntry() {
            return discoveryEntry;
        }

        public String[] getGbids() {
            return gbids;
        }

        public Add1Deferred getDeferred() {
            return deferred;
        }

        public boolean getAwaitGlobalRegistration() {
            return awaitGlobalRegistration;
        }
    }

    @Inject
    // CHECKSTYLE IGNORE ParameterNumber FOR NEXT 1 LINES
    public LocalCapabilitiesDirectoryImpl(CapabilitiesProvisioning capabilitiesProvisioning,
                                          GlobalAddressProvider globalAddressProvider,
                                          DiscoveryEntryStore<DiscoveryEntry> localDiscoveryEntryStore,
                                          DiscoveryEntryStore<GlobalDiscoveryEntry> globalDiscoveryEntryCache,
                                          MessageRouter messageRouter,
                                          GlobalCapabilitiesDirectoryClient globalCapabilitiesDirectoryClient,
                                          ExpiredDiscoveryEntryCacheCleaner expiredDiscoveryEntryCacheCleaner,
                                          @Named(PROPERTY_CAPABILITIES_FRESHNESS_UPDATE_INTERVAL_MS) long freshnessUpdateIntervalMs,
                                          @Named(JOYNR_SCHEDULER_CAPABILITIES_FRESHNESS) ScheduledExecutorService freshnessUpdateScheduler,
                                          ShutdownNotifier shutdownNotifier,
                                          @Named(MessagingPropertyKeys.GBID_ARRAY) String[] knownGbids) {
        globalProviderParticipantIdToGbidListMap = new HashMap<>();
        this.globalAddressProvider = globalAddressProvider;
        // CHECKSTYLE:ON
        this.messageRouter = messageRouter;
        this.localDiscoveryEntryStore = localDiscoveryEntryStore;
        this.globalDiscoveryEntryCache = globalDiscoveryEntryCache;
        this.globalCapabilitiesDirectoryClient = globalCapabilitiesDirectoryClient;
        this.knownGbids = knownGbids.clone();
        Collection<GlobalDiscoveryEntry> provisionedDiscoveryEntries = capabilitiesProvisioning.getDiscoveryEntries();
        this.globalDiscoveryEntryCache.add(provisionedDiscoveryEntries);
        for (GlobalDiscoveryEntry provisionedEntry : provisionedDiscoveryEntries) {
            String participantId = provisionedEntry.getParticipantId();
            if (GlobalCapabilitiesDirectory.INTERFACE_NAME.equals(provisionedEntry.getInterfaceName())) {
                mapGbidsToGlobalProviderParticipantId(participantId, knownGbids);
            } else {
                Address address = CapabilityUtils.getAddressFromGlobalDiscoveryEntry(provisionedEntry);
                mapGbidsToGlobalProviderParticipantId(participantId, getGbids(address));
            }
        }
        expiredDiscoveryEntryCacheCleaner.scheduleCleanUpForCaches(new ExpiredDiscoveryEntryCacheCleaner.CleanupAction() {
            @Override
            public void cleanup(Set<DiscoveryEntry> expiredDiscoveryEntries) {
                for (DiscoveryEntry discoveryEntry : expiredDiscoveryEntries) {
                    remove(discoveryEntry);
                }
            }
        }, globalDiscoveryEntryCache, localDiscoveryEntryStore);
        this.freshnessUpdateScheduler = freshnessUpdateScheduler;
        setUpPeriodicFreshnessUpdate(freshnessUpdateIntervalMs);
        shutdownNotifier.registerForShutdown(this);
    }

    private String[] getGbids(Address address) {
        String[] gbids;
        if (address instanceof MqttAddress) {
            // For backwards compatibility, the GBID is stored in the brokerUri field of MqttAddress which was not evaluated in older joynr versions
            gbids = new String[]{ ((MqttAddress) address).getBrokerUri() };
        } else {
            // use default GBID for all other address types
            gbids = new String[]{ knownGbids[0] };
        }
        return gbids;
    }

    private void mapGbidsToGlobalProviderParticipantId(String participantId, String[] gbids) {
        List<String> newGbidsList = new ArrayList<String>(Arrays.asList(gbids));
        if (globalProviderParticipantIdToGbidListMap.containsKey(participantId)) {
            List<String> nonDuplicateOldGbids = globalProviderParticipantIdToGbidListMap.get(participantId)
                                                                                        .stream()
                                                                                        .filter(gbid -> !newGbidsList.contains(gbid))
                                                                                        .collect(Collectors.toList());
            newGbidsList.addAll(nonDuplicateOldGbids);
        }
        globalProviderParticipantIdToGbidListMap.put(participantId, newGbidsList);

    }

    private void setUpPeriodicFreshnessUpdate(final long freshnessUpdateIntervalMs) {
        logger.trace("Setting up periodic freshness update with interval {}", freshnessUpdateIntervalMs);
        Runnable command = new Runnable() {
            @Override
            public void run() {
                try {
                    logger.debug("Updating last seen date ms.");
                    globalCapabilitiesDirectoryClient.touch();
                } catch (JoynrRuntimeException e) {
                    logger.error("error sending freshness update", e);
                }
            }
        };
        freshnessUpdateScheduler.scheduleAtFixedRate(command,
                                                     freshnessUpdateIntervalMs,
                                                     freshnessUpdateIntervalMs,
                                                     TimeUnit.MILLISECONDS);
    }

    /**
     * Adds local capability to local and (depending on SCOPE) the global directory
     */
    @Override
    public Promise<DeferredVoid> add(final DiscoveryEntry discoveryEntry) {
        boolean awaitGlobalRegistration = false;
        return add(discoveryEntry, awaitGlobalRegistration);
    }

    @Override
    public Promise<DeferredVoid> add(final DiscoveryEntry discoveryEntry, final Boolean awaitGlobalRegistration) {
        Promise<Add1Deferred> addPromise = add(discoveryEntry, awaitGlobalRegistration, new String[]{});
        DeferredVoid deferredVoid = new DeferredVoid();
        addPromise.then(new PromiseListener() {
            @Override
            public void onRejection(JoynrException exception) {
                if (exception instanceof ApplicationException) {
                    DiscoveryError error = ((ApplicationException) exception).getError();
                    deferredVoid.reject(new ProviderRuntimeException("Error registering provider "
                            + discoveryEntry.getParticipantId() + " in default backend: " + error));
                } else if (exception instanceof ProviderRuntimeException) {
                    deferredVoid.reject((ProviderRuntimeException) exception);
                } else {
                    deferredVoid.reject(new ProviderRuntimeException("Unknown error registering provider "
                            + discoveryEntry.getParticipantId() + " in default backend: " + exception));
                }
            }

            @Override
            public void onFulfillment(Object... values) {
                deferredVoid.resolve();
            }
        });
        return new Promise<>(deferredVoid);
    }

    private DiscoveryError validateGbids(final String[] gbids) {
        if (gbids == null) {
            return DiscoveryError.INVALID_GBID;
        }

        HashSet<String> gbidSet = new HashSet<String>();
        for (String gbid : gbids) {
            if (gbid == null || gbid.isEmpty() || gbidSet.contains(gbid)) {
                return DiscoveryError.INVALID_GBID;
            }
            gbidSet.add(gbid);

            boolean found = false;
            for (String validGbid : knownGbids) {
                if (gbid.equals(validGbid)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return DiscoveryError.UNKNOWN_GBID;
            }
        }
        return null;
    }

    @Override
    public Promise<Add1Deferred> add(DiscoveryEntry discoveryEntry, Boolean awaitGlobalRegistration, String[] gbids) {
        final Add1Deferred deferred = new Add1Deferred();

        DiscoveryError validationResult = validateGbids(gbids);
        if (validationResult != null) {
            deferred.reject(validationResult);
            return new Promise<>(deferred);
        }
        if (gbids.length == 0) {
            // register provider in default backend
            gbids = new String[]{ knownGbids[0] };
        }

        if (localDiscoveryEntryStore.hasDiscoveryEntry(discoveryEntry)) {
            Optional<DiscoveryEntry> optionalDiscoveryEntry = localDiscoveryEntryStore.lookup(discoveryEntry.getParticipantId(),
                                                                                              Long.MAX_VALUE);
            DiscoveryEntry localEntry = optionalDiscoveryEntry.isPresent() ? optionalDiscoveryEntry.get() : null;
            if (discoveryEntry.getQos().getScope().equals(ProviderScope.LOCAL) && localEntry.equals(discoveryEntry)) {
                // in this case, no further need for global registration is required. Registration completed.
                deferred.resolve();
                return new Promise<>(deferred);
            }
        }
        localDiscoveryEntryStore.add(discoveryEntry);
        notifyCapabilityAdded(discoveryEntry);

        /*
         * In case awaitGlobalRegistration is true, a result for this 'add' call will not be returned before the call to
         * the globalDiscovery has either succeeded, failed or timed out. In case of failure or timeout the already
         * created discoveryEntry will also be removed again from localDiscoveryStore.
         *
         * If awaitGlobalRegistration is false, the call to the globalDiscovery will just be triggered, but it is not
         * being waited for results or timeout. Also, in case it does not succeed, the entry remains in
         * localDiscoveryStore.
         */
        if (discoveryEntry.getQos().getScope().equals(ProviderScope.GLOBAL)) {
            Add1Deferred deferredForRegisterGlobal;
            if (awaitGlobalRegistration == true) {
                deferredForRegisterGlobal = deferred;
            } else {
                // use an independent DeferredVoid not used for waiting
                deferredForRegisterGlobal = new Add1Deferred();
                deferred.resolve();
            }
            registerGlobal(discoveryEntry, gbids, deferredForRegisterGlobal, awaitGlobalRegistration);
        } else {
            deferred.resolve();
        }
        return new Promise<>(deferred);
    }

    @Override
    public Promise<AddToAllDeferred> addToAll(DiscoveryEntry discoveryEntry, Boolean awaitGlobalRegistration) {
        Promise<Add1Deferred> addPromise = add(discoveryEntry, awaitGlobalRegistration, knownGbids);
        AddToAllDeferred addToAllDeferred = new AddToAllDeferred();
        addPromise.then(new PromiseListener() {
            @Override
            public void onRejection(JoynrException error) {
                if (error instanceof ApplicationException) {
                    addToAllDeferred.reject(((ApplicationException) error).getError());
                } else if (error instanceof ProviderRuntimeException) {
                    addToAllDeferred.reject((ProviderRuntimeException) error);
                } else {
                    addToAllDeferred.reject(new ProviderRuntimeException("Unknown error registering provider "
                            + discoveryEntry.getParticipantId() + " in all known backends: " + error));
                }
            }

            @Override
            public void onFulfillment(Object... values) {
                addToAllDeferred.resolve();
            }
        });
        return new Promise<>(addToAllDeferred);
    }

    private void registerGlobal(final DiscoveryEntry discoveryEntry,
                                final String[] gbids,
                                final Add1Deferred deferred,
                                final boolean awaitGlobalRegistration) {
        synchronized (globalAddressLock) {
            try {
                globalAddress = globalAddressProvider.get();
            } catch (Exception e) {
                logger.debug("error getting global address", e);
                globalAddress = null;
            }

            if (globalAddress == null) {
                queuedDiscoveryEntries.add(new QueuedDiscoveryEntry(discoveryEntry,
                                                                    gbids,
                                                                    deferred,
                                                                    awaitGlobalRegistration));
                globalAddressProvider.registerGlobalAddressesReadyListener(this);
                return;
            }
        }

        final GlobalDiscoveryEntry globalDiscoveryEntry = CapabilityUtils.discoveryEntry2GlobalDiscoveryEntry(discoveryEntry,
                                                                                                              globalAddress);
        if (globalDiscoveryEntry != null) {

            logger.info("starting global registration for " + globalDiscoveryEntry.getDomain() + " : "
                    + globalDiscoveryEntry.getInterfaceName());

            globalCapabilitiesDirectoryClient.add(new CallbackWithModeledError<Void, DiscoveryError>() {

                @Override
                public void onSuccess(Void nothing) {
                    logger.info("global registration for " + globalDiscoveryEntry.getParticipantId() + ", "
                            + globalDiscoveryEntry.getDomain() + " : " + globalDiscoveryEntry.getInterfaceName()
                            + " completed");
                    synchronized (globalDiscoveryEntryCache) {
                        mapGbidsToGlobalProviderParticipantId(discoveryEntry.getParticipantId(), gbids);
                        globalDiscoveryEntryCache.add(globalDiscoveryEntry);
                    }
                    deferred.resolve();
                }

                @Override
                public void onFailure(JoynrRuntimeException exception) {
                    logger.info("global registration for " + globalDiscoveryEntry.getParticipantId() + ", "
                            + globalDiscoveryEntry.getDomain() + " : " + globalDiscoveryEntry.getInterfaceName()
                            + " failed");
                    if (awaitGlobalRegistration == true) {
                        localDiscoveryEntryStore.remove(globalDiscoveryEntry.getParticipantId());
                    }
                    deferred.reject(new ProviderRuntimeException(exception.toString()));
                }

                @Override
                public void onFailure(DiscoveryError errorEnum) {
                    logger.info("global registration for " + globalDiscoveryEntry.getParticipantId() + ", "
                            + globalDiscoveryEntry.getDomain() + " : " + globalDiscoveryEntry.getInterfaceName()
                            + " failed");
                    if (awaitGlobalRegistration == true) {
                        localDiscoveryEntryStore.remove(globalDiscoveryEntry.getParticipantId());
                    }
                    deferred.reject(errorEnum);
                }
            }, globalDiscoveryEntry, gbids);
        }
    }

    @Override
    public io.joynr.provider.Promise<io.joynr.provider.DeferredVoid> remove(String participantId) {
        DeferredVoid deferred = new DeferredVoid();
        Optional<DiscoveryEntry> optionalDiscoveryEntry = localDiscoveryEntryStore.lookup(participantId,
                                                                                          Long.MAX_VALUE);
        if (optionalDiscoveryEntry.isPresent()) {
            remove(optionalDiscoveryEntry.get());
            deferred.resolve();
        } else {
            logger.debug("Failed to remove participantId: {}. ParticipantId is not registered in cluster controller.",
                         participantId);
            deferred.reject(new ProviderRuntimeException("Failed to remove participantId: " + participantId
                    + ". ParticipantId is not registered in cluster controller."));
        }
        return new Promise<>(deferred);
    }

    @Override
    public void remove(final DiscoveryEntry discoveryEntry) {
        final String participantId = discoveryEntry.getParticipantId();
        localDiscoveryEntryStore.remove(participantId);
        notifyCapabilityRemoved(discoveryEntry);
        // Remove from the global capabilities directory if needed
        if (discoveryEntry.getQos().getScope() != ProviderScope.LOCAL) {

            CallbackWithModeledError<Void, DiscoveryError> callback = new CallbackWithModeledError<Void, DiscoveryError>() {

                @Override
                public void onSuccess(Void result) {
                    synchronized (globalDiscoveryEntryCache) {
                        globalDiscoveryEntryCache.remove(participantId);
                        globalProviderParticipantIdToGbidListMap.remove(participantId);
                    }
                }

                @Override
                public void onFailure(JoynrRuntimeException error) {
                    // do nothing
                    logger.warn("Failed to remove participantId {}: {}", participantId, error);
                }

                @Override
                public void onFailure(DiscoveryError errorEnum) {
                    switch (errorEnum) {
                    case NO_ENTRY_FOR_PARTICIPANT:
                    case NO_ENTRY_FOR_SELECTED_BACKENDS:
                        // already removed globally
                        logger.warn("Error removing participantId {} globally: {}. Removing local entry.",
                                    participantId,
                                    errorEnum);
                        globalDiscoveryEntryCache.remove(participantId);
                        globalProviderParticipantIdToGbidListMap.remove(participantId);
                        break;
                    case INVALID_GBID:
                    case UNKNOWN_GBID:
                    case INTERNAL_ERROR:
                    default:
                        // do nothing
                        logger.warn("Failed to remove participantId {}: {}", participantId, errorEnum);
                    }
                }
            };
            if (globalProviderParticipantIdToGbidListMap.containsKey(participantId)) {
                List<String> gbidsToRemove = globalProviderParticipantIdToGbidListMap.get(participantId);
                globalCapabilitiesDirectoryClient.remove(callback,
                                                         participantId,
                                                         gbidsToRemove.toArray(new String[gbidsToRemove.size()]));
            } else {
                logger.warn("Participant {} is not registered globally and cannot be removed!", participantId);
            }
        }

        // Remove endpoint addresses
        messageRouter.removeNextHop(participantId);
    }

    @Override
    public Promise<Lookup1Deferred> lookup(String[] domains, String interfaceName, DiscoveryQos discoveryQos) {
        Promise<Lookup2Deferred> lookupPromise = lookup(domains, interfaceName, discoveryQos, new String[]{});
        Lookup1Deferred lookup1Deferred = new Lookup1Deferred();
        lookupPromise.then(new PromiseListener() {
            @Override
            public void onRejection(JoynrException exception) {
                if (exception instanceof ApplicationException) {
                    DiscoveryError error = ((ApplicationException) exception).getError();
                    lookup1Deferred.reject(new ProviderRuntimeException("Error discovering provider for domain "
                            + Arrays.toString(domains) + " and interface " + interfaceName + " in all backends: "
                            + error));
                } else if (exception instanceof ProviderRuntimeException) {
                    lookup1Deferred.reject((ProviderRuntimeException) exception);
                } else {
                    lookup1Deferred.reject(new ProviderRuntimeException("Unknown error discovering provider for domain "
                            + Arrays.toString(domains) + " and interface " + interfaceName + " in all backends: "
                            + exception));
                }
            }

            @Override
            public void onFulfillment(Object... values) {
                lookup1Deferred.resolve((DiscoveryEntryWithMetaInfo[]) values[0]);
            }
        });
        return new Promise<>(lookup1Deferred);
    }

    @Override
    public Promise<Lookup2Deferred> lookup(String[] domains,
                                           String interfaceName,
                                           DiscoveryQos discoveryQos,
                                           String[] gbids) {
        final Lookup2Deferred deferred = new Lookup2Deferred();

        DiscoveryError validationResult = validateGbids(gbids);
        if (validationResult != null) {
            deferred.reject(validationResult);
            return new Promise<>(deferred);
        }
        if (gbids.length == 0) {
            // lookup provider in all known backends
            gbids = knownGbids;
        }

        CapabilitiesCallback callback = new CapabilitiesCallback() {
            @Override
            public void processCapabilitiesReceived(Optional<Collection<DiscoveryEntryWithMetaInfo>> capabilities) {
                if (!capabilities.isPresent()) {
                    deferred.reject(new ProviderRuntimeException("Received capablities collection was null"));
                } else {
                    deferred.resolve(capabilities.get()
                                                 .toArray(new DiscoveryEntryWithMetaInfo[capabilities.get().size()]));
                }
            }

            @Override
            public void onError(Throwable e) {
                deferred.reject(new ProviderRuntimeException(e.toString()));
            }

            @Override
            public void onError(DiscoveryError error) {
                deferred.reject(error);
            }
        };
        lookup(domains, interfaceName, discoveryQos, gbids, callback);

        return new Promise<>(deferred);
    }

    private boolean isEntryForGbids(GlobalDiscoveryEntry entry, Set<String> gbidSet) {
        if (entry == null) {
            return false;
        }
        List<String> entryBackends = globalProviderParticipantIdToGbidListMap.get(entry.getParticipantId());
        if (entryBackends != null) {
            // local provider which is globally registered
            if (gbidSet.stream().anyMatch(gbid -> entryBackends.contains(gbid))) {
                return true;
            }
            return false;
        }

        // globally looked up provider
        Address entryAddress;
        try {
            entryAddress = CapabilityUtils.getAddressFromGlobalDiscoveryEntry(entry);
        } catch (Exception e) {
            logger.error("Error reading address from GlobalDiscoveryEntry: " + entry);
            return false;
        }
        if (entryAddress instanceof MqttAddress) {
            if (!gbidSet.contains(((MqttAddress) entryAddress).getBrokerUri())) {
                // globally looked up provider in wrong backend
                return false;
            }
        }
        // return true for all other address types
        return true;
    }

    private Set<DiscoveryEntryWithMetaInfo> filterGloballyCachedEntriesByGbids(Collection<GlobalDiscoveryEntry> globalEntries,
                                                                               String[] gbids) {
        Set<DiscoveryEntryWithMetaInfo> result = new HashSet<>();
        Set<String> gbidSet = new HashSet<>(Arrays.asList(gbids));
        for (GlobalDiscoveryEntry entry : globalEntries) {
            if (!isEntryForGbids(entry, gbidSet)) {
                continue;
            }
            result.add(CapabilityUtils.convertToDiscoveryEntryWithMetaInfo(false, entry));
        }
        return result;
    }

    /**
     * Searches for capabilities by domain and interface name and gbids.
     *
     * @param domains The Domains for which the search is to be done.
     * @param interfaceName The interface for which the search is to be done.
     * @param discoveryQos The discovery quality of service for the search.
     * @param gbids Global Backend IDs for which (global) providers should be returned.
     * @param capabilitiesCallback Callback to deliver the results asynchronously.
     */
    public void lookup(final String[] domains,
                       final String interfaceName,
                       final DiscoveryQos discoveryQos,
                       String[] gbids,
                       final CapabilitiesCallback capabilitiesCallback) {
        DiscoveryScope discoveryScope = discoveryQos.getDiscoveryScope();
        Set<DiscoveryEntry> localDiscoveryEntries = getLocalEntriesIfRequired(discoveryScope, domains, interfaceName);
        Set<DiscoveryEntryWithMetaInfo> globalDiscoveryEntries = getGloballyCachedEntriesIfRequired(discoveryScope,
                                                                                                    gbids,
                                                                                                    domains,
                                                                                                    interfaceName,
                                                                                                    discoveryQos.getCacheMaxAge());
        switch (discoveryScope) {
        case LOCAL_ONLY:
            capabilitiesCallback.processCapabilitiesReceived(Optional.of(CapabilityUtils.convertToDiscoveryEntryWithMetaInfoSet(true,
                                                                                                                                localDiscoveryEntries)));
            break;
        case LOCAL_THEN_GLOBAL:
            handleLocalThenGlobal(domains,
                                  interfaceName,
                                  discoveryQos,
                                  gbids,
                                  capabilitiesCallback,
                                  CapabilityUtils.convertToDiscoveryEntryWithMetaInfoSet(true, localDiscoveryEntries),
                                  globalDiscoveryEntries);
            break;
        case GLOBAL_ONLY:
            handleGlobalOnly(domains, interfaceName, discoveryQos, gbids, capabilitiesCallback, globalDiscoveryEntries);
            break;
        case LOCAL_AND_GLOBAL:
            handleLocalAndGlobal(domains,
                                 interfaceName,
                                 discoveryQos,
                                 gbids,
                                 capabilitiesCallback,
                                 localDiscoveryEntries,
                                 globalDiscoveryEntries);
            break;
        default:
            throw new IllegalStateException("Unknown or illegal DiscoveryScope value: " + discoveryScope);
        }
    }

    private void handleLocalThenGlobal(String[] domains,
                                       String interfaceName,
                                       DiscoveryQos discoveryQos,
                                       String[] gbids,
                                       CapabilitiesCallback capabilitiesCallback,
                                       Set<DiscoveryEntryWithMetaInfo> localDiscoveryEntries,
                                       Set<DiscoveryEntryWithMetaInfo> globalDiscoveryEntries) {
        Set<String> domainsForGlobalLookup = new HashSet<>();
        Set<DiscoveryEntryWithMetaInfo> matchedDiscoveryEntries = new HashSet<>();
        for (String domainToMatch : domains) {
            boolean domainMatched = addEntriesForDomain(localDiscoveryEntries, matchedDiscoveryEntries, domainToMatch);
            domainMatched = domainMatched
                    || addEntriesForDomain(globalDiscoveryEntries, matchedDiscoveryEntries, domainToMatch);
            if (!domainMatched) {
                domainsForGlobalLookup.add(domainToMatch);
            }
        }
        handleMissingGlobalEntries(interfaceName,
                                   discoveryQos,
                                   gbids,
                                   capabilitiesCallback,
                                   domainsForGlobalLookup,
                                   matchedDiscoveryEntries);
    }

    private void handleLocalAndGlobal(String[] domains,
                                      String interfaceName,
                                      DiscoveryQos discoveryQos,
                                      String[] gbids,
                                      CapabilitiesCallback capabilitiesCallback,
                                      Set<DiscoveryEntry> localDiscoveryEntries,
                                      Set<DiscoveryEntryWithMetaInfo> globalDiscoveryEntries) {
        Set<DiscoveryEntryWithMetaInfo> localDiscoveryEntriesWithMetaInfo = CapabilityUtils.convertToDiscoveryEntryWithMetaInfoSet(true,
                                                                                                                                   localDiscoveryEntries);

        Set<String> domainsForGlobalLookup = new HashSet<>();
        Set<DiscoveryEntryWithMetaInfo> matchedDiscoveryEntries = new HashSet<>();
        for (String domainToMatch : domains) {
            addEntriesForDomain(localDiscoveryEntriesWithMetaInfo, matchedDiscoveryEntries, domainToMatch);
            if (!addNonDuplicatedEntriesForDomain(globalDiscoveryEntries,
                                                  matchedDiscoveryEntries,
                                                  domainToMatch,
                                                  localDiscoveryEntries)) {
                domainsForGlobalLookup.add(domainToMatch);
            }
        }
        handleMissingGlobalEntries(interfaceName,
                                   discoveryQos,
                                   gbids,
                                   capabilitiesCallback,
                                   domainsForGlobalLookup,
                                   matchedDiscoveryEntries);
    }

    private void handleGlobalOnly(String[] domains,
                                  String interfaceName,
                                  DiscoveryQos discoveryQos,
                                  String[] gbids,
                                  CapabilitiesCallback capabilitiesCallback,
                                  Set<DiscoveryEntryWithMetaInfo> globalDiscoveryEntries) {
        Set<String> domainsForGlobalLookup = new HashSet<>(Arrays.asList(domains));
        for (DiscoveryEntry discoveryEntry : globalDiscoveryEntries) {
            domainsForGlobalLookup.remove(discoveryEntry.getDomain());
        }
        handleMissingGlobalEntries(interfaceName,
                                   discoveryQos,
                                   gbids,
                                   capabilitiesCallback,
                                   domainsForGlobalLookup,
                                   globalDiscoveryEntries);
    }

    private void handleMissingGlobalEntries(String interfaceName,
                                            DiscoveryQos discoveryQos,
                                            String[] gbids,
                                            CapabilitiesCallback capabilitiesCallback,
                                            Set<String> domainsForGlobalLookup,
                                            Set<DiscoveryEntryWithMetaInfo> matchedDiscoveryEntries) {
        if (domainsForGlobalLookup.isEmpty()) {
            capabilitiesCallback.processCapabilitiesReceived(Optional.of(matchedDiscoveryEntries));
        } else {
            asyncGetGlobalCapabilitities(gbids,
                                         domainsForGlobalLookup.toArray(new String[domainsForGlobalLookup.size()]),
                                         interfaceName,
                                         matchedDiscoveryEntries,
                                         discoveryQos.getDiscoveryTimeout(),
                                         capabilitiesCallback);
        }
    }

    private boolean addEntriesForDomain(Collection<DiscoveryEntryWithMetaInfo> discoveryEntries,
                                        Collection<DiscoveryEntryWithMetaInfo> addTo,
                                        String domain) {
        boolean domainMatched = false;
        for (DiscoveryEntryWithMetaInfo discoveryEntry : discoveryEntries) {
            if (discoveryEntry.getDomain().equals(domain)) {
                addTo.add(discoveryEntry);
                domainMatched = true;
            }
        }
        return domainMatched;
    }

    private boolean addNonDuplicatedEntriesForDomain(Collection<DiscoveryEntryWithMetaInfo> discoveryEntries,
                                                     Collection<DiscoveryEntryWithMetaInfo> addTo,
                                                     String domain,
                                                     Collection<DiscoveryEntry> possibleDuplicateEntries) {

        boolean domainMatched = false;
        for (DiscoveryEntryWithMetaInfo discoveryEntry : discoveryEntries) {
            if (discoveryEntry.getDomain().equals(domain)) {
                DiscoveryEntry foundDiscoveryEntry = new DiscoveryEntry(discoveryEntry);
                if (!possibleDuplicateEntries.contains(foundDiscoveryEntry)) {
                    addTo.add(discoveryEntry);
                }
                domainMatched = true;
            }
        }
        return domainMatched;
    }

    private Set<DiscoveryEntryWithMetaInfo> getGloballyCachedEntriesIfRequired(DiscoveryScope discoveryScope,
                                                                               String[] gbids,
                                                                               String[] domains,
                                                                               String interfaceName,
                                                                               long cacheMaxAge) {
        if (INCLUDE_GLOBAL_SCOPES.contains(discoveryScope)) {
            Collection<GlobalDiscoveryEntry> globallyCachedEntries = globalDiscoveryEntryCache.lookup(domains,
                                                                                                      interfaceName,
                                                                                                      cacheMaxAge);
            return filterGloballyCachedEntriesByGbids(globallyCachedEntries, gbids);
        }
        return null;
    }

    private Set<DiscoveryEntry> getLocalEntriesIfRequired(DiscoveryScope discoveryScope,
                                                          String[] domains,
                                                          String interfaceName) {
        if (INCLUDE_LOCAL_SCOPES.contains(discoveryScope)) {
            return new HashSet<DiscoveryEntry>(localDiscoveryEntryStore.lookup(domains, interfaceName));
        }
        return null;
    }

    @Override
    public Promise<Lookup3Deferred> lookup(String participantId) {
        DiscoveryQos discoveryQos = new DiscoveryQos(Long.MAX_VALUE,
                                                     Long.MAX_VALUE,
                                                     DiscoveryScope.LOCAL_AND_GLOBAL,
                                                     false);
        Promise<Lookup4Deferred> lookupPromise = lookup(participantId, discoveryQos, new String[]{});
        Lookup3Deferred lookup3Deferred = new Lookup3Deferred();
        lookupPromise.then(new PromiseListener() {
            @Override
            public void onRejection(JoynrException exception) {
                if (exception instanceof ApplicationException) {
                    DiscoveryError error = ((ApplicationException) exception).getError();
                    lookup3Deferred.reject(new ProviderRuntimeException("Error discovering provider " + participantId
                            + " in all backends: " + error));
                } else if (exception instanceof ProviderRuntimeException) {
                    lookup3Deferred.reject((ProviderRuntimeException) exception);
                } else {
                    lookup3Deferred.reject(new ProviderRuntimeException("Unknown error discovering provider "
                            + participantId + " in all backends: " + exception));
                }
            }

            @Override
            public void onFulfillment(Object... values) {
                lookup3Deferred.resolve((DiscoveryEntryWithMetaInfo) values[0]);
            }
        });
        return new Promise<>(lookup3Deferred);
    }

    @Override
    public Promise<Lookup4Deferred> lookup(String participantId, DiscoveryQos discoveryQos, String[] gbids) {
        Lookup4Deferred deferred = new Lookup4Deferred();
        DiscoveryError validationResult = validateGbids(gbids);
        if (validationResult != null) {
            deferred.reject(validationResult);
            return new Promise<>(deferred);
        }
        if (gbids.length == 0) {
            // lookup provider in all known backends
            gbids = knownGbids;
        }

        lookup(participantId, discoveryQos, gbids, new CapabilityCallback() {

            @Override
            public void processCapabilityReceived(Optional<DiscoveryEntryWithMetaInfo> capability) {
                deferred.resolve(capability.isPresent() ? capability.get() : null);
            }

            @Override
            public void onError(Throwable e) {
                deferred.reject(new ProviderRuntimeException(e.toString()));
            }

            @Override
            public void onError(DiscoveryError error) {
                deferred.reject(error);
            }
        });
        return new Promise<>(deferred);
    }

    /**
     * Searches for capability by participantId and gbids. This is an asynchronous method.
     *
     * @param participantId The participant id to search for.
     * @param discoveryQos The discovery quality of service for the search.
     * @param gbids Global Backend IDs for which (global) provider should be returned.
     * @param callback called if the capability with the given participant ID
     *      is retrieved. Or null if not found.
     */
    public void lookup(final String participantId,
                       final DiscoveryQos discoveryQos,
                       final String[] gbids,
                       final CapabilityCallback capabilityCallback) {

        final Optional<DiscoveryEntry> localDiscoveryEntry = localDiscoveryEntryStore.lookup(participantId,
                                                                                             Long.MAX_VALUE);

        DiscoveryScope discoveryScope = discoveryQos.getDiscoveryScope();
        switch (discoveryScope) {
        case LOCAL_ONLY:
            if (localDiscoveryEntry.isPresent()) {
                capabilityCallback.processCapabilityReceived(Optional.of(CapabilityUtils.convertToDiscoveryEntryWithMetaInfo(true,
                                                                                                                             localDiscoveryEntry.get())));
            } else {
                logger.debug("Local only lookup for participantId {} failed with DiscoveryError: {}",
                             participantId,
                             DiscoveryError.NO_ENTRY_FOR_PARTICIPANT);
                capabilityCallback.onError(DiscoveryError.NO_ENTRY_FOR_PARTICIPANT);
            }
            break;
        case LOCAL_THEN_GLOBAL:
        case LOCAL_AND_GLOBAL:
            if (localDiscoveryEntry.isPresent()) {
                capabilityCallback.processCapabilityReceived(Optional.of(CapabilityUtils.convertToDiscoveryEntryWithMetaInfo(true,
                                                                                                                             localDiscoveryEntry.get())));
            } else {
                asyncGetGlobalCapabilitity(gbids, participantId, discoveryQos, capabilityCallback);
            }
            break;
        case GLOBAL_ONLY:
            asyncGetGlobalCapabilitity(gbids, participantId, discoveryQos, capabilityCallback);
            break;
        default:
            break;
        }
    }

    private void registerIncomingEndpoints(Collection<GlobalDiscoveryEntry> caps) {
        for (GlobalDiscoveryEntry ce : caps) {
            // TODO when are entries purged from the messagingEndpointDirectory?
            if (ce.getParticipantId() != null && ce.getAddress() != null) {
                Address address = CapabilityUtils.getAddressFromGlobalDiscoveryEntry(ce);
                final boolean isGloballyVisible = (ce.getQos().getScope() == ProviderScope.GLOBAL);
                final long expiryDateMs = Long.MAX_VALUE;

                messageRouter.addToRoutingTable(ce.getParticipantId(), address, isGloballyVisible, expiryDateMs);
            }
        }
    }

    private void asyncGetGlobalCapabilitity(final String[] gbids,
                                            final String participantId,
                                            DiscoveryQos discoveryQos,
                                            final CapabilityCallback capabilitiesCallback) {
        Optional<GlobalDiscoveryEntry> cachedGlobalCapability = globalDiscoveryEntryCache.lookup(participantId,
                                                                                                 discoveryQos.getCacheMaxAge());

        if (cachedGlobalCapability.isPresent()
                && isEntryForGbids(cachedGlobalCapability.get(), new HashSet<>(Arrays.asList(gbids)))) {
            capabilitiesCallback.processCapabilityReceived(Optional.of(CapabilityUtils.convertToDiscoveryEntryWithMetaInfo(false,
                                                                                                                           cachedGlobalCapability.get())));
        } else {
            globalCapabilitiesDirectoryClient.lookup(new CallbackWithModeledError<GlobalDiscoveryEntry, DiscoveryError>() {

                @Override
                public void onSuccess(GlobalDiscoveryEntry newGlobalDiscoveryEntry) {
                    if (newGlobalDiscoveryEntry != null) {
                        registerIncomingEndpoints(Arrays.asList(newGlobalDiscoveryEntry));
                        globalDiscoveryEntryCache.add(newGlobalDiscoveryEntry);
                        // No need to filter the received GDE by GBIDs: already done in GCD
                        capabilitiesCallback.processCapabilityReceived(Optional.of(CapabilityUtils.convertToDiscoveryEntryWithMetaInfo(false,
                                                                                                                                       newGlobalDiscoveryEntry)));
                    } else {
                        capabilitiesCallback.onError(new NullPointerException("Received capabilities are null"));
                    }
                }

                @Override
                public void onFailure(DiscoveryError errorEnum) {
                    logger.debug("Global lookup for participantId {} failed with DiscoveryError: {}",
                                 participantId,
                                 errorEnum);
                    capabilitiesCallback.onError(errorEnum);
                }

                @Override
                public void onFailure(JoynrRuntimeException exception) {
                    logger.debug("Global lookup for participantId {} failed with exception: {}",
                                 participantId,
                                 exception);
                    capabilitiesCallback.onError(exception);
                }
            }, participantId, discoveryQos.getDiscoveryTimeout(), gbids);
        }

    }

    /**
     * mixes in the localDiscoveryEntries to global capabilities found by participantId
     */
    private void asyncGetGlobalCapabilitities(final String[] gbids,
                                              final String[] domains,
                                              final String interfaceName,
                                              Collection<DiscoveryEntryWithMetaInfo> localDiscoveryEntries2,
                                              long discoveryTimeout,
                                              final CapabilitiesCallback capabilitiesCallback) {

        final Collection<DiscoveryEntryWithMetaInfo> localDiscoveryEntries = localDiscoveryEntries2 == null
                ? new LinkedList<DiscoveryEntryWithMetaInfo>()
                : localDiscoveryEntries2;

        globalCapabilitiesDirectoryClient.lookup(new CallbackWithModeledError<List<GlobalDiscoveryEntry>, DiscoveryError>() {

            @Override
            public void onSuccess(List<GlobalDiscoveryEntry> globalDiscoverEntries) {
                if (globalDiscoverEntries != null) {
                    registerIncomingEndpoints(globalDiscoverEntries);
                    globalDiscoveryEntryCache.add(globalDiscoverEntries);
                    Collection<DiscoveryEntryWithMetaInfo> allDisoveryEntries = new ArrayList<DiscoveryEntryWithMetaInfo>(globalDiscoverEntries.size()
                            + localDiscoveryEntries.size());
                    // No need to filter the received GDEs by GBIDs: already done in GCD
                    allDisoveryEntries.addAll(CapabilityUtils.convertToDiscoveryEntryWithMetaInfoList(false,
                                                                                                      globalDiscoverEntries));
                    allDisoveryEntries.addAll(localDiscoveryEntries);
                    capabilitiesCallback.processCapabilitiesReceived(Optional.of(allDisoveryEntries));
                } else {
                    capabilitiesCallback.onError(new NullPointerException("Received capabilities are null"));
                }
            }

            @Override
            public void onFailure(DiscoveryError errorEnum) {
                logger.debug("Global lookup for domains {} and interface {} failed with DiscoveryError: {}",
                             Arrays.toString(domains),
                             interfaceName,
                             errorEnum);
                capabilitiesCallback.onError(errorEnum);
            }

            @Override
            public void onFailure(JoynrRuntimeException exception) {
                logger.debug("Global lookup for domains {} and interface {} failed with exception: {}",
                             Arrays.toString(domains),
                             interfaceName,
                             exception);
                capabilitiesCallback.onError(exception);
            }
        }, domains, interfaceName, discoveryTimeout, gbids);
    }

    @Override
    public void shutdown(boolean unregisterAllRegisteredCapabilities) {
        if (unregisterAllRegisteredCapabilities) {
            Set<DiscoveryEntry> allDiscoveryEntries = localDiscoveryEntryStore.getAllDiscoveryEntries();

            List<DiscoveryEntry> discoveryEntries = new ArrayList<>(allDiscoveryEntries.size());

            for (DiscoveryEntry capabilityEntry : allDiscoveryEntries) {
                if (capabilityEntry.getQos().getScope() == ProviderScope.GLOBAL) {
                    discoveryEntries.add(capabilityEntry);
                }
            }

            if (discoveryEntries.size() > 0) {
                try {
                    CallbackWithModeledError<Void, DiscoveryError> callback = new CallbackWithModeledError<Void, DiscoveryError>() {

                        @Override
                        public void onFailure(JoynrRuntimeException error) {
                        }

                        @Override
                        public void onSuccess(Void result) {
                        }

                        @Override
                        public void onFailure(DiscoveryError errorEnum) {
                        }

                    };

                    List<String> participantIds = discoveryEntries.stream()
                                                                  .filter(Objects::nonNull)
                                                                  .map(dEntry -> dEntry.getParticipantId())
                                                                  .collect(Collectors.toList());
                    for (String participantId : participantIds) {
                        if (globalProviderParticipantIdToGbidListMap.containsKey(participantId)) {
                            globalCapabilitiesDirectoryClient.remove(callback,
                                                                     participantId,
                                                                     globalProviderParticipantIdToGbidListMap.get(participantId)
                                                                                                             .toArray(new String[0]));
                        }
                    }
                } catch (DiscoveryException e) {
                    logger.debug("error removing discovery entries", e);
                }
            }
        }
    }

    @Override
    public Set<DiscoveryEntry> listLocalCapabilities() {
        return localDiscoveryEntryStore.getAllDiscoveryEntries();
    }

    @Override
    public void transportReady(Optional<Address> address) {
        synchronized (globalAddressLock) {
            globalAddress = address.isPresent() ? address.get() : null;
        }
        for (QueuedDiscoveryEntry queuedDiscoveryEntry : queuedDiscoveryEntries) {
            registerGlobal(queuedDiscoveryEntry.getDiscoveryEntry(),
                           queuedDiscoveryEntry.getGbids(),
                           queuedDiscoveryEntry.getDeferred(),
                           queuedDiscoveryEntry.getAwaitGlobalRegistration());
        }
    }

}
