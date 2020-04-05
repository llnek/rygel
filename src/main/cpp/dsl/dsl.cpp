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

#include "dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::dsl {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a=czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Data* nothing() { return new Nothing(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, std::string> TOKENS {
  {T_INTEGER, "long"},
  {T_REAL, "double"},
  {T_STRING, "string"},
  {T_IDENT, "id"},
  {T_COMMA, ","},
  {T_COLON, ":"},
  {T_LPAREN, "("},
  {T_RPAREN, ")"},
  {T_LBRACE, "{"},
  {T_RBRACE, "}"},
  {T_EQ, "="},
  {T_MINUS, "-"},
  {T_PLUS, "+"},
  {T_MULT, "*"},
  {T_DIV, "/"},
  {T_GT, ">"},
  {T_LT, "<"},
  {T_QUOTE,"'"},
  {T_DQUOTE,"\""},
  {T_SEMI, ";"},
  {T_TILDA, "~"},
  {T_BACKTICK, "`"},
  {T_BANG, "!"},
  {T_AMPER, "&"},
  {T_AT, "@"},
  {T_PERCENT, "%"},
  {T_QMARK, "?"},
  {T_HAT, "^"},
  {T_LBRACKET, "["},
  {T_RBRACKET, "]"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SemanticError::SemanticError(const std::string& x) : a::Exception(x) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SyntaxError::SyntaxError(const std::string& x) : a::Exception(x) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEqual(int wanted, int got, const stdstr& fn) {
  if (wanted != got)
    RAISE(BadArity,
          "%s requires %d args, got %d.\n", C_STR(fn), wanted, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preMax(int max, int got, const stdstr& fn) {
  if (got > max)
    RAISE(BadArity,
          "%s requires at most %d args, got %d.\n", C_STR(fn), max, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preMin(int min, int got, const stdstr& fn) {
  if (got < min)
    RAISE(BadArity,
          "%s requires at least %d args, got %d.\n", C_STR(fn), min, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preNonZero(int c, const stdstr& fn) {
  if (c == 0)
    RAISE(BadArity,
          "%s requires some args, got %d.\n", C_STR(fn), c);
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEven(int c, const stdstr& fn) {
  if (!a::is_even(c))
    RAISE(BadArity,
          "%s requires even args, got %d.\n", C_STR(fn), c);
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar peek(Context& ctx) {
  // return the current char.
  return ctx.src[ctx.pos];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar peekNext(Context& ctx, int offset) {
  // Peek into the buffer and see what's ahead.
  auto nx = ctx.pos + offset;
  return nx >= 0 && nx < ctx.len ? ctx.src[nx] : '\0';
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool forward(Context& ctx) {
  // move up one char, handling newline.
  if (ctx.eof) { return false; }
  if (peek(ctx) == '\n') {
    ++ctx.line;
    ctx.col=0;
  }
  ++ctx.pos;
  if (ctx.pos >= ctx.len) {
    ctx.eof=true;
  } else {
    ++ctx.col;
  }
  return !ctx.eof;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool advance(Context& ctx, int steps) {
  for (int i=0; i < steps; ++i) { forward(ctx); }
  return !ctx.eof;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skipWhitespace(Context& ctx) {
  while (!ctx.eof &&
         ::isspace(peek(ctx))) advance(ctx);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string line(Context& ctx) {
  std::string res;
  while (!ctx.eof) {
    auto ch = peek(ctx);
    advance(ctx);
    if (ch == '\n')
    break;
    res += peek(ctx);
  }
  return res;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string digits(Context& ctx) {
  std::string res;
  while (!ctx.eof && ::isdigit(peek(ctx))) {
    res += peek(ctx);
    advance(ctx);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string numeric(Context& ctx) {
  // handles 'signed' and floating points.
  auto ch= peek(ctx);
  auto minus=false;
  if (ch == '-' || ch== '+') {
    minus= (ch=='-');
    advance(ctx);
  }
  auto res = digits(ctx);
  if (!ctx.eof && peek(ctx) == '.') {
    res += peek(ctx);
    advance(ctx);
    res += digits(ctx);
  }

  return minus ? "-"+res : res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string str(Context& ctx) {

  std::string res;
  Tchar ch;

  if (!ctx.eof &&
      peek(ctx) == '"') {
    advance(ctx);
    while (!ctx.eof) {
      ch= peek(ctx);
      if (ch == '"') {
        break;
      }
      if (ch == '\\') {
        if (!advance(ctx)) {
          RAISE(SyntaxError,
                "Malformed string value, bad escaped char %c\n.", ch);
        }
        ch=a::unescape_char(peek(ctx));
      }
      res += ch;
      advance(ctx);
    }
    if (ctx.eof || ch != '"') {
      RAISE(SyntaxError,
            "Malformed string value, missing %s\n.", "dquote");
    }
    // good, got the end dquote
    advance(ctx);
  }

  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string identifier(Context& ctx, IdPredicate pred) {
  std::string res;
  Tchar ch;
  while (!ctx.eof) {
    ch=peek(ctx);
    if (res.empty()) {
      if (pred(ch,true)) {
        res += ch;
        advance(ctx);
      } else {
        break;
      }
    } else {
      if (pred(ch,false)) {
        res += ch;
        advance(ctx);
      } else {
        break;
      }
    }
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Number::isZero() const {
  if (type==T_INTEGER) { return u.n==0; } else { return u.r==0.0; }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Number::isInt() const { return type== T_INTEGER;}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Number::getFloat() const { return u.r; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Number::getInt() const { return u.n; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Number::setFloat(double d) { u.r=d; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Number::setInt(llong n) { u.n=n; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Context::Context() {
  S_NIL(src);
  len=0;
  line=0;
  col=0;
  pos=0;
  eof=false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SrcInfo Context::mark() { return s__pair(int,int,line,col); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(const std::string& n, DslFrame outer) : _name(n) { prev=outer; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(const std::string& n) : _name(n) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Frame::pr_str() const {
  stdstr b(40, '=');
  stdstr z(40, '+');
  stdstr out;
  stdstr bits;

  out += b;
  out += "\n";
  out += "frame: " + _name + " => ";

  for (auto i= slots.begin(), e= slots.end(); i != e; ++i) {
    auto v= i->second;
    auto vs= v.isSome() ? v->pr_str(1) : stdstr("null");
    if (!bits.empty()) bits += ", ";
    bits += i->first + "=" + vs;
  }
  if (!bits.empty()) bits += "\n";

  return out + bits + z;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::set<std::string> Frame::keys() const {
  std::set<std::string> out;
  for (auto &x : slots) {
    out.insert(x.first);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Frame::get(const std::string& key) const {

  auto x= slots.find(key);
  auto r= x != slots.end()
          ? x->second
          : (prev.isSome() ? prev->get(key) : DslValue());

  DEBUG("frame:get %s <- %s\n",
        C_STR(key), C_STR(r->pr_str()));

  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Frame::setEx(const std::string& key, DslValue v) {

  auto x= slots.find(key);
  if (x != slots.end()) {
    slots[key]=v;
    DEBUG("frame:setEx %s -> %s\n",
          C_STR(key), C_STR(v->pr_str(1)));
    return v;
  } else if (prev.isSome()) {
    return prev->setEx(key,v);
  } else {
    return DslValue();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Frame::set(const std::string& key, DslValue v) {
  slots[key]=v;
  DEBUG("frame:set %s -> %s\n",
        C_STR(key), C_STR(v->pr_str(1)));
  return v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Frame::contains(const stdstr& key) const {
  return slots.find(key) != slots.end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::search(const stdstr& key) const {
  auto x= slots.find(key);
  return x != slots.end()
         ? DslFrame(const_cast<Frame*>(this))
         : (prev.isSome() ? prev->search(key) : DslFrame());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::getOuterRoot() const {
  return prev.isSome()
         ? prev->getOuterRoot()
         : DslFrame(const_cast<Frame*>(this));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::getOuter() const { return prev; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(const std::string& n,
             const std::map<std::string, DslSymbol>& root) : Table(n) {
  for (auto& x : root) {
    symbols[x.first]=x.second;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(const std::string& n, DslTable outer) : Table(n) {
  enclosing=outer;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(const std::string& n) : _name(n) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Table::insert(DslSymbol s) {
  if (s.isSome())
    symbols[s->name()] = s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslSymbol Table::search(const std::string& name) const {
  if (auto s = symbols.find(name); s != symbols.end()) {
    return s->second;
  } else {
    return enclosing.isNull() ? DslSymbol() : enclosing->search(name);
  }
}

DslSymbol Table::find(const std::string& name) const {
  if (auto s = symbols.find(name); s != symbols.end()) {
    return s->second;
  } else {
    return DslSymbol();
  }
}





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


