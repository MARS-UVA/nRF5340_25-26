#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(potentiometer, LOG_LEVEL_DBG);

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

// https://academy.nordicsemi.com/courses/nrf-connect-sdk-intermediate/lessons/lesson-6-analog-to-digital-converter-adc/topic/exercise-1-interfacing-with-adc-using-zephyr-api/
// https://github.com/NordicDeveloperAcademy/ncs-inter/blob/main/l6/l6_e1_sol/src/main.c

int demo()
{
    int err;
    uint32_t count = 0;

    int16_t buf;
    struct adc_sequence sequence = {
        .buffer = &buf,
        .buffer_size = sizeof(buf),
        //.calibrate = true,
    };

    if (!adc_is_ready_dt(&adc_channel))
    {
        LOG_ERR("ADC controller device %s not ready", adc_channel.dev->name);
        return 0;
    }

    err = adc_channel_setup_dt(&adc_channel);
    if (err < 0)
    {
        LOG_ERR("Could not setup channel #%d (%d)", 0, err);
        return 0;
    }

    err = adc_sequence_init_dt(&adc_channel, &sequence);
    if (err < 0)
    {
        LOG_ERR("Could not initialize sequence");
        return 0;
    }

    while (1)
    {
        err = adc_read(adc_channel.dev, &sequence);
        if (err < 0)
        {
            LOG_ERR("Could not read (%d)", err);
            continue;
        }

        int val = (int)(*(int16_t *)sequence.buffer);

        err = adc_raw_to_millivolts_dt(&adc_channel, &val);
        if (err < 0)
        {
            LOG_ERR("Conversion to millivolts failed, raw: %d", val);
            continue;
        }

        LOG_INF("[%u] Pot reading (%s, ch. %d): %d mV", count++, adc_channel.dev->name, adc_channel.channel_id, val);
        k_msleep(500);
    }

    return 0;
}