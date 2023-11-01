#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "pio_uart_tx.h"
#include "pio_uart_rx.h"
#include <Arduino.h>

/**
 * @brief Software Serial Arduino Stream which uses the Pico PIO.
 *
 * Based on https://github.com/pschatzmann/pico-arduino/tree/00f59dddd68a0683972345d4c56bd440e356aafe/Arduino/SoftwareSerial
 * @author Flamabalistic
 * @copyright GPLv3
 */

class SoftwareSerial : public HardwareSerial
{
public:
    SoftwareSerial(int txPin = -1, int rxPin = -1, uint stateMachineRxIndex = 0, uint stateMachineTxIndex = 1, PIO pio = pio1)
    {
        this->pio = pio;
        this->sm_rx = stateMachineRxIndex; // index of the state machine running the rx program
        this->sm_tx = stateMachineTxIndex; // index of the state machine running the tx program
        this->tx = txPin;
        this->rx = rxPin;
    }

    void begin(unsigned long baudRate = 115200)
    {
        this->baudRate = baudRate;

        if (rx >= 0)
        {
            setupRx(rx);
        }

        if (tx >= 0)
        {
            setupTx(tx);
        }
    }

    void begin(unsigned long baudRate, uint16_t config)
    {
        begin(baudRate);
    }

    void end()
    {
        // TODO - implement
    }

    int available()
    {
        return pio_sm_get_rx_fifo_level(pio, sm_rx);
    }

    int peek()
    {
        peekValue = read();
        return peekValue;
    }

    int read()
    {
        if (peekValue != -1)
        {
            int result = peekValue;
            peekValue = -1;
            return result;
        }
        // 8-bit read from the uppermost byte of the FIFO, as data is left-justified
        io_rw_8 *rxfifo_shift = (io_rw_8 *)&pio->rxf[sm_rx] + 3;
        if (pio_sm_is_rx_fifo_empty(pio, sm_rx))
            return -1;

        tight_loop_contents();
        return (char)*rxfifo_shift;
    }

    void flush()
    {
        // should always be writable
        return;
    }

    operator bool()
    {
        return true;
    }

    size_t write(uint8_t c)
    {
        pio_sm_put_blocking(pio, sm_tx, (uint32_t)c);
        return 1;
    }

    using Print::print;   // pull in write(str) and write(buf, size) from Print
    using Print::println; // pull in write(str) and write(buf, size) from Print
    using Print::write;   // pull in write(str) and write(buf, size) from Print

protected:
    uint baudRate;
    int tx;
    int rx;

    PIO pio;
    uint sm_rx;
    uint sm_tx;
    int offset;
    int peekValue = -1;

    // static uint8_t num_serials = 0; // how many serial objects have been created - used to offset the pio num

    void setupRx(uint pin_rx)
    {
        pio_sm_set_consecutive_pindirs(pio, sm_rx, pin_rx, 1, false);
        pio_gpio_init(pio, pin_rx);
        gpio_pull_up(pin_rx);

        int offset = pio_add_program(pio, &pio_uart_rx_program);
        pio_sm_config c = pio_uart_rx_program_get_default_config(offset);
        sm_config_set_in_pins(&c, pin_rx); // for WAIT, IN
        sm_config_set_jmp_pin(&c, pin_rx); // for JMP
        // Shift to right, autopull disabled
        sm_config_set_in_shift(&c, true, false, 32);
        // Deeper FIFO as we're not doing any TX
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
        // SM transmits 1 bit per 8 execution cycles.
        float div = (float)clock_get_hz(clk_sys) / (8 * baudRate);
        sm_config_set_clkdiv(&c, div);

        pio_sm_init(pio, sm_rx, offset, &c);
        pio_sm_set_enabled(pio, sm_rx, true);
    }

    void setupTx(uint pin_tx)
    {
        // Tell PIO to initially drive output-high on the selected pin, then map PIO
        // onto that pin with the IO muxes.
        pio_sm_set_pins_with_mask(pio, sm_tx, 1u << pin_tx, 1u << pin_tx);
        pio_sm_set_pindirs_with_mask(pio, sm_tx, 1u << pin_tx, 1u << pin_tx);
        pio_gpio_init(pio, pin_tx);

        int offset = pio_add_program(pio, &pio_uart_tx_program);
        pio_sm_config c = pio_uart_tx_program_get_default_config(offset);

        // OUT shifts to right, no autopull
        sm_config_set_out_shift(&c, true, false, 32);

        // We are mapping both OUT and side-set to the same pin, because sometimes
        // we need to assert user data onto the pin (with OUT) and sometimes
        // assert constant values (start/stop bit)
        sm_config_set_out_pins(&c, pin_tx, 1);
        sm_config_set_sideset_pins(&c, pin_tx);

        // We only need TX, so get an 8-deep FIFO!
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

        // SM transmits 1 bit per 8 execution cycles.
        float div = (float)clock_get_hz(clk_sys) / (8 * baudRate);
        sm_config_set_clkdiv(&c, div);

        pio_sm_init(pio, sm_tx, offset, &c);
        pio_sm_set_enabled(pio, sm_tx, true);
    }
};