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
namespace czlab::spi {
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum TokenType {

  // keywords
  T_PROGRAM = 1000,
  T_INT_DIV,
  T_VAR,
  T_STR,
  T_INT,
  T_REAL,
  T_PROCEDURE,
  T_BEGIN,
  T_END,

  // symbols
  T_ASSIGN

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SToken : public d::Token {

  static d::DslToken make(int t, cstdstr& s, d::Mark i) {
    return WRAP_TKN(new SToken(t,s,i));
  }

  static d::DslToken make(int t, Tchar c, d::Mark i) {
    return WRAP_TKN(new SToken(t,c,i));
  }

  virtual stdstr getStr() const;
  virtual ~SToken() {}

  void setLiteral(double d) { number.r=d;}
  void setLiteral(int n) { number.n=n;}
  void setLiteral(llong n) { number.n=n;}

  private:

  SToken(int t, cstdstr& s, d::Mark m) : d::Token(t,s,m) {}
  SToken(int t, Tchar c, d::Mark m) : d::Token(t,c,m) {}

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lexer : public d::IScanner {

  virtual bool isKeyword(cstdstr&) const;
  virtual d::DslToken getNextToken();
  virtual d::DslToken number();
  virtual d::DslToken id();
  virtual d::DslToken string();
  virtual d::DslToken skipComment();

  Lexer(const Tchar* src);
  virtual ~Lexer() {}

  d::Context& ctx() { return _ctx; }

  private:

  d::Context _ctx;
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

