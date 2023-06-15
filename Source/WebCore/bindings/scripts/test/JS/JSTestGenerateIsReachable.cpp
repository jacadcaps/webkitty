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

#include "config.h"
#include "JSTestGenerateIsReachable.h"

#include "ActiveDOMObject.h"
#include "ExtendedDOMClientIsoSubspaces.h"
#include "ExtendedDOMIsoSubspaces.h"
#include "JSDOMAttribute.h"
#include "JSDOMBinding.h"
#include "JSDOMConstructorNotConstructable.h"
#include "JSDOMConvertStrings.h"
#include "JSDOMExceptionHandling.h"
#include "JSDOMGlobalObjectInlines.h"
#include "JSDOMWrapperCache.h"
#include "ScriptExecutionContext.h"
#include "WebCoreJSClientData.h"
#include "WebCoreOpaqueRoot.h"
#include <JavaScriptCore/FunctionPrototype.h>
#include <JavaScriptCore/HeapAnalyzer.h>
#include <JavaScriptCore/JSCInlines.h>
#include <JavaScriptCore/JSDestructibleObjectHeapCellType.h>
#include <JavaScriptCore/SlotVisitorMacros.h>
#include <JavaScriptCore/SubspaceInlines.h>
#include <wtf/GetPtr.h>
#include <wtf/PointerPreparations.h>
#include <wtf/URL.h>


namespace WebCore {
using namespace JSC;

// Attributes

static JSC_DECLARE_CUSTOM_GETTER(jsTestGenerateIsReachableConstructor);
static JSC_DECLARE_CUSTOM_GETTER(jsTestGenerateIsReachable_aSecretAttribute);

class JSTestGenerateIsReachablePrototype final : public JSC::JSNonFinalObject {
public:
    using Base = JSC::JSNonFinalObject;
    static JSTestGenerateIsReachablePrototype* create(JSC::VM& vm, JSDOMGlobalObject* globalObject, JSC::Structure* structure)
    {
        JSTestGenerateIsReachablePrototype* ptr = new (NotNull, JSC::allocateCell<JSTestGenerateIsReachablePrototype>(vm)) JSTestGenerateIsReachablePrototype(vm, globalObject, structure);
        ptr->finishCreation(vm);
        return ptr;
    }

    DECLARE_INFO;
    template<typename CellType, JSC::SubspaceAccess>
    static JSC::GCClient::IsoSubspace* subspaceFor(JSC::VM& vm)
    {
        STATIC_ASSERT_ISO_SUBSPACE_SHARABLE(JSTestGenerateIsReachablePrototype, Base);
        return &vm.plainObjectSpace();
    }
    static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype)
    {
        return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), info());
    }

private:
    JSTestGenerateIsReachablePrototype(JSC::VM& vm, JSC::JSGlobalObject*, JSC::Structure* structure)
        : JSC::JSNonFinalObject(vm, structure)
    {
    }

    void finishCreation(JSC::VM&);
};
STATIC_ASSERT_ISO_SUBSPACE_SHARABLE(JSTestGenerateIsReachablePrototype, JSTestGenerateIsReachablePrototype::Base);

using JSTestGenerateIsReachableDOMConstructor = JSDOMConstructorNotConstructable<JSTestGenerateIsReachable>;

template<> const ClassInfo JSTestGenerateIsReachableDOMConstructor::s_info = { "TestGenerateIsReachable"_s, &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestGenerateIsReachableDOMConstructor) };

template<> JSValue JSTestGenerateIsReachableDOMConstructor::prototypeForStructure(JSC::VM& vm, const JSDOMGlobalObject& globalObject)
{
    UNUSED_PARAM(vm);
    return globalObject.functionPrototype();
}

