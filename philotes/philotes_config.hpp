#pragma once

#include <cassert>
#include <list>
#include <queue>
#include <string>

using std::list;
using std::queue;
using std::string;

#define DEBUG 0

#if DEBUG
#define assert_cast dynamic_cast
#define YYDEBUG 1
#else
#define NDEBUG
#define assert_cast static_cast
#endif

//What language are we parsing?
extern int language_mode;

const static int CPP_MODE = 0;
const static int JAVA_MODE = 1;

extern list<string> error_msg;
extern queue<string> remaining_sources;
extern const char* interaction_buffer;
extern "C" int yywrap();

extern int inject_herald;

struct AST_Node;
extern AST_Node* root;

//Note: set current_file and maybe other configurables in symtab.hpp
