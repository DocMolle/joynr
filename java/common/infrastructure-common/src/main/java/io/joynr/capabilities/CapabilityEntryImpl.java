package io.joynr.capabilities;

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

import io.joynr.endpoints.EndpointAddressBase;
import io.joynr.endpoints.JoynrMessagingEndpointAddress;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import javax.annotation.CheckForNull;

import joynr.types.CapabilityInformation;
import joynr.types.ProviderQos;

public class CapabilityEntryImpl implements CapabilityEntry {

    protected String participantId;
    protected String domain;
    protected String interfaceName;

    protected ProviderQos providerQos;

    protected List<EndpointAddressBase> endpointAddresses;

    protected long dateWhenRegistered;
    protected Origin origin;

    public CapabilityEntryImpl() {
        origin = Origin.LOCAL;
    }

    public CapabilityEntryImpl(String domain,
                               String interfaceName,
                               ProviderQos providerQos,
                               String participantId,
                               long dateWhenRegistered,
                               EndpointAddressBase... endpointAddresses) {

        this.domain = domain;
        this.interfaceName = interfaceName;
        this.providerQos = providerQos;
        this.participantId = participantId;
        this.dateWhenRegistered = dateWhenRegistered;
        origin = Origin.LOCAL;
        this.endpointAddresses = new ArrayList<EndpointAddressBase>();
        this.endpointAddresses.addAll(Arrays.asList(endpointAddresses));
    }

    public CapabilityEntryImpl(CapabilityInformation capInfo) {
        this(capInfo.getDomain(),
             capInfo.getInterfaceName(),
             capInfo.getProviderQos(),
             capInfo.getParticipantId(),
             System.currentTimeMillis(),
             // Assume the Capability entry is not local because it has been serialized
             new JoynrMessagingEndpointAddress(capInfo.getChannelId()));
    }

    public static CapabilityEntryImpl fromCapabilityInformation(CapabilityInformation capInfo) {
        return new CapabilityEntryImpl(capInfo.getDomain(),
                                       capInfo.getInterfaceName(),
                                       capInfo.getProviderQos(),
                                       capInfo.getParticipantId(),
                                       System.currentTimeMillis(),
                                       // Assume the Capability entry is not local because it has been serialized
                                       new JoynrMessagingEndpointAddress(capInfo.getChannelId()));
    }

    /* (non-Javadoc)
     * @see io.joynr.capabilities.CapabilityEntry#toCapabilityInformation()
     */
    @Override
    @CheckForNull
    public CapabilityInformation toCapabilityInformation() {
        String channelId = null;
        for (EndpointAddressBase endpointAddress : getEndpointAddresses()) {
            if (endpointAddress instanceof JoynrMessagingEndpointAddress) {
                channelId = ((JoynrMessagingEndpointAddress) endpointAddress).getChannelId();
                break;
            }
        }
        if (channelId == null) {
            return null;
        }
        return new CapabilityInformation(getDomain(),
                                         getInterfaceName(),
                                         new ProviderQos(getProviderQos()),
                                         channelId,
                                         getParticipantId());
    }

    /* (non-Javadoc)
     * @see io.joynr.capabilities.CapabilityEntry#getProviderQos()
     */
    @Override
    public ProviderQos getProviderQos() {
        return providerQos;
    }

    /* (non-Javadoc)
     * @see io.joynr.capabilities.CapabilityEntry#getEndpointAddresses()
     */
    @Override
    public List<EndpointAddressBase> getEndpointAddresses() {
        return endpointAddresses;
    }

    /* (non-Javadoc)
     * @see io.joynr.capabilities.CapabilityEntry#getParticipantId()
     */
    @Override
    public String getParticipantId() {
        return participantId;
    }

    /* (non-Javadoc)
     * @see io.joynr.capabilities.CapabilityEntry#getDomain()
     */
    @Override
    public String getDomain() {
        return domain;
    }

    /* (non-Javadoc)
     * @see io.joynr.capabilities.CapabilityEntry#getInterfaceName()
     */
    @Override
    public String getInterfaceName() {
        return interfaceName;
    }

    /* (non-Javadoc)
     * @see io.joynr.capabilities.CapabilityEntry#getDateWhenRegistered()
     */
    @Override
    public long getDateWhenRegistered() {
        return dateWhenRegistered;
    }

    protected Origin getOrigin() {
        return origin;
    }

    /* (non-Javadoc)
     * @see io.joynr.capabilities.CapabilityEntry#setDateWhenRegistered(long)
     */
    @Override
    public void setDateWhenRegistered(long dateWhenRegistered) {
        this.dateWhenRegistered = dateWhenRegistered;
    }

    @Override
    public void setOrigin(Origin origin) {
        this.origin = origin;
    }

    protected void setProviderQos(ProviderQos providerQos) {
        this.providerQos = providerQos;
    }

    protected final void setEndpointAddresses(List<EndpointAddressBase> endpointAddresses) {
        this.endpointAddresses = endpointAddresses;
    }

    protected final void setParticipantId(String participantId) {
        this.participantId = participantId;
    }

    protected final void setDomain(String domain) {
        this.domain = domain;
    }

    protected final void setInterfaceName(String interfaceName) {
        this.interfaceName = interfaceName;
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("CapabilityEntry [providerQos=")
               .append(providerQos)
               .append(", endpointAddresses=")
               .append(endpointAddresses)
               .append(", participantId=")
               .append(participantId)
               .append(", domain=")
               .append(domain)
               .append(", interfaceName=")
               .append(interfaceName)
               .append(", dateWhenRegistered=")
               .append(getDateWhenRegistered())
               .append("]");
        return builder.toString();
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + (int) (getDateWhenRegistered() ^ (getDateWhenRegistered() >>> 32));
        result = prime * result + ((domain == null) ? 0 : domain.hashCode());
        result = prime * result + ((endpointAddresses == null) ? 0 : endpointAddresses.hashCode());
        result = prime * result + ((interfaceName == null) ? 0 : interfaceName.hashCode());
        result = prime * result + ((participantId == null) ? 0 : participantId.hashCode());
        result = prime * result + ((providerQos == null) ? 0 : providerQos.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        CapabilityEntryImpl other = (CapabilityEntryImpl) obj;
        if (participantId == null) {
            if (other.participantId != null) {
                return false;
            }
        } else if (!participantId.equals(other.participantId)) {
            return false;
        }
        if (domain == null) {
            if (other.domain != null) {
                return false;
            }
        } else if (!domain.equals(other.domain)) {
            return false;
        }
        if (endpointAddresses == null) {
            if (other.endpointAddresses != null) {
                return false;
            }
        } else if (!endpointAddresses.equals(other.endpointAddresses)) {
            return false;
        }
        if (interfaceName == null) {
            if (other.interfaceName != null) {
                return false;
            }
        } else if (!interfaceName.equals(other.interfaceName)) {
            return false;
        }
        if (providerQos == null) {
            if (other.providerQos != null) {
                return false;
            }
        } else if (!providerQos.equals(other.providerQos)) {
            return false;
        }
        return true;
    }

    @Override
    public void addEndpoint(EndpointAddressBase endpointAddress) {
        if (!endpointAddresses.contains(endpointAddress)) {
            endpointAddresses.add(endpointAddress);
        }
    }
}
