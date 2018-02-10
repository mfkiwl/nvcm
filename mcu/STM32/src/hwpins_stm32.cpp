// hwpins_stm32.cpp

#include "platform.h"
#include "hwpins.h"

#if defined(GPIOK_BASE)
  #define MAX_PORT_NUMBER 11
#elif defined(GPIOJ_BASE)
  #define MAX_PORT_NUMBER 10
#elif defined(GPIOI_BASE)
  #define MAX_PORT_NUMBER  9
#elif defined(GPIOH_BASE)
  #define MAX_PORT_NUMBER  8
#elif defined(GPIOG_BASE)
  #define MAX_PORT_NUMBER  7
#elif defined(GPIOF_BASE)
  #define MAX_PORT_NUMBER  6
#elif defined(GPIOE_BASE)
  #define MAX_PORT_NUMBER  5
#elif defined(GPIOC_BASE)
  #define MAX_PORT_NUMBER  4
#else
  #define MAX_PORT_NUMBER  3
#endif

GPIO_TypeDef * THwPinCtrl_stm32::GetGpioRegs(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return nullptr;
	}
	else
	{
		return (GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(aportnum));
	}
}

bool THwPinCtrl_stm32::PinSetup(int aportnum, int apinnum, unsigned flags)
{
	// STM32 version
	HW_GPIO_REGS * regs = GetGpioRegs(aportnum);
	if (!regs)
	{
		return false;
	}

	if ((apinnum < 0) || (apinnum > 15))
	{
		return false;
	}

	// 1. turn on port power
	GpioPortEnable(aportnum);

#if defined(MCUSF_F1)
  unsigned pinconf = 0;

  if (flags & PINCFG_OUTPUT)
  {
  	if ((flags & PINCFG_SPEED_MASK) == PINCFG_SPEED_SLOW)
		{
			pinconf |= 2;
		}
		else if ((flags & PINCFG_SPEED_MASK) == PINCFG_SPEED_FAST)
		{
			pinconf |= 3;
		}
		else
		{
			pinconf |= 1;
		}

		if (flags & PINCFG_OPENDRAIN)
		{
			pinconf |= 4;
		}

		if (flags & PINCFG_AF_MASK)
		{
			pinconf |= 8;
		}
  }
  else  // input
  {
  	// leave the mode bits at 0 = input mode
    if (flags & PINCFG_PULLUP)
    {
    	pinconf |= 8;
    	regs->BSRR = (1 << apinnum);
    }
    else if (flags & PINCFG_PULLDOWN)
    {
    	pinconf |= 8;
    	regs->BSRR = (1 << apinnum) << 16;
    }
    else
    {
    	pinconf |= 4;  // floating input
    }
  }

  // set the pinconf reg
  if (apinnum < 8)
  {
  	regs->CRL &= ~(0xF << (apinnum * 4));
  	regs->CRL |= pinconf << (apinnum * 4);
  }
  else
  {
  	regs->CRH &= ~(0xF << ((apinnum-8) * 4));
  	regs->CRH |= pinconf << ((apinnum-8) * 4);
  }

#else  // F0, F4, F7

  unsigned n;
  int pinx2 = apinnum * 2;

  // set mode register
	if (flags & PINCFG_AF_MASK)
	{
		n = 2;  // set alternate function mode
	}
	else if (flags & PINCFG_ANALOGUE)
	{
		n = 3;
	}
	else if (flags & PINCFG_OUTPUT)
	{
		n = 1;
	}
	else
	{
	  n = 0;
	}
	regs->MODER &= ~(3 << pinx2);
	regs->MODER |= (n << pinx2);

  // 3. set open-drain
  if (flags & PINCFG_OPENDRAIN)
  {
  	regs->OTYPER |= (1 << apinnum);
  }
  else
  {
  	regs->OTYPER &= ~(1 << apinnum);
  }

  // 4. set pullup / pulldown
  regs->PUPDR &= ~(3 << pinx2);
  if (flags & PINCFG_PULLUP)
  {
  	regs->PUPDR &= ~(1 << pinx2); // pullup
  }
  else if (flags & PINCFG_PULLDOWN)
  {
  	regs->PUPDR &= ~(2 << pinx2); // pulldown
  }

  // 5. set speed
  regs->OSPEEDR &= ~(3 << pinx2);
  if ((flags & PINCFG_SPEED_MASK) == PINCFG_SPEED_MEDIUM)
  {
  	regs->OSPEEDR |= ~(1 << pinx2);
  }
  else if ((flags & PINCFG_SPEED_MASK) == PINCFG_SPEED_FAST)
  {
  	regs->OSPEEDR |= ~(3 << pinx2);
  }

	if (flags & PINCFG_AF_MASK)
	{
		// set the alternate function
		n = ((flags >> PINCFG_AF_SHIFT) & 0xF);

		if (apinnum < 8)
		{
			regs->AFR[0] &= ~(0xF << (apinnum * 4));
			regs->AFR[0] |= (n << (apinnum * 4));
		}
		else
		{
			regs->AFR[1] &= ~(0xF << ((apinnum-8) * 4));
			regs->AFR[1] |= (n << ((apinnum-8) * 4));
		}
	}

#endif

  // 6. initial state
  if (flags & PINCFG_GPIO_INIT_1)
  {
  	regs->BSRR = (1 << apinnum);
  }
  else
  {
  	regs->BSRR = (1 << apinnum) << 16;
  }

  return true;
}

