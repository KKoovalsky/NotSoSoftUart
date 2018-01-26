#ifndef NSS_UART_H
#define NSS_UART_H

#include <inttypes.h>
extern volatile unsigned int nssu_bytes_rcvd;

// Should be called within on edge detected ISR for RX pin
void handle_nssu_rx_pin_change();

// Should be called within ISR which handles bit counting for the receiver
void handle_nssu_rx_tim_overflow();


#endif /* NSS_UART_H */
