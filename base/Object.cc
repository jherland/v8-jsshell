/* -*- mode: c++ -*- */
/*

  Copyright 2013 Jens Lindström

  Licensed under the Apache License, Version 2.0 (the "License"); you may not
  use this file except in compliance with the License.  You may obtain a copy of
  the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
  License for the specific language governing permissions and limitations under
  the License.

*/

#include "Base.h"

namespace {

v8::Handle<v8::String> ToString(std::string value) {
  return base::String::New(value.c_str(), value.length());
}

}

namespace base {

Object::Persistent::Persistent() {
}

Object::Persistent::Persistent(const Persistent& other)
    : handle_(v8::Isolate::GetCurrent(), other.handle_) {
}

Object::Persistent::Persistent(const Object& object)
    : handle_(v8::Isolate::GetCurrent(), object.handle_) {
}

Object::Persistent::~Persistent() {
  Release();
}

Object::Persistent& Object::Persistent::operator= (const Persistent& other) {
  handle_.Reset(v8::Isolate::GetCurrent(), other.handle_);
  return *this;
}

Object::Persistent& Object::Persistent::operator= (const Object& object) {
  handle_.Reset(v8::Isolate::GetCurrent(), object.handle_);
  return *this;
}

bool Object::Persistent::IsEmpty() const {
  return handle_.IsEmpty();
}

Object Object::Persistent::GetObject() const {
  return handle_;
}

void Object::Persistent::Release() {
  handle_.Reset();
}

Object::Object() {
}

Object Object::Create() {
  return v8::Object::New(v8::Isolate::GetCurrent());
}

bool Object::IsEmpty() {
  return handle_.IsEmpty();
}

bool Object::IsCallable() {
  return handle_->IsCallable();
}

bool Object::HasProperty(std::string name) {
  return handle_->Has(ToString(name));
}

bool Object::HasProperty(std::uint32_t index) {
  return handle_->Has(index);
}

namespace {

v8::PropertyAttribute ConvertPropertyFlags(unsigned flags) {
  unsigned result = 0;

  if ((flags & PropertyFlags::kWritable) == 0)
    result |= v8::ReadOnly;
  if ((flags & PropertyFlags::kEnumerable) == 0)
    result |= v8::DontEnum;
  if ((flags & PropertyFlags::kConfigurable) == 0)
    result |= v8::DontDelete;

  return static_cast<v8::PropertyAttribute>(result);
}

unsigned ConvertPropertyAttributes(v8::PropertyAttribute attributes) {
  unsigned result = 0;

  if ((attributes & v8::ReadOnly) == 0)
    result |= PropertyFlags::kWritable;
  if ((attributes & v8::DontEnum) == 0)
    result |= PropertyFlags::kEnumerable;
  if ((attributes & v8::DontDelete) == 0)
    result |= PropertyFlags::kConfigurable;

  return result;
}

}

unsigned Object::GetPropertyFlags(std::string name) {
  if (HasProperty(name)) {
    v8::Handle<v8::Value> key(base::String::New(name.c_str(), name.length()));
    v8::PropertyAttribute attributes = handle_->GetPropertyAttributes(key);
    return ConvertPropertyAttributes(attributes);
  } else {
    return PropertyFlags::kNotFound;
  }
}

unsigned Object::GetPropertyFlags(std::uint32_t name) {
  if (HasProperty(name)) {
    v8::Handle<v8::Value> key(v8::Integer::New(v8::Isolate::GetCurrent(), name));
    v8::PropertyAttribute attributes = handle_->GetPropertyAttributes(key);
    return ConvertPropertyAttributes(attributes);
  } else {
    return PropertyFlags::kNotFound;
  }
}

Variant Object::Get(std::string name) {
  return handle_->Get(ToString(name));
}

Variant Object::Get(std::uint32_t index) {
  return handle_->Get(index);
}

void Object::Put(std::string name, const Variant& value, unsigned flags) {
  handle_->Set(ToString(name), value.handle_, ConvertPropertyFlags(flags));
}

void Object::Put(std::uint32_t index, const Variant& value, unsigned flags) {
  handle_->Set(v8::Integer::NewFromUnsigned(index), value.handle_,
               ConvertPropertyFlags(flags));
}

namespace {

std::vector<std::string> ArrayToVector(v8::Local<v8::Array> array) {
  std::vector<std::string> result;

  for (std::uint32_t index = 0, length = array->Length();
       index < length;
       ++index)
    result.push_back(base::Variant(array->Get(index)).AsString());

  return result;
}

}

std::vector<std::string> Object::GetPropertyNames() {
  return ArrayToVector(handle_->GetPropertyNames());
}

std::vector<std::string> Object::GetOwnPropertyNames() {
  return ArrayToVector(handle_->GetOwnPropertyNames());
}

bool Object::HasHidden(std::string name) {
  return !handle_->GetHiddenValue(ToString(name)).IsEmpty();
}

Variant Object::GetHidden(std::string name) {
  return handle_->GetHiddenValue(ToString(name));
}

void Object::PutHidden(std::string name, const Variant& value) {
  handle_->SetHiddenValue(ToString(name), value.handle_);
}

Variant Object::Call(std::string method,
                     const std::vector<Variant>& arguments) {
  if (!HasProperty(method))
    throw TypeError("no such method");

  Function function(Get(method).AsObject());

  return function.Call(*this, arguments);
}

}
