
#ifndef __BLE_NUS_MESSAGE_H__
#define __BLE_NUS_MESSAGE_H__

#define BLE_NUS_MESSAGE_MAX_DATA_LEN 1024
#define BLE_NUS_MESSAGE_PACKET_HEADER_LEN 1
#define BLE_NUS_MESSAGE_PACKET_MAX_LEN (BLE_NUS_MAX_DATA_LEN - BLE_NUS_MESSAGE_PACKET_HEADER_LEN)

/**@brief Running Speed and Cadence Service feature bits. */
#define BLE_RSCS_FEATURE_INSTANT_STRIDE_LEN_BIT             (0x01 << 0)     /**< Instantaneous Stride Length Measurement Supported bit. */
#define BLE_RSCS_FEATURE_TOTAL_DISTANCE_BIT                 (0x01 << 1)     /**< Total Distance Measurement Supported bit. */
#define BLE_RSCS_FEATURE_WALKING_OR_RUNNING_STATUS_BIT      (0x01 << 2)     /**< Walking or Running Status Supported bit. */
#define BLE_RSCS_FEATURE_CALIBRATION_PROCEDURE_BIT          (0x01 << 3)     /**< Calibration Procedure Supported bit. */
#define BLE_RSCS_FEATURE_MULTIPLE_SENSORS_BIT               (0x01 << 4)     /**< Multiple Sensor Locations Supported bit. */

#define BLE_NUS_MESSAGE_FEATURE_PACKET_CONTENT_EOT_BIT      (0x01 << 0)
#define BLE_NUS_MESSAGE_FEATURE_PACKET_CONTENT_BIT          (0x01 << 1)

#include "ble_nus.h"
#include "ble_nus_c.h"
#include "stdint.h"

typedef enum
{
    BLE_NUS_MSG_EVT_MESSAGE       /**< Message received. */
} ble_nus_msg_evt_type_t;

typedef struct  ble_nus_msg_feature_s
{
    uint8_t     feature;
}               ble_nus_msg_feature_t;

typedef struct  ble_nus_msg_packet_s
{
    uint8_t     feature;
    uint8_t     data[BLE_NUS_MESSAGE_PACKET_MAX_LEN];
}               ble_nus_msg_packet_t;

typedef struct  ble_nus_msg_buffer_s
{
    uint16_t    data_len;
    uint8_t     data[BLE_NUS_MESSAGE_MAX_DATA_LEN];
}               ble_nus_msg_buffer_t;

typedef struct  ble_nus_msg_evt_s
{
    ble_nus_msg_evt_type_t  evt_type;
    uint8_t                 *p_data;
    uint16_t                data_len;
}               ble_nus_msg_evt_t;

typedef void (* ble_nus_msg_handler_t) (ble_nus_msg_evt_t * p_evt);
typedef void (* ble_nus_msg_packet_handler_t) (ble_nus_msg_packet_t * p_evt);
typedef void (* ble_nus_send_handler_t) (uint8_t * p_data, uint16_t data_len);

typedef struct  ble_nus_msg_s
{
    ble_nus_send_handler_t          send_handler;
    ble_nus_msg_buffer_t            evt_buffer;
    ble_nus_msg_handler_t           msg_handler;    /**< Application event handler to be called when there is an event related to the NUS. */
    ble_nus_msg_packet_handler_t    packet_handler;  /**< Function to be called in case of an error. */
}               ble_nus_msg_t;

typedef struct
{
    ble_nus_send_handler_t        send_handler;
    ble_nus_msg_handler_t         evt_handler;    /**< Application event handler to be called when there is an event related to the NUS. */
    ble_nus_msg_packet_handler_t  evt_packet_handler;    /**< Application event handler to be called when there is an event related to the NUS. */
} ble_nus_msg_init_t;

// static void ble_nus_msg_handle(ble_nus_msg_t *p_ble_nus_msg, uint8_t * p_data, uint16_t data_len);
// static void ble_nus_msg_handle_buffer(ble_nus_msg_t *p_ble_nus_msg, uint8_t const * p_data, uint16_t data_len);

void ble_nus_msg_data_send(ble_nus_msg_t *p_ble_nus_msg, uint8_t * p_string, uint16_t length);
void ble_nus_msg_data_handler(ble_nus_msg_t *p_ble_nus_msg, ble_nus_evt_t * p_evt);
void ble_nus_msg_c_data_handler(ble_nus_msg_t *p_ble_nus_msg, ble_nus_c_evt_t const * p_evt);
void ble_nus_msg_flush(ble_nus_msg_t *p_ble_nus_msg);
void ble_nus_msg_init(ble_nus_msg_t *p_ble_nus_msg, ble_nus_msg_init_t *p_ble_nus_msg_init);

#endif // __BLE_NUS_MESSAGE_H__