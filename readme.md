# Not-so-soft-UART

Yet another implementation of UART which allows to use any pin (almost any pin) of Your chip, but this implementation
does not use any bit banging and is non-blocking: while Your chip receives bytes using NSSU you can do anything You
like with Your chip, because NSSU is based on interrupts mainly: timer overflow interrupt and on falling/raising edge
GPIO interrupt.

This implementation is not-so-soft, because it is based on hardware (interrupts) and software, where the main
algorithm is implemented.

It was created mainly to use with RTOSes to not to block the whole application while transmitting/receiving bytes over
UART, but it could be used with bare-metal applications too.

At this stage NSSU supports only communication with no parity bit, one stop bit and 8 bits per symbol. Nonetheless
non-typical baud rates are also supported.

## What do You need to run it

To run NSSU You need a processor which has two timers (one for TX and the second one for RX) and GPIO capable of
handling falling/raising edge interrupts.

## How does it work

When the device connected to the TX pin starts transmitting then on slopes an interrupt is invoked. The handler for
the interrupt checks whether it's a beginning, end or middle of transmission of a byte and asks a RX timer how many
bits were received between slopes. On transmission end the byte is pushed to a RX buffer which is later accesible using 
a public method.

For the TX side the algorithm is quite simpler. User invokes function for transmitting bytes which copies the bytes
to the TX buffer and enables the TX interrupt. The TX interrupt is invoked periodically (at baud rate) and the TX pin
is set or reset according to the byte which is being transmitted (obviously taking into account start bits and stop
bits).

## Bringing Your hardware and NSSU together

### Implementing hardware dependent functions

NSSU is designed to be easily portable. This means that for each port a group of functions must be externally defined.
Those functions are:
* `void nssu_init()` - here You should initialize all the peripherals which will be used with NSSU. Timers, GPIO
and other resources which You will use with NSSU should be configured there.  
* `void nssu_rx_timer_start()` - in this function the timer used to count received bytes must be started and 
the hardware counter should be reset.
* `void nssu_rx_timer_stop()` - within this function the RX timer should be stopped.
* `void nssu_rx_timer_restart()` - generally speaking this function can invoke `nssu_rx_timer_start` and then 
`nssu_rx_timer_stop`, but I've added this if Your hardware needs some extra steps to restart the peripheral.
* `int nssu_get_rx_pin_state()` - this function should return the state of the pin which is used as RX pin.
* `int nssu_get_num_bits_rcvd()` - most tricky part: here you have to calculate how many bits were received between
next two edges. Best approach is to get the timer counter value and divide its value by the value of the
counter which corresponds to one bit duration (see examples to check how I've done it).
* `void nssu_set_tx_pin_state(int state)` - this function should set the state of the TX pin according to the passed
value of the parameter `state` ('0' for reset, '1' for set).
* `void nssu_tx_tim_isr_enable()` - enable interrupt for the TX timer.
* `void nssu_tx_tim_isr_disable()` - disable interrupt for the TX timer.

Those functions are declared as `extern` within NSSU.

### Invoking NSSU handlers inside interrupts

1. Inside the interrupt handler on falling/raising edge on RX pin the function: `nssu_handle_rx_pin_edge` must be 
invoked.
2. Inside the TX timer overflow interrupt handler the function `nssu_handle_tx_tim_overflow` must be invoked.

### Using public functions

**To transmit data call**:

```void nssu_transmit_data(uint8_t *data, size_t len);```

Where `data` is the data to be sent and `len` is the number of bytes to be sent.


**To receive data call**:

```int nssu_get_num_bytes_rcvd();```

The function returns how many bytes were received. To get the bytes invoke:

```uint8_t nssu_get_rcvd_byte();```

The functions could be used in such a combination:
```
for(int i = 0, n = nssu_get_num_bytes_rcvd(); i < n ; ++i)
{
    uint8_t b = nssu_get_rcvd_byte();
    do_something_with_rcvd_byte(b);
}
```

All the functions are nonblocking. 

After you completed those steps You are ready to go and You should receive and transmit bytes like a pro.

## Testing

When compiling for end use `tests` directory must not be included in compilation. If You want to run the test firstly
you have to include [minunit](https://github.com/KKoovalsky/minunit) within Your project. Then `tests` directory should
be included when compiling. Finally you can invoke the function `test_not_so_soft_uart` from the main code. The test 
should be run on the local computer.

## Examples

See [NSSU for STM32F411CE](https://github.com/KKoovalsky/NotSoSoftUart_STM32) where I've added a port for STM32F411CE
microcontroller.

