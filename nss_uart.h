#ifndef NSS_UART_H
#define NSS_UART_H

#include <inttypes.h>
#include <stdio.h>

// Should be called within on edge detected ISR for RX pin
void handle_nssu_rx_pin_change();

// Should be called within ISR which handles transmitting bytes over UART
void handle_nssu_tx_tim_overflow();

// Function which allows to check whether there are some bytes received with NSSU RX side
int nssu_get_num_bytes_rcvd();
// Function used to copy byte to the TX buffer
void push_byte_to_tx_buf(uint8_t val);

// Function used to get a byte from RX buffer. This function doesn't check whether new data is available
uint8_t pop_byte_from_rx_buf();

// Function used to transmit data with NSSU
void transmit_data(uint8_t *data, size_t len);


#endif /* NSS_UART_H */
