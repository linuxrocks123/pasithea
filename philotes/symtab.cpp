#include "StringFunctions.h"
#include "symtab.hpp"

#include "Declaration.hpp"
#include "Statement.hpp"
#include "philotes.h"

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <utility>
#include <vector>

using std::cout;
using std::endl;

using std::initializer_list;
using std::find_if;
using std::make_pair;
using std::pair;
using std::vector;

list<map<string,Symbol> > symtab;
map<string,Type> typetab;
stack<Symbol*> this_stack;

string get_scope_qualifier_prefix()
{
     string to_return="::";
     for(const string& x : static_scopes)
          to_return += (x[0]==':' ? x.substr(2) : x)+"::";
     return to_return;
}

string qualify_unscoped_reference(const string& symbol, bool funcs_valid)
{
     //If we're already scoped, try unscoping ourselves
     if(symbol[0]==':')
          return qualify_unscoped_reference(symbol.substr(2),funcs_valid);
     
     string prefix="";
     for(const string& x : static_scopes)
     {
          prefix+=x;
          string prefixed_symbol = prefix+(symbol[0]==':' ? "" : "::")+symbol;
          if(typetab.count(prefixed_symbol))
               return prefixed_symbol;
          if(find_if(symtab.front().begin(),symtab.front().end(),[&](const auto& y) { return y.first==prefixed_symbol || funcs_valid && y.first.find("(")!=string::npos && y.first.substr(0,y.first.find("("))==prefixed_symbol; })!=symtab.front().end())
               return prefixed_symbol;
     }

     if(find_if(symtab.front().begin(),symtab.front().end(),[&](const auto& y) { return y.first==symbol || funcs_valid && y.first.find("(")!=string::npos && y.first.substr(0,y.first.find("("))==symbol; })!=symtab.front().end())
          return symbol;

     if(symbol[0]==':')
          return symbol;
     return "::"+symbol;
}

list<string> static_scopes;

Symbol* resolve_function_symbol(string name, list<Type*>& parameters)
{
     map<string,Symbol>& global_symtab = symtab.front();
     list<list<Type*> > overload_resolution_types;

     if(parameters.empty())
          if(global_symtab.count(name+"()"))
               return &global_symtab[name+"()"];
          else
               return NULL;

     initializer_list<const char*> primitives_ladder;
     if(language_mode==JAVA_MODE)
          primitives_ladder = {"byte","short","char","int","long","float","double"};
     else
          primitives_ladder = {"bool","char","short","int","long","float","double"};

     bool first_param = true;
     for(Type* param : parameters)
     {
          overload_resolution_types.emplace_back();
          list<Type*>& promotion_ladder = overload_resolution_types.back();
          auto rung = find_if(primitives_ladder.begin(),primitives_ladder.end(),[&](const auto& x) { return param->name==x; });
          while(rung!=primitives_ladder.end())
          {
               promotion_ladder.push_back(&typetab[*rung]);
               ++rung;

               if(first_param && (name=="::=" || name=="::+=" || name=="::-=" || name=="::*=" || name=="::/=" || name=="::%=") && rung!=primitives_ladder.end() && *rung=="float") //prevent assigning floats or doubles to anything non-floatacious
                    break;
          }
          first_param = false;
          if(promotion_ladder.size())
               continue;

          promotion_ladder.push_back(param);
          Type* parent = param;
          while(parent->base_or_parent_types.size())
          {
               parent = parent->base_or_parent_types.front();
               promotion_ladder.push_back(parent);
          } //TODO: handle multiple inheritance
     }

     //Look for compatible method
     list<pair<list<Type*>*,list<Type*>::iterator> > search_stack;
     for(list<Type*>& param : overload_resolution_types)
          search_stack.push_back(make_pair(&param,param.begin()));

     //Construct string using search stack
     auto get_search_string = [&]()
     {
          string to_return = name+"(";
          bool first = true;
          for(auto& ss_pair : search_stack)
          {
               if(!first)
                    to_return+=",";
               else
                    first = false;
               to_return+=(*ss_pair.second)->name;
          }

          to_return+=")";
          return to_return;
     };

     string candidate;
     while(!global_symtab.count(candidate = get_search_string()))
     {
          stack<list<Type*>*> reset_needed;
          while(search_stack.size())
          {
               list<Type*>::iterator& x = search_stack.back().second;
               ++x;
               if(x!=search_stack.back().first->end())
                    break;
               reset_needed.push(search_stack.back().first);
               search_stack.pop_back();
          }
          if(search_stack.empty())
               return NULL;
          while(reset_needed.size())
          {
               search_stack.push_back(make_pair(reset_needed.top(),reset_needed.top()->begin()));
               reset_needed.pop();
          }
     }
     
     return &global_symtab[candidate];
}

