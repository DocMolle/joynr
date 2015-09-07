package io.joynr.integration;

/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
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

import com.google.inject.Module;

import io.joynr.accesscontrol.StaticDomainAccessControlProvisioningModule;
import io.joynr.arbitration.ArbitrationStrategy;
import io.joynr.arbitration.DiscoveryQos;
import io.joynr.dispatcher.rpc.RequestStatusCode;
import io.joynr.exceptions.DiscoveryException;
import io.joynr.exceptions.JoynrException;
import io.joynr.exceptions.JoynrIllegalStateException;
import io.joynr.exceptions.JoynrRuntimeException;
import io.joynr.exceptions.JoynrTimeoutException;
import io.joynr.exceptions.JoynrWaitExpiredException;
import io.joynr.messaging.MessagingPropertyKeys;
import io.joynr.messaging.MessagingQos;
import io.joynr.provider.Deferred;
import io.joynr.provider.Promise;
import io.joynr.proxy.Callback;
import io.joynr.proxy.Future;
import io.joynr.proxy.ProxyBuilder;
import io.joynr.pubsub.publication.BroadcastFilter;
import io.joynr.pubsub.publication.BroadcastListener;
import io.joynr.runtime.AbstractJoynrApplication;
import io.joynr.runtime.JoynrRuntime;
import io.joynr.runtime.PropertyLoader;
import joynr.exceptions.ApplicationException;
import joynr.OnChangeSubscriptionQos;
import joynr.tests.DefaulttestProvider;
import joynr.tests.testAsync.MethodWithMultipleOutputParametersCallback;
import joynr.tests.testBroadcastInterface.LocationUpdateWithSpeedBroadcastAdapter;
import joynr.tests.testProxy;
import joynr.tests.testSync.MethodWithMultipleOutputParametersReturned;
import joynr.tests.testtypes.AnotherDerivedStruct;
import joynr.tests.testtypes.ComplexTestType;
import joynr.tests.testtypes.ComplexTestType2;
import joynr.tests.testtypes.DerivedStruct;
import joynr.tests.testtypes.TestEnum;
import joynr.types.localisation.GpsFixEnum;
import joynr.types.localisation.GpsLocation;
import joynr.types.localisation.Trip;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.UUID;
import java.util.concurrent.Semaphore;

import static org.junit.Assert.*;
import static org.mockito.Matchers.any;
import static org.mockito.Mockito.*;

public abstract class AbstractProviderProxyEnd2EndTest extends JoynrEnd2EndTest {
    private static final Logger logger = LoggerFactory.getLogger(AbstractProviderProxyEnd2EndTest.class);

    // This timeout must be shared by all integration test environments and
    // cannot be too short.
    private static final int CONST_DEFAULT_TEST_TIMEOUT = 8000;

    TestProvider provider;
    String domain;
    String domainAsync;

    public static final String TEST_STRING = "Test String";
    public static final Integer TEST_INTEGER = 633536;
    private static final TestEnum TEST_ENUM = TestEnum.TWO;
    public static final GpsLocation TEST_COMPLEXTYPE = new GpsLocation(1.0,
                                                                       2.0,
                                                                       2.5,
                                                                       GpsFixEnum.MODE2D,
                                                                       3.0,
                                                                       4.0,
                                                                       5.0,
                                                                       6.0,
                                                                       7L,
                                                                       89L,
                                                                       Integer.MAX_VALUE);

    long timeTookToRegisterProvider;

    @Rule
    public TestName name = new TestName();

    private MessagingQos messagingQos;

    private DiscoveryQos discoveryQos;

    @Mock
    Callback<String> callback;

    @Mock
    Callback<Integer> callbackInteger;

    private TestAsyncProviderImpl providerAsync;

    private JoynrRuntime providerRuntime;
    private JoynrRuntime consumerRuntime;

    // Overridden by test environment implementations
    protected abstract JoynrRuntime getRuntime(Properties joynrConfig, Module... modules);

