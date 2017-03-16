/*********************************************************************
 * NAN - Native Abstractions for Node.js
 *
 * Copyright (c) 2017 NAN contributors
 *
 * MIT License <https://github.com/nodejs/nan/blob/master/LICENSE.md>
 ********************************************************************/

#ifndef NAN_JSON_H_
#define NAN_JSON_H_

#if ((NODE_MAJOR_VERSION == 0) && (NODE_MINOR_VERSION < 12))
#define NAN_JSON_H_NEED_PARSE 1
#else
#define NAN_JSON_H_NEED_PARSE 0
#endif

#if (NODE_MAJOR_VERSION >= 7)
#define NAN_JSON_H_NEED_STRINGIFY 0
#else
#define NAN_JSON_H_NEED_STRINGIFY 1
#endif

class JSON {
 public:
  JSON() {
#if (NAN_JSON_H_NEED_PARSE + NAN_JSON_H_NEED_STRINGIFY)
    Nan::HandleScope scope;
    v8::Local<v8::Object> obj = Nan::New<v8::Object>();
    m_persistent.Reset(obj);

    v8::Local<v8::Value> objectName = Nan::New("JSON").ToLocalChecked();
    v8::MaybeLocal<v8::Value> maybeGlobalJSON =
      Nan::Get(Nan::GetCurrentContext()->Global(), objectName);

    if (!maybeGlobalJSON.IsEmpty()) {
      v8::Local<v8::Value> globalJSON = maybeGlobalJSON.ToLocalChecked();

      if (globalJSON->IsObject()) {
        obj->Set(objectName, globalJSON);
      }
    }
#endif
  }

  ~JSON() {
#if (NAN_JSON_H_NEED_PARSE + NAN_JSON_H_NEED_STRINGIFY)
    m_persistent.Reset();
#endif
  }

  inline
  Nan::MaybeLocal<v8::Value> Parse(v8::Local<v8::String> jsonString) {
#if NAN_JSON_H_NEED_PARSE
    Nan::HandleScope scope;
    return parse(jsonString);
#else
#if (NODE_MAJOR_VERSION >= 7)
    Nan::HandleScope scope;
    return v8::JSON::Parse(Nan::GetCurrentContext(), jsonString);
#else
    return v8::JSON::Parse(jsonString);
#endif
#endif
  }

  inline
  Nan::MaybeLocal<v8::String> Stringify(v8::Local<v8::Object> jsonObject) {
    Nan::HandleScope scope;
#if NAN_JSON_H_NEED_STRINGIFY
    return Nan::To<v8::String>(stringify(jsonObject));
#else
    return v8::JSON::Stringify(Nan::GetCurrentContext(), jsonObject);
#endif
  }

  inline
  Nan::MaybeLocal<v8::String> Stringify(v8::Local<v8::Object> jsonObject,
    v8::Local<v8::String> gap) {
    Nan::HandleScope scope;
#if NAN_JSON_H_NEED_STRINGIFY
    return Nan::To<v8::String>(stringify(jsonObject, gap));
#else
    return v8::JSON::Stringify(Nan::GetCurrentContext(), jsonObject, gap);
#endif
  }

 private:
  NAN_DISALLOW_ASSIGN_COPY_MOVE(JSON)
#if (NAN_JSON_H_NEED_PARSE + NAN_JSON_H_NEED_STRINGIFY)
  Nan::Persistent<v8::Object> m_persistent;

  v8::Local<v8::Value> getMethod(v8::Local<v8::Object> obj,
    const char *method) {
    v8::Local<v8::Object> persistent = Nan::New(m_persistent);
    v8::Local<v8::String> methodName = Nan::New(method).ToLocalChecked();

    if (!persistent->Has(methodName)) {
      v8::MaybeLocal<v8::Value> maybeThisMethod =
        Nan::Get(obj, methodName);

      if (maybeThisMethod.IsEmpty()) {
        return Nan::Undefined();
      }

      v8::Local<v8::Value> thisMethod = maybeThisMethod.ToLocalChecked();

      if (!thisMethod->IsFunction()) {
        return Nan::Undefined();
      }

      persistent->Set(methodName, thisMethod);

      return thisMethod;
    }
    return Nan::Get(persistent, methodName).ToLocalChecked();
  }

  v8::Local<v8::Value> Call(const char *method,
    int argc, v8::Local<v8::Value> *argv) {
    v8::MaybeLocal<v8::Value> maybeGlobalJSON = Nan::Get(
      Nan::New(m_persistent), Nan::New("JSON").ToLocalChecked()
    );

    if (maybeGlobalJSON.IsEmpty()) {
      return Nan::Undefined();
    }

    v8::Local<v8::Value> globalJSON = maybeGlobalJSON.ToLocalChecked();

    if (!globalJSON->IsObject()) {
      return Nan::Undefined();
    }

    v8::Local<v8::Object> json =
      Nan::To<v8::Object>(globalJSON).ToLocalChecked();

    v8::Local<v8::Value> thisMethod = getMethod(json, method);

    if (thisMethod.IsEmpty() || !thisMethod->IsFunction()) {
      return Nan::Undefined();
    }

    v8::Local<v8::Function> methodFunction =
      v8::Local<v8::Function>::Cast(thisMethod);

    return methodFunction->Call(json, argc, argv);
  }
#endif

#if NAN_JSON_H_NEED_PARSE
  inline v8::Local<v8::Value> parse(v8::Local<v8::Value> arg) {
    return Call("parse", 1, &arg);
  }
#endif

#if NAN_JSON_H_NEED_STRINGIFY
  inline v8::Local<v8::Value> stringify(v8::Local<v8::Value> arg) {
    return Call("stringify", 1, &arg);
  }

  inline v8::Local<v8::Value> stringify(v8::Local<v8::Value> arg,
    v8::Local<v8::String> gap) {
    v8::Local<v8::Value> argv[] = {
      arg,
      Nan::Null(),
      gap
    };
    return Call("stringify", 3, argv);
  }
#endif
};

#endif /* NAN_JSON_H_ */