template<> void JSTestGenerateIsReachableDOMConstructor::initializeProperties(VM& vm, JSDOMGlobalObject& globalObject)
{
    putDirect(vm, vm.propertyNames->length, jsNumber(0), JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
    JSString* nameString = jsNontrivialString(vm, "TestGenerateIsReachable"_s);
    m_originalName.set(vm, this, nameString);
    putDirect(vm, vm.propertyNames->name, nameString, JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
    putDirect(vm, vm.propertyNames->prototype, JSTestGenerateIsReachable::prototype(vm, globalObject), JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum | JSC::PropertyAttribute::DontDelete);
}

/* Hash table for prototype */

static const HashTableValue JSTestGenerateIsReachablePrototypeTableValues[] =
{
    { "constructor"_s, static_cast<unsigned>(JSC::PropertyAttribute::DontEnum), NoIntrinsic, { HashTableValue::GetterSetterType, jsTestGenerateIsReachableConstructor, 0 } },
    { "aSecretAttribute"_s, static_cast<unsigned>(JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::CustomAccessor | JSC::PropertyAttribute::DOMAttribute), NoIntrinsic, { HashTableValue::GetterSetterType, jsTestGenerateIsReachable_aSecretAttribute, 0 } },
};

const ClassInfo JSTestGenerateIsReachablePrototype::s_info = { "TestGenerateIsReachable"_s, &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestGenerateIsReachablePrototype) };

void JSTestGenerateIsReachablePrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    reifyStaticProperties(vm, JSTestGenerateIsReachable::info(), JSTestGenerateIsReachablePrototypeTableValues, *this);
    bool hasDisabledRuntimeProperties = false;
    if (!jsCast<JSDOMGlobalObject*>(globalObject())->scriptExecutionContext()->isSecureContext()) {
        hasDisabledRuntimeProperties = true;
        auto propertyName = Identifier::fromString(vm, "aSecretAttribute"_s);
        VM::DeletePropertyModeScope scope(vm, VM::DeletePropertyMode::IgnoreConfigurable);
        DeletePropertySlot slot;
        JSObject::deleteProperty(this, globalObject(), propertyName, slot);
    }
    if (hasDisabledRuntimeProperties && structure()->isDictionary())
        flattenDictionaryObject(vm);
    JSC_TO_STRING_TAG_WITHOUT_TRANSITION();
}

const ClassInfo JSTestGenerateIsReachable::s_info = { "TestGenerateIsReachable"_s, &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestGenerateIsReachable) };

JSTestGenerateIsReachable::JSTestGenerateIsReachable(Structure* structure, JSDOMGlobalObject& globalObject, Ref<TestGenerateIsReachable>&& impl)
    : JSDOMWrapper<TestGenerateIsReachable>(structure, globalObject, WTFMove(impl))
{
}

void JSTestGenerateIsReachable::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(info()));

    static_assert(!std::is_base_of<ActiveDOMObject, TestGenerateIsReachable>::value, "Interface is not marked as [ActiveDOMObject] even though implementation class subclasses ActiveDOMObject.");

}

JSObject* JSTestGenerateIsReachable::createPrototype(VM& vm, JSDOMGlobalObject& globalObject)
{
    return JSTestGenerateIsReachablePrototype::create(vm, &globalObject, JSTestGenerateIsReachablePrototype::createStructure(vm, &globalObject, globalObject.objectPrototype()));
}

JSObject* JSTestGenerateIsReachable::prototype(VM& vm, JSDOMGlobalObject& globalObject)
{
    return getDOMPrototype<JSTestGenerateIsReachable>(vm, globalObject);
}

JSValue JSTestGenerateIsReachable::getConstructor(VM& vm, const JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSTestGenerateIsReachableDOMConstructor, DOMConstructorID::TestGenerateIsReachable>(vm, *jsCast<const JSDOMGlobalObject*>(globalObject));
}

void JSTestGenerateIsReachable::destroy(JSC::JSCell* cell)
{
    JSTestGenerateIsReachable* thisObject = static_cast<JSTestGenerateIsReachable*>(cell);
    thisObject->JSTestGenerateIsReachable::~JSTestGenerateIsReachable();
}

JSC_DEFINE_CUSTOM_GETTER(jsTestGenerateIsReachableConstructor, (JSGlobalObject* lexicalGlobalObject, EncodedJSValue thisValue, PropertyName))
{
    VM& vm = JSC::getVM(lexicalGlobalObject);
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto* prototype = jsDynamicCast<JSTestGenerateIsReachablePrototype*>(JSValue::decode(thisValue));
    if (UNLIKELY(!prototype))
        return throwVMTypeError(lexicalGlobalObject, throwScope);
    return JSValue::encode(JSTestGenerateIsReachable::getConstructor(JSC::getVM(lexicalGlobalObject), prototype->globalObject()));
}

static inline JSValue jsTestGenerateIsReachable_aSecretAttributeGetter(JSGlobalObject& lexicalGlobalObject, JSTestGenerateIsReachable& thisObject)
{
    auto& vm = JSC::getVM(&lexicalGlobalObject);
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto& impl = thisObject.wrapped();
    RELEASE_AND_RETURN(throwScope, (toJS<IDLDOMString>(lexicalGlobalObject, throwScope, impl.aSecretAttribute())));
}

