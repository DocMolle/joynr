package io.joynr.capabilities;

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

import java.util.List;

import com.google.common.collect.Lists;

public abstract class AbstractLocalCapabilitiesDirectory implements LocalCapabilitiesDirectory {
    List<CapabilityListener> capabilityListeners = Lists.newArrayList();

    @Override
    public void addCapabilityListener(CapabilityListener listener) {
        if (capabilityListeners.contains(listener)) {
            return;
        }
        capabilityListeners.add(listener);
    }

    @Override
    public void removeCapabilityListener(CapabilityListener listener) {
        capabilityListeners.remove(listener);
    }

    /**
     * Notifies all capability listeners about a newly added capability entry.
     * @param addedCapability the added entry.
     */
    protected void notifyCapabilityAdded(CapabilityEntry addedCapability) {
        for (CapabilityListener capabilityListener : capabilityListeners) {
            capabilityListener.capabilityAdded(addedCapability);
        }
    }

    /**
     * Notifies all capability listeners about a rmoved capability entry.
     * @param removedCapability the removed entry.
     */
    protected void notifyCapabilityRemoved(CapabilityEntry removedCapability) {
        for (CapabilityListener capabilityListener : capabilityListeners) {
            capabilityListener.capabilityRemoved(removedCapability);
        }
    }
}
