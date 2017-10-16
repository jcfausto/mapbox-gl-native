#include "node_conversion.hpp"
#include <mbgl/style/conversion/geojson.hpp>

namespace mbgl {
namespace style {
namespace conversion {

template<> bool ValueTraits<v8::Local<v8::Value>>::isUndefined(const v8::Local<v8::Value>& value) {
    Nan::HandleScope scope;
    
    return value->IsUndefined() || value->IsNull();
}

template<> bool ValueTraits<v8::Local<v8::Value>>::isArray(const v8::Local<v8::Value>& value) {
    Nan::HandleScope scope;
    
    return value->IsArray();
}

template<> std::size_t ValueTraits<v8::Local<v8::Value>>::arrayLength(const v8::Local<v8::Value>& value) {
    Nan::HandleScope scope;
    // const_cast because v8::Local<T>::As is not marked const until node v8.0
    v8::Local<v8::Array> array = const_cast<v8::Local<v8::Value>&>(value).As<v8::Array>();
    return array->Length();
}

template<> v8::Local<v8::Value> ValueTraits<v8::Local<v8::Value>>::arrayMember(const v8::Local<v8::Value>& value, std::size_t i) {
    Nan::EscapableHandleScope scope;
    // const_cast because v8::Local<T>::As is not marked const until node v8.0
    v8::Local<v8::Array> array = const_cast<v8::Local<v8::Value>&>(value).As<v8::Array>();
    return scope.Escape(Nan::Get(array, i).ToLocalChecked());
}

template<> bool ValueTraits<v8::Local<v8::Value>>::isObject(const v8::Local<v8::Value>& value) {
    Nan::HandleScope scope;
    
    return value->IsObject() && !value->IsArray();
}

template<> optional<v8::Local<v8::Value>> ValueTraits<v8::Local<v8::Value>>::objectMember(const v8::Local<v8::Value>& value, const char * name) {
    Nan::EscapableHandleScope scope;
    
    if (!Nan::Has(Nan::To<v8::Object>(value).ToLocalChecked(), Nan::New(name).ToLocalChecked()).FromJust()) {
        return {};
    }
    Nan::MaybeLocal<v8::Value> result = Nan::Get(Nan::To<v8::Object>(value).ToLocalChecked(), Nan::New(name).ToLocalChecked());
    if (result.IsEmpty()) {
        return {};
    }
    return {scope.Escape(result.ToLocalChecked())};
}

template<> optional<Error> ValueTraits<v8::Local<v8::Value>>::eachMember(const v8::Local<v8::Value>& value, const std::function<optional<Error> (const std::string&, const v8::Local<v8::Value>&)>& fn) {
    Nan::HandleScope scope;
    
    v8::Local<v8::Array> names = Nan::GetOwnPropertyNames(Nan::To<v8::Object>(value).ToLocalChecked()).ToLocalChecked();
    for (uint32_t i = 0; i < names->Length(); ++i) {
        v8::Local<v8::Value> k = Nan::Get(names, i).ToLocalChecked();
        v8::Local<v8::Value> v = Nan::Get(Nan::To<v8::Object>(value).ToLocalChecked(), k).ToLocalChecked();
        optional<Error> result = fn(*Nan::Utf8String(k), v);
        if (result) {
            return result;
        }
    }
    return {};
}

template<> optional<bool> ValueTraits<v8::Local<v8::Value>>::toBool(const v8::Local<v8::Value>& value) {
    Nan::HandleScope scope;
    
    if (!value->IsBoolean()) {
        return {};
    }
    return value->BooleanValue();
}

template<> optional<float> ValueTraits<v8::Local<v8::Value>>::toNumber(const v8::Local<v8::Value>& value) {
    Nan::HandleScope scope;
    
    if (!value->IsNumber()) {
        return {};
    }
    return value->NumberValue();
}

template<> optional<double> ValueTraits<v8::Local<v8::Value>>::toDouble(const v8::Local<v8::Value>& value) {
    Nan::HandleScope scope;
    
    if (!value->IsNumber()) {
        return {};
    }
    return value->NumberValue();
}

template<> optional<std::string> ValueTraits<v8::Local<v8::Value>>::toString(const v8::Local<v8::Value>& value) {
    Nan::HandleScope scope;
    
    if (!value->IsString()) {
        return {};
    }
    return std::string(*Nan::Utf8String(value));
}

template<> optional<mbgl::Value> ValueTraits<v8::Local<v8::Value>>::toValue(const v8::Local<v8::Value>& value) {
    
    if (value->IsFalse()) {
        return { false };
    } else if (value->IsTrue()) {
        return { true };
    } else if (value->IsString()) {
        return { std::string(*Nan::Utf8String(value)) };
    } else if (value->IsUint32()) {
        return { std::uint64_t(value->Uint32Value()) };
    } else if (value->IsInt32()) {
        return { std::int64_t(value->Int32Value()) };
    } else if (value->IsNumber()) {
        return { value->NumberValue() };
    } else {
        return {};
    }
}

template<> optional<GeoJSON> ValueTraits<v8::Local<v8::Value>>::toGeoJSON(const v8::Local<v8::Value>& value, Error& error) {
    try {
        Nan::JSON JSON;
        
        std::string string = *Nan::Utf8String(JSON.Stringify(value->ToObject()).ToLocalChecked());
        return parseGeoJSON(string, error);
    } catch (const std::exception& ex) {
        error = { ex.what() };
        return {};
    }
}

} // namespace conversion
} // namespace style
} // namespace mbgl
