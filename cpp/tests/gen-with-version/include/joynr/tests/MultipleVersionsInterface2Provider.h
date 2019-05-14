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
#ifndef GENERATED_INTERFACE_JOYNR_TESTS_MULTIPLEVERSIONSINTERFACE2PROVIDER_H
#define GENERATED_INTERFACE_JOYNR_TESTS_MULTIPLEVERSIONSINTERFACE2PROVIDER_H

#include <string>

#include "joynr/PrivateCopyAssign.h"

#include "joynr/IJoynrProvider.h"
#include "joynr/tests/IMultipleVersionsInterface2.h"
#include "joynr/RequestCallerFactory.h"
#include "joynr/tests/MultipleVersionsInterface2RequestCaller.h"

#include "joynr/tests/InterfaceVersionedStruct2.h"
#include <cstdint>
#include "joynr/tests/MultipleVersionsTypeCollection/VersionedStruct2.h"
#include "joynr/tests/AnonymousVersionedStruct2.h"

#include <memory>

namespace joynr { namespace tests { 

/**
 * @brief Provider class for interface MultipleVersionsInterface2
 *
 * @version 2.0
 */
class  MultipleVersionsInterface2Provider : public virtual IJoynrProvider
{

public:
	/** @brief Default constructor */
	MultipleVersionsInterface2Provider();

	//for each Attribute the provider needs setters, sync and async getters.
	//They have default implementation for pushing Providers and can be overwritten by pulling Providers.

	/** @brief Destructor */
	~MultipleVersionsInterface2Provider() override;

	static const std::string& INTERFACE_NAME();
	/**
	 * @brief MAJOR_VERSION The major version of this provider interface as specified in the
	 * Franca model.
	 */
	static const std::uint32_t MAJOR_VERSION;
	/**
	 * @brief MINOR_VERSION The minor version of this provider interface as specified in the
	 * Franca model.
	 */
	static const std::uint32_t MINOR_VERSION;

	// attributes
	/**
	 * @brief Gets UInt8Attribute1
	 *
	 * This method is called by a joynr middleware thread. The provider implementation
	 * must not block this thread; it must be released immediately after the method is
	 * called. Computations or further blocking calls must be performed asynchronously.
	 * Return the result of these computations by calling the onSuccess or onError
	 * callbacks asynchronously. N.B. Internal joynr data structures of joynr messages are
	 * captured in the callbacks and will be first released after the callback onSuccess
	 * or onError is called.
	 *
	 * @param onSuccess A callback function to be called with the attribute value once the asynchronous computation has
	 * finished with success. It expects a request status object as parameter.
	 * @param onError A callback function to be called once the asynchronous computation fails. It expects an exception.
	 */
	virtual void getUInt8Attribute1(
			std::function<void(
					const std::uint8_t&
			)> onSuccess,
			std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
	) = 0;
	/**
	 * @brief Sets UInt8Attribute1
	 *
	 * This method is called by a joynr middleware thread. The provider implementation
	 * must not block this thread; it must be released immediately after the method is
	 * called. Computations or further blocking calls must be performed asynchronously.
	 * Return the result of these computations by calling the onSuccess or onError
	 * callbacks asynchronously. N.B. Internal joynr data structures of joynr messages are
	 * captured in the callbacks and will be first released after the callback onSuccess
	 * or onError is called.
	 *
	 * @param uInt8Attribute1 the new value of the attribute
	 * @param onSuccess A callback function to be called  once the asynchronous computation has
	 * finished with success. It expects a request status object as parameter.
	 * @param onError A callback function to be called once the asynchronous computation fails. It expects an exception.
	 */
	virtual void setUInt8Attribute1(
			const std::uint8_t& uInt8Attribute1,
			std::function<void()> onSuccess,
			std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
	) = 0;
	/**
	 * @brief uInt8Attribute1Changed must be called by a concrete provider
	 * to signal attribute modifications. It is used to implement onchange
	 * subscriptions.
	 * @param uInt8Attribute1 the new attribute value
	 */
	virtual void uInt8Attribute1Changed(
			const std::uint8_t& uInt8Attribute1
	) = 0;

	/**
	 * @brief Gets UInt8Attribute2
	 *
	 * This method is called by a joynr middleware thread. The provider implementation
	 * must not block this thread; it must be released immediately after the method is
	 * called. Computations or further blocking calls must be performed asynchronously.
	 * Return the result of these computations by calling the onSuccess or onError
	 * callbacks asynchronously. N.B. Internal joynr data structures of joynr messages are
	 * captured in the callbacks and will be first released after the callback onSuccess
	 * or onError is called.
	 *
	 * @param onSuccess A callback function to be called with the attribute value once the asynchronous computation has
	 * finished with success. It expects a request status object as parameter.
	 * @param onError A callback function to be called once the asynchronous computation fails. It expects an exception.
	 */
	virtual void getUInt8Attribute2(
			std::function<void(
					const std::uint8_t&
			)> onSuccess,
			std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
	) = 0;
	/**
	 * @brief Sets UInt8Attribute2
	 *
	 * This method is called by a joynr middleware thread. The provider implementation
	 * must not block this thread; it must be released immediately after the method is
	 * called. Computations or further blocking calls must be performed asynchronously.
	 * Return the result of these computations by calling the onSuccess or onError
	 * callbacks asynchronously. N.B. Internal joynr data structures of joynr messages are
	 * captured in the callbacks and will be first released after the callback onSuccess
	 * or onError is called.
	 *
	 * @param uInt8Attribute2 the new value of the attribute
	 * @param onSuccess A callback function to be called  once the asynchronous computation has
	 * finished with success. It expects a request status object as parameter.
	 * @param onError A callback function to be called once the asynchronous computation fails. It expects an exception.
	 */
	virtual void setUInt8Attribute2(
			const std::uint8_t& uInt8Attribute2,
			std::function<void()> onSuccess,
			std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
	) = 0;
	/**
	 * @brief uInt8Attribute2Changed must be called by a concrete provider
	 * to signal attribute modifications. It is used to implement onchange
	 * subscriptions.
	 * @param uInt8Attribute2 the new attribute value
	 */
	virtual void uInt8Attribute2Changed(
			const std::uint8_t& uInt8Attribute2
	) = 0;

