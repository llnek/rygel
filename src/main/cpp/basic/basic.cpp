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

#include <iostream>
#include "parser.h"
#include "builtins.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Basic::Basic(const char* src) {
  source = src;
  running=false;
  progOffset=0;
  progCounter=0;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame Basic::root_env() {
  return init_natives(pushFrame("root"));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Basic::interpret() {
  BasicParser p(source);
  root_env();
  auto tree= p.parse();
  //std::cout << s__cast(Ast,tree.ptr())->pr_str() << "\n";
  return check(tree), eval(tree);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Basic::eval(d::DslAst tree) {
  return tree->eval(this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame Basic::pushFrame(const stdstr& name) {
  stack = d::Frame::make(name, stack);
  return stack;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame Basic::popFrame() {
  if (stack) {
    auto f= stack;
    //::printf("Frame pop'ed:=\n%s\n", f->pr_str().c_str());
    stack= stack->getOuter();
    return f;
  } else {
    return P_NIL;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame Basic::peekFrame() const {
  return stack;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Basic::setValueEx(const stdstr& name, d::DslValue v) {
  auto x = peekFrame();
  return x ? x->setEx(name, v) : P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Basic::setValue(const stdstr& name, d::DslValue v) {
  auto x = peekFrame();
  return x ? x->set(name, v) : P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Basic::getValue(const stdstr& name) const {
  auto x = peekFrame();
  return x ? x->get(name) : P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<stdstr,d::DslSymbol> BITS {
  {"INTEGER", d::Symbol::make("INTEGER")},
  {"REAL", d::Symbol::make("REAL")},
  {"STRING", d::Symbol::make("STRING")}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::check(d::DslAst tree) {
  symbols= d::Table::make("root", BITS);
  tree->visit(this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslSymbol Basic::search(const stdstr& n) const {
  return symbols ? symbols->search(n) : P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslSymbol Basic::find(const stdstr& n) const {
  return symbols ? symbols->find(n) : P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslSymbol Basic::define(d::DslSymbol s) {
  if (symbols) symbols->insert(s);
  return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslTable Basic::pushScope(const stdstr& name) {
  symbols= d::Table::make(name, symbols);
  return symbols;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslTable Basic::popScope() {
  if (!symbols) {
    return P_NIL;
  }
  auto cur = symbols;
  symbols = cur.get()->outer();
  return cur;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Basic::readString() {
  stdstr s;
  std::cin >> s;
  return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Basic::readFloat() {
  double d;
  std::cin >> d;
  return d;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Basic::readInt() {
  llong n;
  std::cin >> n;
  return n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::writeString(const stdstr& s) {
  ::printf("%s", s.c_str());
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::writeFloat(double d) {
  ::printf("%lf", d);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::writeInt(llong n) {
  std::cout << n;
  //::printf("%ld", n);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::writeln() {
  ::printf("%s", "\n");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::installProgram(const std::map<llong,llong>& m) {
  for (auto& x : m) { lines[x.first] = x.second; }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::uninstall() {
  lines.clear();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::init_counters() {
  while (!gosubReturns.empty()) gosubReturns.pop();
  running=true;
  progOffset=0;
  progCounter= -1;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::finz_counters() {
  progOffset=0;
  progCounter= -1;
  running=false;
  while (!gosubReturns.empty()) gosubReturns.pop();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Basic::retSub() {
  if (gosubReturns.empty())
    RAISE(d::BadArg, "Bad gosub-return: %s\n", "no sub called");
  auto r= gosubReturns.top();
  gosubReturns.pop();
  progOffset = r.second+1;
  return (progCounter = r.first - 1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Basic::jumpSub(llong target, llong from, llong off) {

  auto it= lines.find(target);
  if (it == lines.end())
    RAISE(d::BadArg, "Bad gosub: %d\n", (int)target);

  ASSERT1(progCounter == lines[from]);
  gosubReturns.push(s__pair(llong,llong,progCounter,off));
  auto pc = it->second;
  progOffset=0;
  return (progCounter = pc-1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Basic::jump(llong line) {
  auto it= lines.find(line);
  if (it == lines.end())
    RAISE(d::BadArg, "Bad goto: %d\n", (int)line);
  auto pos = it->second ;
  progOffset=0;
  return (progCounter = pos-1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Basic::jumpFor(DslFLInfo f) {
  auto it= lines.find(f->begin);
  if (it == lines.end())
    RAISE(d::BadArg, "Bad for-loop: %d.", (int) f->begin);
  progOffset=f->beginOffset;
  return (progCounter = it->second -1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Basic::endFor(DslFLInfo f) {
  auto it= lines.find(f->end);
  if (it == lines.end())
    RAISE(d::BadArg, "Bad end-for: %d.", (int)f->end);
  f->init=P_NIL;
  progOffset=f->endOffset+1;
  return (progCounter = it->second-1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::addForLoop(DslFLInfo f) {
  auto x= this->forLoop; // current outer for loop
  auto vn= f->var;
  auto bad=false;
  while (x) {
    bad = (x->var == vn);
    x=x->outer;
  }
  if (bad)
    RAISE(d::SemanticError, "For counter-var %s reused.", C_STR(vn));
  f->outer= forLoop;
  forLoop=f;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::xrefForNext(const stdstr& v, llong n, llong pos) {
  // make sure the next statement matches the current for loop.
  auto c = this->forLoop;
  ASSERT1(c);
  if (!(c->var == v))
    RAISE(d::SemanticError,
          "Expecting  for-counter: %s, got %s.\n",
          c->var.c_str(), C_STR(v));
  c->endOffset=pos;
  c->end= n;
  // find the corresponding counters
  auto b= this->lines[c->begin];
  auto e= this->lines[c->end];

  this->forBegins[b] = c;
  this->forEnds[e] = c;

  // pop it
  forLoop=forLoop->outer;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFLInfo Basic::getForLoop(llong c) const {
  if (auto i = forBegins.find(c); i != forBegins.end()) {
    return i->second;
  }

  if (auto i = forEnds.find(c); i != forEnds.end()) {
    return i->second;
  }

  RAISE(d::SemanticError, "Unknown for-loop: %d.", (int) c);
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


