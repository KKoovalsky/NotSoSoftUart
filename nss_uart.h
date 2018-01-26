#ifndef NSS_UART_H
#define NSS_UART_H

#include <inttypes.h>

// Should be called within on edge detected ISR for RX pin
void handle_nssu_rx_pin_change();

// Should be called within ISR which handles bit counting for the receiver
void handle_nssu_rx_tim_overflow();

// Should be called within ISR which handles transmitting bytes over UART
void handle_nssu_tx_tim_overflow();

// Function used to copy byte to the TX buffer
void push_byte_to_tx_buf(uint8_t val);


#endif /* NSS_UART_H */
