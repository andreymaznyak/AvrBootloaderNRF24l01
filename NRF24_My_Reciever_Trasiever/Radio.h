/*
 * Radio.h
 *
 * Created: 23.03.2016 10:27:37
 *  Author: AndreyMaznyak
 */ 


#ifndef RADIO_H_
#define RADIO_H_

#include "NRF24L01_config_ports.h"
#include "NRF24L01.h"
#include <util/delay.h>
//#include <avr/io.h>

// �������� �������� ��������� (������� �������) �� ����� CE
inline void radio_assert_ce() {
	RADIO_PORT |= (1 << RADIO_CE); // ��������� �������� ������ �� ����� CE
}

// �������� ���������� ��������� (������ �������) �� ����� CE
inline void radio_deassert_ce() {
	RADIO_PORT &= ~(1 << RADIO_CE); // ��������� ������� ������ �� ����� CE
}

// ��������� ������� ��� ������ � csn �� �������������� ������������ � ���� ������, �� ����� �������� static

// �������� �������� ��������� (������ �������) �� ����� CSN
inline static void csn_assert() {
	RADIO_PORT &= ~(1 << RADIO_CSN); // ��������� ������� ������ �� ����� CSN
}

// �������� ���������� ��������� (������� �������) �� ����� CSN
inline static void csn_deassert() {
	RADIO_PORT |= (1 << RADIO_CSN); // ��������� �������� ������ �� ����� CSN
}

//
// SPI
// 

// ������� � ��������� 1 ���� �� SPI, ���������� ���������� ��������
uint8_t spi_send_recv(uint8_t data) {
	for (uint8_t i = 8; i > 0; i--) {
		if (data & 0x80)
		SWSPI_PORT |= (1 << SWSPI_MOSI); // �������� ��������
		else
		SWSPI_PORT &= ~(1 << SWSPI_MOSI); // �������� ������
		SWSPI_PORT |= (1 << SWSPI_SCK);
		data <<= 1;
		if (SWSPI_PIN & (1 << SWSPI_MISO)) // ������ ���� �� ����� MISO
		data |= 1;
		SWSPI_PORT &= ~(1 << SWSPI_SCK);
	}
	return data;
}


// �������������� �����
void radio_init() {
	RADIO_DDR |= (1 << RADIO_CSN) | (1 << RADIO_CE); // ����� CSN � CE �� �����
	RADIO_DDR &= ~(1 < RADIO_IRQ); // IRQ - �� ����
	csn_deassert();
	radio_deassert_ce();
	// ������������� ������������ ���������� SPI
	
	SWSPI_PORT &= ~((1 << SWSPI_MOSI) | (1 << SWSPI_SCK));
	SWSPI_DDR |= (1 << SWSPI_MOSI) | (1 << SWSPI_SCK);
	SWSPI_DDR &= ~ (1 << SWSPI_MISO);
	SWSPI_PORT |= (1 << SWSPI_MISO); // �������� �� ����� MISO
	
}

// ��������� ������� cmd, � ������ count ���� ������, ������� �� � ����� buf, ���������� ������� �������
uint8_t radio_read_buf(uint8_t cmd, uint8_t * buf, uint8_t count) {
	csn_assert();
	uint8_t status = spi_send_recv(cmd);
	while (count--) {
		*(buf++) = spi_send_recv(0xFF);
	}
	csn_deassert();
	return status;
}

// ��������� ������� cmd, � ������� count ���� ���������� �� ������ buf, ���������� ������� �������
uint8_t radio_write_buf(uint8_t cmd, uint8_t * buf, uint8_t count) {
	csn_assert();
	uint8_t status = spi_send_recv(cmd);
	while (count--) {
		spi_send_recv(*(buf++));
	}
	csn_deassert();
	return status;
}

// ������ �������� ������������� �������� reg (�� 0 �� 31) � ���������� ���
uint8_t radio_readreg(uint8_t reg) {
	csn_assert();
	spi_send_recv((reg & 31) | R_REGISTER);
	uint8_t answ = spi_send_recv(0xFF);
	csn_deassert();
	return answ;
}

// ���������� �������� ������������� �������� reg (�� 0 �� 31), ���������� ������� �������
uint8_t radio_writereg(uint8_t reg, uint8_t val) {
	csn_assert();
	uint8_t status = spi_send_recv((reg & 31) | W_REGISTER);
	spi_send_recv(val);
	csn_deassert();
	return status;
}