bool THwPinCtrl_stm32::GpioPortEnable(int aportnum)
{
	if ((aportnum < 0) || (aportnum >= MAX_PORT_NUMBER))
	{
		return false;
	}

	// 1. turn on port power
#if defined(RCC_IOPENR_IOPAEN)
  RCC->IOPENR |= (RCC_IOPENR_IOPAEN << aportnum);
#elif defined(RCC_AHBENR_GPIOAEN)
  RCC->AHBENR |= (RCC_AHBENR_GPIOAEN << aportnum);
#elif defined(RCC_APB2ENR_IOPAEN)
  RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN << aportnum);
#else
  RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN << aportnum);
#endif

  return true;
}

void THwPinCtrl_stm32::GpioSet(int aportnum, int apinnum, int value)
{
	GPIO_TypeDef * regs = (GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(aportnum));

  if (1 == value)
  {
  	regs->BSRR = (1 << apinnum);
  }
  else if (value & 2) // toggle
  {
  	regs->ODR ^= (1 << apinnum);
  }
  else
  {
#if defined(MCUSF_F0) || defined(MCUSF_F1)
  	regs->BRR = (1 << apinnum);
#else
  	regs->BSRR = (1 << apinnum) << 16;
#endif
  }
}

// GPIO Port

void TGpioPort_stm32::Assign(int aportnum)
{
	if ((aportnum < 0) || (aportnum > 6))
	{
		regs = nullptr;
		return;
	}

  regs = (HW_GPIO_REGS *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(aportnum));
  portptr = (volatile unsigned *)&(regs->ODR);
}

void TGpioPort_stm32::Set(unsigned value)
{
  *portptr = value;
}

// GPIO Pin

void TGpioPin_stm32::Assign(int aportnum, int apinnum, bool ainvert)
{
	portnum = aportnum;
  pinnum = apinnum;
  inverted = ainvert;

	regs = hwpinctrl.GetGpioRegs(aportnum);
	if (!regs)
	{
		return;
	}

	if ((apinnum < 0) || (apinnum > 15))
	{
		regs = nullptr;
		return;
	}

  setbitptr = (unsigned *)&(regs->BSRR);
  clrbitptr = (unsigned *)&(regs->BSRR);
  getbitptr = (unsigned *)&(regs->IDR);
  getbitshift = apinnum;

  if (ainvert)
  {
  	setbitvalue = (1 << pinnum) << 16;
  	clrbitvalue = (1 << pinnum);
  }
  else
  {
  	setbitvalue = (1 << pinnum);
  	clrbitvalue = (1 << pinnum) << 16;
  }
}

void TGpioPin_stm32::Toggle()
{
  unsigned pinbit = (1 << pinnum);
  regs->BSRR = (pinbit << 16) | (regs->ODR ^ pinbit);
}

void TGpioPin_stm32::SwitchDirection(int adirection)
{
#if defined(MCUSF_F1)

	// here we override previous pull-up/down open-drain settings !

	unsigned pinconf;

  if (adirection)
  {
  	pinconf = 1;  // push-pull output
  }
  else
  {
  	pinconf = 4;  // floating input
  }

  if (pinnum < 8)
  {
  	regs->CRL &= ~(0xF << (pinnum * 4));
  	regs->CRL |= pinconf << (pinnum * 4);
  }
  else
  {
  	regs->CRH &= ~(0xF << ((pinnum-8) * 4));
  	regs->CRH |= pinconf << ((pinnum-8) * 4);
  }

#else
  int pinx2 = pinnum * 2;

  if (adirection)
  {
  	regs->MODER |= (1 << pinx2);
  }
  else
  {
  	regs->MODER &= ~(1 << pinx2);
  }
#endif
}

