/*
 * #%L
 * %%
 * Copyright (C) 2019 BMW Car IT GmbH
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
/*eslint promise/catch-or-return: "off"*/
import GlobalDiscoveryEntry from "../../generated/joynr/types/GlobalDiscoveryEntry";

import CapabilityDiscovery from "../capabilities/discovery/CapabilityDiscovery";
import CapabilitiesStore from "../capabilities/CapabilitiesStore";
import ChannelAddress from "../../generated/joynr/system/RoutingTypes/ChannelAddress";
import MqttMessagingStubFactory from "../messaging/mqtt/MqttMessagingStubFactory";
import MqttMessagingSkeleton from "../messaging/mqtt/MqttMessagingSkeleton";
import MqttAddress from "../../generated/joynr/system/RoutingTypes/MqttAddress";
import SharedMqttClient from "../messaging/mqtt/SharedMqttClient";
import MqttMulticastAddressCalculator from "../messaging/mqtt/MqttMulticastAddressCalculator";
import MessagingSkeletonFactory from "../messaging/MessagingSkeletonFactory";
import MessagingStubFactory from "../messaging/MessagingStubFactory";
import InProcessSkeleton from "../util/InProcessSkeleton";
import InProcessMessagingStubFactory from "../messaging/inprocess/InProcessMessagingStubFactory";
import InProcessAddress from "../messaging/inprocess/InProcessAddress";
import MessagingQos from "../messaging/MessagingQos";
import DiscoveryQos from "../proxy/DiscoveryQos";
import DiscoveryScope from "../../generated/joynr/types/DiscoveryScope";
import * as UtilInternal from "../util/UtilInternal";
import * as CapabilitiesUtil from "../util/CapabilitiesUtil";
import loggingManager from "../system/LoggingManager";
import nanoid from "nanoid";
import { InProcessProvisioning, ShutdownSettings } from "./interface/Provisioning";
import defaultLibjoynrSettings from "./settings/defaultLibjoynrSettings";
import defaultClusterControllerSettings from "./settings/defaultClusterControllerSettings";
import * as Typing from "../util/Typing";
import LongTimer from "../util/LongTimer";
const JoynrStates: {
    SHUTDOWN: "shut down";
    STARTING: "starting";
    STARTED: "started";
    SHUTTINGDOWN: "shutting down";
} = {
    SHUTDOWN: "shut down",
    STARTING: "starting",
    STARTED: "started",
    SHUTTINGDOWN: "shutting down"
};

import JoynrRuntime from "./JoynrRuntime";

const log = loggingManager.getLogger("joynr.start.InProcessRuntime");

/**
 * The InProcessRuntime is the version of the libjoynr-js runtime that hosts its own
 * cluster controller
 *
 * @name InProcessRuntime
 * @constructor
 *
 * @param provisioning
 */
class InProcessRuntime extends JoynrRuntime<InProcessProvisioning> {
    private freshnessIntervalId: any = null;
    public constructor() {
        super();
    }