    @Before
    public void baseSetup() throws Exception {

        MockitoAnnotations.initMocks(this);

        // prints the tests name in the log so we know what we are testing
        String methodName = name.getMethodName();
        logger.info(methodName + " setup beginning...");

        domain = "ProviderProxyEnd2EndTest." + name.getMethodName() + System.currentTimeMillis();
        domainAsync = domain + "Async";
        provisionPermissiveAccessControlEntry(domain, TestProvider.INTERFACE_NAME);
        provisionPermissiveAccessControlEntry(domainAsync, TestProvider.INTERFACE_NAME);

        // use channelNames = test name
        String channelIdProvider = "JavaTest-" + methodName + UUID.randomUUID().getLeastSignificantBits()
                + "-end2endTestProvider";
        String channelIdConsumer = "JavaTest-" + methodName + UUID.randomUUID().getLeastSignificantBits()
                + "-end2endConsumer";

        Properties joynrConfigProvider = PropertyLoader.loadProperties("testMessaging.properties");
        joynrConfigProvider.put(AbstractJoynrApplication.PROPERTY_JOYNR_DOMAIN_LOCAL, "localdomain."
                + UUID.randomUUID().toString());
        joynrConfigProvider.put(MessagingPropertyKeys.CHANNELID, channelIdProvider);
        joynrConfigProvider.put(MessagingPropertyKeys.RECEIVERID, UUID.randomUUID().toString());

        providerRuntime = getRuntime(joynrConfigProvider, new StaticDomainAccessControlProvisioningModule());

        Properties joynrConfigConsumer = PropertyLoader.loadProperties("testMessaging.properties");
        joynrConfigConsumer.put(AbstractJoynrApplication.PROPERTY_JOYNR_DOMAIN_LOCAL, "localdomain."
                + UUID.randomUUID().toString());
        joynrConfigConsumer.put(MessagingPropertyKeys.CHANNELID, channelIdConsumer);
        joynrConfigConsumer.put(MessagingPropertyKeys.RECEIVERID, UUID.randomUUID().toString());

        consumerRuntime = getRuntime(joynrConfigConsumer);

        provider = new TestProvider();

        providerAsync = new TestAsyncProviderImpl();

        // check that registerProvider does not block
        long startTime = System.currentTimeMillis();
        providerRuntime.registerProvider(domain, provider).waitForFullRegistration(CONST_DEFAULT_TEST_TIMEOUT);
        long endTime = System.currentTimeMillis();
        timeTookToRegisterProvider = endTime - startTime;

        providerRuntime.registerProvider(domainAsync, providerAsync)
                       .waitForFullRegistration(CONST_DEFAULT_TEST_TIMEOUT);

        // The timeouts should not be to small because some test environments are slow
        messagingQos = new MessagingQos(10000);
        discoveryQos = new DiscoveryQos(10000, ArbitrationStrategy.HighestPriority, Long.MAX_VALUE);

        // this sleep greatly speeds up the tests (400 ms vs 2500 / test) by
        // making sure the channel is created before first messages sent.
        Thread.sleep(100);
        logger.info("setup finished");

    }

    @After
    public void tearDown() throws InterruptedException {

        if (providerRuntime != null) {
            providerRuntime.shutdown(true);
        }
        if (consumerRuntime != null) {
            consumerRuntime.shutdown(true);
        }
    }

    protected static class TestProvider extends DefaulttestProvider {

        public TestProvider() {
        }

        // change visibility from protected to public for testing purposes
        @Override
        public void fireBroadcast(String broadcastName, List<BroadcastFilter> broadcastFilters, Object... values) {
            super.fireBroadcast(broadcastName, broadcastFilters, values);
        }

