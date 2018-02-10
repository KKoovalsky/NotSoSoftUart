#include "nss_uart.h"
#include <stdio.h>
#include <stdbool.h>

#define set_bits(byte, num, pos)		byte |= ( ( 1 << num ) - 1 ) << pos

#define UART_RX_BUF_SIZE               256
#define UART_TX_BUF_SIZE               128

const int frame_len = 8;

static volatile uint8_t uart_rx_buf[UART_RX_BUF_SIZE];
static volatile unsigned int uart_rx_buf_head, uart_rx_buf_tail;

static volatile uint8_t uart_tx_buf[UART_TX_BUF_SIZE];
static volatile unsigned int uart_tx_buf_head, uart_tx_buf_tail;

extern void nssu_init();
extern void nssu_rx_timer_start();
extern void nssu_rx_timer_stop();
extern void nssu_rx_timer_restart();
extern int nssu_get_rx_pin_state();
extern int nssu_get_num_bits_rcvd();
extern void nssu_set_tx_pin_state(int state);
extern void nssu_tx_tim_isr_enable();
extern void nssu_tx_tim_isr_disable();

static void push_byte_to_circ_buf(uint8_t val, volatile uint8_t *buf, volatile unsigned int *head, size_t size);
static void push_byte_to_tx_buf(uint8_t val);
static void push_byte_to_rx_buf(uint8_t val);

static uint8_t pop_byte_from_circ_buf(volatile uint8_t *buf, volatile unsigned int *tail, size_t size);
static uint8_t pop_byte_from_tx_buf();

int nssu_get_num_bytes_rcvd()
{
	int tail = uart_rx_buf_tail, head = uart_rx_buf_head;
	if(tail <= head)
		return head - tail;
	else
		return head + UART_RX_BUF_SIZE - tail;
}

void nssu_transmit_data(uint8_t *data, size_t len)
{
	// Push the data to the TX buffer
	for(size_t i = 0 ; i < len ; ++i)
		push_byte_to_tx_buf(data[i]);

	// Start transmission
	nssu_tx_tim_isr_enable();
}

void nssu_handle_rx_pin_edge()
{
    static int bit_cnt = -2;
    static uint8_t byte_rcvd = 0;
    // -1 indicates that this interrupt handles first slope of start bit
	if(bit_cnt == -2)
    {
		// Start the timer to count bits received
        nssu_rx_timer_start();
        bit_cnt++;
        return;
    }
    int n = nssu_get_num_bits_rcvd();
	// Make appropriate bit shift basing on actual state of the pin. 
	// If pin is high then '0' bits have been received or '1' bits otherwise.
	if(!nssu_get_rx_pin_state())
		set_bits(byte_rcvd, n, bit_cnt);
    bit_cnt += n;
	// If all bits have been received reset the algorithm and push the received byte to the buffer.
	// This implementation does not handle error when the bit counter is bigger than length of baud.
	// This situation is fatal, because it could cause desynchronization of algorithm.
    if(bit_cnt >= frame_len)
    {	
		// Save the bit counter to test later how many bits were received
		int prev_bit_cnt = bit_cnt;
		push_byte_to_rx_buf(byte_rcvd);
        bit_cnt = -2;
        byte_rcvd = 0;
        nssu_rx_timer_stop();
		// When last bit was '1' there was no slope on stop bit, so this final slope is also the slope which
		// is the beginning of a start bit. The beginning of start bit should be handled then.
		if(prev_bit_cnt > frame_len)
			 nssu_handle_rx_pin_edge();
    } else
	{
		// If not all bits have been received then reset the timer.
        nssu_rx_timer_restart();
	}
}

void nssu_handle_tx_tim_overflow()
{
	static bool whole_byte_sent = true; // Indicates if a new byte should be transmitted
	static unsigned int bits_sent = 0;	// Counter of bits sent
	static uint8_t byte = 0;			// The data which is sent
	if(whole_byte_sent == true)
	{
		// When there are still some bytes to sent:
		if(uart_tx_buf_head != uart_tx_buf_tail)
		{
			whole_byte_sent = false;
			// Get the next byte to be sent
			byte = pop_byte_from_tx_buf();
			// Transmit start bit
			nssu_set_tx_pin_state(0);
		}
		// Disable the interrupt because there are no more bytes to be sent
		else
			nssu_tx_tim_isr_disable();
	}
	else
	{
		// When all bits has been sent
		if(bits_sent == frame_len)
		{
			whole_byte_sent = true;
			bits_sent = 0;
			// Transmit stop bit
			nssu_set_tx_pin_state(1);
		}
		else
		{
			// Send a bit of a byte
			nssu_set_tx_pin_state(byte & 0x01);
			byte >>= 1;
			bits_sent ++;
		}
	}
}

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

static void push_byte_to_tx_buf(uint8_t val)
{
	push_byte_to_circ_buf(val, uart_tx_buf, &uart_tx_buf_head, UART_TX_BUF_SIZE);
}

static uint8_t pop_byte_from_circ_buf(volatile uint8_t *buf, volatile unsigned int *tail, size_t size)
{
	unsigned int t = *tail;
	uint8_t res = buf[t];
	t = ( t + 1 ) % size;
	*tail = t;
	return res;
}

uint8_t nssu_get_rcvd_byte()
{
	return pop_byte_from_circ_buf(uart_rx_buf, &uart_rx_buf_tail, UART_RX_BUF_SIZE);
}

static uint8_t pop_byte_from_tx_buf()
{
	return pop_byte_from_circ_buf(uart_tx_buf, &uart_tx_buf_tail, UART_TX_BUF_SIZE);
}
