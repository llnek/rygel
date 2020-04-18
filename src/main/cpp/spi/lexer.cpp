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
namespace czlab::spi {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// map, keyword-type -> keyword-string
std::map<int, stdstr> TOKENS {
  {T_PROCEDURE, "PROCEDURE"},
  {T_PROGRAM, "PROGRAM"},
  {T_VAR, "VAR"},
  {T_REAL, "REAL"},
  {T_INT, "INTEGER"},
  {T_STR, "STRING"},
  {T_INT_DIV, "DIV"},
  {T_BEGIN, "BEGIN"},
  {T_ASSIGN, ":="},
  {T_END, "END"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// reverse map, keyword-string -> keyword-type
std::map<stdstr,int> KEYWORDS {
  {map__val(TOKENS,T_BEGIN), T_BEGIN},
  {map__val(TOKENS,T_END), T_END},
  {map__val(TOKENS,T_PROGRAM), T_PROGRAM},
  {map__val(TOKENS,T_PROCEDURE), T_PROCEDURE},
  {map__val(TOKENS,T_VAR), T_VAR},
  {map__val(TOKENS,T_STR), T_STR},
  {map__val(TOKENS,T_INT), T_INT},
  {map__val(TOKENS,T_REAL), T_REAL},
  {map__val(TOKENS,T_INT_DIV), T_INT_DIV}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int type) {
  return s__contains(TOKENS, type)
         ? map__val(TOKENS,type)
         : ("token=" + N_STR(type));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr SToken::getStr() const {
  return s__contains(TOKENS, type()) ? TOKENS.at(type()) : lexeme;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, Tchar c, d::Mark info) {
  return SToken::make(type, c, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, cstdstr& x, d::Mark info) {
  return SToken::make(type, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, cstdstr& s, d::Mark info, llong n) {
  auto t= SToken::make(type, s, info);
  DCAST(SToken,t)->setLiteral(n);
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, cstdstr& s, d::Mark info, double d) {
  auto t= SToken::make(type,s, info);
  DCAST(SToken,t)->setLiteral(d);
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::Lexer(const Tchar* src) {
  _ctx.len= ::strlen(src);
  _ctx.eof=false;
  _ctx.src=src;
  _ctx.line=0;
  _ctx.col=0;
  _ctx.pos=0;
  _ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lexer::isKeyword(cstdstr& k) const {
  return s__contains(KEYWORDS, k);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::skipComment() {
  auto m= _ctx.mark();
  stdstr out;
  while (!_ctx.eof) {
    auto ch= d::peek(_ctx);
    d::advance(_ctx);
    if (ch == '}')
    break;
    out += ch;
  }
  return token(d::T_COMMENT, out, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::number() {
  auto m= _ctx.mark();
  auto s = C_STR(d::numeric(_ctx));
  return ::strchr(s, '.')
    ? token(d::T_REAL, s, m, ::atof(s))
    : token(d::T_INTEGER, s, m, (llong) ::atol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::string() {
  auto m= _ctx.mark();
  return token(d::T_STRING, d::str(_ctx), m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool filter(Tchar ch, bool first) {
  if (first) {
    return (ch == '_' || ::isalpha(ch));
  } else {
    return (ch == '_' || ::isalpha(ch) || ::isdigit(ch));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::id() {
  auto m= _ctx.mark();
  auto s= d::identifier(_ctx, &filter);
  auto S= a::to_upper(s);
  return !isKeyword(S)
    ? token(d::T_IDENT, s, m)
    : token(KEYWORDS.at(S), S, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::getNextToken() {
  Tchar ch;
  while (!_ctx.eof) {
    ch= d::peek(_ctx);
    if (::isspace(ch)) {
      d::skipWhitespace(_ctx);
    }
    else
    if (::isdigit(ch)) {
      return number();
    }
    else
    if (ch == '"') {
      return string();
    }
    else
    if (ch == '*') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_MULT, ch, m);
    }
    else
    if (ch == '/') {
      auto m= _ctx.mark();
      d::advance( _ctx);
      return token(d::T_DIV, ch, m);
    }
    else
    if (ch == '+') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_PLUS, ch, m);
    }
    else
    if (ch == '-') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_MINUS, ch, m);
    }
    else
    if (ch == '(') {
      auto m= _ctx.mark();
      d::advance( _ctx);
      return token(d::T_LPAREN, ch, m);
    }
    else
    if (ch == ')') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_RPAREN, ch, m);
    }
    else
    if (filter(ch,true)) {
      return id();
    }
    else
    if (ch== ':' && '=' == d::peekAhead(_ctx)) {
      auto m= _ctx.mark();
      d::advance(_ctx,2);
      return token(T_ASSIGN, ":=", m);
    }
    else
    if (ch == '{') {
      d::advance(_ctx);
      skipComment();
    }
    else
    if (ch == ';') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_SEMI, ch, m);
    }
    else
    if (ch == ':') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_COLON, ch, m);
    }
    else
    if (ch == ',') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_COMMA, ch, m);
    }
    else
    if (ch == '.') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_DOT, ch, m);
    }
    else {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_ROGUE, ch, m);
    }
  }

  return token(d::T_EOF, "<EOF>", _ctx.mark());
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

