#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#if _WIN32
#define BOOLEAN BOOLEAN_EVIL
#define BYTE BYTE_EVIL
#define CHAR CHAR_EVIL
#define SHORT SHORT_EVIL
#define INT INT_EVIL
#define LONG LONG_EVIL
#define FLOAT FLOAT_EVIL
#define DOUBLE DOUBLE_EVIL
#endif

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>

#if _WIN32
#undef BOOLEAN
#undef BYTE
#undef CHAR
#undef SHORT
#undef INT
#undef LONG
#undef FLOAT
#undef DOUBLE

#undef CONST
#undef DELETE
#undef DOUBLE
#undef VOID
#endif

#include "FLTK_Callback_Editor.hpp"
#include "Pasithean_Echo_Visitor.hpp"
#include "Pasithean_Interpreter.hpp"

#include "philotes/philotes_config.hpp"
#include "philotes/Decl_List.hpp"
#include "philotes/Import_Visitor.hpp"
#include "philotes/Semantic_Visitor.hpp"
#include "philotes/Expression.hpp"
#include "philotes/fmemopen.h"
#include "philotes/philotes.h"
#include "philotes/symtab.hpp"

void yyrestart(FILE* new_file);

using namespace std;

static Pasithean_Echo_Visitor echoer;
static Fl_Text_Buffer text_buffer;
static Fl_Text_Buffer style_buffer;

static bool step_requested = false;
static bool debugger_enabled = false;
static void interpreter_callback()
{
     //TODO
     
     while(debugger_enabled && !step_requested)
     {
          //TODO
          
          Fl::wait(10);
     }
}

static Fl_Text_Buffer iob_buffer;
static Fl_Text_Buffer pseb_buffer;
static Fl_Window* w;
static FLTK_Callback_Editor* mb;
static Pasithean_Interpreter executor;
static bool reset_needed = false;
static string filename;

//FLTK Callback Editor for main buffer
static bool mb_callback(FLTK_Callback_Editor& ed, int key)
{
     //Used so that we don't unnecessarily switch out buffers when nothing changed
     static string old_text;
     
     //Run ed.buffer()'s text through yyparse(),
     //Pasithean_Echo_Visitor, and Semantic_Visitor

     reset();
     philotes_add_primitives();

     char* buffer = text_buffer.text();
     if(old_text==buffer && !reset_needed)
     {
          int pos = ed.insert_position();
          string source_text = text_buffer.text();
          int lines = count(source_text.begin(),source_text.end(),'\n');
          int buffer_line = count(source_text.begin(),source_text.begin()+pos,'\n') + (key==FL_Left || key==FL_Up ? -1 : 1);
          pseb_buffer.text(executor.get_diagnostics_for_line(buffer_line).c_str());
          return true;
     }
     reset_needed = false;

     FILE* bfp = fmemopen(buffer,strlen(buffer),"r");
     yyrestart(bfp);
     
     inject_herald = PROGRAM_HERALD;
     int result = yyparse();
     if(result)
     {
          char style[strlen(buffer)+1];
          memset(style,'H',strlen(buffer));
          style[strlen(buffer)]='\0';
          style_buffer.text(style);

          pseb_buffer.text("You have a syntax error.");
          ed.redraw();

          reset_needed = true;
          return false;
     }

     remaining_sources.push("#System.java");
     remaining_sources.push("#Double.java");
     Import_Visitor importer;
     importer.visit(*static_cast<Decl_List*>(root));

     auto backup_sources = remaining_sources;
     auto backup_errors = error_msg;
     reset();
     remaining_sources = backup_sources;
     error_msg = backup_errors;

     Semantic_Visitor semantics;

     //Parse imported code
     philotes_add_primitives();
     inject_herald = PROGRAM_HERALD;
     result = yyparse();
     assert(!result);
     semantics.visit(*static_cast<Decl_List*>(root));
     assert(!error_msg.size());
     executor.visit(*static_cast<Decl_List*>(root));

     //Parse main file
     current_line = 0;
     rewind(bfp);
     yyrestart(bfp);
     inject_herald = PROGRAM_HERALD;
     result = yyparse();
     assert(!result);

     //Run Pasithean_Echo_Visitor and use output to reset buffers
     Pasithean_Echo_Visitor pasithean_echoer;
     pasithean_echoer.visit(*static_cast<Decl_List*>(root));

     //Get number of lines and current from echoer
     int pos = ed.insert_position();
     string source_text = pasithean_echoer.get_textbuf_text();
     int lines = count(source_text.begin(),source_text.end(),'\n');
     int buffer_line = count(source_text.begin(),source_text.begin()+pos,'\n') + (key==FL_Left || key==FL_Up ? -1 : 1);

     //Reset main editor's style and text buffers
     text_buffer.text(source_text.c_str());
     style_buffer.text(pasithean_echoer.get_stylebuf_text().c_str());

     for(; pos < text_buffer.length() && text_buffer.text()[pos]!='\n'; pos++);
     ed.insert_position(pos);
     
     //Look for semantic errors
     semantics.visit(*static_cast<Decl_List*>(root));

     //Print error messages
     int i=1;
     ostringstream sout;
     for(const string& e : error_msg)
     {
          sout << i << ": " << e << endl;
          i++;
     }

     if(error_msg.size())
     {
          pseb_buffer.text(sout.str().c_str());
          return false;
     }

     //Run the executor
     try
     {
          executor.visit(*static_cast<Decl_List*>(root));

          //Reset pseb_buffer to contain diagnostics for current line
          pseb_buffer.text(executor.get_diagnostics_for_line(buffer_line).c_str());

          //Reset iob_buffer to contain I/O for full execution
          iob_buffer.text(executor.get_output().c_str());
     }
     catch(const char* msg)
     {
          pseb_buffer.text(msg);
          iob_buffer.text(executor.get_output().c_str());
          return false;
     }

     //Save file if requested
     if(filename!="")
     {
          ofstream fout{filename};
          fout << buffer << endl;
          if(!fout.good())
               fl_alert((("Could not save to file ")+filename).c_str());
     }
          
     old_text = buffer;
     return true;
}

