/*
 * This source file is part of ARK
 * For the latest info, see https://github.com/ArkNX
 *
 * Copyright (c) 2013-2020 ArkNX authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#pragma once

#include <zephyr/zephyr.hpp>

#include "interface/AFIModule.hpp"
// Protobuf header files below platform.hpp cuz the std::min and std::numeric_limit<T>::min;
#include <google/protobuf/message.h>

namespace ark {

class AFIHttpClientModule : public AFIModule {
public:
    virtual std::future<std::pair<bool, std::string>> Get(const std::string& host, const uint16_t port,
        const std::string& url, const std::map<std::string, std::string>* url_params = nullptr,
        const std::vector<std::string>* cookies = nullptr, int ioTimeout = 20, int connectTimeout = 5) = 0;

    virtual std::future<std::pair<bool, std::string>> Post(const std::string& host, const uint16_t port,
        const std::string& url, const std::map<std::string, std::string>* url_params = nullptr,
        const std::string* http_body = nullptr, const std::string* content_type = nullptr,
        const std::vector<std::string>* cookies = nullptr, int ioTimeout = 20, int connectTimeout = 5) = 0;

    /**
     * Post an HTTP request
     * @param cb will be called in the same thread where AFIModule::Update is called
     * @param host IP or Domain name
     * @param port
     * @param url /v3/kv/put for example
     * @param url_params will be appended to url as `?k1=v1&k2=v2&k3=v3...`
     * @param http_body
     * @param content_type
     * @param cookies
     * @param ioTimeout in seconds
     * @param connectTimeout in seconds
     */
    virtual void Post(std::function<void(std::pair<bool, std::string>)> cb, const std::string& host, const uint16_t port,
        const std::string& url, const std::map<std::string, std::string>* url_params = nullptr,
        const std::string* http_body = nullptr, const std::string* content_type = nullptr,
        const std::vector<std::string>* cookies = nullptr, int ioTimeout = 20, int connectTimeout = 5) = 0;

    /**
     * Post an HTTP request
     * @param cb will be called in the same thread where AFIModule::Update is called
     * @param host IP or Domain name
     * @param service service name or port
     * @param url /v3/kv/put for example
     * @param url_params will be appended to url as `?k1=v1&k2=v2&k3=v3...`
     * @param http_body
     * @param content_type
     * @param cookies
     * @param ioTimeout in seconds
     * @param connectTimeout in seconds
     */
    virtual void Post(std::function<void(std::pair<bool, std::string>)> cb, const std::string& host,
        const std::string& service, const std::string& url, const std::map<std::string, std::string>* url_params = nullptr,
        const std::string* http_body = nullptr, const std::string* content_type = nullptr,
        const std::vector<std::string>* cookies = nullptr, int ioTimeout = 20, int connectTimeout = 5) = 0;
};

} // namespace ark
