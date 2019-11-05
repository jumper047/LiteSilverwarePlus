#include "osd.h"
#include "drv_serial.h"
#include "drv_time.h"
#include "math.h"
#include <stdlib.h>
#include "drv_osd.h"



#define AETR  ((-0.65f > rx[Yaw]) && (0.3f < rx[Throttle]) && (0.7f > rx[Throttle]) && (0.5f < rx[Pitch]) && (-0.3f < rx[Roll]) && (0.3f > rx[Roll]))
#define TAER  ((-0.65f > rx[Yaw]) && (-0.3f < rx[Pitch]) && (0.3f > rx[Pitch]) && (0.7f < rx[Roll]) && (0.7f > rx[Throttle]) && (0.3f < rx[Throttle]))


extern void flash_load( void);
extern void flash_save( void);
extern void flash_hard_coded_pid_identifier(void);

extern unsigned char OSD_DATA[15];
char save_motor_dir[4] = {1,0,0,1};

extern char aux[16];
extern unsigned int osd_count;
extern unsigned int vol;
extern float electricCur;
extern float rx[4];
extern float pidkp[PIDNUMBER];  
extern float pidki[PIDNUMBER];	
extern float pidkd[PIDNUMBER];
extern int number_of_increments[3][3];
extern unsigned long lastlooptime;
extern float vbattfilt;

unsigned char powerlevel = 0;
unsigned char channel = 0;
unsigned char mode = 0;
unsigned char sa_flag = 0;
unsigned char aetr_or_taer=0;
unsigned char showcase = 0;
extern unsigned char rx_switch;
unsigned char motorDir[4] = {1,0,0,1};
char down_flag = 0;
char up_flag = 0;
char right_flag = 0;
char left_flag = 0;
char menu_flag = 0;
char motor_sta = 0x00;

#ifdef Lite_OSD

Menu_List main_menu,main_menu_head;
Menu_List PID_menu,PID_menu_head;
Menu_List Motor_menu,Motor_menu_head;
Menu_List Receiver_menu,Receiver_menu_head;
Menu_List Menu_pointer;

void osdMenuInit(void)
{
    main_menu = CreateDbCcLinkList(4,0);
	main_menu_head = main_menu;
	
	PID_menu = CreateDbCcLinkList(9,1);
	PID_menu_head = PID_menu;
	
	Motor_menu = CreateDbCcLinkList(4,2);
	Motor_menu_head = Motor_menu;
	
	Receiver_menu = CreateDbCcLinkList(1,3);
	Receiver_menu_head = Receiver_menu;
    
	Menu_pointer = main_menu;    
}


