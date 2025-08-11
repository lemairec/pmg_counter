/*

  MUIDemo.ino

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <MUIU8g2.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
#define LCD_1 23
#define LCD_2 17
#define LCD_3 16


U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, LCD_1, LCD_2, LCD_3, U8X8_PIN_NONE);



// End of constructor list



MUIU8G2 mui;



/*
  global variables which form the communication gateway between the user interface and the rest of the code
*/
uint8_t number_input = 2;       // variable where the user can input a number between 0 and 9
uint8_t number_input2 = 100;       // variable where the user can input a number between 0 and 9
uint8_t fruit_input = 2;
uint8_t fruit_input2 = 2;
uint8_t my_value3 = 0;
uint8_t color_input = 0;
uint8_t food_input = 0;
uint8_t checkbox_input = 0;
uint8_t direction_input = 0;
uint8_t text_input[4] = { ' ',' ',' ',' '} ;
uint8_t exit_code = 0;
uint16_t list_selection = 0;
uint16_t list_selection2 = 0;
uint16_t list_selection3 = 0;
uint16_t list_selection4 = 0;

uint8_t array_pos = 0;
uint8_t array_led_off_time[4] = { 10, 5, 3, 1};
uint8_t led_off_time = 0;
uint8_t array_led_on_time[4] = { 10, 5, 3, 1};
uint8_t led_on_time = 0;


uint8_t muif_array_pos_selection(mui_t *ui, uint8_t msg)
{
  uint8_t return_value = 0; 
  switch(msg)
  {
    case MUIF_MSG_FORM_START:
      led_off_time = array_led_off_time[array_pos];
      led_on_time = array_led_on_time[array_pos];
      return_value = mui_u8g2_u8_min_max_wm_mse_pi(ui, msg);
      break;
    case MUIF_MSG_FORM_END:
      return_value = mui_u8g2_u8_min_max_wm_mse_pi(ui, msg);
      break;
    case MUIF_MSG_CURSOR_SELECT:
    case MUIF_MSG_EVENT_NEXT:
    case MUIF_MSG_EVENT_PREV:
      array_led_off_time[array_pos] = led_off_time;
      array_led_on_time[array_pos] = led_on_time;
      return_value = mui_u8g2_u8_min_max_wm_mse_pi(ui, msg);
      led_off_time = array_led_off_time[array_pos];
      led_on_time = array_led_on_time[array_pos];
      break;
    default:
      return_value = mui_u8g2_u8_min_max_wm_mse_pi(ui, msg);
  }
  return return_value;
}



const char *animals[] = { "Bird", "Bison", "Cat", "Crow", "Dog", "Elephant", "Fish", "Gnu", "Horse", "Koala", "Lion", "Mouse", "Owl", "Rabbit", "Spider", "Turtle", "Zebra" };

uint16_t list_get_cnt(void *data)
{
  return sizeof(animals)/sizeof(*animals);    /* number of animals */
}

const char *list_get_str(void *data, uint16_t index)
{
  return animals[index];
}


uint16_t menu_get_cnt(void *data)
{
  return 10;    /* number of menu entries */
}

const char *menu_get_str(void *data, uint16_t index)
{
  static const char *menu[] = 
  { 
    MUI_0 "Goto Main Menu",
    MUI_10 "Enter a number",
    MUI_11 "Parent/Child Selection",
    MUI_13 "Checkbox",
    MUI_14 "Radio Selection",
    MUI_15 "Text Input",
    MUI_16 "Single Line Selection",
    MUI_17 "List Line Selection",
    MUI_18 "Parent/Child List",
    MUI_20 "Array Edit",
  };
  return menu[index];
}



uint8_t mui_hrule(mui_t *ui, uint8_t msg)
{
  u8g2_t *u8g2 = mui_get_U8g2(ui);
  switch(msg)
  {
    case MUIF_MSG_DRAW:
      u8g2_DrawHLine(u8g2, 0, mui_get_y(ui), u8g2_GetDisplayWidth(u8g2));
      break;
  }
  return 0;
}