        @Override
        public Promise<MethodWithMultipleOutputParametersDeferred> methodWithMultipleOutputParameters() {
            MethodWithMultipleOutputParametersDeferred deferred = new MethodWithMultipleOutputParametersDeferred();
            String aString = TEST_STRING;
            Integer aNumber = TEST_INTEGER;
            GpsLocation aComplexDataType = TEST_COMPLEXTYPE;
            TestEnum anEnumResult = TEST_ENUM;
            deferred.resolve(aString, aNumber, aComplexDataType, anEnumResult);
            return new Promise<MethodWithMultipleOutputParametersDeferred>(deferred);
        }

        @Override
        public Promise<MethodWithEnumParameterDeferred> methodWithEnumParameter(TestEnum input) {
            MethodWithEnumParameterDeferred deferred = new MethodWithEnumParameterDeferred();
            if (TestEnum.ONE.equals(input)) {
                deferred.resolve(1);
            } else if (TestEnum.TWO.equals(input)) {
                deferred.resolve(2);
            } else if (TestEnum.ZERO.equals(input)) {
                deferred.resolve(0);
            } else {
                deferred.resolve(42);
            }
            return new Promise<MethodWithEnumParameterDeferred>(deferred);
        }

        @Override
        public Promise<AddNumbersDeferred> addNumbers(Integer first, Integer second, Integer third) {
            AddNumbersDeferred deferred = new AddNumbersDeferred();
            deferred.resolve(first + second + third);
            return new Promise<AddNumbersDeferred>(deferred);
        }

        @Override
        public Promise<WaitTooLongDeferred> waitTooLong(Long ttl_ms) {
            WaitTooLongDeferred deferred = new WaitTooLongDeferred();
            String returnString = "";
            long enteredAt = System.currentTimeMillis();
            try {
                Thread.sleep(ttl_ms + 1);
            } catch (InterruptedException e) {
                returnString += "InterruptedException... ";
            }
            deferred.resolve(returnString + "time: " + (System.currentTimeMillis() - enteredAt));
            return new Promise<WaitTooLongDeferred>(deferred);
        }

        @Override
        public Promise<MethodWithEnumListReturnDeferred> methodWithEnumListReturn(Integer input) {
            MethodWithEnumListReturnDeferred deferred = new MethodWithEnumListReturnDeferred();
            deferred.resolve(Arrays.asList(TestEnum.getEnumValue(input)));
            return new Promise<MethodWithEnumListReturnDeferred>(deferred);
        }

        @Override
        public Promise<SayHelloDeferred> sayHello() {
            SayHelloDeferred deferred = new SayHelloDeferred();
            deferred.resolve("Hello");
            return new Promise<SayHelloDeferred>(deferred);
        }

        @Override
        public Promise<ToLowerCaseDeferred> toLowerCase(String inputString) {
            ToLowerCaseDeferred deferred = new ToLowerCaseDeferred();
            deferred.resolve(inputString.toLowerCase());
            return new Promise<ToLowerCaseDeferred>(deferred);
        }

        @Override
        public Promise<OptimizeTripDeferred> optimizeTrip(Trip input) {
            OptimizeTripDeferred deferred = new OptimizeTripDeferred();
            deferred.resolve(input);
            return new Promise<OptimizeTripDeferred>(deferred);
        }

        @Override
        public Promise<OverloadedOperation1Deferred> overloadedOperation(DerivedStruct s) {
            OverloadedOperation1Deferred deferred = new OverloadedOperation1Deferred();
            deferred.resolve("DerivedStruct");
            return new Promise<OverloadedOperation1Deferred>(deferred);
        }

        @Override
        public Promise<OverloadedOperation1Deferred> overloadedOperation(AnotherDerivedStruct s) {
            OverloadedOperation1Deferred deferred = new OverloadedOperation1Deferred();
            deferred.resolve("AnotherDerivedStruct");
            return new Promise<OverloadedOperation1Deferred>(deferred);
        }

        @Override
        public Promise<OverloadedOperation2Deferred> overloadedOperation(String input) {
            OverloadedOperation2Deferred deferred = new OverloadedOperation2Deferred();
            int result = Integer.parseInt(input);
            deferred.resolve(new ComplexTestType(result, result));
            return new Promise<OverloadedOperation2Deferred>(deferred);
        }

