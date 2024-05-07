/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <cerrno>
#include <cstring>
#include <utility>
#include <unistd.h>

#include "src/DistrhoDefines.h"
#include "DistrhoPluginInfo.h"

#if defined(DISTRHO_OS_WINDOWS)
# include <Winsock2.h>
# define IS_EADDRINUSE() (WSAGetLastError() == WSAEADDRINUSE) // errno==0
# define CLOSE_SOCKET(s) closesocket(s)
#else
# include <arpa/inet.h>
# include <sys/socket.h>
# define IS_EADDRINUSE() (errno == EADDRINUSE)
# define CLOSE_SOCKET(s) ::close(s)
#endif

#include "NetworkUI.hpp"

#define LOG_TAG "NetworkUI"
#if defined(DPF_WEBUI_NETWORK_SSL)
# define TRANSFER_PROTOCOL "https"
#else
# define TRANSFER_PROTOCOL "http"
#endif
#define FIRST_PORT 49152 // first in dynamic/private range

USE_NAMESPACE_DISTRHO

NetworkUI::NetworkUI(uint widthCssPx, uint heightCssPx, float initPixelRatio)
    : WebUIBase(widthCssPx, heightCssPx, initPixelRatio
#if DPF_WEBUI_PROTOCOL_BINARY
        , /*FunctionArgumentSerializer*/[](const char* f) { return djb2hash(f); }
#endif
    )
    , fServerInit(false)
    , fPort(-1)
    , fThread(nullptr)
#if DPF_WEBUI_ZEROCONF
    , fZeroconfPublish(false)
#endif
    , fParameterLock(false)
{
    if (isDryRun()) {
        return;
    }

#if defined(DISTRHO_OS_WINDOWS)
    WSADATA wsaData;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        d_stderr2(LOG_TAG " : failed WSAStartup()");
        return;
    }
#endif

    setBuiltInFunctionHandlers();

    if ((! DISTRHO_PLUGIN_WANT_STATE) || isStandalone()) {
        // Port is not remembered when state support is disabled
        fPort = findAvailablePort();
        if (fPort != -1) {
            initServer();
        }
    }
}

NetworkUI::~NetworkUI()
{
    if (fThread != nullptr) {
        delete fThread;
        fThread = nullptr;
    }
#if defined(DISTRHO_OS_WINDOWS)
    //WSACleanup();
#endif
}

String NetworkUI::getLocalUrl()
{
    return String(TRANSFER_PROTOCOL "://127.0.0.1:") + String(fPort);
}

String NetworkUI::getPublicUrl()
{
    String url = getLocalUrl();

    const int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        d_stderr(LOG_TAG " : failed socket(), errno %d", errno);
        return url;
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("8.8.8.8"); // Google DNS
    addr.sin_port = htons(53);

    if (connect(sockfd, (const sockaddr*)&addr, sizeof(addr)) == 0) {
        socklen_t addrlen = sizeof(addr);

        if (getsockname(sockfd, (sockaddr*)&addr, &addrlen) == 0) {
            const char* ip = inet_ntoa(addr.sin_addr);
            url = String(TRANSFER_PROTOCOL "://") + ip + ":" + String(fPort);
        } else {
            d_stderr(LOG_TAG " : failed getsockname(), errno %d", errno);
        }
    } else {
        //d_stderr(LOG_TAG " : failed connect(), errno %d", errno);
    }

    if (CLOSE_SOCKET(sockfd) == -1) {
        d_stderr(LOG_TAG " : failed close(), errno %d", errno);
    }

    return url;
}

void NetworkUI::setState(const char* key, const char* value)
{
    // Warning : UI::setState() is non-virtual !
    WebUIBase::setState(key, value);
    fStates[key] = value;
}

void NetworkUI::postMessage(const Variant& payload, uintptr_t destination, uintptr_t exclude)
{
#if DPF_WEBUI_PROTOCOL_BINARY
    BinaryData data = payload.toBSON();
    if (destination == kDestinationAll) {
        if (exclude == kDestinationWebView) {
            String userAgent(kWebViewUserAgent);
            Client excClient = fServer.getClientByUserAgentComponent(userAgent);
            fServer.broadcast(data.data(), data.size(), excClient);
        } else {
            fServer.broadcast(data.data(), data.size());
        }
    } else if (destination == kDestinationWebView) {
        String userAgent(kWebViewUserAgent);
        Client client = fServer.getClientByUserAgentComponent(userAgent);
        if (client != nullptr) {
            fServer.send(data.data(), data.size(), client);
        }
    } else {
        fServer.send(data.data(), data.size(), reinterpret_cast<Client>(destination));
    }
#else
    if (destination == kDestinationAll) {
        if (exclude == kDestinationWebView) {
            String userAgent(kWebViewUserAgent);
            Client excClient = fServer.getClientByUserAgentComponent(userAgent);
            fServer.broadcast(payload.toJSON(), excClient);
        } else {
            fServer.broadcast(payload.toJSON());
        }
    } else if (destination == kDestinationWebView) {
        String userAgent(kWebViewUserAgent);
        Client client = fServer.getClientByUserAgentComponent(userAgent);
        if (client != nullptr) {
            fServer.send(payload.toJSON(), client);
        }
    } else {
        fServer.send(payload.toJSON(), reinterpret_cast<Client>(destination));
    }
#endif
}

