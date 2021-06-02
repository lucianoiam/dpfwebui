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

#ifndef COM_HPP
#define COM_HPP

namespace com {

    template <typename T>
    static HRESULT STDMETHODCALLTYPE Null_QueryInterface(T* This, REFIID riid, void** ppvObject)
    {
        (void)This;
        (void)riid;
        (void)ppvObject;
        return E_NOINTERFACE;
    }

    template <typename T>
    static ULONG STDMETHODCALLTYPE Null_AddRef(T* This)
    {
        (void)This;
        return 1;
    }

    template <typename T>
    static ULONG STDMETHODCALLTYPE Null_Release(T* This)
    {
        (void)This;
        return 1;
    }

} // namespace com

#endif // COM_HPP
