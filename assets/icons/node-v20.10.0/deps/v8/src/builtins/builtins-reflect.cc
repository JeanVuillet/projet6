// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/builtins/builtins-utils-inl.h"
#include "src/builtins/builtins.h"
#include "src/logging/counters.h"
#include "src/objects/keys.h"
#include "src/objects/lookup.h"
#include "src/objects/objects-inl.h"
#include "src/objects/property-descriptor.h"

namespace v8 {
namespace internal {

// -----------------------------------------------------------------------------
// ES6 section 26.1 The Reflect Object

// ES6 section 26.1.3 Reflect.defineProperty
BUILTIN(ReflectDefineProperty) {
  HandleScope scope(isolate);
  DCHECK_LE(4, args.length());
  Handle<Object> target = args.at(1);
  Handle<Object> key = args.at(2);
  Handle<Object> attributes = args.at(3);

  if (!target->IsJSReceiver()) {
    THROW_NEW_ERROR_RETURN_FAILURE(
        isolate, NewTypeError(MessageTemplate::kCalledOnNonObject,
                              isolate->factory()->NewStringFromAsciiChecked(
                                  "Reflect.defineProperty")));
  }

  Handle<Name> name;
  ASSIGN_RETURN_FAILURE_ON_EXCEPTION(isolate, name,
                                     Object::ToName(isolate, key));

  PropertyDescriptor desc;
  if (!PropertyDescriptor::ToPropertyDescriptor(isolate, attributes, &desc)) {
    return ReadOnlyRoots(isolate).exception();
  }

  Maybe<bool> result = JSReceiver::DefineOwnProperty(
      isolate, Handle<JSReceiver>::cast(target), name, &desc, Just(kDontThrow));
  MAYBE_RETURN(result, ReadOnlyRoots(isolate).exception());
  return *isolate->factory()->ToBoolean(result.FromJust());
}

// ES6 section 26.1.11 Reflect.ownKeys
BUILTIN(ReflectOwnKeys) {
  HandleScope scope(isolate);
  DCHECK_LE(2, args.length());
  Handle<Object> target = args.at(1);

  if (!target->IsJSReceiver()) {
    THROW_NEW_ERROR_RETURN_FAILURE(
        isolate, NewTypeError(MessageTemplate::kCalledOnNonObject,
                              isolate->factory()->NewStringFromAsciiChecked(
                                  "Reflect.ownKeys")));
  }

  Handle<FixedArray> keys;
  ASSIGN_RETURN_FAILURE_ON_EXCEPTION(
      isolate, keys,
      KeyAccumulator::GetKeys(isolate, Handle<JSReceiver>::cast(target),
                              KeyCollectionMode::kOwnOnly, ALL_PROPERTIES,
                              GetKeysConversion::kConvertToString));
  return *isolate->factory()->NewJSArrayWithElements(keys);
}

// ES6 section 26.1.13 Reflect.set
BUILTIN(ReflectSet) {
  HandleScope scope(isolate);
  Handle<Object> target = args.atOrUndefined(isolate, 1);
  Handle<Object> key = args.atOrUndefined(isolate, 2);
  Handle<Object> value = args.atOrUndefined(isolate, 3);
  Handle<Object> receiver = args.length() > 4 ? args.at(4) : target;

  if (!target->IsJSReceiver()) {
    THROW_NEW_ERROR_RETURN_FAILURE(
        isolate, NewTypeError(MessageTemplate::kCalledOnNonObject,
                              isolate->factory()->NewStringFromAsciiChecked(
                                  "Reflect.set")));
  }

  Handle<Name> name;
  ASSIGN_RETURN_FAILURE_ON_EXCEPTION(isolate, name,
                                     Object::ToName(isolate, key));

  PropertyKey lookup_key(isolate, name);
  LookupIterator it(isolate, receiver, lookup_key,
                    Handle<JSReceiver>::cast(target));
  Maybe<bool> result = Object::SetSuperProperty(
      &it, value, StoreOrigin::kMaybeKeyed, Just(ShouldThrow::kDontThrow));
  MAYBE_RETURN(result, ReadOnlyRoots(isolate).exception());
  return *isolate->factory()->ToBoolean(result.FromJust());
}

}  // namespace internal
}  // namespace v8