void NetworkUI::parameterChanged(uint32_t index, float value)
{
    fParameters[index] = value;

    if (fParameterLock) {
        fParameterLock = false;
    } else {
        WebUIBase::parameterChanged(index, value);
    }
}

#if DISTRHO_PLUGIN_WANT_STATE
void NetworkUI::stateChanged(const char* key, const char* value)
{
    if (isDryRun()) {
        return;
    }

    if ((std::strcmp(key, "_ws_port") == 0) && ! fServerInit) {
        fPort = std::atoi(value);
        if (fPort == -1) {
            fPort = findAvailablePort();
            setState("_ws_port", std::to_string(fPort).c_str());
        } else {
            //d_stderr(LOG_TAG " : reusing port %d", fPort);
        }
        if (fPort != -1) {
            initServer();
        }
        return;
    }

# if DPF_WEBUI_ZEROCONF
    if (std::strcmp(key, "_zc_publish") == 0) {
        bool publish = std::strcmp(value, "true") == 0;
        if (fZeroconfPublish != publish) {
            fZeroconfPublish = publish;
            zeroconfStateUpdated();
        }
        return;
    } else if (std::strcmp(key, "_zc_id") == 0) {
        if (fZeroconfId != value) {
            fZeroconfId = value;
            zeroconfStateUpdated();
        }
        return;
    } else if (std::strcmp(key, "_zc_name") == 0) {
        if (fZeroconfName != value) {
            fZeroconfName = value;
            zeroconfStateUpdated();
        }
        return;
    }
# endif

    fStates[key] = value;

    WebUIBase::stateChanged(key, value);
}
#endif

void NetworkUI::onClientConnected(Client client)
{
    (void)client;
}

void NetworkUI::setBuiltInFunctionHandlers()
{
    // Broadcast parameter updates to all clients except the originating one
    const FunctionHandler& parameterHandlerSuper = getFunctionHandler("setParameterValue");
    setFunctionHandler("setParameterValue", 2, [this, parameterHandlerSuper](const Variant& args, uintptr_t origin) {
        queue([this, parameterHandlerSuper, args, origin] {
            const uint32_t index = static_cast<uint32_t>(args[0].getNumber());
            const float value = static_cast<float>(args[1].getNumber());
            fParameters[index] = value;
            fParameterLock = true; // avoid echo
            parameterHandlerSuper(args, origin);
        });

        callback("parameterChanged", args, kDestinationAll, /*exclude*/origin);
    });

#if DISTRHO_PLUGIN_WANT_STATE
    // Broadcast state updates to all clients except the originating one
    const FunctionHandler& stateHandlerSuper = getFunctionHandler("setState");
    setFunctionHandler("setState", 2, [this, stateHandlerSuper](const Variant& args, uintptr_t origin) {
        queue([this, stateHandlerSuper, args, origin] {
            const String key = args[0].getString();
            const String value = args[1].getString();
            fStates[key.buffer()] = value.buffer();
            stateHandlerSuper(args, origin);
        });

        callback("stateChanged", args, kDestinationAll, /*exclude*/origin);
    });
#endif

    // Custom method for exchanging UI-only messages between clients
    setFunctionHandler("broadcast", 1, [this](const Variant& args, uintptr_t origin) {
        callback("messageReceived", args, kDestinationAll, /*exclude*/origin);
    });

#if DPF_WEBUI_ZEROCONF
    setFunctionHandler("isZeroconfPublished", 0, [this](const Variant&, uintptr_t origin) {
        callback("isZeroconfPublished", { fZeroconf.isPublished() }, origin);
    });

    setFunctionHandler("setZeroconfPublished", 1, [this](const Variant& args, uintptr_t) {
        fZeroconfPublish = args[0].getBoolean();
        setState("_zc_published", fZeroconfPublish ? "true" : "false");
        zeroconfStateUpdated();
    });

    setFunctionHandler("getZeroconfId", 0, [this](const Variant&, uintptr_t origin) {
        callback("getZeroconfId", { fZeroconfId }, origin);
    });

    setFunctionHandler("getZeroconfName", 0, [this](const Variant&, uintptr_t origin) {
        callback("getZeroconfName", { fZeroconfName }, origin);
    });

    setFunctionHandler("setZeroconfName", 1, [this](const Variant& args, uintptr_t) {
        fZeroconfName = args[0].getString();
        setState("_zc_name", fZeroconfName);
        zeroconfStateUpdated();
    });
#else
    setFunctionHandler("isZeroconfPublished", 0, [this](const Variant&, uintptr_t origin) {
        callback("isZeroconfPublished", { false }, origin);
    });

    setFunctionHandler("getZeroconfName", 0, [this](const Variant&, uintptr_t origin) {
        callback("getZeroconfName", { "" }, origin);
    });
#endif

    setFunctionHandler("getPublicUrl", 0, [this](const Variant&, uintptr_t origin) {
        callback("getPublicUrl", { getPublicUrl() }, origin);
    });

    setFunctionHandler("ping", 0, [this](const Variant&, uintptr_t origin) {
        callback("pong", Variant::createArray(), origin);
    });
}

