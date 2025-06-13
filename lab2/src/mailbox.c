#include "mailbox.h"
#include "mini_uart.h"

//mailbox message calling procedure
int mailbox_call(unsigned int *mailbox)
{
    //put channel 8 at the lowest 4 bits
    unsigned int message_and_channel = (((unsigned int)(unsigned long)mailbox) & ~0xF) | 8;
    
    //check mailbox status is full or not
    while(*MAILBOX_STATUS & MAILBOX_FULL){
      asm volatile("nop");
    }

    //wirte message
    *MAILBOX_WRITE = message_and_channel;
    while(1)
    {
      //check mailbox status is empty or not
      while(*MAILBOX_STATUS & MAILBOX_EMPTY)
      {
        asm volatile("nop");
      }
      //wai for a response for gpu(polling) and check response status
      if(message_and_channel == *MAILBOX_READ) return mailbox[1] == RESPONSE_SUCCESS;
    }
    return 0;
 
}

void get_board_revision()
{
  unsigned int __attribute__((aligned(16))) mailbox[7];
  mailbox[0] = 7 * 4; // buffer size in bytes
  mailbox[1] = REQUEST_CODE;
  // tags begin
  mailbox[2] = GET_BOARD_REVISION; // tag identifier
  mailbox[3] = 4; // maximum of request and response value buffer's length.
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0; // value buffer
  // tags end
  mailbox[6] = END_TAG;

  if(mailbox_call(mailbox))// message passing procedure call, you should implement it following the 6 steps provided above.
  {
    mini_uart_send_string("Board Revision: ");
    mini_uart_send_hex(mailbox[5]);// it should be 0xa020d3 for rpi3 b+,not sure what for qemu rpi3
  }
  else {
    mini_uart_send_string("Failed to get board revision info.");
  }

}

void get_memory_information()
{
  unsigned int __attribute__((aligned(16))) mailbox[8];
  mailbox[0] = 8 * 4;          // Total size of the message buffer (bytes)
  mailbox[1] = REQUEST_CODE;   // Request

  // Tag: Get ARM memory
  mailbox[2] = GET_ARM_MEMORY; // Tag ID
  mailbox[3] = 8;              // Value buffer size
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0;              // Base address (will be filled by firmware)
  mailbox[6] = 0;              // Size (will be filled by firmware)
  mailbox[7] = END_TAG;

  if(mailbox_call(mailbox))
  {
    mini_uart_send_string("Memory Base: ");
    mini_uart_send_hex(mailbox[5]);
    mini_uart_send_string("\r\nMemory Size: ");
    mini_uart_send_hex(mailbox[6]);
  }
  else {
    mini_uart_send_string("Failed to get memory info.");
  }
}