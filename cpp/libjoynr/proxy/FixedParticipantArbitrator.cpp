/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2016 BMW Car IT GmbH
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
#include "joynr/FixedParticipantArbitrator.h"
#include "joynr/ArbitrationStatus.h"
#include "joynr/system/IDiscovery.h"
#include "joynr/types/DiscoveryEntry.h"
#include "joynr/system/RoutingTypes/ChannelAddress.h"
#include "joynr/DiscoveryQos.h"
#include "joynr/exceptions/JoynrException.h"
#include "joynr/TypeUtil.h"

namespace joynr
{

INIT_LOGGER(FixedParticipantArbitrator);

FixedParticipantArbitrator::FixedParticipantArbitrator(
        const std::string& domain,
        const std::string& interfaceName,
        const joynr::types::Version& interfaceVersion,
        joynr::system::IDiscoverySync& discoveryProxy,
        const DiscoveryQos& discoveryQos)
        : ProviderArbitrator(domain, interfaceName, interfaceVersion, discoveryProxy, discoveryQos),
          participantId(discoveryQos.getCustomParameter("fixedParticipantId").getValue()),
          reqCacheDataFreshness(discoveryQos.getCacheMaxAgeMs())
{
}

void FixedParticipantArbitrator::attemptArbitration()
{
    joynr::types::DiscoveryEntry result;
    try {
        discoveryProxy.lookup(result, participantId);
        joynr::types::Version providerVersion = result.getProviderVersion();
        if (providerVersion.getMajorVersion() != interfaceVersion.getMajorVersion() ||
            providerVersion.getMinorVersion() < interfaceVersion.getMinorVersion()) {
            throw joynr::exceptions::DiscoveryException(
                    "Provider with participantId " + participantId + " version is not compatible." +
                    std::to_string(providerVersion.getMajorVersion()) + "." +
                    std::to_string(providerVersion.getMinorVersion()) + " wanted: " +
                    std::to_string(interfaceVersion.getMajorVersion()) + "." +
                    std::to_string(interfaceVersion.getMinorVersion()));
        }
        updateArbitrationStatusParticipantIdAndAddress(
                ArbitrationStatus::ArbitrationSuccessful, participantId);
    } catch (const exceptions::JoynrException& e) {
        JOYNR_LOG_ERROR(logger,
                        "Unable to lookup provider (domain: {}, interface: {}) "
                        "from discovery. Error: {}",
                        domains.size() > 0 ? domains.at(0) : "EMPTY",
                        interfaceName,
                        e.getMessage());
    }
}

} // namespace joynr
