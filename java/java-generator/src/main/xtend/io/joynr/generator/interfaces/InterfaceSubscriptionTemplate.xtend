package io.joynr.generator.interfaces
/*
 * !!!
 *
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
 *
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
 */

import com.google.inject.Inject
import io.joynr.generator.util.TemplateBase
import org.franca.core.franca.FInterface
import io.joynr.generator.util.JoynrJavaGeneratorExtensions
import com.google.common.collect.Collections2
import java.util.HashSet
import io.joynr.generator.util.InterfaceTemplate
import io.joynr.generator.util.JavaTypeUtil

class InterfaceSubscriptionTemplate implements InterfaceTemplate{
	@Inject	extension JoynrJavaGeneratorExtensions
	@Inject extension JavaTypeUtil
	@Inject extension TemplateBase

	override generate(FInterface serviceInterface) {
		val interfaceName =  serviceInterface.joynrName
		val subscriptionClassName = interfaceName + "SubscriptionInterface"
		val packagePath = getPackagePathWithJoynrPrefix(serviceInterface, ".")

		'''
		«warning()»
		package «packagePath»;

		«IF needsListImport(serviceInterface, false, true)»
		import java.util.List;

		«ENDIF»
		import io.joynr.dispatcher.rpc.JoynrSubscriptionInterface;

		«IF getAttributes(serviceInterface).size > 0»
		import com.fasterxml.jackson.core.type.TypeReference;
		«IF hasReadAttribute(serviceInterface)»
		import io.joynr.dispatcher.rpc.annotation.JoynrRpcSubscription;
		import io.joynr.pubsub.subscription.AttributeSubscriptionListener;
		import io.joynr.pubsub.SubscriptionQos;
		«ENDIF»
		«ENDIF»

		«FOR datatype: getRequiredIncludesFor(serviceInterface, false, false, false, true, false)»
			import «datatype»;
		«ENDFOR»

		public interface «subscriptionClassName» extends JoynrSubscriptionInterface, «interfaceName» {

		«val attrTypeset = new HashSet(Collections2::transform(getAttributes(serviceInterface), [attribute | attribute.typeName]))»

			«FOR attributeType: attrTypeset»
			public static class «attributeType.tokenTypeForArrayType»Reference extends TypeReference<«attributeType»> {}
			«ENDFOR»

		«FOR attribute: getAttributes(serviceInterface)»
		«var attributeName = attribute.joynrName»
		«var attributeType = attribute.typeName.objectDataTypeForPlainType»
			«IF isNotifiable(attribute)»

				@JoynrRpcSubscription(attributeName = "«attributeName»", attributeType = «attributeType.tokenTypeForArrayType»Reference.class)
				public String subscribeTo«attributeName.toFirstUpper»(AttributeSubscriptionListener<«attributeType»> listener, SubscriptionQos subscriptionQos);

				@JoynrRpcSubscription(attributeName = "«attributeName»", attributeType = «attributeType.tokenTypeForArrayType»Reference.class)
				public String subscribeTo«attributeName.toFirstUpper»(AttributeSubscriptionListener<«attributeType»> listener, SubscriptionQos subscriptionQos, String subscriptionId);

				public void unsubscribeFrom«attributeName.toFirstUpper»(String subscriptionId);
			«ENDIF»
		«ENDFOR»
		}
'''
	}
}