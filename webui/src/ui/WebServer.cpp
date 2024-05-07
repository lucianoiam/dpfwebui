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

#include <cstring>

#include "WebServer.hpp"

// Keep this include after WebServer.hpp to avoid warning from MinGW gcc:
// "Please include winsock2.h before windows.h"
#include "extra/Path.hpp"

#define LWS_PROTOCOL_NAME "lws-dpf"

USE_NAMESPACE_DISTRHO

WebServer::WebServer()
    : fContext(nullptr)
    , fHandler(nullptr)
{}

// JS injection feature currently not in use, leaving code just in case.
void WebServer::init(int port, WebServerHandler* handler, const char* jsInjectTarget,
                        const char* jsInjectToken)
{
    fHandler = handler;

    lws_set_log_level(LLL_ERR|LLL_WARN/*|LLL_DEBUG*/, 0);

    std::memset(fProtocols, 0, sizeof(fProtocols));
    fProtocols[0].name = LWS_PROTOCOL_NAME;
    fProtocols[0].callback = WebServer::lwsCallback;

    std::memset(fExtensions, 0, sizeof(fExtensions));
    fExtensions[0].name = "permessage-deflate";
    fExtensions[0].callback = lws_extension_callback_pm_deflate;
    fExtensions[0].client_offer = "permessage-deflate"
                                  "; client_no_context_takeover"
                                  "; client_max_window_bits";

    std::strcpy(fMountOrigin, Path::getPluginLibrary() + "/ui/");

    std::memset(&fMount, 0, sizeof(fMount));
    fMount.mountpoint       = "/";
    fMount.mountpoint_len   = std::strlen(fMount.mountpoint);
    fMount.origin           = fMountOrigin;
    fMount.origin_protocol  = LWSMPRO_FILE;
    fMount.def              = "index.html";

    if ((jsInjectTarget != nullptr) && (jsInjectToken != nullptr)) {
        fInjectToken = jsInjectToken;
        std::memset(&fMountOptions, 0, sizeof(fMountOptions));
        fMountOptions.name  = jsInjectTarget;
        fMountOptions.value = LWS_PROTOCOL_NAME;
        fMount.interpret    = &fMountOptions;
    }
#ifndef NDEBUG
    // Send caching headers
    fMount.cache_max_age    = 3600;
    fMount.cache_reusable   = 1;
    fMount.cache_revalidate = 1;
#endif

    std::memset(&fContextInfo, 0, sizeof(fContextInfo));
    fContextInfo.port       = port;
    fContextInfo.protocols  = fProtocols;
    //fContextInfo.extensions = fExtensions;
    fContextInfo.mounts     = &fMount;
    fContextInfo.uid        = -1;
    fContextInfo.gid        = -1;
    fContextInfo.user       = this;

#if defined(DPF_WEBUI_NETWORK_SSL)
    // SSL (WIP)
    // https://github.com/warmcat/libwebsockets/blob/main/READMEs/README.test-apps.md
    // cp -rp ./scripts/client-ca /tmp
    // cd /tmp/client-ca
    // ./create-ca.sh
    // ./create-server-cert.sh server
    // ./create-client-cert.sh client
    //fContextInfo.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    //fContextInfo.ssl_cert_filepath        = "/tmp/client-ca/server.pem";
    //fContextInfo.ssl_private_key_filepath = "/tmp/client-ca/server.key";
    //fContextInfo.ssl_ca_filepath          = "/tmp/client-ca/ca.pem";
#endif

    fContext = lws_create_context(&fContextInfo);
}

WebServer::~WebServer()
{
    if (fContext != nullptr) {
        lws_context_destroy(fContext);
        fContext = nullptr;
    }
}

void WebServer::injectScript(const String& script)
{
    fInjectedScripts.push_back(script);
}

void WebServer::send(const uint8_t* data, size_t size, Client client, bool binary)
{
    ClientContextMap::iterator it = fClients.find(client);
    if (it == fClients.end()) {
        return;
    }

    ClientContext::FrameData frame(binary);
    frame.data.insert(frame.data.end(), data, data + size);

    const MutexLocker writeBufferScopedLock(fMutex);
    it->second.writeBuffer.push_back(frame);

    lws_callback_on_writable(client);
}

void WebServer::send(const char* data, Client client)
{
    send(reinterpret_cast<const uint8_t*>(data), std::strlen(data), client, /*binary*/false);
}

void WebServer::broadcast(const uint8_t* data, size_t size, Client exclude, bool binary)
{
    for (ClientContextMap::iterator it = fClients.begin(); it != fClients.end(); ++it) {
        if (it->first != exclude) {
            send(data, size, it->first, binary);
        }
    }
}

void WebServer::broadcast(const char* data, Client exclude)
{
    broadcast(reinterpret_cast<const uint8_t*>(data), std::strlen(data), exclude, /*binary*/false);
}

void WebServer::serve(bool block)
{
    // Avoid blocking on some platforms by passing timeout=-1
    // https://github.com/warmcat/libwebsockets/issues/1735
    lws_service(fContext, block ? 0 : -1);
}

