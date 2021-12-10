#ifndef __KEY_H__
#define __KEY_H__

#define KEY1            0X01
#define KEY2            0X02
#define KEY3            0X04
#define KEY4            0X08
#define KEY1LONG        0X10
#define KEY2LONG        0X20
#define KEY3LONG        0X40
#define KEY4LONG        0X80
void key_init(void);
extern process_event_t process_event_keydown;
extern process_event_t process_event_keyup;

#endif
