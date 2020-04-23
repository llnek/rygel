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

#include "lexer.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace d = czlab::dsl;
namespace a = czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SValue : public d::Data {

  virtual int compare(d::DValue rhs) const { ASSERT1(rhs); return cmp(rhs); }
  virtual bool equals(d::DValue rhs) const { ASSERT1(rhs); return eq(rhs); }
  virtual stdstr pr_str(bool p = 0) const = 0;
  virtual ~SValue() {}

  protected:

  SValue() {}

  virtual bool eq(d::DValue) const=0;
  virtual int cmp(d::DValue) const=0;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SNumber : public SValue {

  static d::DValue make(double d) {
    return WRAP_VAL(SNumber,d); }
  static d::DValue make(int n) { return WRAP_VAL( SNumber,n); }
  static d::DValue make(llong n) { return WRAP_VAL( SNumber,n); }
  static d::DValue make() { return WRAP_VAL( SNumber); }

  double getFloat() const { return isInt() ? (double)num.n : num.r; }
  llong getInt() const { return isInt() ? num.n : (llong) num.r; }

  void setFloat(double d) { num.r=d; }
  void setInt(llong n) { num.n=n; }

  bool isInt() const { return type== d::T_INTEGER;}

  bool isZero() const {
    return type==d::T_INTEGER ? getInt()==0 : a::fuzzy_zero(getFloat()); }

  bool isNeg() const {
    return type==d::T_INTEGER ? getInt() < 0 : getFloat() < 0.0; }

  bool isPos() const {
    return type==d::T_INTEGER ? getInt() > 0 : getFloat() > 0.0; }

  virtual stdstr pr_str(bool p=0) const {
    return isInt() ? N_STR(getInt()) : N_STR(getFloat());
  }

  SNumber() : type(d::T_INTEGER) { num.n=0; }

  protected:

  explicit SNumber(double d) : type(d::T_REAL) { num.r=d; }
  explicit SNumber(int n) : type(d::T_INTEGER) { num.n=n; }
  SNumber(llong n) : type(d::T_INTEGER) { num.n=n; }

  bool match(const SNumber* rhs) const;

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  int type;
  union {
    llong n; double r; } num;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SStr : public SValue {

  virtual stdstr pr_str(bool p=0) const { return value; }

  static d::DValue make(cstdstr& s) {
    return WRAP_VAL( SStr,s);
  }

  static d::DValue make(const Tchar* s) {
    return WRAP_VAL( SStr,s);
  }

  SStr() {}

  stdstr value;

  private:

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  SStr(cstdstr& s) : value(s) {}
  SStr(const char* s) : value(s) {}

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr& m, d::DValue v);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SNumber* cast_number(d::DValue, int panic=0);
SStr* cast_string(d::DValue, int panic=0);





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

