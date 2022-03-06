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

#if ENABLE(ConditionDerived)

#include "JSTestOperationConditional.h"

#include "ActiveDOMObject.h"
#include "DOMIsoSubspaces.h"
#include "JSDOMBinding.h"
#include "JSDOMConstructorNotConstructable.h"
#include "JSDOMExceptionHandling.h"
#include "JSDOMGlobalObjectInlines.h"
#include "JSDOMOperation.h"
#include "JSDOMWrapperCache.h"
#include "ScriptExecutionContext.h"
#include "WebCoreJSClientData.h"
#include <JavaScriptCore/FunctionPrototype.h>
#include <JavaScriptCore/HeapAnalyzer.h>
#include <JavaScriptCore/JSCInlines.h>
#include <JavaScriptCore/JSDestructibleObjectHeapCellType.h>
#include <JavaScriptCore/SlotVisitorMacros.h>
#include <JavaScriptCore/SubspaceInlines.h>
#include <wtf/GetPtr.h>
#include <wtf/PointerPreparations.h>
#include <wtf/URL.h>

#if ENABLE(ConditionBase) || (ENABLE(ConditionBase) && ENABLE(ConditionOperation))
#include "IDLTypes.h"
#include "JSDOMConvertBase.h"
#endif


namespace WebCore {
using namespace JSC;

// Functions

#if ENABLE(ConditionBase)
static JSC_DECLARE_HOST_FUNCTION(jsTestOperationConditionalPrototypeFunction_nonConditionalOperation);
#endif
#if ENABLE(ConditionBase) && ENABLE(ConditionOperation)
static JSC_DECLARE_HOST_FUNCTION(jsTestOperationConditionalPrototypeFunction_conditionalOperation);
#endif

// Attributes

static JSC_DECLARE_CUSTOM_GETTER(jsTestOperationConditionalConstructor);

class JSTestOperationConditionalPrototype final : public JSC::JSNonFinalObject {
public:
    using Base = JSC::JSNonFinalObject;
    static JSTestOperationConditionalPrototype* create(JSC::VM& vm, JSDOMGlobalObject* globalObject, JSC::Structure* structure)
    {
        JSTestOperationConditionalPrototype* ptr = new (NotNull, JSC::allocateCell<JSTestOperationConditionalPrototype>(vm.heap)) JSTestOperationConditionalPrototype(vm, globalObject, structure);
        ptr->finishCreation(vm);
        return ptr;
    }

    DECLARE_INFO;
    template<typename CellType, JSC::SubspaceAccess>
    static JSC::IsoSubspace* subspaceFor(JSC::VM& vm)
    {
        STATIC_ASSERT_ISO_SUBSPACE_SHARABLE(JSTestOperationConditionalPrototype, Base);
        return &vm.plainObjectSpace;
    }
    static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype)
    {
        return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), info());
    }

private:
    JSTestOperationConditionalPrototype(JSC::VM& vm, JSC::JSGlobalObject*, JSC::Structure* structure)
        : JSC::JSNonFinalObject(vm, structure)
    {
    }

    void finishCreation(JSC::VM&);
};
STATIC_ASSERT_ISO_SUBSPACE_SHARABLE(JSTestOperationConditionalPrototype, JSTestOperationConditionalPrototype::Base);

using JSTestOperationConditionalDOMConstructor = JSDOMConstructorNotConstructable<JSTestOperationConditional>;

template<> const ClassInfo JSTestOperationConditionalDOMConstructor::s_info = { "TestOperationConditional", &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestOperationConditionalDOMConstructor) };

template<> JSValue JSTestOperationConditionalDOMConstructor::prototypeForStructure(JSC::VM& vm, const JSDOMGlobalObject& globalObject)
{
    UNUSED_PARAM(vm);
    return globalObject.functionPrototype();
}