	// methods
	/**
	 * @brief Implementation of the Franca method getTrue
	 *
	 * This method is called by a joynr middleware thread. The provider implementation
	 * must not block this thread; it must be released immediately after the method is
	 * called. Computations or further blocking calls must be performed asynchronously.
	 * Return the result of these computations by calling the onSuccess or onError
	 * callbacks asynchronously. N.B. Internal joynr data structures of joynr messages are
	 * captured in the callbacks and will be first released after the callback onSuccess
	 * or onError is called.
	 *
	 * @param onSuccess A callback function to be called  once the asynchronous computation has
	 * finished with success. It expects a request status object as parameter.
	 * @param onError A callback function to be called once the asynchronous computation fails. It expects an exception.
	 */
	virtual void getTrue(
			std::function<void(
					const bool& result
			)> onSuccess,
			std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
	) = 0;

	/**
	 * @brief Implementation of the Franca method getVersionedStruct
	 *
	 * This method is called by a joynr middleware thread. The provider implementation
	 * must not block this thread; it must be released immediately after the method is
	 * called. Computations or further blocking calls must be performed asynchronously.
	 * Return the result of these computations by calling the onSuccess or onError
	 * callbacks asynchronously. N.B. Internal joynr data structures of joynr messages are
	 * captured in the callbacks and will be first released after the callback onSuccess
	 * or onError is called.
	 *
	 * @param onSuccess A callback function to be called  once the asynchronous computation has
	 * finished with success. It expects a request status object as parameter.
	 * @param onError A callback function to be called once the asynchronous computation fails. It expects an exception.
	 */
	virtual void getVersionedStruct(
			const joynr::tests::MultipleVersionsTypeCollection::VersionedStruct2& input,
			std::function<void(
					const joynr::tests::MultipleVersionsTypeCollection::VersionedStruct2& result
			)> onSuccess,
			std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
	) = 0;

	/**
	 * @brief Implementation of the Franca method getAnonymousVersionedStruct
	 *
	 * This method is called by a joynr middleware thread. The provider implementation
	 * must not block this thread; it must be released immediately after the method is
	 * called. Computations or further blocking calls must be performed asynchronously.
	 * Return the result of these computations by calling the onSuccess or onError
	 * callbacks asynchronously. N.B. Internal joynr data structures of joynr messages are
	 * captured in the callbacks and will be first released after the callback onSuccess
	 * or onError is called.
	 *
	 * @param onSuccess A callback function to be called  once the asynchronous computation has
	 * finished with success. It expects a request status object as parameter.
	 * @param onError A callback function to be called once the asynchronous computation fails. It expects an exception.
	 */
	virtual void getAnonymousVersionedStruct(
			const joynr::tests::AnonymousVersionedStruct2& input,
			std::function<void(
					const joynr::tests::AnonymousVersionedStruct2& result
			)> onSuccess,
			std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
	) = 0;

	/**
	 * @brief Implementation of the Franca method getInterfaceVersionedStruct
	 *
	 * This method is called by a joynr middleware thread. The provider implementation
	 * must not block this thread; it must be released immediately after the method is
	 * called. Computations or further blocking calls must be performed asynchronously.
	 * Return the result of these computations by calling the onSuccess or onError
	 * callbacks asynchronously. N.B. Internal joynr data structures of joynr messages are
	 * captured in the callbacks and will be first released after the callback onSuccess
	 * or onError is called.
	 *
	 * @param onSuccess A callback function to be called  once the asynchronous computation has
	 * finished with success. It expects a request status object as parameter.
	 * @param onError A callback function to be called once the asynchronous computation fails. It expects an exception.
	 */
	virtual void getInterfaceVersionedStruct(
			const joynr::tests::InterfaceVersionedStruct2& input,
			std::function<void(
					const joynr::tests::InterfaceVersionedStruct2& result
			)> onSuccess,
			std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
	) = 0;

private:
	DISALLOW_COPY_AND_ASSIGN(MultipleVersionsInterface2Provider);
};

} // namespace tests
} // namespace joynr


namespace joynr {

// specialization of traits class RequestCallerTraits
// this links MultipleVersionsInterface2Provider with MultipleVersionsInterface2RequestCaller
template <>
struct RequestCallerTraits<joynr::tests::MultipleVersionsInterface2Provider>
{
	using RequestCaller = joynr::tests::MultipleVersionsInterface2RequestCaller;
};

} // namespace joynr

#endif // GENERATED_INTERFACE_JOYNR_TESTS_MULTIPLEVERSIONSINTERFACE2PROVIDER_H
