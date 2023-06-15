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
#include "JSTestDefaultToJSONIndirectInheritance.h"

#include "ActiveDOMObject.h"
#include "ExtendedDOMClientIsoSubspaces.h"
#include "ExtendedDOMIsoSubspaces.h"
#include "JSDOMBinding.h"
#include "JSDOMConstructorNotConstructable.h"
#include "JSDOMExceptionHandling.h"
#include "JSDOMGlobalObjectInlines.h"
#include "JSDOMWrapperCache.h"
#include "ScriptExecutionContext.h"
#include "WebCoreJSClientData.h"
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

static JSC_DECLARE_CUSTOM_GETTER(jsTestDefaultToJSONIndirectInheritanceConstructor);

class JSTestDefaultToJSONIndirectInheritancePrototype final : public JSC::JSNonFinalObject {
public:
    using Base = JSC::JSNonFinalObject;
    static JSTestDefaultToJSONIndirectInheritancePrototype* create(JSC::VM& vm, JSDOMGlobalObject* globalObject, JSC::Structure* structure)
    {
        JSTestDefaultToJSONIndirectInheritancePrototype* ptr = new (NotNull, JSC::allocateCell<JSTestDefaultToJSONIndirectInheritancePrototype>(vm)) JSTestDefaultToJSONIndirectInheritancePrototype(vm, globalObject, structure);
        ptr->finishCreation(vm);
        return ptr;
    }

    DECLARE_INFO;
    template<typename CellType, JSC::SubspaceAccess>
    static JSC::GCClient::IsoSubspace* subspaceFor(JSC::VM& vm)
    {
        STATIC_ASSERT_ISO_SUBSPACE_SHARABLE(JSTestDefaultToJSONIndirectInheritancePrototype, Base);
        return &vm.plainObjectSpace();
    }
    static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype)
    {
        return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), info());
    }

private:
    JSTestDefaultToJSONIndirectInheritancePrototype(JSC::VM& vm, JSC::JSGlobalObject*, JSC::Structure* structure)
        : JSC::JSNonFinalObject(vm, structure)
    {
    }

    void finishCreation(JSC::VM&);
};
STATIC_ASSERT_ISO_SUBSPACE_SHARABLE(JSTestDefaultToJSONIndirectInheritancePrototype, JSTestDefaultToJSONIndirectInheritancePrototype::Base);

using JSTestDefaultToJSONIndirectInheritanceDOMConstructor = JSDOMConstructorNotConstructable<JSTestDefaultToJSONIndirectInheritance>;

template<> const ClassInfo JSTestDefaultToJSONIndirectInheritanceDOMConstructor::s_info = { "TestDefaultToJSONIndirectInheritance"_s, &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestDefaultToJSONIndirectInheritanceDOMConstructor) };

template<> JSValue JSTestDefaultToJSONIndirectInheritanceDOMConstructor::prototypeForStructure(JSC::VM& vm, const JSDOMGlobalObject& globalObject)
{
    return JSTestDefaultToJSONInherit::getConstructor(vm, &globalObject);
}

template<> void JSTestDefaultToJSONIndirectInheritanceDOMConstructor::initializeProperties(VM& vm, JSDOMGlobalObject& globalObject)
{
    putDirect(vm, vm.propertyNames->length, jsNumber(0), JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
    JSString* nameString = jsNontrivialString(vm, "TestDefaultToJSONIndirectInheritance"_s);
    m_originalName.set(vm, this, nameString);
    putDirect(vm, vm.propertyNames->name, nameString, JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
    putDirect(vm, vm.propertyNames->prototype, JSTestDefaultToJSONIndirectInheritance::prototype(vm, globalObject), JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum | JSC::PropertyAttribute::DontDelete);
}

/* Hash table for prototype */

static const HashTableValue JSTestDefaultToJSONIndirectInheritancePrototypeTableValues[] =
{
    { "constructor"_s, static_cast<unsigned>(JSC::PropertyAttribute::DontEnum), NoIntrinsic, { HashTableValue::GetterSetterType, jsTestDefaultToJSONIndirectInheritanceConstructor, 0 } },
};

const ClassInfo JSTestDefaultToJSONIndirectInheritancePrototype::s_info = { "TestDefaultToJSONIndirectInheritance"_s, &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestDefaultToJSONIndirectInheritancePrototype) };

void JSTestDefaultToJSONIndirectInheritancePrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    reifyStaticProperties(vm, JSTestDefaultToJSONIndirectInheritance::info(), JSTestDefaultToJSONIndirectInheritancePrototypeTableValues, *this);
    JSC_TO_STRING_TAG_WITHOUT_TRANSITION();
}

