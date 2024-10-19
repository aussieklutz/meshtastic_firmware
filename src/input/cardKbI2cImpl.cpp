#include "cardKbI2cImpl.h"
#include "InputBroker.h"
#include "detect/ScanI2CTwoWire.h"
#include "main.h"

CardKbI2cImpl *cardKbI2cImpl;

CardKbI2cImpl::CardKbI2cImpl() : KbI2cBase("cardKB") {}

void CardKbI2cImpl::init()
{
#if !MESHTASTIC_EXCLUDE_I2C && !defined(ARCH_PORTDUINO)
    if (cardkb_found.address == 0x00) {
        LOG_DEBUG("Rescanning for I2C keyboard\n");
        uint8_t i2caddr_scan[] = {CARDKB_ADDR, TDECK_KB_ADDR, BBQ10_KB_ADDR, MPR121_KB_ADDR};
        uint8_t i2caddr_asize = 4;
        auto i2cScanner = std::unique_ptr<ScanI2CTwoWire>(new ScanI2CTwoWire());

#if WIRE_INTERFACES_COUNT == 2
        i2cScanner->scanPort(ScanI2C::I2CPort::WIRE1, i2caddr_scan, i2caddr_asize);
#endif
        i2cScanner->scanPort(ScanI2C::I2CPort::WIRE, i2caddr_scan, i2caddr_asize);
        auto kb_info = i2cScanner->firstKeyboard();
        LOG_DEBUG("Detected Keyboard Address: %02x\n", kb_info.address.address);

        if (kb_info.type != ScanI2C::DeviceType::NONE) {
            cardkb_found = kb_info.address;
            switch (kb_info.type) {
            case ScanI2C::DeviceType::RAK14004:
                kb_model = 0x02;
                cardkb_found.address = CARDKB_ADDR;
                LOG_DEBUG("Keyboard is RAK14004\n");
                break;
            case ScanI2C::DeviceType::CARDKB:
                kb_model = 0x00;
                cardkb_found.address = CARDKB_ADDR;
                LOG_DEBUG("Keyboard is CARDKB\n");
                break;
            case ScanI2C::DeviceType::TDECKKB:
                // assign an arbitrary value to distinguish from other models
                kb_model = 0x10;
                cardkb_found.address = TDECK_KB_ADDR;
                LOG_DEBUG("Keyboard is TDECKKB\n");
                break;
            case ScanI2C::DeviceType::BBQ10KB:
                // assign an arbitrary value to distinguish from other models
                kb_model = 0x11;
                cardkb_found.address = BBQ10_KB_ADDR;
                LOG_DEBUG("Keyboard is BBQ10KB\n");
                break;
            case ScanI2C::DeviceType::MPR121KB:
                // assign an arbitrary value to distinguish from other models
                kb_model = 0x37;
                cardkb_found.address = MPR121_KB_ADDR;
                LOG_DEBUG("Keyboard is MPR121KB\n");
                break;
            default:
                // use this as default since it's also just zero
                LOG_WARN("kb_info.type is unknown(0x%02x), setting kb_model=0x00\n", kb_info.type);
                kb_model = 0x00;
            }
        }
        LOG_DEBUG("Keyboard Type: 0x%02x Model: 0x%02x Address: 0x%02x\n", kb_info.type, kb_model, cardkb_found.address);
        if (cardkb_found.address == 0x00) {
            disable();
            return;
        }
    }
#else
    if (cardkb_found.address == 0x00) {
        disable();
        return;
    }
#endif
    inputBroker->registerSource(this);
}