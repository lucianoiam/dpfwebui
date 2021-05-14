/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
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

#include "WebView.h"

/*
	The "right" way to work with WebView2 requires WIL which is provided by the NuGet package
	Microsoft.Windows.ImplementationLibrary
	But WIL is not compatible with MinGW, see https://github.com/microsoft/wil/issues/117
	Using this solution instead https://github.com/jchv/webview2-in-mingw
*/

#define CINTERFACE
#include "WebView2.h"
#include "event.h"
#include <tchar.h>
#include <locale>
#include <string>
#ifndef UNICODE
#include <codecvt>
#endif

#include <sstream>

ICoreWebView2* webview2 = nullptr;
ICoreWebView2Controller* controller = nullptr;

std::wstring TStrToWStr(LPCTSTR str)
{
#ifdef UNICODE
    return std::wstring(str);
#else
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
#endif
}

void GetDataPath(LPTSTR szOut, DWORD nSize)
{
	TCHAR szExePath[MAX_PATH];
	GetModuleFileName(nullptr, szExePath, _countof(szExePath));

	LPTSTR szExeName = _tcsrchr(szExePath, TEXT('\\'));
	szExeName = szExeName ? szExeName + 1 : szExePath;

	TCHAR szAppData[MAX_PATH];
	GetEnvironmentVariable(TEXT("AppData"), szAppData, _countof(szAppData));

	_tcsncpy(szOut, szAppData, nSize);
	_tcsncat(szOut, TEXT("\\"), nSize);
	_tcsncat(szOut, szExeName, nSize);
}

void ResizeBrowser(HWND hWnd)
{
	if (!controller)
	{
		return;
	}
	RECT bounds;
	GetClientRect(hWnd, &bounds);
	controller->lpVtbl->put_Bounds(controller, bounds);
}

void MessagePopup(LPCTSTR format, ...)
{
	MessageBox(nullptr, format, TEXT("webview2-mingw"), MB_OK | MB_ICONSTOP);
}



void createWebView(uintptr_t windowPtr, const char * cUrl)
{
	HWND hWnd = (HWND)windowPtr;

	// Get data path.
	TCHAR szDataPath[MAX_PATH];
	GetDataPath(szDataPath, _countof(szDataPath));

	// Set up some event handlers.
	EventHandler handler{};

	handler.EnvironmentCompleted = [&](HRESULT result, ICoreWebView2Environment* created_environment)
	{
		if (FAILED(result))
		{
			MessagePopup(TEXT("Failed to create environment?"));
		}
		created_environment->lpVtbl->CreateCoreWebView2Controller(created_environment, hWnd, &handler);
		// FIXME - removing the following popup makes host crash (!?)
		//         some call is missing here
		MessagePopup(TEXT("Created controller"));
		return S_OK;
	};

	handler.ControllerCompleted = [&](HRESULT result, ICoreWebView2Controller* new_controller)
	{
		if (FAILED(result))
		{
			MessagePopup(TEXT("Failed to create controller?"));
		}
		controller = new_controller;
		controller->lpVtbl->AddRef(controller);
		controller->lpVtbl->get_CoreWebView2(controller, &webview2);
		webview2->lpVtbl->AddRef(webview2);
		webview2->lpVtbl->Navigate(webview2, TStrToWStr(cUrl).c_str());
		ResizeBrowser(hWnd);
		return S_OK;
	};


	// Fails with 0x80070005 E_ACCESSDENIED
	// https://stackoverflow.com/questions/66374105/opening-webview2-in-wpf-causes-system-unauthorizedaccessexception-while-calling
	// Need to set files location
	//HRESULT result = CreateCoreWebView2Environment(&handler);


	HRESULT result = CreateCoreWebView2EnvironmentWithOptions(
		nullptr,
		//TStrToWStr(szDataPath).c_str(),	// comment out for now
		L"C:\\tmp",		// works
		nullptr,
		&handler
	);


	if (FAILED(result))
	{
		std::stringstream  ss;
		ss << "Call to CreateCoreWebView2EnvironmentWithOptions failed! - HRESULT " << std::hex << result;
		MessagePopup(ss.str().c_str());
	}



}
