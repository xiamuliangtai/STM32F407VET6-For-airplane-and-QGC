#include "oled.h"

#include <string.h>

#include "main.h"

#define OLED_WIDTH                 128U
#define OLED_PAGES                 8U
#define OLED_I2C_ADDR_WRITE_0X3C   0x78U
#define OLED_I2C_ADDR_WRITE_0X3D   0x7AU
#define OLED_CTRL_CMD              0x00U
#define OLED_CTRL_DATA             0x40U

static uint8_t s_oled_ready;
static uint8_t s_oled_addr_write;
static uint8_t s_oled_buffer[OLED_WIDTH * OLED_PAGES];

static const uint8_t font_digits[10][5] =
{
    {0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU},
    {0x00U, 0x42U, 0x7FU, 0x40U, 0x00U},
    {0x42U, 0x61U, 0x51U, 0x49U, 0x46U},
    {0x21U, 0x41U, 0x45U, 0x4BU, 0x31U},
    {0x18U, 0x14U, 0x12U, 0x7FU, 0x10U},
    {0x27U, 0x45U, 0x45U, 0x45U, 0x39U},
    {0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U},
    {0x01U, 0x71U, 0x09U, 0x05U, 0x03U},
    {0x36U, 0x49U, 0x49U, 0x49U, 0x36U},
    {0x06U, 0x49U, 0x49U, 0x29U, 0x1EU}
};

static const uint8_t font_upper[26][5] =
{
    {0x7EU, 0x11U, 0x11U, 0x11U, 0x7EU}, /* A */
    {0x7FU, 0x49U, 0x49U, 0x49U, 0x36U}, /* B */
    {0x3EU, 0x41U, 0x41U, 0x41U, 0x22U}, /* C */
    {0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU}, /* D */
    {0x7FU, 0x49U, 0x49U, 0x49U, 0x41U}, /* E */
    {0x7FU, 0x09U, 0x09U, 0x09U, 0x01U}, /* F */
    {0x3EU, 0x41U, 0x49U, 0x49U, 0x7AU}, /* G */
    {0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU}, /* H */
    {0x00U, 0x41U, 0x7FU, 0x41U, 0x00U}, /* I */
    {0x20U, 0x40U, 0x41U, 0x3FU, 0x01U}, /* J */
    {0x7FU, 0x08U, 0x14U, 0x22U, 0x41U}, /* K */
    {0x7FU, 0x40U, 0x40U, 0x40U, 0x40U}, /* L */
    {0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU}, /* M */
    {0x7FU, 0x04U, 0x08U, 0x10U, 0x7FU}, /* N */
    {0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU}, /* O */
    {0x7FU, 0x09U, 0x09U, 0x09U, 0x06U}, /* P */
    {0x3EU, 0x41U, 0x51U, 0x21U, 0x5EU}, /* Q */
    {0x7FU, 0x09U, 0x19U, 0x29U, 0x46U}, /* R */
    {0x46U, 0x49U, 0x49U, 0x49U, 0x31U}, /* S */
    {0x01U, 0x01U, 0x7FU, 0x01U, 0x01U}, /* T */
    {0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU}, /* U */
    {0x1FU, 0x20U, 0x40U, 0x20U, 0x1FU}, /* V */
    {0x7FU, 0x20U, 0x18U, 0x20U, 0x7FU}, /* W */
    {0x63U, 0x14U, 0x08U, 0x14U, 0x63U}, /* X */
    {0x03U, 0x04U, 0x78U, 0x04U, 0x03U}, /* Y */
    {0x61U, 0x51U, 0x49U, 0x45U, 0x43U}  /* Z */
};

static void OLED_DelayShort(void)
{
    volatile uint32_t i;

    for (i = 0U; i < 200U; i++)
    {
        __NOP();
    }
}

