#include <string.h>
#include <stdbool.h>
#include "nss_uart.h"
#include "nssu_tests.h"
#include "utils.h"
#include "minunit.h"

// Pin state is initially set because UART line in reset state is set
static unsigned int pin_state = 1;

// This is set by TX handler when all data has been sent
static bool all_data_sent = false;

// This variable indicates how many bits were sent between consecutive slopes
static unsigned int state_duration;

// Those four methods are for use with HW so we can leave them empty
void init_not_so_soft_uart()
{
}

void start_nssu_rx_timer()
{
}

void stop_nssu_rx_timer()
{
}

void reset_nssu_rx_timer()
{
}

int get_nssu_rx_pin_state()
{
	return pin_state;
}

int get_nssu_bits_rcvd()
{
	return state_duration;
}

void set_nssu_tx_pin_state(int state)
{
	pin_state = state;
}

void disable_tx_tim_isr()
{
	// Will stop the inner while loop within the test
	all_data_sent = true;
}

MU_TEST(nssu_self_test)
{
	// The test data which will be virtually sent
	const char test_tbl[] = "ACJQUORPQKSALQ&!74108$@*(";
	const unsigned int test_tbl_len = array_len(test_tbl) - 1;
	// An array which will hold the received data
	char results[test_tbl_len + 1];

	// Push the data to the TX buffer
	for(int i = 0 ; i < test_tbl_len; ++i)
		push_byte_to_tx_buf(test_tbl[i]);

	// This variable is used to check if slope occurred
	unsigned int prev_pin_state = pin_state;
	// Lets virtually connect TX pin with RX pin, so perform a self-test
	while(!all_data_sent)
	{
		handle_nssu_tx_tim_overflow();
		// Test for slope
		if(prev_pin_state != pin_state)
		{
			// On slope normally this handler is invoked
			handle_nssu_rx_pin_change();
			prev_pin_state = pin_state;
			state_duration = 0;
		}
		state_duration ++;
	}
	
	// Pop the received data
	for(int i = 0 ; i < test_tbl_len; ++i)
		results[i] = pop_byte_from_rx_buf();
	results[test_tbl_len] = '\0';
	
	// Check if the test is successful
	mu_assert_string_eq(test_tbl, results);
}

MU_TEST_SUITE(nssu_test)
{
	MU_RUN_TEST(nssu_self_test);
}

void test_not_so_soft_uart()
{
	MU_RUN_SUITE(nssu_test);
	MU_REPORT();
}