const ClassInfo JSTestDefaultToJSONIndirectInheritance::s_info = { "TestDefaultToJSONIndirectInheritance"_s, &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestDefaultToJSONIndirectInheritance) };

JSTestDefaultToJSONIndirectInheritance::JSTestDefaultToJSONIndirectInheritance(Structure* structure, JSDOMGlobalObject& globalObject, Ref<TestDefaultToJSONIndirectInheritance>&& impl)
    : JSTestDefaultToJSONInherit(structure, globalObject, WTFMove(impl))
{
}

void JSTestDefaultToJSONIndirectInheritance::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(info()));

    static_assert(!std::is_base_of<ActiveDOMObject, TestDefaultToJSONIndirectInheritance>::value, "Interface is not marked as [ActiveDOMObject] even though implementation class subclasses ActiveDOMObject.");

}

JSObject* JSTestDefaultToJSONIndirectInheritance::createPrototype(VM& vm, JSDOMGlobalObject& globalObject)
{
    return JSTestDefaultToJSONIndirectInheritancePrototype::create(vm, &globalObject, JSTestDefaultToJSONIndirectInheritancePrototype::createStructure(vm, &globalObject, JSTestDefaultToJSONInherit::prototype(vm, globalObject)));
}

JSObject* JSTestDefaultToJSONIndirectInheritance::prototype(VM& vm, JSDOMGlobalObject& globalObject)
{
    return getDOMPrototype<JSTestDefaultToJSONIndirectInheritance>(vm, globalObject);
}

JSValue JSTestDefaultToJSONIndirectInheritance::getConstructor(VM& vm, const JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSTestDefaultToJSONIndirectInheritanceDOMConstructor, DOMConstructorID::TestDefaultToJSONIndirectInheritance>(vm, *jsCast<const JSDOMGlobalObject*>(globalObject));
}

JSC_DEFINE_CUSTOM_GETTER(jsTestDefaultToJSONIndirectInheritanceConstructor, (JSGlobalObject* lexicalGlobalObject, EncodedJSValue thisValue, PropertyName))
{
    VM& vm = JSC::getVM(lexicalGlobalObject);
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto* prototype = jsDynamicCast<JSTestDefaultToJSONIndirectInheritancePrototype*>(JSValue::decode(thisValue));
    if (UNLIKELY(!prototype))
        return throwVMTypeError(lexicalGlobalObject, throwScope);
    return JSValue::encode(JSTestDefaultToJSONIndirectInheritance::getConstructor(JSC::getVM(lexicalGlobalObject), prototype->globalObject()));
}

JSC::GCClient::IsoSubspace* JSTestDefaultToJSONIndirectInheritance::subspaceForImpl(JSC::VM& vm)
{
    return WebCore::subspaceForImpl<JSTestDefaultToJSONIndirectInheritance, UseCustomHeapCellType::No>(vm,
        [] (auto& spaces) { return spaces.m_clientSubspaceForTestDefaultToJSONIndirectInheritance.get(); },
        [] (auto& spaces, auto&& space) { spaces.m_clientSubspaceForTestDefaultToJSONIndirectInheritance = std::forward<decltype(space)>(space); },
        [] (auto& spaces) { return spaces.m_subspaceForTestDefaultToJSONIndirectInheritance.get(); },
        [] (auto& spaces, auto&& space) { spaces.m_subspaceForTestDefaultToJSONIndirectInheritance = std::forward<decltype(space)>(space); }
    );
}

void JSTestDefaultToJSONIndirectInheritance::analyzeHeap(JSCell* cell, HeapAnalyzer& analyzer)
{
    auto* thisObject = jsCast<JSTestDefaultToJSONIndirectInheritance*>(cell);
    analyzer.setWrappedObjectForCell(cell, &thisObject->wrapped());
    if (thisObject->scriptExecutionContext())
        analyzer.setLabelForCell(cell, "url "_s + thisObject->scriptExecutionContext()->url().string());
    Base::analyzeHeap(cell, analyzer);
}


}
