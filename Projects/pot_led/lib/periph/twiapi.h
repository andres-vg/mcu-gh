#ifndef __TWIAPI_H__
#define __TWIAPI_H__

// Maximum amount of data we can send/receive
#define TWI_MAX_BUF         8

// Buffer used to receive data
typedef struct __twiRxBuf_t
{
    // bytes received so far [1, TWI_MAX_BUF-1]
    uint8_t size;

    // 0x01 : receiving (set on first data received)
    // 0x02 : finished receiving (STOP was received)
    // 0x04 : data overflow
    // 0x08 : data from general call
    uint8_t status;

    // Buffer where to store data received
    uint8_t buffer[TWI_MAX_BUF];    
} twiRxBuf_t;

#define TWI_RX_Receiving        0x01
#define TWI_RX_RecvCompleted    0x02
#define TWI_RX_DataOverflow     0x04
#define TWI_RX_DataFromGC       0x08

// Buffer used to send data
typedef struct __twiTxBuf_t
{
    // To address
    uint8_t toAddr;

    // Number of bytes sent
    uint8_t size;

    // 0x01 : sending
    // 0x02 : finished sending
    uint8_t status;

    // Number of bytes to send
    uint8_t len; 

    // Buffer from where to send data
    uint8_t buffer[TWI_MAX_BUF];    
} twiTxBuf_t;

#define TWI_TX_Sending          0x01
#define TWI_TX_SendCompleted    0x02

// Initialize
void twiInit(uint8_t slaveAddress);

// Query and receive
// Return true if we have received data
bool twiRecv(twiRxBuf_t *recvBuf);

// Start a send
// Return true if succeeded in starting the send
bool twiSend(twiTxBuf_t *sendBuf);

#endif // __TWIAPI_H__