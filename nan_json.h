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
    v8::Local<v8::Value> globalJSON =
      Nan::Get(
        Nan::GetCurrentContext()->Global(),
        Nan::New("JSON").ToLocalChecked()
      ).ToLocalChecked();

    if (globalJSON->IsObject()) {
#if NAN_JSON_H_NEED_PARSE
      v8::Local<v8::Value> parseMethod = Nan::Get(
        globalJSON->ToObject(), Nan::New("parse").ToLocalChecked()
      ).ToLocalChecked();

      if (!parseMethod.IsEmpty() && parseMethod->IsFunction()) {
        m_cb_parse.Reset(v8::Local<v8::Function>::Cast(parseMethod));
      }
#endif

#if NAN_JSON_H_NEED_STRINGIFY
      v8::Local<v8::Value> stringifyMethod = Nan::Get(
        globalJSON->ToObject(), Nan::New("stringify").ToLocalChecked()
      ).ToLocalChecked();

      if (!stringifyMethod.IsEmpty() && stringifyMethod->IsFunction()) {
        m_cb_stringify.Reset(v8::Local<v8::Function>::Cast(stringifyMethod));
      }
#endif
    }
#endif
  }

  ~JSON() {
#if NAN_JSON_H_NEED_PARSE
    m_cb_parse.Reset();
#endif
#if NAN_JSON_H_NEED_STRINGIFY
    m_cb_stringify.Reset();
#endif
  }

  inline
  Nan::MaybeLocal<v8::Value> Parse(v8::Local<v8::String> jsonString) {
    Nan::HandleScope scope;
#if NAN_JSON_H_NEED_PARSE
    return parse(jsonString);
#else
#if (NODE_MAJOR_VERSION >= 7)
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
#if NAN_JSON_H_NEED_PARSE
  Nan::Callback m_cb_parse;
#endif
#if NAN_JSON_H_NEED_STRINGIFY
  Nan::Callback m_cb_stringify;
#endif

#if NAN_JSON_H_NEED_PARSE
  inline v8::Local<v8::Value> parse(v8::Local<v8::Value> arg) {
    return m_cb_parse.Call(1, &arg);
  }
#endif

#if NAN_JSON_H_NEED_STRINGIFY
  inline v8::Local<v8::Value> stringify(v8::Local<v8::Value> arg) {
    return m_cb_stringify.Call(1, &arg);
  }

  inline v8::Local<v8::Value> stringify(v8::Local<v8::Value> arg,
    v8::Local<v8::String> gap) {
    v8::Local<v8::Value> argv[] = {
      arg,
      Nan::Null(),
      gap
    };
    return m_cb_stringify.Call(3, argv);
  }
#endif
};

#endif /* NAN_JSON_H_ */
