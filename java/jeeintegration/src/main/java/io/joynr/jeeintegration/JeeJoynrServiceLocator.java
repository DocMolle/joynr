/**
 *
 */
package io.joynr.jeeintegration;

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

import static java.lang.String.format;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

import javax.ejb.Singleton;
import javax.inject.Inject;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import io.joynr.arbitration.DiscoveryQos;
import io.joynr.dispatcher.rpc.JoynrInterface;
import io.joynr.jeeintegration.api.ServiceLocator;
import io.joynr.messaging.MessagingQos;

/**
 * JEE integration joynr service locator which uses a joynr proxy to provide an implementation for a service interface.
 * The BCI service interface is mapped to its joynr *Proxy equivalent by naming convention; the BCI interface must be in
 * the same package as the Proxy interface, and the name preceding the appendage [BCI|Proxy] must be the same. So if we
 * have <code>a.b.c.MyServiceBCI</code> we must be able to find <code>a.b.c.MyServiceProxy</code>, which must also be a
 * valid joynr proxy interface.
 *
 * @author clive.jevons commissioned by MaibornWolff
 */
@Singleton
public class JeeJoynrServiceLocator implements ServiceLocator {

    private static final Logger LOG = LoggerFactory.getLogger(JeeJoynrServiceLocator.class);

    private final JoynrIntegrationBean joynrIntegrationBean;

    @Inject
    public JeeJoynrServiceLocator(JoynrIntegrationBean joynrIntegrationBean) {
        this.joynrIntegrationBean = joynrIntegrationBean;
    }

    @Override
    public <I> I get(Class<I> serviceInterface, String domain) {
        return get(serviceInterface, domain, new MessagingQos(), new DiscoveryQos());
    }

    @Override
    public <I> I get(Class<I> serviceInterface, String domain, long ttl) {
        return get(serviceInterface, domain, new MessagingQos(ttl), new DiscoveryQos());
    }

    @Override
    @SuppressWarnings("unchecked")
    public <I> I get(Class<I> serviceInterface, String domain, MessagingQos messagingQos, DiscoveryQos discoveryQos) {
        if (joynrIntegrationBean.getRuntime() == null) {
            throw new IllegalStateException("You can't get service proxies until the joynr runtime has been initialised.");
        }
        final Class<? extends JoynrInterface> joynrProxyInterface = findJoynrProxyInterface(serviceInterface);
        final JoynrInterface joynrProxy = joynrIntegrationBean.getRuntime()
                                                              .getProxyBuilder(domain, joynrProxyInterface)
                                                              .setMessagingQos(messagingQos)
                                                              .setDiscoveryQos(discoveryQos)
                                                              .build();
        return (I) Proxy.newProxyInstance(serviceInterface.getClassLoader(),
                                          new Class<?>[]{ serviceInterface },
                                          new InvocationHandler() {

                                              @Override
                                              public Object invoke(Object proxy, Method method, Object[] args)
                                                                                                              throws Throwable {
                                                  if (LOG.isTraceEnabled()) {
                                                      LOG.trace(format("Forwarding call to %s from service interface %s to joynr proxy %s",
                                                                       method,
                                                                       serviceInterface,
                                                                       joynrProxyInterface));
                                                  }
                                                  return joynrProxy.getClass()
                                                                   .getMethod(method.getName(),
                                                                              method.getParameterTypes())
                                                                   .invoke(joynrProxy, args);
                                              }
                                          });
    }

    private <I> Class<? extends JoynrInterface> findJoynrProxyInterface(Class<I> serviceInterface) {
        String joynrProxyInterfaceName = serviceInterface.getName().replaceAll("BCI$", "Proxy");
        if (LOG.isDebugEnabled()) {
            LOG.debug(format("Looking for joynr proxy interface named %s for BCI interface %s",
                             joynrProxyInterfaceName,
                             serviceInterface));
        }
        try {
            @SuppressWarnings("unchecked")
            Class<? extends JoynrInterface> result = (Class<? extends JoynrInterface>) Class.forName(joynrProxyInterfaceName);
            if (!JoynrInterface.class.isAssignableFrom(result)) {
                throw new IllegalArgumentException(format("The Proxy class %s found for the BCI interface %s does not extend JoynrInterface.",
                                                          result,
                                                          serviceInterface));
            }
            return result;
        } catch (ClassNotFoundException e) {
            throw new IllegalArgumentException(format("Unable to find suitable joynr proxy interface named %s for BCI interface %s",
                                                      joynrProxyInterfaceName,
                                                      serviceInterface));
        }
    }

}