        @Override
        public Promise<OverloadedOperation3Deferred> overloadedOperation(String input1, String input2) {
            OverloadedOperation3Deferred deferred = new OverloadedOperation3Deferred();
            deferred.resolve(new ComplexTestType2(Integer.parseInt(input1), Integer.parseInt(input2)));
            return new Promise<OverloadedOperation3Deferred>(deferred);
        }

        boolean broadcastSubscriptionArrived = false;

        public void waitForBroadcastSubscription() {
            synchronized (this) {
                while (!broadcastSubscriptionArrived) {
                    try {
                        this.wait();
                    } catch (InterruptedException e) {
                    }
                }
            }
        }

        @Override
        public void registerBroadcastListener(String broadcastName, BroadcastListener broadcastListener) {
            super.registerBroadcastListener(broadcastName, broadcastListener);
            if (broadcastName.equals("locationUpdateWithSpeed") && !broadcastSubscriptionArrived) {
                synchronized (this) {
                    broadcastSubscriptionArrived = true;
                    this.notify();
                }
            }
        }
    }

    protected static class TestAsyncProviderImpl extends DefaulttestProvider {

        @Override
        public Promise<MethodWithEnumReturnValueDeferred> methodWithEnumReturnValue() {
            MethodWithEnumReturnValueDeferred deferred = new MethodWithEnumReturnValueDeferred();
            deferred.resolve(TestEnum.TWO);
            return new Promise<MethodWithEnumReturnValueDeferred>(deferred);
        }

