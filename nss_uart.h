#ifndef NSS_UART_H
#define NSS_UART_H

extern volatile unsigned int nssu_bytes_rcvd;

void handle_nssu_pin_change();
void handle_nssu_tim_overflow();

#endif /* NSS_UART_H */
