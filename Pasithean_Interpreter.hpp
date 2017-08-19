#pragma once

#include "philotes/Hypnos_Visitor.hpp"

#include <list>
#include <map>
#include <unordered_map>
#include <sstream>

using std::list;
using std::map;
using std::unordered_map;

class Pasithean_Interpreter : public Hypnos_Visitor
{
public:
     //Override: reset pi_sout and diagnostics_map when called with depth 0
     void visit(Decl_List& decl_list);
     
     //Override: fill in diagnostics_map statement by statement
     void visit(Statement_List& stmt_list);

     //Overrides: infinite loop termination
     void visit(While_Statement& while_stmt);
     void visit(For_Statement& for_stmt);

     //Public API
     string get_diagnostics_for_line(int line);
     string get_output() { return pi_sout.str(); }
     void reset_input() { pi_sin.str(""); pi_sin.clear(); }
     void set_max_iterations(int max_iterations_) { max_iterations = max_iterations_; }
     Pasithean_Interpreter() : Hypnos_Visitor(pi_sin,pi_sout) { }

protected:
     //Override: handle buffered output and cached/graphical input
     void handle_prayer();
     
private:
     std::stringstream pi_sin;
     std::ostringstream pi_sout;

     int depth = 0;
     int max_iterations = 100;
     bool infinite_loop_detected = false;
     unordered_map<int,map<string,list<string>>> diagnostics_map;
};
