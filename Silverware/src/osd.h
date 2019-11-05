#ifndef __osd_h_
#define __osd_h_

#include "project.h"
#include "defines.h"

#pragma anon_unions

typedef struct Menu{
    union{
		char dir;
		float PID_value;
    };
    char menu_class;      //0:main_menu   1:PID_menu   2:motor_menu
    char menu_index;
    struct Menu *next;
    struct Menu *prior;
}Menu_Node,*Menu_List;

Menu_List CreateDbCcLinkList(char length,char list_class);

void osdMenuInit(void);

void osd_setting(void);

#endif