void WebServer::cancel()
{
    lws_cancel_service(fContext);
}

Client WebServer::getClientByUserAgentComponent(String& userAgentComponent)
{
    for (ClientContextMap::iterator it = fClients.begin(); it != fClients.end(); ++it) {
        if (it->second.userAgent.contains(userAgentComponent)) {
            return it->first;
        }
    }

    return nullptr;
}

void WebServer::setClientUserAgent(Client client, String& userAgent)
{
    ClientContextMap::iterator it = fClients.find(client);

    if (it != fClients.end()) {
        it->second.userAgent = userAgent;
    }
}

int WebServer::lwsCallback(struct lws* wsi, enum lws_callback_reasons reason,
                           void* user, void* in, size_t len)
{
    void* userdata = lws_context_user(lws_get_context(wsi));
    WebServer* server = static_cast<WebServer*>(userdata);
    int rc = 0; /* 0 OK, close connection otherwise */
    
    switch (reason) {
        case LWS_CALLBACK_PROCESS_HTML: {
            lws_process_html_args* args = static_cast<lws_process_html_args*>(in);
            rc = server->injectScripts(args);
            break;
        }
        case LWS_CALLBACK_ESTABLISHED: {
            char userAgent[1024];
            if (lws_hdr_copy(wsi, userAgent, sizeof(userAgent), WSI_TOKEN_HTTP_USER_AGENT) < 0) {
                userAgent[0] = '\0';   
            }
            ClientContext ctx;
            ctx.userAgent = userAgent;
            server->fClients.emplace(wsi, ctx);
            server->fHandler->handleWebServerConnect(wsi);
            break;
        }
        case LWS_CALLBACK_CLOSED:
            server->fClients.erase(server->fClients.find(wsi));
            server->fHandler->handleWebServerDisconnect(wsi);
            break;
        case LWS_CALLBACK_RECEIVE:
            rc = server->handleRead(wsi, in, len, lws_frame_is_binary(wsi));
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE: {
            rc = server->handleWrite(wsi);
            break;
        }
        default:
            rc = lws_callback_http_dummy(wsi, reason, user, in, len);
            break;
    }

    return rc;
}

const char* WebServer::lwsReplaceFunc(void* data, int index)
{
    switch (index) {
        case 0:
            return static_cast<const char*>(data);
        default:
            return "";
    }
}

int WebServer::injectScripts(lws_process_html_args* args)
{
    lws_process_html_state phs;
    std::memset(&phs, 0, sizeof(phs));
    
    if (fInjectedScripts.size() == 0) {
        return lws_chunked_html_process(args, &phs) ? -1 : 0;
    }

    const char* vars[1] = {fInjectToken};
    phs.vars = vars;
    phs.count_vars = 1;
    phs.replace = WebServer::lwsReplaceFunc;

    size_t len = 0;
    typedef StringList::const_iterator Iterator;

    for (Iterator it = fInjectedScripts.cbegin(); it != fInjectedScripts.cend(); ++it) {
        len += it->length();
    }

    len += strlen(fInjectToken) + 2;
    char* js = new char[len + 1];
    std::strcat(js, fInjectToken);
    std::strcat(js, ";\n");

    for (Iterator it = fInjectedScripts.cbegin(); it != fInjectedScripts.cend(); ++it) {
        std::strcat(js, *it);
    }

    phs.data = js;

    int rc = lws_chunked_html_process(args, &phs) ? -1 : 0;
    delete[] js;

    return rc;
}

int WebServer::handleRead(Client client, void* in, size_t len, bool binary)
{
    int rc = 0;

    ByteVector& rb = fClients[client].readBuffer;
    rb.insert(rb.end(), static_cast<uint8_t*>(in), static_cast<uint8_t*>(in) + len);

    if (lws_remaining_packet_payload(client) != 0) {
        return rc;
    }

    if (binary) {
        rc = fHandler->handleWebServerRead(client, rb);
    } else {
        rb.push_back('\0');
        rc = fHandler->handleWebServerRead(client, reinterpret_cast<const char*>(rb.data()));
    }

    rb.clear();

    return rc;
}

int WebServer::handleWrite(Client client)
{
    const MutexLocker writeBufferScopedLock(fMutex);

    // Exactly one lws_write() call per LWS_CALLBACK_SERVER_WRITEABLE callback
    ClientContext::ByteVectorList& wb = fClients[client].writeBuffer;
    if (wb.empty()) {
        return 0;
    }

    ClientContext::FrameData frame = wb.front();
    wb.pop_front();

    size_t dataSize = frame.data.size() - LWS_PRE;
    size_t writeSize = lws_write(client, static_cast<unsigned char*>(frame.data.data() + LWS_PRE),
                                 dataSize, frame.binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT);
    if (! wb.empty()) {
        lws_callback_on_writable(client);
    }

    return writeSize == dataSize ? 0 : -1;
}
