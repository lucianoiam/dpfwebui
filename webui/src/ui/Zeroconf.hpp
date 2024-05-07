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

#ifndef ZEROCONF_HPP
#define ZEROCONF_HPP

#include <vector>
#if DISTRHO_OS_LINUX
# include <cstdio>
# include <signal.h>
# include <spawn.h>
#elif DISTRHO_OS_MAC
# include <dns_sd.h>
# include <arpa/inet.h>
#elif DISTRHO_OS_WINDOWS
# include <codecvt>
# include <cstring>
# include <locale>
# include <libloaderapi.h>
# include <windns.h>
# include <winerror.h>
#endif

#include "src/DistrhoDefines.h"

#if DISTRHO_OS_LINUX
extern char **environ;
#endif

START_NAMESPACE_DISTRHO

#if DISTRHO_OS_WINDOWS
# define LOAD_DNSAPI_DLL() LoadLibrary("dnsapi.dll")
class Zeroconf;
struct DnsApiHelper
{
    Zeroconf* weakThis;
    DNS_SERVICE_INSTANCE* instance;
};
#endif
struct KeyValuePair
{
    const char *key;
    const char *value;
};

typedef std::vector<KeyValuePair> KeyValuePairsVector;

class Zeroconf
{
public:
    Zeroconf()
        : fPublished(false)
#if DISTRHO_OS_LINUX
        , fPid(0)
#elif DISTRHO_OS_MAC
        , fService(nullptr)
#elif DISTRHO_OS_WINDOWS
        , fHelper(nullptr)
#endif
    {}

    ~Zeroconf()
    {
        unpublish();
    }

    bool isPublished() const noexcept
    {
        return fPublished;
    }

    void publish(const char* name, const char* type, int port, const KeyValuePairsVector txtData) noexcept
    {
        unpublish();

#if DISTRHO_OS_LINUX
        typedef std::vector<const char*> CStringVector;

        const char* const bin = "avahi-publish";
        const std::string sport = std::to_string(port);
        CStringVector argv = { bin, "-s", name, type, sport.c_str() };
        const size_t offset = argv.size(); // for freeing below

        for (KeyValuePairsVector::const_iterator it = txtData.cbegin(); it != txtData.cend(); ++it) {
            char *buf = new char[255];
            snprintf(buf, 255, "%s=%s", it->key, it->value);
            argv.push_back(buf);
        }

        argv.push_back(nullptr);

        const int status = posix_spawnp(&fPid, bin, nullptr/*file_actions*/, nullptr/*attrp*/,
                                        const_cast<char* const*>(argv.data()), environ);
        if (status == 0) {
            fPublished = true;
        } else {
            d_stderr2("Zeroconf : failed publish()");
        }

        for (CStringVector::const_iterator it = argv.cbegin() + offset ; it != argv.cend(); ++it) {
            delete *it;
        }
#elif DISTRHO_OS_MAC
        TXTRecordRef txtRecord;
        TXTRecordCreate(&txtRecord, 0/*bufferLen*/, nullptr/*buffer*/);
        for (KeyValuePairsVector::const_iterator it = txtData.cbegin(); it != txtData.cend(); ++it) {
            TXTRecordSetValue(&txtRecord, it->key, strlen(it->value), it->value);
        }

        DNSServiceErrorType err = DNSServiceRegister(&fService, 0/*flags*/,
            kDNSServiceInterfaceIndexAny, name, type, nullptr/*domain*/, nullptr/*host*/,
            htons(port), TXTRecordGetLength(&txtRecord), TXTRecordGetBytesPtr(&txtRecord),
            nullptr/*callBack*/, nullptr/*context*/);
        if (err == kDNSServiceErr_NoError) {
            fPublished = true;
        } else {
            d_stderr2("Zeroconf : failed publish()");
        }

        TXTRecordDeallocate(&txtRecord);
#elif DISTRHO_OS_WINDOWS
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
# endif
        HMODULE dnsapi = LOAD_DNSAPI_DLL();
        if (dnsapi == nullptr) {
            return;
        }

        typedef PDNS_SERVICE_INSTANCE (*PFN_DnsServiceConstructInstance)(PCWSTR pServiceName, PCWSTR pHostName,
                PIP4_ADDRESS pIp4, PIP6_ADDRESS pIp6, WORD wPort, WORD wPriority, WORD wWeight, DWORD dwPropertiesCount,
                PCWSTR *keys, PCWSTR *values);
        const PFN_DnsServiceConstructInstance pDnsServiceConstructInstance =
            reinterpret_cast<PFN_DnsServiceConstructInstance>(GetProcAddress(dnsapi, "DnsServiceConstructInstance"));      

        typedef DWORD (*PFN_DnsServiceRegister)(PDNS_SERVICE_REGISTER_REQUEST pRequest, PDNS_SERVICE_CANCEL pCancel);
        const PFN_DnsServiceRegister pDnsServiceRegister =
            reinterpret_cast<PFN_DnsServiceRegister>(GetProcAddress(dnsapi, "DnsServiceRegister"));

        if ((pDnsServiceConstructInstance == nullptr) || (pDnsServiceRegister == nullptr)) {
            FreeLibrary(dnsapi);
            return;
        }

        char hostname[128];
        DWORD size = sizeof(hostname);
        GetComputerNameEx(ComputerNameDnsHostname, hostname, &size);
        std::strcat(hostname, ".local");

        char service[128];
        std::strcpy(service, name);
        std::strcat(service, ".");
        std::strcat(service, type);
        std::strcat(service, ".local");

        typedef std::vector<PCWSTR> CWideStringVector;
        CWideStringVector keys, values;
        for (KeyValuePairsVector::const_iterator it = txtData.cbegin(); it != txtData.cend(); ++it) {
            PWSTR key = new WCHAR[255], value = new WCHAR[255];
            mbstowcs(key, it->key, 255);
            mbstowcs(value, it->value, 255);
            keys.push_back(key);
            values.push_back(value);
        }

        const size_t propCount = keys.size();
        std::wstring_convert<std::codecvt_utf8<wchar_t>> wconv;

        // Helper outlives Zeroconf instance because the DNS API is asynchronous
        fHelper = new DnsApiHelper();
        fHelper->weakThis = this;
        fHelper->instance = pDnsServiceConstructInstance(wconv.from_bytes(service).c_str(),
                wconv.from_bytes(hostname).c_str(), nullptr, nullptr, static_cast<WORD>(port),
                0, 0, propCount, keys.data(), values.data());

        for (size_t i = 0; i < propCount; ++i) {
            delete keys[i];
            delete values[i];
        }

        if (fHelper->instance != nullptr) {
            std::memset(&fCancel, 0, sizeof(fCancel));
            std::memset(&fRequest, 0, sizeof(fRequest));
            fRequest.Version = DNS_QUERY_REQUEST_VERSION1;
            fRequest.pServiceInstance = fHelper->instance;
            fRequest.pRegisterCompletionCallback = dnsServiceRegisterComplete;
            fRequest.pQueryContext = fHelper;
            pDnsServiceRegister(&fRequest, &fCancel);
        }

        FreeLibrary(dnsapi);
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic pop
# endif
#endif
    }