/* 

  User interface fields list. Each entry is defined with the MUIF macro MUIF(id,cflags,data,cb)
  Arguments are:
    id: 
      A string with exactly two characters. This is the unique "id" of the field, which is later used in the form definition string (fds)
      There are some special id's: ".L" for text labels and ".G" for a goto form command. 
    cflags: 
      Flags, which define static (constant) properties of the field. Currently this is either 0 or MUIF_CFLAG_IS_CURSOR_SELECTABLE which marks the field as editable.
    data: 
      A pointer to a local variable, where the result of an editiable field is stored. Currently this is a pointer to uint8_t in most cases.
      It depends in the callback function (cb) whether this is used or what kind of data is stored
    cb:
      A callback function.
      The callback function will receive messages and have to react accordingly to the message. Some predefined callback functions are avilable in mui_u8g2.c    
  
*/


muif_t muif_list[] = {
  /* normal text style */
  MUIF_U8G2_FONT_STYLE(0, u8g2_font_helvR08_tr),
  
  /* bold text style */
  MUIF_U8G2_FONT_STYLE(1, u8g2_font_helvB08_tr),

  /* monospaced font */
  MUIF_U8G2_FONT_STYLE(2, u8g2_font_profont12_tr),
  
  /* food and drink */
  MUIF_U8G2_FONT_STYLE(3, u8g2_font_streamline_food_drink_t),
  
  /* horizontal line (hrule) */
  MUIF_RO("HR", mui_hrule),

  /* main menu */
  MUIF_RO("GP",mui_u8g2_goto_data),
  MUIF_BUTTON("GC", mui_u8g2_goto_form_w1_pi),


  /* Goto Form Button where the width is equal to the size of the text, spaces can be used to extend the size */
  MUIF_BUTTON("G1", mui_u8g2_btn_goto_wm_fi),
  
  /* input for a number between 0 to 9 */
  MUIF_U8G2_U8_MIN_MAX("IN", &number_input, 0, 9, mui_u8g2_u8_min_max_wm_mse_pi),

  /* input for a number between 0 to 100 */
  MUIF_U8G2_U8_MIN_MAX("IH", &number_input2, 0, 100, mui_u8g2_u8_min_max_wm_mud_pi),
  
  /* input for text with four chars  */
  MUIF_VARIABLE("T0", text_input+0, mui_u8g2_u8_char_wm_mud_pi),
  MUIF_VARIABLE("T1", text_input+1, mui_u8g2_u8_char_wm_mud_pi),
  MUIF_VARIABLE("T2", text_input+2, mui_u8g2_u8_char_wm_mud_pi),
  MUIF_VARIABLE("T3", text_input+3, mui_u8g2_u8_char_wm_mud_pi),
  
  /* input for a fruit (0..3), implements a selection, where the user can cycle through the options  */
  MUIF_VARIABLE("IF",&fruit_input,mui_u8g2_u8_opt_line_wa_mse_pi),
  MUIF_VARIABLE("IG",&fruit_input2,mui_u8g2_u8_opt_line_wa_mud_pi),
  
  /* checkbox */
  MUIF_VARIABLE("CB",&checkbox_input,mui_u8g2_u8_chkbox_wm_pi),
  
  /* the following two fields belong together and implement a single selection combo box to select a color */
  MUIF_VARIABLE("IC",&color_input,mui_u8g2_u8_opt_parent_wm_pi),
  MUIF_VARIABLE("OC",&color_input,mui_u8g2_u8_opt_radio_child_w1_pi),

  MUIF_VARIABLE("ID",&food_input,mui_u8g2_u8_opt_parent_wm_pi),
  MUIF_VARIABLE("OD",&food_input,mui_u8g2_u8_opt_child_wm_pi),

  /* radio button style */
  MUIF_VARIABLE("RS",&direction_input,mui_u8g2_u8_radio_wm_pi),
  
  MUIF_U8G2_U16_LIST("L1", &list_selection, NULL, list_get_str, list_get_cnt, mui_u8g2_u16_list_line_wa_mse_pi),
  MUIF_U8G2_U16_LIST("L2", &list_selection2, NULL, list_get_str, list_get_cnt, mui_u8g2_u16_list_line_wa_mud_pi),
  
  MUIF_U8G2_U16_LIST("LP", &list_selection3, NULL, list_get_str, list_get_cnt, mui_u8g2_u16_list_parent_wm_pi),
  MUIF_U8G2_U16_LIST("LC", &list_selection3, NULL, list_get_str, list_get_cnt, mui_u8g2_u16_list_child_w1_pi),

  MUIF_U8G2_U16_LIST("LG", &list_selection4, NULL, menu_get_str, menu_get_cnt, mui_u8g2_u16_list_goto_w1_pi),

  /* MUI_GOTO uses the fixed ".G" id and is intended for goto buttons. This is a full display width style button */  
  MUIF_GOTO(mui_u8g2_btn_goto_w1_pi),
  
  /* register MUI_LABEL, which is used to place read only text on a form */
  MUIF_U8G2_LABEL(),

  /* array example */
  MUIF_U8G2_U8_MIN_MAX("AP", &array_pos, 0, 3, muif_array_pos_selection),
  MUIF_U8G2_U8_MIN_MAX("AF", &led_off_time, 0, 20, mui_u8g2_u8_min_max_wm_mse_pi),
  MUIF_U8G2_U8_MIN_MAX("AN", &led_on_time, 0, 20, mui_u8g2_u8_min_max_wm_mse_pi),

};