template<> void JSTestOperationConditionalDOMConstructor::initializeProperties(VM& vm, JSDOMGlobalObject& globalObject)
{
    putDirect(vm, vm.propertyNames->prototype, JSTestOperationConditional::prototype(vm, globalObject), JSC::PropertyAttribute::DontDelete | JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
    putDirect(vm, vm.propertyNames->name, jsNontrivialString(vm, "TestOperationConditional"_s), JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
    putDirect(vm, vm.propertyNames->length, jsNumber(0), JSC::PropertyAttribute::ReadOnly | JSC::PropertyAttribute::DontEnum);
}

/* Hash table for prototype */

static const HashTableValue JSTestOperationConditionalPrototypeTableValues[] =
{
    { "constructor", static_cast<unsigned>(JSC::PropertyAttribute::DontEnum), NoIntrinsic, { (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsTestOperationConditionalConstructor), (intptr_t) static_cast<PutPropertySlot::PutValueFunc>(0) } },
#if ENABLE(ConditionBase)
    { "nonConditionalOperation", static_cast<unsigned>(JSC::PropertyAttribute::Function), NoIntrinsic, { (intptr_t)static_cast<RawNativeFunction>(jsTestOperationConditionalPrototypeFunction_nonConditionalOperation), (intptr_t) (0) } },
#else
    { 0, 0, NoIntrinsic, { 0, 0 } },
#endif
#if ENABLE(ConditionBase) && ENABLE(ConditionOperation)
    { "conditionalOperation", static_cast<unsigned>(JSC::PropertyAttribute::Function), NoIntrinsic, { (intptr_t)static_cast<RawNativeFunction>(jsTestOperationConditionalPrototypeFunction_conditionalOperation), (intptr_t) (0) } },
#else
    { 0, 0, NoIntrinsic, { 0, 0 } },
#endif
};

const ClassInfo JSTestOperationConditionalPrototype::s_info = { "TestOperationConditional", &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestOperationConditionalPrototype) };

void JSTestOperationConditionalPrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    reifyStaticProperties(vm, JSTestOperationConditional::info(), JSTestOperationConditionalPrototypeTableValues, *this);
    JSC_TO_STRING_TAG_WITHOUT_TRANSITION();
}

const ClassInfo JSTestOperationConditional::s_info = { "TestOperationConditional", &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSTestOperationConditional) };

JSTestOperationConditional::JSTestOperationConditional(Structure* structure, JSDOMGlobalObject& globalObject, Ref<TestOperationConditional>&& impl)
    : JSDOMWrapper<TestOperationConditional>(structure, globalObject, WTFMove(impl))
{
}

void JSTestOperationConditional::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));

    static_assert(!std::is_base_of<ActiveDOMObject, TestOperationConditional>::value, "Interface is not marked as [ActiveDOMObject] even though implementation class subclasses ActiveDOMObject.");

}

JSObject* JSTestOperationConditional::createPrototype(VM& vm, JSDOMGlobalObject& globalObject)
{
    return JSTestOperationConditionalPrototype::create(vm, &globalObject, JSTestOperationConditionalPrototype::createStructure(vm, &globalObject, globalObject.objectPrototype()));
}

JSObject* JSTestOperationConditional::prototype(VM& vm, JSDOMGlobalObject& globalObject)
{
    return getDOMPrototype<JSTestOperationConditional>(vm, globalObject);
}

JSValue JSTestOperationConditional::getConstructor(VM& vm, const JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSTestOperationConditionalDOMConstructor, DOMConstructorID::TestOperationConditional>(vm, *jsCast<const JSDOMGlobalObject*>(globalObject));
}

void JSTestOperationConditional::destroy(JSC::JSCell* cell)
{
    JSTestOperationConditional* thisObject = static_cast<JSTestOperationConditional*>(cell);
    thisObject->JSTestOperationConditional::~JSTestOperationConditional();
}

