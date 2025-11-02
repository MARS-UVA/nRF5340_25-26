#include "talon_srx.h"

void talon_srx_send_can_message(talon_srx_t *self, int msg_id, char *message, uint8_t length)
{
    send_can_message(self->can_dev, self->identifier | msg_id, message, length);
}

void talon_srx_set(talon_srx_t *self, double speed)
{
    short valueInt = (short)(speed * 1024);
    if (valueInt < 0)
    {
        valueInt = 0xfff - (-1 * valueInt);
    }

    talon_srx_send_can_message(self, 0x2040200, (char[]){(valueInt >> 16) & 255, (valueInt >> 8) & 255, valueInt & 255, 0, 0, 0, 0x0b, self->inverted ? 0x40 : 0x00}, 8);
}

void talon_srx_set_inverted(talon_srx_t *self, bool invert)
{
    self->inverted = invert;
}

bool talon_srx_init(talon_srx_t *instance, const struct device *can_dev, int identifier, bool inverted)
{
    if (!instance)
        return false;

    *instance = (talon_srx_t){
        .initialized = true,
        .can_dev = (struct device *)can_dev,
        .identifier = identifier,
        .set = talon_srx_set,
        .set_inverted = talon_srx_set_inverted,
        .inverted = inverted,
    };

    return true;
}
