#include <msp430g2553.h>
#include "bq32000.h"
#define SDA     BIT7		// SDA on P1.7
#define SCL		BIT6     	// SCL on P1.6

void ConfigI2C()
{
	 P1DIR |= SCL;                         
	 P1DIR &= ~SDA; 
	 
	 do
	 {
		P1OUT &= ~SCL;
		__no_operation(); 
		P1OUT |= SCL;
		__no_operation(); 
	 }
	 while(P1IN&SDA == 0);
	 
	 P1SEL |= SCL + SDA;					// SDA and SCL is true
	 P1SEL2 |= SCL + SDA;					// SDA and SCL is true
	 
	 UCB0CTL0 |= UCMST + UCMODE_3 + UCSYNC;	// Master mode select, I2C mode, synchronous mode enable
	 UCB0CTL1 |= UCSSEL_2 + UCSWRST;		// USCI clock from SMCLK and Enable SW reset
	 
	 UCB0BR0 = 10;							// value = SMCLK / 100khz 
	 UCB0BR1 = 0;							// value = SMCLK / 100khz 
	 
	 UCB0I2CSA = 0x68;						// Set slave address
	 UCB0I2CIE |= UCNACKIE; 				// Cho phep ngat acnk recever
	 UCB0CTL1 &= ~UCSWRST;       			// Clear SW reset, resume operation	
}

//==============================================================
unsigned char Read_RTC(unsigned char address)
{
	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent	
	
	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition       
	
	while (!(IFG2&UCB0TXIFG));     
	UCB0TXBUF = address;					// Address to read

	while (!(IFG2&UCB0TXIFG));  

	UCB0CTL1 &= ~UCTR;                      // I2C RX 
	UCB0CTL1 |= UCTXSTT;                    // I2C start condition	
	IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
	
	while (UCB0CTL1 & UCTXSTT);             // Loop until I2C STT is sent
    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
	
	return(UCB0RXBUF);
}

//=========================================================================
void Read_all_RTC(RTC_TIME* time)
{
	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent	
	
	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition       
	
	while (!(IFG2&UCB0TXIFG));     
	UCB0TXBUF = 0x00;						// Read from second

	while (!(IFG2&UCB0TXIFG));  

	UCB0CTL1 &= ~UCTR;                      // I2C RX 
	UCB0CTL1 |= UCTXSTT;                    // I2C start condition	
	IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
	
	while (UCB0CTL1 & UCTXSTT);             // Loop until I2C STT is sent
	
	time->seconds = UCB0RXBUF;                 
	
	while (!(IFG2&UCB0RXIFG));  
	time->minutes = UCB0RXBUF;
	
	while (!(IFG2&UCB0RXIFG));  
	time->hours = UCB0RXBUF;
	
	while (!(IFG2&UCB0RXIFG));  
	time->day = UCB0RXBUF;                     
	
	while (!(IFG2&UCB0RXIFG));  
	time->date = UCB0RXBUF;                    
	
	while (!(IFG2&UCB0RXIFG));  
	time->month = UCB0RXBUF;                    
	
	while (!(IFG2&UCB0RXIFG));  
	time->year = UCB0RXBUF;                   
	

    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
	
}

//=========================================================================
void Write_RTC(RTC_TIME* time)
{

	while (UCB0CTL1 & UCTXSTP);             // Loop until I2C STT is sent
	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition

	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = 0x00; 						// adress

	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = time->seconds;
	
	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = time->minutes;
	
	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = time->hours;
	
	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = time->day;
	
	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = time->date;
	
	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = time->month;
	
	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = time->year;
	
	while (!(IFG2&UCB0TXIFG));	
	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
	IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag 
}	

//==========================================================================
void CAL_RTC(void)
{
	while (UCB0CTL1 & UCTXSTP);             // Loop until I2C STT is sent
	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
	UCB0TXBUF = 0x07;						// CAL_CFG1 register address

	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = 0xFF;						//  Enable the test signal on the IRQ pin and speed the RTC to -126ppm
	
	while (!(IFG2&UCB0TXIFG));

	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
	IFG2 &= ~UCB0TXIFG;       
	
	while (UCB0CTL1 & UCTXSTP);             // Loop until I2C STT is sent
	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
	UCB0TXBUF = 0x20;						// CAL_CFG1 register address
	
	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = 0x5E;						//  SF KEY 1

	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = 0xC7;						// SF KEY 2

	while (!(IFG2&UCB0TXIFG)); 
	UCB0TXBUF = 0x01;						// SFR, force calibration to 1 Hz

	while (!(IFG2&UCB0TXIFG));

	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
	IFG2 &= ~UCB0TXIFG;       
}	