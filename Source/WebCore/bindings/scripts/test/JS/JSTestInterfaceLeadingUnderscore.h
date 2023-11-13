/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#pragma once

#include "JSDOMWrapper.h"
#include "TestInterfaceLeadingUnderscore.h"
#include <wtf/NeverDestroyed.h>

namespace WebCore {

class JSTestInterfaceLeadingUnderscore : public JSDOMWrapper<TestInterfaceLeadingUnderscore> {
public:
    using Base = JSDOMWrapper<TestInterfaceLeadingUnderscore>;
    static JSTestInterfaceLeadingUnderscore* create(JSC::Structure* structure, JSDOMGlobalObject* globalObject, Ref<TestInterfaceLeadingUnderscore>&& impl)
    {
        JSTestInterfaceLeadingUnderscore* ptr = new (NotNull, JSC::allocateCell<JSTestInterfaceLeadingUnderscore>(globalObject->vm())) JSTestInterfaceLeadingUnderscore(structure, *globalObject, WTFMove(impl));
        ptr->finishCreation(globalObject->vm());
        return ptr;
    }

    static JSC::JSObject* createPrototype(JSC::VM&, JSDOMGlobalObject&);
    static JSC::JSObject* prototype(JSC::VM&, JSDOMGlobalObject&);
    static TestInterfaceLeadingUnderscore* toWrapped(JSC::VM&, JSC::JSValue);
    static void destroy(JSC::JSCell*);

    DECLARE_INFO;

    static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype)
    {
        return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), info(), JSC::NonArray);
    }

    static JSC::JSValue getConstructor(JSC::VM&, const JSC::JSGlobalObject*);
    template<typename, JSC::SubspaceAccess mode> static JSC::GCClient::IsoSubspace* subspaceFor(JSC::VM& vm)
    {
        if constexpr (mode == JSC::SubspaceAccess::Concurrently)
            return nullptr;
        return subspaceForImpl(vm);
    }
    static JSC::GCClient::IsoSubspace* subspaceForImpl(JSC::VM& vm);
    static void analyzeHeap(JSCell*, JSC::HeapAnalyzer&);
protected:
    JSTestInterfaceLeadingUnderscore(JSC::Structure*, JSDOMGlobalObject&, Ref<TestInterfaceLeadingUnderscore>&&);

    DECLARE_DEFAULT_FINISH_CREATION;
};

class JSTestInterfaceLeadingUnderscoreOwner final : public JSC::WeakHandleOwner {
public:
    bool isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown>, void* context, JSC::AbstractSlotVisitor&, const char**) final;
    void finalize(JSC::Handle<JSC::Unknown>, void* context) final;
};

inline JSC::WeakHandleOwner* wrapperOwner(DOMWrapperWorld&, TestInterfaceLeadingUnderscore*)
{
    static NeverDestroyed<JSTestInterfaceLeadingUnderscoreOwner> owner;
    return &owner.get();
}

inline void* wrapperKey(TestInterfaceLeadingUnderscore* wrappableObject)
{
    return wrappableObject;
}

JSC::JSValue toJS(JSC::JSGlobalObject*, JSDOMGlobalObject*, TestInterfaceLeadingUnderscore&);
inline JSC::JSValue toJS(JSC::JSGlobalObject* lexicalGlobalObject, JSDOMGlobalObject* globalObject, TestInterfaceLeadingUnderscore* impl) { return impl ? toJS(lexicalGlobalObject, globalObject, *impl) : JSC::jsNull(); }
JSC::JSValue toJSNewlyCreated(JSC::JSGlobalObject*, JSDOMGlobalObject*, Ref<TestInterfaceLeadingUnderscore>&&);
inline JSC::JSValue toJSNewlyCreated(JSC::JSGlobalObject* lexicalGlobalObject, JSDOMGlobalObject* globalObject, RefPtr<TestInterfaceLeadingUnderscore>&& impl) { return impl ? toJSNewlyCreated(lexicalGlobalObject, globalObject, impl.releaseNonNull()) : JSC::jsNull(); }

template<> struct JSDOMWrapperConverterTraits<TestInterfaceLeadingUnderscore> {
    using WrapperClass = JSTestInterfaceLeadingUnderscore;
    using ToWrappedReturnType = TestInterfaceLeadingUnderscore*;
};

} // namespace WebCore
