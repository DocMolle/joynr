/*
 *
 * Copyright (C) 2011 - 2018 BMW Car IT GmbH
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

// #####################################################
//#######################################################
//###                                                 ###
//##    WARNING: This file is generated. DO NOT EDIT   ##
//##             All changes will be lost!             ##
//###                                                 ###
//#######################################################
// #####################################################

package joynr.tests.performance;

import io.joynr.StatelessAsync;
import io.joynr.dispatcher.rpc.annotation.StatelessCallbackCorrelation;
import io.joynr.proxy.MessageIdCallback;
import io.joynr.UsedBy;

import joynr.tests.performance.Types.ComplexStruct;

@StatelessAsync
@UsedBy(EchoProxy.class)
public interface EchoStatelessAsync extends Echo {

	/*
	* simpleAttribute getter
	*/
	@StatelessCallbackCorrelation("2065380052")
	void getSimpleAttribute(MessageIdCallback messageIdCallback);
	/*
	* simpleAttribute setter
	*/
	@StatelessCallbackCorrelation("-1309553592")
	void setSimpleAttribute(String simpleAttribute, MessageIdCallback messageIdCallback);

	/*
	* echoString
	*/
	@StatelessCallbackCorrelation("-1172639566")
	void echoString(
			String data,
			MessageIdCallback messageIdCallback
	);

	/*
	* echoByteArray
	*/
	@StatelessCallbackCorrelation("-1728974786")
	void echoByteArray(
			Byte[] data,
			MessageIdCallback messageIdCallback
	);

	/*
	* echoComplexStruct
	*/
	@StatelessCallbackCorrelation("716873786")
	void echoComplexStruct(
			ComplexStruct data,
			MessageIdCallback messageIdCallback
	);
}