// ������ count ���� �������������� �������� reg (�� 0 �� 31) � ��������� ��� � ����� buf,
// ���������� ������� �������
uint8_t radio_readreg_buf(uint8_t reg, uint8_t * buf, uint8_t count) {
	return radio_read_buf((reg & 31) | R_REGISTER, buf, count);
}

// ���������� count ���� �� ������ buf � ������������� ������� reg (�� 0 �� 31), ���������� ������� �������
uint8_t radio_writereg_buf(uint8_t reg, uint8_t * buf, uint8_t count) {
	return radio_write_buf((reg & 31) | W_REGISTER, buf, count);
}

// ���������� ������ ������ � ������ FIFO ������� ��������
uint8_t radio_read_rx_payload_width() {
	csn_assert();
	spi_send_recv(R_RX_PL_WID);
	uint8_t answ = spi_send_recv(0xFF);
	csn_deassert();
	return answ;
}

// ��������� �������. ���������� ������� �������
uint8_t radio_cmd(uint8_t cmd) {
	csn_assert();
	uint8_t status = spi_send_recv(cmd);
	csn_deassert();
	return status;
}

// ���������� 1, ���� �� ����� IRQ �������� (������) �������.
uint8_t radio_is_interrupt() {
	return (radio_cmd(NOP) & ((1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT))) ? 1 : 0;
}

// ������� ���������� �������������� ��������� ����������. ���������� 1, � ������ ������, 0 � ������ ������
uint8_t radio_start() {
	uint8_t self_addr[] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; // ����������� �����
	uint8_t remote_addr[] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2}; // ����� �������� �������
	uint8_t chan = 3; // ����� �����-������ (� ��������� 0 - 125)

	radio_deassert_ce();
	for(uint8_t cnt = 100;;) {
		radio_writereg(CONFIG, (1 << EN_CRC) | (1 << CRCO) | (1 << PRIM_RX)); // ���������� �������
		if (radio_readreg(CONFIG) == ((1 << EN_CRC) | (1 << CRCO) | (1 << PRIM_RX)))
		break;
		// ���� ��������� �� �� ��� ��������, �� ������ ���� �����-��� ��� ����������������, ���� �� ��������.
		if (!cnt--)
		return 0; // ���� ����� 100 ������� �� ������� �������� ��� �����, �� ������� � �������
		_delay_ms(1);
	}
	
	radio_cmd(FLUSH_TX);
	radio_cmd(FLUSH_RX);

	radio_writereg(EN_AA, (1 << ENAA_P1)); // ��������� ����������������� ������ �� ������ 1
	radio_writereg(EN_RXADDR, (1 << ERX_P0) | (1 << ERX_P1)); // ��������� ������� 0 � 1
	radio_writereg(SETUP_AW, SETUP_AW_5BYTES_ADDRESS); // ����� ����� ������ 5 ����
	radio_writereg(SETUP_RETR, SETUP_RETR_DELAY_500MKS | SETUP_RETR_UP_TO_15_RETRANSMIT);
	radio_writereg(RF_CH, chan); // ����� ���������� ������
	radio_writereg(RF_SETUP, RF_SETUP_1MBPS | RF_SETUP_0DBM); // ����� �������� 1 ����/� � �������� 0dBm
	
	radio_writereg_buf(RX_ADDR_P0, &remote_addr[0], 5); // ������������� �������� �� ����� 0
	radio_writereg_buf(TX_ADDR, &remote_addr[0], 5);

	radio_writereg_buf(RX_ADDR_P1, &self_addr[0], 5);
	
	radio_writereg(RX_PW_P0, 32);
	radio_writereg(RX_PW_P1, 32);
	radio_writereg(DYNPD, (1 << DPL_P0) | (1 << DPL_P1)); // ��������� ������������ ����� ��� ������� 0 � 1
	radio_writereg(FEATURE, 0x06/*0x04*/); // ���������� ������������ ����� ������ ������

	radio_writereg(CONFIG,  0b00001111); // ��������� ������� (1 << EN_CRC) | (1 << CRCO) | (1 << PWR_UP) | (1 << PRIM_RX)
	return (radio_readreg(CONFIG) == ((1 << EN_CRC) | (1 << CRCO) | (1 << PWR_UP) | (1 << PRIM_RX))) ? 1 : 0;
}