        @Override
        public Promise<Deferred<TestEnum>> getEnumAttributeReadOnly() {
            Deferred<TestEnum> deferred = new Deferred<TestEnum>();
            deferred.resolve(TestEnum.ONE);
            return new Promise<Deferred<TestEnum>>(deferred);
        }

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void registerProviderCreateProxyAndCallMethod() throws DiscoveryException, JoynrIllegalStateException,
                                                          InterruptedException {
        int result;
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        result = proxy.addNumbers(6, 3, 2);
        Assert.assertEquals(11, result);

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void sendObjectsAsArgumentAndReturnValue() throws DiscoveryException, JoynrIllegalStateException,
                                                     InterruptedException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        List<GpsLocation> locationList = new ArrayList<GpsLocation>();
        locationList.add(new GpsLocation(50.1, 20.1, 500.0, GpsFixEnum.MODE3D, 0.0, 0.0, 0.0, 0.0, 0l, 0l, 1000));
        locationList.add(new GpsLocation(50.1, 20.1, 500.0, GpsFixEnum.MODE3D, 0.0, 0.0, 0.0, 0.0, 0l, 0l, 1000));
        locationList.add(new GpsLocation(50.1, 20.1, 500.0, GpsFixEnum.MODE3D, 0.0, 0.0, 0.0, 0.0, 0l, 0l, 1000));
        Trip testObject = new Trip(locationList, "Title");
        Trip result;

        result = proxy.optimizeTrip(testObject);
        Assert.assertEquals(Double.valueOf(500.0), result.getLocations().get(0).getAltitude());

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void asyncMethodCallWithCallback() throws DiscoveryException, JoynrIllegalStateException,
                                             InterruptedException, JoynrWaitExpiredException, ApplicationException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        Future<String> future = proxy.sayHello(callback);
        String answer = future.getReply(30000);
        Assert.assertEquals(RequestStatusCode.OK, future.getStatus().getCode());
        String expected = "Hello";
        Assert.assertEquals(expected, answer);
        verify(callback).resolve(expected);
        verifyNoMoreInteractions(callback);

        @SuppressWarnings("unchecked")
        // needed on jenkins
        Callback<String> callback2 = mock(Callback.class);
        Future<String> future2 = proxy.toLowerCase(callback2, "Argument");
        String answer2 = future2.getReply(30000);
        Assert.assertEquals(RequestStatusCode.OK, future2.getStatus().getCode());
        String expected2 = "argument";

        Assert.assertEquals(expected2, answer2);
        verify(callback2).resolve(expected2);
        verifyNoMoreInteractions(callback2);

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void calledMethodReturnsMultipleOutputParameters() throws Exception {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();
        MethodWithMultipleOutputParametersReturned result = proxy.methodWithMultipleOutputParameters();
        assertEquals(TEST_INTEGER, result.aNumber);
        assertEquals(TEST_STRING, result.aString);
        assertEquals(TEST_COMPLEXTYPE, result.aComplexDataType);
        assertEquals(TEST_ENUM, result.anEnumResult);

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void calledMethodReturnsMultipleOutputParametersAsyncCallback() throws Exception {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        final Object untilCallbackFinished = new Object();
        final Map<String, Object> result = new HashMap<String, Object>();

        proxy.methodWithMultipleOutputParameters(new MethodWithMultipleOutputParametersCallback() {

            @Override
            public void onFailure(JoynrException error) {
                logger.error("error in calledMethodReturnsMultipleOutputParametersAsyncCallback", error);
            }

            @Override
            public void onSuccess(String aString, Integer aNumber, GpsLocation aComplexDataType, TestEnum anEnumResult) {
                result.put("receivedString", aString);
                result.put("receivedNumber", aNumber);
                result.put("receivedComplexDataType", aComplexDataType);
                result.put("receivedEnum", anEnumResult);
                synchronized (untilCallbackFinished) {
                    untilCallbackFinished.notify();
                }
            }
        });

        synchronized (untilCallbackFinished) {
            untilCallbackFinished.wait(CONST_DEFAULT_TEST_TIMEOUT);
        }

        assertEquals(TEST_INTEGER, result.get("receivedNumber"));
        assertEquals(TEST_STRING, result.get("receivedString"));
        assertEquals(TEST_COMPLEXTYPE, result.get("receivedComplexDataType"));
        assertEquals(TEST_ENUM, result.get("receivedEnum"));
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void calledMethodReturnsMultipleOutputParametersAsyncFuture() throws Exception {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        Future<MethodWithMultipleOutputParametersReturned> future = proxy.methodWithMultipleOutputParameters(new MethodWithMultipleOutputParametersCallback() {
            @Override
            public void onFailure(JoynrException error) {
                logger.error("error in calledMethodReturnsMultipleOutputParametersAsyncCallback", error);
            }

            @Override
            public void onSuccess(String aString, Integer aNumber, GpsLocation aComplexDataType, TestEnum anEnumResult) {
                Assert.assertEquals(TEST_INTEGER, aNumber);
                Assert.assertEquals(TEST_STRING, aString);
                Assert.assertEquals(TEST_COMPLEXTYPE, aComplexDataType);
                Assert.assertEquals(TEST_ENUM, anEnumResult);
            }
        });

        MethodWithMultipleOutputParametersReturned reply = future.getReply();
        Assert.assertEquals(TEST_INTEGER, reply.aNumber);
        Assert.assertEquals(TEST_STRING, reply.aString);
        Assert.assertEquals(TEST_COMPLEXTYPE, reply.aComplexDataType);
        Assert.assertEquals(TEST_ENUM, reply.anEnumResult);

    }

    @Ignore
    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void asyncMethodCallWithTtlExpiring() throws DiscoveryException, JoynrIllegalStateException,
                                                InterruptedException, ApplicationException {

        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        long ttl = 2000L;
        MessagingQos testMessagingQos = new MessagingQos(ttl);
        DiscoveryQos testDiscoveryQos = new DiscoveryQos(30000, ArbitrationStrategy.HighestPriority, Long.MAX_VALUE);
        testProxy proxyShortTll = proxyBuilder.setMessagingQos(testMessagingQos)
                                              .setDiscoveryQos(testDiscoveryQos)
                                              .build();

        // the provider waits ttl before responding, causing a ttl
        boolean timeoutExceptionThrown = false;
        // the ttl parameter tells the provider to wait this long before
        // replying, thereby forcing a ttl exception
        Future<String> waitTooLongFuture = proxyShortTll.waitTooLong(callback, ttl * 2);
        try {
            waitTooLongFuture.getReply(10 * ttl); // should never have to wait
            // this long
            // the JoynWaitExpiredException should not be thrown.
        } catch (JoynrWaitExpiredException e) {
            timeoutExceptionThrown = false;
        } catch (JoynrTimeoutException e) {
            timeoutExceptionThrown = true;
        }
        Assert.assertEquals(true, timeoutExceptionThrown);
        Assert.assertEquals(RequestStatusCode.ERROR, waitTooLongFuture.getStatus().getCode());
        verify(callback).onFailure(any(JoynrRuntimeException.class));
        verifyNoMoreInteractions(callback);

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void testMethodWithEnumInputReturnsResult() throws DiscoveryException, JoynrIllegalStateException,
                                                      InterruptedException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        TestEnum input = TestEnum.TWO;

        int result = proxy.methodWithEnumParameter(input);
        assertEquals(2, result);

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void testVoidOperation() throws DiscoveryException, JoynrIllegalStateException, InterruptedException,
                                   JoynrWaitExpiredException, ApplicationException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        final Future<Boolean> future = new Future<Boolean>();
        proxy.voidOperation(new Callback<Void>() {

            @Override
            public void onSuccess(Void result) {
                future.onSuccess(true);
            }

            @Override
            public void onFailure(JoynrException error) {
                future.onFailure(error);
            }

        });
        Boolean reply = future.getReply(8000);

        assertTrue(reply);
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void testAsyncProviderCall() {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domainAsync, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        proxy.methodStringDoubleParameters("text", 42d);
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void testMethodWithNullEnumInputReturnsSomethingSensible() throws DiscoveryException,
                                                                     JoynrIllegalStateException, InterruptedException {
        TestEnum input = null;
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();
        int result = proxy.methodWithEnumParameter(input);
        assertEquals(42, result);
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void sendingANullValueOnceDoesntCrashProvider() throws DiscoveryException, JoynrIllegalStateException,
                                                          InterruptedException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();
        proxy.methodWithEnumParameter(null);
        TestEnum input = TestEnum.TWO;
        int result = proxy.methodWithEnumParameter(input);
        assertEquals(2, result);
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void testEnumAttribute() throws DiscoveryException, JoynrIllegalStateException, InterruptedException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();
        proxy.setEnumAttribute(TestEnum.TWO);
        TestEnum result = proxy.getEnumAttribute();
        assertEquals(TestEnum.TWO, result);
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void testSimpleBroadcast() throws DiscoveryException, JoynrIllegalStateException, InterruptedException {
        final Semaphore broadcastReceived = new Semaphore(0);
        final GpsLocation gpsLocation = new GpsLocation(1.0,
                                                        2.0,
                                                        3.0,
                                                        GpsFixEnum.MODE3D,
                                                        4.0,
                                                        5.0,
                                                        6.0,
                                                        7.0,
                                                        8L,
                                                        9L,
                                                        10);
        final Double currentSpeed = Double.MAX_VALUE;

        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();
        long minInterval_ms = 0;
        long expiryDate = System.currentTimeMillis() + CONST_DEFAULT_TEST_TIMEOUT;
        long publicationTtl_ms = CONST_DEFAULT_TEST_TIMEOUT;
        OnChangeSubscriptionQos subscriptionQos = new OnChangeSubscriptionQos(minInterval_ms,
                                                                              expiryDate,
                                                                              publicationTtl_ms);
        proxy.subscribeToLocationUpdateWithSpeedBroadcast(new LocationUpdateWithSpeedBroadcastAdapter() {

            @Override
            public void onReceive(GpsLocation receivedGpsLocation, Double receivedCurrentSpeed) {
                assertEquals(gpsLocation, receivedGpsLocation);
                assertEquals(currentSpeed, receivedCurrentSpeed);
                broadcastReceived.release();

            }
        }, subscriptionQos);

        // wait to allow the subscription request to arrive at the provider
        provider.waitForBroadcastSubscription();
        provider.fireBroadcast("locationUpdateWithSpeed", null, gpsLocation, currentSpeed);
        broadcastReceived.acquire();

    }

    @Ignore
    // methods that return enums are not working at the moment - see JOYN-1027
    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void testMethodWithEnumOutput() {
        // TODO write this test, once JOYN-1027 is solved
    }

    @Ignore
    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void asyncMethodCallWithCallbackAndParameter() throws DiscoveryException, JoynrIllegalStateException,
                                                         InterruptedException, JoynrWaitExpiredException,
                                                         ApplicationException {

        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);

        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        Future<String> future = proxy.toLowerCase(callback, "Argument");
        String answer = future.getReply(21000);

        Assert.assertEquals(RequestStatusCode.OK, future.getStatus().getCode());
        String expected = "argument";
        Assert.assertEquals(expected, answer);
        verify(callback).resolve(expected);
        verifyNoMoreInteractions(callback);

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void asyncMethodCallWithIntegerParametersAndFuture() throws DiscoveryException, JoynrIllegalStateException,
                                                               InterruptedException, JoynrWaitExpiredException,
                                                               ApplicationException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        Future<Integer> future = proxy.addNumbers(callbackInteger, 1, 2, 3);
        Integer reply = future.getReply(30000);

        Assert.assertEquals(RequestStatusCode.OK, future.getStatus().getCode());
        Integer expected = 6;
        Assert.assertEquals(expected, reply);
        verify(callbackInteger).resolve(expected);
        verifyNoMoreInteractions(callbackInteger);

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void asyncMethodCallWithEnumParametersAndFuture() throws DiscoveryException, JoynrIllegalStateException,
                                                            InterruptedException, JoynrWaitExpiredException,
                                                            ApplicationException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        Future<Integer> future = proxy.methodWithEnumParameter(callbackInteger, TestEnum.TWO);
        Integer reply = future.getReply(40000);

        Integer expected = 2;
        Assert.assertEquals(RequestStatusCode.OK, future.getStatus().getCode());
        Assert.assertEquals(expected, reply);
        verify(callbackInteger).resolve(expected);
        verifyNoMoreInteractions(callbackInteger);

    }

    @Ignore
    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void asyncMethodCallWithEnumListReturned() throws DiscoveryException, JoynrIllegalStateException,
                                                     InterruptedException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        List<TestEnum> enumList = proxy.methodWithEnumListReturn(2);
        assertArrayEquals(new TestEnum[]{ TestEnum.TWO }, enumList.toArray());

    }

    @Ignore
    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void overloadedMethodWithInheritance() throws DiscoveryException, JoynrIllegalStateException,
                                                 InterruptedException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        DerivedStruct derivedStruct = new DerivedStruct();
        AnotherDerivedStruct anotherDerivedStruct = new AnotherDerivedStruct();

        String anotherDerivedResult = proxy.overloadedOperation(anotherDerivedStruct);
        String derivedResult = proxy.overloadedOperation(derivedStruct);
        Assert.assertEquals("DerivedStruct", derivedResult);
        Assert.assertEquals("AnotherDerivedStruct", anotherDerivedResult);

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void overloadedMethodWithDifferentReturnTypes() throws DiscoveryException, JoynrIllegalStateException,
                                                          InterruptedException {
        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);
        testProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        ComplexTestType expectedResult1 = new ComplexTestType(42, 42);
        ComplexTestType2 expectedResult2 = new ComplexTestType2(43, 44);

        ComplexTestType result1 = proxy.overloadedOperation("42");
        ComplexTestType2 result2 = proxy.overloadedOperation("43", "44");

        Assert.assertEquals(expectedResult1, result1);
        Assert.assertEquals(expectedResult2, result2);
    }
}
