
#include <stdint.h>
#include <stdbool.h>
#include "app_error.h"
#include "app_scheduler.h"
#include "boards.h"
#include "nfc_t4t_lib.h"
#include "nfc_uri_msg.h"
#include "nrf_log_ctrl.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define APP_SCHED_MAX_EVENT_SIZE 0                  /**< Maximum size of scheduler events. */
#define APP_SCHED_QUEUE_SIZE     4                  /**< Maximum number of events in the scheduler queue. */

static uint8_t m_ndef_msg_buf[NDEF_FILE_SIZE];      /**< Buffer for NDEF file. */
static uint8_t m_ndef_msg_len;                      /**< Length of the NDEF message. */


/**
 * @brief Function for handling an NFC write
 */
static void scheduler_ndef_msg_written(void * p_event_data, uint16_t event_size)
{
    ret_code_t err_code;

    UNUSED_PARAMETER(p_event_data);
    UNUSED_PARAMETER(event_size);

    // TODO
    // Send over UART
    // err_code = write uart here
    //APP_ERROR_CHECK(err_code);

    // Wipe ndef buffer

    NRF_LOG_INFO("NDEF message relayed!");
}


/**
 * @brief Function for setting ndef URI with recieved UART
 */
static void scheduler_ndef_buffer_update(void * p_event_data, uint16_t event_size)
{
    ret_code_t err_code;

    UNUSED_PARAMETER(p_event_data);
    UNUSED_PARAMETER(event_size);

    // TODO 
    // Update buffer with new ndef message
    // err_code = write buffer here
    //APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("NDEF message updated!");
}


/**
 * @brief Callback function for handling NFC events.
 */
static void nfc_callback(void          * context,
                         nfc_t4t_event_t event,
                         const uint8_t * data,
                         size_t          dataLength,
                         uint32_t        flags)
{
    (void)context;

    switch (event)
    {
        case NFC_T4T_EVENT_FIELD_ON:
            bsp_board_led_on(BSP_BOARD_LED_0);
            break;

        case NFC_T4T_EVENT_FIELD_OFF:
            bsp_board_leds_off();
            break;

        case NFC_T4T_EVENT_NDEF_READ:
            bsp_board_led_on(BSP_BOARD_LED_3);
            break;

        case NFC_T4T_EVENT_NDEF_UPDATED:
            if (dataLength > 0)
            {
                ret_code_t err_code;

                bsp_board_led_on(BSP_BOARD_LED_1);

                // Schedule sending written NDEF over UART
                m_ndef_msg_len = dataLength;
                err_code       = app_sched_event_put(NULL, 0, scheduler_ndef_msg_written);
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}


/**
 *@brief Function for initializing logging.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**
 * @brief   Function for application main entry.
 */
int main(void)
{
    ret_code_t err_code;

    log_init();

    /* Configure LED-pins as outputs */
    bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);

    /* Initialize App Scheduler. */
    APP_SCHED_INIT(APP_SCHED_MAX_EVENT_SIZE, APP_SCHED_QUEUE_SIZE);

    /* Set up NFC */
    err_code = nfc_t4t_setup(nfc_callback, NULL);
    APP_ERROR_CHECK(err_code);

    /* Run Read-Write mode for Type 4 Tag platform */
    // TODO set initial nfc message
    err_code = nfc_t4t_ndef_rwpayload_set(m_ndef_msg_buf, sizeof(m_ndef_msg_buf));
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("NFC Reader Initialised");

    /* Start sensing NFC field */
    err_code = nfc_t4t_emulation_start();
    APP_ERROR_CHECK(err_code);

    while (1)
    {
        app_sched_execute();

        NRF_LOG_FLUSH();
        __WFE();
    }
}


/** @} */
