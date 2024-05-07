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

#ifndef NETWORK_UI_HPP
#define NETWORK_UI_HPP

#include <string>
#include <unordered_map>

#include "distrho/extra/Thread.hpp"

#include "WebUIBase.hpp"
#include "WebServer.hpp"
#include "Variant.hpp"
#if DPF_WEBUI_ZEROCONF
# include "Zeroconf.hpp"
#endif

START_NAMESPACE_DISTRHO

class WebServerThread;

class NetworkUI : public WebUIBase, public WebServerHandler
{
public:
    NetworkUI(uint widthCssPx, uint heightCssPx, float initPixelRatio);
    virtual ~NetworkUI();

    WebServer& getServer() { return fServer; }

    String getLocalUrl();
    String getPublicUrl();

protected:
    void setState(const char* key, const char* value);

    void postMessage(const Variant& payload, uintptr_t destination, uintptr_t exclude) override;

    void parameterChanged(uint32_t index, float value) override;
#if DISTRHO_PLUGIN_WANT_STATE
    void stateChanged(const char* key, const char* value) override;
#endif

    virtual void onClientConnected(Client client);

private:
    void setBuiltInFunctionHandlers();
    void initServer();
    int  findAvailablePort();
#if DPF_WEBUI_ZEROCONF
    void zeroconfStateUpdated();
#endif

    void handleWebServerConnect(Client client) override;
    int  handleWebServerRead(Client client, const ByteVector& data) override;
    int  handleWebServerRead(Client client, const char* data) override;

    static int32_t djb2hash(const char *str);

    bool             fServerInit;
    int              fPort;
    WebServer        fServer;
    WebServerThread* fThread;
#if DPF_WEBUI_ZEROCONF
    Zeroconf fZeroconf;
    bool     fZeroconfPublish;
    String   fZeroconfId;
    String   fZeroconfName;
#endif
    typedef std::unordered_map<uint32_t, float> ParameterMap;
    ParameterMap fParameters;
    bool         fParameterLock;
    typedef std::unordered_map<std::string, std::string> StateMap;
    StateMap     fStates;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetworkUI)

};

class WebServerThread : public Thread
{
public:
    WebServerThread(WebServer* server) noexcept;
    virtual ~WebServerThread() noexcept;

    void run() noexcept override;

private:
    WebServer* fServer;
    bool       fRun;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebServerThread)
};

END_NAMESPACE_DISTRHO

#endif  // NETWORK_UI_HPP