    void unpublish() noexcept
    {
#if DISTRHO_OS_LINUX
        if (fPid != 0) {
            kill(fPid, SIGTERM);
            int stat;
            waitpid(fPid, &stat, 0);
            fPid = 0;
        }
#elif DISTRHO_OS_MAC
        if (fService != nullptr) {
            DNSServiceRefDeallocate(fService);
            fService = nullptr;
        }
#elif DISTRHO_OS_WINDOWS
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
# endif
        HMODULE dnsapi = LOAD_DNSAPI_DLL();
        if (dnsapi == nullptr) {
            return;
        }

        if (fHelper != nullptr) {
            if (fPublished) {
                // Avoid callback to deleted this, helper instance is deleted later.
                fHelper->weakThis = nullptr;

                typedef (*PFN_DnsServiceDeRegister)(PDNS_SERVICE_REGISTER_REQUEST pRequest, PDNS_SERVICE_CANCEL pCancel);
                const PFN_DnsServiceDeRegister pDnsServiceDeRegister =
                    reinterpret_cast<PFN_DnsServiceDeRegister>(GetProcAddress(dnsapi, "DnsServiceDeRegister"));
                pDnsServiceDeRegister(&fRequest, nullptr);
            } else {
                typedef (*PFN_DnsServiceRegisterCancel)(PDNS_SERVICE_CANCEL pCancelHandle);
                const PFN_DnsServiceRegisterCancel pDnsServiceRegisterCancel =
                    reinterpret_cast<PFN_DnsServiceRegisterCancel>(GetProcAddress(dnsapi, "DnsServiceRegisterCancel"));
                pDnsServiceRegisterCancel(&fCancel);

                typedef (*PFN_DnsServiceFreeInstance)(PDNS_SERVICE_INSTANCE pInstance);
                const PFN_DnsServiceFreeInstance pDnsServiceFreeInstance =
                    reinterpret_cast<PFN_DnsServiceFreeInstance>(GetProcAddress(dnsapi, "DnsServiceFreeInstance"));
                pDnsServiceFreeInstance(fHelper->instance);

                delete fHelper;
            }

            fHelper = nullptr;
        }

        FreeLibrary(dnsapi);
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic pop
# endif
#endif
    }

#if DISTRHO_OS_WINDOWS
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
# endif
    static void dnsServiceRegisterComplete(DWORD status, PVOID pQueryContext, PDNS_SERVICE_INSTANCE /*pInstance*/)
    {
        DnsApiHelper* helper = static_cast<DnsApiHelper*>(pQueryContext);

        if (helper->weakThis == nullptr) {
            HMODULE dnsapi = LOAD_DNSAPI_DLL();
            
            typedef (*PFN_DnsServiceFreeInstance)(PDNS_SERVICE_INSTANCE pInstance);
            const PFN_DnsServiceFreeInstance pDnsServiceFreeInstance =
                reinterpret_cast<PFN_DnsServiceFreeInstance>(GetProcAddress(dnsapi, "DnsServiceFreeInstance"));
            pDnsServiceFreeInstance(helper->instance);
            
            FreeLibrary(dnsapi);
            delete helper;

            return;
        }

        if (status == ERROR_SUCCESS) {
            helper->weakThis->fPublished = true;
            helper->weakThis = nullptr;
        } else {
            d_stderr2("Zeroconf : failed publish()");
        }
    }
# if defined(__GNUC__) && (__GNUC__ >= 9)
#  pragma GCC diagnostic pop
# endif
#endif

private:
    bool fPublished;
#if DISTRHO_OS_LINUX
    pid_t fPid;
#elif DISTRHO_OS_MAC
    DNSServiceRef fService;
#elif DISTRHO_OS_WINDOWS
    DnsApiHelper* fHelper;
    DNS_SERVICE_REGISTER_REQUEST fRequest;
    DNS_SERVICE_CANCEL fCancel;
#endif

};

END_NAMESPACE_DISTRHO

#endif // ZEROCONF_HPP