    /**
     * Starts up the libjoynr instance
     *
     * @returns an A+ promise object, reporting when libjoynr startup is
     *          then({InProcessRuntime}, {Error})-ed
     * @throws {Error} if libjoynr is not in SHUTDOWN state
     */
    public async start(provisioning: InProcessProvisioning): Promise<void> {
        super.start(provisioning);

        if (UtilInternal.checkNullUndefined(provisioning.bounceProxyUrl)) {
            throw new Error("bounce proxy URL not set in provisioning.bounceProxyUrl");
        }
        if (UtilInternal.checkNullUndefined(provisioning.bounceProxyBaseUrl)) {
            throw new Error("bounce proxy base URL not set in provisioning.bounceProxyBaseUrl");
        }
        if (UtilInternal.checkNullUndefined(provisioning.brokerUri)) {
            throw new Error("broker URI not set in provisioning.brokerUri");
        }

        const initialRoutingTable: Record<string, any> = {};

        await this.initializePersistency(provisioning);

        const channelId =
            provisioning.channelId ||
            (this.persistency && this.persistency.getItem("joynr.channels.channelId.1")) ||
            `chjs_${nanoid()}`;
        if (this.persistency) this.persistency.setItem("joynr.channels.channelId.1", channelId);

        let untypedCapabilities = provisioning.capabilities || [];
        const clusterControllerSettings = defaultClusterControllerSettings({
            bounceProxyBaseUrl: provisioning.bounceProxyBaseUrl,
            brokerUri: provisioning.brokerUri
        });
        const defaultClusterControllerCapabilities = clusterControllerSettings.capabilities || [];

        untypedCapabilities = untypedCapabilities.concat(defaultClusterControllerCapabilities);

        this.typeRegistry.addType(ChannelAddress);
        this.typeRegistry.addType(MqttAddress);

        const typedCapabilities = [];
        for (let i = 0; i < untypedCapabilities.length; i++) {
            const capability = new GlobalDiscoveryEntry(untypedCapabilities[i]);
            if (!capability.address) {
                throw new Error(`provisioned capability is missing address: ${JSON.stringify(capability)}`);
            }
            initialRoutingTable[capability.participantId] = Typing.augmentTypes(JSON.parse(capability.address));
            typedCapabilities.push(capability);
        }

        const globalClusterControllerAddress = new MqttAddress({
            brokerUri: "joynrdefaultgbid",
            topic: channelId
        });
        const serializedGlobalClusterControllerAddress = JSON.stringify(globalClusterControllerAddress);

        const mqttClusterControllerAddress = new MqttAddress({
            brokerUri: provisioning.brokerUri,
            topic: channelId
        });

        const mqttClient = new SharedMqttClient({
            address: mqttClusterControllerAddress,
            provisioning: provisioning.mqtt || {}
        });

        const messagingSkeletonFactory = new MessagingSkeletonFactory();

        const messagingStubFactories: Record<string, any> = {};
        messagingStubFactories[InProcessAddress._typeName] = new InProcessMessagingStubFactory();
        //messagingStubFactories[ChannelAddress._typeName] = channelMessagingStubFactory;
        messagingStubFactories[MqttAddress._typeName] = new MqttMessagingStubFactory({
            client: mqttClient
        });

        const messagingStubFactory = new MessagingStubFactory({
            messagingStubFactories
        });

        const messageRouterSettings = {
            initialRoutingTable,
            joynrInstanceId: channelId,
            typeRegistry: this.typeRegistry,
            messagingStubFactory,
            multicastAddressCalculator: new MqttMulticastAddressCalculator({
                globalAddress: globalClusterControllerAddress
            })
        };

        super.initializeComponents(provisioning, messageRouterSettings);

        const mqttMessagingSkeleton = new MqttMessagingSkeleton({
            address: globalClusterControllerAddress,
            client: mqttClient,
            messageRouter: this.messageRouter
        });

        this.messagingSkeletons[MqttAddress._typeName] = mqttMessagingSkeleton;
        messagingSkeletonFactory.setSkeletons(this.messagingSkeletons);

        this.messageRouter.setReplyToAddress(serializedGlobalClusterControllerAddress);

        const localCapabilitiesStore = new CapabilitiesStore(
            CapabilitiesUtil.toDiscoveryEntries(defaultLibjoynrSettings.capabilities || [])
        );
        const globalCapabilitiesCache = new CapabilitiesStore(typedCapabilities);

        const internalMessagingQos = new MessagingQos(provisioning.internalMessagingQos);

        const defaultProxyBuildSettings = {
            domain: "io.joynr",
            messagingQos: internalMessagingQos,
            discoveryQos: new DiscoveryQos({
                discoveryScope: DiscoveryScope.GLOBAL_ONLY,
                cacheMaxAgeMs: UtilInternal.getMaxLongValue()
            })
        };

        const capabilityDiscovery = new CapabilityDiscovery(
            localCapabilitiesStore,
            globalCapabilitiesCache,
            this.messageRouter,
            // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
            this.proxyBuilder!,
            defaultProxyBuildSettings.domain,
            provisioning.gbids || clusterControllerSettings.gbids
        );

        mqttClient.onConnected().then(() => {
            // TODO remove workaround when multiple backend support is implemented in JS
            const globalClusterControllerAddressWithGbid = new MqttAddress({
                brokerUri: "joynrdefaultgbid",
                topic: globalClusterControllerAddress.topic
            });
            capabilityDiscovery.globalAddressReady(globalClusterControllerAddressWithGbid);
        });

        this.discovery.setSkeleton(new InProcessSkeleton(capabilityDiscovery));

        const period = provisioning.capabilitiesFreshnessUpdateIntervalMs || 3600000; // default: 1 hour
        this.freshnessIntervalId = LongTimer.setInterval(() => {
            capabilityDiscovery.touch(channelId, period).catch((error: any) => {
                log.error(`error sending freshness update: ${error}`);
            });
        }, period);

        if (provisioning.logging) {
            loggingManager.configure(provisioning.logging);
        }

        this.joynrState = JoynrStates.STARTED;
        this.publicationManager.restore();
        log.debug("joynr initialized");
    }

    /**
     * Shuts down libjoynr
     *
     * @throws {Error} if libjoynr is not in the STARTED state
     */
    public shutdown(settings: ShutdownSettings): Promise<void> {
        LongTimer.clearInterval(this.freshnessIntervalId);
        return super.shutdown(settings);
    }
}

export = InProcessRuntime;