#include <U8g2lib.h>

#define LCD_1 23
#define LCD_2 17
#define LCD_3 16

#define RAMPS_D9 9
#define RAMPS_T2 15


//2 3 6 9

#include <MUIU8g2.h>
/**                  Gadgets3D Smart Adapter
     *               ------                                 ------
     *      33 4-11 | 1  2 | 4-12 31        (MISO) 50 3-03 | 1  2 | 3-05 52 (SCK)
     *  LCD 17 4-17 | 3  4 | 4-18 16 LCD           35 4-10 | 3  4 | 3-06 53
     *  LCD 23 4-16   5  6 | 4-15 25               37 4-09   5  6 | 3-04 51 (MOSI)
     *      27 4-14 | 7  8 | 4-13 29               49 3-02 | 7  8 | 4-07 41
     *   (GND) 4-02 | 9 10 | 4-01 (5V)                  -- | 9 10 | --
     *              ------                                  ------
     *               EXP1                                    EXP2
     */

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, LCD_1, LCD_2, LCD_3, U8X8_PIN_NONE);


MUIU8G2 mui;

/*
  global variables which form the communication gateway between the user interface and the rest of the code
*/
uint8_t number_input = 2;       // variable where the user can input a number between 0 and 9
uint8_t exit_code = 0;                  // return value from the menu system

/* 
  MUIF table: Each entry defines a field, which can be used in FDS to describe a form.
*/


muif_t muif_list[] = {
    /* normal text style */
    MUIF_U8G2_FONT_STYLE(0, u8g2_font_helvR08_tr),
    
    /* Leave the menu system */
    MUIF_VARIABLE("LV",&exit_code,mui_u8g2_btn_exit_wm_fi),
    
    /* input for a number between 0 to 9 */
    MUIF_U8G2_U8_MIN_MAX("IN", &number_input, 0, 9, mui_u8g2_u8_min_max_wm_mse_pi),
    
    /* MUI_LABEL is used to place fixed text on the screeen */
    MUIF_LABEL(mui_u8g2_draw_text)
};

/*
  The form definition string (fds) defines all forms and the fields on those forms.
  A new form always starts with MUI_FORM(u). The form ends with the next MUI_FORM() or the end of the fds.
  Inside the form use fields or the style command (MUI_STYLE)
  The fields are placed on the form with
    MUI_XY(id, x, y)                     Field 'id' without extra argument or text placed at the specified xy position
    MUI_XYT(id, x, y, text)            Field 'id' with the specified test at position xy
    MUI_XYA(id, x, y, a)                 Field 'id' with argument 'a' at position xy
    MUI_XYAT(id, x, y, a, text)         Field 'id' with argument and text placed at position xy
    MUI_LABEL(x, y, text)                 Place "text" on the form. Can be used only if "MUIF_LABEL(mui_u8g2_draw_text)" is available in MUIF table.

*/


fds_t fds_data[] MUI_PROGMEM = 
MUI_FORM(1)
MUI_STYLE(0)
MUI_LABEL(5,12, "Countdown Time")
MUI_LABEL(5,30, "Seconds:")
MUI_XY("IN",60, 30)
MUI_XYT("LV",64, 59, " OK ")
;


void setup(void) {
    Serial.begin(115200);
    u8g2.begin(35, 33, 31 , /* menu_up_pin= */ U8X8_PIN_NONE, /* menu_down_pin= */ U8X8_PIN_NONE, /* menu_home_pin= */ U8X8_PIN_NONE);

    mui.begin(u8g2, fds_data, muif_list, sizeof(muif_list)/sizeof(muif_t));
    mui.gotoForm(/* form_id= */ 1, /* initial_cursor_position= */ 0);
    pinMode(RAMPS_D9, OUTPUT);
    pinMode(RAMPS_T2, INPUT);
    Serial.println("setup end");
}


uint8_t is_redraw = 1;
long milliseconds = 0;
int count = 0;
int zero = 1;

void loop(void) {

    int i = analogRead(RAMPS_T2);

    if(i>10 && zero){
        zero = 0;
        count++;
    }

    if(i<10){
        zero=1;
    }

    Serial.print(count);
    Serial.print("  ");
    Serial.println(i);


    
  /* check whether the menu is active */
    if ( mui.isFormActive() ) {

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
        
        /* did the user deactivate the menu? If so, start the countdown */
        
        if ( mui.isFormActive() ){
            milliseconds = ((long)number_input) * 1000;
        }
          
    } else {
    
        /* menu not active: show countdown */
        
        if ( milliseconds <= 0 ) {
            /* countdown reached 0: activate menu again */
            /* The exit button has stored its location: restoreForm will */
            /* activate MUI and will put focus on the ok button */
            mui.restoreForm();
            digitalWrite(RAMPS_D9, LOW);     
        } else {
            /* execute countdown */
            u8g2.setFont(u8g2_font_helvR08_tr);
            u8g2.firstPage();
            do {
                u8g2.setCursor(0,20);
                u8g2.print(milliseconds);
            } while( u8g2.nextPage() );
            milliseconds -= 100;
            delay(100);
            digitalWrite(RAMPS_D9, HIGH);
    
        }
    }
}