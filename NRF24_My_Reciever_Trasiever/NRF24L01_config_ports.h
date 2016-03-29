/*
 * NRF24L01_config_ports.h
 *
 * Created: 23.03.2016 10:24:01
 *  Author: AndreyMaznyak
 */ 


#ifndef NRF24L01_CONFIG_PORTS_H_
#define NRF24L01_CONFIG_PORTS_H_

// Зависящие от устройства настройки

//-->atmega8

//для <util/delay> зависит от тактовой частоты 8мг
#define F_CPU 1000000L

//SPI
#define SWSPI_DDR DDRB
#define SWSPI_PORT PORTB
#define SWSPI_PIN PINB

#define SWSPI_MOSI 3
#define SWSPI_MISO 4
#define SWSPI_SCK 5

//CSN CE IRQ
#define RADIO_PORT PORTB
#define RADIO_DDR DDRB
#define RADIO_PIN PINB

#define RADIO_IRQ_DDR DDRD
#define RADIO_IRQ_PIN PIND

#define RADIO_CSN 1
#define RADIO_CE 0
#define RADIO_IRQ 2

//<--atmega8


#endif /* NRF24L01_CONFIG_PORTS_H_ */