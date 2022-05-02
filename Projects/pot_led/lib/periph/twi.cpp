#include <string.h>

#include <dbg.h>
#include <regs.h>
#include <gpio.h>
#include <twipriv.h>
#include <twiapi.h>

static volatile twiContext_t g_ctx;
static volatile twiRxBuf_t g_rxBuf;
static volatile twiTxBuf_t g_txBuf;

#ifdef __cplusplus
extern "C" {
#endif
    void ISR_Twi(void)
    __attribute__ ((signal,used,externally_visible));
#ifdef __cplusplus
}
#endif

// Memory mapped IO addresses for TWI (I2C)
volatile uint8_t * const pui8Twbr = (uint8_t *)TWBR_ADDR;  // Register TWBR
volatile uint8_t * const pui8Twsr = (uint8_t *)TWSR_ADDR;  // Register TWSR
volatile uint8_t * const pui8Twar = (uint8_t *)TWAR_ADDR;  // Register TWAR
volatile uint8_t * const pui8Twdr = (uint8_t *)TWDR_ADDR;  // Register TWDR
volatile uint8_t * const pui8Twcr = (uint8_t *)TWCR_ADDR;  // Register TWCR
volatile uint8_t * const pui8Twamr = (uint8_t *)TWAMR_ADDR;// Register TWAMR

void twiInit(uint8_t slaveAddress)
{
    g_ctx.slaveAddr = slaveAddress & 0x7f;
    g_ctx.twiSending = false;
    g_ctx.twiReceiving = false;

    memset((void *)&g_rxBuf, 0, sizeof(g_rxBuf));
    memset((void *)&g_txBuf, 0, sizeof(g_txBuf));
    
    // Enable internal pull-up resistors for SCL/SDA
    // See data-sheet section: "SCL and SDA Pins"
    // NOTE Adding this did not help much the waveform,
    // but adding external 10K resitors made it more square.
    TWI_DDR &= ~(b2m(TWI_SCL_PULL_UP) | b2m(TWI_SDA_PULL_UP));
    TWI_PORT |= (b2m(TWI_SCL_PULL_UP) | b2m(TWI_SDA_PULL_UP));

    // Configure TWI (I2C)
    //
    // SCL frequency: 100,000 Hz
    // Enable automatic ACK as we can receive at any time
    //
    // Communication modes will only be:
    //  * Master transmitter - When we have a pot read we send it
    //  * Slave receiver - When we receive a pot read we apply it
    twiSetBitRateDivisionFactor(100, 0); // Will set TWPS1:0 and TWBR
    TWCR = TWCR_MASK_READY;
    TWAR = (g_ctx.slaveAddr << 1);  // Set slave address
    TWAMR = 0;

    SerialPr(("TWSR:0x"));
    SerialPr((TWSR, 16));
    SerialPr((" TWBR:"));
    SerialPr((TWBR));
    SerialPr((" TWCR:0x"));
    SerialPrLn((TWCR, 16));
}

// Receive data
// ONLY CALLED FROM WITHIN AN ISR
bool twiRecv(twiRxBuf_t* recvBuf)
{
    bool received = false;

    if (g_rxBuf.status & TWI_RX_RecvCompleted)
    {
        memcpy((void *)recvBuf, (void *)&g_rxBuf, sizeof(twiRxBuf_t));
        memset((void *)&g_rxBuf, 0, sizeof(g_rxBuf));
        received = true;
    }

    return received;
}

// Send data
// ONLY CALLED FROM WITHIN AN ISR
bool twiSend(twiTxBuf_t* sendBuf)
{
    bool started = false;

    if (!g_ctx.twiSending &&
        sendBuf->len > 0 && sendBuf->len <= sizeof(sendBuf->buffer))
    {
        // Note toAddr 0 (General Call address) is allowed
        memcpy((void *)&g_txBuf, (void *)sendBuf, sizeof(twiTxBuf_t));
        g_ctx.twiSending = true;
        g_txBuf.size = 0;
        g_txBuf.status = TWI_TX_Sending;
        started = true;
        if (!g_ctx.twiReceiving)
        {
            // Kick off sending if not already receiving
            SerialPrLn(("Kick off sender"));
            TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTA);

            // NOTE while testing TX only, Uno was ending at state 0x20
            // with the STOP bit still set, and unable to send again, 
            // but!, I noticed I had the SCL/SDA wires attached and 
            // the other end hanging free (i.e. not connected), and once 
            // I removed the wires, it began to behave as expected.
        }
    }
    
    return started;
}