JSC_DEFINE_CUSTOM_GETTER(jsTestOperationConditionalConstructor, (JSGlobalObject* lexicalGlobalObject, EncodedJSValue thisValue, PropertyName))
{
    VM& vm = JSC::getVM(lexicalGlobalObject);
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto* prototype = jsDynamicCast<JSTestOperationConditionalPrototype*>(vm, JSValue::decode(thisValue));
    if (UNLIKELY(!prototype))
        return throwVMTypeError(lexicalGlobalObject, throwScope);
    return JSValue::encode(JSTestOperationConditional::getConstructor(JSC::getVM(lexicalGlobalObject), prototype->globalObject()));
}

#if ENABLE(ConditionBase)
static inline JSC::EncodedJSValue jsTestOperationConditionalPrototypeFunction_nonConditionalOperationBody(JSC::JSGlobalObject* lexicalGlobalObject, JSC::CallFrame* callFrame, typename IDLOperation<JSTestOperationConditional>::ClassParameter castedThis)
{
    auto& vm = JSC::getVM(lexicalGlobalObject);
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    UNUSED_PARAM(throwScope);
    UNUSED_PARAM(callFrame);
    auto& impl = castedThis->wrapped();
    RELEASE_AND_RETURN(throwScope, JSValue::encode(toJS<IDLUndefined>(*lexicalGlobalObject, throwScope, [&]() -> decltype(auto) { return impl.nonConditionalOperation(); })));
}

JSC_DEFINE_HOST_FUNCTION(jsTestOperationConditionalPrototypeFunction_nonConditionalOperation, (JSGlobalObject* lexicalGlobalObject, CallFrame* callFrame))
{
    return IDLOperation<JSTestOperationConditional>::call<jsTestOperationConditionalPrototypeFunction_nonConditionalOperationBody>(*lexicalGlobalObject, *callFrame, "nonConditionalOperation");
}

#endif

#if ENABLE(ConditionBase) && ENABLE(ConditionOperation)
static inline JSC::EncodedJSValue jsTestOperationConditionalPrototypeFunction_conditionalOperationBody(JSC::JSGlobalObject* lexicalGlobalObject, JSC::CallFrame* callFrame, typename IDLOperation<JSTestOperationConditional>::ClassParameter castedThis)
{
    auto& vm = JSC::getVM(lexicalGlobalObject);
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    UNUSED_PARAM(throwScope);
    UNUSED_PARAM(callFrame);
    auto& impl = castedThis->wrapped();
    RELEASE_AND_RETURN(throwScope, JSValue::encode(toJS<IDLUndefined>(*lexicalGlobalObject, throwScope, [&]() -> decltype(auto) { return impl.conditionalOperation(); })));
}

JSC_DEFINE_HOST_FUNCTION(jsTestOperationConditionalPrototypeFunction_conditionalOperation, (JSGlobalObject* lexicalGlobalObject, CallFrame* callFrame))
{
    return IDLOperation<JSTestOperationConditional>::call<jsTestOperationConditionalPrototypeFunction_conditionalOperationBody>(*lexicalGlobalObject, *callFrame, "conditionalOperation");
}

#endif

JSC::IsoSubspace* JSTestOperationConditional::subspaceForImpl(JSC::VM& vm)
{
    auto& clientData = *static_cast<JSVMClientData*>(vm.clientData);
    auto& spaces = clientData.subspaces();
    if (auto* space = spaces.m_subspaceForTestOperationConditional.get())
        return space;
    static_assert(std::is_base_of_v<JSC::JSDestructibleObject, JSTestOperationConditional> || !JSTestOperationConditional::needsDestruction);
    if constexpr (std::is_base_of_v<JSC::JSDestructibleObject, JSTestOperationConditional>)
        spaces.m_subspaceForTestOperationConditional = makeUnique<IsoSubspace> ISO_SUBSPACE_INIT(vm.heap, vm.destructibleObjectHeapCellType.get(), JSTestOperationConditional);
    else
        spaces.m_subspaceForTestOperationConditional = makeUnique<IsoSubspace> ISO_SUBSPACE_INIT(vm.heap, vm.cellHeapCellType.get(), JSTestOperationConditional);
    auto* space = spaces.m_subspaceForTestOperationConditional.get();
IGNORE_WARNINGS_BEGIN("unreachable-code")
IGNORE_WARNINGS_BEGIN("tautological-compare")
    void (*myVisitOutputConstraint)(JSC::JSCell*, JSC::SlotVisitor&) = JSTestOperationConditional::visitOutputConstraints;
    void (*jsCellVisitOutputConstraint)(JSC::JSCell*, JSC::SlotVisitor&) = JSC::JSCell::visitOutputConstraints;
    if (myVisitOutputConstraint != jsCellVisitOutputConstraint)
        clientData.outputConstraintSpaces().append(space);
IGNORE_WARNINGS_END
IGNORE_WARNINGS_END
    return space;
}