// �������� ����� � ������� ��������.
// buf - ����� � �������, size - ����� ������ (�� 1 �� 32)
uint8_t send_data(uint8_t * buf, uint8_t size) {
	radio_deassert_ce(); // ���� � ������ �����, �� ��������� ���
	uint8_t conf = radio_readreg(CONFIG);
	if (!(conf & (1 << PWR_UP))) // ���� ������� �� �����-�� ������� ���������, ������������ � �������
	return 0;
	uint8_t status = radio_writereg(CONFIG, conf & ~(1 << PRIM_RX)); // ���������� ��� PRIM_RX
	if (status & (1 << TX_FULL_STATUS))  // ���� ������� ����������� ���������, ������������ � �������
	return 0;
	radio_write_buf(W_ACK_PAYLOAD/*W_TX_PAYLOAD*/, buf, size); // ������ ������ �� ��������
	radio_assert_ce(); // ������� �� ����� CE ������� � ������ ��������
	_delay_us(15); // ����� ������� 10���, ������ � �������
	radio_deassert_ce();
	return 1;
}

// ����������, ����� ��������� ����� ������� ��������, � ������������� ��� � �� ���� ��������.
void on_send_error() {
	// TODO ����� ����� ������� ���������� ��������� ��������
	PORTD &= ~(1<<7);
	_delay_ms(300);
}

// ���������� ��� ��������� ������ ������ �� ������ 1 �� �������� �������.
// buf - ����� � �������, size - ����� ������ (�� 1 �� 32)
void on_packet(uint8_t * buf, uint8_t size) {
	// TODO ����� ����� �������� ���������� ��������� ������

	// ���� �������������� ����������� �������� ������, �� ���������� ���������� �������� ,
	// �� ����� ������� ��� �������� ������������� � �����
	// ����� � ������� ����� ������ �� �������� � ����� PTX ������:
	// 130��� + ((�����_������ + �����_CRC + �����_������_�������������) * 8 + 17) / ��������_������
	// ��� �������� �������� � ������� �� 8 ��� ���������� �������������� �������� 100���
	_delay_ms(200);
	buf[0]++;
	buf[1] = size;
	//������ ��������
	send_data(buf, size);
}

void check_radio() {
	if (!radio_is_interrupt()) // ���� ���������� ���, �� �� �������������
	return;
	uint8_t status = radio_cmd(NOP);
	radio_writereg(STATUS, status); // ������ ������� ������� �������, ��� ����� ������� ���� ����������
	
	if (status & ((1 << TX_DS) | (1 << MAX_RT))) { // ��������� �������� �������, ��� ���,
		if (status & (1 << MAX_RT)) { // ���� ���������� ������������ ����� �������
			radio_cmd(FLUSH_TX); // ������ ��������� ����� �� �������
			on_send_error(); // ������� ����������
		}
		if (!(radio_readreg(FIFO_STATUS) & (1 << TX_EMPTY))) { // ���� � ������� ����������� ���� ��� ����������
			radio_assert_ce(); // ������� �� ����� CE ������� � ������ ��������
			_delay_us(15); // ����� ������� 10���, ������ � �������
			radio_deassert_ce();
			} else {
			uint8_t conf = radio_readreg(CONFIG);
			radio_writereg(CONFIG, conf | (1 << PRIM_RX)); // ������������� ��� PRIM_RX: ����
			radio_assert_ce(); // ������� ������� �� ����� CE ��������� �����-��� � ����� �����
		}
	}
	uint8_t protect = 4; // � ������� FIFO �� ������ ���� ����� 3 �������. ���� ������, ������ ���-�� �� ���
	while (((status & (7 << RX_P_NO)) != (7 << RX_P_NO)) && protect--) { // ���� � ������� ���� �������� �����
		uint8_t l = radio_read_rx_payload_width(); // ����� ����� ������
		if (l > 32) { // ������. ����� ����� ����� ��������
			radio_cmd(FLUSH_RX);
			} else {
			uint8_t buf[32]; // ����� ��� ��������� ������
			radio_read_buf(R_RX_PAYLOAD, &buf[0], l); // ������������ �����
			if ((status & (7 << RX_P_NO)) == (1 << RX_P_NO)) { // ���� datapipe 1
				on_packet(&buf[0], l); // �������� ���������� ����������� ������
			}
		}
		status = radio_cmd(NOP);
	}
}

#endif /* RADIO_H_ */