#ifndef MAILBOX_H
#define MAILBOX_H

#include "base.h"

#define MAILBOX_READ    ((volatile unsigned int *)(MAILBOX_BASE))
#define MAILBOX_STATUS  ((volatile unsigned int *)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE   ((volatile unsigned int *)(MAILBOX_BASE + 0x20))

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

#define REQUEST_CODE      0x00000000
#define RESPONSE_SUCCESS  0x80000000
#define RESPONSE_ERROR    0x80000001

//TAGS
#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

void get_board_revision();
void get_memory_information();
#endif