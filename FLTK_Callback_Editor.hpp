#pragma once

#include <FL/Fl_Text_Editor.H>

class FLTK_Callback_Editor : public Fl_Text_Editor
{
     using Fl_Text_Editor::Fl_Text_Editor;
     
public:
     bool (*line_cb)(FLTK_Callback_Editor& ed, int key); //returns true if ok to press enter or otherwise leave line
     int handle(int e);

private:
     bool ignore_next_key_up = false;
};
