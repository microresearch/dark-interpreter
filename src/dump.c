
void switch_jack(void)
{
  // clear first 3 and JACKOUT is on

  //  GPIO_SetBits(GPIOB, GPIO_Pin_2); 

  //  GPIO_SetBits(GPIOC, GPIO_Pin_8); // feedback on PC8

  GPIOB->ODR &= ~(7); // should clear more than this?
  GPIOB->ODR = JACKOUT;// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

}


void test_filter(void)
{
  // connect the filter in signal path:

  //- SW1 PC11 - out to filter PC10 also 1

  GPIOC->ODR |= (1<<11) | (1<<10); //test distortion in filter = remove 1<<10

  GPIOB->ODR &= ~(7); ///???
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));

  //- SW4 PB4 - filter to jack

  GPIOB->ODR |= FILTIN;// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in
  GPIOB->ODR |= (1<<8) | (1<<9);// test switch - puts left in/out across
  // without left in/out across we have ringing on LM!

}

void test_40106(void)
{



  //  GPIOC->ODR |= (1<<10); //test distortion in filter = remove 1<<10

  //- SW0 PB0 - out to 40106
  //- SW6 PB6 - 40106 to jack

  GPIOB->ODR &= ~(7); ///???
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));


  GPIOB->ODR = 1 | (1<<6);// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

  // was |=

}

/* fill in rest of switch functions - also to leave all floating for
   pads as HW option FLOATIT */

void test_filtand40106(void)
{

  /*
- SW3 PB3 - filter to 40106
- SW6 PB6 - 40106 to jack
- SW1 PC11 - out to filter
  */

  // connect the filter in signal path:

  //- SW1 PC11 - out to filter PC10 also 1

  GPIOC->ODR |= (1<<11) | (1<<10);   //test distortion in filter = remove 1<<10


  //- SW4 PB3 and PB6

  GPIOB->ODR &= ~(7);
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));
  GPIOB->ODR |= (1<<8) | (1<<9);// test switch - puts left in/out across

  GPIOB->ODR |= (1<<3) | (1<<6);// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in
}

void test_40106andfilt(void)
{
  /*
- SW0 PB0 - out to 40106
- SW4 PB4 - filter to jack
- SW5 PB5 - 40106 to filter
  */

  GPIOC->ODR |= (1<<10); //test distortion in filter = remove 1<<10


  GPIOB->ODR &= ~(7);
  GPIOB->ODR &= ~((1<<7) | (1<<8) | (1<<9));
  GPIOB->ODR |= (1<<8) | (1<<9);// test switch - puts left in/out across

  GPIOB->ODR |= (1<<0 | (1<<4)) | (1<<5);// | LINEINN;// lineinn should be zero - toggle lineinn for 4053=lm358in

}

