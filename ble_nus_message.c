
#include "ble_nus_message.h"

#define NRF_LOG_MODULE_NAME ble_nus_msg
#if BLE_NUS_MSG_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       BLE_NUS_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  BLE_NUS_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR BLE_NUS_CONFIG_DEBUG_COLOR
#else // BLE_NUS_MSG_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // BLE_NUS_MSG_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

static void ble_nus_msg_copy(ble_nus_msg_t *p_ble_nus_msg, uint8_t * p_data, uint16_t data_len)
{
    const uint16_t idx = p_ble_nus_msg->evt_buffer.data_len;

    if (p_ble_nus_msg->evt_buffer.data_len + data_len <= BLE_NUS_MESSAGE_MAX_DATA_LEN)
    {
        memcpy(&p_ble_nus_msg->evt_buffer.data[idx], p_data, data_len);
        p_ble_nus_msg->evt_buffer.data_len += data_len;
    } else
    {
        NRF_LOG_WARNING("BLE_NUS_MESSAGE_MAX_DATA_LEN is too small");
    }
}

void ble_nus_msg_data_send(ble_nus_msg_t *p_ble_nus_msg, uint8_t * p_string, uint16_t length)
{
    ble_nus_msg_packet_t  ble_nus_msg_packet;
    uint8_t               copy_len;

    memset(&ble_nus_msg_packet, 0, sizeof(ble_nus_msg_packet));
    ble_nus_msg_packet.feature |= BLE_NUS_MESSAGE_FEATURE_PACKET_CONTENT_BIT;

    for (uint16_t offset = 0; offset < length;)
    {
        copy_len = offset + BLE_NUS_MESSAGE_PACKET_MAX_LEN >= length ? length - offset : BLE_NUS_MESSAGE_PACKET_MAX_LEN;
        memcpy(&ble_nus_msg_packet.data[0], p_string, copy_len);
        p_ble_nus_msg->send_handler((uint8_t *)&ble_nus_msg_packet, sizeof(ble_nus_msg_packet));
        offset += copy_len;
    }

    ble_nus_msg_packet.feature = 0 | BLE_NUS_MESSAGE_FEATURE_PACKET_CONTENT_EOT_BIT;
    p_ble_nus_msg->send_handler((uint8_t *)&ble_nus_msg_packet, sizeof(ble_nus_msg_packet));
}

static void ble_nus_msg_handle(ble_nus_msg_t *p_ble_nus_msg, uint8_t * p_data, uint16_t data_len)
{
    uint8_t feature = 0;
    uint8_t content_offset = BLE_NUS_MESSAGE_PACKET_HEADER_LEN;

    NRF_LOG_INFO("Processing now content (len=%d)", data_len);
    if (data_len >= BLE_NUS_MESSAGE_PACKET_HEADER_LEN)
    {
        feature = p_data[0];
    }

    if (feature & BLE_NUS_MESSAGE_FEATURE_PACKET_CONTENT_BIT)
    {
        NRF_LOG_DEBUG("Received a content packet");
        ble_nus_msg_copy(p_ble_nus_msg, &p_data[content_offset], data_len - content_offset);
    }
    
    if (feature & BLE_NUS_MESSAGE_FEATURE_PACKET_CONTENT_EOT_BIT)
    {
        NRF_LOG_DEBUG("Received a flush packet");
        ble_nus_msg_flush(p_ble_nus_msg);
    }
}

static void ble_nus_msg_copy_buffer(ble_nus_msg_t *p_ble_nus_msg, uint8_t const * p_data, uint16_t data_len)
{
    const uint16_t idx = p_ble_nus_msg->evt_buffer.data_len;

    if (p_ble_nus_msg->evt_buffer.data_len + data_len <= BLE_NUS_MESSAGE_MAX_DATA_LEN)
    {
      memcpy(&p_ble_nus_msg->evt_buffer.data[idx], p_data, data_len);
      p_ble_nus_msg->evt_buffer.data_len += data_len;
    } else
    {
        NRF_LOG_WARNING("BLE_NUS_MESSAGE_MAX_DATA_LEN is too small");
    }
}

static void ble_nus_msg_handle_buffer(ble_nus_msg_t *p_ble_nus_msg, uint8_t const * p_data, uint16_t data_len)
{
    uint8_t feature = 0;
    uint8_t content_offset = BLE_NUS_MESSAGE_PACKET_HEADER_LEN;

    NRF_LOG_INFO("Processing now content (len=%d)", data_len);
    if (data_len >= BLE_NUS_MESSAGE_PACKET_HEADER_LEN)
    {
        feature = p_data[0];
    }

    if (feature & BLE_NUS_MESSAGE_FEATURE_PACKET_CONTENT_BIT)
    {
        NRF_LOG_DEBUG("Received a content packet");
        ble_nus_msg_copy_buffer(p_ble_nus_msg, &p_data[content_offset], data_len - content_offset);
    }

    if (feature & BLE_NUS_MESSAGE_FEATURE_PACKET_CONTENT_EOT_BIT)
    {
        NRF_LOG_DEBUG("Received a flush packet");
        ble_nus_msg_flush(p_ble_nus_msg);
    }
}

void ble_nus_msg_data_handler(ble_nus_msg_t *p_ble_nus_msg, ble_nus_evt_t *p_evt)
{
    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        NRF_LOG_DEBUG("Received buffered data from BLE NUS MSG.");
        ble_nus_msg_handle_buffer(p_ble_nus_msg, p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

    }
}

void ble_nus_msg_c_data_handler(ble_nus_msg_t *p_ble_nus_msg, ble_nus_c_evt_t const *p_evt)
{
    if (p_evt->evt_type == BLE_NUS_C_EVT_NUS_TX_EVT)
    {
        NRF_LOG_DEBUG("Received data from BLE NUS MSG.");
        ble_nus_msg_handle(p_ble_nus_msg, p_evt->p_data, p_evt->data_len);
    }
}

void ble_nus_msg_flush(ble_nus_msg_t *p_ble_nus_msg)
{
    ble_nus_msg_evt_t msg_evt;

    NRF_LOG_DEBUG("Flushing message buffer");
    msg_evt.evt_type = BLE_NUS_MSG_EVT_MESSAGE;
    msg_evt.data_len = p_ble_nus_msg->evt_buffer.data_len;
    msg_evt.p_data = &p_ble_nus_msg->evt_buffer.data[0];

    if (p_ble_nus_msg->msg_handler != NULL) {
        p_ble_nus_msg->msg_handler(&msg_evt);
    }

    p_ble_nus_msg->evt_buffer.data_len = 0;
    memset(p_ble_nus_msg->evt_buffer.data, 0, BLE_NUS_MESSAGE_MAX_DATA_LEN);
}

void ble_nus_msg_init(ble_nus_msg_t *p_ble_nus_msg, ble_nus_msg_init_t *p_ble_nus_msg_init)
{
    p_ble_nus_msg->msg_handler = p_ble_nus_msg_init->evt_handler;
    p_ble_nus_msg->packet_handler = p_ble_nus_msg_init->evt_packet_handler;
    p_ble_nus_msg->send_handler = p_ble_nus_msg_init->send_handler;

    memset(&p_ble_nus_msg->evt_buffer, 0, sizeof(p_ble_nus_msg->evt_buffer));
}
