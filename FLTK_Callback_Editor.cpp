#include "FLTK_Callback_Editor.hpp"

#include <FL/Fl.H>

int FLTK_Callback_Editor::handle(int e)
{
     switch(e)
     {
     case FL_KEYUP:
          if(ignore_next_key_up)
          {
               ignore_next_key_up = false;
               return 1;
          }
          break;

     case FL_KEYDOWN:
          int key = Fl::event_key();
          switch(key)
          {
          case FL_Left:
               if(!(insert_position() && buffer()->char_at(insert_position()-1)=='\n'))
                    break;
               else
                    goto skip;
          case FL_Right:
               if(!(insert_position()==buffer()->length() || buffer()->char_at(insert_position())=='\n'))
                    break;
          case FL_Enter: case FL_KP_Enter: case FL_Up: case FL_Down:
          skip:
                    if(!line_cb(*this,key))
               {
                    ignore_next_key_up = true;
                    return 1;
               }
          }
          break;
     }
     return Fl_Text_Editor::handle(e);
}
