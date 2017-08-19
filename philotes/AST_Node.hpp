#pragma once

#include "philotes_config.hpp"
#include "Visitor.hpp"

struct AST_Node
{
     int line;
     string file;
     void copy_from(const AST_Node& other)
          {
               line = other.line;
               file = other.file;
          }

     virtual void visit_with(Visitor& v) = 0;
};
