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
package io.joynr.util;

import com.google.inject.AbstractModule;
import com.google.inject.Provides;

import io.joynr.messaging.routing.RoutingTable;

public class PreconfiguredEndpointDirectoryModule extends AbstractModule {
    RoutingTable routingTable;

    public PreconfiguredEndpointDirectoryModule(RoutingTable messagingEndpointDirectory) {
        this.routingTable = messagingEndpointDirectory;
    }

    @Provides
    RoutingTable provideRoutingTable() {
        return routingTable;
    }

    @Override
    protected void configure() {
        //do nothing
    }
}
