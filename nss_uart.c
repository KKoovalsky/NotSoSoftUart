#include <stdio.h>
#include <inttypes.h>
#include "esp8266.h"
#include "espressif/esp_common.h"

#define set_bits(byte, num, pos)		byte |= ( ( 1 << num ) - 1 ) << pos

#define UART_RCV_BUF_SIZE               256

#define UART_RCV_PIN		5

#define TIMER_FREQ			40000000
#define BAUD_RATE			9600.
#define BITS_PER_BAUD		10.
#define BIT_DUR_IN_TIMER_TICKS ( TIMER_FREQ ) / ( BAUD_RATE ) / ( BITS_PER_BAUD )

static void handle_pin_change(uint8_t pin);
int frame_len;

volatile uint8_t uart_rcv_buf[UART_RCV_BUF_SIZE];
volatile uint8_t *uart_rcv_buf_head = uart_rcv_buf;

extern void start_timer();
extern void stop_timer();
extern void reset_timer();
extern int get_uart_pin_state();
extern int get_num_bytes_rcvd();
extern void init_not_so_soft_uart();

inline void push_byte(uint8_t byte, uint8_t *buf_head)
{
    //*buf_head++ = byte;
	printf("%c", byte);
}

static void handle_pin_change(uint8_t pin)
{
    static int bit_cnt = -2;
    static uint8_t byte_rcvd = 0;

    // -1 indicates that this interrupt handles first slope of start bit
	if(bit_cnt == -2)
    {
		// Start the timer to count bits received
        start_timer();
        bit_cnt++;
        return;
    } 
    
    int n = get_num_bytes_rcvd();
	
	// Make appropriate bit shift basing on actual state of the pin. 
	// If pin is high then '0' bits have been received or '1' bits otherwise.
	if(!get_uart_pin_state())
		set_bits(byte_rcvd, n, bit_cnt);
	
    bit_cnt += n;

	// If all bits have been received reset the algorithm and push the received byte to the buffer.
	// This implementation does not handle error when the bit counter is bigger than length of baud.
	// This situation is fatal, because it could cause desynchronization of algorithm.
    if(bit_cnt >= frame_len)
    {
        push_byte(byte_rcvd, (uint8_t* ) uart_rcv_buf_head);
        bit_cnt = -2;
        byte_rcvd = 0;
        stop_timer();
    } else
	{
		// If not all bits have been received then reset the timer.
        reset_timer();
	}
}