///Each mark is the lowest index of the symtab stack that is visible in the current context
stack<int> severances;

///Mark the current top of the symtab stack as the lowest visible index
void sever_visibility()
{
     while(severances.size() && severances.top() >= symtab.size())
          severances.pop();
     severances.push(symtab.size()-1);
}

Symbol* resolve_atomic_symbol(string name)
{
     //Deal with severances stack
     while(severances.size() && severances.top() >= symtab.size())
          severances.pop();

     int lowest_visible_index = severances.empty() ? 0 : severances.top();
     
     //Normal
     auto loc_sym_it = symtab.rbegin();
     for(int i=symtab.size()-1; i>=lowest_visible_index; i--,++loc_sym_it)
          if(loc_sym_it->count(name))
               return &(*loc_sym_it)[name];

     //Current object
     Symbol* tthis = this_stack.size() ? this_stack.top() : NULL;
     if(tthis && tthis->components.count(name))
          return &tthis->components[name];
     
     //Globals
     auto& global_syms = *symtab.begin();
     string global_name = qualify_unscoped_reference(name,false);
     if(global_syms.count(global_name))
          return &global_syms[global_name];
     return NULL;
}

int current_visibility_level;
int current_line = 1;
string current_file;

void reset()
{
     symtab.clear();
     typetab.clear();
     this_stack = stack<Symbol*>{};
     severances = stack<int>{};
     static_scopes.clear();
     error_msg.clear();
     remaining_sources = queue<string>{};
     current_file = "";
     current_line = 1;
     current_visibility_level = 0;
}