void JSTestOperationConditional::analyzeHeap(JSCell* cell, HeapAnalyzer& analyzer)
{
    auto* thisObject = jsCast<JSTestOperationConditional*>(cell);
    analyzer.setWrappedObjectForCell(cell, &thisObject->wrapped());
    if (thisObject->scriptExecutionContext())
        analyzer.setLabelForCell(cell, "url " + thisObject->scriptExecutionContext()->url().string());
    Base::analyzeHeap(cell, analyzer);
}

bool JSTestOperationConditionalOwner::isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown> handle, void*, AbstractSlotVisitor& visitor, const char** reason)
{
    UNUSED_PARAM(handle);
    UNUSED_PARAM(visitor);
    UNUSED_PARAM(reason);
    return false;
}

void JSTestOperationConditionalOwner::finalize(JSC::Handle<JSC::Unknown> handle, void* context)
{
    auto* jsTestOperationConditional = static_cast<JSTestOperationConditional*>(handle.slot()->asCell());
    auto& world = *static_cast<DOMWrapperWorld*>(context);
    uncacheWrapper(world, &jsTestOperationConditional->wrapped(), jsTestOperationConditional);
}

#if ENABLE(BINDING_INTEGRITY)
#if PLATFORM(WIN)
#pragma warning(disable: 4483)
extern "C" { extern void (*const __identifier("??_7TestOperationConditional@WebCore@@6B@")[])(); }
#else
extern "C" { extern void* _ZTVN7WebCore24TestOperationConditionalE[]; }
#endif
#endif

JSC::JSValue toJSNewlyCreated(JSC::JSGlobalObject*, JSDOMGlobalObject* globalObject, Ref<TestOperationConditional>&& impl)
{

#if ENABLE(BINDING_INTEGRITY)
    const void* actualVTablePointer = getVTablePointer(impl.ptr());
#if PLATFORM(WIN)
    void* expectedVTablePointer = __identifier("??_7TestOperationConditional@WebCore@@6B@");
#else
    void* expectedVTablePointer = &_ZTVN7WebCore24TestOperationConditionalE[2];
#endif

    // If this fails TestOperationConditional does not have a vtable, so you need to add the
    // ImplementationLacksVTable attribute to the interface definition
    static_assert(std::is_polymorphic<TestOperationConditional>::value, "TestOperationConditional is not polymorphic");

    // If you hit this assertion you either have a use after free bug, or
    // TestOperationConditional has subclasses. If TestOperationConditional has subclasses that get passed
    // to toJS() we currently require TestOperationConditional you to opt out of binding hardening
    // by adding the SkipVTableValidation attribute to the interface IDL definition
    RELEASE_ASSERT(actualVTablePointer == expectedVTablePointer);
#endif
    return createWrapper<TestOperationConditional>(globalObject, WTFMove(impl));
}

JSC::JSValue toJS(JSC::JSGlobalObject* lexicalGlobalObject, JSDOMGlobalObject* globalObject, TestOperationConditional& impl)
{
    return wrap(lexicalGlobalObject, globalObject, impl);
}

TestOperationConditional* JSTestOperationConditional::toWrapped(JSC::VM& vm, JSC::JSValue value)
{
    if (auto* wrapper = jsDynamicCast<JSTestOperationConditional*>(vm, value))
        return &wrapper->wrapped();
    return nullptr;
}

}

#endif // ENABLE(ConditionDerived)