void osd_setting()
{
    if(!showcase)
    {
        if(AETR || TAER )
        {
            int a;
            menu_flag = 1;
            showcase = 1;
            for(a=0;a<3;a++)
            {
                    PID_menu->PID_value = pidkp[a];
                    PID_menu = PID_menu->next;
            }
            
            for(a=0;a<3;a++)
            {
                    PID_menu->PID_value = pidki[a];
                    PID_menu = PID_menu->next;
            }
            
            for(a=0;a<3;a++)
            {
                    PID_menu->PID_value = pidkd[a];
                    PID_menu = PID_menu->next;
            }
            PID_menu = PID_menu_head;
            int i;
            for(i=0;i<4;i++)
            {
                    if( save_motor_dir[i] == 0x00)
                    {
                            motor_sta &= ~(0x01<<i);
                    }
                    else if(save_motor_dir[i] == 0x01)
                    {
                            motor_sta |= (0x01<<i);
                    }
                    Motor_menu->dir = save_motor_dir[i];
                    Motor_menu = Motor_menu->next;
            }
            Motor_menu = Motor_menu_head;	
        }
    }

    if(1 == menu_flag)
    {
		if((rx[Pitch] < -0.6f) && (down_flag == 1))
		{
			  Menu_pointer = Menu_pointer->next;
				down_flag = 0;
		}		
		
		if((rx[Pitch] > 0.6f) && (up_flag == 1))
		{
			  Menu_pointer = Menu_pointer->prior;
				up_flag = 0;
		}
		
		if((rx[Pitch]) < 0.6f && (rx[Pitch] > -0.6f))
		{
				up_flag = 1;
				down_flag = 1;
		}
		
		/******************************************************************/
		if((rx[Roll] > 0.6f) && right_flag == 1) 
		{
            if(0 == Menu_pointer->menu_class && 0 == Menu_pointer->menu_index) 
            {
                Menu_pointer = PID_menu_head;
                showcase = 2;
            }		
            else if(1 == Menu_pointer->menu_class)
            {
                int a;
                Menu_pointer->PID_value += 0.01f;
                if(Menu_pointer->PID_value >= 100)
                {
                        Menu_pointer->PID_value = 100;
                }
                PID_menu = PID_menu_head;
                for(a=0;a<3;a++)
                {
                        pidkp[a] = PID_menu->PID_value;
                        PID_menu = PID_menu->next;
                }
                
                for(a=0;a<3;a++)
                {
                        pidki[a] = PID_menu->PID_value;
                        PID_menu = PID_menu->next;
                }
                
                for(a=0;a<3;a++)
                {
                        pidkd[a] = PID_menu->PID_value;
                        PID_menu = PID_menu->next;
                }
            }
            
            if(1 == Menu_pointer->menu_class && 9 == Menu_pointer->menu_index)
            {
                    Menu_pointer = main_menu_head;
                    PID_menu = PID_menu_head;
                    showcase = 1;
            }
            if(0 == Menu_pointer->menu_class  && 1 == Menu_pointer->menu_index) 
            {
                  Menu_pointer = Motor_menu_head;
                  showcase = 3;
            }
            else if(2 == Menu_pointer->menu_class)
            {
              int i;
                Menu_pointer->dir ++;
                if(Menu_pointer->dir > 1)
                {
                        Menu_pointer->dir = 1;
                }
                Motor_menu = Motor_menu_head;
                for(i=0;i<4;i++)
                {
                        if(Motor_menu->dir == 0)
                        {
                                motor_sta &= ~(0x01<<i);
                        }
                        else
                        {
                                motor_sta |= (0x01<<i);
                        }
                        Motor_menu = Motor_menu->next;
                }
            }
            
            if(2 == Menu_pointer->menu_class && 4 == Menu_pointer->menu_index)
            {
                int i;
                showcase = 1;
                Menu_pointer = main_menu_head;
                Motor_menu = Motor_menu_head;
                for(i=0;i<4;i++)
                {
                        save_motor_dir[i] = Motor_menu->dir;
                        Motor_menu = Motor_menu->next;
                }
                Motor_menu = Motor_menu_head;
            }
            
            
            if(0 == Menu_pointer->menu_class && 2 == Menu_pointer->menu_index)
            {
                showcase =4;
                Menu_pointer = Receiver_menu_head;
            }
            else if(3 == Menu_pointer->menu_class)
            {
                if(aux[LEVELMODE])
                {
                    if(Menu_pointer->menu_index ==0)
                    {
                        aetr_or_taer = !aetr_or_taer;
                    }
                    else
                    {
                        Menu_pointer = main_menu_head;
                        showcase =1;
                    }
                }
            }
            if(0 == Menu_pointer->menu_class && 3 == Menu_pointer->menu_index)
            {
                flash_hard_coded_pid_identifier();					
                flash_save( );
                flash_load( );
                extern int number_of_increments[3][3];
                
                for( int i = 0 ; i < 3 ; i++)
                    for( int j = 0 ; j < 3 ; j++)
                        number_of_increments[i][j] = 0; 
                showcase = 0;
               delay(1000);
               NVIC_SystemReset();
            }
            
            if(0 == Menu_pointer->menu_class && 4 == Menu_pointer->menu_index)   //menu exit
            {
                //init main menu index paramenter
                menu_flag = 0;
                down_flag = 0;
                up_flag = 0;
                showcase = 0;
                Menu_pointer = main_menu_head;
            }
            right_flag = 0;
		}
		
		if((rx[Roll] < -0.6f) && left_flag == 1)
		{
            int a;
            if(1 == Menu_pointer->menu_class)
            {
                Menu_pointer->PID_value -= 0.01f;
                if(Menu_pointer->PID_value <= 0)
                {
                    Menu_pointer->PID_value = 0;
                }
                    PID_menu = PID_menu_head;
                for(a=0;a<3;a++)
                {
                    pidkp[a] = PID_menu->PID_value;
                    PID_menu = PID_menu->next;
                }
                
                for(a=0;a<3;a++)
                {
                    pidki[a] = PID_menu->PID_value;
                    PID_menu = PID_menu->next;
                }
                
                for(a=0;a<3;a++)
                {
                    pidkd[a] = PID_menu->PID_value;
                    PID_menu = PID_menu->next;
                }
            }
            
            if(2 == Menu_pointer->menu_class)
            {
                int z;
                Menu_pointer->dir --;
                if((Menu_pointer->dir - 0xf) > 0)
                {
                    Menu_pointer->dir = 0;
                }
                Motor_menu = Motor_menu_head;
                for(z=0;z<4;z++)
                {
                    if(Motor_menu->dir == 0x00)
                    {
                        motor_sta &= (~(0x01<<z));
                    }
                    else if(Motor_menu->dir == 0x01)
                    {
                        motor_sta |= (0x01<<z);
                    }
                    Motor_menu = Motor_menu->next;
                }
            }
            if(3 == Menu_pointer->menu_class)
            {
                if(aux[LEVELMODE])
                {
                    aetr_or_taer = !aetr_or_taer;
                }
            }
            left_flag = 0;
		}
		
		if((rx[Roll]) < 0.6f && (rx[Roll] > -0.6f))
		{
            right_flag = 1;
            left_flag = 1;
		}
    }

    make_vol_pack(OSD_DATA,(int)(vbattfilt*100),aetr_or_taer,rx,aux,pidkp,pidki,pidkd,menu_flag,Menu_pointer->menu_class,Menu_pointer->menu_index,showcase);
    OSD_Tx_Data(OSD_DATA,pack_len);
    
}


Menu_List CreateDbCcLinkList(char length,char list_class)
{
		char i;
		Menu_List p_new = NULL,pTail = NULL;
		Menu_List pHead = (Menu_List)malloc(sizeof(Menu_Node));
		if(NULL == pHead)
		{
				return NULL;
		}
		pHead->menu_index = 0;
		pHead->menu_class = list_class;
		pHead->next = pHead;
		pHead->prior = pHead;
		pTail = pHead;
		
		for(i=1;i<length+1;i++)
		{
				p_new = (Menu_List)malloc(sizeof(Menu_Node));
				if(NULL == p_new)
				{
						return NULL;
				}
				
				p_new->menu_index = i;
				p_new->menu_class = list_class;
				p_new->prior = pTail;
				p_new->next = pHead;
				pTail->next = p_new;
				pHead->prior = p_new;
				pTail = p_new;
		}
		return pHead;
}


#endif


