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

#if ENABLE(Condition22) || ENABLE(Condition23)

#include "JSDOMWrapper.h"
#include "TestLegacyNoInterfaceObject.h"
#include <wtf/NeverDestroyed.h>

namespace WebCore {

class JSTestLegacyNoInterfaceObject : public JSDOMWrapper<TestLegacyNoInterfaceObject> {
public:
    using Base = JSDOMWrapper<TestLegacyNoInterfaceObject>;
    static JSTestLegacyNoInterfaceObject* create(JSC::Structure* structure, JSDOMGlobalObject* globalObject, Ref<TestLegacyNoInterfaceObject>&& impl)
    {
        JSTestLegacyNoInterfaceObject* ptr = new (NotNull, JSC::allocateCell<JSTestLegacyNoInterfaceObject>(globalObject->vm())) JSTestLegacyNoInterfaceObject(structure, *globalObject, WTFMove(impl));
        ptr->finishCreation(globalObject->vm());
        return ptr;
    }

    static JSC::JSObject* createPrototype(JSC::VM&, JSDOMGlobalObject&);
    static JSC::JSObject* prototype(JSC::VM&, JSDOMGlobalObject&);
    static TestLegacyNoInterfaceObject* toWrapped(JSC::VM&, JSC::JSValue);
    static void destroy(JSC::JSCell*);

    DECLARE_INFO;

    static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype)
    {
        return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), info(), JSC::NonArray);
    }

    template<typename, JSC::SubspaceAccess mode> static JSC::GCClient::IsoSubspace* subspaceFor(JSC::VM& vm)
    {
        if constexpr (mode == JSC::SubspaceAccess::Concurrently)
            return nullptr;
        return subspaceForImpl(vm);
    }
    static JSC::GCClient::IsoSubspace* subspaceForImpl(JSC::VM& vm);
    static void analyzeHeap(JSCell*, JSC::HeapAnalyzer&);

    // Custom attributes
    JSC::JSValue customGetterSetterStringAttribute(JSC::JSGlobalObject&) const;
    void setCustomGetterSetterStringAttribute(JSC::JSGlobalObject&, JSC::JSValue);

    // Custom functions
    JSC::JSValue customOperation(JSC::JSGlobalObject&, JSC::CallFrame&);
protected:
    JSTestLegacyNoInterfaceObject(JSC::Structure*, JSDOMGlobalObject&, Ref<TestLegacyNoInterfaceObject>&&);

    DECLARE_DEFAULT_FINISH_CREATION;
};

class JSTestLegacyNoInterfaceObjectOwner final : public JSC::WeakHandleOwner {
public:
    bool isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown>, void* context, JSC::AbstractSlotVisitor&, const char**) final;
    void finalize(JSC::Handle<JSC::Unknown>, void* context) final;
};

inline JSC::WeakHandleOwner* wrapperOwner(DOMWrapperWorld&, TestLegacyNoInterfaceObject*)
{
    static NeverDestroyed<JSTestLegacyNoInterfaceObjectOwner> owner;
    return &owner.get();
}

inline void* wrapperKey(TestLegacyNoInterfaceObject* wrappableObject)
{
    return wrappableObject;
}

JSC::JSValue toJS(JSC::JSGlobalObject*, JSDOMGlobalObject*, TestLegacyNoInterfaceObject&);
inline JSC::JSValue toJS(JSC::JSGlobalObject* lexicalGlobalObject, JSDOMGlobalObject* globalObject, TestLegacyNoInterfaceObject* impl) { return impl ? toJS(lexicalGlobalObject, globalObject, *impl) : JSC::jsNull(); }
JSC::JSValue toJSNewlyCreated(JSC::JSGlobalObject*, JSDOMGlobalObject*, Ref<TestLegacyNoInterfaceObject>&&);
inline JSC::JSValue toJSNewlyCreated(JSC::JSGlobalObject* lexicalGlobalObject, JSDOMGlobalObject* globalObject, RefPtr<TestLegacyNoInterfaceObject>&& impl) { return impl ? toJSNewlyCreated(lexicalGlobalObject, globalObject, impl.releaseNonNull()) : JSC::jsNull(); }

template<> struct JSDOMWrapperConverterTraits<TestLegacyNoInterfaceObject> {
    using WrapperClass = JSTestLegacyNoInterfaceObject;
    using ToWrappedReturnType = TestLegacyNoInterfaceObject*;
};

} // namespace WebCore

#endif // ENABLE(Condition22) || ENABLE(Condition23)
