#include "nss_uart.h"
#include <stdio.h>
#include <stdbool.h>

#define set_bits(byte, num, pos)		byte |= ( ( 1 << num ) - 1 ) << pos

#define UART_RX_BUF_SIZE               256

const int frame_len = 8;

static volatile uint8_t uart_rx_buf[UART_RX_BUF_SIZE];
static volatile unsigned int uart_rx_buf_head, uart_rx_buf_tail;

static volatile unsigned int nssu_bits_rcvd = 0;

extern void init_not_so_soft_uart();
extern void start_nssu_rx_timer();
extern void stop_nssu_rx_timer();
extern void reset_nssu_rx_timer();
extern int get_nssu_rx_pin_state();
extern int get_nssu_bits_rcvd();

static void push_byte_to_circ_buf(uint8_t val, volatile uint8_t *buf, volatile unsigned int *head, size_t size);
static void push_byte_to_rx_buf(uint8_t val);


void handle_nssu_rx_pin_change()
{
    static int bit_cnt = -2;
    static uint8_t byte_rcvd = 0;

    // -1 indicates that this interrupt handles first slope of start bit
	if(bit_cnt == -2)
    {
		// Start the timer to count bits received
        start_nssu_rx_timer();
        bit_cnt++;
        return;
    } 
    
    int n = get_nssu_bits_rcvd();
	
	// Make appropriate bit shift basing on actual state of the pin. 
	// If pin is high then '0' bits have been received or '1' bits otherwise.
	if(!get_nssu_rx_pin_state())
		set_bits(byte_rcvd, n, bit_cnt);
	
    bit_cnt += n;

	// If all bits have been received reset the algorithm and push the received byte to the buffer.
	// This implementation does not handle error when the bit counter is bigger than length of baud.
	// This situation is fatal, because it could cause desynchronization of algorithm.
    if(bit_cnt >= frame_len)
    {
		push_byte_to_rx_buf(byte_rcvd);
        bit_cnt = -2;
        byte_rcvd = 0;
        stop_nssu_rx_timer();
    } else
	{
		// If not all bits have been received then reset the timer.
        reset_nssu_rx_timer();
	}
}

void handle_nssu_rx_tim_overflow()
{
	nssu_bits_rcvd++;
static void push_byte_to_circ_buf(uint8_t val, volatile uint8_t *buf, volatile unsigned int *head, size_t size)
{
	unsigned int h = *head;
	buf[h] = val;
	h = ( h + 1 ) % size;
	*head = h;
}

static void push_byte_to_rx_buf(uint8_t val)
{
	push_byte_to_circ_buf(val, uart_rx_buf, &uart_rx_buf_head, UART_RX_BUF_SIZE);
}

}
