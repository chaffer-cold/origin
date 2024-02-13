#include "string.h"
/*STM32部分*/
#include "stm32f1xx_hal.h"
#include "main.h"
#include "bsp_ili9341_lcd.h"
#include "fonts.h"
/*FreeRTOS部分*/
#include "cmsis_os.h"
#include "tim.h"


/*LVGL部分*/
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
//全局变量

lv_obj_t* base;
lv_obj_t*page1;
lv_obj_t*page2;
TimerHandle_t BeepTimerHandle;
void beepcallback(void);
void Switch_Handler(lv_event_t *e)
{
     
        //sendByte('A');
        lv_obj_t *sw_now=lv_event_get_target(e);
        lv_event_code_t event=lv_event_get_code(e);
        char* num=lv_event_get_user_data(e);
        static int car_state=0;
        static int buzzer_state=0;
        if(strcmp(num,"switch1")==0)
        {
            
            if(event==LV_EVENT_VALUE_CHANGED)
            {
            //    sendByte('C');
                if (lv_obj_has_state(sw_now,LV_STATE_CHECKED)) 
                {   //sendByte('T');
                    car_state=1;
                    //xQueueOverwrite(car_statemail,&car_state);
                } 
                else 
                {
                    //sendByte('F');
                    car_state=0;
                    //xQueueOverwrite(car_statemail,&car_state);
                }
            }
        }

        
        if(strcmp(num,"switch2")==0)
        {
            if(event==LV_EVENT_VALUE_CHANGED)
            {
                if (lv_obj_has_state(sw_now,LV_STATE_CHECKED)) 
                {
                    
                    //vTaskResume(LEDHandler);
                    
                } 
                else 
                {
                    //TIM_SetCompare4(TIM3,0);
                    //TIM_SetCompare1(TIM4,0);
                    //vTaskSuspend(LEDHandler);
                }
            }
        }

        if(strcmp(num,"switch3")==0)
        {
            if(event==LV_EVENT_VALUE_CHANGED)
            {
                if (lv_obj_has_state(sw_now,LV_STATE_CHECKED)) 
                {
                    
                    buzzer_state=1;
                    //xQueueOverwrite(buzzer_statemail,&buzzer_state);
                    
                } 
                else 
                {
                    buzzer_state=0;
                    //xQueueOverwrite(buzzer_statemail,&buzzer_statemail);
                }
            }
        }
}


void Button_Handler(lv_event_t *e)
{
    lv_obj_t* btn=lv_event_get_target(e);
    char* user_data=lv_event_get_user_data(e);
    if(strcmp(user_data,"button1")==0)
    {
        lv_scr_load(page2);
    }
    else if(strcmp(user_data,"button2")==0)
    {
        lv_scr_load(page1);
    }
    else if(strcmp(user_data,"button_led_on")==0)
    {
        LED_P->Mode=mode_on;
    }
    else if(strcmp(user_data,"button_led_off")==0)
    {
        LED_P->Mode=mode_off;
    }
    else if(strcmp(user_data,"button_beep")==0)
    {
        HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,ENABLE);
        xTimerStart(BeepTimerHandle,0);

    }
    else if(strcmp(user_data,"button_exti")==0)
    {
        ILI9341_BackLed_Control(DISABLE);
    }
}