/*
  The form definition string (fds) defines all forms and the fields on those forms.
  A new form always starts with MUI_FORM(u). The form ends with the next MUI_FORM() or the end of the fds.
  Inside the form use fields or the style command (MUI_STYLE)
  The fields are define with
    MUI_XY(id, x, y)                     Field 'id' without extra argument or text placed at the specified xy position
    MUI_XYT(id, x, y, text)            Field 'id' with the specified test at position xy
    MUI_XYA(id, x, y, a)                 Field 'id' with argument 'a' at position xy
    MUI_XYAT(id, x, y, a, text)         Field 'id' with argument and text placed at position xy
    MUI_LABEL(x,y,text)                 Field '.L' (usually some readonly text) placed at position xy
    MUI_GOTO(x,y,n,text)                Field '.G', usually a button placed at xy, which activates form n 

  Note:
    MUI_LABEL(x,y,text) is the same as MUI_XYT(".L", x, y, text)
    MUI_GOTO(x,y,text) is the same as MUI_XYAT(".G", x, y, n, text)
    
*/



fds_t fds_data[] = 


/* top level main menu */
MUI_FORM(0)
MUI_STYLE(1)
MUI_LABEL(5,10, "Main Menu")
MUI_XY("HR", 0,13)
MUI_STYLE(0)

MUI_DATA("GP", 
    MUI_10 "Enter a number|"
    MUI_11 "Parent/Child Selection|"
    MUI_13 "Checkbox|"
    MUI_14 "Radio Selection|"
    MUI_15 "Text Input|"
    MUI_16 "Single Line Selection|"
    MUI_17 "List Line Selection|"
    MUI_18 "Parent/Child List|"
    MUI_20 "Array Edit|"
    MUI_3 "Alternative Menu"
    )
MUI_XYA("GC", 5, 25, 0) 
MUI_XYA("GC", 5, 37, 1) 
MUI_XYA("GC", 5, 49, 2) 
MUI_XYA("GC", 5, 61, 3) 


MUI_FORM(3)
MUI_STYLE(1)
MUI_LABEL(5,10, "Alternative Menu")
MUI_XY("HR", 0,13)
MUI_STYLE(0)
MUI_XYA("LG", 5, 25, 0) 
MUI_XYA("LG", 5, 37, 1) 
MUI_XYA("LG", 5, 49, 2) 
MUI_XYA("LG", 5, 61, 3) 