static void OLED_SCL_Write(uint8_t level)
{
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, (level != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void OLED_SDA_Write(uint8_t level)
{
    HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, (level != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t OLED_SDA_Read(void)
{
    return (HAL_GPIO_ReadPin(OLED_SDA_GPIO_Port, OLED_SDA_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

static void OLED_I2C_Start(void)
{
    OLED_SDA_Write(1U);
    OLED_SCL_Write(1U);
    OLED_DelayShort();
    OLED_SDA_Write(0U);
    OLED_DelayShort();
    OLED_SCL_Write(0U);
}

static void OLED_I2C_Stop(void)
{
    OLED_SDA_Write(0U);
    OLED_DelayShort();
    OLED_SCL_Write(1U);
    OLED_DelayShort();
    OLED_SDA_Write(1U);
    OLED_DelayShort();
}

static uint8_t OLED_I2C_WriteByte(uint8_t value)
{
    uint8_t bit;

    for (bit = 0U; bit < 8U; bit++)
    {
        OLED_SDA_Write((value & 0x80U) != 0U);
        OLED_DelayShort();
        OLED_SCL_Write(1U);
        OLED_DelayShort();
        OLED_SCL_Write(0U);
        value <<= 1U;
    }

    OLED_SDA_Write(1U);
    OLED_DelayShort();
    OLED_SCL_Write(1U);
    OLED_DelayShort();
    bit = (uint8_t)(OLED_SDA_Read() == 0U);
    OLED_SCL_Write(0U);
    OLED_DelayShort();

    return bit;
}

static uint8_t OLED_WriteBytes(uint8_t control, const uint8_t *data, uint16_t len)
{
    uint16_t i;

    OLED_I2C_Start();
    if (OLED_I2C_WriteByte(s_oled_addr_write) == 0U)
    {
        OLED_I2C_Stop();
        return 0U;
    }

    if (OLED_I2C_WriteByte(control) == 0U)
    {
        OLED_I2C_Stop();
        return 0U;
    }

    for (i = 0U; i < len; i++)
    {
        if (OLED_I2C_WriteByte(data[i]) == 0U)
        {
            OLED_I2C_Stop();
            return 0U;
        }
    }

    OLED_I2C_Stop();
    return 1U;
}

static uint8_t OLED_WriteCommand(uint8_t cmd)
{
    return OLED_WriteBytes(OLED_CTRL_CMD, &cmd, 1U);
}

static const uint8_t *OLED_GetGlyph(char ch)
{
    static const uint8_t blank[5] = {0U, 0U, 0U, 0U, 0U};
    static const uint8_t colon[5] = {0x00U, 0x36U, 0x36U, 0x00U, 0x00U};
    static const uint8_t dash[5]  = {0x08U, 0x08U, 0x08U, 0x08U, 0x08U};

    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch = (char)(ch - ('a' - 'A'));
    }

    if ((ch >= '0') && (ch <= '9'))
    {
        return font_digits[(uint8_t)(ch - '0')];
    }

    if ((ch >= 'A') && (ch <= 'Z'))
    {
        return font_upper[(uint8_t)(ch - 'A')];
    }

    if (ch == ':')
    {
        return colon;
    }

    if (ch == '-')
    {
        return dash;
    }

    return blank;
}

static uint8_t OLED_TryInitAtAddress(uint8_t addr_write)
{
    static const uint8_t init_cmds[] =
    {
        0xAEU,
        0xD5U, 0x80U,
        0xA8U, 0x3FU,
        0xD3U, 0x00U,
        0x40U,
        0x8DU, 0x14U,
        0x20U, 0x02U,
        0xA1U,
        0xC8U,
        0xDAU, 0x12U,
        0x81U, 0xCFU,
        0xD9U, 0xF1U,
        0xDBU, 0x40U,
        0xA4U,
        0xA6U,
        0x2EU,
        0xAFU
    };
    uint8_t i;

    s_oled_addr_write = addr_write;

    for (i = 0U; i < (uint8_t)sizeof(init_cmds); i++)
    {
        if (OLED_WriteCommand(init_cmds[i]) == 0U)
        {
            return 0U;
        }
    }

    return 1U;
}

static void OLED_DrawChar(uint8_t x, uint8_t page, char ch)
{
    const uint8_t *glyph;
    uint16_t base;
    uint8_t i;

    if ((x > (OLED_WIDTH - 6U)) || (page >= OLED_PAGES))
    {
        return;
    }

    glyph = OLED_GetGlyph(ch);
    base = (uint16_t)page * OLED_WIDTH + x;

    for (i = 0U; i < 5U; i++)
    {
        s_oled_buffer[base + i] = glyph[i];
    }
    s_oled_buffer[base + 5U] = 0x00U;
}

static void OLED_DrawBorder(void)
{
    uint8_t x;
    uint8_t page;

    for (x = 0U; x < OLED_WIDTH; x++)
    {
        s_oled_buffer[x] = 0x01U;
        s_oled_buffer[((OLED_PAGES - 1U) * OLED_WIDTH) + x] = 0x80U;
    }

    for (page = 0U; page < OLED_PAGES; page++)
    {
        s_oled_buffer[(page * OLED_WIDTH)] = 0xFFU;
        s_oled_buffer[(page * OLED_WIDTH) + (OLED_WIDTH - 1U)] = 0xFFU;
    }
}

uint8_t OLED_Init(void)
{
    static const uint8_t addr_candidates[] =
    {
        OLED_I2C_ADDR_WRITE_0X3C,
        OLED_I2C_ADDR_WRITE_0X3D
    };
    uint8_t i;

    OLED_SCL_Write(1U);
    OLED_SDA_Write(1U);
    HAL_Delay(20U);

    s_oled_ready = 0U;
    s_oled_addr_write = OLED_I2C_ADDR_WRITE_0X3C;
    for (i = 0U; i < (uint8_t)sizeof(addr_candidates); i++)
    {
        if (OLED_TryInitAtAddress(addr_candidates[i]) != 0U)
        {
            s_oled_ready = 1U;
            break;
        }
    }

    if (s_oled_ready != 0U)
    {
        OLED_Clear();
        OLED_Refresh();
    }

    return s_oled_ready;
}

uint8_t OLED_IsReady(void)
{
    return s_oled_ready;
}

void OLED_Clear(void)
{
    memset(s_oled_buffer, 0, sizeof(s_oled_buffer));
}

void OLED_Refresh(void)
{
    uint8_t page;

    if (s_oled_ready == 0U)
    {
        return;
    }

    for (page = 0U; page < OLED_PAGES; page++)
    {
        (void)OLED_WriteCommand((uint8_t)(0xB0U + page));
        (void)OLED_WriteCommand(0x00U);
        (void)OLED_WriteCommand(0x10U);
        (void)OLED_WriteBytes(OLED_CTRL_DATA, &s_oled_buffer[page * OLED_WIDTH], OLED_WIDTH);
    }
}

void OLED_ShowString(uint8_t x, uint8_t page, const char *str)
{
    if ((str == 0) || (page >= OLED_PAGES))
    {
        return;
    }

    while ((*str != '\0') && (x <= (OLED_WIDTH - 6U)))
    {
        OLED_DrawChar(x, page, *str);
        x = (uint8_t)(x + 6U);
        str++;
    }
}

void OLED_ShowUInt(uint8_t x, uint8_t page, uint32_t value, uint8_t width)
{
    uint8_t digits[10];
    uint8_t i;

    if ((width == 0U) || (width > 10U))
    {
        return;
    }

    for (i = 0U; i < width; i++)
    {
        digits[width - 1U - i] = (uint8_t)('0' + (value % 10U));
        value /= 10U;
    }

    for (i = 0U; i < width; i++)
    {
        OLED_DrawChar((uint8_t)(x + (i * 6U)), page, (char)digits[i]);
    }
}

void OLED_ShowTestScreen(uint32_t counter, uint8_t gs_online)
{
    if (s_oled_ready == 0U)
    {
        return;
    }

    OLED_Clear();
    OLED_DrawBorder();
    OLED_ShowString(8U, 0U, "STM32 OLED TEST");
    OLED_ShowString(16U, 2U, "OLED READY");
    OLED_ShowString(16U, 4U, "CNT:");
    OLED_ShowUInt(46U, 4U, counter, 6U);
    OLED_ShowString(16U, 6U, "GS:");
    OLED_ShowString(34U, 6U, (gs_online != 0U) ? "ON" : "OFF");
    OLED_Refresh();
}

void OLED_ShowGsRxScreen(uint8_t gs_online,
                         uint8_t rx_status,
                         uint8_t rx_code,
                         uint8_t rx_seq,
                         uint8_t mission_valid,
                         uint8_t mission_seq,
                         uint8_t mission_points)
{
    if (s_oled_ready == 0U)
    {
        return;
    }

    OLED_Clear();
    OLED_DrawBorder();
    OLED_ShowString(14U, 0U, "GS RX STATUS");

    OLED_ShowString(8U, 2U, "LINK:");
    OLED_ShowString(44U, 2U, (gs_online != 0U) ? "ON" : "OFF");

    OLED_ShowString(8U, 3U, "RX:");
    if (rx_status == 1U)
    {
        OLED_ShowString(26U, 3U, "OK");
    }
    else if (rx_status == 2U)
    {
        OLED_ShowString(26U, 3U, "ERR");
    }
    else
    {
        OLED_ShowString(26U, 3U, "WAIT");
    }

    OLED_ShowString(8U, 4U, "SEQ:");
    if (rx_status == 0U)
    {
        OLED_ShowString(32U, 4U, "---");
    }
    else if ((rx_status == 1U) && (mission_valid != 0U))
    {
        OLED_ShowUInt(32U, 4U, mission_seq, 3U);
    }
    else
    {
        OLED_ShowUInt(32U, 4U, rx_seq, 3U);
    }

    OLED_ShowString(8U, 5U, "PTS:");
    if ((rx_status == 1U) && (mission_valid != 0U))
    {
        OLED_ShowUInt(32U, 5U, mission_points, 3U);
    }
    else
    {
        OLED_ShowString(32U, 5U, "---");
    }

    OLED_ShowString(8U, 6U, "CODE:");
    if (rx_status == 0U)
    {
        OLED_ShowString(38U, 6U, "--");
    }
    else
    {
        OLED_ShowUInt(38U, 6U, rx_code, 2U);
    }

    OLED_Refresh();
}
