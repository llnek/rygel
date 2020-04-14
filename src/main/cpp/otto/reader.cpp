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

#include "reader.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::otto {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, stdstr> TOKENS {
  {T_SPLICE_UNQUOTE, "~@"},
  {T_ANONFN, "#("},
  {T_SET, "#{"},
  {T_TRUE,"true"},
  {T_FALSE,"false"},
  {T_NIL,"nil"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<stdstr,int> KEYWORDS {
  {map__val(TOKENS,T_SPLICE_UNQUOTE),T_SPLICE_UNQUOTE},
  {map__val(TOKENS,T_ANONFN),T_ANONFN},
  {map__val(TOKENS,T_SET),T_SET},
  {map__val(TOKENS,T_FALSE),T_FALSE},
  {map__val(TOKENS,T_TRUE),T_TRUE},
  {map__val(TOKENS,T_NIL),T_NIL}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::typeToString(int type) {
  return s__contains(TOKENS,type)
         ? map__val(TOKENS,type)
         : stdstr("token-type=") + std::to_string(type);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const stdstr& s, d::SrcInfo info) : d::AbstractToken(type) {
  this->info = info;
  _impl.txt= s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, Tchar ch, d::SrcInfo info) : d::AbstractToken(type) {
  _impl.txt= stdstr { ch };
  this->info = info;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Token::getLiteralAsReal() const {
  if (type() != d::T_REAL) {
    RAISE(d::SemanticError,
          "Expecting float near %d(%d).\n", info.first, info.second);
  }
  return _impl.num.getFloat();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Token::getLiteralAsInt() const {
  if (type() != d::T_INTEGER) {
    RAISE(d::SemanticError,
          "Expecting int near %d(%d).\n", info.first, info.second);
  }
  return _impl.num.getInt();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::getLiteralAsStr() const {

  if (type() == d::T_IDENT ||
      type() == T_KEYWORD ||
      type() == d::T_STRING) { return _impl.txt; }

  if (! s__contains(TOKENS,type())) {
    RAISE(d::SemanticError,
          "Unexpected token %d near %d(%d).\n",
          type(), info.first, info.second);
  }

  return TOKENS.at(type());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& x, d::SrcInfo info) {
  return Token::make(type, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, Tchar x, d::SrcInfo info) {
  return Token::make(type, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& x, d::SrcInfo info, const stdstr& s) {
  return Token::make(type, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& s, d::SrcInfo info, llong n) {
  auto t= Token::make(type, s, info);
  s__cast(Token,t.get())->impl().num.setInt(n);
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& s, d::SrcInfo info, double d) {
  auto t= Token::make(type, s, info);
  s__cast(Token,t.get())->impl().num.setFloat(d);
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::pr_str() const {
  char buf[1024];
  ::sprintf(buf,
            "Token#{%d, %s}",
            type(), C_STR(_impl.txt));
  return stdstr(buf);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Reader::Reader(const char* src) {
  _ctx.len= ::strlen(src);
  _ctx.eof=false;
  _ctx.src=src;
  _ctx.line=0;
  _ctx.col=0;
  _ctx.pos=0;
  _ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Reader::isKeyword(const stdstr& k) const {
  throw d::Unsupported("not allowed!");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::skipComment() {
  auto m= _ctx.mark();
  stdstr s;
  Tchar c;

  while (!_ctx.eof) {
    d::advance(_ctx);
    c= d::peek(_ctx);
    if (c == '\n') {
      break;
    }
    s += c;
  }

  return token(d::T_COMMENT, s, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::number() {
  auto m= _ctx.mark();
  auto s= C_STR(d::numeric(_ctx));
  return ::strchr(s, '.')
    ? token(d::T_REAL, s, m, ::atof(s))
    : token(d::T_INTEGER, s, m, (llong) ::atol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::string() {
  auto m= _ctx.mark();
  auto s= d::str(_ctx);
  return token(d::T_STRING, s, m, s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool isKeywdChar(Tchar ch)  {
  return ch == '_' || ch == '-' ||
         ch == '/' ||
         ::isalpha(ch) || ::isdigit(ch);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::keywd() {
  auto m= _ctx.mark();
  stdstr res;
  d::advance(_ctx); // skip the colon
  while (!_ctx.eof) {
    auto ch=peek(_ctx);
    if (isKeywdChar(ch)) {
      res += ch;
      advance(_ctx);
    } else {
      break;
    }
  }
  return token(T_KEYWORD, res, m, res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::id() {
  static stdstr bad("{}[]()'\"\\`@~^,.;");
  auto m= _ctx.mark();
  stdstr res;
  while (!_ctx.eof) {
    auto ch=peek(_ctx);
    auto pos = bad.find(ch);
    if (::isspace(ch) ||
        pos != std::string::npos) { break; }
    res += ch;
    d::advance(_ctx);
  }
  if (res == "false") {
    return token(T_FALSE, res, m);
  }
  if (res == "true") {
    return token(T_TRUE, res, m);
  }
  if (res == "nil") {
    return token(T_NIL, res, m);
  }
  DEBUG("::id = %s.\n", C_STR(res));
  return token(d::T_IDENT, res, m, res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Reader::skipCommas() {
  while (!_ctx.eof &&
         (peek(_ctx) == ',')) advance(_ctx);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::getNextToken() {

  Tchar ch;
  while (!_ctx.eof) {
    ch= d::peek(_ctx);
    if (::isspace(ch)) {
      d::skipWhitespace(_ctx);
    }
    else
    if (ch == ',') {
      skipCommas();
    }
    else
    if (ch == ';') {
      skipComment();
    }
    else
    if (ch == '~' &&
        d::peekNext(_ctx) == '@') {
      auto m= _ctx.mark();
      d::advance(_ctx, 2);
      return token(T_SPLICE_UNQUOTE, "~@", m);
    }
    else
    if (ch == '\'') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_QUOTE, ch, m);
    }
    else
    if (ch == '`') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_BACKTICK, ch, m);
    }
    else
    if (ch == '~') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_TILDA, ch, m);
    }
    else
    if (ch == '^') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_HAT, ch, m);
    }
    else
    if (ch == '@') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_AT, ch, m);
    }
    else
    if ((ch == '-' || ch == '+') &&
        ::isdigit(d::peekNext(_ctx))) {
      //skip the pointless + sign
      if (ch=='+') { d::advance(_ctx); }
      return number();
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
    if (ch == '#' &&
        d::peekNext(_ctx) == '{') {
      auto m=_ctx.mark();
      d::advance(_ctx, 2);
      return token(T_SET, "#{", m);
    }
    else
    if (ch == '#' &&
        d::peekNext(_ctx) == '(') {
      auto m=_ctx.mark();
      d::advance(_ctx, 2);
      return token(T_ANONFN, "#(", m);
    }
    else
    if (ch == '(') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_LPAREN, ch, m);
    }
    else
    if (ch == ')') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_RPAREN, ch, m);
    }
    else
    if (ch == '{') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_LBRACE, ch, m);
    }
    else
    if (ch == '}') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_RBRACE, ch, m);
    }
    else
    if (ch == '[') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_LBRACKET, ch, m);
    }
    else
    if (ch == ']') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_RBRACKET, ch, m);
    }
    else
    if (ch == ':') {
      auto nx= d::peekNext(_ctx);
      if (isKeywdChar(nx)) {
        return keywd();
      } else {
        auto m=_ctx.mark();
        d::advance(_ctx);
        return token(d::T_COLON, ch, m);
      }
    }
    else
    if (ch == '.') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_DOT, ch, m);
    }
    else
    if (ch) {
      return id();
    }
    else {
      RAISE(d::SyntaxError,
            "Unexpected char %c near line %d(%d).\n", ch, _ctx.line, _ctx.col);
    }
  }

  return token(d::T_EOF, "<EOF>", _ctx.mark());
}







//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


