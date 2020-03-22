#pragma once
/* Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright © 2013-2020, Kenneth Leung. All rights reserved. */


#include "../dsl/dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::kirby {
namespace a= czlab::aeon;
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::vector<d::NumberSlot> NumberSlots;
typedef std::vector<d::DslValue> VVec;
typedef VVec::iterator VIter;
typedef std::pair<d::DslValue,d::DslValue> VPair;
struct VSlice {
  VSlice(VIter b, VIter e) : begin(b), end(e) {}
  VSlice(VVec& v) { begin=v.begin(); end=v.end(); }
  int size() { return std::distance(begin,end); }
  VIter begin;
  VIter end;
};
struct Lisper;
typedef d::DslValue (Invoker)(Lisper*, VSlice);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define HASH_VAL(k,v) std::pair<d::DslValue,d::DslValue>(k,v)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoSuchVar : public a::Exception {
  NoSuchVar(const stdstr& m) : a::Exception (m) {}
};
struct BadArg : public a::Exception {
  BadArg(const stdstr& m) : a::Exception(m) {}
};
struct DivByZero : public a::Exception {
  DivByZero(const stdstr& m) : a::Exception(m) {}
};
struct BadArity : public a::Exception {
  BadArity(int wanted, int got)
    : a::Exception("Expected " + std::to_string(wanted) + " args, got " + std::to_string(got) + ".\n") {}
  BadArity(const stdstr& m) : a::Exception(m) {}
};
struct BadEval : public a::Exception {
  BadEval(const stdstr& m) : a::Exception(m) {}
};
struct IndexOOB : public a::Exception {
  IndexOOB(const stdstr& m) : a::Exception(m) {}
};
struct Unsupported : public a::Exception {
  Unsupported(const stdstr& m) : a::Exception(m) {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define float_value(n) czlab::dsl::DslValue(new LFloat(n))
#define int_value(n) czlab::dsl::DslValue(new LInt(n))
#define empty_list() czlab::dsl::DslValue(new LList())
#define char_value(c) czlab::dsl::DslValue(new LChar(c))
#define atom_value(a) czlab::dsl::DslValue(new LAtom(a))
#define false_value() czlab::dsl::DslValue(new LFalse())
#define true_value() czlab::dsl::DslValue(new LTrue())
#define nil_value() czlab::dsl::DslValue(new LNil())
#define vector_value(v) czlab::dsl::DslValue(new LVec(v))
#define list_value(v) czlab::dsl::DslValue(new LList(v))
#define map_value(v) czlab::dsl::DslValue(new LHash(v))
#define keyword_value(s) czlab::dsl::DslValue(new LKeyword(s))
#define string_value(s) czlab::dsl::DslValue(new LString(s))
#define symbol_value(s) czlab::dsl::DslValue(new LSymbol(s))
#define bool_value(b) ((b) ? true_value() : false_value())
#define fn_value(n, f) czlab::dsl::DslValue(new LNative(n, f))
#define macro_value(n,p,b,e) czlab::dsl::DslValue(new LMacro(n,p,b,e))
#define lambda_value(n,p,b,e) czlab::dsl::DslValue(new LLambda(n,p,b,e))

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lisper {
  d::DslValue macroExpand(d::DslValue ast, d::DslFrame env);
  d::DslValue syntaxQuote(d::DslValue ast, d::DslFrame env);
  d::DslValue evalAst(d::DslValue ast, d::DslFrame env);
  d::DslValue EVAL(d::DslValue ast, d::DslFrame env);
  std::pair<int,d::DslValue> READ(const stdstr& s) ;
  stdstr PRINT(const d::DslValue& v);
  Lisper() { seed=0; }
  ~Lisper() {}
  private:
  int seed;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LValue : public d::Data {

  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual bool equals(const d::Data*) const;
  d::DslValue meta() const;

  virtual bool keyable() const { return false; }
  virtual bool truthy() const { return true; }
  virtual bool seqable() const { return false; }

  virtual d::DslValue eval(Lisper*, d::DslFrame);
  virtual ~LValue() {}
  LValue() {}

  protected:

  virtual bool eq(const d::Data*) const = 0;

  LValue(const d::DslValue& m) : metaObj(m) {}

  d::DslValue metaObj;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSeqable {

  virtual bool contains(const d::DslValue&) const=0;
  virtual d::DslValue first() const = 0;
  virtual d::DslValue rest() const = 0;
  virtual d::DslValue seq() const = 0;
  virtual d::DslValue nth(int) const = 0;
  virtual bool isEmpty() const = 0;
  virtual int count() const = 0 ;

  protected:
  ~LSeqable() {}
  LSeqable() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFalse : public LValue {

  virtual d::DslValue withMeta(d::DslValue&) const;

  virtual bool truthy() const { return false; }

  virtual stdstr toString(bool pretty) const;
  virtual ~LFalse() {}
  LFalse(const LFalse&, const d::DslValue&);
  LFalse() {}

  protected:

  virtual bool eq(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LTrue : public LValue {

  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual ~LTrue() {}
  LTrue(const LTrue&, const d::DslValue&);
  LTrue() {}

  protected:
  virtual bool eq(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNil : public LValue {

  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual bool truthy() const { return false; }

  virtual stdstr toString(bool pretty) const;
  virtual ~LNil() {}
  LNil() {}
  LNil(const LNil&, const d::DslValue&);

  protected:
  virtual bool eq(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LChar : public LValue {

  virtual d::DslValue withMeta(d::DslValue&) const;

  virtual stdstr toString(bool p) const;
  virtual ~LChar() {}

  LChar(const LChar&, const d::DslValue&);
  LChar(Tchar c) { value = c; }
  Tchar impl() { return value; }

  protected:
  virtual bool eq(const d::Data* ) const;
  Tchar value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LAtom : public LValue {

  virtual d::DslValue withMeta(d::DslValue&) const;

  LAtom(const LAtom&, const d::DslValue&);
  LAtom(d::DslValue);
  LAtom() { value=nil_value();}
  virtual stdstr toString(bool p) const;
  virtual ~LAtom() {}

  d::DslValue deref() const { return value; }
  d::DslValue reset(d::DslValue x) { return (value = x); }

  protected:
  virtual bool eq(const d::Data* ) const;
  d::DslValue value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFloat : public LValue {
  virtual d::DslValue withMeta(d::DslValue& m) const;
  virtual d::NumberSlot number() const;
  LFloat(const LFloat& rhs, const d::DslValue& m);
  LFloat(double n) : value(n) {}
  virtual stdstr toString(bool pretty) const;
  double impl() const { return value; };
  virtual ~LFloat() {}

  protected:

  virtual bool eq(const d::Data* rhs) const;
  double value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LInt : public LValue {
  virtual d::DslValue withMeta(d::DslValue& m) const;
  virtual d::NumberSlot number() const;
  LInt(const LInt& rhs, const d::DslValue& m);
  LInt(llong n) : value(n) {}
  virtual stdstr toString(bool pretty) const;
  llong impl() const { return value; };
  virtual ~LInt() {}

  protected:
  virtual bool eq(const d::Data* rhs) const;
  llong value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LString : public LValue, public LSeqable {
  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual bool keyable() const { return true; }
  virtual bool seqable() const { return true; }
  LString(const LString&, const d::DslValue&);
  LString(const stdstr&);
  stdstr encoded() const;
  stdstr impl() const { return value; }

  virtual bool contains(const d::DslValue&) const;
  virtual d::DslValue first() const;
  virtual d::DslValue rest() const;
  virtual d::DslValue seq() const;
  virtual d::DslValue nth(int) const;
  virtual bool isEmpty() const;
  virtual int count() const;

  protected:
  virtual bool eq(const d::Data*) const;
  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LKeyword : public LValue {
  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual bool keyable() const { return true; }
  stdstr impl() const { return value; }
  LKeyword(const stdstr& s);
  LKeyword(const LKeyword&, const d::DslValue&);
  protected:
  virtual bool eq(const d::Data*) const;
  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSymbol : public LValue {
  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual d::DslValue eval(Lisper*, d::DslFrame);
  LSymbol(const LSymbol& rhs, const d::DslValue&);
  LSymbol(const stdstr& s);
  stdstr impl() const { return value; }

  protected:
  virtual bool eq(const d::Data*) const;
  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSequential : public LValue, public LSeqable {

  void evalEach(Lisper*, d::DslFrame, VVec&) const;
  virtual stdstr toString(bool pretty) const;
  virtual ~LSequential() {}
  virtual bool seqable() const { return true; }

  virtual d::DslValue conj(VSlice) const = 0;

  virtual bool contains(const d::DslValue&) const;
  virtual d::DslValue first() const;
  virtual d::DslValue rest() const;
  virtual d::DslValue seq() const;
  virtual d::DslValue nth(int) const;
  virtual bool isEmpty() const;
  virtual int count() const;

  protected:

  virtual bool eq(const d::Data*) const;

  LSequential(const LSequential& rhs, const d::DslValue&);
  LSequential(VSlice chunk);
  LSequential(VVec&);
  LSequential() {}

  VVec values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LList : public LSequential {

  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;

  virtual d::DslValue eval(Lisper*, d::DslFrame);

  LList(const LList& rhs, const d::DslValue&);
  LList(VVec&);
  LList(VSlice);
  LList() {}
  virtual d::DslValue conj(VSlice) const;

  virtual ~LList() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LVec : public LSequential {

  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;

  virtual d::DslValue eval(Lisper*, d::DslFrame);

  LVec(const LVec& rhs, const d::DslValue& m);
  LVec(VSlice);
  LVec(VVec&);
  LVec() {}
  virtual d::DslValue conj(VSlice) const;

  virtual ~LVec() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LHash : public LValue, public LSeqable {

  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual bool seqable() const { return true; }

  d::DslValue get(d::DslValue&) const;

  virtual bool contains(const d::DslValue&) const;
  virtual d::DslValue first() const;
  virtual d::DslValue rest() const;
  virtual d::DslValue seq() const;
  virtual d::DslValue nth(int) const;
  virtual bool isEmpty() const;
  virtual int count() const;


  LHash(const LHash& rhs, const d::DslValue&);
  LHash(const std::map<stdstr, VPair>&);
  LHash(VSlice);
  LHash();

  d::DslValue eval(Lisper*, d::DslFrame);

  d::DslValue dissoc(VSlice) const;
  d::DslValue assoc(VSlice) const;

  d::DslValue keys() const;
  d::DslValue vals() const;

  protected:

  virtual bool eq(const d::Data*) const;
  std::map<stdstr,VPair> values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFunction : public LValue {

  virtual d::DslValue invoke(Lisper*, VSlice) = 0;
  virtual d::DslValue invoke(Lisper*) = 0;

  stdstr name() { return _name; }
  virtual ~LFunction() {}

  protected:
  stdstr _name;
  LFunction(const stdstr& n) : _name(n) {}
  LFunction(const d::DslValue& m) : LValue(m) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LLambda : public LFunction {

  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual ~LLambda() {}

  virtual d::DslValue invoke(Lisper*,VSlice);
  virtual d::DslValue invoke(Lisper*);

  LLambda(const stdstr& name, const StrVec& args, d::DslValue body, d::DslFrame);
  LLambda(const StrVec& args, d::DslValue body, d::DslFrame);
  LLambda(const LLambda&, const d::DslValue&);
  LLambda() : LFunction("") {}

  d::DslFrame bindContext(VSlice args);

  d::DslValue body;
  StrVec params;
  d::DslFrame env;

  protected:
  virtual bool eq(const d::Data* rhs) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LMacro : public LLambda {
  LMacro(const stdstr& name, const StrVec& args, d::DslValue body, d::DslFrame);
  LMacro(const StrVec& args, d::DslValue body, d::DslFrame);
  LMacro(const LMacro&, const d::DslValue&);
  LMacro() {}
  virtual ~LMacro() {}
  virtual bool isMacro() const { return true; }
  stdstr toString(bool pretty) const;
  virtual d::DslValue withMeta(d::DslValue&) const;
  protected:
  virtual bool eq(const d::Data* rhs) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNative : public LFunction {

  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual ~LNative() {}

  virtual d::DslValue invoke(Lisper*, VSlice);
  virtual d::DslValue invoke(Lisper*);

  LNative(const LNative&, const d::DslValue&);
  LNative(const stdstr& name, Invoker*);
  LNative() : LFunction("") { S_NIL(fn); }

  protected:

  virtual bool eq(const d::Data*) const;
  Invoker* fn;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEqual(int wanted, int got, const stdstr& fn);
int preMin(int min, int got, const stdstr& fn);
int preNonZero(int c, const stdstr& fn);
int preEven(int c, const stdstr& fn);
d::DslValue expected(const stdstr&, d::DslValue);
void appendAll(VSlice, int, VVec&);
void appendAll(LSeqable*, VVec&);
void appendAll(LSeqable*, int, VVec&);
bool truthy(d::DslValue);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool cast_numeric(VSlice, NumberSlots&);
LAtom* cast_atom(d::DslValue, int panic=0);
LNil* cast_nil(d::DslValue, int panic=0);
LHash* cast_map(d::DslValue, int panic=0);
LFloat* cast_float(d::DslValue, int panic=0);
LInt* cast_int(d::DslValue, int panic=0);
LChar* cast_char(d::DslValue, int panic=0);
LSymbol* cast_symbol(d::DslValue, int panic=0);
LList* cast_list(d::DslValue, int panic=0);
LVec* cast_vec(d::DslValue, int panic=0);
LSequential* cast_seq(d::DslValue, int panic=0);
LSeqable* cast_seqable(d::DslValue, int panic=0);
LLambda* cast_lambda(d::DslValue, int panic=0);
LMacro* cast_macro(d::DslValue, int panic=0);
LNative* cast_native(d::DslValue, int panic=0);
LString* cast_string(d::DslValue, int panic=0);
LKeyword* cast_keyword(d::DslValue, int panic=0);
LFunction* cast_function(d::DslValue, int panic=0);




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF
