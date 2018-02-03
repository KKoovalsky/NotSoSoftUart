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

// Function used to get a byte from RX buffer. This function doesn't check whether new data is available
uint8_t pop_byte_from_rx_buf();

// Setter for the function which will be called when a byte has been received
void register_nssu_byte_received_callback(void (*callback_fn)(void));

#endif /* NSS_UART_H */