JSC_DEFINE_CUSTOM_GETTER(jsTestGenerateIsReachable_aSecretAttribute, (JSGlobalObject* lexicalGlobalObject, EncodedJSValue thisValue, PropertyName attributeName))
{
    return IDLAttribute<JSTestGenerateIsReachable>::get<jsTestGenerateIsReachable_aSecretAttributeGetter, CastedThisErrorBehavior::Assert>(*lexicalGlobalObject, thisValue, attributeName);
}

JSC::GCClient::IsoSubspace* JSTestGenerateIsReachable::subspaceForImpl(JSC::VM& vm)
{
    return WebCore::subspaceForImpl<JSTestGenerateIsReachable, UseCustomHeapCellType::No>(vm,
        [] (auto& spaces) { return spaces.m_clientSubspaceForTestGenerateIsReachable.get(); },
        [] (auto& spaces, auto&& space) { spaces.m_clientSubspaceForTestGenerateIsReachable = std::forward<decltype(space)>(space); },
        [] (auto& spaces) { return spaces.m_subspaceForTestGenerateIsReachable.get(); },
        [] (auto& spaces, auto&& space) { spaces.m_subspaceForTestGenerateIsReachable = std::forward<decltype(space)>(space); }
    );
}

void JSTestGenerateIsReachable::analyzeHeap(JSCell* cell, HeapAnalyzer& analyzer)
{
    auto* thisObject = jsCast<JSTestGenerateIsReachable*>(cell);
    analyzer.setWrappedObjectForCell(cell, &thisObject->wrapped());
    if (thisObject->scriptExecutionContext())
        analyzer.setLabelForCell(cell, "url "_s + thisObject->scriptExecutionContext()->url().string());
    Base::analyzeHeap(cell, analyzer);
}

bool JSTestGenerateIsReachableOwner::isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown> handle, void*, AbstractSlotVisitor& visitor, const char** reason)
{
    auto* jsTestGenerateIsReachable = jsCast<JSTestGenerateIsReachable*>(handle.slot()->asCell());
    TestGenerateIsReachable* owner = &jsTestGenerateIsReachable->wrapped();
    if (UNLIKELY(reason))
        *reason = "Reachable from TestGenerateIsReachable";
    return containsWebCoreOpaqueRoot(visitor, owner);
}

void JSTestGenerateIsReachableOwner::finalize(JSC::Handle<JSC::Unknown> handle, void* context)
{
    auto* jsTestGenerateIsReachable = static_cast<JSTestGenerateIsReachable*>(handle.slot()->asCell());
    auto& world = *static_cast<DOMWrapperWorld*>(context);
    uncacheWrapper(world, &jsTestGenerateIsReachable->wrapped(), jsTestGenerateIsReachable);
}

#if ENABLE(BINDING_INTEGRITY)
#if PLATFORM(WIN)
#pragma warning(disable: 4483)
extern "C" { extern void (*const __identifier("??_7TestGenerateIsReachable@WebCore@@6B@")[])(); }
#else
extern "C" { extern void* _ZTVN7WebCore23TestGenerateIsReachableE[]; }
#endif
#endif

JSC::JSValue toJSNewlyCreated(JSC::JSGlobalObject*, JSDOMGlobalObject* globalObject, Ref<TestGenerateIsReachable>&& impl)
{

    if constexpr (std::is_polymorphic_v<TestGenerateIsReachable>) {
#if ENABLE(BINDING_INTEGRITY)
        const void* actualVTablePointer = getVTablePointer(impl.ptr());
#if PLATFORM(WIN)
        void* expectedVTablePointer = __identifier("??_7TestGenerateIsReachable@WebCore@@6B@");
#else
        void* expectedVTablePointer = &_ZTVN7WebCore23TestGenerateIsReachableE[2];
#endif

        // If you hit this assertion you either have a use after free bug, or
        // TestGenerateIsReachable has subclasses. If TestGenerateIsReachable has subclasses that get passed
        // to toJS() we currently require TestGenerateIsReachable you to opt out of binding hardening
        // by adding the SkipVTableValidation attribute to the interface IDL definition
        RELEASE_ASSERT(actualVTablePointer == expectedVTablePointer);
#endif
    }
    return createWrapper<TestGenerateIsReachable>(globalObject, WTFMove(impl));
}

JSC::JSValue toJS(JSC::JSGlobalObject* lexicalGlobalObject, JSDOMGlobalObject* globalObject, TestGenerateIsReachable& impl)
{
    return wrap(lexicalGlobalObject, globalObject, impl);
}

TestGenerateIsReachable* JSTestGenerateIsReachable::toWrapped(JSC::VM&, JSC::JSValue value)
{
    if (auto* wrapper = jsDynamicCast<JSTestGenerateIsReachable*>(value))
        return &wrapper->wrapped();
    return nullptr;
}

}