// TWI
void ISR_Twi(void)
{
    uint8_t data;
    // Note the different states purposely left with its hex value
    // to match the Data Sheet documentation in section:
    // "2-Wire Serial Interface" / "Transmission Modes"
    volatile uint8_t twsr = TWSR & 0xf8; // Keep only the status bits

    switch(twsr)
    {
        // +++++++++++++++++++
        // Receiving states
        // Slave Receiver mode
        // -------------------
        case 0x60:
            // Own SLA+W has been received
            TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
            g_ctx.twiReceiving = true;
            SerialPrLn2(("* 0x60 Own SLA+W has been received"));
            break;
        case 0x68:
            // Arbitration lost in SLA+R/W (owm address)
            TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
            g_ctx.twiReceiving = true;
            SerialPrLn(("! 0x68 Arbitration lost in SLA+R/W"));
            break;
        case 0x70:
            // General call address received
            TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
            g_ctx.twiReceiving = true;
            SerialPrLn2(("* 0x70 General call address received"));
            break;
        case 0x78:
            // Arbitration lost in SLA+R/W (GC address)
            TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
            g_ctx.twiReceiving = true;
            SerialPrLn(("! 0x78 Arbitration lost in SLA+R/W (GC)"));
            break;
        case 0x80:
        case 0x88:
            // Data received
            data = TWDR;
            TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
            SerialPrLn2(("* 0x80 Data received"));
            if (!g_ctx.twiReceiving)
            {
                SerialPrLn(("! 0x80 Unexpected data"));
                g_ctx.twiReceiving = true;
            }
            if (g_rxBuf.status & TWI_RX_RecvCompleted)
            {
                // We had completed data already, discard it
                SerialPrLn(("! 0x80 Previous packet being lost"));
                g_rxBuf.status = TWI_RX_Receiving;
                g_rxBuf.size = 0;
            }
            if (g_rxBuf.size < TWI_MAX_BUF)
            {
                g_rxBuf.buffer[g_rxBuf.size] = data;
                g_rxBuf.size++;
            }
            else
            {
                // Overflow
                g_rxBuf.status |= TWI_RX_DataOverflow;
            }
            break;
        case 0x90:
        case 0x98:
            // Data received (GC)
            data = TWDR;
            TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
            SerialPrLn2(("* 0x90 Data received (GC)"));
            if (!g_ctx.twiReceiving)
            {
                SerialPrLn(("! 0x90 Unexpected data"));
                g_ctx.twiReceiving = true;
            }
            if (g_rxBuf.status & TWI_RX_RecvCompleted)
            {
                // We had completed data already, discard it
                SerialPrLn(("! 0x90 Previous packet being lost"));
                g_rxBuf.status = TWI_RX_Receiving;
                g_rxBuf.size = 0;
            }
            g_rxBuf.status |= TWI_RX_DataFromGC;
            if (g_rxBuf.size < TWI_MAX_BUF)
            {
                g_rxBuf.buffer[g_rxBuf.size] = data;
                g_rxBuf.size++;
            }
            else
            {
                // Overflow
                g_rxBuf.status |= TWI_RX_DataOverflow;
                SerialPrLn(("! 0x90 Data reception overflow"));
            }
            break;
        case 0xa0:
            // STOP has been received
            if (g_ctx.twiReceiving)
            {
                // Completed reception (STOP received)
                g_rxBuf.status |= TWI_RX_RecvCompleted;
                SerialPrLn2(("* 0xa0 Data reception complete"));
                g_ctx.twiReceiving = false;
            }
            else
            {
                SerialPrLn(("! 0xa0 Unexpected data"));
            }
            if (g_ctx.twiSending)
            {
                // We have data to send
                TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTA);
                SerialPrLn2(("* 0xa0 Receive completed, begin to send"));
            }
            else
            {
                TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
            }
            break;
        // +++++++++++++++++++++++
        // Sending states
        // Master Transmitter mode
        // -----------------------
        case 0x08:
            SerialPrLn2(("* 0x08 Start was sent"));
            // Note START must be manually cleared, which I do
            // by writting new value to TWCR without START bit set
            if (g_ctx.twiSending)
            {
                // For a sender, flag R/W = 0
                TWDR = ((g_txBuf.toAddr & 0x7f) << 1);
                TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
                g_ctx.txRetry = 0;
            }
            else
            {
                SerialPrLn(("! 0x08 Start was sent but we are not sending"));
                TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTO);
            }
            break;
        case 0x10:
            SerialPrLn2(("* 0x10 Repeat start was sent"));
            if (g_ctx.twiSending)
            {
                // For a sender, flag R/W = 0
                TWDR = ((g_txBuf.toAddr & 0x7f) << 1);
                TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
            }
            else
            {
                SerialPrLn(("! 0x10 Repeat start was sent but we are not sending"));
                TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTO);
            }
            break;
        case 0x18:
            SerialPrLn2(("* 0x18 SLA+W was sent"));
            if (g_ctx.twiSending)
            {
                if (g_txBuf.len)
                {
                    g_txBuf.size = 0; // First data byte
                    TWDR = g_txBuf.buffer[g_txBuf.size];
                    TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
                    g_txBuf.size++;
                    g_ctx.txRetry = 0;
                }
                else
                {
                    // We should have data to send
                    SerialPrLn(("! 0x18 SLA+W was sent but don't have data to send"));
                    TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTO);
                }
            }
            else
            {
                SerialPrLn(("! 0x18 SLA+W was sent but we are not sending"));
                TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTO);
            }
            break;
        case 0x20:
            SerialPrLn(("! 0x20 SLA+W was sent but did not get ACK"));
            // Send a repeated START if allowed
            if (g_ctx.txRetry < TWI_MAX_TX_RETRY)
            {
                TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTA);
                g_ctx.txRetry++;
            }
            else
            {
                // Failed to send packet
                TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTO);
                g_ctx.twiSending = false;
                SerialPr(("! 0x20 failed to send packet after retries, giving up, TWCR: 0x"));
                SerialPrLn((TWCR, 16));
            }
            break;
        case 0x28:
            SerialPrLn2(("* 0x28 Data was sent and ACK received"));
            if (g_ctx.twiSending)
            {
                if (g_txBuf.size < g_txBuf.len)
                {
                    TWDR = g_txBuf.buffer[g_txBuf.size];
                    TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
                    g_txBuf.size++;
                }
                else
                {
                    // Have transmitted all data, send STOP
                    TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTO);
                    // We can now accept another packet to send
                    g_ctx.twiSending = false;
                    g_txBuf.status |= TWI_TX_SendCompleted;
                }
            }
            else
            {
                SerialPrLn(("! 0x28 Data was sent but we are not sending"));
                TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTO);
            }
            break;
        case 0x30:
            SerialPrLn(("! 0x30 Data was sent but did not get ACK"));
            // Abort sending, we retry only at the begining, 
            // but data failure, fails the whole sending
            // Failed to send packet
            TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTO);
            break;
        case 0x38:
            SerialPrLn(("! 0x38 Arbitration lost in SLA+W/Data, send START when bus is free"));
            TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT) | b2m(TWCR_BIT_TWSTA);
            break;
        default:
            SerialPr(("! 0x"));
            SerialPr((twsr, 16));
            SerialPrLn((" Unexpected state"));
            TWCR = TWCR_MASK_READY | b2m(TWCR_BIT_TWINT);
    }
}
