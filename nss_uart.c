#include "nss_uart.h"
#include <stdio.h>
#include <inttypes.h>

#define set_bits(byte, num, pos)		byte |= ( ( 1 << num ) - 1 ) << pos

#define UART_RCV_BUF_SIZE               256

const int frame_len = 8;

static volatile uint8_t uart_rcv_buf[UART_RCV_BUF_SIZE];
static volatile unsigned int uart_rcv_buf_head, uart_rcv_buf_tail;

volatile unsigned int nssu_bytes_rcvd = 0;

extern void init_not_so_soft_uart();
extern void start_nssu_timer();
extern void stop_nssu_timer();
extern void reset_nssu_timer();
extern int get_nssu_pin_state();
extern int get_nssu_bytes_rcvd();

static inline void push_byte_to_circ_buf(uint8_t val, volatile uint8_t *buf, volatile unsigned int *buf_head, 
		size_t buf_size)
{
	buf[*buf_head] = val;
	*buf_head = (*buf_head + 1) % buf_size;
}

void handle_nssu_pin_change()
{
    static int bit_cnt = -2;
    static uint8_t byte_rcvd = 0;

    // -1 indicates that this interrupt handles first slope of start bit
	if(bit_cnt == -2)
    {
		// Start the timer to count bits received
        start_nssu_timer();
        bit_cnt++;
        return;
    } 
    
    int n = get_nssu_bytes_rcvd();
	
	// Make appropriate bit shift basing on actual state of the pin. 
	// If pin is high then '0' bits have been received or '1' bits otherwise.
	if(!get_nssu_pin_state())
		set_bits(byte_rcvd, n, bit_cnt);
	
    bit_cnt += n;

	// If all bits have been received reset the algorithm and push the received byte to the buffer.
	// This implementation does not handle error when the bit counter is bigger than length of baud.
	// This situation is fatal, because it could cause desynchronization of algorithm.
    if(bit_cnt >= frame_len)
    {
		push_byte_to_circ_buf(byte_rcvd, uart_rcv_buf, &uart_rcv_buf_head, UART_RCV_BUF_SIZE);
        bit_cnt = -2;
        byte_rcvd = 0;
        stop_nssu_timer();
    } else
	{
		// If not all bits have been received then reset the timer.
        reset_nssu_timer();
	}
}

void handle_nssu_tim_overflow()
{
	nssu_bytes_rcvd++;
}
