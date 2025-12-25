#include "ch32fun.h"

// Simple delay functions
void Delay_Us(uint32_t n)
{
    uint32_t count = n * (FUNCONF_SYSTEM_CORE_CLOCK / 4000000);
    asm volatile(
        "1: addi %[count], %[count], -1\n"
        "   bnez %[count], 1b"
        : [count] "+r"(count)
        );
}

void Delay_Ms(uint32_t n)
{
    while (n--)
    {
        Delay_Us(1000);
    }
}

void LED_Flash_Test(void)
{
    for (int i = 0; i < 3; i++)
    {
        GPIOC->BCR = (1 << 2);  // ON (reset bit = low)
        Delay_Ms(500);
        GPIOC->BSHR = (1 << 2); // OFF (set bit = high)
        Delay_Ms(500);
    }
    Delay_Ms(500);
}

void Software_PWM(uint8_t duty_cycle)
{
    if (duty_cycle == 0)
    {
        GPIOC->BSHR = (1 << 2);  // OFF
    }
    else if (duty_cycle >= 100)
    {
        GPIOC->BCR = (1 << 2);  // ON
    }
    else
    {
        GPIOC->BCR = (1 << 2);
        Delay_Us(duty_cycle * 10);
        GPIOC->BSHR = (1 << 2);
        Delay_Us((100 - duty_cycle) * 10);
    }
}

int main(void)
{
    uint8_t brightness = 0;
    uint8_t direction = 1;
    uint16_t cycle_count = 0;

    SystemInit();

    // Enable GPIOC clock
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

    // Configure PC2 as push-pull output, 10MHz
    GPIOC->CFGLR &= ~(0xf << (4 * 2));
    GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 2);

    LED_Flash_Test();

    while (1)
    {
        // Call software PWM multiple times for smooth operation
        for (int i = 0; i < 50; i++)
        {
            Software_PWM(brightness);
        }

        // Update brightness
        cycle_count++;
        if (cycle_count >= 10)
        {
            cycle_count = 0;

            if (direction)
            {
                brightness += 5;
                if (brightness >= 100)
                {
                    brightness = 100;
                    direction = 0;
                }
            }
            else
            {
                brightness -= 5;
                if (brightness <= 0)
                {
                    brightness = 0;
                    direction = 1;
                }
            }
        }
    }
}