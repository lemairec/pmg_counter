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

#define RAMPS_D9 9
#define RAMPS_T2 15



U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, LCD_1, LCD_2, LCD_3, U8X8_PIN_NONE);
MUIU8G2 mui;

uint8_t is_redraw = 1;

/*
  global variables which form the communication gateway between the user interface and the rest of the code
*/

int m_pmg_count = 0;
int m_pmg_zero = 1;

int m_pmg_run = 0;

void setup_pmg(){
    Serial.begin(115200);
    pinMode(RAMPS_D9, OUTPUT);
    pinMode(RAMPS_T2, INPUT);
    Serial.println("setup_pmg end");
}

void loop_pmg(){
    int i = analogRead(RAMPS_T2);

    if(i>10 && m_pmg_zero){
        m_pmg_zero = 0;
        m_pmg_count++;
        is_redraw = 1;
    }

    if(i<10){
        m_pmg_zero=1;
    }

    if(m_pmg_run){

    }

    Serial.print(m_pmg_count);
    Serial.print("  ");
    Serial.println(i);
}


long stop_watch_timer = 0;                      // stop watch timer 1/100 seconds 
long stop_watch_millis = 0;                      // millis() value, when the stop watch was started
uint8_t is_stop_watch_running = 1;          // defines the current state of the stop watch: running or not running

/* draw the current stop watch value */
uint8_t mui_draw_current_timer(mui_t *ui, uint8_t msg) {
    if ( msg == MUIF_MSG_DRAW ) {
        u8g2.setCursor(mui_get_x(ui), mui_get_y(ui));
        u8g2.print(m_pmg_count);
    }
    return 0;
}

/* start the stop watch */
uint8_t mui_start_current_timer(mui_t *ui, uint8_t msg) {
    if ( msg == MUIF_MSG_FORM_START ) {
        is_stop_watch_running = 1;
        m_pmg_run = 1;
        stop_watch_millis = millis();
        stop_watch_timer = 0;
    }
    return 0;
}

/* stop the stop watch timer */
uint8_t mui_stop_current_timer(mui_t *ui, uint8_t msg) {
    if ( msg == MUIF_MSG_FORM_START ){
        is_stop_watch_running = 0;
        m_pmg_run = 0;
    }
    return 0;
}



muif_t muif_list[] = {
    MUIF_U8G2_FONT_STYLE(0, u8g2_font_helvR08_tr),

    MUIF_RO("CT", mui_draw_current_timer),
    MUIF_RO("ST", mui_start_current_timer),
    MUIF_RO("SO", mui_stop_current_timer),
    MUIF_BUTTON("GO", mui_u8g2_btn_goto_wm_fi),
    MUIF_LABEL(mui_u8g2_draw_text)
};


fds_t fds_data[] = 
MUI_FORM(1)
MUI_AUX("SO")                      // this will stop the stop watch time once this form is entered
MUI_STYLE(0)
MUI_LABEL(5,12, "Stopwatch")
MUI_XY("CT", 5, 24)
MUI_XYAT("GO",20, 36, 2, " Start ")     // jump to the second form to start the timer

MUI_FORM(2)
MUI_AUX("ST")                      // this will start the stop watch time once this form is entered
MUI_STYLE(0)
MUI_LABEL(5,12, "Stopwatch")
MUI_XY("CT", 5, 24)
MUI_XYAT("GO",20, 36, 1, " Stop ")      // jump to the first form to stop the timer
;


void setup(void) {
    setup_pmg();
    u8g2.begin(35, 33, 31 , /* menu_up_pin= */ U8X8_PIN_NONE, /* menu_down_pin= */ U8X8_PIN_NONE, /* menu_home_pin= */ U8X8_PIN_NONE);  
    mui.begin(u8g2, fds_data, muif_list, sizeof(muif_list)/sizeof(muif_t));
    mui.gotoForm(/* form_id= */ 1, /* initial_cursor_position= */ 0);
}


long milliseconds = 0;

void loop(void) {
    loop_pmg();
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
        
        /* update the stop watch timer */
        if ( is_stop_watch_running != 0 ) {
            stop_watch_timer = millis() - stop_watch_millis;
            is_redraw = 1;
        }
        
    } else {
        /* the menu should never become inactive, but if so, then restart the menu system */
        mui.gotoForm(/* form_id= */ 1, /* initial_cursor_position= */ 0);
    }
}