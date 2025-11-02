#ifndef INC_TALONSRX_H_
#define INC_TALONSRX_H_

#include "can.h"

typedef struct talon_srx
{
    bool initialized;
    struct device *can_dev;
    int identifier;
    bool inverted;

    // Function pointers for control actions
    void (*set)(struct talon_srx *self, double value);
    void (*set_inverted)(struct talon_srx *self, bool invert);
} talon_srx_t;

/**
 * @brief Initializes a TalonSRX structure.
 * @return Initialized TalonSRX instance.
 */
bool talon_srx_init(talon_srx_t *instance, const struct device *can_dev, int identifier, bool inverted);

#endif /* INC_TALONSRX_H_ */