void philotes_add_primitives()
{
     symtab.emplace_back();
     this_stack.push(NULL);
     map<string,Symbol>& syms = symtab.front();
     
     Symbol skel{};
     skel.hidden = true;
     Type t_skel{};

     auto init_prim = [&](string x)
     {
          skel.name = x;
          syms[x] = skel;
          t_skel.name = x;
          typetab[x] = t_skel;
     };

     auto init_obj = [&](string x)
     {
          skel.name = x;
          syms[x] = skel;
          t_skel.name = x;
          t_skel.aggregate_type = CLASS;
          if(language_mode==JAVA_MODE && x!="Object")
               t_skel.base_or_parent_types.push_back(&typetab["Object"]);
          typetab[x] = t_skel;
          t_skel = Type{};
     };

     //Initialize primitive types
     init_prim("void");
     if(language_mode==JAVA_MODE)
     {
          init_prim("boolean");
          init_prim("byte");
     }
     else
          init_prim("bool");
     init_prim("char");
     init_prim("short");
     init_prim("int");
     init_prim("long");
     init_prim("float");
     init_prim("double");
     if(language_mode==JAVA_MODE)
     {
          init_obj("Object");
          init_obj("String");
     }
     else
          init_obj("std::string");
     //TODO: C++-specific types

     //NULL/null
     if(language_mode==JAVA_MODE)
     {
          Symbol null = Symbol{};
          null.hidden = true;
          null.type = &typetab["Object"];
          syms["null"] = null;
     }
     else
          assert(false); //TODO

     Statement* prayer = new Sentinel{PRAYER};
     prayer->line = 0;

     //Primitive operators, please stand by
     //TODO: C++
     auto primitive_names = {"+(byte,byte)","+=(byte,byte)","+(char,char)","+=(char,char)","+(short,short)","+=(short,short)","+(int,int)","+=(int,int)","+(long,long)","+=(long,long)","+(float,float)","+=(float,float)","+(double,double)","+=(double,double)","-(byte,byte)","-=(byte,byte)","-(char,char)","-=(char,char)","-(short,short)","-=(short,short)","-(int,int)","-=(int,int)","-(long,long)","-=(long,long)","-(float,float)","-=(float,float)","-(double,double)","-=(double,double)","-_(int)","-_(long)","-_(float)","-_(double)","*(byte,byte)","*=(byte,byte)","*(char,char)","*=(char,char)","*(short,short)","*=(short,short)","*(int,int)","*=(int,int)","*(long,long)","*=(long,long)","*(float,float)","*=(float,float)","*(double,double)","*=(double,double)","/(byte,byte)","/=(byte,byte)","/(char,char)","/=(char,char)","/(short,short)","/=(short,short)","/(int,int)","/=(int,int)","/(long,long)","/=(long,long)","/(float,float)","/=(float,float)","/(double,double)","/=(double,double)","%(byte,byte)","%=(byte,byte)","%(char,char)","%=(char,char)","%(short,short)","%=(short,short)","%(int,int)","%=(int,int)","%(long,long)","%=(long,long)","%(float,float)","%=(float,float)","%(double,double)","%=(double,double)","<<(byte,byte)","<<=(byte,byte)","<<(char,char)","<<=(char,char)","<<(short,short)","<<=(short,short)","<<(int,int)","<<=(int,int)","<<(long,long)","<<=(long,long)","<<(float,float)","<<=(float,float)","<<(double,double)","<<=(double,double)",">>(byte,byte)",">>=(byte,byte)",">>(char,char)",">>=(char,char)",">>(short,short)",">>=(short,short)",">>(int,int)",">>=(int,int)",">>(long,long)",">>=(long,long)",">>(float,float)",">>=(float,float)",">>(double,double)",">>=(double,double)",">>>(byte,byte)",">>>=(byte,byte)",">>>(char,char)",">>>=(char,char)",">>>(short,short)",">>>=(short,short)",">>>(int,int)",">>>=(int,int)",">>>(long,long)",">>>=(long,long)",">>>(float,float)",">>>=(float,float)",">>>(double,double)",">>>=(double,double)","&(byte,byte)","&=(byte,byte)","&(char,char)","&=(char,char)","&(short,short)","&=(short,short)","&(int,int)","&=(int,int)","&(long,long)","&=(long,long)","&(float,float)","&=(float,float)","&(double,double)","&=(double,double)","|(byte,byte)","|=(byte,byte)","|(char,char)","|=(char,char)","|(short,short)","|=(short,short)","|(int,int)","|=(int,int)","|(long,long)","|=(long,long)","|(float,float)","|=(float,float)","|(double,double)","|=(double,double)","^(byte,byte)","^=(byte,byte)","^(char,char)","^=(char,char)","^(short,short)","^=(short,short)","^(int,int)","^=(int,int)","^(long,long)","^=(long,long)","^(float,float)","^=(float,float)","^(double,double)","^=(double,double)","+(String,String)","+=(String,String)","+(String,long)","+=(String,long)","+(String,char)","+=(String,char)","+(String,double)","+=(String,double)","<(double,double)","<=(double,double)","==(double,double)","!=(double,double)",">=(double,double)",">(double,double)","==(boolean,boolean)","!=(boolean,boolean)","!_(boolean)","&&(boolean,boolean)","||(boolean,boolean)","==(Object,Object)","!=(Object,Object)","System.out.println(boolean)","System.out.println(long)","System.out.println(char)","System.out.println(double)","System.out.println(String)","System.out.print(boolean)","System.out.print(long)","System.out.print(char)","System.out.print(double)","System.out.print(String)","++_(byte)","++_(char)","++_(short)","++_(int)","++_(long)","++_(float)","++_(double)","_++(byte)","_++(char)","_++(short)","_++(int)","_++(long)","_++(float)","_++(double)","--_(byte)","--_(char)","--_(short)","--_(int)","--_(long)","--_(float)","--_(double)","_--(byte)","_--(char)","_--(short)","_--(int)","_--(long)","_--(float)","_--(double)","=(boolean,boolean)","=(long,long)","=(double,double)","=(Object,Object)","(byte)(long)","(byte)(double)","(short)(long)","(short)(double)","(int)(long)","(int)(double)","(long)(long)","(long)(double)","(float)(long)","(float)(double)","(double)(long)","(double)(double)","String::length()","String::charAt(int)","String::equals(String)","String::compareTo(String)","System.out.println()","^(boolean,boolean)"};
     auto primitive_types = {"byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","int(int)","long(long)","float(float)","double(double)","byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","byte(byte,byte)","byte(byte,byte)","char(char,char)","char(char,char)","short(short,short)","short(short,short)","int(int,int)","int(int,int)","long(long,long)","long(long,long)","float(float,float)","float(float,float)","double(double,double)","double(double,double)","String(String,String)","String(String,String)","String(String,long)","String(String,long)","String(String,char)","String(String,char)","String(String,double)","String(String,double)","boolean(double,double)","boolean(double,double)","boolean(double,double)","boolean(double,double)","boolean(double,double)","boolean(double,double)","boolean(boolean,boolean)","boolean(boolean,boolean)","boolean(boolean)","boolean(boolean,boolean)","boolean(boolean,boolean)","boolean(Object,Object)","boolean(Object,Object)","void(boolean)","void(long)","void(char)","void(double)","void(String)","void(boolean)","void(long)","void(char)","void(double)","void(String)","byte(byte)","char(char)","short(short)","int(int)","long(long)","float(float)","double(double)","byte(byte)","char(char)","short(short)","int(int)","long(long)","float(float)","double(double)","byte(byte)","char(char)","short(short)","int(int)","long(long)","float(float)","double(double)","byte(byte)","char(char)","short(short)","int(int)","long(long)","float(float)","double(double)","boolean(boolean,boolean)","long(long,long)","double(double,double)","Object(Object,Object)","byte(long)","byte(double)","short(long)","short(double)","int(long)","int(double)","long(long)","long(double)","float(long)","float(double)","double(long)","double(double)","int()","char(int)","boolean(String)","int(String)","void()","boolean(boolean,boolean)"};
     for(auto i = primitive_names.begin(), j = primitive_types.begin(); i!=primitive_names.end(); ++i,++j)
     {
          vector<string> parameters;
          StringFunctions::tokenize(parameters,*j,"(,)");
          if(!typetab.count(*j))
          {
               Type tti{}; //type to insert
               tti.name = *j;
               for(auto param_str : parameters)
                    tti.parameters.push_back(&typetab[param_str]);
               typetab[*j] = tti;
          }

          string scoped_name = qualify_unscoped_reference(*i);
          Symbol sti{}; //symbol to insert
          sti.name = scoped_name;
          sti.type = &typetab[*j];

          Func_Decl* extraplanar_func = new Func_Decl{};
          extraplanar_func->line = 0;
          extraplanar_func->name = scoped_name;
          extraplanar_func->modifiers.static_type = &typetab[*j];
          auto k = typetab[*j].parameters.begin();
          auto k_e = typetab[*j].parameters.end();
          ++k;
          char var_name = 'a';
          while(k!=k_e)
          {
               Var_Decl_Statement* v_decl = new Var_Decl_Statement{};
               v_decl->modifiers.static_type = &typetab[(*k)->name];
               v_decl->name = var_name;
               extraplanar_func->modifiers.parameters.push_back(v_decl);
               ++k;
               var_name++;
          }
          extraplanar_func->body.data.push_back(prayer);
          
          sti.value.ary.ptr = extraplanar_func;
          syms[scoped_name] = sti;
     }

     //The array "constructor" symbol
     Symbol ac{};
     ac.hidden = true;
     ac.type = NULL;
     syms["[]"] = ac;
}

void inspector_gadget(string command, string* buffer)
{
     if(command=="#print") //print source code, with lines numbered
     {
     }
     else if(command=="#edit") //retype a line; only valid if buffer!=NULL
     {
     }
     else if(command=="#dump") //dump source code to a file
     {
     }
     else if(command=="#vars") //print all currently accessible variables and values
     {
     }
     else if(command=="#vars!") //print all variables and values, event those not accessible
     {
     }
     else if(command.find("#var ")==0) //print value of named variable
     {
     }
     else if(command.find("#eval ")==0) //evaluate expression: See https://www.gnu.org/software/bison/manual/html_node/Multiple-start_002dsymbols.html
     {
     }
     else if(command=="#exit" || command=="#next")
          return;
     else
          cout << "<< Invalid Command >>" << endl;
}