void my_UIInit(void)
{
    //page1
    //需要三个开关一个按钮一个滑条
    //开关1:小车状态
    //开关2:灯的状态
    //开关3:蜂鸣器状态
    //按钮转化界面
    //创建对象
    
   
    
    page1=lv_obj_create(NULL);
    page2=lv_obj_create(NULL);
    //样式创建
    //lv_style_t *style_transp;
    //lv_style_init(style_transp);
    //lv_style_set_bg_opa(style_transp, LV_OPA_TRANSP);  // 设置背景透明度
    //lv_style_set_border_opa(style_transp, LV_OPA_TRANSP);  // 设置边框透明度


    //page1的元素
    
    //lv_obj_t *sw1=lv_switch_create(page1);
    //lv_obj_t *sw2=lv_switch_create(page1);
    //lv_obj_t *sw3=lv_switch_create(page1);

    lv_obj_t *button1=lv_btn_create(page1);
    lv_obj_t *button_led_on=lv_btn_create(page1);
    lv_obj_t *button_led_off=lv_btn_create(page1);
    lv_obj_t *button_beep=lv_btn_create(page1);
    lv_obj_t *button_exti=lv_btn_create(page1);
    
    //lv_obj_t  *label1=lv_label_create(sw1);
    //lv_obj_t  *label2=lv_label_create(sw2);
    //lv_obj_t  *label3=lv_label_create(sw3);
   // lv_obj_t  *label1=lv_label_create(sw1);
   // lv_obj_t  *label2=lv_label_create(sw2);
   // lv_obj_t  *label3=lv_label_create(sw3);
    //label4给按钮
    lv_obj_t  *label4=lv_label_create(button1);
    lv_obj_t  *label_button_led_on=lv_label_create(button_led_on);
    lv_obj_t  *label_button_led_off=lv_label_create(button_led_off);
    lv_obj_t  *label_button_beep=lv_label_create(button_beep);
    lv_obj_t  *label_button_exti=lv_label_create(button_exti);
    lv_label_set_text(label_button_led_on,"LED_On");
    lv_label_set_text(label_button_led_off,"LED_Off");
    lv_label_set_text(label_button_beep,"Beep");
    lv_label_set_text(label_button_exti,"exti");
    lv_label_set_text(label4,"Pic");
    lv_obj_set_size(button1,100,150);
    //lv_obj_add_state(sw2,LV_STATE_CHECKED);
    
    
    //page2的元素
    lv_obj_t *button2=lv_btn_create(page2);
    lv_obj_t *label5=lv_label_create(button2);
    lv_label_set_text(label5,LV_SYMBOL_NEW_LINE);
    //图片
    lv_obj_t *pic1=lv_img_create(page2);
    lv_obj_t *pic2=lv_img_create(page2);
    LV_IMG_DECLARE(gta6);
    LV_IMG_DECLARE(Elaina);
    lv_img_set_src(pic1, &gta6);
    lv_img_set_src(pic2, &Elaina);


    
    

    //page1设置位置
    lv_obj_set_pos(button_led_on, 150, 50);
    lv_obj_set_pos(button_led_off, 150, 125);
    lv_obj_set_pos(button_beep, 150, 200);
    lv_obj_set_pos(button_exti, 150, 275);
    //lv_obj_set_pos(sw1, 150, 50);
    //lv_obj_set_pos(sw2, 150, 125);
    //lv_obj_set_pos(sw3, 150, 200);
    lv_obj_set_pos(button1, 20, 40);
    //lv_obj_align(label1,LV_ALIGN_CENTER,0,10);
    //lv_obj_align(label2,LV_ALIGN_CENTER,0,10);
    //lv_obj_align(label3,LV_ALIGN_CENTER,0,10);
    lv_obj_align(label4,LV_ALIGN_CENTER,0,0);
    //page2设置位置
    lv_obj_set_pos(button2, 180, 10);
    lv_obj_set_pos(pic2,0,110);

    lv_obj_move_foreground(button2);
    //美化
    //page1
    
    lv_obj_t *sys_label = lv_label_create(page1);
    lv_label_set_text(sys_label, "Hello World!");
    lv_obj_set_pos(sys_label, 10, 10);
    //page2

    // 将按钮设置成透明
    //lv_obj_add_style(button2,style_transp,0);
    
    //对象交互
    
    //开关交互
    //lv_obj_add_event_cb(sw1,Switch_Handler,LV_EVENT_VALUE_CHANGED,"switch1");
    //lv_obj_add_event_cb(sw2,Switch_Handler,LV_EVENT_VALUE_CHANGED,"switch2");
    //lv_obj_add_event_cb(sw3,Switch_Handler,LV_EVENT_VALUE_CHANGED,"switch3");
    
    //按钮交互
    lv_obj_add_event_cb(button1,Button_Handler,LV_EVENT_CLICKED,"button1");
    lv_obj_add_event_cb(button2,Button_Handler,LV_EVENT_CLICKED,"button2");
    lv_obj_add_event_cb(button_led_on,Button_Handler,LV_EVENT_CLICKED,"button_led_on");
    lv_obj_add_event_cb(button_led_off,Button_Handler,LV_EVENT_CLICKED,"button_led_off");
    lv_obj_add_event_cb(button_beep,Button_Handler,LV_EVENT_CLICKED,"button_beep");
    lv_obj_add_event_cb(button_exti,Button_Handler,LV_EVENT_CLICKED,"button_exti");
    lv_scr_load(page1);
    //timer设置
    BeepTimerHandle=xTimerCreate("beep",500,DISABLE,"TIM_beep",beepcallback);
}
void beepcallback(void)
{
    HAL_GPIO_WritePin(Beep_GPIO_Port,Beep_Pin,DISABLE);

}