/* number entry demo */
MUI_FORM(10)
MUI_STYLE(1)
MUI_LABEL(5,10, "Number Menu")
MUI_XY("HR", 0,13)
MUI_STYLE(0)

MUI_LABEL(5,27, "Number [mse]:")
MUI_XY("IN",76, 27)
MUI_LABEL(5,41, "Number [mud]:")
MUI_XY("IH",76, 41)

MUI_XYAT("G1",64, 59, 0, " OK ")

/* parent / child selection */
MUI_FORM(11)
MUI_STYLE(1)
MUI_LABEL(5,10, "Parent/Child Selection")
MUI_XY("HR", 0,13)
MUI_STYLE(0)

MUI_LABEL(5,24, "Color:")
MUI_XYAT("IC",80, 24, 12, "red|orange|yellow|green|cyan|azure|blue|violet|magenta|rose")     /* jump to sub form 12 */

MUI_LABEL(5,42, "We need:")
MUI_STYLE(3)
MUI_XYAT("ID",80, 49, 21, "\x30|\x31|\x32|\x33|\x34|\x35|\x36|\x37|\x38|\x39|\x40|\x41|\x42|\x43") 

MUI_STYLE(0)
MUI_XYAT("G1",64, 60, 0, " OK ")


/* combo box color selection */
MUI_FORM(12)
MUI_STYLE(1)
MUI_LABEL(5,10, "Color Selection")
MUI_XY("HR", 0,13)
MUI_STYLE(0)
MUI_XYA("OC", 5, 30, 0) /* takeover the selection text from calling field ("red") */
MUI_XYA("OC", 5, 42, 1) /* takeover the selection text from calling field ("green") */
MUI_XYA("OC", 5, 54, 2)  /* */
/* no ok required, clicking on the selection, will jump back */

/* Checkbox demo */
MUI_FORM(13)
MUI_STYLE(1)
MUI_LABEL(5,10, "Checkbox Menu")
MUI_XY("HR", 0,13)
MUI_STYLE(0)

MUI_LABEL(5,30, "Checkbox:")
MUI_XY("CB",60, 30)

MUI_XYAT("G1",64, 59, 0, " OK ")

/* Radio selection demo */
MUI_FORM(14)
MUI_STYLE(1)
MUI_LABEL(5,10, "Radio Selection Menu")
MUI_XY("HR", 0,13)
MUI_STYLE(0)

MUI_XYAT("RS",10, 28,0,"North")
MUI_XYAT("RS",10, 40,1,"South")

MUI_XYAT("RS",65, 28,2,"East")
MUI_XYAT("RS",65, 40,3,"West")

MUI_XYAT("G1",64, 59, 0, " OK ")

/* text demo */
MUI_FORM(15)
MUI_STYLE(1)
MUI_LABEL(5,10, "Enter Text Menu")
MUI_XY("HR", 0,13)
MUI_STYLE(0)

MUI_LABEL(5,30, "Text:")
MUI_STYLE(2)
MUI_XY("T0",40, 30)
MUI_XY("T1",48, 30)
MUI_XY("T2",56, 30)
MUI_XY("T3",64, 30)
MUI_STYLE(0)

MUI_XYAT("G1",64, 59, 0, " OK ")

/* single line selection */
MUI_FORM(16)
MUI_STYLE(1)
MUI_LABEL(5,10, "Single Line Selection")
MUI_XY("HR", 0,13)
MUI_STYLE(0)

MUI_LABEL(5,29, "Fruit [mse]:")
MUI_XYAT("IF",60, 29, 60, "Banana|Apple|Melon|Cranberry")

MUI_LABEL(5,43, "Fruit [mud]:")
MUI_XYAT("IG",60, 43, 60, "Banana|Apple|Melon|Cranberry")

MUI_XYAT("G1",64, 59, 0, " OK ")

/* long list example with list callback functions */
MUI_FORM(17)
MUI_STYLE(1)
MUI_LABEL(5,10, "List Line Selection")
MUI_XY("HR", 0,13)
MUI_STYLE(0)

