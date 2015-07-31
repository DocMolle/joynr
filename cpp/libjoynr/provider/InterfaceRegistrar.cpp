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
#include "joynr/InterfaceRegistrar.h"

#include <cassert>

namespace joynr
{

InterfaceRegistrar* InterfaceRegistrar::registrarInstance = 0;

InterfaceRegistrar::InterfaceRegistrar()
        : requestInterpreters(), requestInterpretersMutex(), requestInterpreterCounts()
{
}

InterfaceRegistrar& InterfaceRegistrar::instance()
{
    static QMutex mutex;

    // Use double-checked locking so that, under normal use, a
    // mutex lock is not required.
    if (!registrarInstance) {
        QMutexLocker locker(&mutex);
        if (!registrarInstance) {
            registrarInstance = new InterfaceRegistrar();
        }
    }

    return *registrarInstance;
}

void InterfaceRegistrar::unregisterRequestInterpreter(const std::string& interfaceName)
{
    QMutexLocker locker(&requestInterpretersMutex);

    QString qInterfaceName(QString::fromStdString(interfaceName));
    // It is a programming error if the request interpreter does not exist
    assert(requestInterpreters.contains(qInterfaceName));

    int count = --requestInterpreterCounts[qInterfaceName];

    // Remove the requestInterpreter if it is no longer needed
    if (count == 0) {
        requestInterpreters.remove(qInterfaceName);
        requestInterpreterCounts.remove(qInterfaceName);
    }
}

QSharedPointer<IRequestInterpreter> InterfaceRegistrar::getRequestInterpreter(
        const std::string& interfaceName)
{
    QMutexLocker locker(&requestInterpretersMutex);
    QString qInterfaceName(QString::fromStdString(interfaceName));

    assert(requestInterpreters.contains(qInterfaceName));
    return requestInterpreters[qInterfaceName];
}

void InterfaceRegistrar::reset()
{
    QMutexLocker locker(&requestInterpretersMutex);
    requestInterpreters.clear();
    requestInterpreterCounts.clear();
}

} // namespace joynr