static int shortcut_handler(int e_type)
{
     if(e_type!=FL_SHORTCUT)
          return 0;

     if(!(Fl::event_state()&FL_CTRL))
          return 0;

     ifstream fin;
     const char* temp_cstr;
     string temp_str;
     int iterations;
     switch(Fl::event_key())
     {
     case 'o': temp_cstr = fl_file_chooser("Select file to open","*.java","",0);
          if(!temp_cstr)
               return 0;
          w->copy_label(temp_cstr);
          filename = temp_cstr;
          fin.open(filename.c_str());
          getline(fin,temp_str,'\0');
          fin.close();
          text_buffer.text(temp_str.c_str());
          return 1;
     case 'i': temp_cstr = fl_input("Enter number of max iterations allowed to loops:","100");
          if(!temp_cstr)
               return 1;
          iterations = atoi(temp_cstr);
          if(iterations <= 0)
          {
               fl_alert("That input was invalid.");
               return 1;
          }
          executor.set_max_iterations(iterations);
          reset_needed = true;
          return 1;
     case 'r': executor.reset_input();
          reset_needed = true;
          return 1;
     case 'l':
          if(!mb->linenumber_width())
               mb->linenumber_width(30);
          else
               mb->linenumber_width(0);
          return 1;
     }

     return 0;
}

static void main_window_callback(Fl_Widget* w, void* ignored)
{
     if(Fl::event_key()==FL_Escape)
          return;
     exit(0);
}

extern int yydebug;
int main()
{
     #if DEBUG
     yydebug = 1;
     #endif
     
     //Create main UI interface

     //Main window
     w = new Fl_Window(1000, 500);
     w->callback(main_window_callback);
     w->resizable(w);

     //Main editor
     mb = new FLTK_Callback_Editor(0, 0, 1000, 300);
     mb->line_cb = &mb_callback;
     mb->buffer(text_buffer);
     mb->highlight_data(&style_buffer,styletable,STYLENUM,'\0',NULL,NULL);
     mb->resizable(w);

     //Read-only editor display, for when debugging
     Fl_Text_Display* mb_ro = new Fl_Text_Display(0,0,1000,300);
     mb_ro->buffer(text_buffer);
     mb->highlight_data(&style_buffer,styletable,STYLENUM,'\0',NULL,NULL);
     mb_ro->resizable(w);
     mb_ro->hide();

     //I/O editor
     Fl_Text_Display* iob = new Fl_Text_Display(0, 300, 500, 200);
     iob->buffer(iob_buffer);
     iob->resizable(w);

     //Error and debug information display
     Fl_Text_Display* pseb = new Fl_Text_Display(500, 300, 500, 200);
     pseb->buffer(pseb_buffer);
     pseb->resizable(w);

     w->end();
     w->show();

     language_mode = JAVA_MODE;

     Fl::add_handler(shortcut_handler);

     while(Fl::wait());
     return 0;
}