MUI_LABEL(5,29, "List [mse]:")
MUI_XYA("L1",60, 29, 60)
MUI_LABEL(5,43, "List [mud]:")
MUI_XYA("L2",60, 43, 60)


MUI_XYAT("G1",64, 59, 0, " OK ")

/* parent / child selection */
MUI_FORM(18)
MUI_STYLE(1)
MUI_LABEL(5,10, "Parent/Child List")
MUI_XY("HR", 0,13)
MUI_STYLE(0)

MUI_LABEL(5,29, "Animal:")
MUI_XYA("LP",50, 29, 19)     /* jump to sub form 19 */
MUI_XYAT("G1",64, 59, 0, " OK ")

/* combo box color selection */
MUI_FORM(19)
MUI_STYLE(1)
MUI_LABEL(5,10, "Animal Selection")
MUI_XY("HR", 0,13)
MUI_STYLE(0)
MUI_XYA("LC", 5, 30, 0) /* takeover the selection text from calling field ("red") */
MUI_XYA("LC", 5, 42, 1) /* takeover the selection text from calling field ("green") */
MUI_XYA("LC", 5, 54, 2)  /* */
/* no ok required, clicking on the selection, will jump back */

MUI_FORM(20)
MUI_STYLE(1)
MUI_LABEL(5,10, "Array Edit")
MUI_XY("HR", 0,13)
MUI_STYLE(0)

MUI_LABEL(5,24, "Position:")
MUI_XY("AP",76, 24)
MUI_LABEL(5,35, "LED off:")
MUI_XY("AF",76, 35)
MUI_LABEL(5,46, "LED on:")
MUI_XY("AN",76, 46)
MUI_XYAT("G1",64, 59, 0, " OK ")


/* combo box Food & Drink Selection, called from form 11 */
MUI_FORM(21)
MUI_STYLE(1)
MUI_LABEL(5,10, "Food & Drink Selection")
MUI_XY("HR", 0,13)
MUI_STYLE(3)

MUI_XYA("OD", 3, 45, 0)
MUI_XYA("OD", 28, 45, 1)
MUI_XYA("OD", 53, 45, 2)
MUI_XYA("OD", 78, 45, 3)
MUI_XYA("OD", 103, 45, 4)


;


void setup(void) {

  // U8g2 SH1106 Proto-Shield
  //u8g2.begin(/* menu_select_pin= */ 2, /* menu_next_pin= */ 4, /* menu_prev_pin= */ 7, /* menu_up_pin= */ 6, /* menu_down_pin= */ 5, /* menu_home_pin= */ 3);

  // U8g2 SH1106 MUI Test Shield
  u8g2.begin(35, 33, 31 , /* menu_up_pin= */ U8X8_PIN_NONE, /* menu_down_pin= */ U8X8_PIN_NONE, /* menu_home_pin= */ U8X8_PIN_NONE);  
    

  u8g2.setFontMode(1);
  mui.begin(u8g2, fds_data, muif_list, sizeof(muif_list)/sizeof(muif_t));
  mui.gotoForm(/* form_id= */ 0, /* initial_cursor_position= */ 0);
}

uint8_t is_redraw = 1;

void loop(void) {

  /* check whether the menu is active */
  if ( mui.isFormActive() )
  {

    /* if so, then draw the menu */

    if ( is_redraw ) {
      u8g2.firstPage();
      do {
          mui.draw();
      } while( u8g2.nextPage() );
      is_redraw = 0;
    }
    
    /* handle events */
    
    switch(u8g2.getMenuEvent()) {
      case U8X8_MSG_GPIO_MENU_SELECT:
        mui.sendSelect();
        is_redraw = 1;
        break;
      case U8X8_MSG_GPIO_MENU_NEXT:
        mui.nextField();
        is_redraw = 1;
        break;
      case U8X8_MSG_GPIO_MENU_PREV:
        mui.prevField();
        is_redraw = 1;
        break;
    }
    
  }
  else
  {
    /* do something else, maybe clear the screen */
  }

}