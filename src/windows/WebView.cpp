/*
 * Hip-Hop / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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


EventHandler handler{};
wchar_t* wUrl = new wchar_t[4096];


void createWebView(uintptr_t windowPtr, const char * cUrl)
{
	HWND hWnd = (HWND)windowPtr;

	if (controller != nullptr) {
		return;
	}

	// Get data path.
	TCHAR szDataPath[MAX_PATH];
	GetDataPath(szDataPath, _countof(szDataPath));

	// Set up some event handlers.


	handler.EnvironmentCompleted = [&](HRESULT result, ICoreWebView2Environment* created_environment)
	{
		if (FAILED(result))
		{
			MessagePopup(TEXT("Failed to create environment"));
		}
		created_environment->lpVtbl->CreateCoreWebView2Controller(created_environment, hWnd, &handler);
		return S_OK;
	};

	MultiByteToWideChar(CP_ACP, 0, cUrl, -1, wUrl, 4096);

	handler.ControllerCompleted = [&](HRESULT result, ICoreWebView2Controller* new_controller)
	{
		if (FAILED(result))
		{
			MessagePopup(TEXT("Failed to create controller"));
		}
		controller = new_controller;
		controller->lpVtbl->AddRef(controller);
		controller->lpVtbl->get_CoreWebView2(controller, &webview2);
		webview2->lpVtbl->AddRef(webview2);
		webview2->lpVtbl->Navigate(webview2, wUrl);
		//webview2->lpVtbl->Navigate(webview2, TStrToWStr(cUrl).c_str());	crashes
		//ResizeBrowser(hWnd);	//does not work as expected (wrong rect?)
		RECT bounds;
		bounds.top = 0;
		bounds.left = 0;
		bounds.right = 800;
		bounds.bottom = 600;
		controller->lpVtbl->put_Bounds(controller, bounds);
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
