/*
 * NRF24_My_Reciever_Trasiever.c
 *
 * Created: 23.03.2016 10:12:26
 *  Author: AndreyMaznyak
 */ 


#include <avr/io.h>
#include "Radio.h"
#include "WriteProgramm.h"

#define NOT_CHECK 0
#define UPDATE_PROGRAM 1
#define PUT_PAGE 2
#define PAGE_NUMBER 3
#define GET_PAGE_DATA 4

uint8_t status = 0;
uint32_t page = 0;

// ����������, ����� ��������� ����� ������� ��������, � ������������� ��� � �� ���� ��������.
void on_send_error_IRQ() {
	// TODO ����� ����� ������� ���������� ��������� ��������
	PORTD &= ~(1<<7);
	_delay_ms(300);
}

// ���������� ��� ��������� ������ ������ �� ������ 1 �� �������� �������.
// buf - ����� � �������, size - ����� ������ (�� 1 �� 32)
void on_packet_IRQ(uint8_t * buf, uint8_t size) {
	// TODO ����� ����� �������� ���������� ��������� ������

	// ���� �������������� ����������� �������� ������, �� ���������� ���������� �������� ,
	// �� ����� ������� ��� �������� ������������� � �����
	// ����� � ������� ����� ������ �� �������� � ����� PTX ������:
	// 130��� + ((�����_������ + �����_CRC + �����_������_�������������) * 8 + 17) / ��������_������
	// ��� �������� �������� � ������� �� 8 ��� ���������� �������������� �������� 100���
	_delay_ms(200);
	if(status == GET_PAGE_DATA){
		//boot_program_page(page,buf);	
	}
	if(buf[0] == PAGE_NUMBER){
		page = (buf[4] << 24) | (buf[3] << 16) | (buf[2] << 8) | buf[1];
		status = GET_PAGE_DATA; 
	}
	//������ ��������
	send_data(buf, size);
}


ISR(INT0_vect)
{
	cli();
	PORTD|= 1<<7;
	uint8_t status = radio_cmd(NOP);
	radio_writereg(STATUS, status); // ������ ������� ������� �������, ��� ����� ������� ���� ����������
	
	if (status & ((1 << TX_DS) | (1 << MAX_RT))) { // ��������� �������� �������, ��� ���,
		if (status & (1 << MAX_RT)) { // ���� ���������� ������������ ����� �������
			radio_cmd(FLUSH_TX); // ������ ��������� ����� �� �������
			on_send_error_IRQ(); // ������� ����������
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
				on_packet_IRQ(&buf[0], l); // �������� ���������� ����������� ������
			}
		}
		status = radio_cmd(NOP);
	}
	PORTD	&= ~(1<<7);
	sei();
}


int main(void)
{
    
    //����������
    //led started
    DDRD	|= 1<<6;
    DDRD	|= 1<<7;
    
    PORTD	|= 1<<6;
    _delay_ms(1000);
	//_delay_ms(10000);
    //PORTD	&= ~(1<<6);
    //_delay_ms(1000);
    //PORTD	|= 1<<6;
    //_delay_ms(1000);
    //PORTD	&= ~(1<<6);
    //_delay_ms(1000);
    
    radio_init();
    	
    while (!radio_start()) {
	    PORTD	&= ~(1<<6);
	    _delay_ms(1000);
    }
    PORTD	|= 1<<6;
    
	
	//����������� �� ������������ INT0 �� ��������� ������
	MCUCR &= ~( (1<<ISC11)|(1<<ISC10)|(1<<ISC01)|(1<<ISC00) );
	//��������� ������� ���������� INT0
	GICR |= (1<<INT0);
	
	sei();
    
	// ����� ���������� ������� ���� � �������� CE ������ ������ ����� ����������� ��� ������ ������ �����������
    // ��� �������� ����������� � ������������� �������������� �� ����� 30��� ���������� 1.5 ��
    _delay_ms(2);
    
    radio_assert_ce();
    
    //uint8_t data = 0;//[2] = {0,1};//,3};//,2,3,4,5,6,7,8,9,10,11,12,13,14,15};//,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
	
    while(1) {
	    //data++;
	    //send_data(&data, sizeof(data));
	    //_delay_us(500);
	    //check_radio();
	   
		//_delay_ms(300);
	    // TODO ����� �������� ��� ���������
	    
	    /*if(data > 254){
		    data = 0;
			//data[1]++;
			
		    radio_init();
		    
		    while (!radio_start()) {
			    PORTD	&= ~(1<<6);
			    PORTD	&= ~(1<<7); 
				_delay_ms(1000);
		    }
		    PORTD	|= 1<<6;
		    
	    }*/
    }
}