/*
 * #%L
 * %%
 * Copyright (C) 2020 BMW Car IT GmbH
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
package io.joynr.generator.interfaces;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.Map;

import org.junit.Before;
import org.junit.Test;

import io.joynr.generator.AbstractJoynrTSGeneratorTest;

public class ProviderTest extends AbstractJoynrTSGeneratorTest {
    @Before
    public void setup() throws Exception {
        final boolean generateProxy = false;
        final boolean generateProvider = true;
        super.setup(generateProxy, generateProvider);
    }

    @Test
    public void testProviderAndProxyCodeFound() {
        Map<String, String> result = generate("multi-out-method-test.fidl");
        assertNotNull(result);
        boolean providerFound = false;
        // since the user code might need parts of the generated proxy code
        // as well at the moment even for provider implementation, we do not
        //  check for non-existance of proxy files here.
        for (Map.Entry<String, String> entry : result.entrySet()) {
            if (entry.getKey().contains("Provider")) {
                providerFound = true;
            }
        }
        assertTrue("Expected provider files not found", providerFound);
    }
}