void NetworkUI::initServer()
{
    fServerInit = true;
    fServer.init(fPort, this);
    fThread = new WebServerThread(&fServer);
    d_stderr(LOG_TAG " : server up @ %s", getPublicUrl().buffer());
}

int NetworkUI::findAvailablePort()
{
    // Ports are not reused during process lifetime unless SO_REUSEADDR is set.
    // Once a plugin binds to a port it is safe to assume that other plugin
    // instances will not claim the same port. A single plugin can reuse a port
    // by enabling DISTRHO_PLUGIN_WANT_STATE so it can be stored in a DPF state.

    const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        d_stderr(LOG_TAG " : failed socket(), errno %d", errno);
        return -1;
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    int port = -1, i = FIRST_PORT;

    while ((port == -1) && (i < 65535)) {
        addr.sin_port = htons(i);

        if (bind(sockfd, (const sockaddr*)&addr, sizeof(addr)) == 0) {
            port = i;
        } else {
            if (IS_EADDRINUSE()) {
                i++;
            } else {
                d_stderr(LOG_TAG " : failed bind(), errno %d", errno);
                break;
            }
        }
    }

    if (CLOSE_SOCKET(sockfd) == -1) {
        d_stderr(LOG_TAG " : failed close(), errno %d", errno);
    }

    if (port == -1) {
        d_stderr2(LOG_TAG " : could not find available port");
    } else {
        //d_stderr(LOG_TAG " : found available port %d", port);
    }

    return port;
}

#if DPF_WEBUI_ZEROCONF
void NetworkUI::zeroconfStateUpdated()
{
    if (fZeroconfPublish && ! fZeroconfId.isEmpty() && ! fZeroconfName.isEmpty()) {
        fZeroconf.publish(fZeroconfName, "_http._tcp", fPort, {
            { "dpfuri", DISTRHO_PLUGIN_URI },
            { "instanceid", fZeroconfId } // ID is useless unless plugin state is made persistent
        });
    } else {
        fZeroconf.unpublish();
    }
}
#endif

void NetworkUI::handleWebServerConnect(Client client)
{
    queue([this, client] {
        // Send all current parameters and states
        for (ParameterMap::const_iterator it = fParameters.cbegin(); it != fParameters.cend(); ++it) {
            const Variant args = { it->first, it->second };
            callback("parameterChanged", args, reinterpret_cast<uintptr_t>(client));
        }

        for (StateMap::const_iterator it = fStates.cbegin(); it != fStates.cend(); ++it) {
            const Variant args = { it->first.c_str(), it->second.c_str() };
            callback("stateChanged", args, reinterpret_cast<uintptr_t>(client));
        }

        onClientConnected(client);
    });
}

int NetworkUI::handleWebServerRead(Client client, const ByteVector& data)
{
#if DPF_WEBUI_PROTOCOL_BINARY
    handleMessage(Variant::fromBSON(data, /*asArray*/true), reinterpret_cast<uintptr_t>(client));
#else
    (void)client;
    (void)data;
#endif
    return 0;
}

int NetworkUI::handleWebServerRead(Client client, const char* data)
{
#if ! DPF_WEBUI_PROTOCOL_BINARY
    handleMessage(Variant::fromJSON(data), reinterpret_cast<uintptr_t>(client));
#else
    (void)client;
    (void)data;
#endif
    return 0;
}

int32_t NetworkUI::djb2hash(const char* str)
{
    int32_t h = 5381;
    int32_t c;

    while ((c = *str++)) {
         h = h * 33 ^ c;
    }

    return h;
}

WebServerThread::WebServerThread(WebServer* server) noexcept
    : fServer(server)
    , fRun(true)
{
    startThread();
}

WebServerThread::~WebServerThread() noexcept
{
    fRun = false;
    fServer->cancel();
    stopThread(-1 /*wait forever*/);
}

void WebServerThread::run() noexcept
{
    while (fRun) {
#if defined(DISTRHO_OS_WINDOWS)
        // Telling serve() to block creates lags during new connections setup
        fServer->serve(false);
        Sleep(1);
#else
        fServer->serve(true);
#endif
    }
}
